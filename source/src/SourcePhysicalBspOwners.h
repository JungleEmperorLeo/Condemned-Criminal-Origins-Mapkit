#pragma once
#include "SourcePhysicalBspDisk.h"
namespace SourcePhysicalBspOwners {
struct Model {Model* next;unsigned int group,ownerNode,sourceContours,sourceReferences,splits,depth;unsigned char* data;unsigned int size;char name[256];};
class Builder {
 PrefabScene::Arena memory;
 Builder(const Builder&);Builder& operator=(const Builder&);
 bool Fail(const char* reason,unsigned int group){char saved[512];lstrcpynA(saved,reason?reason:"Source owner BSP compilation failed",512);Clear();lstrcpyA(diagnostic,saved);error=diagnostic;failedGroup=group;return false;}
public:
 Model* first;Model* last;WorldText::Vec3* normals;unsigned int normalCount,count,emptyCount,aliases,omittedGroups,materials,failedGroup;const char* error;char diagnostic[512];
 Builder():first(0),last(0),normals(0),normalCount(0),count(0),emptyCount(0),aliases(0),omittedGroups(0),materials(0),failedGroup(0xffffffffu),error(0){diagnostic[0]=0;}
 void Clear(){memory.Clear();first=last=0;normals=0;normalCount=count=emptyCount=aliases=omittedGroups=materials=0;failedGroup=0xffffffffu;error=0;diagnostic[0]=0;}
 bool Build(const PrefabScene::Model& model,const WorldModels::Partition& partition,const WorldText::Vec3& offset,const PrefabScene::Project& project){
  Clear();SourceBlockerPlanePool::Prepared planes;if(!planes.Build(model,partition,offset))return Fail(planes.error,0xffffffffu);
  SourcePhysicalBspNormals::Table global;if(!global.Build(model,partition,offset))return Fail(global.error,0xffffffffu);
  SourcePhysicalBspMetadata::Resolver metadata;if(!metadata.Initialize(project))return Fail(metadata.error,0xffffffffu);
  normals=(WorldText::Vec3*)memory.Allocate((global.count+1)*12);if(!normals)return Fail("Owner global normal table allocation failed",0xffffffffu);normalCount=global.count;memcpy(normals,global.normals,normalCount*12);
  for(unsigned int g=1;g<partition.groupCount;g++){
   if(!SourcePhysicalBspContours::RetainedGroup(model,partition,g)){omittedGroups++;continue;}
   if(!planes.EmitGroup(g)){aliases++;continue;}
   SourcePhysicalBspContours::Main contours;if(!contours.BuildGroup(model,partition,g,planes))return Fail(contours.error,g);
   SourcePhysicalBspDisk::Builder block;if(!block.Build(model,contours,normals,normalCount,metadata,2))return Fail(block.error,g);
   Model* output=(Model*)memory.Allocate(sizeof(Model));if(!output)return Fail("Owner BSP descriptor allocation failed",g);
   output->data=(unsigned char*)memory.Allocate(block.size);if(!output->data)return Fail("Owner BSP payload allocation failed",g);
   memcpy(output->data,block.data,block.size);output->size=block.size;output->group=g;output->ownerNode=partition.groups[g].ownerIndex;output->sourceContours=contours.count;output->sourceReferences=contours.references;output->splits=contours.Tree().splits;output->depth=contours.Tree().maxDepth;
   lstrcpynA(output->name,partition.groups[g].owner->name,256);output->next=0;
   if(last)last->next=output;else first=output;last=output;count++;if(!block.Block().header.polygons&&!block.Block().header.nodes)emptyCount++;
  }
  materials=metadata.Materials();return true;
 }
};
}
