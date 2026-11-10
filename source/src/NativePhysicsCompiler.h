#pragma once
#include "NativePhysicsMesh.h"
#include "NativePhysicsOwners.h"
#include "NativePhysicsMainExplicit.h"
#include "PhysicsMeshBlock.h"
namespace NativePhysicsCompiler {
// Backend transport explicitly associates bytecode with a source mesh prefix.
// Structural validation here does not execute MOPP or establish collision parity.
struct MoppInput {const void* mesh;unsigned int meshBytes;const void* code;unsigned int codeBytes;MoppInput():mesh(0),meshBytes(0),code(0),codeBytes(0){}};
struct Settings {
 unsigned int version,maximumBytes;NativePhysicsMesh::Settings mesh;NativePhysicsOwners::Settings owners;NativePhysicsMainExplicit::Settings mainExplicit;
 Settings():version(1),maximumBytes(16u*1024u*1024u){}
};
class Builder {
 PrefabScene::Arena memory;Builder(const Builder&);Builder& operator=(const Builder&);
 bool Fail(const char* message){char saved[512];lstrcpynA(saved,message?message:"Source physics compilation failed",512);Clear();lstrcpyA(diagnostic,saved);error=diagnostic;return false;}
 static bool Equal(const void* a,const void* b,unsigned int size){const unsigned char* x=(const unsigned char*)a;const unsigned char* y=(const unsigned char*)b;for(unsigned int i=0;i<size;i++)if(x[i]!=y[i])return false;return true;}
 static bool Put(unsigned char* target,unsigned int capacity,unsigned int& cursor,const void* source,unsigned int bytes){if(bytes>capacity-cursor||(!source&&bytes))return false;memcpy(target+cursor,source,bytes);cursor+=bytes;return true;}
 static bool U(unsigned char* target,unsigned int capacity,unsigned int& cursor,unsigned int value){return Put(target,capacity,cursor,&value,4);}
public:
 unsigned char* data;unsigned int size,shapeCount,mainType,nonMainCount,mainExplicitCount,meshVertices,meshTriangles;const char* error;char diagnostic[512];
 Builder():data(0),size(0),shapeCount(0),mainType(0),nonMainCount(0),mainExplicitCount(0),meshVertices(0),meshTriangles(0),error(0){diagnostic[0]=0;}
 void Clear(){memory.Clear();data=0;size=shapeCount=mainType=nonMainCount=mainExplicitCount=meshVertices=meshTriangles=0;error=0;diagnostic[0]=0;}
 bool Build(const NativePreparedRenderScene::Context& scene,const MoppInput& mopp,const Settings& settings=Settings()){
  Clear();if(!scene.Ready()||settings.version!=1)return Fail("Invalid source physics scene/settings");
  NativePhysicsMesh::Builder mesh;NativePhysicsOwners::Builder owners;NativePhysicsMainExplicit::Builder helpers;
  if(!mesh.Build(scene,settings.mesh))return Fail(mesh.error);if(!owners.Build(scene,settings.owners))return Fail(owners.error);if(!helpers.Build(scene,settings.mainExplicit))return Fail(helpers.error);
  return BuildParts(mesh,owners,helpers,mopp,settings);
 }
 bool BuildParts(const NativePhysicsMesh::Builder& mesh,const NativePhysicsOwners::Builder& owners,const NativePhysicsMainExplicit::Builder& helpers,const MoppInput& mopp,const Settings& settings=Settings()){
  Clear();if(settings.version!=1||!settings.maximumBytes||settings.maximumBytes>16u*1024u*1024u||mesh.error||owners.error||helpers.error||!mesh.data||!owners.data||!helpers.data||!mopp.mesh||mopp.meshBytes!=mesh.size||!Equal(mesh.data,mopp.mesh,mesh.size))return Fail("Invalid physics parts or MOPP mesh association");
  PhysicsMeshBlock::Block complete;if(!complete.Build(mesh.Prefix(),mopp.code,mopp.codeBytes))return Fail(complete.error);
  const float identity[7]={0,0,0,0,0,0,1};if(!Equal(mesh.data+4,identity,28))return Fail("MAIN static mesh requires identity transform");
  unsigned int recordCount=0,recordBytes=0;for(const NativePhysicsOwners::Record* record=owners.first;record;record=record->next){if(recordCount>=owners.count||record->modelIndex!=recordCount+1||!record->data||record->bytes<32||record->bytes>owners.size-recordBytes)return Fail("Invalid physics owner order/size");recordBytes+=record->bytes;recordCount++;}
  if(recordCount!=owners.count||recordBytes!=owners.size||owners.count>4095||helpers.count>100000||helpers.count!=owners.unownedExplicitShapes)return Fail("Physics owner/MAIN shape membership mismatch");
  recordCount=recordBytes=0;for(const NativePhysicsMainExplicit::Record* record=helpers.first;record;record=record->next){if(recordCount>=helpers.count||!record->data||record->bytes>helpers.size-recordBytes||!((record->type==2&&record->bytes==60)||(record->type==3&&record->bytes==48)||(record->type==7&&record->bytes==40)))return Fail("Invalid MAIN explicit child framing");recordBytes+=record->bytes;recordCount++;}
  if(recordCount!=helpers.count||recordBytes!=helpers.size)return Fail("MAIN explicit byte/count mismatch");
  const unsigned int overhead=helpers.count?40:0;
  if(complete.size>settings.maximumBytes||owners.size>settings.maximumBytes-complete.size||helpers.size>settings.maximumBytes-complete.size-owners.size||overhead>settings.maximumBytes-complete.size-owners.size-helpers.size)return Fail("Complete physics exceeds byte budget");
  size=complete.size+owners.size+helpers.size+overhead;data=(unsigned char*)memory.Allocate(size);if(!data)return Fail("Complete physics output allocation failed");unsigned int at=0;shapeCount=owners.count+1;
  if(!U(data,size,at,shapeCount))return Fail("Physics shape count write failed");
  if(helpers.count){if(!Put(data,size,at,identity,28)||!U(data,size,at,6)||!U(data,size,at,helpers.count+1)||!U(data,size,at,5))return Fail("MAIN physics compound write failed");}
  if(!Put(data,size,at,complete.data+4,complete.size-4)||!Put(data,size,at,helpers.data,helpers.size)||!Put(data,size,at,owners.data,owners.size)||at!=size)return Fail("Complete physics output size mismatch");
  mainType=helpers.count?6:1;nonMainCount=owners.count;mainExplicitCount=helpers.count;meshVertices=mesh.points;meshTriangles=mesh.indices/3;return true;
 }
};
}
