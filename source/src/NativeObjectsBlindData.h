#pragma once
#include "NativeKeyFramer.h"
#include "NativeShatter.h"
#include "NativeScatter.h"
#include "NativeStalk.h"
#include "NativeObjectsCompiler.h"
#include "NativeProcessedNavMesh.h"
#include "NativeProcessedNavMeshMulti.h"
#include "NativeProcessedNavMeshConvex.h"
#include "NativeProcessedNavMeshPlanar.h"
#include "NativeProcessedNavMeshGeneral.h"
// Shared boundary for implemented producers, including source-bound cell-grid
// navigation, ScatterVolume and AINodeStalk. Broader navigation profiles remain rejected.
namespace NativeObjectsBlindData {
struct Settings {unsigned int version;NativeKeyFramer::Settings keys;NativeShatter::Settings shatter;NativeScatter::Settings scatter;NativeProcessedNavMesh::Input navigation;
 bool rawNavigation; // RawNavMesh option (0x10028B81): the record is marker 0 + the raw packet, no SDK processing
 Settings():version(1),rawNavigation(false){}};
class Builder {
 PrefabScene::Arena memory;Builder(const Builder&);Builder& operator=(const Builder&);
 bool Fail(const char* message,unsigned int node=0xffffffffu){char saved[512];lstrcpynA(saved,message?message:"Objects/BlindData compilation failed",512);Clear();lstrcpyA(diagnostic,saved);error=diagnostic;errorNode=node;return false;}
 bool Aliases(const void* p,unsigned int n)const{return NativeBlindDataRegistry::detail::Overlap(p,n,objects,objectsBytes)||NativeBlindDataRegistry::detail::Overlap(p,n,blind,blindBytes);}
public:
 unsigned char* objects;unsigned char* blind;unsigned int objectsBytes,blindBytes,objectCount,keyFramerCount,keyCount,shatterCount,shatterBindings,navMeshCount,navMeshBindings,errorNode;const char* error;char diagnostic[512];
 Builder():objects(0),blind(0),objectsBytes(0),blindBytes(0),objectCount(0),keyFramerCount(0),keyCount(0),shatterCount(0),shatterBindings(0),navMeshCount(0),navMeshBindings(0),errorNode(0xffffffffu),error(0){diagnostic[0]=0;}
 void Clear(){memory.Clear();objects=blind=0;objectsBytes=blindBytes=objectCount=keyFramerCount=keyCount=shatterCount=shatterBindings=navMeshCount=navMeshBindings=0;errorNode=0xffffffffu;error=0;diagnostic[0]=0;}
 bool BuildKnownTypes(const NativePreparedRenderScene::Context& scene,const Settings& supplied=Settings()){
  Settings settings=supplied;
  if(settings.navigation.bytes>16u*1024u*1024u||settings.navigation.sourceRawBytes>16u*1024u*1024u)return Fail("Navigation optional input exceeds byte limits");
  if(Aliases(settings.navigation.processed,settings.navigation.bytes)||Aliases(settings.navigation.sourceRaw,settings.navigation.sourceRawBytes))return Fail("Navigation optional input aliases prior Objects/BlindData output");
  Clear();if(!scene.Ready()||settings.version!=1)return Fail("Invalid Objects/BlindData scene/settings");
  // Do not fabricate a slot for a missing family: it would shift all following
  // real-valued bindings. Standalone family research has its separate APIs.
   const PrefabScene::Node** navigationRoots=(const PrefabScene::Node**)memory.Allocate((scene.Model().count?scene.Model().count:1)*sizeof(void*));if(!navigationRoots)return Fail("Navigation root index allocation failed");unsigned int rootCount=0;unsigned char* navigationOwned=(unsigned char*)memory.Allocate(scene.Model().count?scene.Model().count:1);if(!navigationOwned)return Fail("Navigation root ownership allocation failed");
   // Only outermost AINavMesh objects are roots; nested ones are deleted with the root subtree (original VA 0x100055E0, 0x100054D0).
   for(const PrefabScene::Node* n=scene.Model().first;n;n=n->next){if(n->index>=scene.Model().count||(n->parent!=0xffffffffu&&n->parent>=n->index))return Fail("Invalid navigation root ancestry",n->index);navigationOwned[n->index]=(unsigned char)(n->parent!=0xffffffffu&&navigationOwned[n->parent]);if(n->kind==2){if(!navigationOwned[n->index]&&FearBrushFlags::Equal(n->className,"AINavMesh")){navigationOwned[n->index]=1;if(rootCount==scene.Model().count)return Fail("Registered navigation root count exceeds the source model",n->index);navigationRoots[rootCount++]=n;if(!settings.rawNavigation&&(!settings.navigation.processed||!settings.navigation.bytes||!settings.navigation.sourceRaw||!settings.navigation.sourceRawBytes))return Fail("Unimplemented BlindData family requires its source producer before registration",n->index);}if(GameClassMetadata::Exact(n->className,"ScatterFX"))return Fail("Unimplemented BlindData family requires its source producer before registration",n->index);}}
   const bool hasNavigation=rootCount!=0;if(!hasNavigation&&(settings.navigation.processed||settings.navigation.bytes||settings.navigation.sourceRaw||settings.navigation.sourceRawBytes))return Fail("Navigation optional input requires a matching source root");
  NativeRawNavMesh::Builder rawNavigation;if(hasNavigation){if(!rawNavigation.Build(scene.Model(),scene.Offset()))return Fail(rawNavigation.error,rawNavigation.errorNode);if(rawNavigation.rootCount!=rootCount)return Fail("Navigation source root count differs",rawNavigation.sourceNode);for(unsigned int r=0;r<rootCount;r++){const PrefabScene::Node* navigationRoot=navigationRoots[r];unsigned int fields=0,runtimeSixteen=0;for(const PrefabScene::Property* p=navigationRoot->properties;p;p=p->next){if(fields++==4096)return Fail("Navigation property count exceeds profile",navigationRoot->index);if(GameClassMetadata::Exact(p->name,"CharacterType16")){if(runtimeSixteen++||!GameClassMetadata::Exact(p->type,"string")||!p->data||(!rawNavigation.generalProfile&&!GameClassMetadata::Exact(p->data,"None")))return Fail("Registered navigation runtime CharacterType16 must be one typed string; active values require the general source profile",navigationRoot->index);}}}const char* problem=0;bool valid=settings.rawNavigation?true:rawNavigation.generalProfile?NativeProcessedNavMeshGeneral::Validate(rawNavigation.data,rawNavigation.size,rawNavigation.rootCount,settings.navigation,problem):rawNavigation.planarPolygonProfile?NativeProcessedNavMeshPlanar::Validate(rawNavigation,settings.navigation,problem):((rawNavigation.convexPolygonProfile||rawNavigation.slopePolygonProfile)?NativeProcessedNavMeshConvex::Validate(rawNavigation,settings.navigation,problem):(rootCount>1?NativeProcessedNavMeshMulti::ValidateRootSet(rawNavigation,settings.navigation,problem,rootCount):(rawNavigation.brushCount==1?NativeProcessedNavMesh::Validate(rawNavigation,settings.navigation,problem):NativeProcessedNavMeshMulti::Validate(rawNavigation,settings.navigation,problem))));if(!valid)return Fail(problem,rawNavigation.sourceNode);}
  NativeKeyFramer::Builder keys;if(!keys.Build(scene,settings.keys))return Fail(keys.error,keys.errorNode);NativeStalk::Builder stalk;if(!stalk.Build(scene))return Fail(stalk.error,stalk.errorNode);NativeShatter::Builder shatter;if(!shatter.Build(scene,settings.shatter))return Fail(shatter.error,shatter.errorNode);
  // ConvertScatter runs after Import Geometry: its records follow Shatter.
  NativeScatter::Builder scatter;if(!scatter.Build(scene,settings.scatter))return Fail(scatter.error,scatter.errorNode);
  // "Converting AI Nodes" (Stalk) runs between key data and the AINavMesh build.
  const unsigned int navCount=hasNavigation?1u:0u,head=keys.count+stalk.count,bindHead=keys.count+stalk.bindingCount;unsigned int recordsCount=head+navCount+shatter.count+scatter.count,bindingsCount=bindHead+navCount+shatter.bindingCount+scatter.bindingCount;
  NativeBlindDataRegistry::RecordInput* records=(NativeBlindDataRegistry::RecordInput*)memory.Allocate((recordsCount?recordsCount:1)*sizeof(*records));NativeBlindDataRegistry::BindingInput* bindings=(NativeBlindDataRegistry::BindingInput*)memory.Allocate((bindingsCount?bindingsCount:1)*sizeof(*bindings));if(!records||!bindings)return Fail("Objects/BlindData registry input allocation failed");
  for(unsigned int i=0;i<keys.count;i++){records[i]=keys.records[i];bindings[i]=keys.bindings[i];}
  for(unsigned int i=0;i<stalk.count;i++)records[keys.count+i]=stalk.records[i];
  for(unsigned int i=0;i<stalk.bindingCount;i++){bindings[keys.count+i]=stalk.bindings[i];bindings[keys.count+i].ordinal+=keys.count;}
  const unsigned char* navigationRecord=(const unsigned char*)settings.navigation.processed;unsigned int navigationBytes=settings.navigation.bytes;
  if(hasNavigation&&settings.rawNavigation){unsigned char* packed=(unsigned char*)memory.Allocate(rawNavigation.size+4);if(!packed)return Fail("Raw navigation record allocation failed");memset(packed,0,4);memcpy(packed+4,rawNavigation.data,rawNavigation.size);navigationRecord=packed;navigationBytes=rawNavigation.size+4;}
  if(hasNavigation){records[head].typeId=NativeProcessedNavMesh::TypeId;records[head].data=navigationRecord;records[head].bytes=navigationBytes;bindings[bindHead].sourceNode=rawNavigation.sourceNode;bindings[bindHead].property="BlindDataIndex";bindings[bindHead].ordinal=head;bindings[bindHead].typeId=NativeProcessedNavMesh::TypeId;}
  for(unsigned int i=0;i<shatter.count;i++)records[head+navCount+i]=shatter.records[i];
  for(unsigned int i=0;i<shatter.bindingCount;i++){bindings[bindHead+navCount+i]=shatter.registryBindings[i];bindings[bindHead+navCount+i].ordinal+=head+navCount;}
  const unsigned int scatterRecord=head+navCount+shatter.count,scatterBinding=bindHead+navCount+shatter.bindingCount;
  for(unsigned int i=0;i<scatter.count;i++)records[scatterRecord+i]=scatter.records[i];
  for(unsigned int i=0;i<scatter.bindingCount;i++){bindings[scatterBinding+i]=scatter.bindings[i];bindings[scatterBinding+i].ordinal+=scatterRecord;}
  NativeBlindDataRegistry::Registry registry;if(!registry.Build(records,recordsCount,bindings,bindingsCount))return Fail(registry.error);
  NativeObjectsCompiler::GeneratedReal* values=(NativeObjectsCompiler::GeneratedReal*)memory.Allocate((bindingsCount+scatter.missingCount+rawNavigation.generalRealCount+1)*sizeof(*values));if(!values)return Fail("Objects/BlindData generated property allocation failed");
  for(unsigned int i=0;i<bindingsCount;i++){const NativeBlindDataRegistry::Binding& b=registry.Bindings()[i];values[i].sourceNode=b.sourceNode;values[i].property=b.property;
   if(b.typeId==NativeKeyFramer::TypeId&&GameClassMetadata::Exact(b.property,"KeyDataIndex"))values[i].value=(float)((float)b.ordinal+.1f);
   else if(b.typeId==NativeShatter::TypeId&&GameClassMetadata::Exact(b.property,"BlindObjectIndex"))values[i].value=(float)b.ordinal;
   else if(b.typeId==NativeProcessedNavMesh::TypeId&&GameClassMetadata::Exact(b.property,"BlindDataIndex"))values[i].value=(float)((float)b.ordinal+.1f);
   else if(b.typeId==NativeScatter::TypeId&&GameClassMetadata::Exact(b.property,"BlindDataIndex"))values[i].value=(float)((float)b.ordinal+.1f);
   else if(b.typeId==NativeStalk::TypeId&&GameClassMetadata::Exact(b.property,"StalkDataIndex"))values[i].value=(float)((float)b.ordinal+.1f);
   else return Fail("Unknown Objects/BlindData property encoding",b.sourceNode);
  }
   NativeObjectsCompiler::Settings objectSettings;objectSettings.generatedReals=values;unsigned int realCount=bindingsCount;for(unsigned int i=0;i<scatter.missingCount;i++){values[realCount].sourceNode=scatter.missingNodes[i];values[realCount].property="BlindDataIndex";values[realCount].value=-1.f;realCount++;}/* ConvertScatterVolume sets -1 before any early return */for(unsigned int i=0;i<rawNavigation.generalRealCount;i++){values[realCount].sourceNode=rawNavigation.generalReals[i].sourceNode;values[realCount].property=rawNavigation.generalReals[i].property;values[realCount].value=rawNavigation.generalReals[i].value;realCount++;}objectSettings.generatedRealCount=realCount;if(hasNavigation){NativeObjectsCompiler::GeneratedLong* navIndices=(NativeObjectsCompiler::GeneratedLong*)memory.Allocate(rootCount*sizeof(*navIndices));if(!navIndices)return Fail("Navigation generated identity allocation failed");for(unsigned int r=0;r<rootCount;r++){new(navIndices+r)NativeObjectsCompiler::GeneratedLong();navIndices[r].sourceNode=rawNavigation.generatedLongs[r].sourceNode;navIndices[r].property=rawNavigation.generatedLongs[r].property;navIndices[r].value=rawNavigation.generatedLongs[r].value;navIndices[r].createIfMissing=true;}objectSettings.generatedLongs=navIndices;objectSettings.generatedLongCount=rootCount;}NativeObjectsCompiler::Builder writer;if(!writer.Build(scene,objectSettings))return Fail(writer.error,writer.errorNode);
  objects=(unsigned char*)memory.Allocate(writer.size);blind=(unsigned char*)memory.Allocate(registry.Size());if(!objects||!blind)return Fail("Owned Objects/BlindData output allocation failed");memcpy(objects,writer.data,writer.size);memcpy(blind,registry.Data(),registry.Size());objectsBytes=writer.size;blindBytes=registry.Size();objectCount=writer.count;keyFramerCount=keys.count;keyCount=keys.totalKeys;shatterCount=shatter.count;shatterBindings=shatter.bindingCount;navMeshCount=navCount;navMeshBindings=navCount;return true;
 }
};
}
