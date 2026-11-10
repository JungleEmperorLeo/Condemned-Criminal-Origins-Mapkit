#pragma once
#include <string.h>
#include <emmintrin.h>
// Native MOPP compiler: a transcription of the Havok MOPP builder statically linked
// into the original FEAR WorldPacker.dll (hkMoppUtility::buildCode, VA 0x1006B670,
// and its call graph), recovered from the disassembly (mopp-native/research).
// The original runs with x87 control word 0x027F (53-bit significand, round to
// nearest); register values are modelled with double arithmetic and every float
// store of the original is an explicit F(). Nodes, primitives and the work buffer
// keep the original byte layouts and sizes so that pool exhaustion, the partial
// (interleaved) assembly and buffer reuse behave identically.
// Input: one triangle mesh (float xyz vertices, uint16 indices), the packer's
// radius 0.03f and default hkMoppCompilerInput (0x1006B830). Output: CodeInfo
// (origin xyz, scale) and the byte code.
namespace NativeMopp {
typedef unsigned char u8;typedef unsigned int u32;
#define MI32(p,o) (*(int*)((u8*)(p)+(o)))
#define MU32(p,o) (*(u32*)((u8*)(p)+(o)))
#define MF32(p,o) (*(float*)((u8*)(p)+(o)))
#define MU8(p,o) (*(u8*)((u8*)(p)+(o)))
#define MPTR(p,o) (*(u8**)((u8*)(p)+(o)))
inline float F(double v){volatile float f=(float)v;return f;}
// IEEE square root (fsqrt at 53-bit precision) and |x| without the CRT.
inline double Sqrt(double v){return _mm_cvtsd_f64(_mm_sqrt_sd(_mm_set_sd(v),_mm_set_sd(v)));}
inline double Abs(double v){return v<0?-v:v;}
inline u32 Bits(float f){u32 b;memcpy(&b,&f,4);return b;}
inline float FromBits(u32 b){float f;memcpy(&f,&b,4);return f;}
// 0x10071D00: floor by bit manipulation (returns a float).
inline float FloorBits(float value){
 int a=(int)Bits(value),e=((a>>23)&0xFF)-0x7F,ecx=0x17-e,eax=ecx>>31,edi=a&0x7FFFFFFF;edi--;edi>>=31;edi=~edi;int esi=a&edi;
 eax=~eax;edi=eax&ecx;ecx=(eax&0x17)-edi;edi=(int)0xFF800000>>(ecx&31);eax=~eax;edi|=eax;int ebp=~edi&esi;
 int edx=0x800000>>(e&31),ebx=esi>>31;eax=e>>31;ebp--;ebp=~ebp;ecx=~eax;eax&=ebx;ebp&=edx;ebp&=ebx;ebp+=esi;ebp&=ecx;ebp&=edi;eax&=(int)0xBF800000;ebp|=eax;
 return FromBits((u32)ebp);
}
// 0x10071DA0: truncation toward zero by bit manipulation.
inline int TruncBits(float value){
 int a=(int)Bits(value),e=((a>>23)&0xFF)-0x7F,ebx=e-0x17,ecx=(ebx-1)>>31,ebp=~ecx,edi=a&0x7FFFFFFF;ebx&=ebp;int esi=(0x17-e)&ecx;
 edi--;edi>>=31;edi=~edi;int eax=a&edi;ecx=(ecx&0x17)-esi;edi=(int)0xFF800000>>(ecx&31);ecx=eax;eax=(eax&0x7FFFFF)|0x800000;edi|=ebp;eax&=edi;edi=eax;
 ecx>>=31;eax=(edi+edi-1)|ecx;eax=eax-edi+1;int edx=~(e>>31);eax&=edx;eax>>=(esi&31);eax=(int)((u32)eax<<(ebx&31));return eax;
}
inline int FloorInt(double v){return TruncBits(FloorBits(F(v)));}
inline int BitLength(u32 v){int n=0;while(v){v>>=1;n++;}return n;}

// Split plane directions (assembler constructor 0x1006F360): 13 entries of 32 bytes.
struct Plane {float n[4];float cost;int index;int pad[2];};
inline void BuildPlanes(Plane* p){
 static const float d[13][3]={{1,0,0},{0,1,0},{0,0,1},{0,1,1},{1,0,1},{1,1,0},{0,1,-1},{1,0,-1},{1,-1,0},{1,1,1},{1,1,-1},{1,-1,-1},{1,-1,-1}};
 static const u32 cost[13]={0,0,0,0x3E4CCCCD,0x3E4CCCCD,0x3E4CCCCD,0x3E800000,0x3E800000,0x3E800000,0x3E99999A,0x3EA3D70A,0x3EA3D70A,0x3EAE147B};
 for(int i=0;i<13;i++){
  memset(&p[i],0,sizeof(Plane));p[i].n[0]=d[i][0];p[i].n[1]=d[i][1];p[i].n[2]=d[i][2];p[i].n[3]=0;p[i].cost=FromBits(cost[i]);
  double x=p[i].n[0],y=p[i].n[1],z=p[i].n[2];double s=(z*z+x*x)+y*y,inv=s==0.0?0.0:1.0/Sqrt(s);
  for(int k=0;k<4;k++)p[i].n[k]=F(inv*p[i].n[k]);p[i].index=i;
 }
}

// The mesh shape and the shape mediator (vtable 0x100915D4).
struct Mesh {const float* vertices;const unsigned short* indices;const unsigned int* indices32;u32 triangles;float radius;}; // CONDEMNED FORK: indices32 is used when set (32-bit subpart)
struct Triangle {float v[3][3];};
inline void GetTriangle(const Mesh& m,u32 key,Triangle& t){u32 i=key&(0xFFFFFFFFu>>12);for(int j=0;j<3;j++){const float* p=m.vertices+3*(m.indices32?m.indices32[3*i+j]:m.indices[3*i+j]);for(int k=0;k<3;k++)t.v[j][k]=F((double)p[k]*1.0);}}
// 0x1006BC30: degenerate triangle test.
inline bool Degenerate(const Triangle& t){
 const float* v0=t.v[0];const float* v1=t.v[1];const float* v2=t.v[2];
 double ax=(double)v0[0]-v1[0],ay=(double)v0[1]-v1[1],az=(double)v0[2]-v1[2];float bx=F((double)v0[0]-v2[0]),by=F((double)v0[1]-v2[1]);double bz=(double)v0[2]-v2[2];
 float cx=F(bz*ay-(double)by*az),cy=F(az*bx-bz*ax);double cz=ax*by-ay*(double)bx;
 double l=(cz*cz+(double)cy*cy)+(double)cx*cx;return l<(double)FromBits(0x3089705Fu);
}
// 0x1006B8D0 / 0x1006BCD0 (one subpart, 12 subpart bits).
inline int NextKey(const Mesh& m,u32 key){
 u32 k=key;for(;;){k++;u32 sub=k>>20,tri=k&(0xFFFFFFFFu>>12);if(sub==0&&tri>=m.triangles)k=1u<<20;if((k>>20)>=1)return -1;Triangle t;GetTriangle(m,k,t);if(!Degenerate(t))return (int)k;}
}
// 0x1006D900 support vertex and 0x1006D360 maximum projection.
inline double MaxProjection(const Triangle& t,const float* d,float radius){
 double dot[3];for(int j=0;j<3;j++)dot[j]=((double)d[2]*t.v[j][2]+(double)d[1]*t.v[j][1])+(double)d[0]*t.v[j][0];
 int idx;double best;if(dot[2]>dot[1]){best=(double)F(dot[2]);idx=2;}else{best=dot[1];idx=1;}if(dot[0]>best)idx=0;
 const float* s=t.v[idx];return (((double)s[2]*d[2]+(double)s[1]*d[1])+(double)s[0]*d[0])+(double)radius;
}
struct Mediator {
 Mesh mesh;int numPrimitives;
 void Init(const Mesh& m){mesh=m;numPrimitives=0;for(int k=0;k!=-1;k=NextKey(mesh,(u32)k))numPrimitives++;}
 void Fill(u8* prims){int key=0;for(int i=0;i<numPrimitives;i++){MU32(prims,16*i)=(u32)key;MU32(prims,16*i+4)=0;key=NextKey(mesh,(u32)key);}}
 // 0x1006C8F0
 void ProjectPrimitive(const u8* prim,const float* dir,float& mn,float& mx)const{
  Triangle t;GetTriangle(mesh,MU32(prim,0),t);mx=F(MaxProjection(t,dir,mesh.radius));
  float neg[4]={-dir[0],-dir[1],-dir[2],-dir[3]};mn=F(-MaxProjection(t,neg,mesh.radius));
 }
 // 0x1006C980 (stores per-primitive extents at +8/+C) and 0x1006CA10.
 void Project(const float* dir,u8* prims,int n,float* outMin,float* outMax,bool store)const{
  *outMin=FromBits(0x7F7FFFFF);*outMax=FromBits(0xFF7FFFFF);
  for(int i=0;i<n;i++){u8* p=prims+16*i;float mn,mx;ProjectPrimitive(p,dir,mn,mx);if(store){MF32(p,12)=mx;MF32(p,8)=mn;}if(mn<*outMin)*outMin=mn;if(mx>*outMax)*outMax=mx;}
 }
};

// Code writer (vtable 0x10091A3C): bytes are written backwards from the end.
struct Writer {
 u32 capacity,count;u8* buffer;
 static u8* Alloc(u32 n){return new u8[n?n:1];}
 void Init(u32 n){capacity=n;count=0;buffer=Alloc(n);}
 void Release(){delete[] buffer;buffer=0;}
 void Grow(){u8* b=Alloc(capacity*2);memcpy(b+capacity,buffer,capacity);capacity*=2;delete[] buffer;buffer=b;}
 void Put(int v){buffer[capacity-count-1]=(u8)v;count++;if(count>=capacity)Grow();}
 void PutSum(int a,int b){Put((u8)a+(u8)b);}                                   // 0x1006FFF0
 void Put2(int op,int v){Put(v);Put(op);}                                       // 0x10070020
 void Put3(int op,int v){Put(v);Put(v>>8);Put(op);}                             // 0x10070080
 void Put4(int op,int v){Put(v);Put(v>>8);Put(v>>16);Put(op);}                  // 0x10070110
 void Put24(int v){Put(v);Put(v>>8);Put(v>>16);}                                // 0x100701D0
 void Put5(int op,int v){u32 u=(u32)v;Put(u);Put(u>>8);Put(u>>16);Put(u>>24);Put(op);} // 0x10070260
};

// Node layout (internal 0xF8 bytes, terminal 0xBC bytes).
enum {N_PARENT=0x00,N_TERMINAL=0x04,N_COUNT=0x08,N_EXT=0x0C,N_KEYMIN=0x24,N_KEYMAX=0x28,N_NUMPROPS=0x2C,N_PROPS=0x30,
 N_DONE=0x38,N_MARK=0x39,N_RESCALE=0x3C,N_FLAG=0x40,N_LO=0x44,N_HI=0x50,N_POS=0x5C,
 N_PLANE=0xB8,N_SPLIT=0xBC,N_END=0xC0,N_BEST=0xC4,N_DETAIL=0xC8,N_CHILD0=0xEC,N_CHILD1=0xF0,N_NUMPRIMS=0xF4};
enum {InternalSize=0xF8,TerminalSize=0xBC};

struct Splitter;struct Assembler;
// Cost function (vtable 0x10091A6C); costParams at +8..+1C.
struct CostFunction {
 float w0,w1,w2,w3,w4,half;
 // SC: split candidate context of 0x10070A00 (see Candidate).
 double Position(const u8* sc)const{ // 0x10071860
  double x=((double)MF32(sc,0x0C)+MF32(sc,0x08))*FromBits(0x3F000000);x=(double)MF32(sc,0x20)-x;double f;
  if(MI32(sc,0)<MI32(sc,4)){double w=(double)MF32(sc,0x0C)-MF32(sc,0x08);double d=((double)half<w)?w+half:w+w;f=1.0/d;}else f=MF32(sc,0x10);
  x*=f;if(x<0.0)return 0.0;return Poly(x)*w3;
 }
 double End(const u8* sc)const{ // 0x100718F0
  double x=((double)MF32(sc,0x0C)+MF32(sc,0x08))*FromBits(0x3F000000);x=(double)MF32(sc,0x38)-x;double f;
  if(MI32(sc,0)<MI32(sc,4)){double w=(double)MF32(sc,0x0C)-MF32(sc,0x08);double d=((double)half<w)?w+half:w+w;f=1.0/d;}else f=MF32(sc,0x10);
  x*=f;if(x>0.0)return 0.0;return Poly(x)*w3;
 }
 static double Poly(double x){double a=x*x,b=a*a,c=b*b;return ((c*FromBits(0x44BB8000))+(b*FromBits(0x40400000)))+(a*FromBits(0x4039999A));}
 double Balance(const u8* sc)const{ // 0x10071800
  double r=Abs((double)(MI32(sc,0x14)-MI32(sc,0x34)))*MF32(sc,0x1C);double q;
  if(MI32(sc,0)<MI32(sc,4)){q=(r-FromBits(0x3F400000))*FromBits(0x40C00000);if(q<0.0)return 0.0;}else q=r*r;
  double q2=q*q,q3=q2*q,q4=q3*q;return q4*w2;
 }
 double Base(const u8* sc)const{ // vfn 4, 0x10071A00
  float t=F(Position(sc));double v=MF32(MPTR(sc,0x24),0x10);
  if(MI32(sc,0x2C)-MI32(sc,0x28)<16)return (v+(double)w4*FromBits(0xBCF5C28F))+t;return (v+0.0)+t;
 }
 double Split(const u8* sc)const{ // vfn 8, 0x10071A60
  float t1=F(End(sc));double t2=Balance(sc);
  double s=(((double)MI32(sc,0x30)*MF32(sc,0x1C))*w0)*FromBits(0x40A00000);
  double o=(((double)MF32(sc,0x38)-MF32(sc,0x20))*MF32(sc,0x10))*w1;
  return (t2+(o+s))+t1;
 }
 void Detail(const u8* sc,u8* out)const{ // vfn 0x10, 0x10071AB0
  MF32(out,4)=F(Position(sc));MF32(out,0x0C)=F(End(sc));MF32(out,0x10)=F(Balance(sc));
  MF32(out,0)=F((((double)MI32(sc,0x30)*MF32(sc,0x1C))*w0)*FromBits(0x40A00000));
  MF32(out,0x14)=F((((double)MF32(sc,0x38)-MF32(sc,0x20))*MF32(sc,0x10))*w1);
  MU32(out,0x18)=MU32(sc,8);MU32(out,0x1C)=MU32(sc,0x0C);
 }
};

// Sorted list of primitives overlapping the sweep position (by max, descending).
struct ActiveList {int count,capacity;u8** data;u8* last;
 void Insert(u8* prim){ // 0x100703B0
  float key=MF32(prim,12);int j=count-1;while(j>=0&&data[j]&&MF32(data[j],12)<key){data[j+1]=data[j];j--;}data[j+1]=prim;count++;
 }
 void RemoveBelow(float coord){ // 0x100704E0
  int j=count-1;while(j>=0&&data[j]&&MF32(data[j],12)<coord){last=data[j];data[j]=0;count--;j--;}
 }
};

struct Assembler;
struct Splitter { // vtable 0x10091A48
 // params at +8: {0.2f, 1000, 50, 5, 5}
 float p0;int maxExtra,listLimit,window,windowEnable;
 ActiveList* list;u8* freeNodes;int freeNodeCount;u8* freeTerms;int freeTermCount;
 CostFunction* cf;Assembler* assembler;Mediator* mediator;u8* root;int numPlanes;Plane* planes;int bitlen;
 void Release(u8* node){ // 0x10070DF0
  if(!node)return;if(MU8(node,N_TERMINAL)){MPTR(node,0)=freeTerms;freeTerms=node;freeTermCount++;}else{MPTR(node,0)=freeNodes;freeNodes=node;freeNodeCount++;}
 }
 u8* BuildTree(u8* buffer,u8* nodePool,u8* termPool);
 u8* BuildNode(u8* parent,int* range,int side,int depth);
 u8* Terminal(u8* parent,int* range);
 void Keys(int* range,u8* node);
 void AxisOrder(const u8* parent,int* order,float* cost);
 void Evaluate(u8* node,Plane* plane,int* range,float* ext,int depth);
 void Candidates(int i,u8* node,int* best,u8* sc,int* range);
 void Partition(int* range,u8* node,int depth,int* outA,int* outB);
 void Classify(int kind,u8* node,int depth,int* extra,int* dup,u8** cur,u8** end,u8** cap,u8** cap2);
 void Distribute(u8** startA,u8** endA,u8** startB,u8** endB,u8** either,u8** eitherEnd,u8* node);
};

// 0x10070E60: quicksort of 16-byte primitives by their min (+8).
inline void SortPrimitives(u8* a,int lo,int hi){
 for(;;){
  u8 pivot[16];memcpy(pivot,a+16*((lo+hi)>>1),16);float p=MF32(pivot,8);int i=lo,j=hi;
  do{
   while(MF32(a+16*i,8)<p)i++;
   while(p<MF32(a+16*j,8))j--;
   if(j<i)break;
   if(j!=i){u8 t[16];memcpy(t,a+16*j,16);memcpy(a+16*j,a+16*i,16);memcpy(a+16*i,t,16);}
   j--;i++;
  }while(i<=j);
  if(lo<j)SortPrimitives(a,lo,j);
  if(i<hi){lo=i;continue;}
  return;
 }
}

// Assembler state (0x60 bytes; 0x48 copied between levels).
enum {Q_LEVEL=0x00,Q_FLAG=0x04,Q_BITS=0x08,Q_EXT=0x0C,Q_SHIFT=0x24,Q_OFF=0x28,Q_KEYOFF=0x34,Q_KEYRANGE=0x38,Q_NUMPROPS=0x3C,Q_PROPMIN=0x40,Q_PROPRANGE=0x44,Q_ALIGNED=0x48,Q_SIZE=0x60,Q_COPY=0x48};
struct Rescale {u8 flag;int delta;int q[3];};

// Research-only observers (unset in production): each assemble call, each
// terminal bounds emission.
typedef void (*TreeObserver)(u8* root,int limit);
inline TreeObserver& Observer(){static TreeObserver o=0;return o;}
typedef void (*BoundsObserver)(const u8* node,const u8* state);
inline BoundsObserver& BoundsWatch(){static BoundsObserver o=0;return o;}
struct Assembler { // vtable 0x10091A28
 Mediator* mediator;Splitter* splitter;Writer* writer;
 int depthLimit,halfBits,limit,counter;float info[4];
 float absTri,relInt,absInt,tol[3],unused;int step;Plane planes[13];
 void Init(Mediator* m,Writer* w){
  mediator=m;writer=w;splitter=0;depthLimit=halfBits=limit=counter=0;memset(info,0,sizeof(info));
  absTri=FromBits(0x3ECCCCCD);relInt=FromBits(0x3DCCCCCD);absInt=FromBits(0x3E99999A);tol[0]=tol[1]=tol[2]=FromBits(0x3D4CCCCD);unused=0;step=4;BuildPlanes(planes);
 }
 // 0x1006F2C0
 void ComputeInfo(const u8* node,float* out){
  out[1]=MF32(node,0x14);out[0]=MF32(node,0x0C);out[3]=0;out[2]=MF32(node,0x1C);
  double m=0.0;float d=F((double)MF32(node,0x10)-MF32(node,0x0C));if(!(0.0>d))m=d;
  d=F((double)MF32(node,0x18)-MF32(node,0x14));if(!(m>d))m=d;
  d=F((double)MF32(node,0x20)-MF32(node,0x1C));if(!(m>d))m=d;
  out[3]=F((FromBits(0x437E0000)/m)*FromBits(0x47800000));
 }
 static void Keys(u8* q,const u8* node){ // 0x1006E200
  MI32(q,Q_KEYRANGE)=MI32(node,N_KEYMAX)-MI32(node,N_KEYMIN);MI32(q,Q_NUMPROPS)=MI32(node,N_NUMPROPS);
  for(int i=0;i<MI32(node,N_NUMPROPS);i++){MI32(q,Q_PROPMIN+4*i)=MI32(node,N_PROPS+4*i);MI32(q,Q_PROPRANGE+4*i)=MI32(node,N_PROPS+4+4*i)-MI32(node,N_PROPS+4*i);}
 }
 static void Quantize(u8* q,const u8* node,const float* in){ // 0x1006E700
  for(int d=0;d<3;d++){
   MI32(q,Q_EXT+8*d)=FloorInt(((double)MF32(node,N_EXT+8*d)-in[d])*in[3]);
   MI32(q,Q_EXT+4+8*d)=FloorInt(((double)MF32(node,N_EXT+4+8*d)-in[d])*in[3])+1;
  }
 }
 static void NeededBits(u8* q){ // 0x1006E790
  int r=MI32(q,Q_EXT+4)-MI32(q,Q_EXT);int y=MI32(q,Q_EXT+12)-MI32(q,Q_EXT+8);if(!(r>y))r=y;int z=MI32(q,Q_EXT+20)-MI32(q,Q_EXT+16);if(!(r>z))r=z;
  int bits=BitLength((u32)r);u32 e=(1u<<((bits-4)&31))+(u32)r;int b=BitLength(e);MI32(q,Q_BITS)=b>24?24:b;
 }
 static void Align(u8* q){ // 0x1006E250
  int s=MI32(q,Q_SHIFT);
  for(int d=0;d<3;d++){MI32(q,Q_ALIGNED+4*d)=(MI32(q,Q_EXT+8*d)>>s)<<s;MI32(q,Q_ALIGNED+12+4*d)=((MI32(q,Q_EXT+4+8*d)>>s)+1)<<s;}
 }
 void Child(u8* q,const u8* src,const u8* node){ // 0x1006E810
  memcpy(q,src,Q_COPY);Keys(q,node);Quantize(q,node,info);MI32(q,Q_LEVEL)=MI32(src,Q_LEVEL)+1;MU8(q,Q_FLAG)=0;NeededBits(q);
 }
 static bool NeedRescale(const u8* qp,const u8* qc){ // 0x1006E860
  int c=MI32(qc,Q_BITS)-8;if(!(c>0))c=0;int e=MI32(qp,Q_SHIFT)-c;if(e>MI32(qp,Q_SHIFT))e=MI32(qp,Q_SHIFT);return e>2;
 }
 static void DoRescale(const u8* qp,u8* qc,Rescale& out){ // 0x1006E8A0
  int c=MI32(qc,Q_BITS)-8;if(!(c>0))c=0;int delta=MI32(qp,Q_SHIFT)-c;if(delta>MI32(qp,Q_SHIFT))delta=MI32(qp,Q_SHIFT);out.flag=0;
  if(!(delta>0)){for(int k=0;k<4;k++)MI32(qc,Q_SHIFT+4*k)=MI32(qp,Q_SHIFT+4*k);return;}
  if(delta>=4)delta=4;
  for(;;){
   MI32(qc,Q_SHIFT)=MI32(qp,Q_SHIFT)-delta;bool fit=true;
   for(int i=0;i<3;i++){
    int off=MI32(qp,Q_OFF+4*i),s=MI32(qp,Q_SHIFT);int q=(MI32(qc,Q_EXT+8*i)-off)>>(s&31);MI32(qc,Q_OFF+4*i)=(q<<(s&31))+off;out.q[i]=q;
    int hi=(MI32(qc,Q_EXT+4+8*i)-MI32(qc,Q_OFF+4*i))>>(MI32(qc,Q_SHIFT)&31);if(hi>=0xFF){delta--;fit=false;break;}
   }
   if(fit)break;
  }
  out.delta=delta;out.flag=1;
 }
 static void MarkUp(u8* node){for(u8* p=node;p&&!MU8(p,N_MARK);p=MPTR(p,N_PARENT))MU8(p,N_MARK)=1;} // 0x1006E2F0
 void ReleaseChildren(u8* node){ // 0x1006E2A0
  if(MU8(node,N_TERMINAL))return;splitter->Release(MPTR(node,N_CHILD0));splitter->Release(MPTR(node,N_CHILD1));MPTR(node,N_CHILD0)=0;MPTR(node,N_CHILD1)=0;
 }
 bool Ready(const u8* node,const u8* qc){return MI32(node,N_COUNT)<0x16||MI32(qc,Q_LEVEL)>depthLimit;} // 0x1006E320
 static void KeyOffset(const u8* node,const u8* qp,u8* qc){ // 0x1006E350
  MI32(qc,Q_KEYOFF)=MI32(qp,Q_KEYOFF);u32 range=MU32(qc,Q_KEYRANGE),pr=MU32(qp,Q_KEYRANGE);u32 e=range-MU32(qp,Q_KEYOFF)+MU32(node,N_KEYMIN);
  if(e<0x20)return;if(range<=2&&e<0x100)return;
  if((range<0x20&&pr>=0x20)||(range<0x100&&pr>=0x100)||(range<0x10000&&pr>=0x10000))MI32(qc,Q_KEYOFF)=MI32(node,N_KEYMIN);
 }
 void PropertyCommand(int i,int v){ // 0x1006E3C0
  if(v<0)writer->Put5(i+0x68,v);else if(v<0x100)writer->Put2(i+0x60,v);else if(v<0x10000)writer->Put3(i+0x64,v);else writer->Put5(i+0x68,v);
 }
 void RescaleCommand(const Rescale& r){writer->Put(r.q[2]);writer->Put(r.q[1]);writer->Put(r.q[0]);writer->Put(r.delta);} // 0x1006E430
 void KeyOffsetCommand(u32 d){if(d<0x100)writer->Put2(9,(int)d);else if(d<0x10000)writer->Put3(0x0A,(int)d);else writer->Put5(0x0B,(int)d);} // 0x1006E470
 void Bounds(const u8* node,const u8* qc){ // 0x1006E4B0
  for(int i=0;i<3;i++){
   if(MU8(node,N_FLAG+i)==1){int off=MI32(qc,Q_OFF+4*i),s=MI32(qc,Q_SHIFT)&31;int hi=(MI32(qc,Q_EXT+4+8*i)-off)>>s,lo=(MI32(qc,Q_EXT+8*i)-off)>>s;writer->Put(hi+1);writer->Put(lo);writer->Put(0x26+i);}
   if(MU8(node,N_FLAG+i)==2){writer->Put24(MI32(qc,Q_EXT+4+8*i)+1);writer->Put24(MI32(qc,Q_EXT+8*i));writer->Put(0x29+i);}
  }
 }
 void SubtreeProperty(const u8* node,const u8* qp,const u8* qc){ // 0x1006E590
  if(!MPTR(node,N_PARENT)){if(!MI32(qc,Q_PROPRANGE)&&MI32(qc,Q_PROPMIN))PropertyCommand(0,MI32(qc,Q_PROPMIN));return;}
  if(MI32(qc,Q_PROPRANGE))return;if(!MI32(qp,Q_PROPRANGE))return;PropertyCommand(0,MI32(qc,Q_PROPMIN));
 }
 int TerminalCommand(const u8* node,const u8* qp,const u8* qc){ // 0x1006E5E0
  u32 start=writer->count;u32 v=MU32(MPTR(node,N_PLANE),0)-MU32(qc,Q_KEYOFF);
  if(v<0x20)writer->PutSum(0x30,(int)v);else if(v<0x100)writer->Put2(0x50,(int)v);else if(v<0x10000)writer->Put3(0x51,(int)v);else if(v<0x1000000)writer->Put4(0x52,(int)v);else writer->Put5(0x53,(int)v);
  for(int i=0;i<MI32(node,N_NUMPROPS);i++)if(MI32(node,N_PROPS+4*i)&&MI32(qp,Q_PROPRANGE+4*i))PropertyCommand(i,MI32(node,N_PROPS+4*i));
  return (int)(writer->count-start);
 }
 void Jump(u32 pos){ // 0x1006E6B0
  int d=(int)(writer->count-pos);if(d<=0)return;if(d<0xFF)writer->Put2(5,d);else if(d<0xFFFF)writer->Put3(6,d);else if(d<0xFFFFFF)writer->Put4(7,d);else writer->Put5(8,d);
 }
 void SplitCommand(int a1,int a2,int axis,u32 pos0,u32 pos1){ // 0x1006ED30
  int d1=(int)(writer->count-pos1);u32 edi=pos1;
  if(axis<3){
   int d0=(int)(writer->count-pos0);
   if((d0>0||d1>0xF8)&&d0<0x10000&&d1<0x10000){writer->Put(d1&0xFF);writer->Put((d1>>8)&0xFF);writer->Put(d0&0xFF);writer->Put((d0>>8)&0xFF);writer->Put(a1);writer->Put(a2);writer->Put(axis+0x23);return;}
   if(a2-a1==1){if(d1>0xFB){Jump(pos1);edi=writer->count;}Jump(pos0);writer->Put((int)(writer->count-edi));writer->Put(a1);writer->Put(axis+0x20);return;}
  }
  if(d1>0xFB){Jump(pos1);edi=writer->count;}Jump(pos0);writer->Put((int)(writer->count-edi));writer->Put(a1);writer->Put(a2);writer->Put(axis+0x10);
 }
 static int AxisCode(const Plane* p){ // 0x1006EEA0
  const double hi=FromBits(0x3DCCCCCD),lo=FromBits(0xBDCCCCCD);double x=p->n[0],y=p->n[1],z=p->n[2];
  if(x>hi){
   if(z>hi){if(y>hi)return 9;if(y<lo)return 11;return 5;}
   if(z<lo){if(y>hi)return 10;if(y<lo)return 12;return 6;}
   if(y>hi)return 7;if(y<lo)return 8;return 0;
  }
  if(y>hi){if(z>hi)return 3;if(z<lo)return 4;return 1;}
  return 2;
 }
 void SplitValues(const u8* node,const u8* qc,int* out3,int* out4){ // 0x1006EFD0
  const Plane* p=(const Plane*)MPTR(node,N_PLANE);int nonzero=0,negative=0,last=0;
  if(p->n[0]!=0.0f){nonzero=1;if(p->n[0]<0.0f)negative=1;}
  if(p->n[1]!=0.0f){nonzero++;last=1;if(p->n[1]<0.0f)negative++;}
  if(p->n[2]!=0.0f){nonzero++;last=2;if(p->n[2]<0.0f)negative++;}
  double bc=MF32(node,N_SPLIT),c0=MF32(node,N_END);
  if(nonzero==1){
   int a=FloorInt((bc-info[last])*info[3]);int b=FloorInt(info[3]*(c0-info[last]));
   int off=MI32(qc,Q_OFF+4*last),s=MI32(qc,Q_SHIFT)&31;b=(b-off)>>s;a=(a-off)>>s;b++;
   if(a<=0)a=0;else if(a>=0xFF)a=0xFF;if(b<=0)b=0;else if(b>=0xFF)b=0xFF;*out4=a;*out3=b;if(*out4<0)*out4=0;return;
  }
  double qx=MI32(qc,Q_OFF),qy=MI32(qc,Q_OFF+4);float qz=F((double)MI32(qc,Q_OFF+8));
  double inv=1.0/(double)info[3];float invF=F(inv);float px=F(inv*qx+(double)info[0]);
  double py=qy*invF+(double)info[1];double pz=(double)qz*invF+(double)info[2];
  double d0=(pz*p->n[2]+py*p->n[1])+(double)px*p->n[0];
  double inv2=1.0/(double)(1<<(MI32(qc,Q_SHIFT)&31));
  double v1=((bc-d0)*info[3])*inv2;double v2=((c0-d0)*info[3])*inv2;
  double k=1.0,w=1.0;if(nonzero==2){k=1.4142135381698608;w=0.5;}else if(nonzero==3){k=1.7320507764816284;w=0.3333333333333333;}
  v1=v1*k;v2=k*v2;
  if(negative){double t=(double)(negative*0xFF);v1=v1+t;v2=v2+t;}
  int a=FloorInt(w*v1);int b=FloorInt(w*v2)+1;
  if(a<=0)a=0;else if(a>=0xFF)a=0xFF;
  if(b<=0){*out4=a;*out3=0;return;}if(b>=0xFF)b=0xFF;*out4=a;*out3=b;
 }
 void TerminalBounds(u8* node,const u8* qc){ // 0x1006EC40
  for(int d=0;d<3;d++){
   float width=F((double)MF32(node,N_EXT+4+8*d)-MF32(node,N_EXT+8*d));double q=(double)(1<<(MI32(qc,Q_SHIFT)&31))/(double)info[3];
   if(!(q>(double)absInt))q=absInt;
   if(width<tol[d]){if(!(q<(double)tol[d]))q=tol[d];}
   int t=TruncBits(F((double)info[3]*q));
   MI32(node,N_LO+4*d)=MI32(qc,Q_EXT+8*d)-t;MI32(node,N_HI+4*d)=MI32(qc,Q_EXT+4+8*d)+t+1;
  }
 }
 int Precision(u8* node,u8* qp,u8* qc); // 0x1006E9B0
 void Ranges(u8* node,u8* qp,u8* qc);   // 0x1006F700
 int Emit(u8* node,u8* qp,u8* qc);      // 0x1006F9B0
 void Assemble(u8* root,Splitter* s,int lim); // 0x1006FCC0
};

inline int Assembler::Precision(u8* node,u8* qp,u8* qc){
 if(MU8(node,N_TERMINAL)){
  float cur=absInt;
  for(int i=0;i<3;i++){
   if(!MU8(node,N_MARK))MU8(node,N_FLAG+i)=1;
   double width=(double)MF32(node,N_EXT+4+8*i)-MF32(node,N_EXT+8*i);float t=tol[i];
   if(width<t&&t<cur){int v=TruncBits(F((double)info[3]*t));if((v<<7)<(1<<(MI32(qc,Q_BITS)&31))){if(!MU8(node,N_MARK))MU8(node,N_FLAG+i)=2;}else cur=t;}
  }
  int n=TruncBits(F(((double)info[3]*FromBits(0x3F666666))*cur));int r=BitLength((u32)n)+6;return r>MI32(qc,Q_BITS)?r:MI32(qc,Q_BITS);
 }
 if(!MU8(node,N_MARK)&&MI32(qc,Q_LEVEL)<halfBits&&NeedRescale(qp,qc))MU8(node,N_RESCALE)=1;
 if(MU8(node,N_RESCALE)==1){Rescale tmp;DoRescale(qp,qc,tmp);}
 int r[2]={-1,-1};int minBits=0x7FFFFFFF;u8* child[2]={MPTR(node,N_CHILD1),MPTR(node,N_CHILD0)};
 for(int k=0;k<2;k++){
  if(!child[k])continue;u8 qt[Q_SIZE];memset(qt,0,sizeof(qt));memcpy(qt,qc,Q_COPY);Keys(qt,child[k]);Quantize(qt,child[k],info);MI32(qt,Q_LEVEL)=MI32(qc,Q_LEVEL)+1;MU8(qt,Q_FLAG)=0;NeededBits(qt);
  r[k]=Precision(child[k],qc,qt);if(!(minBits<MI32(qt,Q_BITS)))minBits=MI32(qt,Q_BITS);
 }
 int bits=MI32(qc,Q_BITS);int eax=minBits+2;if(bits+2<eax)eax=bits+2;
 // child0 first, then child1 (r[1] is child0, r[0] child1)
 u8* c0=child[1];int r0=r[1];
 if(c0){
  if(MU8(node,N_MARK)){if(!MU8(c0,N_MARK)&&r0<eax)MU8(c0,N_RESCALE)=1;}else if(r0<bits)MU8(c0,N_RESCALE)=1;
  if(!MU8(c0,N_RESCALE)&&!(eax<r0))eax=r0;
 }
 u8* c1=child[0];int r1=r[0];
 if(c1){
  if(MU8(node,N_MARK)){if(!MU8(c1,N_MARK)&&r1<eax)MU8(c1,N_RESCALE)=1;}else if(r1<bits)MU8(c1,N_RESCALE)=1;
  if(!MU8(c1,N_RESCALE)&&!(eax<r1))eax=r1;
 }
 return eax;
}

inline void Assembler::Ranges(u8* node,u8* qp,u8* qc){
 if(MU8(node,N_DONE))return;
 if(MU8(node,N_TERMINAL)){Align(qc);TerminalBounds(node,qc);return;}
 if(MU8(node,N_RESCALE)==1){Rescale tmp;DoRescale(qp,qc,tmp);}
 Align(qc);
 int r=AxisCode((const Plane*)MPTR(node,N_PLANE));
 u8* c1=MPTR(node,N_CHILD1);u8* c0=MPTR(node,N_CHILD0);
 for(int i=0;i<3;i++){MI32(node,N_LO+4*i)=0;MI32(node,N_HI+4*i)=0x7FFFFFFF;}
 u8* child[2]={c1,c0};
 for(int k=0;k<2;k++){
  u8* c=child[k];if(!c)continue;
  u8 qt[Q_SIZE];memset(qt,0,sizeof(qt));memcpy(qt,qc,Q_COPY);Keys(qt,c);Quantize(qt,c,info);MI32(qt,Q_LEVEL)=MI32(qc,Q_LEVEL)+1;MU8(qt,Q_FLAG)=0;NeededBits(qt);
  Ranges(c,qc,qt);
  if(c==c1){if(r<=2)MI32(c,N_LO+4*r)=0;}else{if(r<=2)MI32(c,N_HI+4*r)=0x7FFFFFFF;}
  for(int a=0;a<3;a++){
   signed char f=(signed char)MU8(c,N_FLAG+a);
   if(f==1){if(!(MI32(c,N_LO+4*a)>MI32(qc,Q_ALIGNED+4*a))&&!(MI32(c,N_HI+4*a)<MI32(qc,Q_ALIGNED+12+4*a))){int nf=(signed char)MU8(node,N_FLAG+a);MU8(node,N_FLAG+a)=(u8)(nf>=1?nf:1);MU8(c,N_FLAG+a)=0;}}
   else if(f==2){if(!(MI32(c,N_LO+4*a)>MI32(qc,Q_EXT+8*a))&&!(MI32(c,N_HI+4*a)<MI32(qc,Q_EXT+4+8*a))){MU8(node,N_FLAG+a)=2;MU8(c,N_FLAG+a)=0;}}
   if(!MU8(c,N_FLAG+a)){int lo=MI32(c,N_LO+4*a);if(MI32(node,N_LO+4*a)>lo)lo=MI32(node,N_LO+4*a);MI32(node,N_LO+4*a)=lo;int hi=MI32(c,N_HI+4*a);if(MI32(node,N_HI+4*a)<hi)hi=MI32(node,N_HI+4*a);MI32(node,N_HI+4*a)=hi;}
  }
 }
}

inline int Assembler::Emit(u8* node,u8* qp,u8* qc){
 if(MU8(node,N_DONE))return 3;
 if(!(MI32(qc,Q_LEVEL)>depthLimit)&&!MU8(qc,Q_FLAG)&&MI32(qc,Q_LEVEL)%step==0){
  MU8(qc,Q_FLAG)=1;int saved=depthLimit;int eax=-1;
  while(!MU8(node,N_DONE)){
   u32 before=writer->count;u8 qt[Q_SIZE];memset(qt,0,sizeof(qt));memcpy(qt,qc,Q_COPY);
   eax=Emit(node,qp,qt);
   if(counter>limit)break;
   if(before==writer->count){if(depthLimit<0)break;depthLimit-=step;}
  }
  depthLimit=saved;MU8(qc,Q_FLAG)=0;return eax;
 }
 if(MU8(node,N_TERMINAL)){
  MarkUp(node);
  if(!(MI32(node,N_COUNT)<0x16)&&!(MI32(qc,Q_LEVEL)>depthLimit))return -1;
  counter++;u32 start=writer->count;TerminalCommand(node,qp,qc);if(BoundsWatch())BoundsWatch()(node,qc);Bounds(node,qc);MI32(node,N_POS)=(int)writer->count;MU8(node,N_DONE)=1;return (int)(writer->count-start);
 }
 Rescale tmp;tmp.flag=0;
 if(MU8(node,N_RESCALE)==1)DoRescale(qp,qc,tmp);
 KeyOffset(node,qp,qc);
 int axis=AxisCode((const Plane*)MPTR(node,N_PLANE));
 int v24,v28;SplitValues(node,qc,&v24,&v28);
 int pos1=-1,r1=-1,pos0;
 u8* c=MPTR(node,N_CHILD1);
 if(c){u8 qt[Q_SIZE];memset(qt,0,sizeof(qt));Child(qt,qc,c);r1=Emit(c,qc,qt);if(!(r1<0)){pos1=MI32(c,N_POS);ReleaseChildren(c);}}
 c=MPTR(node,N_CHILD0);if(!c)return -1;
 u8 qt[Q_SIZE];memset(qt,0,sizeof(qt));Child(qt,qc,c);int r0=Emit(c,qc,qt);if(r0<0)return -1;
 pos0=MI32(c,N_POS);ReleaseChildren(c);
 if(r1<0)return -1;
 if(!Ready(node,qc))return -1;
 MarkUp(node);counter++;
 u32 start=writer->count;
 SplitCommand(v28,v24,axis,(u32)pos0,(u32)pos1);
 int diff=MI32(qc,Q_KEYOFF)-MI32(qp,Q_KEYOFF);if(diff)KeyOffsetCommand((u32)diff);
 SubtreeProperty(node,qp,qc);
 Bounds(node,qc);
 if(tmp.flag)RescaleCommand(tmp);
 MI32(node,N_POS)=(int)writer->count;MU8(node,N_DONE)=1;
 return (int)(writer->count-start)+r0+r1;
}

inline void Assembler::Assemble(u8* root,Splitter* s,int lim){
 if(Observer())Observer()(root,lim);
 splitter=s;limit=lim;counter=0;ComputeInfo(root,info);if(!root)return;
 depthLimit=BitLength(MU32(root,N_KEYMAX));halfBits=depthLimit>>1;depthLimit=((depthLimit/step)+2)*step-1;
 u8 q0[Q_SIZE],q1[Q_SIZE],q2[Q_SIZE];memset(q0,0,sizeof(q0));memset(q1,0,sizeof(q1));memset(q2,0,sizeof(q2));
 Quantize(q0,root,info);MI32(q0,Q_SHIFT)=16;MI32(q0,Q_OFF+8)=0;MI32(q0,Q_OFF+4)=0;MI32(q0,Q_OFF)=0;MI32(q0,Q_LEVEL)=-1;MI32(q0,Q_KEYOFF)=0;MU8(q0,Q_FLAG)=0;Keys(q0,root);NeededBits(q0);
 memcpy(q1,q0,Q_COPY);Keys(q1,root);Quantize(q1,root,info);MI32(q1,Q_LEVEL)=MI32(q0,Q_LEVEL)+1;MU8(q1,Q_FLAG)=0;NeededBits(q1);
 Precision(root,q0,q1);
 memset(q1+Q_COPY,0,Q_SIZE-Q_COPY);
 Quantize(q1,root,info);MI32(q1,Q_SHIFT)=16;MI32(q1,Q_OFF+8)=0;MI32(q1,Q_OFF+4)=0;MI32(q1,Q_OFF)=0;MI32(q1,Q_LEVEL)=-1;MI32(q1,Q_KEYOFF)=0;MU8(q1,Q_FLAG)=0;Keys(q1,root);NeededBits(q1);
 memcpy(q2,q1,Q_COPY);Keys(q2,root);Quantize(q2,root,info);MI32(q2,Q_LEVEL)=MI32(q1,Q_LEVEL)+1;MU8(q2,Q_FLAG)=0;NeededBits(q2);
 Ranges(root,q1,q2);
 memcpy(q1,q0,Q_COPY);Keys(q1,root);Quantize(q1,root,info);MI32(q1,Q_LEVEL)=MI32(q0,Q_LEVEL)+1;MU8(q1,Q_FLAG)=0;NeededBits(q1);
 Emit(root,q0,q1);
}

// ---- splitter (vtable 0x10091A48) ----
inline void Splitter::Keys(int* range,u8* node){ // 0x10070790
 u8* p=(u8*)range[0];u32 mn=MU32(p,0),mx=mn;MI32(node,N_NUMPROPS)=0;MI32(node,N_PROPS+4)=0;MI32(node,N_PROPS)=-1;
 for(int i=0;i<range[1];i++,p+=16){
  u32 key=MU32(p,0);if(key>mx)mx=key;if(key<mn)mn=key;
  u32 prop=MU32(p,4);if(1>MI32(node,N_NUMPROPS))MI32(node,N_NUMPROPS)=1;if(prop<MU32(node,N_PROPS))MU32(node,N_PROPS)=prop;if(prop>MU32(node,N_PROPS+4))MU32(node,N_PROPS+4)=prop;
 }
 MU32(node,N_KEYMIN)=mn;MI32(node,N_COUNT)=range[1];MU32(node,N_KEYMAX)=mx;
}
inline u8* Splitter::Terminal(u8* parent,int* range){ // 0x10070D70
 u8* node=freeTerms;freeTerms=MPTR(node,0);freeTermCount--;memset(node+0x38,0,0x80);
 MPTR(node,N_PARENT)=parent;MU8(node,N_TERMINAL)=1;MPTR(node,N_PLANE)=(u8*)range[0];Keys(range,node);
 for(int d=0;d<3;d++)mediator->Project(planes[d].n,(u8*)range[0],range[1],(float*)(node+N_EXT+8*d),(float*)(node+N_EXT+4+8*d),false);
 return node;
}
inline void Splitter::AxisOrder(const u8* parent,int* order,float* cost){ // 0x10070850
 if(!parent){order[0]=0;order[1]=1;order[2]=2;cost[0]=cost[1]=cost[2]=0;return;}
 const u8* sp=MPTR(parent,N_PLANE);const float k=FromBits(0x3F28F5C3);float e[3];double best=0.0;int idx=0;
 e[0]=F((double)MF32(parent,0x10)-MF32(parent,0x0C));if(sp==(const u8*)&planes[0])e[0]=F((double)e[0]*k);if(e[0]>best)best=e[0];
 e[1]=F((double)MF32(parent,0x18)-MF32(parent,0x14));if(sp==(const u8*)&planes[1])e[1]=F((double)e[1]*k);if(e[1]>best){best=e[1];idx=1;}
 e[2]=F((double)MF32(parent,0x20)-MF32(parent,0x1C));if(sp==(const u8*)&planes[2])e[2]=F((double)e[2]*k);if(e[2]>best)idx=2;
 if(idx==2)best=e[2];
 int i1=(idx+1)%3,i2=(idx+2)%3;order[0]=idx;cost[0]=0;double inv=1.0/best;
 const double s16=FromBits(0x41800000),s5=FromBits(0x3D4CCCCD);
 int first=i1,second=i2;if(e[i1]<e[i2]){first=i2;second=i1;}
 double t=(best-e[first])*inv;cost[1]=F((((t*t)*t)*s16)*s5);order[1]=first;
 t=(best-e[second])*inv;cost[2]=F((((t*t)*t)*s16)*s5);order[2]=second;
}
inline void Splitter::Candidates(int i,u8* node,int* best,u8* sc,int* range){ // 0x100705F0
 float c0=F(cf->Base(sc));int half=window>>1;int lo=0;float bestLocal=FromBits(0x4B189680);int ebx=0;bool evaluate=false;
 if(list->count==0){
  if(!list->last)goto end;
  MU32(sc,0x38)=MU32(list->last,12);ebx=0;evaluate=true;
 }else{
  if(c0>MF32(node,N_BEST))goto end;
  if(windowEnable>0){int s=*best;ebx=s+half;if(ebx>list->count)ebx=list->count;lo=s-half;if(lo<0)lo=0;}else ebx=list->count;
 }
 for(;;){
  if(!evaluate){
   ebx--;if(ebx<lo){(*best)--;break;}
   u8* entry=ebx<=list->count?list->data[ebx]:0;MU32(sc,0x38)=MU32(entry,12);
  }
  evaluate=false;
  MI32(sc,0x30)=ebx;MI32(sc,0x34)=range[1]-i+ebx;
  double c=cf->Split(sc)+(double)c0;
  if(c<bestLocal){
   bestLocal=F(c);*best=ebx;
   if(c<MF32(node,N_BEST)){
    MF32(node,N_BEST)=F(c);MPTR(node,N_PLANE)=MPTR(sc,0x24);MU32(node,N_SPLIT)=MU32(sc,0x20);MU32(node,N_END)=MU32(sc,0x38);
    int e=ebx-half;if(e<lo){lo=e;if(e<0)lo=0;}
    cf->Detail(sc,node+N_DETAIL);
   }
  }
 }
end:
 list->Insert((u8*)range[0]+16*i);
}
inline void Splitter::Evaluate(u8* node,Plane* plane,int* range,float* ext,int depth){ // 0x10070A00
 u8 sc[0x3C];memset(sc,0,sizeof(sc));
 MI32(sc,0)=depth;MI32(sc,4)=bitlen;MF32(sc,8)=ext[0];MF32(sc,0x0C)=ext[1];
 int count=range[1];u8* base=(u8*)range[0];int limit=range[2];
 MF32(sc,0x1C)=F(1.0/(double)count);MI32(sc,0x18)=count;MPTR(sc,0x24)=(u8*)plane;
 MU32(sc,0x28)=MU32(base,0);MU32(sc,0x2C)=MU32(base,0);
 double width=(double)ext[1]-ext[0];if(width<(double)FromBits(0x3A83126F))MF32(sc,0x10)=1.0f;else MF32(sc,0x10)=F(1.0/width);
 if(limit>listLimit)limit=listLimit;
 int bestIdx=0;
 for(int i=0;i<count;i++){
  u8* p=base+16*i;MI32(sc,0x14)=i;MU32(sc,0x20)=MU32(p,8);u32 key=MU32(p,0);
  if(!(MU32(sc,0x28)<key))MU32(sc,0x28)=key;if(!(MU32(sc,0x2C)>key))MU32(sc,0x2C)=key;
  list->RemoveBelow(MF32(p,8));
  for(int j=list->count-1;j>limit;j--){list->last=list->data[j];list->data[j]=0;list->count--;}
  Candidates(i,node,&bestIdx,sc,range);
 }
 for(int j=list->count-1;j>0;j--)list->data[j]=0;list->last=0;list->count=0;
}
inline void Splitter::Classify(int kind,u8* node,int depth,int* extra,int* dup,u8** cur,u8** end,u8** cap,u8** cap2){ // 0x10070B70
 (void)node;(void)depth;u8 t[16],t2[16];
 switch(kind){
 case 1:*cur+=16;return;
 case 2:*cap-=16;*end-=16;if(*cur==*cap)return;memcpy(t,*end,16);memcpy(*cap,*cur,16);memcpy(*cur,t,16);return;
 case 3:(*extra)--;(*dup)++;*cap-=16;memcpy(*cap,*cur,16);*cur+=16;return;
 case 0:*cap-=16;*cap2-=16;*end-=16;memcpy(t,*cur,16);memcpy(t2,*end,16);memcpy(*cap,*cap2,16);memcpy(*cap2,t,16);if(*cur!=*end)memcpy(*cur,t2,16);return;
 }
}
inline void Splitter::Distribute(u8** startA,u8** endA,u8** startB,u8** endB,u8** either,u8** eitherEnd,u8* node){ // 0x10070FC0
 const Plane* sp=(const Plane*)MPTR(node,N_PLANE);int idx=sp->index,axis=0;double best=0.0;
 if(idx!=0){float d=F((double)MF32(node,0x10)-MF32(node,0x0C));if(d>best)best=d;}
 if(idx!=1){float d=F((double)MF32(node,0x18)-MF32(node,0x14));if(d>best){best=d;axis=1;}}
 if(idx!=2){float d=F((double)MF32(node,0x20)-MF32(node,0x1C));if(d>best)axis=2;}
 u8* s=*either;int n=(int)((*eitherEnd-s)>>4);float mn,mx;mediator->Project(planes[axis].n,s,n,&mn,&mx,true);
 if(n>1)SortPrimitives(s,0,n-1);
 bool flag=((int)(*endA-*startA)&~15)<((int)(*endB-*startB)&~15);
 while(*either<*eitherEnd){
  int nA=(int)(*endA-*startA)>>4,nB=(int)(*endB-*startB)>>4;int lhs=nA,rhs=nB;if(flag)rhs=nB<<2;else lhs=nA<<2;
  if(lhs>=rhs){*endB+=16;*either+=16;flag=false;}
  else{u8* last=*eitherEnd-16;u8 t1[16],t2[16];memcpy(t1,last,16);memcpy(last,*either,16);memcpy(t2,*startB,16);memcpy(*endA,t1,16);memcpy(*either,t2,16);*endA+=16;*startB+=16;*either+=16;*endB+=16;flag=true;}
 }
}
inline void Splitter::Partition(int* range,u8* node,int depth,int* outA,int* outB){ // 0x100711C0
 u8* base=(u8*)range[0];int count=range[1],extra=range[2];
 u8* cur=base;u8* end=base+16*count;u8* cap=base+16*(count+extra);u8* cap2=cap;u8* capEnd=cap;int dup=0;
 float split=MF32(node,N_SPLIT),endc=MF32(node,N_END);
 while(cur<end){
  float mn=MF32(cur,8),mx=MF32(cur,12);int kind;
  if(mn<split)kind=(mx<=endc)?1:3;else kind=(mx<=endc)?0:2;
  Classify(kind,node,depth,&extra,&dup,&cur,&end,&cap,&cap2);
 }
 u8* startA=base;u8* endA=end;u8* startB=cap;u8* endB=cap2;u8* either=cap2;
 Distribute(&startA,&endA,&startB,&endB,&either,&capEnd,node);
 int cntA=(int)(endA-startA)>>4;outA[1]=cntA;outA[2]=TruncBits(F(((double)cntA*extra)/(double)count));
 outB[1]=(int)(endB-startB)>>4;outB[2]=extra-outA[2];outA[0]=(int)startA;
 u8* dst=endA+16*outA[2];outB[0]=(int)dst;
 if(outB[2]!=0)for(u8* p=startB;p<endB;p+=16,dst+=16)memcpy(dst,p,16);
}
inline u8* Splitter::BuildNode(u8* parent,int* range,int side,int depth){ // 0x10071350
 if(range[1]==1){if(freeTermCount==0)assembler->Assemble(root,this,0x800);return Terminal(parent,range);}
 if(freeNodeCount==0)assembler->Assemble(root,this,0x800);
 u8* node=freeNodes;freeNodes=MPTR(node,0);freeNodeCount--;memset(node+0x38,0,0x80);
 MU8(node,N_TERMINAL)=0;MU32(node,N_BEST)=0x49742400u;MU32(node,N_SPLIT)=0x501502F9u;MPTR(node,N_CHILD0)=0;MPTR(node,N_CHILD1)=0;MI32(node,N_NUMPRIMS)=range[1];MPTR(node,N_PARENT)=parent;
 if(!parent)root=node;else{if(side==0)MPTR(parent,N_CHILD0)=node;else MPTR(parent,N_CHILD1)=node;memcpy(node+N_EXT,parent+N_EXT,24);}
 int order[3];float cost[3];AxisOrder(parent,order,cost);
 u8* base=(u8*)range[0];int count=range[1];u8* lastSorted=0;int i=0;
 for(;i<3;i++){
  int d=order[i];Plane* p=&planes[d];if((double)cost[i]+p->cost>MF32(node,N_BEST))break;
  float* ext=(float*)(node+N_EXT+8*d);mediator->Project(p->n,base,count,&ext[0],&ext[1],true);if(count>1)SortPrimitives(base,0,count-1);
  Evaluate(node,p,range,ext,depth);lastSorted=(u8*)p;
 }
 for(;i<3;i++){int d=order[i];float* ext=(float*)(node+N_EXT+8*d);mediator->Project(planes[d].n,base,count,&ext[0],&ext[1],false);}
 for(int k=3;k<numPlanes;k++){
  Plane* p=&planes[k];if((double)p->cost>MF32(node,N_BEST))break;
  float local[2];mediator->Project(p->n,base,count,&local[0],&local[1],true);if(count>1)SortPrimitives(base,0,count-1);
  Evaluate(node,p,range,local,depth);lastSorted=(u8*)p;
 }
 Keys(range,node);
 if(MPTR(node,N_PLANE)!=lastSorted){float a,b;mediator->Project(((Plane*)MPTR(node,N_PLANE))->n,base,count,&a,&b,true);}
 int A[3],B[3];Partition(range,node,depth,A,B);
 MPTR(node,N_CHILD1)=BuildNode(node,B,1,depth+1);
 MPTR(node,N_CHILD0)=BuildNode(node,A,0,depth+1);
 return node;
}
inline u8* Splitter::BuildTree(u8* buffer,u8* nodePool,u8* termPool){ // 0x10071660
 bitlen=0;for(int n=mediator->numPrimitives;n!=0;n>>=1)bitlen++;
 freeNodes=0;freeNodeCount=0;u8* p=nodePool;
 for(int k=0;k<0x400;k++,p+=0x3E0){MPTR(p,0)=freeNodes;freeNodeCount++;MPTR(p+0xF8,0)=p;freeNodeCount++;MPTR(p+0x1F0,0)=p+0xF8;freeNodeCount++;MPTR(p+0x2E8,0)=p+0x1F0;freeNodes=p+0x2E8;freeNodeCount++;}
 freeTerms=0;freeTermCount=0;p=termPool;
 for(int k=0;k<0x400;k++,p+=0x2F0){MPTR(p,0)=freeTerms;freeTermCount++;MPTR(p+0xBC,0)=p;freeTermCount++;MPTR(p+0x178,0)=p+0xBC;freeTermCount++;MPTR(p+0x234,0)=p+0x178;freeTerms=p+0x234;freeTermCount++;}
 numPlanes=13;planes=assembler->planes;
 int capacity=listLimit+2;u8** data=new u8*[capacity];memset(data,0,capacity*sizeof(u8*));ActiveList active={0,capacity,data,0};list=&active;
 int range[3]={(int)buffer,mediator->numPrimitives,maxExtra};
 u8* result=BuildNode(0,range,0,0);
 list=0;delete[] data;return result;
}

// hkMoppUtility::buildCode (0x1006B670) with hkMoppCompiler::compile (0x1006CBB0).
struct Result {float info[4];u8* code;u32 size;const char* error;Result():code(0),size(0),error(0){memset(info,0,sizeof(info));}~Result(){delete[] code;}};
inline bool Compile(const Mesh& mesh,Result& out){
 if(!mesh.vertices||(!mesh.indices&&!mesh.indices32)||!mesh.triangles||mesh.triangles>(1u<<20)){out.error="Unsupported MOPP mesh";return false;}
 Mediator mediator;mediator.Init(mesh);int n=mediator.numPrimitives;
 // compiler params at +4: {0.2f, 1000 spare primitive slots, 50, 5, 5}; the spare
 // slots follow the primitives in the work buffer, the node pools follow them.
 const int spare=1000;
 Writer writer;writer.Init((u32)(n+spare)*12);
 CostFunction cf;cf.w0=cf.w1=cf.w2=cf.w3=cf.w4=1.0f;cf.half=0.5f;
 Assembler* assembler=new Assembler;assembler->Init(&mediator,&writer);
 u32 bytes=(u32)(n+spare+0x1B400)<<4;u8* buffer=new u8[bytes];memset(buffer,0,bytes);
 mediator.Fill(buffer);
 Splitter splitter;memset(&splitter,0,sizeof(splitter));splitter.p0=FromBits(0x3E4CCCCD);splitter.maxExtra=1000;splitter.listLimit=50;splitter.window=5;splitter.windowEnable=5;
 splitter.cf=&cf;splitter.assembler=assembler;splitter.mediator=&mediator;
 u8* root=splitter.BuildTree(buffer,buffer+16*(n+spare),buffer+16*(n+spare)+0xF8000);
 assembler->Assemble(root,&splitter,0x2000);
 for(int i=0;i<0x30;i++)writer.Put(0);
 u32 total=writer.count;out.size=total-0x30;out.code=new u8[out.size?out.size:1];memcpy(out.code,writer.buffer+writer.capacity-total+0x30,out.size);
 assembler->ComputeInfo(root,out.info);
 delete[] buffer;writer.Release();delete assembler;return true;
}
}
