#pragma once
#include "RenderPackedRanges.h"
#include "RenderTrees.h"
#include "RenderGlobalSectionOrder.h"
namespace RenderPackedSection {
struct Input {RenderPackedRanges::Input range;unsigned int node,category;};
// One inline global mesh adapter. Caller provides working vertices, resolved
// declarations, flattened tree/node membership and ordering identities. Tree
// scaffolds carry independently prepared SkyPortal polygons. Streaming mesh
// partition and world-model geometry generation are separate caller stages.
class Block {
 PrefabScene::Arena memory;unsigned int cursor;
 bool Fail(const char* reason){memory.Clear();data=0;size=cursor=0;error=reason;return false;}
 bool Put(const void* p,unsigned int n){if(n>size-cursor||(!p&&n))return false;memcpy(data+cursor,p,n);cursor+=n;return true;}
 bool U32(unsigned int n){return Put(&n,4);}
 Block(const Block&);Block& operator=(const Block&);
public:
 unsigned char* data;unsigned int size;const char* error;
 Block():cursor(0),data(0),size(0),error(0){}
 bool Build(const Input* input,unsigned int count,unsigned int nodeCount,bool sort){
  if(nodeCount>65536)return Fail("Unsupported packed render adapter counts");
  PrefabScene::Arena scaffoldMemory;RenderTrees::Node* scaffoldNodes=(RenderTrees::Node*)scaffoldMemory.Allocate(nodeCount*sizeof(RenderTrees::Node));if(!scaffoldNodes)return Fail("Packed render scaffold allocation failed");memset(scaffoldNodes,0,nodeCount*sizeof(RenderTrees::Node));
  RenderTrees::Tree tree={scaffoldNodes,nodeCount};RenderTrees::Layout scaffold={&tree,1};return Build(input,count,scaffold,sort);
 }
 bool Build(const Input* input,unsigned int count,const RenderTrees::Layout& scaffold,bool sort,const RenderGlobalSectionOrder::MainPortalGraph* mainGraph=0){
  Fail(0);if((count&&!input)||count>RenderPackedRanges::MaximumSections||!scaffold.trees||!scaffold.count||scaffold.count>4096)return Fail("Unsupported packed render adapter counts");
  unsigned int nodeCount=0;for(unsigned int t=0;t<scaffold.count;t++){const RenderTrees::Tree& tree=scaffold.trees[t];if((tree.count&&!tree.nodes)||tree.count>65536-nodeCount)return Fail("Invalid packed render scaffold trees");nodeCount+=tree.count;for(unsigned int n=0;n<tree.count;n++)if(tree.nodes[n].count)return Fail("Packed render scaffold already contains branches");}
  RenderPackedRanges::Input* ranges=(RenderPackedRanges::Input*)memory.Allocate(count*sizeof(RenderPackedRanges::Input));
  unsigned int* surface=(unsigned int*)memory.Allocate(count*4);const char** names=(const char**)memory.Allocate(count*4);
  RenderTrees::Node* nodes=(RenderTrees::Node*)memory.Allocate(nodeCount*sizeof(RenderTrees::Node));
  RenderTrees::Tree* trees=(RenderTrees::Tree*)memory.Allocate(scaffold.count*sizeof(RenderTrees::Tree));
  RenderTrees::Branch* branches=(RenderTrees::Branch*)memory.Allocate(count*sizeof(RenderTrees::Branch));
  if(!ranges||!surface||!names||!nodes||!trees||!branches)return Fail("Packed render adapter allocation failed");
  for(unsigned int i=0;i<count;i++){if(input[i].node>=nodeCount||input[i].category>3)return Fail("Invalid packed render membership/category");ranges[i]=input[i].range;}
  if(mainGraph){
   if(!mainGraph->graph||mainGraph->graph->error||mainGraph->tree>=scaffold.count||mainGraph->graph->count!=scaffold.trees[mainGraph->tree].count)return Fail("Incompatible main-world portal graph");
   unsigned int firstNode=0;for(unsigned int t=0;t<mainGraph->tree;t++)firstNode+=scaffold.trees[t].count;
   unsigned int* keys=(unsigned int*)memory.Allocate(count*4);if(!keys)return Fail("Connected section keys allocation failed");RenderGlobalSectionOrder::Builder ordering;
   if(count&&!ordering.Apply(input,count,firstNode,*mainGraph->graph,keys))return Fail(ordering.error);for(unsigned int i=0;i<count;i++)ranges[i].orderKey=keys[i];
  }
  RenderPackedRanges::Builder packed;if(!packed.Build(ranges,count,sort))return Fail(packed.error);
  unsigned int nameBytes=0,definitionBytes=0;
  memset(names,0,count*4);
  for(unsigned int i=0;i<count;i++){
   surface[packed.order[i]]=i;unsigned int id=packed.ranges[i].material;
   if(!names[id]){names[id]=input[packed.order[i]].range.material;nameBytes+=2+lstrlenA(names[id]);}
  }
  for(unsigned int d=0;d<packed.declarationCount;d++)definitionBytes+=4+packed.declarationElements[d]*8;
  unsigned int nodeAt=0;for(unsigned int t=0;t<scaffold.count;t++){trees[t].nodes=nodes+nodeAt;trees[t].count=scaffold.trees[t].count;for(unsigned int n=0;n<trees[t].count;n++)nodes[nodeAt++]=scaffold.trees[t].nodes[n];}
  unsigned int at=0;
  for(unsigned int n=0;n<nodeCount;n++){
   RenderTrees::Node& node=nodes[n];node.branches=branches+at;node.count=0;
   // Each node retains its caller's indexed-section order. The global packed
   // sort remaps only mesh references, without permuting node branches.
   for(unsigned int i=0;i<count;i++)if(input[i].node==n){
    RenderTrees::Branch& b=branches[at++];const RenderPackedRanges::Input& r=input[i].range;
    b.type=input[i].category==3?0:input[i].category+1;b.meshIndex=0;b.surface=surface[i];
    if(r.vertexCount)b.low=b.high=r.vertices[0].position;else RenderTrees::EmptyBounds(b);
    for(unsigned int v=0;v<r.vertexCount;v++){
     const WorldText::Vec3& p=r.vertices[v].position;if(!WorldText::FiniteVector(p))return Fail("Invalid packed render branch position");
     if(p.x<b.low.x)b.low.x=p.x;if(p.y<b.low.y)b.low.y=p.y;if(p.z<b.low.z)b.low.z=p.z;
     if(p.x>b.high.x)b.high.x=p.x;if(p.y>b.high.y)b.high.y=p.y;if(p.z>b.high.z)b.high.z=p.z;
    }node.count++;
   }
  }
  RenderTrees::Layout layout={trees,scaffold.count};RenderTrees::Block topology;
  if(at!=count||!topology.Build(layout,count))return Fail("Invalid packed render topology");
  size=60+packed.vertexBytes+packed.indexCount*2+4+definitionBytes+4+count*36+nameBytes+topology.size;
  if(size>32*1024*1024)return Fail("Packed render section exceeds bound");data=(unsigned char*)memory.Allocate(size);if(!data)return Fail("Packed render section allocation failed");
  unsigned int header[15]={scaffold.count,nodeCount,count,1,packed.materialCount,1,count,topology.skyPortals,topology.skyPortalVertices,mainGraph?mainGraph->tree:0,0,count,packed.materialCount,packed.vertexBytes,packed.indexCount*2};
  if(!Put(header,60)||!Put(packed.vertices,packed.vertexBytes)||!Put(packed.indices,packed.indexCount*2)||!U32(packed.declarationCount))return Fail("Packed render buffer overflow");
  for(unsigned int d=0;d<packed.declarationCount;d++)if(!U32(packed.declarationElements[d]*8)||!Put(packed.declarations[d],packed.declarationElements[d]*8))return Fail("Packed render declaration overflow");
  if(!U32(count)||!Put(packed.ranges,count*36))return Fail("Packed render surface overflow");
  for(unsigned int m=0;m<packed.materialCount;m++){unsigned short n=(unsigned short)lstrlenA(names[m]);if(!Put(&n,2)||!Put(names[m],n))return Fail("Packed render material overflow");}
  if(!Put(topology.data,topology.size)||cursor!=size)return Fail("Packed render topology overflow");return true;
 }
};
}
