#pragma once
#include "RenderFrameProjection.h"
namespace RenderFrameHandedness {
inline bool Build(const WorldText::Vec3& p,const WorldText::Vec3& q,const WorldText::Vec3& normal,unsigned int& result){
 WorldText::Vec3 a=p,b=q;if(!RenderFrameProjection::Normalize(a)||!RenderFrameProjection::Normalize(b))return false;
 WorldText::Vec3 c={(float)(double(b.y)*a.z-double(b.z)*a.y),(float)(double(b.z)*a.x-double(b.x)*a.z),(float)(double(b.x)*a.y-double(b.y)*a.x)};
 result=((double(c.y)*normal.y+double(c.z)*normal.z)+double(c.x)*normal.x)>0?1u:0u;return true;
}
}
