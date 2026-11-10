#pragma once
#include "RenderTextureCoordinates.h"
#include "SourceSurfaceRotation.h"
namespace SourceTextureBasis {
// Strict bridge only: original mapping repair/orthogonalization is not implied.
inline bool Build(const WorldText::Polygon& polygon,const WorldText::Vec3& offset,RenderTextureCoordinates::Basis* output){
 if(!output||!WorldText::FiniteVector(offset)||!polygon.mappings||polygon.mappingCount>32)return false;
 RenderTextureCoordinates::Basis prepared[4]={};unsigned int seen=0;
 for(unsigned int i=0;i<polygon.mappingCount;i++){
  const WorldText::Mapping& m=polygon.mappings[i];if(m.channel>=4)continue;
  unsigned int bit=1u<<m.channel;if((seen&bit)||!WorldText::FiniteVector(m.p)||!WorldText::FiniteVector(m.q))return false;
#ifndef FEAR_NONFINITE_ORIGIN_EXPERIMENT
  if(!m.valid||!WorldText::FiniteVector(m.origin))return false;
#endif
  seen|=bit;RenderTextureCoordinates::Basis& b=prepared[m.channel];
  b.origin.x=m.origin.x-offset.x;b.origin.y=m.origin.y-offset.y;b.origin.z=m.origin.z-offset.z;b.u=m.p;b.v=m.q;
#ifdef FEAR_SURFACE_IDENTITY_ROTATION_EXPERIMENT
  b.u=SourceSurfaceRotation::Identity(m.p);b.v=SourceSurfaceRotation::Identity(m.q);
#endif
#ifdef FEAR_NONFINITE_ORIGIN_EXPERIMENT
  if(!WorldText::FiniteVector(m.origin)){
   b.origin=SourceSurfaceRotation::Identity(m.origin);
   b.origin.x-=offset.x;b.origin.y-=offset.y;b.origin.z-=offset.z;
  }
#else
  if(!WorldText::FiniteVector(b.origin))return false;
#endif
 }
 if(seen!=15)return false;memcpy(output,prepared,sizeof(prepared));return true;
}
}
