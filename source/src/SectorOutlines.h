#pragma once
#include "SectorPreparation.h"
namespace SectorOutlines {
// Original box shortcut RVA 0x2D8E0; writer RVA 0x2C5C1.
inline unsigned int Count(const SectorOrientation::Face* faces,unsigned int count){
 if(!faces||!count)return 0;if(count!=6)return count;
 for(unsigned int i=0;i<count;i++){bool aligned=false;for(unsigned int a=0;a<3;a++){float value=(&faces[i].plane.x)[a];if(value<0)value=-value;if(value>=0.9999f)aligned=true;}if(!aligned)return count;}return 0;
}
}
