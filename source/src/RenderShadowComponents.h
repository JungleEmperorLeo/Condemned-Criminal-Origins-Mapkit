#pragma once
// FEAR shadow topology before shadow corner generation. Caller excludes native
// shader triangles; category is the surface ShadowLOD (0, 1 or 2). The optional
// category filter is one complete original 0x4A450 pass, in source order.
namespace RenderShadowComponents {
struct Triangle {WorldText::Vec3 p[3],normal;unsigned int category;};
struct Result {unsigned int point[3];int adjacent[3];unsigned int closed,oneSided;};
static_assert(sizeof(Triangle)==52,"shadow component input");
static_assert(sizeof(Result)==32,"shadow component output");
inline float Difference(float a,float b){float value;__asm {
 fld a
 fsub b
 fstp value
 }return value;}
inline float CrossTerm(float a,float b,float c,float d){float value;__asm {
 fld a
 fmul b
 fld c
 fmul d
 fsubp st(1),st
 fstp value
 }return value;}
inline bool Near(const WorldText::Vec3& a,const WorldText::Vec3& b){
 float x=Difference(a.x,b.x),y=Difference(a.y,b.y),z=Difference(a.z,b.z),limit=.0001f;unsigned int status;
 __asm {
 fld x
 fmul x
 fld y
 fmul y
 faddp st(1),st
 fld z
 fmul z
 faddp st(1),st
 fcomp limit
 fnstsw ax
 movzx eax,ax
 mov status,eax
 }
 return (status&0x4100)!=0;
}
inline int RoundedCell(float value){
 unsigned short before,temporary;int result;float half=.5f;
 __asm {fnstcw before}
 temporary=(unsigned short)((before&0xf3ff)|0x0c00);
 __asm {
 fldcw temporary
 fld value
 fadd half
 fistp result
 fldcw before
 }
 return result;
}
inline unsigned int Bucket(const WorldText::Vec3& p){return ((unsigned int)RoundedCell(p.x)-108u*(unsigned int)RoundedCell(p.y)-19u*(unsigned int)RoundedCell(p.z))&255;}
inline bool Outward(const Triangle* input,const unsigned int* source,const unsigned int* component,unsigned int count){
 float maximum[3]={-3.402823466e38f,-3.402823466e38f,-3.402823466e38f};bool positive[3]={true,true,true};
 // Classification examines ascending selected triangle ordinals, independently
 // of the traversal order used to discover the component.
 for(unsigned int i=0;i<count;i++)if(component[i]){
  const Triangle& t=input[source[i]];WorldText::Vec3 a={Difference(t.p[1].x,t.p[0].x),Difference(t.p[1].y,t.p[0].y),Difference(t.p[1].z,t.p[0].z)},b={Difference(t.p[2].x,t.p[0].x),Difference(t.p[2].y,t.p[0].y),Difference(t.p[2].z,t.p[0].z)};
  float cross[3]={CrossTerm(a.y,b.z,a.z,b.y),CrossTerm(a.z,b.x,a.x,b.z),CrossTerm(a.x,b.y,a.y,b.x)};
  float bound[3]={t.p[0].x,t.p[0].y,t.p[0].z};for(unsigned int j=1;j<3;j++){const float* p=(const float*)&t.p[j];for(unsigned int k=0;k<3;k++)if(bound[k]<=p[k])bound[k]=p[k];}
  for(unsigned int k=0;k<3;k++){float absolute=cross[k]<0?-cross[k]:cross[k];if(bound[k]>maximum[k]&&absolute>.001f){maximum[k]=bound[k];positive[k]=cross[k]>0;}}
 }
 return positive[0]&&positive[1]&&positive[2];
}
class Mesh {
 PrefabScene::Arena memory;
 struct Edge {unsigned int a,b,triangle,used;};
 static unsigned int EdgeHash(unsigned int a,unsigned int b){return (a*0x9e3779b9u)^(b*0x85ebca6bu);}
 static Edge* Find(Edge* edges,unsigned int mask,unsigned int a,unsigned int b){unsigned int slot=EdgeHash(a,b)&mask;while(edges[slot].used&&(edges[slot].a!=a||edges[slot].b!=b))slot=(slot+1)&mask;return edges+slot;}
 bool Fail(const char* message){memory.Clear();results=0;source=0;points=0;count=pointCount=0;error=message;return false;}
 Mesh(const Mesh&);Mesh& operator=(const Mesh&);
public:
 Result* results;unsigned int* source;WorldText::Vec3* points;unsigned int count,pointCount;const char* error;
 Mesh():results(0),source(0),points(0),count(0),pointCount(0),error(0){}
 bool Build(const Triangle* input,unsigned int size,unsigned int category=0xffffffffu){
  Fail(0);if(!input||!size||size>100000)return Fail("Unsupported shadow input count");
  for(unsigned int i=0;i<size;i++){
   for(unsigned int k=0;k<3;k++){const WorldText::Vec3& p=input[i].p[k];if(!WorldText::FiniteVector(p)||p.x>10000000.f||p.x< -10000000.f||p.y>10000000.f||p.y< -10000000.f||p.z>10000000.f||p.z< -10000000.f)return Fail("Unsupported shadow position");}
   if(category==0xffffffffu||input[i].category==category)count++;
  }
  if(!count)return true;
  results=(Result*)memory.Allocate(count*sizeof(Result));source=(unsigned int*)memory.Allocate(count*4);points=(WorldText::Vec3*)memory.Allocate(count*36);unsigned int* next=(unsigned int*)memory.Allocate(count*12);
  unsigned int edgeSize=8;while(edgeSize<count*6)edgeSize*=2;Edge* edges=(Edge*)memory.Allocate(edgeSize*sizeof(Edge));unsigned int* queue=(unsigned int*)memory.Allocate(count*4),*seen=(unsigned int*)memory.Allocate(count*4),*component=(unsigned int*)memory.Allocate(count*4);
  if(!results||!source||!points||!next||!edges||!queue||!seen||!component)return Fail("Shadow topology allocation failed");memset(edges,0,edgeSize*sizeof(Edge));memset(seen,0,count*4);
  unsigned int ordinal=0,heads[256],tails[256];for(unsigned int b=0;b<256;b++)heads[b]=tails[b]=0xffffffffu;
  for(unsigned int i=0;i<size;i++)if(category==0xffffffffu||input[i].category==category){
   source[ordinal]=i;Result& result=results[ordinal++];result.closed=result.oneSided=0;
   for(int k=2;k>=0;k--){const WorldText::Vec3& p=input[i].p[k];unsigned int b=Bucket(p),found=heads[b];while(found!=0xffffffffu&&!Near(p,points[found]))found=next[found];if(found==0xffffffffu){found=pointCount++;points[found]=p;next[found]=0xffffffffu;if(heads[b]==0xffffffffu)heads[b]=found;else next[tails[b]]=found;tails[b]=found;}result.point[k]=found;}
  }
  for(unsigned int i=0;i<count;i++)for(unsigned int k=0;k<3;k++){Edge* edge=Find(edges,edgeSize-1,results[i].point[k],results[i].point[(k+1)%3]);if(edge->used)results[i].adjacent[k]=-1;else{edge->a=results[i].point[k];edge->b=results[i].point[(k+1)%3];edge->triangle=i;edge->used=1;results[i].adjacent[k]=0;}}
  for(unsigned int i=0;i<count;i++)for(unsigned int k=0;k<3;k++)if(results[i].adjacent[k]!=-1){Edge* edge=Find(edges,edgeSize-1,results[i].point[(k+1)%3],results[i].point[k]);results[i].adjacent[k]=edge->used?(int)edge->triangle:-1;}
  // Original reciprocal cleanup mutates adjacency in ascending triangle order.
  for(unsigned int i=0;i<count;i++)for(unsigned int k=0;k<3;k++){int adjacent=results[i].adjacent[k];if(adjacent!=-1&&results[adjacent].adjacent[0]!=(int)i&&results[adjacent].adjacent[1]!=(int)i&&results[adjacent].adjacent[2]!=(int)i)results[i].adjacent[k]=-1;}
  for(unsigned int start=0;start<count;start++)if(!seen[start]){
   memset(component,0,count*4);unsigned int head=0,tail=1;queue[0]=start;component[start]=seen[start]=1;bool closed=true;
   while(head<tail){unsigned int i=queue[head++];for(unsigned int k=0;k<3;k++){int adjacent=results[i].adjacent[k];if(adjacent==-1)closed=false;else if(!component[adjacent]){component[adjacent]=seen[adjacent]=1;queue[tail++]=(unsigned int)adjacent;}}}
   bool oneSided=closed&&Outward(input,source,component,count);for(unsigned int i=0;i<tail;i++){results[queue[i]].closed=closed?1:0;results[queue[i]].oneSided=oneSided?1:0;}
  }
  return true;
 }
};
}
