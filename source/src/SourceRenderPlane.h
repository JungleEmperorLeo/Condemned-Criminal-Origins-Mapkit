#pragma once
#include "SourceRenderNormals.h"
namespace SourceRenderPlane {
// Original RVA 0x7630: transform the rounded source normal, normalize in double,
// snap tiny components, renormalize, then snap near-cardinal vectors.
// Caller supplies the normal after the owner transform (identity in root controls).
inline double Abs(double x){return x<0?-x:x;}
inline bool Normalize(double* v){double length=SourceRenderNormals::Root((v[0]*v[0]+v[1]*v[1])+v[2]*v[2]);if(length<=0)return false;double inverse=1.0/length;for(unsigned int i=0;i<3;i++)v[i]*=inverse;return true;}
inline bool Build(const WorldText::Vec3& input,WorldText::Vec3& output){
 if(!WorldText::FiniteVector(input))return false;
 double v[3]={input.x,input.y,input.z};if(!Normalize(v))return false;
 bool changed=false;for(unsigned int i=0;i<3;i++)if(Abs(v[i])<0.0001){v[i]=0;changed=true;}
 if(changed&&!Normalize(v))return false;
 for(unsigned int i=0;i<3;i++)if(Abs(v[i])>=0.9999){v[(i+1)%3]=0;v[(i+2)%3]=0;break;}
 WorldText::Vec3 result={(float)v[0],(float)v[1],(float)v[2]};if(!WorldText::FiniteVector(result))return false;output=result;return true;
}
}
