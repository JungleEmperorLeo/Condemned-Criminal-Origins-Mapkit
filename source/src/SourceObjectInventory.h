#pragma once
#include "NativePreparedRenderScene.h"
// Diagnostic source boundary. Candidate means only !CF_NORUNTIME, not the final
// post-preprocessing runtime object table. No property defaults are synthesized.
namespace SourceObjectInventory {
static const unsigned int NoRuntimeFlag=2,MaximumObjects=100000,Missing=0xffffffffu;
struct Row {unsigned int sourceNode,classIndex,classFlags,authoredProperties,consumedCategory,candidateIndex,sourceParent,sourceId,importedFromPrefab,sourceKind;char name[256],className[64];};
class Builder {
 PrefabScene::Arena memory;Builder(const Builder&);Builder& operator=(const Builder&);
 bool Fail(const char* message,unsigned int node=Missing){Clear();error=message;errorNode=node;return false;}
public:
 Row* rows;unsigned int count,candidates,noRuntime,consumedCandidates,errorNode;const char* error;
 Builder():rows(0),count(0),candidates(0),noRuntime(0),consumedCandidates(0),errorNode(Missing),error(0){}
 void Clear(){memory.Clear();rows=0;count=candidates=noRuntime=consumedCandidates=0;errorNode=Missing;error=0;}
 bool Build(const NativePreparedRenderScene::Context& scene){
  Clear();if(!scene.Ready())return Fail("Object inventory requires a prepared source scene");
  const PrefabScene::Model& model=scene.Model();const GameClassMetadata::Catalog& catalog=scene.Catalog();const SourceExpandedRenderContours::Consumed& consumed=scene.Consumed();
  if(model.count>MaximumObjects||!catalog.classes||!catalog.count||consumed.count!=model.count||!consumed.categories)return Fail("Invalid source object inventory boundaries");
  unsigned int wanted=0,seen=0;for(const PrefabScene::Node* n=model.first;n;n=n->next){if(n->index!=seen||seen>=model.count)return Fail("Invalid source object inventory order");seen++;if(n->kind==2)wanted++;}if(seen!=model.count)return Fail("Incomplete source object inventory");
  rows=(Row*)memory.Allocate((wanted?wanted:1)*sizeof(Row));if(!rows)return Fail("Source object inventory allocation failed");
  for(const PrefabScene::Node* n=model.first;n;n=n->next){if(n->kind!=2)continue;
   // An undefined class is an ordinary runtime object without class flags.
   const GameClassMetadata::Class* definition=catalog.Find(n->className);
   unsigned int props=0;for(const PrefabScene::Property* p=n->properties;p;p=p->next){if(props==4096)return Fail("Authored property count limit",n->index);props++;}
   Row& row=rows[count++];row.sourceNode=n->index;row.classIndex=definition?(unsigned int)(definition-catalog.classes):Missing;row.classFlags=definition?definition->flags:0;row.authoredProperties=props;row.consumedCategory=consumed.categories[n->index];row.candidateIndex=Missing;
   row.sourceParent=n->parent;row.sourceId=n->sourceId;row.importedFromPrefab=n->importedFromPrefab?1u:0u;row.sourceKind=n->sourceKind;
   lstrcpynA(row.name,n->name,256);lstrcpynA(row.className,n->className,64);
   if(row.classFlags&NoRuntimeFlag)noRuntime++;else{row.candidateIndex=candidates++;if(row.consumedCategory)consumedCandidates++;}
  }
  return count==wanted||Fail("Source object inventory count mismatch");
 }
};
}
