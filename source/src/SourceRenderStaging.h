#pragma once
#include "RenderAttributes.h"
#include "RenderClipVertex.h"
#include "SourceRenderFrameRecord.h"
#include "RenderFrameAggregation.h"
#include "SourceRenderPositionCleanup.h"
namespace SourceRenderStaging {
struct Input {
 RenderClipVertex::Vertex vertex;
 RenderTextureCoordinates::Basis bases[4];
 WorldText::Vec3 plane;
 unsigned int group,mode;
};
static_assert(sizeof(Input)==208,"source staging input stride");
// Ordered source endpoints become complete working vertices before later
// coordinate cleanup, mesh welding and section serialization. No DLL inputs.
class Builder {
 PrefabScene::Arena memory;
 bool Fail(const char* reason){memory.Clear();vertices=0;frames=0;count=0;error=reason;return false;}
 Builder(const Builder&);Builder& operator=(const Builder&);
public:
 RenderAttributes::Vertex* vertices;
 RenderFrameAggregation::Frame* frames;
 unsigned int count;const char* error;
 Builder():vertices(0),frames(0),count(0),error(0){}
 bool Build(const Input* input,unsigned int size,bool allowExceptionalUV=false){
  Fail(0);if(!input||!size||size>RenderFrameLimits::MaximumEndpoints)return Fail("Invalid source staging size");
  RenderFrameSort::Record* records=(RenderFrameSort::Record*)memory.Allocate(size*72);
  vertices=(RenderAttributes::Vertex*)memory.Allocate(size*84);
  frames=(RenderFrameAggregation::Frame*)memory.Allocate(size*40);
  if(!records||!vertices||!frames)return Fail("Source staging allocation failed");
  for(unsigned int i=0;i<size;i++){
   const Input& in=input[i];RenderAttributes::Vertex& out=vertices[i];out=RenderAttributes::Vertex();out.position=in.vertex.point;
   if(!SourceRenderFrameRecord::Build(in.vertex,in.bases,in.plane,in.group,in.mode,i,records[i],allowExceptionalUV)||
      !RenderTextureCoordinates::Build(in.vertex.point,in.bases,out.uv,allowExceptionalUV)||
      !SourceRenderColor::PackWhiteRGB(in.vertex.attributes[3],out.color))return Fail("Invalid source endpoint attributes");
  }
  if(!RenderFrameSort::Build(records,size)||!RenderFrameAggregation::Build(records,size,frames))return Fail("Unsupported source frame aggregation");
  for(unsigned int i=0;i<size;i++){
   const RenderFrameAggregation::Frame& frame=frames[i];if(frame.vertex>=size)return Fail("Invalid source frame ordinal");
   RenderAttributes::Vertex& out=vertices[frame.vertex];out.normal=frame.normal;out.tangent=frame.tangent;out.binormal=frame.binormal;
  }
  count=size;return true;
 }
 bool SnapPositions(){
  if(!vertices||!count)return false;
  WorldText::Vec3* points=(WorldText::Vec3*)memory.Allocate(count*12);if(!points)return Fail("Position cleanup allocation failed");
  for(unsigned int i=0;i<count;i++)points[i]=vertices[i].position;
  SourceRenderPositionCleanup::Pool pool;if(!pool.Initialize(points,count))return Fail("Invalid position cleanup pool");
  for(unsigned int i=0;i<count;i++){bool matched=false;if(!pool.Snap(vertices[i].position,i/3,matched))return Fail("Position cleanup failed");}
  return true;
 }
};
}
