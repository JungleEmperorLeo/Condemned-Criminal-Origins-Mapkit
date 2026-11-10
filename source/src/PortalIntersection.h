#pragma once
#include "ExtrusionVolume.h"
#include "PortalContour.h"
namespace PortalIntersection {
// Recovered 0x2FEC0 arithmetic. Bounded coplanar convex profile;
// output cleanup follows recovered 0x14260 scan order.
inline bool Clip(const WorldText::Vec3* subject,unsigned int subjectCount,const WorldText::Vec3* clipper,unsigned int clipperCount,const ExtrusionVolume::Plane& plane,WorldText::Vec3* output,unsigned int& count,bool* invalid=0){
 // invalid: the clipped portal fails the polygon check (0x10030516 "Generated invalid portal").
 count=0;if(invalid)*invalid=false;if(!subject||!clipper||!output||subjectCount<3||clipperCount<3||subjectCount>128||clipperCount>128)return false;
 PrefabScene::Arena memory;WorldText::Vec3* current=(WorldText::Vec3*)memory.Allocate(256*12);WorldText::Vec3* next=(WorldText::Vec3*)memory.Allocate(256*12);if(!current||!next)return false;
 for(unsigned int i=0;i<subjectCount;i++){if(!WorldText::FiniteVector(subject[i]))return false;current[i]=subject[i];}unsigned int size=subjectCount;
 for(unsigned int edge=0;edge<clipperCount;edge++){
  const WorldText::Vec3& u=clipper[edge];const WorldText::Vec3& v=clipper[(edge+1)%clipperCount];if(!WorldText::FiniteVector(u)||!WorldText::FiniteVector(v))return false;
  float dx=v.x-u.x,dy=v.y-u.y,dz=v.z-u.z;WorldText::Vec3 n={(float)(double(dy)*plane.z-double(dz)*plane.y),(float)(double(dz)*plane.x-double(dx)*plane.z),(float)(double(dx)*plane.y-double(dy)*plane.x)};if(!ExtrusionVolume::Normalize(n))return false;
  float d=(float)(double(n.y)*u.y+double(n.x)*u.x+double(n.z)*u.z);unsigned int used=0;
  for(unsigned int i=0;i<size;i++){
   const WorldText::Vec3& a=current[i];const WorldText::Vec3& b=current[(i+1)%size];float da=(float)(double(n.x)*a.x+double(n.z)*a.z+double(n.y)*a.y-d),db=(float)(double(n.z)*b.z+double(n.y)*b.y+double(n.x)*b.x-d);
   if(da>0){if(used>=256)return false;next[used++]=a;}
   if((da>0)!=(db>0)){
    if(used>=256)return false;float reciprocal=(float)(1.0/(double(da)-db));float delta[3]={b.x-a.x,b.y-a.y,b.z-a.z};WorldText::Vec3 q;float* dest=&q.x;const float* start=&a.x;
    for(unsigned int j=0;j<3;j++){float product=(float)(double(delta[j])*da);float scaled=(float)(double(product)*reciprocal);dest[j]=(float)(double(scaled)+start[j]);}if(!WorldText::FiniteVector(q))return false;next[used++]=q;
   }
  }
  if(used<3)return true;WorldText::Vec3* swap=current;current=next;next=swap;size=used;
 }
 if(size>128)return false;ExtrusionVolume::Plane cleanedPlane;if(!PortalContour::Clean(current,size,cleanedPlane)){if(invalid)*invalid=true;return true;}for(unsigned int i=0;i<size;i++)output[i]=current[i];count=size;return true;
}
}
