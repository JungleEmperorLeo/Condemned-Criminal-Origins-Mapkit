#pragma once
// Explicit class metadata is required. No object-name heuristic or silent fallback.
namespace WorldModels {
enum ClassKind {Unknown,Ordinary,WorldModel};
typedef ClassKind (*ResolveClass)(const PrefabScene::Node&,void*);
typedef bool (*IgnoreBrush)(const PrefabScene::Node&,void*);
struct Group {unsigned int ownerIndex,brushes,polygons;const PrefabScene::Node* owner;};
struct Assignment {const PrefabScene::Node* brush;unsigned int group;};
class Partition {
public:
 PrefabScene::Arena memory;Group* groups;Assignment* assignments;
 unsigned int groupCount,assignmentCount,ignoredBrushes;const char* error;
 Partition():groups(0),assignments(0),groupCount(0),assignmentCount(0),ignoredBrushes(0),error(0){}
 void Clear(){memory.Clear();groups=0;assignments=0;groupCount=assignmentCount=ignoredBrushes=0;error=0;}
 bool Fail(const char* message){Clear();error=message;return false;}
 bool Build(const PrefabScene::Model& model,ResolveClass resolve,void* context,IgnoreBrush ignore=0){
  Clear();if(model.count>100000)return Fail("World-model node limit exceeded");
  unsigned int* owners=(unsigned int*)memory.Allocate((model.count+1)*sizeof(unsigned int));
  groups=(Group*)memory.Allocate((model.count+1)*sizeof(Group));
  assignments=(Assignment*)memory.Allocate((model.brushes+1)*sizeof(Assignment));
  if(!owners||!groups||!assignments)return Fail("Could not allocate world-model partition");
  const PrefabScene::Node** nodes=(const PrefabScene::Node**)memory.Allocate((model.count+1)*sizeof(*nodes));
  unsigned char* worldModel=(unsigned char*)memory.Allocate(model.count+1);
  if(!nodes||!worldModel)return Fail("Could not allocate world-model partition");
  // Pass 1: ownership in authored order (ShadowOverride is not a WorldModel).
  unsigned int seen=0;
  for(const PrefabScene::Node* node=model.first;node;node=node->next){
   if(node->index!=seen||seen>=model.count||(node->parent!=0xffffffffu&&node->parent>=seen))return Fail("Invalid ordered hierarchy");
   nodes[seen]=node;worldModel[seen]=0;
   if(node->kind==2){ClassKind kind=resolve?resolve(*node,context):Unknown;
    if(kind==Unknown)return Fail("Class metadata required for object");
    worldModel[seen]=kind==WorldModel;
   }
   seen++;
  }
  if(seen!=model.count)return Fail("Node count inconsistent with model");
  unsigned int* order=0;unsigned int ordered=0;if(!Order(memory,model,nodes,order,ordered))return Fail("Could not allocate world-model order");
  // Pass 2: groups and brush assignments in the order the original compiler
  // traverses after "Applying Shadow Overrides" (see Order).
  // Group zero is the main-world bucket, including an empty main world.
  groupCount=1;groups[0].ownerIndex=0xffffffffu;
  for(unsigned int i=0;i<model.count;i++){const PrefabScene::Node* node=nodes[i];owners[i]=node->parent==0xffffffffu?0:owners[node->parent];if(worldModel[i])owners[i]=0xfffffffeu;}
  for(unsigned int k=0;k<ordered;k++){const PrefabScene::Node* node=nodes[order[k]];
   if(worldModel[node->index]){unsigned int owner=groupCount++;groups[owner].ownerIndex=node->index;groups[owner].owner=node;owners[node->index]=owner;}
  }
  // Re-resolve brush/object ownership through owner indices assigned above.
  for(unsigned int i=0;i<model.count;i++){const PrefabScene::Node* node=nodes[i];if(!worldModel[i])owners[i]=node->parent==0xffffffffu?0:owners[node->parent];}
  for(unsigned int k=0;k<ordered;k++){const PrefabScene::Node* node=nodes[order[k]];
   if(node->kind==1){if(ignore&&ignore(*node,context)){ignoredBrushes++;continue;}
    if(assignmentCount>=model.brushes)return Fail("Brush count inconsistent with model");
    unsigned int owner=owners[node->index];Assignment& a=assignments[assignmentCount++];a.brush=node;a.group=owner;
    groups[owner].brushes++;groups[owner].polygons+=node->geometry.polygons;
   }
  }
  if(assignmentCount+ignoredBrushes!=model.brushes)return Fail("Node or brush count inconsistent with model");
  return true;
 }
 // Original "Applying Shadow Overrides" (WorldPacker VA 0x10005DF0) collects
 // ShadowOverride objects in preorder and then DestroyNode(VA 0x1005BCD0)s each
 // one, moving its children in order to the END of its parent's child list
 // (VA 0x1005BB00). Later stages traverse that tree. Returns the preorder of
 // the remaining nodes (ShadowOverride nodes themselves are omitted).
 // nodes[i] must be the node with index i. Allocates from the caller's arena.
 static bool Order(PrefabScene::Arena& memory,const PrefabScene::Model& model,const PrefabScene::Node** nodes,unsigned int*& order,unsigned int& ordered){
  unsigned int n=model.count;order=(unsigned int*)memory.Allocate((n+1)*4);
  unsigned int* parent=(unsigned int*)memory.Allocate((n+1)*4);unsigned int* first=(unsigned int*)memory.Allocate((n+2)*4);unsigned int* last=(unsigned int*)memory.Allocate((n+2)*4);
  unsigned int* next=(unsigned int*)memory.Allocate((n+1)*4);unsigned int* stack=(unsigned int*)memory.Allocate((n+1)*4);unsigned int* overrides=(unsigned int*)memory.Allocate((n+1)*4);
  if(!order||!parent||!first||!last||!next||!stack||!overrides)return false;
  const unsigned int none=0xffffffffu,root=n;
  for(unsigned int i=0;i<=n;i++){first[i]=last[i]=none;}
  for(unsigned int i=0;i<n;i++){next[i]=none;parent[i]=nodes[i]->parent==none?root:nodes[i]->parent;unsigned int p=parent[i];if(last[p]==none)first[p]=i;else next[last[p]]=i;last[p]=i;}
  // Preorder of the authored tree gives the override collection order.
  unsigned int count=0,depth=0;if(first[root]!=none)stack[depth++]=first[root];
  while(depth){unsigned int i=stack[--depth];if(next[i]!=none)stack[depth++]=next[i];
   const PrefabScene::Node* node=nodes[i];if(node->kind==2&&FearBrushFlags::Equal(node->className,"ShadowOverride"))overrides[count++]=i;
   if(first[i]!=none)stack[depth++]=first[i];}
  for(unsigned int k=0;k<count;k++){unsigned int s=overrides[k],p=parent[s];
   // Unlink s from p, then append its children (in order) to p.
   unsigned int prev=none;for(unsigned int c=first[p];c!=none&&c!=s;c=next[c])prev=c;
   if(prev==none)first[p]=next[s];else next[prev]=next[s];if(last[p]==s)last[p]=prev;
   for(unsigned int c=first[s];c!=none;){unsigned int after=next[c];next[c]=none;parent[c]=p;if(last[p]==none)first[p]=c;else next[last[p]]=c;last[p]=c;c=after;}
   first[s]=last[s]=none;
  }
  ordered=0;depth=0;if(first[root]!=none)stack[depth++]=first[root];
  while(depth){unsigned int i=stack[--depth];if(next[i]!=none)stack[depth++]=next[i];order[ordered++]=i;if(first[i]!=none)stack[depth++]=first[i];}
  return true;
 }
};
// Post-"Applying Shadow Overrides" preorder of a model (ShadowOverride objects
// omitted), for stages that traverse the compiler's tree rather than authored order.
class CompileOrder {
 PrefabScene::Arena memory;
public:
 const PrefabScene::Node** nodes;unsigned int* order;unsigned int count;
 CompileOrder():nodes(0),order(0),count(0){}
 bool Build(const PrefabScene::Model& model){
  memory.Clear();nodes=0;order=0;count=0;if(model.count>100000)return false;
  nodes=(const PrefabScene::Node**)memory.Allocate((model.count+1)*sizeof(*nodes));if(!nodes)return false;
  unsigned int seen=0;for(const PrefabScene::Node* n=model.first;n;n=n->next){if(n->index!=seen||seen>=model.count||(n->parent!=0xffffffffu&&n->parent>=seen))return false;nodes[seen++]=n;}
  return seen==model.count&&Partition::Order(memory,model,nodes,order,count);
 }
};
// Partition references the owned model; destroy it before destroying that model.
// Empty groups are retained for diagnostics, not emitted as serialized WorldModels.
}
