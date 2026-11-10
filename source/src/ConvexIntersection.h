#pragma once
#include "ExtrusionVolume.h"
namespace ConvexIntersection {
struct Face {const WorldText::Vec3* points;unsigned int count;ExtrusionVolume::Plane plane;};
struct Volume {const Face* faces;unsigned int count;WorldText::Vec3 low,high;};
inline float Distance(const ExtrusionVolume::Plane& p,const WorldText::Vec3& v){return (float)((double)p.z*v.z+(double)p.y*v.y+(double)p.x*v.x-p.d);}
inline bool Clip(const Face& input,const Volume& target,float tolerance,WorldText::Vec3* first,WorldText::Vec3* second,bool& hit){
 hit=false;if(input.count<3||input.count>128||target.count>4096)return false;unsigned int n=input.count;for(unsigned int i=0;i<n;i++)first[i]=input.points[i];float band=tolerance<0?-tolerance:tolerance;
 for(unsigned int plane=0;plane<target.count;plane++){
  const ExtrusionVolume::Plane& p=target.faces[plane].plane;unsigned int positive=0,negative=0;
  for(unsigned int i=0;i<n;i++){float d=ConvexIntersection::Distance(p,first[i]);if(d>=band)positive++;else if(d<=-band)negative++;}
  if(!positive&&!negative){double dot=(double)p.z*input.plane.z+(double)p.y*input.plane.y+(double)p.x*input.plane.x;if(dot>0)positive++;else negative++;}
  if(!negative)continue;if(!positive)return true;
  unsigned int output=0;
  for(unsigned int i=0;i<n;i++){
   const WorldText::Vec3& a=first[i];const WorldText::Vec3& b=first[(i+1)%n];float da=(float)((double)p.x*a.x+(double)p.y*a.y+(double)p.z*a.z-p.d+tolerance),db=(float)((double)p.x*b.x+(double)p.y*b.y+(double)p.z*b.z-p.d+tolerance);bool ina=da>=0,inb=db>=0;
   if(ina){if(output>=8192)return false;second[output++]=a;}
   if(ina!=inb){if(output>=8192)return false;float t=(float)((double)da/(double)(float)(da-db));WorldText::Vec3 q={(float)((double)a.x+(double)(float)(b.x-a.x)*t),(float)((double)a.y+(double)(float)(b.y-a.y)*t),(float)((double)a.z+(double)(float)(b.z-a.z)*t)};second[output++]=q;}
  }
  if(output<3)return true;n=output;WorldText::Vec3* swap=first;first=second;second=swap;
 }
 hit=true;return true;
}
inline bool Test(const Volume& a,const Volume& b,float tolerance,bool& hit){
 hit=false;if(!a.faces||!b.faces||!a.count||!b.count||a.count>4096||b.count>4096)return false;
 const float* al=&a.low.x;const float* ah=&a.high.x;const float* bl=&b.low.x;const float* bh=&b.high.x;
 for(unsigned int i=0;i<3;i++)if((double)bh[i]+1<al[i]||(double)bl[i]-1>ah[i])return true;
 WorldText::Vec3* memory=(WorldText::Vec3*)VirtualAlloc(0,2*8192*sizeof(WorldText::Vec3),MEM_COMMIT|MEM_RESERVE,4);if(!memory)return false;bool ok=true;
 for(unsigned int side=0;side<2&&!hit&&ok;side++){const Volume& source=side?b:a;const Volume& target=side?a:b;for(unsigned int i=0;i<source.count&&!hit&&ok;i++)ok=Clip(source.faces[i],target,tolerance,memory,memory+8192,hit);}
 VirtualFree(memory,0,MEM_RELEASE);return ok;
}
}

