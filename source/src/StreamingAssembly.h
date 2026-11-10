#pragma once
#include "StreamingConvex.h"
#include "StreamingTreeBuilder.h"
#include "StreamingSourceMetadata.h"
namespace StreamingAssembly {
class Block {
 PrefabScene::Arena memory;
public:
 unsigned char* data;unsigned int size;StreamingConvex::Block convex;StreamingTreeBuilder::Builder tree;StreamingTables::Tables tables;StreamingSourceMetadata::Builder metadata;
 Block():data(0),size(0){}
 bool Build(const PrefabScene::Model& source,const StreamingResources::Sets& resources,unsigned int regionCount,unsigned int globalBudget,unsigned int streamingBudget,int selected,unsigned int visibilitySectorCount=0,const StreamingMetadata::External* sectors=0,const WorldText::Vec3* sectorOffset=0){
  memory.Clear();data=0;size=0;metadata.assignment.Clear();
  if(!convex.Build(source)||regionCount!=convex.regionCount||selected< -1||(selected>=0&&(unsigned int)selected>=convex.volumeCount))return false;
  const StreamingTables::Region* regions=resources.regions;
  if(sectors||sectorOffset){
   if(!sectorOffset||(visibilitySectorCount&&!sectors)||(regionCount&&!regions)||!metadata.Build(convex,sectors,visibilitySectorCount,*sectorOffset))return false;
   StreamingTables::Region* assigned=(StreamingTables::Region*)memory.Allocate((regionCount+1)*sizeof(StreamingTables::Region));if(!assigned)return false;
   for(unsigned int i=0;i<regionCount;i++){assigned[i]=regions[i];assigned[i].references=metadata.assignment.regions[i].indices;assigned[i].referenceCount=metadata.assignment.regions[i].count;}regions=assigned;
  }
  if(!tables.Build(resources.global,resources.globalCount,regions,regionCount,visibilitySectorCount))return false;
  unsigned int treeBytes=0,treeNodes=0;if(convex.volumeCount){if(!tree.Build(convex.bounds,convex.volumeCount))return false;treeBytes=tree.block.size;treeNodes=tree.nodeCount;}
  if(tables.size>16*1024*1024-44||convex.size>16*1024*1024-44-tables.size||treeBytes>16*1024*1024-44-tables.size-convex.size)return false;
  size=44+tables.size+convex.size+treeBytes;data=(unsigned char*)memory.Allocate(size);if(!data)return false;
  unsigned int header[]={tables.poolBytes,globalBudget,streamingBudget,regionCount,tables.totalAssetReferences,convex.volumeCount,convex.polygonCount,convex.vertexCount,treeNodes,(unsigned int)selected,tables.aggregate};
  unsigned int cursor=0;const unsigned char* parts[]={(const unsigned char*)header,tables.data,convex.data,tree.block.data};unsigned int lengths[]={44,tables.size,convex.size,treeBytes};
  for(unsigned int k=0;k<4;k++)for(unsigned int j=0;j<lengths[k];j++)data[cursor++]=parts[k][j];return cursor==size;
 }
};
// Geometry and tree come from source. Resource seeds, budgets and selected region
// are explicit inputs; this does not implement shader/game dependency callbacks.
}

