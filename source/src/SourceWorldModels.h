#pragma once
#include "MsvcSort.h"
#include "SourcePhysicalBspDisk.h"
#include "WorldModelsSerialization.h"
// Source-owned complete WorldModels payload (between world tree and blockers).
// Geometry, global normals, names/aliases and numeric BSP IDs come from one scene.
namespace SourceWorldModels {
struct Settings {
 unsigned int depthLimit,fragmentLimit,pointLimit;
 Settings():depthLimit(1500),fragmentLimit(1000000),pointLimit(10000000){} /* CONDEMNED FORK: raised from 256/100000/1000000 */
};
class Builder {
 WorldModelsSerialization::Block output;PrefabScene::Arena statistics;
 Builder(const Builder&);Builder& operator=(const Builder&);
 struct Disks {
  PrefabScene::Arena memory;SourcePhysicalBspDisk::Builder** values;unsigned int count;
  Disks():values(0),count(0){}
  ~Disks(){for(unsigned int i=0;i<count;i++)values[i]->~Builder();}
  bool Initialize(unsigned int maximum){values=(SourcePhysicalBspDisk::Builder**)memory.Allocate((maximum+1)*sizeof(void*));return values!=0;}
  SourcePhysicalBspDisk::Builder* Add(){void* place=memory.Allocate(sizeof(SourcePhysicalBspDisk::Builder));if(!place)return 0;SourcePhysicalBspDisk::Builder* value=new(place)SourcePhysicalBspDisk::Builder;values[count++]=value;return value;}
 };
 bool Fail(const char* message,unsigned int group){char saved[512];lstrcpynA(saved,message?message:"Source WorldModels compilation failed",512);Clear();lstrcpyA(diagnostic,saved);error=diagnostic;failedGroup=group;return false;}
 static int NameCompare(const char* a,const char* b){for(;;a++,b++){unsigned char x=(unsigned char)*a,y=(unsigned char)*b;if(x>='A'&&x<='Z')x+=32;if(y>='A'&&y<='Z')y+=32;if(x!=y)return x<y?-1:1;if(!x)return 0;}}
 struct NameLess {bool operator()(const WorldModelsSerialization::Name& a,const WorldModelsSerialization::Name& b)const{return NameCompare(a.text,b.text)<0;}};
public:
 unsigned char* data;unsigned int size,modelCount,nameCount,normalCount,emptyCount,materials,failedGroup;const char* error;char diagnostic[512];
 // Packer log statistics (no effect on the section): global normal map size with
 // the 18 seeds, BSP block owner names in build order, internal BSP polygons.
 unsigned int initialNormalCount,internalPolygons,mainInternalPolygons;const char** blockNames;unsigned int* blockBrushes;unsigned int** blockProgress;unsigned int* blockProgressCount;unsigned int* blockPolygons;WorldText::Vec3** blockBadPoints;unsigned int* blockBadCount;
 Builder():data(0),size(0),modelCount(0),nameCount(0),normalCount(0),emptyCount(0),materials(0),failedGroup(0xffffffffu),error(0),initialNormalCount(0),internalPolygons(0),mainInternalPolygons(0),blockNames(0),blockBrushes(0),blockProgress(0),blockProgressCount(0),blockPolygons(0),blockBadPoints(0),blockBadCount(0){diagnostic[0]=0;}
 void Clear(){output.Clear();statistics.Clear();data=0;size=modelCount=nameCount=normalCount=emptyCount=materials=initialNormalCount=internalPolygons=mainInternalPolygons=0;blockNames=0;blockBrushes=0;blockProgress=0;blockProgressCount=0;blockPolygons=0;blockBadPoints=0;blockBadCount=0;failedGroup=0xffffffffu;error=0;diagnostic[0]=0;}
 const WorldModelsSerialization::Block& Block()const{return output;}
 bool Build(const PrefabScene::Model& model,const WorldModels::Partition& partition,const WorldText::Vec3& offset,const PrefabScene::Project& project,const Settings& settings=Settings()){
  Clear();if(!settings.depthLimit||settings.depthLimit>4096||!settings.fragmentLimit||settings.fragmentLimit>1000000||!settings.pointLimit||settings.pointLimit>10000000)return Fail("Invalid source WorldModels capacity settings",0xffffffffu);
  SourceBlockerPlanePool::Prepared planes;if(!planes.Build(model,partition,offset))return Fail(planes.error,0xffffffffu);
  SourcePhysicalBspNormals::Table global;if(!global.Build(model,partition,offset))return Fail(global.error,0xffffffffu);
  SourcePhysicalBspMetadata::Resolver metadata;if(!metadata.Initialize(project))return Fail(metadata.error,0xffffffffu);
  WorldModelSharing::Identity sharing;if(!sharing.Build(model,partition))return Fail("Source WorldModels identity preparation failed",0xffffffffu);
  Disks disks;if(!disks.Initialize(partition.groupCount))return Fail("Source WorldModels block storage allocation failed",0xffffffffu);
  const PhysicalBspSerialization::Block** blocks=(const PhysicalBspSerialization::Block**)disks.memory.Allocate((partition.groupCount+1)*sizeof(void*));
  WorldModelsSerialization::Name* names=(WorldModelsSerialization::Name*)disks.memory.Allocate((partition.groupCount+1)*sizeof(WorldModelsSerialization::Name));
  unsigned int* modelIndex=(unsigned int*)disks.memory.Allocate((partition.groupCount+1)*4);if(!blocks||!names||!modelIndex)return Fail("Source WorldModels tables allocation failed",0xffffffffu);
  unsigned int used=0,named=0,empty=0,*brushStamp=0;
  for(unsigned int g=0;g<partition.groupCount;g++){
   modelIndex[g]=0xffffffffu;if(g&&!SourcePhysicalBspContours::RetainedGroup(model,partition,g))continue;
   const unsigned int representative=sharing.representative[g];
   if(representative==g){
    SourcePhysicalBspContours::Main contours;if(!contours.BuildGroup(model,partition,g,planes,settings.depthLimit,settings.fragmentLimit,settings.pointLimit))return Fail(contours.error,g);
    SourcePhysicalBspDisk::Builder* disk=disks.Add();if(!disk)return Fail("Source WorldModels disk allocation failed",g);
    if(!disk->Build(model,contours,global.normals,global.count,metadata,g?2u:1u))return Fail(disk->error,g);
    if(!blockNames){blockNames=(const char**)statistics.Allocate((partition.groupCount+1)*sizeof(char*));blockBrushes=(unsigned int*)statistics.Allocate((partition.groupCount+1)*4);blockProgress=(unsigned int**)statistics.Allocate((partition.groupCount+1)*sizeof(void*));blockProgressCount=(unsigned int*)statistics.Allocate((partition.groupCount+1)*4);blockBadPoints=(WorldText::Vec3**)statistics.Allocate((partition.groupCount+1)*sizeof(void*));blockBadCount=(unsigned int*)statistics.Allocate((partition.groupCount+1)*4);blockPolygons=(unsigned int*)statistics.Allocate((partition.groupCount+1)*4);}if(!blockNames||!blockBrushes||!blockProgress||!blockProgressCount||!blockPolygons)return Fail("Source WorldModels statistics allocation failed",g);
    {const FearBspTree::Builder& tree=contours.Tree();blockProgressCount[used]=tree.progressCount;blockProgress[used]=(unsigned int*)statistics.Allocate((tree.progressCount+1)*4);if(!blockProgress[used])return Fail("Source WorldModels statistics allocation failed",g);for(unsigned int k=0;k<tree.progressCount;k++)blockProgress[used][k]=tree.progressTotals[k];
     if(blockBadPoints&&blockBadCount){blockBadCount[used]=tree.badCount;blockBadPoints[used]=(WorldText::Vec3*)statistics.Allocate((tree.badCount+1)*sizeof(WorldText::Vec3));if(!blockBadPoints[used])return Fail("Source WorldModels statistics allocation failed",g);for(unsigned int k=0;k<tree.badCount;k++)blockBadPoints[used][k]=tree.badPoints[k];}}
    // Brushes transferred into this BSP (progress of "Converting Brushes", 0x100080C0).
    {if(!brushStamp){brushStamp=(unsigned int*)statistics.Allocate((model.count+1)*4);if(!brushStamp)return Fail("Source WorldModels statistics allocation failed",g);memset(brushStamp,0,(model.count+1)*4);}
     unsigned int brushes=0;for(unsigned int c=0;c<contours.count;c++){unsigned int node=contours.contours[c].sourceNode;if(node<model.count&&brushStamp[node]!=used+1){brushStamp[node]=used+1;brushes++;}}blockBrushes[used]=brushes;}{const PrefabScene::Node* owner=g?partition.groups[g].owner:0;blockNames[used]=owner?owner->name+(owner->localName<lstrlenA(owner->name)?owner->localName:0):"PhysicsBSP";}blockPolygons[used]=disk->collisionPolygons;internalPolygons+=disk->materialPolygons;if(!g)mainInternalPolygons=disk->collisionPolygons;
    modelIndex[g]=used;blocks[used++]=&disk->Block();if(!disk->Block().header.polygons&&!disk->Block().header.nodes)empty++;
   }else{if(representative>=g||modelIndex[representative]==0xffffffffu)return Fail("Source WorldModels alias has no representative",g);modelIndex[g]=modelIndex[representative];}
   const char* name=g?partition.groups[g].owner->name:"PhysicsBSP";unsigned int length=0;while(length<256&&name[length])length++;if(!length||length==256)return Fail("Invalid source WorldModels name",g);
   names[named].text=name;names[named++].model=modelIndex[g];
  }
  // Disk names are alphabetic/case-insensitive, independent of BSP emission
  // order. Aliases retain their representative index through this permutation.
  // Hypothesis under test: group-order input sorted by MSVC std::sort (unstable),
  // explaining SpillKill graf_2601 59-before-24 and Refinery 5-before-45.
  MsvcSort::Sort(names,named,NameLess());
  if(!output.Build(names,named,global.normals,global.count,blocks,used))return Fail(output.error,0xffffffffu);
 initialNormalCount=global.initialCount;data=output.data;size=output.size;modelCount=used;nameCount=named;normalCount=global.count;emptyCount=empty;materials=metadata.Materials();return true;
 }
};
}
