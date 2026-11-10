#pragma once
#include "SourceObjectInventory.h"
#include "StreamingStructure.h"
// Recovered registration order in the four declared source profiles: authored
// runtime candidates first, then loaded prefab candidates, each stable. This
// owns membership/order only; it does not compile property values or BlindData.
//
// Navigation removal (original VA 0x100054D0 after Nav generation): each
// registered link - an AINavMeshLink* below an outermost AINavMesh whose single
// child is a brush (VA 0x10003100) - is detached (0x10004D80 -> 0x10060500),
// then every remaining descendant of each outermost root is deleted
// recursively (0x1005BCD0). Those objects never reach the runtime table; the
// roots themselves and objects outside roots stay.
namespace SourceRuntimeObjectOrder {
typedef SourceObjectInventory::Row Row;
inline char Lower(char c){return c>='A'&&c<='Z'?(char)(c+32):c;}
inline bool SameName(const char* a,const char* b,unsigned int prefix=0xffffffffu){for(unsigned int i=0;i<64;i++){if(i==prefix)return true;if(Lower(a[i])!=Lower(b[i]))return false;if(!a[i])return true;}return false;}
class Builder {
 PrefabScene::Arena memory;Builder(const Builder&);Builder& operator=(const Builder&);
 bool Fail(const char* message,unsigned int node=0xffffffffu){Clear();error=message;errorNode=node;return false;}
public:
 Row* rows;unsigned int count,sourceObjects,excludedNoRuntime,authoredCount,importedCount,navigationRemoved,errorNode;const char* error;
 Builder():rows(0),count(0),sourceObjects(0),excludedNoRuntime(0),authoredCount(0),importedCount(0),navigationRemoved(0),errorNode(0xffffffffu),error(0){}
 void Clear(){memory.Clear();rows=0;count=sourceObjects=excludedNoRuntime=authoredCount=importedCount=navigationRemoved=0;errorNode=0xffffffffu;error=0;}
 bool Build(const NativePreparedRenderScene::Context& scene){
  Clear();SourceObjectInventory::Builder inventory;if(!inventory.Build(scene))return Fail(inventory.error,inventory.errorNode);
  const PrefabScene::Model& model=scene.Model();unsigned int total=model.count?model.count:1;
  unsigned char* inRoot=(unsigned char*)memory.Allocate(total);unsigned char* removed=(unsigned char*)memory.Allocate(total);unsigned int* children=(unsigned int*)memory.Allocate(total*4);unsigned int* firstChildKind=(unsigned int*)memory.Allocate(total*4);
  if(!inRoot||!removed||!children||!firstChildKind)return Fail("Runtime object navigation removal allocation failed");
  unsigned int seen=0;for(const PrefabScene::Node* n=model.first;n;n=n->next){if(n->index!=seen||seen>=model.count||(n->parent!=0xffffffffu&&n->parent>=seen))return Fail("Invalid runtime object ancestry",n->index);
   children[seen]=0;firstChildKind[seen]=0xffffffffu;bool rootParent=false;if(n->parent!=0xffffffffu){if(!children[n->parent]++)firstChildKind[n->parent]=n->kind;rootParent=inRoot[n->parent]==2;}
   inRoot[seen]=(unsigned char)(n->parent!=0xffffffffu&&(inRoot[n->parent]==1||rootParent)?1:0);
   if(!inRoot[seen]&&n->kind==2&&SameName(n->className,"AINavMesh"))inRoot[seen]=2; // outermost root
   seen++;}
  for(const PrefabScene::Node* n=model.first;n;n=n->next){unsigned int i=n->index;
   bool registered=n->kind==2&&SameName(n->className,"AINavMeshLink",13)&&children[i]==1&&firstChildKind[i]==1;
   removed[i]=(unsigned char)(inRoot[i]==1&&!registered?1:0);}
  // World Streaming then deletes every StreamingRegion, StreamingLink and
  // StreamingInfo node with its subtree (0x10036CF0, StreamingStructure.h);
  // an ObjectsOnly pack stops before that stage.
  if(model.count&&!scene.Profile().objectsOnly){WorldText::Vec3 zero={0,0,0};StreamingStructure::Builder streaming;if(!streaming.Build(model,zero))return Fail(streaming.error);for(unsigned int i=0;i<model.count;i++)if(streaming.removed[i])removed[i]=1;}
  rows=(Row*)memory.Allocate((inventory.candidates?inventory.candidates:1)*sizeof(Row));if(!rows)return Fail("Runtime object order allocation failed");
  for(unsigned int pass=0;pass<2;pass++)for(unsigned int i=0;i<inventory.count;i++){const Row& source=inventory.rows[i];if(source.candidateIndex==SourceObjectInventory::Missing||source.importedFromPrefab!=pass)continue;if(source.sourceNode>=model.count)return Fail("Runtime object source node outside the model",source.sourceNode);if(removed[source.sourceNode]){navigationRemoved++;continue;}rows[count]=source;rows[count].candidateIndex=count;count++;if(pass)importedCount++;else authoredCount++;}
  if(count+navigationRemoved!=inventory.candidates)return Fail("Runtime object order count mismatch");sourceObjects=inventory.count;excludedNoRuntime=inventory.noRuntime;return true;
 }
};
}
