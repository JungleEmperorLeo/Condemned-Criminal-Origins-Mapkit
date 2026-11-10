#pragma once
#include "StreamingConvex.h"
#include "StreamingMetadata.h"
namespace StreamingSourceMetadata {
// Adapts source-derived region geometry to the recovered assignment algorithm.
// External volumes use the same center as geometry and dense final SaveIDs.
// VisibilityCompiler retains construction order. Original 0x32D91 -> 0x2D9B0
// overwrites its sector vector in visibility-tree save order before 0x37990.
// Recreate that caller view here; the primitive Assignment keeps input order.
class Builder {
 PrefabScene::Arena memory;
 bool Fail(const char* message){assignment.Clear();error=message;return false;}
public:
 StreamingMetadata::Assignment assignment;const char* error;
 Builder():error(0){}
 bool Build(const StreamingConvex::Block& geometry,const StreamingMetadata::External* external,unsigned int count,const WorldText::Vec3& externalOffset){
  memory.Clear();error=0;assignment.Clear();
  if(!geometry.data||geometry.error)return Fail("Region geometry not built");
  for(unsigned int a=0;a<3;a++)if((&geometry.offset.x)[a]!=(&externalOffset.x)[a])return Fail("Visibility/streaming center mismatch");
  if(count>100000||(count&&!external))return Fail("Invalid prepared streaming sectors");
  StreamingMetadata::External* saved=(StreamingMetadata::External*)memory.Allocate((count+1)*sizeof(StreamingMetadata::External));
  unsigned char* seen=(unsigned char*)memory.Allocate(count+1);int* savedOwners=(int*)memory.Allocate((count+1)*sizeof(int));
  if(!saved||!seen||!savedOwners)return Fail("Allocation failed");
  for(unsigned int i=0;i<count;i++)seen[i]=0;
  for(unsigned int i=0;i<count;i++){
   unsigned int id=external[i].index;if(id>=count||seen[id])return Fail("Invalid prepared streaming SaveIDs");
   seen[id]=1;saved[id]=external[i];
  }
  unsigned int nr=geometry.regionCount;
  StreamingMetadata::Region* regions=(StreamingMetadata::Region*)memory.Allocate((nr+1)*sizeof(StreamingMetadata::Region));
  if(!regions)return Fail("Allocation failed");
  for(unsigned int r=0;r<nr;r++){
   unsigned int ordinary=0,linked=0;for(unsigned int i=0;i<geometry.volumeCount;i++)if(geometry.volumes[i].owner==r){if(geometry.volumes[i].linked<0)ordinary++;else linked++;}
   ConvexIntersection::Volume* o=(ConvexIntersection::Volume*)memory.Allocate((ordinary+1)*sizeof(ConvexIntersection::Volume));
   ConvexIntersection::Volume* l=(ConvexIntersection::Volume*)memory.Allocate((linked+1)*sizeof(ConvexIntersection::Volume));
   if(!o||!l)return Fail("Allocation failed");regions[r].ordinary=o;regions[r].ordinaryCount=ordinary;regions[r].linked=l;regions[r].linkedCount=linked;
   ordinary=linked=0;
   for(unsigned int i=0;i<geometry.volumeCount;i++)if(geometry.volumes[i].owner==r){
    const StreamingConvex::Volume& source=geometry.volumes[i];ConvexIntersection::Volume& v=source.linked<0?o[ordinary++]:l[linked++];
    ConvexIntersection::Face* faces=(ConvexIntersection::Face*)memory.Allocate(source.count*sizeof(ConvexIntersection::Face));if(!faces)return Fail("Allocation failed");
    v.faces=faces;v.count=source.count;v.low=*(const WorldText::Vec3*)geometry.bounds[i].low;v.high=*(const WorldText::Vec3*)geometry.bounds[i].high;
    for(unsigned int j=0;j<source.count;j++){faces[j].points=source.faces[j].points;faces[j].count=source.faces[j].count;faces[j].plane=source.faces[j].plane;}
   }
  }
  if(!assignment.Build(regions,nr,saved,count))return Fail(assignment.error);
  // Ownership reporting remains indexed by the caller's construction order;
  // region member lists preserve the original saved-sector traversal order.
  for(unsigned int i=0;i<count;i++)savedOwners[i]=assignment.owners[i];
  for(unsigned int i=0;i<count;i++)assignment.owners[i]=savedOwners[external[i].index];
  return true;
 }
};
}
