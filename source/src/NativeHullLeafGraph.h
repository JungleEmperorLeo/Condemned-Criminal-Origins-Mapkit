#pragma once
// Deterministic zero/one/two/three-point Hull leaf primitives.
// Geometry arithmetic preserves recovered x87 stores. No original DLL is used.
namespace NativeHullLeafGraph {
struct Vec4 {float x,y,z,w;};
struct Edge {unsigned short origin,twin,next,state;};
static_assert(sizeof(Vec4)==16,"Hull point stride");
static_assert(sizeof(Edge)==8,"Hull halfedge stride");
enum Status {Success=0,InvalidInput,TooManyActivePoints,IndexOutOfProfile,NonfiniteResult,UnsupportedArithmetic};
struct Leaf {Edge edges[6];unsigned int count;Leaf():count(0){for(unsigned int i=0;i<6;i++){edges[i].origin=edges[i].twin=edges[i].next=edges[i].state=0;}}};
inline bool Finite(float value){union B {float f;unsigned int u;};B b;b.f=value;return (b.u&0x7f800000u)!=0x7f800000u;}
inline void Set(Edge& e,unsigned int p,unsigned int twin,unsigned int next){e.origin=(unsigned short)p;e.twin=(unsigned short)twin;e.next=(unsigned short)next;e.state=0;}
inline void Point(Leaf& leaf,unsigned int p0){leaf.count=1;Set(leaf.edges[0],p0,0,0);}
inline void Segment(Leaf& leaf,unsigned int p0,unsigned int p1){leaf.count=2;Set(leaf.edges[0],p0,1,1);Set(leaf.edges[1],p1,0,0);}
#if defined(_MSC_VER) && defined(_M_IX86)
inline bool Geometry(const Vec4* p0,const Vec4* p1,const Vec4* p2,float* squaredLengths){
 float scratch[28];const float zero=0,one=1,threshold=0.000001f;bool collinear=false;
 __asm {
  mov eax,p1
  mov esi,p2
  mov         edx,dword ptr [eax+8]
  fld         dword ptr [eax]
  fld         dword ptr [eax+4]
  mov         dword ptr scratch[68h],edx
  fxch        st(1)
  mov         edx,dword ptr [eax+0Ch]
  mov         dword ptr scratch[6Ch],edx
  mov edx,p0
  fsub        dword ptr [edx]
  fstp        dword ptr scratch[60h]
  mov         ebx,dword ptr [esi+4]
  fsub        dword ptr [edx+4]
  mov         dword ptr scratch[54h],ebx
  fld         dword ptr scratch[68h]
  mov         ebx,dword ptr [esi+8]
  fsub        dword ptr [edx+8]
  mov         dword ptr scratch[58h],ebx
  fld         dword ptr scratch[6Ch]
  mov         ebx,dword ptr [esi+0Ch]
  fsub        dword ptr [edx+0Ch]
  mov         dword ptr scratch[5Ch],ebx
  fld         st(1)
  fmul        st,st(2)
  fld         st(3)
  fmul        st,st(4)
  fld         dword ptr scratch[60h]
  fmul        dword ptr scratch[60h]
  fld         st(3)
  fmul        st,st(4)
  fadd        st,st(1)
  fadd        st,st(2)
  fadd        st,st(3)
  fstp        dword ptr scratch[10h]
  fld         dword ptr [esi]
  fsub        dword ptr [eax]
  fstp        dword ptr scratch[50h]
  fld         dword ptr scratch[54h]
  fsub        dword ptr [eax+4]
  fstp        dword ptr scratch[54h]
  fld         dword ptr scratch[58h]
  fsub        dword ptr [eax+8]
  fstp        dword ptr scratch[58h]
  fld         dword ptr scratch[5Ch]
  fsub        dword ptr [eax+0Ch]
  mov         eax,dword ptr [edx+4]
  fld         dword ptr scratch[58h]
  mov         dword ptr scratch[24h],eax
  fmul        dword ptr scratch[58h]
  mov         eax,dword ptr [edx+8]
  mov         dword ptr scratch[28h],eax
  fstp        dword ptr scratch[48h]
  fld         dword ptr scratch[54h]
  fmul        dword ptr scratch[54h]
  fstp        dword ptr scratch[40h]
  fld         dword ptr scratch[50h]
  fmul        dword ptr scratch[50h]
  fstp        dword ptr scratch[38h]
  fld         st(0)
  fmul        st,st(1)
  fadd        dword ptr scratch[38h]
  fadd        dword ptr scratch[40h]
  fadd        dword ptr scratch[48h]
  fstp        dword ptr scratch[18h]
  fstp        st(0)
  fld         dword ptr [edx]
  fsub        dword ptr [esi]
  mov         edx,dword ptr [edx+0Ch]
  mov         dword ptr scratch[2Ch],edx
  fstp        dword ptr scratch[20h]
  fld         dword ptr scratch[24h]
  fsub        dword ptr [esi+4]
  fstp        dword ptr scratch[24h]
  fld         dword ptr scratch[28h]
  fsub        dword ptr [esi+8]
  fstp        dword ptr scratch[28h]
  fld         dword ptr scratch[2Ch]
  fsub        dword ptr [esi+0Ch]
  fst         dword ptr scratch[2Ch]
  fmul        dword ptr scratch[2Ch]
  fld         dword ptr scratch[28h]
  fmul        dword ptr scratch[28h]
  faddp       st(1),st
  fld         dword ptr scratch[24h]
  fmul        dword ptr scratch[24h]
  faddp       st(1),st
  fld         dword ptr scratch[20h]
  fmul        dword ptr scratch[20h]
  faddp       st(1),st
  fstp        dword ptr scratch[20h]
  fadd        st,st(1)
  fadd        st,st(2)
  fstp        st(3)
  fstp        st(0)
  fstp        st(0)
  fld         dword ptr zero
  fld         st(1)
  fucompp
  fnstsw      ax
  test        ah,44h
  jnp         Leaf_6a9fa
  fsqrt
  fdivr       dword ptr one
  jmp         Leaf_6aa02
Leaf_6a9fa:
  fstp        st(0)
  fld         dword ptr zero
Leaf_6aa02:
  fld         dword ptr scratch[60h]
  fmul        st,st(1)
  fstp        dword ptr scratch[60h]
  fld         st(0)
  fmul        st,st(3)
  fstp        dword ptr scratch[64h]
  fmul        st,st(1)
  fstp        dword ptr scratch[68h]
  fstp        st(0)
  fstp        st(0)
  fld         dword ptr scratch[38h]
  fadd        dword ptr scratch[40h]
  fadd        dword ptr scratch[48h]
  fld         dword ptr zero
  fld         st(1)
  fucompp
  fnstsw      ax
  test        ah,44h
  jnp         Leaf_6aa45
  fsqrt
  fdivr       dword ptr one
  jmp         Leaf_6aa4d
Leaf_6aa45:
  fstp        st(0)
  fld         dword ptr zero
Leaf_6aa4d:
  fld         dword ptr scratch[50h]
  fmul        st,st(1)
  fld         st(1)
  fmul        dword ptr scratch[54h]
  fstp        dword ptr scratch[54h]
  fxch        st(1)
  fmul        dword ptr scratch[58h]
  fstp        dword ptr scratch[58h]
  fsub        dword ptr scratch[60h]
  fld         dword ptr scratch[54h]
  fsub        dword ptr scratch[64h]
  fld         dword ptr scratch[58h]
  fsub        dword ptr scratch[68h]
  fld         st(0)
  fmul        st,st(1)
  fld         st(2)
  fmul        st,st(3)
  faddp       st(1),st
  fld         st(3)
  fmul        st,st(4)
  faddp       st(1),st
  fcomp       dword ptr threshold
  fstp        st(0)
  fnstsw      ax
  fstp        st(0)
  test        ah,5
  fstp        st(0)
  setnp al
  mov collinear,al
 }
 squaredLengths[0]=scratch[4];squaredLengths[1]=scratch[6];squaredLengths[2]=scratch[8];return collinear;
}
#endif
// The input indexes are already active and must refer to w==0 points. Upstream
// recursion collects these in ascending range order. State is our initialized
// scratch, not a claim about uninitialized original local state words.
inline Status Build(const Vec4* points,unsigned int pointCount,const unsigned int* active,unsigned int count,Leaf* output){
 if(!output)return InvalidInput;*output=Leaf();if(count>3)return TooManyActivePoints;
 if((pointCount&&!points)||(count&&!active)||count>pointCount||pointCount>65535)return InvalidInput;
 for(unsigned int i=0;i<count;i++){
  if(active[i]>=pointCount||active[i]>65534)return IndexOutOfProfile;
  const Vec4& p=points[active[i]];if(!Finite(p.x)||!Finite(p.y)||!Finite(p.z)||!Finite(p.w)||p.w!=0)return InvalidInput;
  for(unsigned int j=0;j<i;j++)if(active[j]==active[i])return InvalidInput;
 }
 if(!count)return Success;if(count==1){Point(*output,active[0]);return Success;}
 if(count==2){Segment(*output,active[0],active[1]);return Success;}
#if defined(_MSC_VER) && defined(_M_IX86)
 float lengths[3];bool collinear=Geometry(points+active[0],points+active[1],points+active[2],lengths);
 if(!Finite(lengths[0])||!Finite(lengths[1])||!Finite(lengths[2]))return NonfiniteResult;
 if(collinear){float largest=lengths[1]>lengths[2]?lengths[1]:lengths[2];if(lengths[0]>largest)largest=lengths[0];
  if(largest==lengths[0])Segment(*output,active[0],active[1]);else if(largest==lengths[1])Segment(*output,active[1],active[2]);else Segment(*output,active[2],active[0]);return Success;
 }
 output->count=6;Set(output->edges[0],active[0],1,2);Set(output->edges[1],active[1],0,5);Set(output->edges[2],active[1],3,4);
 Set(output->edges[3],active[2],2,1);Set(output->edges[4],active[2],5,0);Set(output->edges[5],active[0],4,3);return Success;
#else
 return UnsupportedArithmetic;
#endif
}
// Bounded recursion boundary: count active points over [low,high] inclusive;
// skip every nonzero finite w and reject nonfinite data. Input itself is owned
// by the caller and is never reordered or modified by the leaf primitive.
inline Status BuildRange(const Vec4* points,unsigned int pointCount,unsigned int low,unsigned int high,Leaf* output){
 if(!output)return InvalidInput;*output=Leaf();if(!points||!pointCount||pointCount>65535||low>high||high>=pointCount)return InvalidInput;
 unsigned int active[3],count=0;
 for(unsigned int i=low;i<=high;i++){const Vec4& p=points[i];if(!Finite(p.x)||!Finite(p.y)||!Finite(p.z)||!Finite(p.w))return InvalidInput;if(p.w!=0)continue;if(count==3)return TooManyActivePoints;active[count++]=i;}
 return Build(points,pointCount,active,count,output);
}
}
