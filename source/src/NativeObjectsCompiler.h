#pragma once
#include "SourceRuntimeObjectOrder.h"
#include "ObjectsBlock.h"
#include "SourceOwnerRotation.h"
#include "NativeObjectProperties.h"
#include "NativeObjectHierarchyLists.h"
#include "NativeAINodeLists.h"
#include "NativeObjectCommand.h"
#include "NativeObjectShadow.h"
// Ordered source properties, filename conversion and explicit generated values.
// Other source producers must run before this becomes a whole-world input.
namespace NativeObjectsCompiler {
struct GeneratedReal {unsigned int sourceNode;const char* property;float value;};
struct GeneratedLong {unsigned int sourceNode;const char* property;int value;bool createIfMissing;GeneratedLong():sourceNode(0xffffffffu),property(0),value(0),createIfMissing(false){}};
struct Settings:ObjectsBlock::Settings {const GeneratedReal* generatedReals;unsigned int generatedRealCount;const GeneratedLong* generatedLongs;unsigned int generatedLongCount;bool convertFilenames;Settings():generatedReals(0),generatedRealCount(0),generatedLongs(0),generatedLongCount(0),convertFilenames(true){rotationReplay=true;}};
class Builder {
 PrefabScene::Arena memory;Builder(const Builder&);Builder& operator=(const Builder&);
 bool Fail(const char* message,unsigned int node=0xffffffffu,const char* property=0){char saved[512],key[256];lstrcpynA(saved,message?message:"Objects compilation failed",512);lstrcpynA(key,property?property:"",256);Clear();lstrcpyA(diagnostic,saved);lstrcpyA(errorProperty,key);error=diagnostic;errorNode=node;return false;}
 static bool Include(const PrefabScene::Node&){return true;}
 bool AliasesOutput(const void* pointer,unsigned int bytes)const{if(!pointer||!bytes||!data||!size)return false;const unsigned long long a=(unsigned long long)pointer,b=(unsigned long long)data;return a<b+size&&b<a+bytes;}
 static char* LongText(PrefabScene::Arena& arena,int value){char* text=(char*)arena.Allocate(12);if(!text)return 0;unsigned int magnitude=value<0?0u-(unsigned int)value:(unsigned int)value;char reverse[10];unsigned int digits=0;do{reverse[digits++]=(char)('0'+magnitude%10u);magnitude/=10u;}while(magnitude);unsigned int at=0;if(value<0)text[at++]='-';while(digits)text[at++]=reverse[--digits];text[at]=0;return text;}
public:
 unsigned char* data;unsigned int size,count,properties,errorNode;const char* error;char diagnostic[512],errorProperty[256];
 Builder():data(0),size(0),count(0),properties(0),errorNode(0xffffffffu),error(0){diagnostic[0]=errorProperty[0]=0;}
 void Clear(){memory.Clear();data=0;size=count=properties=0;errorNode=0xffffffffu;error=0;diagnostic[0]=errorProperty[0]=0;}
 bool Build(const NativePreparedRenderScene::Context& scene,const Settings& settings=Settings()){
   // Inputs may be borrowed, but cannot refer to output released by this rebuild.
   // Inspect aliases before Clear or any generated array/name dereference.
  if(settings.generatedRealCount>100000||(!settings.generatedReals&&settings.generatedRealCount))return Fail("Invalid generated real property array");
  if(settings.generatedLongCount>100000||(!settings.generatedLongs&&settings.generatedLongCount))return Fail("Invalid generated long property array");
   if(AliasesOutput(settings.generatedReals,settings.generatedRealCount*sizeof(GeneratedReal)))return Fail("Generated real array aliases prior Objects output");
   if(AliasesOutput(settings.generatedLongs,settings.generatedLongCount*sizeof(GeneratedLong)))return Fail("Generated long array aliases prior Objects output");
   for(unsigned int i=0;i<settings.generatedRealCount;i++)if(AliasesOutput(settings.generatedReals[i].property,256))return Fail("Generated property name aliases prior Objects output",settings.generatedReals[i].sourceNode);
   for(unsigned int i=0;i<settings.generatedLongCount;i++)if(AliasesOutput(settings.generatedLongs[i].property,256))return Fail("Generated property name aliases prior Objects output",settings.generatedLongs[i].sourceNode);
   Clear();if(!scene.Ready()||settings.version!=1)return Fail("Objects compiler requires a prepared scene/settings");
  for(unsigned int i=0;i<settings.generatedRealCount;i++){const GeneratedReal& value=settings.generatedReals[i];if(value.sourceNode==0xffffffffu||!value.property||!value.property[0]||lstrlenA(value.property)>255||!WorldText::Finite(value.value))return Fail("Invalid generated real property",value.sourceNode);for(unsigned int j=0;j<i;j++)if(settings.generatedReals[j].sourceNode==value.sourceNode&&GameClassMetadata::Exact(settings.generatedReals[j].property,value.property))return Fail("Duplicate generated real property",value.sourceNode,value.property);}
  for(unsigned int i=0;i<settings.generatedLongCount;i++){const GeneratedLong& value=settings.generatedLongs[i];if(value.sourceNode==0xffffffffu||!value.property||!value.property[0]||lstrlenA(value.property)>255)return Fail("Invalid generated long property",value.sourceNode);for(unsigned int j=0;j<i;j++)if(settings.generatedLongs[j].sourceNode==value.sourceNode&&GameClassMetadata::Exact(settings.generatedLongs[j].property,value.property))return Fail("Duplicate generated long property",value.sourceNode,value.property);for(unsigned int j=0;j<settings.generatedRealCount;j++)if(settings.generatedReals[j].sourceNode==value.sourceNode&&GameClassMetadata::Exact(settings.generatedReals[j].property,value.property))return Fail("Generated real and long property conflict",value.sourceNode,value.property);}
  unsigned char* applied=(unsigned char*)memory.Allocate(settings.generatedRealCount?settings.generatedRealCount:1);if(!applied)return Fail("Generated property lookup allocation failed");
  unsigned char* longApplied=(unsigned char*)memory.Allocate(settings.generatedLongCount?settings.generatedLongCount:1);if(!longApplied)return Fail("Generated long property lookup allocation failed");
  SourceRuntimeObjectOrder::Builder order;if(!order.Build(scene))return Fail(order.error,order.errorNode);
  if(order.count>settings.maximumObjects)return Fail("Runtime object count exceeds profile");
   const PrefabScene::Model& model=scene.Model();NativeObjectShadow::Policy shadow;if(!shadow.Build(model,scene.Catalog()))return Fail(shadow.error);const PrefabScene::Node** nodes=(const PrefabScene::Node**)memory.Allocate((model.count?model.count:1)*sizeof(void*));if(!nodes)return Fail("Object source index allocation failed");
  unsigned int seen=0;for(const PrefabScene::Node* n=model.first;n;n=n->next){if(seen>=model.count||n->index!=seen)return Fail("Invalid runtime source node order");nodes[seen++]=n;}if(seen!=model.count)return Fail("Incomplete runtime source index");
   unsigned char* runtime=(unsigned char*)memory.Allocate(model.count?model.count:1);if(!runtime)return Fail("Runtime hierarchy membership allocation failed");for(unsigned int i=0;i<order.count;i++){if(order.rows[i].sourceNode>=seen)return Fail("Invalid runtime hierarchy member");runtime[order.rows[i].sourceNode]=1;}
   unsigned char* aiListApply=(unsigned char*)memory.Allocate(seen?seen:1);if(!aiListApply)return Fail("AI Node list plan allocation failed");
   {unsigned int overflowNode;const char* failure=0;if(!NativeAINodeLists::Plan(nodes,seen,aiListApply,overflowNode,failure))return Fail(failure,overflowNode);}
  PrefabScene::Model ordered;for(unsigned int i=0;i<order.count;i++){const unsigned int source=order.rows[i].sourceNode;if(source>=seen||nodes[source]->kind!=2)return Fail("Invalid runtime object source membership",source);PrefabScene::Node* n=ordered.Add();if(!n)return Fail("Ordered object allocation failed",source);*n=*nodes[source];n->next=0;ordered.objects++;
    {n->properties=0;PrefabScene::Property** tail=&n->properties;unsigned int copied=0;for(const PrefabScene::Property* p=nodes[source]->properties;p;p=p->next){if(copied++>=settings.maximumProperties)return Fail("Property copy count exceeds profile",source);PrefabScene::Property* copy=(PrefabScene::Property*)ordered.memory.Allocate(sizeof(PrefabScene::Property));if(!copy)return Fail("Generated property copy allocation failed",source);*copy=*p;copy->next=0;*tail=copy;tail=&copy->next;
     if(settings.convertFilenames&&!NativeObjectProperties::PrepareFilename(*copy,ordered.memory))return Fail("Filename property exceeds the bounded resource profile",source,p->name);
      if(!NativeObjectCommand::PrepareCompiled(*copy,ordered.memory))return Fail("Invalid bounded command property",source,p->name);
     for(unsigned int j=0;j<settings.generatedRealCount;j++){const GeneratedReal& value=settings.generatedReals[j];if(value.sourceNode!=source||!GameClassMetadata::Exact(value.property,p->name))continue;if(applied[j]||!GameClassMetadata::Exact(p->type,"real"))return Fail("Generated property needs one authored real field",source,p->name);char* text=(char*)ordered.memory.Allocate(11);if(!text)return Fail("Generated property text allocation failed",source,p->name);unsigned int bits;memcpy(&bits,&value.value,4);text[0]='0';text[1]='x';const char* hex="0123456789abcdef";for(unsigned int k=0;k<8;k++)text[2+k]=hex[(bits>>(28-k*4))&15];text[10]=0;copy->data=text;applied[j]=1;}
     for(unsigned int j=0;j<settings.generatedLongCount;j++){const GeneratedLong& value=settings.generatedLongs[j];if(value.sourceNode!=source||!GameClassMetadata::Exact(value.property,p->name))continue;if(longApplied[j]||!GameClassMetadata::Exact(p->type,"longint"))return Fail("Generated property needs one authored long field",source,p->name);copy->data=LongText(ordered.memory,value.value);if(!copy->data)return Fail("Generated long text allocation failed",source,p->name);longApplied[j]=1;}
    }
    for(unsigned int j=0;j<settings.generatedLongCount;j++){const GeneratedLong& value=settings.generatedLongs[j];if(value.sourceNode!=source||longApplied[j]||!value.createIfMissing)continue;if(copied>=settings.maximumProperties)return Fail("Generated long property count exceeds profile",source,value.property);PrefabScene::Property* copy=(PrefabScene::Property*)ordered.memory.Allocate(sizeof(PrefabScene::Property));if(!copy)return Fail("Generated long property allocation failed",source,value.property);lstrcpyA(copy->type,"longint");lstrcpyA(copy->name,value.property);copy->data=LongText(ordered.memory,value.value);if(!copy->data)return Fail("Generated long text allocation failed",source,value.property);*tail=copy;tail=&copy->next;copied++;longApplied[j]=1;}
   }
    if(FearBrushFlags::Equal(n->className,"Group")){const char* failure=0;if(!NativeObjectHierarchyLists::ApplyGroup(*n,nodes,seen,runtime,ordered.memory,failure))return Fail(failure,source);}
     if(NativeAINodeLists::SupportedClass(n->className)&&aiListApply[source]){const char* failure=0;bool overflow=false;if(!NativeAINodeLists::Apply(*n,nodes,seen,ordered.memory,failure,&overflow))return Fail(failure,source);}
     {const char* failure=0;if(!NativeObjectShadow::Apply(*n,shadow.worldModelLOD[source],ordered.memory,failure))return Fail(failure,source,"ShadowLOD");}
   }
  for(unsigned int i=0;i<settings.generatedRealCount;i++)if(!applied[i])return Fail("Unclaimed generated real property",settings.generatedReals[i].sourceNode,settings.generatedReals[i].property);
  for(unsigned int i=0;i<settings.generatedLongCount;i++)if(!longApplied[i])return Fail("Unclaimed generated long property",settings.generatedLongs[i].sourceNode,settings.generatedLongs[i].property);
  ObjectsBlock::Settings codec=settings;if(settings.rotationReplay)codec.rotationConverter=SourceOwnerRotation::Quaternion;
  ObjectsBlock::Block block;if(!block.BuildConfigured(ordered,Include,scene.Offset(),codec))return Fail(block.error,block.errorNode,block.errorProperty);
  if(block.count!=order.count)return Fail("Compiled runtime object count mismatch");data=(unsigned char*)memory.Allocate(block.size);if(!data)return Fail("Owned Objects output allocation failed");memcpy(data,block.data,block.size);size=block.size;count=block.count;properties=block.properties;return true;
 }
};
}
