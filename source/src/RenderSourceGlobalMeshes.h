#pragma once
#include "SourceRenderNodes.h"
#include "StreamingSourceMetadata.h"
#include "RenderGlobalMeshes.h"
#include "SourceRenderAllTrees.h"
namespace RenderSourceGlobalMeshes {
// Source adapter. Main surfaces carry final sector SaveIDs; optional owner
// surfaces carry flattened node IDs from SourceRenderAllTrees. This stage derives
// portal order and region membership from source. Additional trees must come
// from the same owned model/visibility builder, never a packed reference world.
class Block {
 PrefabScene::Arena memory;
 SourceRenderNodes::Builder* scaffold;
 StreamingConvex::Block* geometry;
 StreamingSourceMetadata::Builder* metadata;
 RenderVisibilityPortalGraph::Graph* graph;
 RenderGlobalMeshes::Block* render;
 Block(const Block&);Block& operator=(const Block&);
 void ReleaseStages(){if(graph)graph->~Graph();if(metadata)metadata->~Builder();if(geometry)geometry->~Block();if(scaffold)scaffold->~Builder();graph=0;metadata=0;geometry=0;scaffold=0;}
 bool Fail(const char* message,const char* stage){Clear();error=message;errorStage=stage;return false;}
 const char* CopyName(const char* name){if(!name)return 0;unsigned int n=0;while(n<=4096&&name[n])n++;if(!n||n>4096)return 0;char* copy=(char*)memory.Allocate(n+1);if(copy)memcpy(copy,name,n+1);return copy;}
 bool SourceHierarchy(const PrefabScene::Model& model){
  if(!model.count||model.count>100000)return Fail("Invalid source model node count","source hierarchy");
  const PrefabScene::Node** nodes=(const PrefabScene::Node**)memory.Allocate(model.count*sizeof(void*));if(!nodes)return Fail("Source hierarchy allocation failed","allocation");memset(nodes,0,model.count*sizeof(void*));unsigned int seen=0;
  for(const PrefabScene::Node* n=model.first;n;n=n->next){if(seen++>=model.count||n->index>=model.count||nodes[n->index]||(n->parent!=0xffffffffu&&n->parent>=model.count))return Fail("Invalid source node index or parent","source hierarchy");nodes[n->index]=n;}
  if(seen!=model.count)return Fail("Incomplete source node enumeration","source hierarchy");
  // A StreamingLink outside every region is ignored by streaming and deleted with
  // its subtree (0x10036CF0), so its brushes never reach mesh assembly.
  return true;
 }
public:
 unsigned char* data;unsigned int size,meshCount;RenderGlobalMeshes::Mesh* meshes;
 RenderMeshGroups::Region* regions;unsigned int regionCount,nodeCount,unassigned,ambiguous,skyPortalCount;
 unsigned int* constructionIndexBySaveID;int* ownersByConstruction;WorldText::Vec3 offset;bool splitMeshes;
 const char* error;const char* errorStage;
 Block():scaffold(0),geometry(0),metadata(0),graph(0),render(0),data(0),size(0),meshCount(0),meshes(0),regions(0),regionCount(0),nodeCount(0),unassigned(0),ambiguous(0),skyPortalCount(0),constructionIndexBySaveID(0),ownersByConstruction(0),splitMeshes(false),error(0),errorStage(0){offset.x=offset.y=offset.z=0;}
 ~Block(){Clear();}
 void Clear(){
  if(render)render->~Block();ReleaseStages();
  memory.Clear();scaffold=0;geometry=0;metadata=0;graph=0;render=0;data=0;meshes=0;regions=0;constructionIndexBySaveID=0;ownersByConstruction=0;
  size=meshCount=regionCount=nodeCount=unassigned=ambiguous=skyPortalCount=0;offset.x=offset.y=offset.z=0;splitMeshes=false;error=errorStage=0;
 }
 bool Build(const PrefabScene::Model& model,const VisibilityCompiler::Builder& visibility,const RenderPackedSection::Input* input,unsigned int count,const WorldText::Vec3& inputOffset,bool split,const char* worldStem,const char* projectRoot,const SourceSkyPortals::Builder* sky=0,bool sort=true,const SourceRenderAllTrees::Builder* allTrees=0){
  Clear();
  if(!SourceHierarchy(model))return false;
  if(visibility.errorCode||!visibility.data||!visibility.size||(!visibility.streamingSectors&&visibility.sectorCount)||visibility.sectorCount>65536)return Fail("Source visibility preparation failed","visibility");
  if(!WorldText::FiniteVector(inputOffset)||!WorldText::FiniteVector(visibility.offset)||inputOffset.x!=visibility.offset.x||inputOffset.y!=visibility.offset.y||inputOffset.z!=visibility.offset.z)return Fail("Packed surface/source visibility center mismatch","packed sections");
  if(count>RenderPackedRanges::MaximumSections||(!input&&count))return Fail("Invalid source packed surface count","packed sections");
  unsigned int renderNodes=visibility.sectorCount;
  if(allTrees){
   if(allTrees->error||allTrees->sourceModel!=&model||allTrees->sourceVisibility!=&visibility||allTrees->mainNodeCount!=visibility.sectorCount||allTrees->totalNodeCount<visibility.sectorCount||allTrees->totalNodeCount>65536||!allTrees->layout.trees||!allTrees->layout.count||allTrees->layout.trees[0].count!=visibility.sectorCount)return Fail("WorldModel/source scaffold mismatch","render scaffold");
   renderNodes=allTrees->totalNodeCount;
  }
  for(unsigned int i=0;i<count;i++)if(input[i].node>=renderNodes)return Fail("Packed surface node outside prepared source scaffold","packed sections");
  offset=visibility.offset;splitMeshes=split;nodeCount=visibility.sectorCount;
  void* stageStorage=memory.Allocate(sizeof(SourceRenderNodes::Builder));if(!stageStorage)return Fail("Source scaffold allocation failed","allocation");scaffold=new(stageStorage)SourceRenderNodes::Builder();
  if(!scaffold->Build(visibility,sky))return Fail(scaffold->error,"render scaffold");
  constructionIndexBySaveID=(unsigned int*)memory.Allocate((nodeCount+1)*4);ownersByConstruction=(int*)memory.Allocate((nodeCount+1)*4);if(!constructionIndexBySaveID||!ownersByConstruction)return Fail("Source node metadata allocation failed","allocation");
  for(unsigned int n=0;n<nodeCount;n++)constructionIndexBySaveID[n]=scaffold->sectors[n].constructionIndex;skyPortalCount=scaffold->skyPortalCount;
  // Region table of the original stage: ignored and removed regions are absent.
  StreamingStructure::Builder table;if(!table.Build(model,visibility.offset)||table.fatal)return Fail("Invalid source streaming regions","region geometry");
  regionCount=table.TableCount();if(regionCount>4096)return Fail("Source region count bound","region geometry");
  stageStorage=memory.Allocate(sizeof(StreamingConvex::Block));if(!stageStorage)return Fail("Region geometry allocation failed","allocation");geometry=new(stageStorage)StreamingConvex::Block();
  if(!geometry->Build(model,&visibility.offset))return Fail(geometry->error,"region geometry");if(geometry->regionCount!=regionCount)return Fail("Source region enumeration mismatch","region geometry");
  stageStorage=memory.Allocate(sizeof(StreamingSourceMetadata::Builder));if(!stageStorage)return Fail("Region assignment allocation failed","allocation");metadata=new(stageStorage)StreamingSourceMetadata::Builder();
  if(!metadata->Build(*geometry,visibility.streamingSectors,nodeCount,visibility.offset))return Fail(metadata->error,"region assignment");
  unassigned=metadata->assignment.unassigned;ambiguous=metadata->assignment.ambiguous;for(unsigned int n=0;n<nodeCount;n++)ownersByConstruction[n]=metadata->assignment.owners[n];
  regions=(RenderMeshGroups::Region*)memory.Allocate((regionCount+1)*sizeof(RenderMeshGroups::Region));if(!regions)return Fail("Source render region allocation failed","allocation");unsigned int at=0;
  for(unsigned int r=0;r<table.regionCount;r++)if(!table.regions[r].removed){const PrefabScene::Node* n=table.regions[r].node;
   RenderMeshGroups::Region& region=regions[at];region.name=CopyName(n->name);if(!region.name)return Fail("Empty or oversized source region name","region assignment");const StreamingMetadata::References& source=metadata->assignment.regions[at++];region.count=source.count;
   unsigned int* ids=(unsigned int*)memory.Allocate((source.count+1)*4);if(!ids)return Fail("Source render region SaveID allocation failed","allocation");if(source.count)memcpy(ids,source.indices,source.count*4);region.sectors=ids;
   for(unsigned int i=0;i<region.count;i++)if(region.sectors[i]>=nodeCount)return Fail("Source region SaveID outside prepared scaffold","region assignment");
  }
  stageStorage=memory.Allocate(sizeof(RenderVisibilityPortalGraph::Graph));if(!stageStorage)return Fail("Source portal graph allocation failed","allocation");graph=new(stageStorage)RenderVisibilityPortalGraph::Graph();
  if(!graph->Build(visibility.data,visibility.size))return Fail(graph->error,"source portal graph");
  stageStorage=memory.Allocate(sizeof(RenderGlobalMeshes::Block));if(!stageStorage)return Fail("Source global render allocation failed","allocation");render=new(stageStorage)RenderGlobalMeshes::Block();
  const RenderTrees::Layout& layout=allTrees?allTrees->layout:scaffold->layout;
  if(!render->Build(input,count,layout,0,split?regions:0,split?regionCount:0,worldStem,sort,graph,projectRoot))return Fail(render->error,"global render writer");
  data=render->data;size=render->size;meshCount=render->meshCount;meshes=render->meshes;ReleaseStages();return true;
 }
};
}
