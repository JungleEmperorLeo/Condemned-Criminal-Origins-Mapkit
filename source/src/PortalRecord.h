#pragma once
#include "PortalBounds.h"
namespace PortalRecord {
inline bool Write(unsigned char* destination,unsigned int capacity,const WorldText::Vec3* vertices,unsigned int count,const ExtrusionVolume::Plane& plane,unsigned int& bytes){
 bytes=0;if(!destination||!vertices||count<3||count>128||capacity<34+count*12)return false;for(unsigned int i=0;i<4;i++)if(!WorldText::Finite((&plane.x)[i]))return false;WorldText::Vec3 center;float radius;if(!PortalBounds::Compute(vertices,count,center,radius))return false;
 destination[0]=(unsigned char)count;destination[1]=(unsigned char)(count>>8);unsigned char* p=destination+2;const unsigned char* source=(const unsigned char*)vertices;for(unsigned int i=0;i<count*12;i++)*p++=source[i];source=(const unsigned char*)&plane;for(unsigned int i=0;i<16;i++)*p++=source[i];source=(const unsigned char*)&center;for(unsigned int i=0;i<12;i++)*p++=source[i];source=(const unsigned char*)&radius;for(unsigned int i=0;i<4;i++)*p++=source[i];bytes=34+count*12;return true;
}
}
