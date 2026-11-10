#pragma once
// Source-only replay of FEAR BAB0/BED0, after ConvertKeyData removes Keys.
// Deliberately preserves BAB0's parent-name duplicate-check quirk.
namespace NativeAINodeLists {
static const unsigned int Missing=0xffffffffu,MaximumNodes=100000,MaximumProperties=4096;
inline bool Exact(const char* a,const char* b){if(!a||!b)return false;while(*a&&*a==*b){a++;b++;}return *a==*b;}
inline bool Equal(const char* a,const char* b){if(!a||!b)return false;for(;;a++,b++){unsigned char x=(unsigned char)*a,y=(unsigned char)*b;if(x>='A'&&x<='Z')x+=32;if(y>='A'&&y<='Z')y+=32;if(x!=y)return false;if(!x)return true;}}
inline bool Length(const char* value,unsigned int maximum,unsigned int& count){if(!value)return false;for(count=0;count<=maximum;count++)if(!value[count])return true;return false;}
inline bool SupportedClass(const char* name){return Exact(name,"AINodeGuard")||Exact(name,"AINodeFollow")||Exact(name,"AINodeSearch")||Exact(name,"AINodeSafety");}
inline bool Key(const PrefabScene::Node& node){return node.kind==2&&Exact(node.className,"Key");}
inline bool Fail(const char*& error,const char* message){error=message;return false;}
inline void SlotName(unsigned int index,char* target){lstrcpyA(target,"Node");char digits[10];unsigned int count=0;do{digits[count++]=(char)('0'+index%10);index/=10;}while(index);for(unsigned int i=0;i<count;i++)target[4+i]=digits[count-1-i];target[4+count]=0;}
// Run exactly once on an authored property copy: the original pass is not
// idempotent when existing child names differ from the parent name.
// target.index retains source identity. Source parent indices must precede their
// children; direct source children remain ordered after leaf Key deletion.
// A relevant non-leaf Key is rejected until post-removal child order is exposed.
// With overflow set, a child without an empty slot ends the pass like the original
// (0x1000BD1F): the earlier children stay written, *overflow=true, result true.
inline bool Apply(PrefabScene::Node& target,const PrefabScene::Node* const* sourceNodes,unsigned int count,PrefabScene::Arena& memory,const char*& error,bool* overflow=0){
 error=0;if(overflow)*overflow=false;if(!sourceNodes||!count||count>MaximumNodes||target.index>=count)return Fail(error,"Invalid AI Node list hierarchy input");
 unsigned int length=0;if(!SupportedClass(target.className)||target.kind!=2)return Fail(error,"Unsupported AI Node list target class");
 PrefabScene::Arena scratch;
 unsigned int* children=(unsigned int*)scratch.Allocate(count*4);if(!children)return Fail(error,"AI Node child lookup allocation failed");memset(children,0,count*4);
 for(unsigned int i=0;i<count;i++){const PrefabScene::Node* n=sourceNodes[i];if(!n||n->index!=i||(n->parent!=Missing&&n->parent>=i)||!Length(n->className,63,length)||!Length(n->name,255,length))return Fail(error,"Invalid ordered AI Node source hierarchy");if(n->parent!=Missing)children[n->parent]++;}
 const PrefabScene::Node& original=*sourceNodes[target.index];if(original.kind!=2||!Exact(original.className,target.className))return Fail(error,"AI Node target source identity mismatch");
 unsigned int propertyCount=0;for(const PrefabScene::Property* p=target.properties;p;p=p->next){if(propertyCount==MaximumProperties||!Length(p->name,255,length)||!Length(p->type,31,length))return Fail(error,"Invalid AI Node property chain");propertyCount++;}
 const PrefabScene::Property** properties=(const PrefabScene::Property**)scratch.Allocate((propertyCount+1)*sizeof(void*));if(!properties)return Fail(error,"AI Node property lookup allocation failed");unsigned int at=0;for(const PrefabScene::Property* p=target.properties;p;p=p->next)properties[at++]=p;
 struct Slot {const PrefabScene::Property* property;const char* value;bool occupied,changed;};Slot* slots=(Slot*)scratch.Allocate((propertyCount+1)*sizeof(Slot));if(!slots)return Fail(error,"AI Node slot allocation failed");unsigned int slotCount=0;
 for(unsigned int index=1;index<=propertyCount+1;index++){char name[20];SlotName(index,name);const PrefabScene::Property* found=0;for(unsigned int p=0;p<propertyCount;p++)if(Exact(properties[p]->name,name)){found=properties[p];break;}if(!found||!Exact(found->type,"string"))break;if(!Length(found->data,4095,length))return Fail(error,"Invalid AI Node string slot");Slot& slot=slots[slotCount++];slot.property=found;slot.value=found->data;slot.occupied=slot.value[0]!=0;slot.changed=false;}
 bool changed=false;
 for(unsigned int i=0;i<count;i++){
  const PrefabScene::Node& child=*sourceNodes[i];if(child.parent!=target.index||child.kind!=2)continue;
  if(Key(child)){if(children[i])return Fail(error,"AI Node lists with non-leaf Key children require post-removal hierarchy preparation");continue;}
  unsigned int empty=Missing;bool parentAlreadyListed=false;
  // Original BAB0 compares occupied slots with the PARENT name at BCAA/BD06,
  // not the child name that is separately fetched only before writing at BD87.
  for(unsigned int s=0;s<slotCount;s++){if(slots[s].occupied){if(Equal(slots[s].value,original.name)){parentAlreadyListed=true;break;}}else if(empty==Missing)empty=s;}
  if(parentAlreadyListed)continue;if(empty==Missing){if(!overflow)return Fail(error,"AI Node has insufficient contiguous Node string slots");*overflow=true;break;}slots[empty].value=child.name;slots[empty].occupied=true;slots[empty].changed=true;changed=true;
 }
 if(!changed)return true;
 PrefabScene::Property* first=0;PrefabScene::Property** tail=&first;
 for(unsigned int p=0;p<propertyCount;p++){PrefabScene::Property* copy=(PrefabScene::Property*)memory.Allocate(sizeof(PrefabScene::Property));if(!copy)return Fail(error,"AI Node property copy allocation failed");*copy=*properties[p];copy->next=0;for(unsigned int s=0;s<slotCount;s++)if(slots[s].property==properties[p]&&slots[s].changed){if(!Length(slots[s].value,255,length))return Fail(error,"Invalid AI Node child name");char* value=(char*)memory.Allocate(length+1);if(!value)return Fail(error,"AI Node generated name allocation failed");memcpy(value,slots[s].value,length+1);copy->data=value;break;}*tail=copy;tail=&copy->next;}
 target.properties=first;return true;
}
// Pass order of 0x1000BED0: AINodeGuard, Follow, Search, Safety, each over the
// global object list (0x1000B540, exact class name). The first overflow logs
// "AINode: %s has too many child Nodes!" and ends every remaining pass; packing
// continues (the caller 0x10029B56 ignores the result). apply[i]=1 for nodes
// that are processed (the overflowing one partially), overflowNode its index.
inline bool Plan(const PrefabScene::Node* const* nodes,unsigned int count,unsigned char* apply,unsigned int& overflowNode,const char*& error){
 static const char* const classes[4]={"AINodeGuard","AINodeFollow","AINodeSearch","AINodeSafety"};
 error=0;overflowNode=Missing;if(!nodes||!apply||count>MaximumNodes)return Fail(error,"Invalid AI Node list plan input");
 for(unsigned int i=0;i<count;i++)apply[i]=0;
 for(unsigned int c=0;c<4&&overflowNode==Missing;c++)for(unsigned int i=0;i<count&&overflowNode==Missing;i++){
  if(!nodes[i]||nodes[i]->kind!=2||!Exact(nodes[i]->className,classes[c]))continue;
  PrefabScene::Node copy=*nodes[i];PrefabScene::Arena scratch;bool overflow=false;
  if(!Apply(copy,nodes,count,scratch,error,&overflow))return false;
  apply[i]=1;if(overflow)overflowNode=i;
 }
 return true;
}
}
