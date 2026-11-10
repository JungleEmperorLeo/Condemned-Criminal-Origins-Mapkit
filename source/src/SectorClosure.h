#pragma once
#include "SectorOrientation.h"
namespace SectorClosure {
inline bool Near(const WorldText::Vec3& a,const WorldText::Vec3& b,float squaredTolerance){float x=a.x-b.x,y=a.y-b.y,z=a.z-b.z;return double(x)*x+double(y)*y+double(z)*z<=double(squaredTolerance);}
// RVA 0x12BF0: every edge must have exactly one matching edge on other faces.
// Either direction is accepted. Tolerance is squared and rounded once to float.
inline bool Check(const SectorOrientation::Face* faces,unsigned int count,float tolerance){
 if(!faces||count>4096||!WorldText::Finite(tolerance))return false;float squared=(float)(double(tolerance)*tolerance);
 for(unsigned int i=0;i<count;i++){if(!faces[i].points||faces[i].count<3||faces[i].count>128)return false;for(unsigned int k=0;k<faces[i].count;k++)if(!WorldText::FiniteVector(faces[i].points[k]))return false;}
 for(unsigned int i=0;i<count;i++)for(unsigned int a=0;a<faces[i].count;a++){
  const WorldText::Vec3& start=faces[i].points[a];const WorldText::Vec3& end=faces[i].points[(a+1)%faces[i].count];unsigned int matches=0;
  for(unsigned int j=0;j<count;j++)if(j!=i)for(unsigned int b=0;b<faces[j].count;b++){const WorldText::Vec3& otherStart=faces[j].points[b];const WorldText::Vec3& otherEnd=faces[j].points[(b+1)%faces[j].count];if((Near(otherStart,start,squared)&&Near(otherEnd,end,squared))||(Near(otherStart,end,squared)&&Near(otherEnd,start,squared)))matches++;}
  if(matches!=1)return false;
 }return true;
}
}
