#pragma once
#include "ExtrusionVolume.h"
namespace PortalBounds {
// Serializer RVA 0x2C9F0 rounds min+max before multiplying by 0.5.
inline bool Compute(const WorldText::Vec3* points,unsigned int count,WorldText::Vec3& center,float& radius){
 if(!points||!count||count>128)return false;WorldText::Vec3 low=points[0],high=points[0];for(unsigned int i=0;i<count;i++){if(!WorldText::FiniteVector(points[i]))return false;for(unsigned int a=0;a<3;a++){float v=(&points[i].x)[a];if(v<(&low.x)[a])(&low.x)[a]=v;if(v>(&high.x)[a])(&high.x)[a]=v;}}
 for(unsigned int a=0;a<3;a++){float sum=(float)(double((&low.x)[a])+(&high.x)[a]);(&center.x)[a]=(float)(double(sum)*0.5);}radius=0;
 for(unsigned int i=0;i<count;i++){float x=center.x-points[i].x,y=center.y-points[i].y,z=center.z-points[i].z;float length=ExtrusionVolume::Length(x,y,z);if(length>radius)radius=length;}return true;
}
}
