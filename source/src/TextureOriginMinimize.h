#pragma once
#include "RenderTextureCoordinates.h"
namespace TextureOriginMinimize {
inline float Dot(const WorldText::Vec3& p,const WorldText::Vec3& o,const WorldText::Vec3& v){float x=p.x-o.x,y=p.y-o.y,z=p.z-o.z;return (float)((double(x)*v.x+double(y)*v.y)+double(z)*v.z);}
inline bool Build(const WorldText::Vec3* points,unsigned int count,RenderTextureCoordinates::Basis* bases){
 if(!points||count<3||count>128||!bases)return false;
 for(unsigned int slot=0;slot<4;slot++)for(unsigned int axis=0;axis<2;axis++){
 WorldText::Vec3& origin=bases[slot].origin;const WorldText::Vec3 v=axis?bases[slot].v:bases[slot].u;
#ifdef FEAR_NONFINITE_ORIGIN_EXPERIMENT
 // Original search retains block zero when exceptional origin makes scores unordered.
 if(!WorldText::FiniteVector(origin)&&WorldText::FiniteVector(v))continue;
#endif
 float length=(float)((double(v.x)*v.x+double(v.z)*v.z)+double(v.y)*v.y);if(!WorldText::Finite(length))return false;
 // Original zero-axis search selects block zero and leaves origin unchanged.
 // Do not form its unused infinite reciprocal and NaN candidates.
 if(length==0&&v.x==0&&v.y==0&&v.z==0)continue;
 if(length<=0)return false;
 float inverse=(float)(1.0/length);WorldText::Vec3 inv={(float)(double(v.x)*inverse),(float)(double(v.y)*inverse),(float)(double(v.z)*inverse)};
 float low=1e37f,high=-1e37f;for(unsigned int i=0;i<count;i++){float t=Dot(points[i],origin,v);if(t<low)low=t;if(t>high)high=t;}
 float extent=(-low>high)?-low:high;if(!WorldText::Finite(extent))return false;int limit=extent>=511488?500:(int)(double(extent)/1024+.5);if(limit<0)limit=0;if(limit>500)limit=500;
 int best=0;float bestScore=0;
 for(int block=-limit;block<=limit;block++){
 float amount=(float)(unsigned int)(block*1024);WorldText::Vec3 candidate={(float)(double(origin.x)+(float)(double(inv.x)*amount)),(float)(double(origin.y)+(float)(double(inv.y)*amount)),(float)(double(origin.z)+(float)(double(inv.z)*amount))};float score=0;
 for(unsigned int i=0;i<count;i++){float t=Dot(points[i],candidate,v);if(t<0)t=-t;score+=t;}
 if(block==-limit||score<bestScore){bestScore=score;best=block;}
 }
 if(best){float amount=(float)(unsigned int)(best*1024);origin.x+=(float)(double(inv.x)*amount);origin.y+=(float)(double(inv.y)*amount);origin.z+=(float)(double(inv.z)*amount);}
 }return true;
}
}
