#pragma once
#include "RenderAttributes.h"
#include "RenderExceptionalUV.h"
#include "RenderIndexedLimits.h"
// Indexed-vertex insertion core. Caller supplies already-final working corners
// and explicit section boundaries; no section selection or triangle reordering.
namespace RenderVertexWeld {
struct Input {unsigned int reset;RenderAttributes::Vertex vertex;};
static_assert(sizeof(Input)==88,"weld input stride");
inline bool Near3(const WorldText::Vec3& a,const WorldText::Vec3& b,float epsilon){
 float x=(float)((double)a.x-b.x),y=(float)((double)a.y-b.y),z=(float)((double)a.z-b.z);unsigned int status;
 __asm {
  fld x
  fmul x
  fld y
  fmul y
  faddp st(1),st
  fld z
  fmul z
  faddp st(1),st
  fld epsilon
  fmul epsilon
  fcompp
  fnstsw ax
  movzx eax,ax
  mov status,eax
 }
 return !(status&0x100);
}
inline bool Near2(const float* a,const float* b,float epsilon){
 float x=(float)((double)a[0]-b[0]),y=(float)((double)a[1]-b[1]);unsigned int status;
 __asm {
  fld x
  fmul x
  fld y
  fmul y
  faddp st(1),st
  fld epsilon
  fmul epsilon
  fcompp
  fnstsw ax
  movzx eax,ax
  mov status,eax
 }
 return !(status&0x100);
}
inline bool Equal(const RenderAttributes::Vertex& a,const RenderAttributes::Vertex& b){
 if(!Near3(a.position,b.position,.1f)||a.color!=b.color||!Near3(a.normal,b.normal,.01f)||!Near3(a.tangent,b.tangent,.01f)||!Near3(a.binormal,b.binormal,.01f))return false;
 for(unsigned int i=0;i<4;i++)if(!Near2(a.uv+i*2,b.uv+i*2,.002f))return false;return true;
}
inline int Truncate(double value){
 unsigned short before,temporary;int result;
 __asm {fnstcw before}
 temporary=(unsigned short)((before&0xf3ff)|0x0c00);
 __asm {
  fldcw temporary
  fld value
  fistp result
  fldcw before
 }
 return result;
}
inline bool Valid(const RenderAttributes::Vertex& v,bool allowExceptionalUV=false){
 if(!WorldText::FiniteVector(v.position)||!WorldText::FiniteVector(v.normal)||!WorldText::FiniteVector(v.tangent)||!WorldText::FiniteVector(v.binormal))return false;
 if(v.position.x>10000000.f||v.position.x< -10000000.f||v.position.y>10000000.f||v.position.y< -10000000.f||v.position.z>10000000.f||v.position.z< -10000000.f)return false;
 for(unsigned int i=0;i<8;i++)if(!RenderExceptionalUV::Allowed(v.uv[i],allowExceptionalUV))return false;return true;
}
inline unsigned int Bucket(const WorldText::Vec3& p,const int* origin){
 unsigned int x=(unsigned int)Truncate((double)p.x+.5)-(unsigned int)origin[0];
 unsigned int y=(unsigned int)Truncate((double)p.y+.5)-(unsigned int)origin[1];
 unsigned int z=(unsigned int)Truncate((double)p.z+.5)-(unsigned int)origin[2];
 return (x-108u*y-19u*z)&255;
}
class Mesh {
 PrefabScene::Arena memory;
 bool Fail(const char* reason){memory.Clear();vertices=0;indices=0;count=inputs=resets=0;error=reason;return false;}
 Mesh(const Mesh&);Mesh& operator=(const Mesh&);
public:
 RenderAttributes::Vertex* vertices;unsigned int* indices;unsigned int count,inputs,resets;const char* error;
 Mesh():vertices(0),indices(0),count(0),inputs(0),resets(0),error(0){}
 bool Build(const Input* input,unsigned int size,const int* origin,bool allowExceptionalUV=false){
  Fail(0);if(!input||!origin||!size||size>RenderIndexedLimits::MaximumCorners||input[0].reset!=1)return Fail("Invalid indexed corner stream");
  vertices=(RenderAttributes::Vertex*)memory.Allocate(size*84);indices=(unsigned int*)memory.Allocate(size*4);unsigned int* next=(unsigned int*)memory.Allocate(size*4);
  if(!vertices||!indices||!next)return Fail("Indexed corner allocation failed");unsigned int heads[256],tails[256];
  for(unsigned int i=0;i<size;i++){
   if(input[i].reset>1||!Valid(input[i].vertex,allowExceptionalUV))return Fail("Unsupported indexed corner");
   if(input[i].reset){for(unsigned int b=0;b<256;b++)heads[b]=tails[b]=0xffffffff;resets++;}
   unsigned int b=Bucket(input[i].vertex.position,origin),found=heads[b];
   while(found!=0xffffffff&&!Equal(input[i].vertex,vertices[found]))found=next[found];
   if(found==0xffffffff){found=count++;vertices[found]=input[i].vertex;next[found]=0xffffffff;if(heads[b]==0xffffffff)heads[b]=found;else next[tails[b]]=found;tails[b]=found;}
   indices[i]=found;
  }
  inputs=size;return true;
 }
};
}
