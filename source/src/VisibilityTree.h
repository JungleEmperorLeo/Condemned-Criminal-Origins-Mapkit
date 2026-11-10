#pragma once
#include "StreamingTreeBuilder.h"
namespace VisibilityTree {
// Original RVA 0x2D9B0 gathers sector pointers in preorder: node members,
// child 0, child 1. Final save IDs follow this order, not authoring order.
class Builder {
 PrefabScene::Arena memory;
public:
 StreamingTreeBuilder::Builder spatial;unsigned int* order;unsigned int* saveIds;
 unsigned char* data;unsigned int size,nodeCount;const char* error;
 Builder():order(0),saveIds(0),data(0),size(0),nodeCount(0),error(0){}
 bool Build(const StreamingSplit::Bounds* bounds,unsigned int count){
  memory.Clear();order=saveIds=0;data=0;size=nodeCount=0;error=0;
  if(!spatial.Build(bounds,count)){error="Could not build visibility tree";return false;}
  order=(unsigned int*)memory.Allocate(count*4);saveIds=(unsigned int*)memory.Allocate(count*4);data=(unsigned char*)memory.Allocate(spatial.block.size);if(!order||!saveIds||!data){error="Allocation failed";return false;}
  for(unsigned int i=0;i<count;i++)saveIds[i]=0xffffffff;
  const unsigned int* cursor=(const unsigned int*)spatial.block.data;const unsigned int* end=(const unsigned int*)(spatial.block.data+spatial.block.size);unsigned int next=0;
  for(unsigned int node=0;node<spatial.nodeCount;node++){
   if(cursor>=end)return false;unsigned int n=*cursor++;if(n>count||(unsigned int)(end-cursor)<n+4)return false;
   for(unsigned int i=0;i<n;i++){unsigned int id=cursor[i];if(id>=count||saveIds[id]!=0xffffffff||next>=count){error="Invalid sector traversal";return false;}order[next]=id;saveIds[id]=next++;}cursor+=n+4;
  }
  if(cursor!=end||next!=count){error="Incomplete sector traversal";return false;}
  size=spatial.block.size;nodeCount=spatial.nodeCount;for(unsigned int i=0;i<size;i++)data[i]=spatial.block.data[i];unsigned int* out=(unsigned int*)data;
  for(unsigned int node=0;node<nodeCount;node++){unsigned int n=*out++;for(unsigned int i=0;i<n;i++)out[i]=saveIds[out[i]];out+=n+4;}return true;
 }
};
}
