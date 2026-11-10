#pragma once
#include "ConvexIntersection.h"
namespace ResourceAreaIntersection {
// Original resource AABB path RVA 0x12910. Unlike visibility assignment,
// this uses strict bounds overlap and plane support points, with no tolerance.
inline bool Test(const ConvexIntersection::Volume& volume,const WorldText::Vec3& low,const WorldText::Vec3& high,bool& hit){
 hit=false;if(!volume.faces||!volume.count||volume.count>4096)return false;
 for(unsigned int a=0;a<3;a++)if((&low.x)[a]>=(&volume.high.x)[a]||(&high.x)[a]<=(&volume.low.x)[a])return true;
 for(unsigned int i=0;i<volume.count;i++){
  const ExtrusionVolume::Plane& p=volume.faces[i].plane;
  float x=p.x>=0?high.x:low.x,y=p.y>=0?high.y:low.y,z=p.z>=0?high.z:low.z;
  float distance=(float)(((double)p.y*y+(double)p.z*z)+(double)p.x*x-p.d);
  if(distance<0)return true;
 }
 hit=true;return true;
}
}
