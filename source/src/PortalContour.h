#pragma once
#include "ExtrusionVolume.h"
namespace PortalContour {
inline void Remove(WorldText::Vec3* points,unsigned int& count,unsigned int index){for(unsigned int j=index;j+1<count;j++)points[j]=points[j+1];count--;}
// RVA 0x14260: preserve original scan/retry order on each deletion.
inline bool Clean(WorldText::Vec3* points,unsigned int& count,ExtrusionVolume::Plane& plane){
 if(!points||count<3||count>128)return false;for(unsigned int i=0;i<count;i++)if(!WorldText::FiniteVector(points[i]))return false;
 for(unsigned int i=0;i<count;){const WorldText::Vec3& a=points[i];const WorldText::Vec3& b=points[(i+1)%count];float x=a.x-b.x,y=a.y-b.y,z=a.z-b.z;double squared=double(x)*x+double(y)*y+double(z)*z;if(squared<=double(0.01f)){Remove(points,count,i);if(count<3)return false;}else i++;}
 for(unsigned int i=0;i<count;){const WorldText::Vec3& a=points[(i+count-1)%count];const WorldText::Vec3& b=points[i];const WorldText::Vec3& c=points[(i+1)%count];WorldText::Vec3 first={b.x-a.x,b.y-a.y,b.z-a.z},second={c.x-a.x,c.y-a.y,c.z-a.z};if(!ExtrusionVolume::Normalize(first)||!ExtrusionVolume::Normalize(second))return false;float x=second.x-first.x,y=second.y-first.y,z=second.z-first.z;double squared=double(x)*x+double(y)*y+double(z)*z;if(squared<=double(0.0001f)){Remove(points,count,i);if(count<3)return false;}else i++;}
 if(!ExtrusionVolume::ComputeInternalPlane(points,count,plane))return false;
 // 0x100145E9: every point within 1.0 of the plane (|n.p - d| stored as float).
 for(unsigned int i=0;i<count;i++){volatile float r=(float)(((double(points[i].y)*plane.y+double(points[i].z)*plane.z)+double(points[i].x)*plane.x)-plane.d);float a=r<0?-r:r;if(a>1.0f)return false;}
 // 0x10014647: for edge i (i < count-2) with inward normal n x e, the later points
 // i+2.. lie at most 1.0 beyond it.
 for(unsigned int i=0;i+2<count;i++){
  volatile float ez=points[i+1].z-points[i].z,ey=points[i+1].y-points[i].y,ex=points[i+1].x-points[i].x;WorldText::Vec3 e={ex,ey,ez};if(!ExtrusionVolume::Normalize(e))return false;
  volatile float cz=(float)(double(e.y)*plane.x-double(e.x)*plane.y),cx=(float)(double(e.z)*plane.y-double(e.y)*plane.z),cy=(float)(double(e.x)*plane.z-double(e.z)*plane.x);WorldText::Vec3 c={cx,cy,cz};if(!ExtrusionVolume::Normalize(c))return false;
  volatile float d0=(float)((double(c.z)*points[i].z+double(c.y)*points[i].y)+double(c.x)*points[i].x);
  for(unsigned int j=i+2;j<count;j++)if(((double(c.z)*points[j].z+double(c.y)*points[j].y)+double(c.x)*points[j].x)-d0>1.0)return false;
 }
 return true;
}
}
