#pragma once
#include "SectorOrientation.h"
namespace PortalOrientation {
// RVA 0x2DD20: greatest absolute signed distance over first owner volume.
// Ties keep the earlier value; negative greatest distance flips the portal.
inline bool Orient(WorldText::Vec3* points,unsigned int count,ExtrusionVolume::Plane& plane,const SectorOrientation::Face* owner,unsigned int ownerCount){
 if(!points||count<3||count>128||!owner||!ownerCount)return false;float greatest=0;
 for(unsigned int i=0;i<ownerCount;i++)for(unsigned int j=0;j<owner[i].count;j++){const WorldText::Vec3& p=owner[i].points[j];if(!WorldText::FiniteVector(p))return false;float d=(float)(double(plane.z)*p.z+double(plane.y)*p.y+double(plane.x)*p.x-plane.d);float magnitude=d<0?-d:d,best=greatest<0?-greatest:greatest;if(magnitude>best)greatest=d;}
 if(greatest<0){SectorOrientation::Face face={points,count,plane};SectorOrientation::Flip(&face,1);plane=face.plane;}return true;
}
}
