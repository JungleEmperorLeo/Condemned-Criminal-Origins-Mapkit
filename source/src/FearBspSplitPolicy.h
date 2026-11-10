#pragma once
#include "WorldText.h"
// Recovered from FEAR retail WorldPacker: 0x8500, 0x8960, 0x8FF0,
// 0x9170 and 0x92E0. This is a selector, not a complete BSP builder.
namespace FearBspSplitPolicy {
enum Side { Negative=0, Positive=1, Intersect=2 };
struct Polygon {
 WorldText::Vec3 normal;float distance;
 const WorldText::Vec3* points;unsigned int count,flags;
 WorldText::Vec3 center;float radius;
};
struct Selector {unsigned int splits,positive,negative,sideDifference,maxSide,index;float weight;};
inline unsigned int SearchCount(unsigned int count){unsigned int n=count>169?count/13:count;return n<169?n:169;}
inline unsigned int Type(unsigned int flags){return (flags&2)?0:((flags&8)?1:2);}
// The x87 operation order is y, z, x, with only the final distance stored.
__declspec(noinline) inline float Dot(const WorldText::Vec3& a,const WorldText::Vec3& b){
 float result;
#if defined(_M_IX86)
 const float* pa=&a.x;const float* pb=&b.x;
 __asm {
 mov eax,pa
 mov edx,pb
 fld dword ptr [eax+4]
 fmul dword ptr [edx+4]
 fld dword ptr [eax+8]
 fmul dword ptr [edx+8]
 faddp st(1),st
 fld dword ptr [eax]
 fmul dword ptr [edx]
 faddp st(1),st
 fstp result
 }
#else
 result=(float)(((double)a.y*b.y+(double)a.z*b.z)+(double)a.x*b.x);
#endif
 return result;
}
__declspec(noinline) inline float Distance(const Polygon& plane,const WorldText::Vec3& b){
 float result;
#if defined(_M_IX86)
 const float* pa=&plane.normal.x;const float* pb=&b.x;const float* pd=&plane.distance;
 __asm {
 mov eax,pa
 mov edx,pb
 fld dword ptr [eax+4]
 fmul dword ptr [edx+4]
 fld dword ptr [eax+8]
 fmul dword ptr [edx+8]
 faddp st(1),st
 fld dword ptr [eax]
 fmul dword ptr [edx]
 faddp st(1),st
 mov eax,pd
 fsub dword ptr [eax]
 fstp result
 }
#else
 result=(float)((((double)plane.normal.y*b.y+(double)plane.normal.z*b.z)+(double)plane.normal.x*b.x)-plane.distance);
#endif
 return result;
}
__declspec(noinline) inline Side CoplanarSide(const Polygon& plane,const Polygon& polygon){
#if defined(_M_IX86)
 const float* pa=&plane.normal.x;const float* pb=&polygon.normal.x;const float zero=0.0f;unsigned int result;
 __asm {
 mov eax,pa
 mov edx,pb
 fld dword ptr [eax+4]
 fmul dword ptr [edx+4]
 fld dword ptr [eax+8]
 fmul dword ptr [edx+8]
 faddp st(1),st
 fld dword ptr [eax]
 fmul dword ptr [edx]
 faddp st(1),st
 fcomp zero
 fnstsw ax
 and eax,100h
 shr eax,8
 xor eax,1
 mov result,eax
 }
 return result?Positive:Negative;
#else
 return (((double)plane.normal.y*polygon.normal.y+(double)plane.normal.z*polygon.normal.z)+(double)plane.normal.x*polygon.normal.x)>=0?Positive:Negative;
#endif
}
inline Side QuickSide(const Polygon& plane,const Polygon& polygon){
 float distance=Distance(plane,polygon.center);
 if(distance>polygon.radius)return Positive;if(distance<-polygon.radius)return Negative;
 bool positive=false,negative=false;
 for(unsigned int i=0;i<polygon.count;i++){
  distance=Distance(plane,polygon.points[i]);
  if((double)distance>0.1){if(negative)return Intersect;positive=true;}
  else if((double)distance<-0.1){if(positive)return Intersect;negative=true;}
 }
 return positive?Positive:(negative?Negative:CoplanarSide(plane,polygon));
}
inline Side FullSide(const Polygon& plane,const Polygon& polygon){
 bool positive=false,negative=false;const float epsilon=0.01f;
 for(unsigned int i=0;i<polygon.count;i++){
  const float distance=Distance(plane,polygon.points[i]);
  if(distance>epsilon)positive=true;else if(distance<-epsilon)negative=true;
 }
 return positive?(negative?Intersect:Positive):(negative?Negative:CoplanarSide(plane,polygon));
}
// Integer exponent, with double stores at every multiplication. Original pow
// returns double and then rounds the balance once to float.
inline float Balance(unsigned int depth){
 volatile double base=0.9,value=1.0;
 while(depth){if(depth&1)value=value*base;depth>>=1;if(depth)base=base*base;}
 return (float)value;
}
__declspec(noinline) inline float Weight(unsigned int added,unsigned int difference,unsigned int depth){
 const float balance=Balance(depth);float factor,diff,result,addedFloat=(float)added,diffFloat=(float)difference;
#if defined(_M_IX86)
 const float one=1.0f;
 __asm {
 fld one
 fsub balance
 fmul addedFloat
 fstp factor
 fld diffFloat
 fmul balance
 fstp diff
 fld diff
 fadd factor
 fstp result
 }
#else
 factor=(float)((1.0-(double)balance)*addedFloat);diff=diffFloat*balance;result=diff+factor;
#endif
 return result;
}
inline bool Validate(const Polygon& p){
 if(!p.points||p.count<3||p.count>65535||!WorldText::Finite(p.radius)||p.radius<0||!WorldText::Finite(p.distance))return false;
 for(unsigned int k=0;k<3;k++)if(!WorldText::Finite((&p.normal.x)[k])||!WorldText::Finite((&p.center.x)[k]))return false;
 for(unsigned int i=0;i<p.count;i++)for(unsigned int k=0;k<3;k++)if(!WorldText::Finite((&p.points[i].x)[k]))return false;
 return p.normal.x!=0||p.normal.y!=0||p.normal.z!=0;
}
inline bool Select(const Polygon* polygons,unsigned int count,unsigned int depth,Selector& result){
 if(!polygons||!count||count>100000||depth>4096)return false;
 unsigned int type=2;for(unsigned int i=0;i<count;i++){if(!Validate(polygons[i]))return false;const unsigned int t=Type(polygons[i].flags);if(t<type)type=t;}
 if(type==2)return false;
 unsigned int typedCount=0;for(unsigned int i=0;i<count;i++)if(Type(polygons[i].flags)==type)typedCount++;
 const unsigned int comparisons=SearchCount(typedCount);bool found=false;
 for(unsigned int candidate=0;candidate<count;candidate++)if(Type(polygons[candidate].flags)==type){
  Selector current={};current.index=candidate;unsigned int scanned=0;
  for(unsigned int i=0;i<count&&scanned<comparisons;i++)if(Type(polygons[i].flags)==type){
   scanned++;if(i==candidate)continue;
   Side side=QuickSide(polygons[candidate],polygons[i]);if(side==Intersect)current.splits++;else if(side==Positive)current.positive++;else current.negative++;
  }
  current.sideDifference=current.positive>current.negative?current.positive-current.negative:current.negative-current.positive;
  current.maxSide=current.positive>current.negative?current.positive:current.negative;
  current.weight=Weight(current.splits,current.sideDifference,depth);
  if(!found||current.weight<result.weight){result=current;found=true;}
 }
 return found;
}
}
