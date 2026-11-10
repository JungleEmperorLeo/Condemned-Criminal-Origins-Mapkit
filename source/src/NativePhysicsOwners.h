#pragma once
#include "NativePreparedRenderScene.h"
#include "SourcePhysicalBspContours.h"
#include "PhysicsExplicitShapes.h"
#include "NativePhysicsExplicit.h"
#include "NativePhysicsProperties.h"
#include "NativePhysicsAutomaticSphere.h"
#include "NativePhysicsAutomaticHull.h"
#include "NativePhysicsOwnerMeshTransport.h"
#include "PhysicsMeshBlock.h"
// Source-owned non-MAIN records; Mesh bytecode must carry the exact source prefix.
namespace NativePhysicsOwners {
struct Record {Record* next;unsigned int group,modelIndex,type,bytes,children;unsigned char* data;char name[256];};
struct Settings {unsigned int version;bool exactOwnerCoordinates;const NativePhysicsOwnerMeshTransport::Association* meshAssociations;unsigned int meshAssociationCount;Settings():version(1),exactOwnerCoordinates(true),meshAssociations(0),meshAssociationCount(0){}};
class Builder {
 PrefabScene::Arena memory;
 Builder(const Builder&);Builder& operator=(const Builder&);
 bool Fail(const char* message,unsigned int group=0xffffffffu){char saved[512];lstrcpynA(saved,message?message:"Source physics owner compilation failed",512);Clear();lstrcpyA(diagnostic,saved);error=diagnostic;failedGroup=group;return false;}
 static unsigned int BodySize(unsigned int kind){return kind==2?24:kind==3?12:kind==7?36:0;}
  static bool Equal(const void* a,const void* b,unsigned int bytes){const unsigned char* x=(const unsigned char*)a;const unsigned char* y=(const unsigned char*)b;for(unsigned int i=0;i<bytes;i++)if(x[i]!=y[i])return false;return true;}
 static bool Explicit(const PrefabScene::Node& n){return n.kind==2&&(FearBrushFlags::Equal(n.className,"PhysicsShapeOBB")||FearBrushFlags::Equal(n.className,"PhysicsShapeSphere")||FearBrushFlags::Equal(n.className,"PhysicsShapeCapsule"));}
 static bool Put(unsigned char* data,unsigned int capacity,unsigned int& at,const void* source,unsigned int bytes){if(bytes>capacity-at||(!source&&bytes))return false;memcpy(data+at,source,bytes);at+=bytes;return true;}
 static bool U(unsigned char* data,unsigned int capacity,unsigned int& at,unsigned int value){return Put(data,capacity,at,&value,4);}
 static bool AutoOBB(const WorldModels::Partition& partition,unsigned int group,const WorldText::Vec3& offset,float mass,float density,float* body,WorldText::Vec3& translation,bool exact){
  if(!exact)return PhysicsOwnerBounds::BuildGroup(partition,group,mass,density,body,translation,&offset);
  const PrefabScene::Node& owner=*partition.groups[group].owner;PhysicsOwnerBounds::Bounds all,physics;
  for(unsigned int a=0;a<partition.assignmentCount;a++){const WorldModels::Assignment& item=partition.assignments[a];if(item.group!=group)continue;
   const WorldText::Geometry& geometry=item.brush->geometry;
   for(unsigned int v=0;v<geometry.points;v++){if(!PhysicsOwnerBounds::Referenced(geometry,v))continue;WorldText::Vec3 local;if(!SourceOwnerRotation::Local(owner,geometry.vertices[v],offset,local)||!all.Add(local))return false;if((item.brush->surfaceFlags&0x10)&&!physics.Add(local))return false;}
  }
  // Original empty collision bounds are zero initialized. An OBB request still
  // produces its minimum box and a translation relative to the geometry center.
  if(all.count&&!physics.count){WorldText::Vec3 zero={};if(!physics.Add(zero))return false;}
  return PhysicsOwnerBounds::Finish(all,physics,mass,density,body,translation);
 }
public:
  Record* first;Record* last;unsigned char* data;unsigned int size,count,noneCount,obbCount,sphereCount,hullCount,meshCount,compoundCount,explicitCount,unownedExplicitShapes,failedGroup;const char* error;char diagnostic[512];
  Builder():first(0),last(0),data(0),size(0),count(0),noneCount(0),obbCount(0),sphereCount(0),hullCount(0),meshCount(0),compoundCount(0),explicitCount(0),unownedExplicitShapes(0),failedGroup(0xffffffffu),error(0){diagnostic[0]=0;}
  void Clear(){memory.Clear();first=last=0;data=0;size=count=noneCount=obbCount=sphereCount=hullCount=meshCount=compoundCount=explicitCount=unownedExplicitShapes=0;failedGroup=0xffffffffu;error=0;diagnostic[0]=0;}
 bool Build(const NativePreparedRenderScene::Context& scene,const Settings& settings=Settings()){
  Clear();if(!scene.Ready()||settings.version!=1)return Fail("Invalid source physics scene/settings");
   if(settings.meshAssociationCount>4095||(!settings.meshAssociations&&settings.meshAssociationCount))return Fail("Invalid owner Mesh associations");
   unsigned int associationBytes=0;for(unsigned int i=0;i<settings.meshAssociationCount;i++){const NativePhysicsOwnerMeshTransport::Association& a=settings.meshAssociations[i];if(!a.group||!a.modelIndex||a.modelIndex>4095||!a.mesh||!a.code||!a.meshBytes||!a.codeBytes||a.meshBytes>16u*1024u*1024u-associationBytes)return Fail("Invalid owner Mesh association payload");associationBytes+=a.meshBytes;if(a.codeBytes>16u*1024u*1024u-associationBytes)return Fail("Owner Mesh association byte limit");associationBytes+=a.codeBytes;for(unsigned int j=0;j<i;j++)if(settings.meshAssociations[j].group==a.group||settings.meshAssociations[j].modelIndex==a.modelIndex)return Fail("Duplicate owner Mesh association");}
   unsigned char* used=(unsigned char*)memory.Allocate(settings.meshAssociationCount?settings.meshAssociationCount:1);if(!used)return Fail("Owner Mesh association lookup allocation failed");
  const PrefabScene::Model& model=scene.Model();const WorldModels::Partition& partition=scene.Partition();const WorldText::Vec3& offset=scene.Offset();
  WorldModelSharing::Identity sharing;if(!sharing.Build(model,partition))return Fail("Physics owner sharing failed");
  unsigned int* groups=(unsigned int*)memory.Allocate((model.count+1)*4);unsigned int* children=(unsigned int*)memory.Allocate((partition.groupCount+1)*4);unsigned int* owners=(unsigned int*)memory.Allocate((model.count+1)*4);
  if(!groups||!children||!owners)return Fail("Physics owner lookup allocation failed");
  for(unsigned int g=1;g<partition.groupCount;g++)owners[partition.groups[g].ownerIndex]=g;
  for(const PrefabScene::Node* n=model.first;n;n=n->next){groups[n->index]=owners[n->index]?owners[n->index]:(n->parent==0xffffffffu?0:groups[n->parent]);if(Explicit(*n)){if(groups[n->index])children[groups[n->index]]++;else unownedExplicitShapes++;}}
  unsigned int modelIndex=1;
  for(unsigned int g=1;g<partition.groupCount;g++){
   if(!SourcePhysicalBspContours::RetainedGroup(model,partition,g)||sharing.representative[g]!=g)continue;
   const PrefabScene::Node& owner=*partition.groups[g].owner;const NativePhysicsProperties::Mode mode=NativePhysicsProperties::Automatic(owner);
   // Missing authored property uses the no-automatic-shape branch; no class or
   // source/map name is used as a surrogate for a physics mode.
   const bool automatic=mode==NativePhysicsProperties::OBB||mode==NativePhysicsProperties::Sphere||mode==NativePhysicsProperties::Hull||mode==NativePhysicsProperties::Mesh,none=mode==NativePhysicsProperties::None;
   if(!automatic&&!none)return Fail("Unsupported automatic physics mode",g);
   const unsigned int childCount=children[g]+(automatic?1u:0u);if(childCount>100000)return Fail("Physics child count limit",g);
   NativePhysicsAutomaticHull::Builder hull;
   NativePhysicsOwnerMeshPrefix::Builder mesh;PhysicsMeshBlock::Block complete;
   if(mode==NativePhysicsProperties::Hull){float mass=0,density=0;if(!NativePhysicsProperties::MassDensity(owner,true,mass,density)||!hull.Build(model,partition,g,offset,mass,density,childCount>1))return Fail(hull.error?hull.error:"Automatic Hull mass/density preparation failed",g);}
   if(mode==NativePhysicsProperties::Mesh){unsigned int found=settings.meshAssociationCount;for(unsigned int i=0;i<settings.meshAssociationCount;i++)if(settings.meshAssociations[i].group==g&&settings.meshAssociations[i].modelIndex==modelIndex){found=i;break;}if(found==settings.meshAssociationCount)return Fail("Missing source owner Mesh MOPP association",g);const NativePhysicsOwnerMeshTransport::Association& a=settings.meshAssociations[found];float mass=0,density=0;if(!NativePhysicsProperties::MassDensity(owner,true,mass,density)||!mesh.Build(model,partition,g,offset,mass,density))return Fail(mesh.error?mesh.error:"Automatic Mesh mass/density preparation failed",g);if(a.meshBytes!=mesh.size||!Equal(a.mesh,mesh.data,mesh.size))return Fail("Owner Mesh MOPP source prefix mismatch",g);if(!complete.Build(mesh.Prefix(),a.code,a.codeBytes))return Fail(complete.error,g);used[found]=1;}
   const unsigned int overhead=36+childCount*72;if(hull.size>16u*1024u*1024u-overhead||complete.size>16u*1024u*1024u-overhead-hull.size)return Fail("Physics owner payload byte limit",g);
   const unsigned int capacity=overhead+hull.size+complete.size;unsigned char* output=(unsigned char*)memory.Allocate(capacity);Record* record=(Record*)memory.Allocate(sizeof(Record));if(!output||!record)return Fail("Physics owner payload allocation failed",g);
   unsigned int at=0;float identity[7]={0,0,0,0,0,0,1};
   if(!childCount){if(!Put(output,capacity,at,identity,28)||!U(output,capacity,at,0))return Fail("Physics NONE write failed",g);noneCount++;}
   else if(childCount>1){if(!Put(output,capacity,at,identity,28)||!U(output,capacity,at,6)||!U(output,capacity,at,childCount))return Fail("Physics compound write failed",g);compoundCount++;}
   if(mode==NativePhysicsProperties::Hull){if(!Put(output,capacity,at,hull.data,hull.size))return Fail("Automatic Hull record write failed",g);hullCount++;}
   else if(mode==NativePhysicsProperties::Mesh){if(childCount>1&&!U(output,capacity,at,5))return Fail("Physics Mesh wrapper write failed",g);if(!Put(output,capacity,at,complete.data+4,complete.size-4))return Fail("Automatic Mesh record write failed",g);meshCount++;}
   else if(automatic){float mass=0,density=0,body[6];WorldText::Vec3 translation;unsigned int kind=mode==NativePhysicsProperties::Sphere?3u:2u;if(!NativePhysicsProperties::MassDensity(owner,true,mass,density))return Fail("Automatic physics mass/density preparation failed",g);
    if(kind==3){NativePhysicsAutomaticSphere::Builder sphere;if(!sphere.Build(model,partition,g,offset,mass,density))return Fail(sphere.error,g);memcpy(body,sphere.body,12);translation=sphere.translation;}
    else if(!AutoOBB(partition,g,offset,mass,density,body,translation,settings.exactOwnerCoordinates))return Fail("Automatic OBB preparation failed",g);
    float transform[7]={translation.x,translation.y,translation.z,0,0,0,1};if(childCount>1&&!U(output,capacity,at,5))return Fail("Physics automatic wrapper failed",g);if(!Put(output,capacity,at,transform,28)||!U(output,capacity,at,kind)||!Put(output,capacity,at,body,BodySize(kind)))return Fail("Physics automatic shape write failed",g);if(kind==3)sphereCount++;else obbCount++;
   }
   for(const PrefabScene::Node* n=model.first;n;n=n->next){if(!Explicit(*n)||groups[n->index]!=g)continue;unsigned int kind=0;float body[9]={},transform[7]={0,0,0,0,0,0,1};if(!(settings.exactOwnerCoordinates?NativePhysicsExplicit::Build(model,partition,g,*n,offset,kind,body,transform):PhysicsExplicitShapes::Build(partition,g,*n,offset,kind,body,transform)))return Fail("Explicit physics preparation failed",g);
    if(childCount==1){if(!Put(output,capacity,at,kind==7?identity:transform,28))return Fail("Explicit physics transform failed",g);}else if(kind!=7){if(!U(output,capacity,at,5)||!Put(output,capacity,at,transform,28))return Fail("Explicit physics wrapper failed",g);}
    if(!U(output,capacity,at,kind)||!Put(output,capacity,at,body,BodySize(kind)))return Fail("Explicit physics body write failed",g);explicitCount++;
   }
   record->next=0;record->group=g;record->modelIndex=modelIndex++;record->type=childCount>1?6:(childCount?*(unsigned int*)(output+28):0);record->data=output;record->bytes=at;record->children=childCount;lstrcpynA(record->name,owner.name,256);
   if(last)last->next=record;else first=record;last=record;count++;if(at>16u*1024u*1024u-size)return Fail("Physics owner aggregate byte limit",g);size+=at;
  }
  for(unsigned int i=0;i<settings.meshAssociationCount;i++)if(!used[i])return Fail("Unclaimed source owner Mesh MOPP association",settings.meshAssociations[i].group);
  data=(unsigned char*)memory.Allocate(size?size:1);if(!data)return Fail("Physics owner final output allocation failed");unsigned int at=0;for(const Record* record=first;record;record=record->next){memcpy(data+at,record->data,record->bytes);at+=record->bytes;}return at==size||Fail("Physics owner output size mismatch");
 }
};
}
