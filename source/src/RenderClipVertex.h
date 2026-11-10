#pragma once
#include "RenderContourCleanup.h"
namespace RenderClipVertex {
struct Vertex {unsigned int flags;WorldText::Vec3 point;float attributes[7];};
static_assert(sizeof(Vertex)==44,"Original intermediate vertex size");
inline float Mix(float a,float b,float t){return (float)(double(a)+(double(b)-a)*t);}
inline bool Interpolate(const Vertex& a,const Vertex& b,float t,Vertex& out){
 if(!WorldText::Finite(t))return false;
 Vertex result;result.flags=a.flags;
 result.point.x=Mix(a.point.x,b.point.x,t);result.point.y=Mix(a.point.y,b.point.y,t);result.point.z=Mix(a.point.z,b.point.z,t);
 for(unsigned int i=0;i<7;i++)result.attributes[i]=Mix(a.attributes[i],b.attributes[i],t);
 float x=result.attributes[4],y=result.attributes[5],z=result.attributes[6];
 float squared=(float)((double(x)*x+double(y)*y)+double(z)*z);
 if(!WorldText::FiniteVector(result.point)||!WorldText::Finite(squared)||squared<=0)return false;
 float inverse=(float)(1.0/RenderContourCleanup::Root(squared));
 result.attributes[4]=(float)(double(x)*inverse);result.attributes[5]=(float)(double(y)*inverse);result.attributes[6]=(float)(double(z)*inverse);
 for(unsigned int i=0;i<7;i++)if(!WorldText::Finite(result.attributes[i]))return false;
 out=result;return true;
}
}
