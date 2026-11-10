#pragma once
#include "ExtrusionVolume.h"
namespace PortalCandidates {
// RVA 0x31EA9..0x31F52. Component sums are float32; squared sum stays
// extended. fabs of the extended plane-distance sum is stored as float32.
inline bool FacePlanesMatch(const ExtrusionVolume::Plane& a,const ExtrusionVolume::Plane& b){
 float x=(float)(double(b.x)+a.x),y=(float)(double(b.y)+a.y),z=(float)(double(b.z)+a.z);
 if(double(x)*x+double(y)*y+double(z)*z>double(0.0001f))return false;
 double sum=double(a.d)+b.d;float distance=(float)(sum<0?-sum:sum);return distance<=0.3f;
}
// RVA 0x124A0; original caller 0x31DD8 supplies float32 +0.1.
// x87 sums remain extended until comparison; equality is accepted.
inline bool BoundsOverlap(const WorldText::Vec3& aLow,const WorldText::Vec3& aHigh,const WorldText::Vec3& bLow,const WorldText::Vec3& bHigh,float tolerance){
 for(unsigned int i=0;i<3;i++)if(double((&bHigh.x)[i])+tolerance<(&aLow.x)[i]||double((&bLow.x)[i])-tolerance>(&aHigh.x)[i])return false;return true;
}
}

