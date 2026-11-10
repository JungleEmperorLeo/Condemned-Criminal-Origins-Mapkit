#pragma once
#include "ConvexIntersection.h"
namespace StreamingMetadata {
struct Region {const ConvexIntersection::Volume* ordinary;unsigned int ordinaryCount;const ConvexIntersection::Volume* linked;unsigned int linkedCount;};
// Original caller supplies visibility-sector volumes from world+0xD8.
struct External {ConvexIntersection::Volume volume;unsigned int index;};
struct References {unsigned int* indices;unsigned int count;};
class Assignment {
 PrefabScene::Arena memory;
 bool Fail(const char* message){error=message;return false;}
 bool Intersects(const Region& region,const ConvexIntersection::Volume& external,bool& hit){
  hit=false;for(unsigned int pass=0;pass<2&&!hit;pass++){
   unsigned int count=pass?region.linkedCount:region.ordinaryCount;const ConvexIntersection::Volume* volumes=pass?region.linked:region.ordinary;
   if(count&&!volumes)return Fail("Missing region volumes");
   for(unsigned int i=0;i<count&&!hit;i++)if(!ConvexIntersection::Test(external,volumes[i],-2,hit))return Fail("Invalid convex intersection input");
  }return true;
 }
public:
 // multiple[i]: external i (input order) lies in more than one region (0x10037C37).
 References* regions;int* owners;unsigned char* multiple;unsigned int unassigned,ambiguous;const char* error;
 Assignment():regions(0),owners(0),multiple(0),unassigned(0),ambiguous(0),error(0){}
 void Clear(){memory.Clear();regions=0;owners=0;multiple=0;unassigned=ambiguous=0;error=0;}
 bool Build(const Region* input,unsigned int regionCount,const External* external,unsigned int externalCount){
  Clear();
  if(regionCount>10000||externalCount>100000||(regionCount&&!input)||(externalCount&&!external))return Fail("Invalid metadata input");
  regions=(References*)memory.Allocate((regionCount+1)*sizeof(References));owners=(int*)memory.Allocate((externalCount+1)*sizeof(int));multiple=(unsigned char*)memory.Allocate(externalCount+1);if(!regions||!owners||!multiple)return Fail("Allocation failed");
  for(unsigned int i=0;i<regionCount;i++){regions[i].indices=0;regions[i].count=0;}
  for(unsigned int i=0;i<externalCount;i++){
   owners[i]=-1;multiple[i]=0;for(unsigned int j=0;j<regionCount;j++){bool hit;if(!Intersects(input[j],external[i].volume,hit))return false;if(!hit)continue;
    if(owners[i]>=0){ambiguous++;multiple[i]=1;break;}owners[i]=(int)j;regions[j].count++;
   }if(owners[i]<0&&regionCount)unassigned++;
  }
  for(unsigned int j=0;j<regionCount;j++){unsigned int count=regions[j].count;regions[j].indices=(unsigned int*)memory.Allocate((count+1)*4);if(!regions[j].indices)return Fail("Allocation failed");regions[j].count=0;}
  for(unsigned int i=0;i<externalCount;i++)if(owners[i]>=0){References& refs=regions[owners[i]];refs.indices[refs.count++]=external[i].index;}
  return true;
 }
};
}
