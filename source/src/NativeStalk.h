#pragma once
#include "NativePreparedRenderScene.h"
#include "SourceRuntimeObjectOrder.h"
#include "NativeBlindDataRegistry.h"
#include "StalkNodes.h"
// AISTALK BlindData (0x6aaf0885), FEAR WorldPacker ConvertStalkNodes (VA 0x1000B600):
// u32 version 0, then the carver hull record of the single child brush written by
// the shared serializer VA 0x10032E20 (float height, u32 count, count * xyz with the
// common hull y). StalkDataIndex = record index + 0.1f. Records follow KeyFramer and
// precede the AINavMesh record ("Converting Key Data", "Converting AI Nodes", Nav).
// A hull the carver builder rejects as invalid (degenerate silhouette) is refused.
namespace NativeStalk {
static const unsigned int TypeId=0x6aaf0885u,Missing=0xffffffffu;
class Builder {
 PrefabScene::Arena memory;Builder(const Builder&);Builder& operator=(const Builder&);
 bool Fail(const char* message,unsigned int node=Missing){char saved[512];lstrcpynA(saved,message?message:"Stalk source compilation failed",512);Clear();lstrcpyA(diagnostic,saved);error=diagnostic;errorNode=node;return false;}
public:
 NativeBlindDataRegistry::RecordInput* records;NativeBlindDataRegistry::BindingInput* bindings;unsigned int count,bindingCount,errorNode;const char* error;char diagnostic[512];
 Builder():records(0),bindings(0),count(0),bindingCount(0),errorNode(Missing),error(0){diagnostic[0]=0;}
 void Clear(){memory.Clear();records=0;bindings=0;count=bindingCount=0;errorNode=Missing;error=0;diagnostic[0]=0;}
 bool Build(const NativePreparedRenderScene::Context& scene){
  Clear();if(!scene.Ready())return Fail("Invalid Stalk source scene");const PrefabScene::Model& model=scene.Model();
  // The object list before Nav: registration order (authored, then loaded prefab
  // candidates) without the later removal of AINavMesh root descendants.
  SourceObjectInventory::Builder inventory;if(!inventory.Build(scene))return Fail(inventory.error,inventory.errorNode);
  const PrefabScene::Node** nodes=(const PrefabScene::Node**)memory.Allocate((model.count?model.count:1)*sizeof(void*));unsigned int* order=(unsigned int*)memory.Allocate((inventory.count?inventory.count:1)*4);if(!nodes||!order)return Fail("Stalk node index allocation failed");
  unsigned int seen=0;for(const PrefabScene::Node* n=model.first;n;n=n->next){if(seen>=model.count||n->index!=seen)return Fail("Invalid Stalk source node order");nodes[seen++]=n;}if(seen!=model.count)return Fail("Incomplete Stalk source index");
  unsigned int ordered=0,candidates=0;for(unsigned int pass=0;pass<2;pass++)for(unsigned int i=0;i<inventory.count;i++){const SourceObjectInventory::Row& row=inventory.rows[i];if(row.candidateIndex==SourceObjectInventory::Missing||row.importedFromPrefab!=pass)continue;if(row.sourceNode>=seen)return Fail("Invalid Stalk runtime object");order[ordered++]=row.sourceNode;if(StalkNodes::IsStalk(*nodes[row.sourceNode]))candidates++;}
  SourceRuntimeObjectOrder::Builder runtime;if(!runtime.Build(scene))return Fail(runtime.error,runtime.errorNode);
  unsigned char* survives=(unsigned char*)memory.Allocate(seen?seen:1);if(!survives)return Fail("Stalk node index allocation failed");memset(survives,0,seen?seen:1);
  for(unsigned int i=0;i<runtime.count;i++){if(runtime.rows[i].sourceNode>=seen)return Fail("Invalid Stalk runtime object");survives[runtime.rows[i].sourceNode]=1;}
  records=(NativeBlindDataRegistry::RecordInput*)memory.Allocate((candidates?candidates:1)*sizeof(*records));bindings=(NativeBlindDataRegistry::BindingInput*)memory.Allocate((candidates?candidates:1)*sizeof(*bindings));if(!records||!bindings)return Fail("Stalk record allocation failed");
  for(unsigned int i=0;i<ordered;i++){
   const PrefabScene::Node& node=*nodes[order[i]];if(!StalkNodes::IsStalk(node))continue;
   const PrefabScene::Node* child=StalkNodes::ConvertedChild(node);if(!child)continue;
   WorldText::Vec3 points[64];unsigned int hull=0;float height=0;
   if(!NativeRawNavMeshGeneral::Builder::Hull(*child,scene.Offset(),points,hull,height))return Fail("AINodeStalk brush hull is outside the observed profile",node.index);
   // An empty projection (one polygon, 32+ faces, no closed silhouette) keeps the
   // builder's cleared fields: height 0, count 0 (stalk-35520b8d flat/thirty-sided).
   if(!hull)height=0;
   // CONDEMNED FORK: version 1 = version, center (float sequential sum of the hull points / count;
   // bit-exact on all 202 retail Condemned AISTALK records), then the FEAR hull record.
   unsigned int bytes=24+12*hull;unsigned char* data=(unsigned char*)memory.Allocate(bytes);if(!data)return Fail("Stalk record allocation failed",node.index);
   float center[3]={0,0,0};
   if(hull){volatile float sx=0,sy=0,sz=0;for(unsigned int k=0;k<hull;k++){sx=sx+points[k].x;sy=sy+points[k].y;sz=sz+points[k].z;}float n=(float)hull;center[0]=sx/n;center[1]=sy/n;center[2]=sz/n;}
   unsigned int version=1;memcpy(data,&version,4);memcpy(data+4,center,12);memcpy(data+16,&height,4);memcpy(data+20,&hull,4);for(unsigned int k=0;k<hull;k++)memcpy(data+24+12*k,&points[k].x,12);
   records[count].typeId=TypeId;records[count].data=data;records[count].bytes=bytes;
   // A node below an AINavMesh root keeps its record but is deleted with the root
   // subtree after Nav, so no StalkDataIndex reaches the object table.
   if(survives[node.index]){bindings[bindingCount].sourceNode=node.index;bindings[bindingCount].property="StalkDataIndex";bindings[bindingCount].ordinal=count;bindings[bindingCount].typeId=TypeId;bindingCount++;}
   count++;
  }
  return true;
 }
};
}
