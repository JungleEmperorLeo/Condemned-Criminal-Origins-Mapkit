#pragma once
// FEAR Group hierarchy preprocessing, RVA 1BAD0/1BBE0. This source-only helper
// runs before Key removal. runtimeMask contains the direct CF_NORUNTIME policy.
// It changes only a staged owned property chain, publishing after full success.
namespace NativeObjectHierarchyLists {
static const unsigned int NoIndex=0xffffffffu,MaximumNodes=100000,MaximumProperties=4096;
inline bool Exact(const char* a,const char* b){if(!a||!b)return false;while(*a&&*a==*b){a++;b++;}return *a==*b;}
inline bool Equal(const char* a,const char* b){if(!a||!b)return false;for(;;a++,b++){unsigned char x=(unsigned char)*a,y=(unsigned char)*b;if(x>='A'&&x<='Z')x+=32;if(y>='A'&&y<='Z')y+=32;if(x!=y)return false;if(!x)return true;}}
inline bool Length(const char* text,unsigned int limit,unsigned int& length){if(!text)return false;for(length=0;length<=limit;length++)if(!text[length])return true;return false;}
inline bool Group(const PrefabScene::Node& node){return node.kind==2&&Equal(node.className,"Group");}
inline void SlotName(unsigned int index,char* name){lstrcpyA(name,"Object");char digits[10];unsigned int count=0;do{digits[count++]=(char)('0'+index%10);index/=10;}while(index);for(unsigned int i=0;i<count;i++)name[6+i]=digits[count-1-i];name[6+count]=0;}
inline bool Fail(const char*& error,const char* message){error=message;return false;}
// A full Group (no empty string Object slot before the first missing one) only
// reports "There was not enough room in group object %s to add the object %s"
// (0x1001BB9F, logged by the compiler) and the child is not added.
inline bool ApplyGroup(PrefabScene::Node& target,const PrefabScene::Node* const* sourceNodes,unsigned int count,const unsigned char* runtimeMask,PrefabScene::Arena& memory,const char*& error){
 error=0;if(!sourceNodes||!runtimeMask||!count||count>MaximumNodes||target.index>=count)return Fail(error,"Invalid Group hierarchy input");
 for(unsigned int i=0;i<count;i++){const PrefabScene::Node* n=sourceNodes[i];unsigned int length;if(!n||n->index!=i||(n->parent!=NoIndex&&n->parent>=i)||runtimeMask[i]>1||!Length(n->className,63,length)||!Length(n->name,255,length))return Fail(error,"Invalid ordered Group hierarchy or runtime mask");}
 if(!Group(target)||!Group(*sourceNodes[target.index]))return Fail(error,"Group preprocessing requires a source Group target");
 PrefabScene::Arena scratch;
 unsigned int* enclosingGroup=(unsigned int*)scratch.Allocate(count*4);if(!enclosingGroup)return Fail(error,"Group ancestor lookup allocation failed");
 for(unsigned int i=0;i<count;i++){unsigned int parent=sourceNodes[i]->parent;enclosingGroup[i]=parent==NoIndex?NoIndex:Group(*sourceNodes[parent])?parent:enclosingGroup[parent];}
 unsigned int propertyCount=0;
 for(const PrefabScene::Property* p=target.properties;p;p=p->next){unsigned int length;if(propertyCount==MaximumProperties||!Length(p->name,255,length)||!Length(p->type,31,length))return Fail(error,"Invalid or oversized Group property chain");propertyCount++;}
 const PrefabScene::Property** properties=(const PrefabScene::Property**)scratch.Allocate((propertyCount+1)*sizeof(void*));if(!properties)return Fail(error,"Group property lookup allocation failed");unsigned int copied=0;for(const PrefabScene::Property* p=target.properties;p;p=p->next)properties[copied++]=p;
 struct Slot {const PrefabScene::Property* property;const char* value;};
 Slot* slots=(Slot*)scratch.Allocate((propertyCount+1)*sizeof(Slot));if(!slots)return Fail(error,"Group slot allocation failed");unsigned int slotCount=0;
 for(unsigned int index=1;index<=propertyCount+1;index++){char name[20];SlotName(index,name);const PrefabScene::Property* found=0;for(unsigned int p=0;p<propertyCount;p++)if(Exact(properties[p]->name,name)){found=properties[p];break;}if(!found)break;
  Slot& slot=slots[slotCount++];slot.property=found;slot.value=0;
  if(Exact(found->type,"string")){unsigned int length;if(!Length(found->data,4095,length))return Fail(error,"Invalid Group string slot");slot.value=found->data;}
 }
 bool changed=false;
 for(unsigned int i=0;i<count;i++){
  const PrefabScene::Node& candidate=*sourceNodes[i];if(candidate.kind!=2||!runtimeMask[i]||i==target.index)continue;
  if(enclosingGroup[i]!=target.index)continue;unsigned int empty=NoIndex;bool exists=false;
  for(unsigned int s=0;s<slotCount;s++){if(!slots[s].value)continue;if(Equal(slots[s].value,candidate.name)){exists=true;break;}if(empty==NoIndex&&!slots[s].value[0])empty=s;}
  if(exists||empty==NoIndex)continue;slots[empty].value=candidate.name;changed=true;
 }
 if(!changed)return true;
 PrefabScene::Property* first=0;PrefabScene::Property** tail=&first;
 for(unsigned int p=0;p<propertyCount;p++){
  PrefabScene::Property* copy=(PrefabScene::Property*)memory.Allocate(sizeof(PrefabScene::Property));if(!copy)return Fail(error,"Group property copy allocation failed");*copy=*properties[p];copy->next=0;
  for(unsigned int s=0;s<slotCount;s++)if(slots[s].property==properties[p]&&slots[s].value&&slots[s].value!=properties[p]->data){unsigned int length;if(!Length(slots[s].value,255,length))return Fail(error,"Invalid Group generated object name");char* value=(char*)memory.Allocate(length+1);if(!value)return Fail(error,"Group generated name allocation failed");memcpy(value,slots[s].value,length+1);copy->data=value;break;}
  *tail=copy;tail=&copy->next;
 }
 target.properties=first;return true;
}
// Packer-log pass of "Filling in Group Objects" (0x1001BBE0): pre-order, every
// runtime object (runtime[i]) below a Group is offered to its nearest enclosing
// Group; sink(group, object) for each one that finds no slot.
inline bool Overflows(const PrefabScene::Model& model,const unsigned char* runtime,void (*sink)(void*,const char*,const char*),void* context){
 if(!model.count||model.count>MaximumNodes||!runtime)return false;PrefabScene::Arena scratch;
 const PrefabScene::Node** nodes=(const PrefabScene::Node**)scratch.Allocate(model.count*sizeof(void*));unsigned int* enclosing=(unsigned int*)scratch.Allocate(model.count*4);const char*** values=(const char***)scratch.Allocate(model.count*sizeof(void*));unsigned int* slotCount=(unsigned int*)scratch.Allocate(model.count*4);
 if(!nodes||!enclosing||!values||!slotCount)return false;unsigned int seen=0;
 for(const PrefabScene::Node* n=model.first;n;n=n->next){if(n->index!=seen||seen>=model.count||(n->parent!=NoIndex&&n->parent>=seen))return false;nodes[seen]=n;values[seen]=0;slotCount[seen]=0;unsigned int p=n->parent;enclosing[seen]=p==NoIndex?NoIndex:Group(*nodes[p])?p:enclosing[p];seen++;}
 for(unsigned int i=0;i<seen;i++){const PrefabScene::Node& o=*nodes[i];const unsigned int g=enclosing[i];if(o.kind!=2||!runtime[i]||g==NoIndex)continue;
  if(!values[g]){unsigned int properties=0;for(const PrefabScene::Property* p=nodes[g]->properties;p&&properties<MaximumProperties;p=p->next)properties++;values[g]=(const char**)scratch.Allocate((properties+1)*sizeof(char*));if(!values[g])return false;
   for(unsigned int index=1;index<=properties+1;index++){char name[20];SlotName(index,name);const PrefabScene::Property* found=0;for(const PrefabScene::Property* p=nodes[g]->properties;p;p=p->next)if(Exact(p->name,name)){found=p;break;}if(!found)break;values[g][slotCount[g]++]=Exact(found->type,"string")&&found->data?found->data:0;}}
  unsigned int empty=NoIndex;bool exists=false;for(unsigned int s=0;s<slotCount[g];s++){const char* v=values[g][s];if(!v)continue;if(Equal(v,o.name)){exists=true;break;}if(empty==NoIndex&&!v[0])empty=s;}
  if(exists)continue;if(empty==NoIndex){sink(context,nodes[g]->name,o.name);continue;}values[g][empty]=o.name;}
 return true;
}
// AINodeGuard/Follow/Search/Safety use a separate direct-child pass after Keys
// are removed. That API is intentionally not claimed by this Group-only helper.
}
