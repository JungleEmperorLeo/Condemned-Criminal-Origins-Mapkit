#pragma once
#include "BrushRenderSettings.h"
// Compiler-preparation policy. Original 0x5DF0/0x5D00 walks the
// authored hierarchy and replaces descendant ShadowLOD properties; source
// nodes and their properties remain immutable in this reconstruction.
namespace SourceShadowOverrides {
inline const PrefabScene::Property* Property(const PrefabScene::Node& n,const char* name){for(const PrefabScene::Property* p=n.properties;p;p=p->next)if(FearBrushFlags::Equal(p->name,name))return p;return 0;}
inline bool Select(const PrefabScene::Node& n,const char* name,const char*& out){
 const PrefabScene::Property* p=Property(n,name);out=0;if(!p)return true;if(!p->string)return false;
 if(FearBrushFlags::Equal(p->data,"No Override"))return true;
 if(!BrushRenderSettings::KnownLOD(p->data))return false;out=p->data;return true;
}
inline bool Boolean(const PrefabScene::Property* p,bool& value){
 value=false;if(!p)return true;WorldText::Document doc;float number;
 if(p->string||!FearBrushFlags::Equal(p->type,"bool")||!p->data||!doc.Parse(p->data,lstrlenA(p->data))||doc.Elements(0)!=1||!doc.Number(doc.First(0),number)||!WorldText::Finite(number))return false;value=number!=0;return true;
}
class Policy {
 PrefabScene::Arena memory;
 bool Fail(const char* reason){memory.Clear();flags=0;worldModelLOD=0;count=overrides=brushes=worldModels=0;error=reason;return false;}
public:
 unsigned int* flags;const char** worldModelLOD;unsigned int count,overrides,brushes,worldModels;const char* error;
 Policy():flags(0),worldModelLOD(0),count(0),overrides(0),brushes(0),worldModels(0),error(0){}
 // Prototype sharing currently resolves authored instance identity before
 // this compiler policy. Reject aliases whose ordered source brush flags
 // diverge, rather than silently borrowing a representative's shadow mesh.
 // Exact sequences avoid treating a hash collision as compatibility proof.
 bool CompatibleBrushFlags(const WorldModels::Partition& partition,unsigned int group,unsigned int representative)const{
  if(!flags||!count||group>=partition.groupCount||representative>=partition.groupCount)return false;
  unsigned int left=0,right=0;
  for(;;){
   while(left<partition.assignmentCount&&partition.assignments[left].group!=group)left++;
   while(right<partition.assignmentCount&&partition.assignments[right].group!=representative)right++;
   if(left==partition.assignmentCount||right==partition.assignmentCount)return left==partition.assignmentCount&&right==partition.assignmentCount;
   const PrefabScene::Node* a=partition.assignments[left++].brush;const PrefabScene::Node* b=partition.assignments[right++].brush;
   if(!a||!b||a->index>=count||b->index>=count||a->sourceId!=b->sourceId||!FearBrushFlags::Equal(a->source,b->source)||flags[a->index]!=flags[b->index])return false;
  }
 }
 bool Build(const PrefabScene::Model& model,const WorldModels::Partition& partition){
  Fail(0);if(model.count>100000||partition.error||!partition.groups||!partition.groupCount||partition.groupCount>4096||partition.assignmentCount>model.brushes||(!partition.assignments&&partition.assignmentCount))return Fail("Shadow override model/partition invalid");
  flags=(unsigned int*)memory.Allocate((model.count+1)*4);worldModelLOD=(const char**)memory.Allocate((model.count+1)*4);
  const char** brush=(const char**)memory.Allocate((model.count+1)*4);const char** wm=(const char**)memory.Allocate((model.count+1)*4);unsigned char* owners=(unsigned char*)memory.Allocate(model.count+1);unsigned char* assigned=(unsigned char*)memory.Allocate(model.count+1);
  if(!flags||!worldModelLOD||!brush||!wm||!owners||!assigned)return Fail("Shadow override allocation failed");memset(owners,0,model.count+1);memset(assigned,0,model.count+1);memset(worldModelLOD,0,(model.count+1)*4);
  for(unsigned int g=1;g<partition.groupCount;g++){const WorldModels::Group& group=partition.groups[g];if(!group.owner||group.ownerIndex>=model.count||group.owner->index!=group.ownerIndex)return Fail("Shadow override owner metadata invalid");owners[group.ownerIndex]=1;}
  for(unsigned int a=0;a<partition.assignmentCount;a++){const WorldModels::Assignment& item=partition.assignments[a];if(!item.brush||item.brush->kind!=1||item.brush->index>=model.count||item.group>=partition.groupCount||assigned[item.brush->index])return Fail("Shadow override brush assignment invalid");assigned[item.brush->index]=1;}
  unsigned int seen=0;for(const PrefabScene::Node* n=model.first;n;n=n->next){
   if(n->index!=seen||seen>=model.count||(n->parent!=0xffffffffu&&n->parent>=seen))return Fail("Shadow override hierarchy invalid");
   const char* b=n->parent==0xffffffffu?0:brush[n->parent];const char* w=n->parent==0xffffffffu?0:wm[n->parent];flags[seen]=n->surfaceFlags;
   if(n->kind==2&&FearBrushFlags::Equal(n->className,"ShadowOverride")){
    // A nested object replaces both selectors, including clearing them for
    // missing/No Override values. It does not inherit either outer selector.
    if(!Select(*n,"BrushOverride",b)||!Select(*n,"WMOverride",w))return Fail("Unknown shadow override property");overrides++;
   }else if(n->kind==2&&owners[seen]){
    // Original class metadata CF_WORLDMODEL applies WMOverride to this owner's
    // property, then clears both selectors for the child traversal: VA
    // 0x10005DC7 zeroes ebx (brush) and skips its reload at 0x10005D5B. SDK
    // Campus copiers under BrushOverride "Never" keep their shadow volumes.
    worldModelLOD[seen]=w;if(w)worldModels++;w=0;b=0;
   }else if(n->kind==1&&b&&assigned[seen]){
    unsigned int base;bool step,clip;const PrefabScene::Property* clipProperty=Property(*n,"ClipLight");
    if(!FearBrushFlags::Base(n->type,base)||!Boolean(Property(*n,"NotAStep"),step)||!Boolean(clipProperty,clip))return Fail("Invalid overridden brush flags");
    flags[seen]=FearBrushFlags::Modify(base,step,clipProperty!=0,clip,b);brushes++;
   }
   brush[seen]=b;wm[seen]=w;seen++;
  }
  if(seen!=model.count)return Fail("Shadow override model incomplete");count=seen;return true;
 }
};
}
