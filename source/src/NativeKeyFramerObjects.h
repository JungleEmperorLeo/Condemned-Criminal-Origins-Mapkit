#pragma once
#include "NativeKeyFramer.h"
#include "NativeObjectsCompiler.h"
// Joint KeyFramer+Objects boundary only. Other BlindData producers are still
// separate; callers must not treat this partial registry as a complete map.
namespace NativeKeyFramerObjects {
class Builder {
 PrefabScene::Arena memory;Builder(const Builder&);Builder& operator=(const Builder&);
 bool Fail(const char* message,unsigned int node=0xffffffffu){char saved[512];lstrcpynA(saved,message?message:"KeyFramer/Objects assembly failed",512);Clear();lstrcpyA(diagnostic,saved);error=diagnostic;errorNode=node;return false;}
public:
 unsigned char* objects;unsigned char* blind;unsigned int objectsBytes,blindBytes,objectCount,keyFramerCount,keyCount,errorNode;const char* error;char diagnostic[512];
 Builder():objects(0),blind(0),objectsBytes(0),blindBytes(0),objectCount(0),keyFramerCount(0),keyCount(0),errorNode(0xffffffffu),error(0){diagnostic[0]=0;}
 void Clear(){memory.Clear();objects=blind=0;objectsBytes=blindBytes=objectCount=keyFramerCount=keyCount=0;errorNode=0xffffffffu;error=0;diagnostic[0]=0;}
 bool Build(const NativePreparedRenderScene::Context& scene,const NativeKeyFramer::Settings& settings=NativeKeyFramer::Settings()){
  Clear();NativeKeyFramer::Builder keys;if(!keys.Build(scene,settings))return Fail(keys.error,keys.errorNode);NativeBlindDataRegistry::Registry registry;if(!registry.Build(keys.records,keys.count,keys.bindings,keys.count))return Fail(registry.error);
  NativeObjectsCompiler::GeneratedReal* values=(NativeObjectsCompiler::GeneratedReal*)memory.Allocate((registry.BindingCount()?registry.BindingCount():1)*sizeof(NativeObjectsCompiler::GeneratedReal));if(!values)return Fail("KeyFramer binding value allocation failed");
  for(unsigned int i=0;i<registry.BindingCount();i++){const NativeBlindDataRegistry::Binding& binding=registry.Bindings()[i];if(binding.typeId!=NativeKeyFramer::TypeId||!GameClassMetadata::Exact(binding.property,"KeyDataIndex"))return Fail("Unexpected KeyFramer registry binding",binding.sourceNode);values[i].sourceNode=binding.sourceNode;values[i].property=binding.property;values[i].value=(float)((float)binding.ordinal+.1f);}
  NativeObjectsCompiler::Settings objectSettings;objectSettings.generatedReals=values;objectSettings.generatedRealCount=registry.BindingCount();NativeObjectsCompiler::Builder writer;if(!writer.Build(scene,objectSettings))return Fail(writer.error,writer.errorNode);
  objects=(unsigned char*)memory.Allocate(writer.size);blind=(unsigned char*)memory.Allocate(registry.Size());if(!objects||!blind)return Fail("Owned KeyFramer/Objects outputs allocation failed");memcpy(objects,writer.data,writer.size);memcpy(blind,registry.Data(),registry.Size());objectsBytes=writer.size;blindBytes=registry.Size();objectCount=writer.count;keyFramerCount=keys.count;keyCount=keys.totalKeys;return true;
 }
};
}
