#pragma once
#include "SourcePhysicalBspDisk.h"
// MAIN physical BSP disk block, generated from an explicitly prepared scene.
// Its accompanying ordered normal table is source-owned and byte-exact on the
// verified profiles. Owner/SP BSPs remain separate WorldModels compiler work.
namespace SourcePhysicalBspMain {
class Builder {
 PrefabScene::Arena memory;SourcePhysicalBspDisk::Builder* output;
 Builder(const Builder&);Builder& operator=(const Builder&);
 bool Fail(const char* reason){char saved[512];lstrcpynA(saved,reason?reason:"Source MAIN BSP compilation failed",512);Clear();lstrcpyA(diagnostic,saved);error=diagnostic;return false;}
public:
 unsigned char* data;unsigned int size,normalCount,materials;WorldText::Vec3* normalTable;const char* error;char diagnostic[512];
  Builder():output(0),data(0),size(0),normalCount(0),materials(0),normalTable(0),error(0){diagnostic[0]=0;}
  ~Builder(){Clear();}
  void Clear(){if(output){output->~Builder();output=0;}memory.Clear();data=0;size=normalCount=materials=0;normalTable=0;error=0;diagnostic[0]=0;}
  __declspec(noinline) bool Build(const PrefabScene::Model& model,const WorldModels::Partition& partition,const WorldText::Vec3& offset,const PrefabScene::Project& project){
  Clear();SourcePhysicalBspContours::Main source;if(!source.Build(model,partition,offset))return Fail(source.error);
  SourcePhysicalBspNormals::Table planes;if(!planes.Build(model,partition,offset))return Fail(planes.error);
  SourcePhysicalBspMetadata::Resolver metadata;if(!metadata.Initialize(project))return Fail(metadata.error);
  normalCount=planes.count;normalTable=(WorldText::Vec3*)memory.Allocate(normalCount*12);if(!normalTable)return Fail("MAIN normal table allocation failed");memcpy(normalTable,planes.normals,normalCount*12);
  void* diskStorage=memory.Allocate(sizeof(SourcePhysicalBspDisk::Builder));if(!diskStorage)return Fail("MAIN disk compiler allocation failed");output=new(diskStorage)SourcePhysicalBspDisk::Builder;
  if(!output->Build(model,source,normalTable,normalCount,metadata,1))return Fail(output->error);data=output->data;size=output->size;materials=metadata.Materials();return true;
 }
};
}
