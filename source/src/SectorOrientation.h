#pragma once
#include "ExtrusionVolume.h"
namespace SectorOrientation {
struct Face {WorldText::Vec3* points;unsigned int count;ExtrusionVolume::Plane plane;};
// RVA 0x12A90 tests each plane against vertices of later faces only.
inline bool Check(const Face* faces,unsigned int count,float tolerance){
 for(unsigned int i=0;i<count;i++)for(unsigned int j=i+1;j<count;j++)for(unsigned int k=0;k<faces[j].count;k++){const WorldText::Vec3& p=faces[j].points[k];const ExtrusionVolume::Plane& n=faces[i].plane;float distance=(float)(double(p.z)*n.z+double(p.y)*n.y+double(p.x)*n.x-n.d+tolerance);if(distance<0)return false;}return true;
}
inline void Flip(Face* faces,unsigned int count){for(unsigned int i=0;i<count;i++){Face& f=faces[i];f.plane.x=-f.plane.x;f.plane.y=-f.plane.y;f.plane.z=-f.plane.z;f.plane.d=-f.plane.d;for(unsigned int j=0;j<f.count/2;j++){WorldText::Vec3 p=f.points[j];f.points[j]=f.points[f.count-j-1];f.points[f.count-j-1]=p;}}}
}
