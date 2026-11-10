#pragma once
#include "RenderAttributes.h"
namespace RenderShadowEdges {
// Original working helpers 0x49B50 (connected) and 0x49DE0 (boundary).
// Caller supplies the edge classification. No connectivity or component
// orientation policy is inferred here. Working records retain degeneracy.
struct Input {
 WorldText::Vec3 own[3],neighbor[3],ownNormal,neighborNormal;
 unsigned int kind,start,end,twoSided; // kind: 0 connected, 1 boundary
};
struct Triangle {RenderAttributes::Vertex vertices[3];};
struct Output {unsigned int count;Triangle triangles[2];};
static_assert(sizeof(Input)==112,"shadow edge input");
static_assert(sizeof(Triangle)==252,"shadow working corner payload");
inline bool Match(const WorldText::Vec3& a,const WorldText::Vec3& b){
 // 0x127F0 stores each subtraction to float, then compares the extended
 // precision sum to the square of the float 0.01 threshold, inclusively.
 float x=a.x-b.x,y=a.y-b.y,z=a.z-b.z,epsilon=.01f;unsigned short state;
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
  fnstsw state
 }
 return !(state&0x100);
}
inline int Find(const WorldText::Vec3* points,const WorldText::Vec3& position){for(int i=0;i<3;i++)if(Match(position,points[i]))return i;return -1;}
inline WorldText::Vec3 Flip(const WorldText::Vec3& n){WorldText::Vec3 r={-n.x,-n.y,-n.z};return r;}
inline void Corner(RenderAttributes::Vertex& v,const WorldText::Vec3& p,const WorldText::Vec3& n){v.position=p;v.normal=n;}
inline bool Build(const Input& input,Output& result){
 memset(&result,0,sizeof(result));if(input.kind>1||input.start>2||input.end>2||input.twoSided>1)return false;
 for(unsigned int i=0;i<3;i++)if(!WorldText::FiniteVector(input.own[i])||!WorldText::FiniteVector(input.neighbor[i]))return false;
 if(!WorldText::FiniteVector(input.ownNormal)||!WorldText::FiniteVector(input.neighborNormal))return false;
 const WorldText::Vec3& start=input.own[input.start];const WorldText::Vec3& end=input.own[input.end];const WorldText::Vec3& normal=input.ownNormal;
 Triangle& first=result.triangles[0];
 if(input.kind==0){
  // Both original lookups must succeed. The second lookup supplies the
  // middle position; first matching neighbor corner wins independently.
  if(Find(input.neighbor,start)<0)return true;int match=Find(input.neighbor,end);if(match<0)return true;
  Corner(first.vertices[0],start,normal);Corner(first.vertices[1],input.neighbor[match],input.neighborNormal);Corner(first.vertices[2],end,normal);result.count=1;
  if(input.twoSided){Triangle& second=result.triangles[1];second.vertices[0]=first.vertices[0];second.vertices[1]=first.vertices[2];second.vertices[2]=first.vertices[1];for(unsigned int i=0;i<3;i++)second.vertices[i].normal=Flip(second.vertices[i].normal);result.count=2;}
 }else{
  WorldText::Vec3 flipped=Flip(normal);Corner(first.vertices[0],start,normal);Corner(first.vertices[1],end,flipped);Corner(first.vertices[2],end,normal);
  Triangle& second=result.triangles[1];Corner(second.vertices[0],start,normal);Corner(second.vertices[1],start,flipped);Corner(second.vertices[2],end,flipped);result.count=2;
 }
 return true;
}
}
