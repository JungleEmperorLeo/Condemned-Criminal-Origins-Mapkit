#pragma once
#include "BrushRenderSettings.h"
#include "WorldModels.h"
#include "GameClassMetadata.h"
// Objects-side ShadowOverride traversal, independent of a geometry partition.
// The original tests direct CF_WORLDMODEL flags, including owners without brushes.
namespace NativeObjectShadow {
static const unsigned int NoIndex=0xffffffffu,MaximumNodes=100000,MaximumProperties=4096;
inline bool Exact(const char* a,const char* b){if(!a||!b)return false;while(*a&&*a==*b){a++;b++;}return *a==*b;}
inline bool Length(const char* value,unsigned int limit,unsigned int& length){if(!value)return false;for(length=0;length<=limit;length++)if(!value[length])return true;return false;}
inline bool Failure(const char*& error,const char* message){error=message;return false;}
inline bool ValidateProperties(const PrefabScene::Node& node){unsigned int count=0;for(const PrefabScene::Property* p=node.properties;p;p=p->next){unsigned int length;if(count++==MaximumProperties||!Length(p->name,255,length)||!Length(p->type,31,length))return false;}return true;}
inline const PrefabScene::Property* Property(const PrefabScene::Node& node,const char* name){for(const PrefabScene::Property* p=node.properties;p;p=p->next)if(Exact(p->name,name))return p;return 0;}
class Policy {
 PrefabScene::Arena memory;Policy(const Policy&);Policy& operator=(const Policy&);
 bool Fail(const char* message){Clear();error=message;return false;}
 bool Select(const PrefabScene::Node& node,const char* name,const char*& out){out=0;const PrefabScene::Property* p=Property(node,name);if(!p)return true;unsigned int length;
  if(!Exact(p->type,"string")||!p->string||!Length(p->data,4095,length))return false;if(FearBrushFlags::Equal(p->data,"No Override"))return true;
  if(!BrushRenderSettings::KnownLOD(p->data))return false;char* owned=(char*)memory.Allocate(length+1);if(!owned)return false;memcpy(owned,p->data,length+1);out=owned;return true;
 }
public:
 const char** worldModelLOD;unsigned int count,overrides,worldModels;const char* error;
 Policy():worldModelLOD(0),count(0),overrides(0),worldModels(0),error(0){}
 void Clear(){memory.Clear();worldModelLOD=0;count=overrides=worldModels=0;error=0;}
 bool Build(const PrefabScene::Model& model,const GameClassMetadata::Catalog& catalog){
  Clear();if(model.count>MaximumNodes||catalog.error||catalog.count>GameClassMetadata::MaximumClasses||(!catalog.classes&&catalog.count))return Fail("Invalid shadow Objects model/class catalog");
  worldModelLOD=(const char**)memory.Allocate((model.count+1)*sizeof(void*));const char** wm=(const char**)memory.Allocate((model.count+1)*sizeof(void*));if(!worldModelLOD||!wm)return Fail("Shadow Objects selector allocation failed");memset(worldModelLOD,0,(model.count+1)*sizeof(void*));
  unsigned int seen=0;for(const PrefabScene::Node* node=model.first;node;node=node->next){unsigned int length;
   if(seen>=model.count||node->index!=seen||(node->parent!=NoIndex&&node->parent>=seen)||node->kind>2||!Length(node->className,63,length)||!ValidateProperties(*node))return Fail("Invalid shadow Objects source hierarchy");
   const char* inherited=node->parent==NoIndex?0:wm[node->parent];
   if(node->kind==2&&FearBrushFlags::Equal(node->className,"ShadowOverride")){
    const char* brush=0;if(!Select(*node,"BrushOverride",brush)||!Select(*node,"WMOverride",inherited))return Fail("Unknown or invalid bounded shadow override selector");overrides++;
   }else if(node->kind==2){const GameClassMetadata::Class* definition=catalog.Find(node->className);if(definition&&(definition->flags&GameClassMetadata::WorldModelFlag)){worldModelLOD[seen]=inherited;if(inherited)worldModels++;inherited=0;}}
   wm[seen]=inherited;seen++;
  }
  if(seen!=model.count)return Fail("Incomplete shadow Objects source hierarchy");count=seen;return true;
 }
};
// The original inserts a missing string ShadowLOD at property-list count, i.e.
// appends it. First exact-name property governs typed lookup; a nonstring first
// match causes the original to append a separate string property of that name.
inline bool Apply(PrefabScene::Node& target,const char* selector,PrefabScene::Arena& memory,const char*& error){
 error=0;if(!selector)return true;unsigned int length;if(!Length(selector,4095,length)||!BrushRenderSettings::KnownLOD(selector)||target.kind!=2||!ValidateProperties(target))return Failure(error,"Invalid shadow Objects property application");
 const PrefabScene::Property* firstNamed=Property(target,"ShadowLOD");const PrefabScene::Property* replace=firstNamed&&Exact(firstNamed->type,"string")?firstNamed:0;
 if(replace&&replace->data&&Exact(replace->data,selector))return true;
 unsigned int propertyCount=0;for(const PrefabScene::Property* p=target.properties;p;p=p->next)propertyCount++;if(!replace&&propertyCount==MaximumProperties)return Failure(error,"ShadowLOD append exceeds property limit");
 char* owned=(char*)memory.Allocate(length+1);if(!owned)return Failure(error,"ShadowLOD text allocation failed");memcpy(owned,selector,length+1);
 PrefabScene::Property* first=0;PrefabScene::Property** tail=&first;for(const PrefabScene::Property* p=target.properties;p;p=p->next){PrefabScene::Property* copy=(PrefabScene::Property*)memory.Allocate(sizeof(PrefabScene::Property));if(!copy)return Failure(error,"ShadowLOD property copy allocation failed");*copy=*p;copy->next=0;if(p==replace)copy->data=owned;*tail=copy;tail=&copy->next;}
 if(!replace){PrefabScene::Property* extra=(PrefabScene::Property*)memory.Allocate(sizeof(PrefabScene::Property));if(!extra)return Failure(error,"ShadowLOD property append allocation failed");memset(extra,0,sizeof(*extra));lstrcpyA(extra->name,"ShadowLOD");lstrcpyA(extra->type,"string");extra->string=true;extra->data=owned;*tail=extra;}
 target.properties=first;return true;
}
}
