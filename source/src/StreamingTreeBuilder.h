#pragma once
#include "StreamingSplit.h"
#include "StreamingTree.h"
namespace StreamingTreeBuilder {
class Builder {
 PrefabScene::Arena memory;const StreamingSplit::Bounds* bounds;StreamingTree::Node* nodes;unsigned int capacity;
 bool Visit(const unsigned int* ids,unsigned int n,unsigned int depth,int& index){
  if(!n||depth>128||nodeCount>=capacity)return false;index=(int)nodeCount++;
  StreamingSplit::Bounds* local=(StreamingSplit::Bounds*)memory.Allocate(n*sizeof(StreamingSplit::Bounds));if(!local)return false;
  StreamingSplit::Result split;for(unsigned int k=0;k<3;k++){split.ids[k]=(unsigned int*)memory.Allocate(n*4);if(!split.ids[k])return false;}
  for(unsigned int i=0;i<n;i++)local[i]=bounds[ids[i]];
  if(!StreamingSplit::Choose(local,n,split))return false;
  StreamingTree::Node& node=nodes[index];node.axis=split.axis;node.split=split.coordinate;node.childA=node.childB=-1;node.convexCount=split.count[0];
  for(unsigned int k=0;k<3;k++)for(unsigned int i=0;i<split.count[k];i++)split.ids[k][i]=ids[split.ids[k][i]];
  node.convexIds=split.ids[0];
  if(split.axis!=3){if(split.count[1]>=n||split.count[2]>=n)return false;if(!Visit(split.ids[1],split.count[1],depth+1,node.childA)||!Visit(split.ids[2],split.count[2],depth+1,node.childB))return false;}
  return true;
 }
public:
 StreamingTree::Block block;unsigned int nodeCount;
 Builder():bounds(0),nodes(0),capacity(0),nodeCount(0){}
 bool Build(const StreamingSplit::Bounds* input,unsigned int count){
  memory.Clear();nodeCount=0;if(!input||!count||count>50000)return false;bounds=input;capacity=2*count-1;nodes=(StreamingTree::Node*)memory.Allocate(capacity*sizeof(StreamingTree::Node));unsigned int* ids=(unsigned int*)memory.Allocate(count*4);if(!nodes||!ids)return false;for(unsigned int i=0;i<count;i++)ids[i]=i;int root;if(!Visit(ids,count,0,root))return false;return block.Build(nodes,nodeCount,(unsigned int)root,count);
 }
};
}
