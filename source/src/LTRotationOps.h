#pragma once
#include "WorldText.h"
// Retail WorldPacker LTRotation vector rotation (VA 0x100067F0): q*v*conj(q)
// with float temporaries t1..t4; doubles are the x87 53-bit register values.
namespace LTRotationOps {
inline float F(double v){return (float)v;}
inline WorldText::Vec3 Rotate(const float* q,const WorldText::Vec3& v){
 const float x=q[0],y=q[1],z=q[2],w=q[3];
 const float t1=F(((double)y*v.z+(double)w*v.x)-(double)z*v.y),t2=F(((double)w*v.y-(double)x*v.z)+(double)z*v.x);
 const float t3=F(((double)v.y*x+(double)w*v.z)-(double)y*v.x),t4=F((-((double)v.x*x)-(double)v.y*y)-(double)z*v.z);
 const float nx=-x,ny=-y,nz=-z;WorldText::Vec3 r;
 r.x=F((((double)nx*t4+(double)nz*t2)+(double)w*t1)-(double)ny*t3);
 r.y=F((((double)ny*t4-(double)nz*t1)+(double)w*t2)+(double)nx*t3);
 r.z=F((((double)nz*t4+(double)ny*t1)-(double)nx*t2)+(double)w*t3);return r;
}
// LTRigidTransform * vector (VA 0x10006920): rotate, then add the position.
inline WorldText::Vec3 Apply(const float* q,const WorldText::Vec3& position,const WorldText::Vec3& v){
 const WorldText::Vec3 r=Rotate(q,v);WorldText::Vec3 out;out.z=F((double)r.z+position.z);out.y=F((double)r.y+position.y);out.x=F((double)r.x+position.x);return out;
}
}
