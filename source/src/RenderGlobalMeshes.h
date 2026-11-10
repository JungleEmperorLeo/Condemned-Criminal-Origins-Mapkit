#pragma once
#include "RenderPackedSection.h"
#include "RenderPackedMesh.h"
#include "RenderMeshGroups.h"
#include "RenderMeshFilename.h"
namespace RenderGlobalMeshes {
struct Mesh {unsigned int regionIndex;const char* name;const char* filename;RenderPackedMesh::Block* geometry;unsigned char* external;unsigned int externalBytes;};
class Block {
 PrefabScene::Arena memory;unsigned int cursor,initializedMeshes;
 Block(const Block&);Block& operator=(const Block&);
 bool Fail(const char* message){Clear();error=message;return false;}
 bool Put(const void* source,unsigned int bytes){if(bytes>size-cursor||(!source&&bytes))return false;if(bytes)memcpy(data+cursor,source,bytes);cursor+=bytes;return true;}
 bool U32(unsigned int value){return Put(&value,4);}
 bool Filename(const char* stem,const char* region,const char* projectRoot,Mesh& mesh){
  char* filename=(char*)memory.Allocate(RenderMeshFilename::MaximumFilename+1);char* reference=(char*)memory.Allocate(260);if(!filename||!reference||!RenderMeshFilename::Build(stem,region,projectRoot?projectRoot:"",filename,RenderMeshFilename::MaximumFilename+1,reference,260))return false;mesh.filename=filename;mesh.name=reference;return true;
 }
public:
 unsigned char* data;unsigned int size,meshCount;Mesh* meshes;const char* error;
 Block():cursor(0),initializedMeshes(0),data(0),size(0),meshCount(0),meshes(0),error(0){}~Block(){Clear();}
 void Clear(){for(unsigned int m=0;m<initializedMeshes;m++)meshes[m].geometry->~Block();memory.Clear();data=0;meshes=0;size=meshCount=cursor=initializedMeshes=0;error=0;}
 bool Build(const RenderPackedSection::Input* input,unsigned int count,const RenderTrees::Layout& scaffold,unsigned int mainTree,const RenderMeshGroups::Region* regions,unsigned int regionCount,const char* worldStem,bool sort,const RenderVisibilityPortalGraph::Graph* mainGraph=0,const char* projectRoot=0){
  Clear();if(count>RenderPackedRanges::MaximumSections||(!input&&count)||!scaffold.trees||!scaffold.count||scaffold.count>4096||mainTree>=scaffold.count)return Fail("Invalid global render inputs");
  RenderMeshGroups::Tree* groupTrees=(RenderMeshGroups::Tree*)memory.Allocate(scaffold.count*sizeof(RenderMeshGroups::Tree));if(!groupTrees)return Fail("Global tree allocation failed");
  unsigned int nodeCount=0;for(unsigned int t=0;t<scaffold.count;t++){const RenderTrees::Tree& tree=scaffold.trees[t];if(tree.count>65536-nodeCount||(tree.count&&!tree.nodes))return Fail("Global node bound");groupTrees[t].nodeCount=tree.count;nodeCount+=tree.count;for(unsigned int n=0;n<tree.count;n++)if(tree.nodes[n].count)return Fail("Global scaffold contains branches");}
  unsigned int totalVertices=0,totalIndices=0;for(unsigned int i=0;i<count;i++){const RenderPackedRanges::Input& r=input[i].range;if(input[i].node>=nodeCount||input[i].category>3||r.triangles>100000||!RenderPackedLimits::Fits(r.vertexCount,totalVertices,RenderPackedLimits::MaximumVertices)||!RenderPackedLimits::Fits(r.triangles*3,totalIndices,RenderPackedLimits::MaximumIndices))return Fail("Invalid global section membership/bound");totalVertices+=r.vertexCount;totalIndices+=r.triangles*3;}
  RenderMeshGroups::Builder groups;if(!groups.Build(groupTrees,scaffold.count,mainTree,regions,regionCount))return Fail(groups.error);if(groups.count>4096)return Fail("Global mesh count bound");meshCount=groups.count;
  unsigned int* keys=(unsigned int*)memory.Allocate(count*4);unsigned int* meshBySource=(unsigned int*)memory.Allocate(count*4);unsigned int* surfaceBySource=(unsigned int*)memory.Allocate(count*4);unsigned int* meshSurfaces=(unsigned int*)memory.Allocate(meshCount*4);
  meshes=(Mesh*)memory.Allocate(meshCount*sizeof(Mesh));if(!keys||!meshBySource||!surfaceBySource||!meshSurfaces||!meshes)return Fail("Global mesh metadata allocation failed");
  for(unsigned int i=0;i<count;i++){keys[i]=input[i].range.orderKey;meshBySource[i]=surfaceBySource[i]=0xffffffffu;}
  if(mainGraph){if(mainGraph->error||mainGraph->count!=scaffold.trees[mainTree].count)return Fail("Global portal graph node mismatch");RenderGlobalSectionOrder::Builder ordering;if(count&&!ordering.Apply(input,count,groups.mainBegin,*mainGraph,keys))return Fail(ordering.error);}
  unsigned int worldBytes=40,materialCount=0;
  for(unsigned int m=0;m<meshCount;m++){
   const RenderMeshGroups::Group& group=groups.groups[m];RenderPackedRanges::Input* ranges=(RenderPackedRanges::Input*)memory.Allocate(count*sizeof(RenderPackedRanges::Input));unsigned int* source=(unsigned int*)memory.Allocate(count*4);if(!ranges||!source)return Fail("Grouped sections allocation failed");
   unsigned int sectionCount=0;for(unsigned int n=0;n<group.count;n++)for(unsigned int i=0;i<count;i++)if(input[i].node==group.nodes[n]){ranges[sectionCount]=input[i].range;ranges[sectionCount].orderKey=keys[i];source[sectionCount++]=i;}
   Mesh& mesh=meshes[m];mesh.regionIndex=group.regionIndex;mesh.name=mesh.filename=0;mesh.external=0;mesh.externalBytes=0;void* placement=memory.Allocate(sizeof(RenderPackedMesh::Block));if(!placement)return Fail("Grouped mesh allocation failed");mesh.geometry=new(placement)RenderPackedMesh::Block();initializedMeshes++;
   if(!mesh.geometry->Build(ranges,sectionCount,sort))return Fail(mesh.geometry->error);meshSurfaces[m]=sectionCount;materialCount+=mesh.geometry->materials;
   for(unsigned int s=0;s<sectionCount;s++){unsigned int original=source[mesh.geometry->order[s]];if(meshBySource[original]!=0xffffffffu)return Fail("Section assigned to multiple mesh groups");meshBySource[original]=m;surfaceBySource[original]=s;}
   if(m==0)worldBytes+=4+mesh.geometry->size;
   else {
    if(!Filename(worldStem,group.name,projectRoot,mesh))return Fail("Invalid external mesh filename");for(unsigned int previous=1;previous<m;previous++)if(!RenderPackedRanges::CompareNames(mesh.filename,meshes[previous].filename))return Fail("Duplicate external mesh filename unsupported");
    // External 4F1C0 writes only 4E880 geometry. Material strings belong to
    // the world reference entry; they are not repeated in Mesh00p.
    mesh.externalBytes=28+mesh.geometry->namesOffset;mesh.external=(unsigned char*)memory.Allocate(mesh.externalBytes);if(!mesh.external)return Fail("External mesh bytes allocation failed");
    unsigned int h[7]={0x4853454du,1,1,sectionCount,mesh.geometry->definitions,mesh.geometry->definitionElements,((mesh.geometry->vertexBytes+3)&~3u)+((mesh.geometry->indexBytes+3)&~3u)};memcpy(mesh.external,h,28);memcpy(mesh.external+28,mesh.geometry->data,mesh.geometry->namesOffset);
    worldBytes+=14+lstrlenA(mesh.name)+mesh.geometry->size-mesh.geometry->namesOffset;
   }
  }
  for(unsigned int i=0;i<count;i++)if(meshBySource[i]==0xffffffffu)return Fail("Global source section not assigned");
  RenderTrees::Tree* trees=(RenderTrees::Tree*)memory.Allocate(scaffold.count*sizeof(RenderTrees::Tree));RenderTrees::Node* nodes=(RenderTrees::Node*)memory.Allocate(nodeCount*sizeof(RenderTrees::Node));RenderTrees::Branch* branches=(RenderTrees::Branch*)memory.Allocate(count*sizeof(RenderTrees::Branch));if(!trees||!nodes||!branches)return Fail("Global topology allocation failed");
  unsigned int at=0;for(unsigned int t=0;t<scaffold.count;t++){trees[t].nodes=nodes+at;trees[t].count=scaffold.trees[t].count;for(unsigned int n=0;n<trees[t].count;n++)nodes[at++]=scaffold.trees[t].nodes[n];}
  at=0;for(unsigned int n=0;n<nodeCount;n++){RenderTrees::Node& node=nodes[n];node.branches=branches+at;node.count=0;for(unsigned int i=0;i<count;i++)if(input[i].node==n){const RenderPackedRanges::Input& range=input[i].range;RenderTrees::Branch& branch=branches[at++];branch.meshIndex=meshBySource[i];branch.surface=surfaceBySource[i];branch.type=input[i].category==3?0:input[i].category+1;if(range.vertexCount)branch.low=branch.high=range.vertices[0].position;else RenderTrees::EmptyBounds(branch);for(unsigned int v=0;v<range.vertexCount;v++){const WorldText::Vec3& p=range.vertices[v].position;if(!WorldText::FiniteVector(p))return Fail("Invalid global branch position");if(p.x<branch.low.x)branch.low.x=p.x;if(p.y<branch.low.y)branch.low.y=p.y;if(p.z<branch.low.z)branch.low.z=p.z;if(p.x>branch.high.x)branch.high.x=p.x;if(p.y>branch.high.y)branch.high.y=p.y;if(p.z>branch.high.z)branch.high.z=p.z;}node.count++;}}
  RenderTrees::Layout layout={trees,scaffold.count};RenderTrees::Block topology;if(at!=count||!topology.Build(layout,meshSurfaces,meshCount))return Fail("Invalid global topology");
  size=worldBytes+topology.size;if(size>32*1024*1024)return Fail("Global render bytes bound");data=(unsigned char*)memory.Allocate(size);if(!data)return Fail("Global render allocation failed");
  unsigned int h[10]={scaffold.count,nodeCount,count,meshCount,materialCount,1,meshes[0].geometry->count,topology.skyPortals,topology.skyPortalVertices,mainTree};if(!Put(h,40))return Fail("Global render header overflow");
  for(unsigned int m=0;m<meshCount;m++){const Mesh& mesh=meshes[m];if(!U32(m?1:0))return Fail("Global mesh kind overflow");if(!m){if(!Put(mesh.geometry->data,mesh.geometry->size))return Fail("Inline mesh overflow");}else {unsigned short length=(unsigned short)lstrlenA(mesh.name);if(!U32(mesh.geometry->materials)||!Put(&length,2)||!Put(mesh.name,length)||!U32(0)||!Put(mesh.geometry->data+mesh.geometry->namesOffset,mesh.geometry->size-mesh.geometry->namesOffset))return Fail("Referenced mesh overflow");}}
  if(!Put(topology.data,topology.size)||cursor!=size)return Fail("Global tree overflow");return true;
 }
};
}
