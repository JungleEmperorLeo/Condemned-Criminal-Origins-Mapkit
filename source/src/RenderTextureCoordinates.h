#pragma once
#include "RenderExceptionalUV.h"
namespace RenderTextureCoordinates {
struct Basis {WorldText::Vec3 origin,u,v;};
inline bool Build(const WorldText::Vec3& position,const Basis* basis,float* output,bool allowExceptionalUV=false){
 if(!basis||!output||!WorldText::FiniteVector(position))return false;float result[8];
 for(unsigned int i=0;i<4;i++){
 const Basis& b=basis[i];if(!RenderExceptionalUV::Origin(b.origin,allowExceptionalUV)||!WorldText::FiniteVector(b.u)||!WorldText::FiniteVector(b.v))return false;
 float x=position.x-b.origin.x,y=position.y-b.origin.y,z=position.z-b.origin.z;
 result[i*2]=(float)((double(y)*b.u.y+double(z)*b.u.z)+double(x)*b.u.x);
 result[i*2+1]=(float)((double(y)*b.v.y+double(z)*b.v.z)+double(x)*b.v.x);
 if(!RenderExceptionalUV::Allowed(result[i*2],allowExceptionalUV)||!RenderExceptionalUV::Allowed(result[i*2+1],allowExceptionalUV))return false;
 }memcpy(output,result,sizeof(result));return true;
}
}
