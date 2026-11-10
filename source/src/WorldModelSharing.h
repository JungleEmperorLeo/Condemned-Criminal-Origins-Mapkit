#pragma once
#include "WorldModels.h"
// Direct prefab instances share the loaded prototype. Nested prototype identity
// also retains its outermost instance context, even for identical files/poses.
namespace WorldModelSharing {
inline bool PathEqual(const char* a,const char* b){for(;;a++,b++){char x=*a,y=*b;if(x>='A'&&x<='Z')x+=32;if(y>='A'&&y<='Z')y+=32;if(x!=y)return false;if(!x)return true;}}
class Identity {
 PrefabScene::Arena memory;const PrefabScene::Node** nodes;unsigned int count;
 const PrefabScene::Node* Ref(unsigned int index)const{while(index!=0xffffffffu){if(index>=count)return 0;const PrefabScene::Node* n=nodes[index];if(n->sourceKind==3)return n;index=n->parent;}return 0;}
public:
 unsigned int* representative;unsigned int* serializedIndex;unsigned int uniqueCount;
 Identity():nodes(0),count(0),representative(0),serializedIndex(0),uniqueCount(0){}
 bool Same(const PrefabScene::Node& a,const PrefabScene::Node& b)const{
  if(a.sourceId!=b.sourceId||!PathEqual(a.source,b.source))return false;
  const PrefabScene::Node* x=Ref(a.parent);const PrefabScene::Node* y=Ref(b.parent);
  // Source nodes outside prefabs are separate instances, never aliases.
  if(!x||!y)return a.index==b.index;
  unsigned int depth=1;
  for(;;){const PrefabScene::Node* px=Ref(x->parent);const PrefabScene::Node* py=Ref(y->parent);
   if(!px||!py)return !px&&!py&&(depth==1||x->index==y->index);
   if(x->sourceId!=y->sourceId||!PathEqual(x->source,y->source))return false;
   x=px;y=py;depth++;
  }
 }
 bool Build(const PrefabScene::Model& model,const WorldModels::Partition& partition){
  memory.Clear();nodes=0;representative=0;serializedIndex=0;uniqueCount=0;count=model.count;
  nodes=(const PrefabScene::Node**)memory.Allocate((count+1)*sizeof(*nodes));representative=(unsigned int*)memory.Allocate((partition.groupCount+1)*sizeof(unsigned int));serializedIndex=(unsigned int*)memory.Allocate((partition.groupCount+1)*sizeof(unsigned int));if(!nodes||!representative||!serializedIndex)return false;
  unsigned int seen=0;for(const PrefabScene::Node* n=model.first;n;n=n->next){if(n->index!=seen||seen>=count||(n->parent!=0xffffffffu&&n->parent>=seen))return false;nodes[seen++]=n;}if(seen!=count)return false;
  representative[0]=0;serializedIndex[0]=0;uniqueCount=1;
  for(unsigned int i=1;i<partition.groupCount;i++){if(!partition.groups[i].owner)return false;representative[i]=i;for(unsigned int j=1;j<i;j++)if(representative[j]==j&&Same(*partition.groups[i].owner,*partition.groups[j].owner)){representative[i]=j;break;}if(representative[i]==i)serializedIndex[i]=uniqueCount++;else serializedIndex[i]=serializedIndex[representative[i]];}
  return true;
 }
};
}
