#pragma once
#include "RenderFrameHandedness.h"
#include "RenderFrameSort.h"
#include "RenderTextureCoordinates.h"
namespace SourceRenderFrameRecord {
// Caller provides provenance group. Root fixtures verify geometry ordinal+1;
// expanded prefab/world-model numbering must be resolved by the caller.
inline bool Build(const RenderClipVertex::Vertex& vertex,const RenderTextureCoordinates::Basis* bases,const WorldText::Vec3& plane,unsigned int group,unsigned int mode,unsigned int ordinal,RenderFrameSort::Record& output,bool allowExceptionalUV=false){
 if(!bases||mode>2||!WorldText::FiniteVector(plane))return false;
 WorldText::Vec3 p,q;if(!RenderFrameProjection::Build(bases[0].u,bases[0].v,plane,p,q))return false;
 unsigned int handedness;if(!RenderFrameHandedness::Build(bases[0].u,bases[0].v,plane,handedness))return false;
 float uv[8];if(!RenderTextureCoordinates::Build(vertex.point,bases,uv,allowExceptionalUV))return false;
 RenderFrameSort::Record record={};memcpy(record.bytes,&vertex.point,12);
 if(mode==2)memcpy(record.bytes+12,&plane,12);else memcpy(record.bytes+12,vertex.attributes+4,12);
 memcpy(record.bytes+24,&p,12);memcpy(record.bytes+36,&q,12);memcpy(record.bytes+48,&mode,4);record.bytes[52]=(unsigned char)handedness;memcpy(record.bytes+56,&group,4);memcpy(record.bytes+60,uv,8);memcpy(record.bytes+68,&ordinal,4);output=record;return true;
}
}
