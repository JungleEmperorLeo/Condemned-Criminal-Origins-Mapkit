#pragma once
// Original mesh-group extraction 0x4E320 and swap-last null removal 0x4DF10.
// A node identity is its flattened tree-order/node-order ordinal, not a pointer.
namespace RenderMeshGroups {
struct Tree {unsigned int nodeCount;};
struct Region {const char* name;const unsigned int* sectors;unsigned int count;};
struct Group {unsigned int regionIndex;const char* name;unsigned int* nodes;unsigned int count;};
class Builder {
 PrefabScene::Arena memory;
 Builder(const Builder&);Builder& operator=(const Builder&);
 bool Fail(const char* message){memory.Clear();groups=0;count=nodeCount=mainBegin=0;error=message;return false;}
 static void Compact(unsigned int* nodes,unsigned int& size){unsigned int i=0;while(i<size){if(nodes[i]==0xffffffffu)nodes[i]=nodes[--size];else i++;}}
 const char* CopyName(const char* name){unsigned int size=0;while(name[size])size++;char* owned=(char*)memory.Allocate(size+1);if(!owned)return 0;for(unsigned int i=0;i<=size;i++)owned[i]=name[i];return owned;}
public:
 Group* groups;unsigned int count,nodeCount,mainBegin;const char* error;
 Builder():groups(0),count(0),nodeCount(0),mainBegin(0),error(0){}
 bool Build(const Tree* trees,unsigned int treeCount,unsigned int mainTree,const Region* regions=0,unsigned int regionCount=0){
  Fail(0);if(!trees||!treeCount||treeCount>4096||mainTree>=treeCount||regionCount>4096||(!regions&&regionCount))return Fail("Invalid render mesh group tree/region input");
  for(unsigned int t=0;t<treeCount;t++){if(t==mainTree)mainBegin=nodeCount;if(trees[t].nodeCount>1000000-nodeCount)return Fail("Render mesh group node bound");nodeCount+=trees[t].nodeCount;}
  unsigned int references=0,nonempty=0;
  for(unsigned int r=0;r<regionCount;r++){
   const Region& region=regions[r];if(!region.name||(!region.sectors&&region.count)||region.count>1000000-references)return Fail("Invalid region name/sector input");references+=region.count;if(region.count)nonempty++;
   unsigned int length=0;for(;length<=4096&&region.name[length];length++)if((unsigned char)region.name[length]>=128)return Fail("Non-ASCII render region name unsupported");if(length>4096)return Fail("Render region name bound");
   for(unsigned int i=0;i<region.count;i++)if(region.sectors[i]>=trees[mainTree].nodeCount)return Fail("Region save ID outside main render tree");
  }
  groups=(Group*)memory.Allocate((nonempty+1)*sizeof(Group));unsigned int* defaultNodes=(unsigned int*)memory.Allocate((nodeCount+1)*4);if(!groups||!defaultNodes)return Fail("Render mesh group allocation failed");
  for(unsigned int n=0;n<nodeCount;n++)defaultNodes[n]=n;groups[0].regionIndex=0xffffffffu;groups[0].name=0;groups[0].nodes=defaultNodes;groups[0].count=nodeCount;count=1;
  for(unsigned int r=0;r<regionCount;r++){
   const Region& region=regions[r];if(!region.count)continue;Group& out=groups[count++];out.regionIndex=r;out.name=CopyName(region.name);out.nodes=(unsigned int*)memory.Allocate(region.count*4);out.count=region.count;if(!out.name||!out.nodes)return Fail("Render region group allocation failed");
   for(unsigned int i=0;i<region.count;i++){unsigned int global=mainBegin+region.sectors[i];out.nodes[i]=defaultNodes[global];defaultNodes[global]=0xffffffffu;}
   Compact(out.nodes,out.count);
  }
  Compact(groups[0].nodes,groups[0].count);return true;
 }
};
}
