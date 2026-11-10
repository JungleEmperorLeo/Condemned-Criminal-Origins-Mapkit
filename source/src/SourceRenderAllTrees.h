#pragma once
#include "SourceRenderNodes.h"
#include "SourceExpandedRenderContours.h"
#include "WorldModelSharing.h"
namespace SourceRenderAllTrees {
// Pure source scaffold. Model/visibility identities remain caller-owned;
// topology, main portals and mapping arrays remain owned by this builder.
// Geometry branches are filled later by the global packed-section writer.
class Builder {
 PrefabScene::Arena memory;
 SourceRenderNodes::Builder mainStage;
 Builder(const Builder&);Builder& operator=(const Builder&);
 bool Fail(const char* message){Clear();error=message;return false;}
 static bool Sector(const PrefabScene::Node& node){return FearBrushFlags::Equal(node.type,"Sector")||FearBrushFlags::Equal(node.type,"ExtrudeSector")||FearBrushFlags::Equal(node.type,"SemiExtrudeSector");}
public:
 const PrefabScene::Model* sourceModel;
 const VisibilityCompiler::Builder* sourceVisibility;
 unsigned int mainNodeCount,totalNodeCount,groupCount,mainTree,aliasCount;
 unsigned int* groupTree;unsigned int* groupNode;unsigned int* groupRepresentative;
 RenderTrees::Layout layout;const char* error;
 Builder():sourceModel(0),sourceVisibility(0),mainNodeCount(0),totalNodeCount(0),groupCount(0),mainTree(0),aliasCount(0),groupTree(0),groupNode(0),groupRepresentative(0),error(0){layout.trees=0;layout.count=0;}
 void Clear(){mainStage.~Builder();new(&mainStage)SourceRenderNodes::Builder();memory.Clear();sourceModel=0;sourceVisibility=0;mainNodeCount=totalNodeCount=groupCount=mainTree=aliasCount=0;groupTree=groupNode=groupRepresentative=0;layout.trees=0;layout.count=0;error=0;}
 // Shared topology is explicit and does not prepare canonical prototype geometry.
 // Full source-render callers retain the guarded default until that stage joins.
 bool Build(const PrefabScene::Model& model,const WorldModels::Partition& partition,const VisibilityCompiler::Builder& visibility,const SourceSkyPortals::Builder* sky=0,bool sharePrototypes=false){
  Clear();SourceExpandedRenderContours::Consumed consumed;
  if(!model.count||model.count>100000||!consumed.Build(model))return Fail(consumed.error?consumed.error:"Invalid all-tree source hierarchy");
  if(partition.error||!partition.groups||!partition.groupCount||partition.groupCount>4096||partition.assignmentCount>model.brushes||(!partition.assignments&&partition.assignmentCount)||partition.ignoredBrushes!=consumed.brushes||partition.assignmentCount+consumed.brushes!=model.brushes)return Fail("All-tree partition/consumed mismatch");
  if(partition.groups[0].owner||partition.groups[0].ownerIndex!=0xffffffffu)return Fail("Invalid all-tree main ownership group");
  const PrefabScene::Node** nodes=(const PrefabScene::Node**)memory.Allocate(model.count*sizeof(void*));
  unsigned int* ownerGroup=(unsigned int*)memory.Allocate(model.count*4);
  unsigned int* assigned=(unsigned int*)memory.Allocate(model.count*4);
  unsigned int* ownerAt=(unsigned int*)memory.Allocate(model.count*4);
  unsigned int* brushCounts=(unsigned int*)memory.Allocate(partition.groupCount*4);
  unsigned int* polygonCounts=(unsigned int*)memory.Allocate(partition.groupCount*4);
  if(!nodes||!ownerGroup||!assigned||!ownerAt||!brushCounts||!polygonCounts)return Fail("All-tree validation allocation failed");
  for(unsigned int n=0;n<model.count;n++){assigned[n]=0xffffffffu;ownerAt[n]=0;}
  memset(brushCounts,0,partition.groupCount*4);memset(polygonCounts,0,partition.groupCount*4);
  for(const PrefabScene::Node* node=model.first;node;node=node->next)nodes[node->index]=node;
  // Group order follows the post-ShadowOverride traversal (WorldModels::Order),
  // which need not be ascending in authored node index.
  for(unsigned int g=1;g<partition.groupCount;g++){
   const WorldModels::Group& group=partition.groups[g];
   if(!group.owner||group.ownerIndex>=model.count||nodes[group.ownerIndex]!=group.owner||group.owner->kind!=2||ownerAt[group.ownerIndex])return Fail("Invalid ordered all-tree WorldModel owner");
   ownerAt[group.ownerIndex]=g;
  }
  for(const PrefabScene::Node* node=model.first;node;node=node->next)ownerGroup[node->index]=ownerAt[node->index]?ownerAt[node->index]:(node->parent==0xffffffffu?0:ownerGroup[node->parent]);
  for(unsigned int a=0;a<partition.assignmentCount;a++){
   const WorldModels::Assignment& assignment=partition.assignments[a];const PrefabScene::Node* brush=assignment.brush;
   if(!brush||brush->index>=model.count||nodes[brush->index]!=brush||brush->kind!=1||!brush->knownType||assignment.group>=partition.groupCount||assigned[brush->index]!=0xffffffffu||consumed.Ignore(*brush)||ownerGroup[brush->index]!=assignment.group)return Fail("Invalid all-tree brush ownership assignment");
   if(assignment.group&&Sector(*brush))return Fail("WorldModel-owned sector preparation not supported");
   if(assignment.group&&FearBrushFlags::Equal(brush->type,"SkyPortal"))return Fail("WorldModel-owned SkyPortal preparation not supported");
   assigned[brush->index]=assignment.group;brushCounts[assignment.group]++;
   if(brush->geometry.polygons>0xffffffffu-polygonCounts[assignment.group])return Fail("All-tree polygon count overflow");polygonCounts[assignment.group]+=brush->geometry.polygons;
  }
  for(const PrefabScene::Node* node=model.first;node;node=node->next)if(node->kind==1&&!consumed.Ignore(*node)&&assigned[node->index]==0xffffffffu)return Fail("All-tree source brush missing from partition");
  unsigned int treeCount=1;
  for(unsigned int g=0;g<partition.groupCount;g++){
   if(brushCounts[g]!=partition.groups[g].brushes||polygonCounts[g]!=partition.groups[g].polygons)return Fail("All-tree partition group counts inconsistent with source");
  }
  WorldModelSharing::Identity sharing;if(!sharing.Build(model,partition))return Fail("All-tree source identity preparation failed");
  for(unsigned int g=1;g<partition.groupCount;g++){
   unsigned int representative=sharing.representative[g];
   if(representative>g||!representative||sharing.representative[representative]!=representative)return Fail("Invalid shared source representative");
   if(representative!=g){
    if(!sharePrototypes)return Fail("WorldModel prototype sharing requires joined source profile");
    if(brushCounts[g]!=brushCounts[representative]||polygonCounts[g]!=polygonCounts[representative])return Fail("Shared source prototype ownership counts disagree");
    aliasCount++;
   }else if(brushCounts[g])treeCount++;
  }
  if(!mainStage.Build(visibility,sky))return Fail(mainStage.error?mainStage.error:"All-tree main source scaffold failed");
  if(treeCount>4096||mainStage.tree.count>65536-(treeCount-1))return Fail("All-tree serialized node bound exceeded");
  RenderTrees::Tree* trees=(RenderTrees::Tree*)memory.Allocate(treeCount*sizeof(RenderTrees::Tree));
  RenderTrees::Node* ownerNodes=(RenderTrees::Node*)memory.Allocate((treeCount-1+1)*sizeof(RenderTrees::Node));
  groupTree=(unsigned int*)memory.Allocate(partition.groupCount*4);groupNode=(unsigned int*)memory.Allocate(partition.groupCount*4);groupRepresentative=(unsigned int*)memory.Allocate(partition.groupCount*4);
  if(!trees||!ownerNodes||!groupTree||!groupNode||!groupRepresentative)return Fail("All-tree source scaffold allocation failed");
  trees[0]=mainStage.tree;groupTree[0]=0;groupNode[0]=0;groupRepresentative[0]=0;unsigned int tree=1;
  for(unsigned int g=1;g<partition.groupCount;g++){
   groupRepresentative[g]=sharing.representative[g];groupTree[g]=groupNode[g]=0xffffffffu;if(!brushCounts[g])continue;
   if(groupRepresentative[g]!=g){unsigned int representative=groupRepresentative[g];if(groupTree[representative]==0xffffffffu||groupNode[representative]==0xffffffffu)return Fail("Shared source representative has no tree");groupTree[g]=groupTree[representative];groupNode[g]=groupNode[representative];continue;}
   RenderTrees::Node& node=ownerNodes[tree-1];node.branches=0;node.count=0;node.skyPortals=0;node.skyPortalCount=0;
   trees[tree].nodes=&node;trees[tree].count=1;groupTree[g]=tree;groupNode[g]=mainStage.tree.count+tree-1;tree++;
  }
  if(tree!=treeCount)return Fail("All-tree enumeration mismatch");
  sourceModel=&model;sourceVisibility=&visibility;groupCount=partition.groupCount;mainNodeCount=mainStage.tree.count;totalNodeCount=mainNodeCount+treeCount-1;mainTree=0;layout.trees=trees;layout.count=treeCount;return true;
 }
};
}
