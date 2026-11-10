#define FEAR_PRIMARY_SURFACE_PREPARATION 1
#include "WorldPacker.cpp"
#include "NativeAINodeLists.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
static unsigned int results[80],used=0;
#define CHECK(name,test) results[used++]=(test)?1u:0u
static void Node(PrefabScene::Node& n,unsigned int i,unsigned int parent,unsigned int kind,const char* cls,const char* name){memset(&n,0,sizeof(n));n.index=i;n.parent=parent;n.kind=kind;lstrcpyA(n.className,cls);lstrcpyA(n.name,name);}
static PrefabScene::Node* AllocateNodes(PrefabScene::Arena& memory,unsigned int count){void* place=memory.Allocate(count*sizeof(PrefabScene::Node));if(!place)ExitProcess(2);PrefabScene::Node* nodes=(PrefabScene::Node*)place;for(unsigned int i=0;i<count;i++)new(nodes+i)PrefabScene::Node;return nodes;}
static void Prop(PrefabScene::Property& p,const char* name,const char* value,const char* type="string"){memset(&p,0,sizeof(p));lstrcpyA(p.name,name);lstrcpyA(p.type,type);p.string=NativeAINodeLists::Exact(type,"string");p.data=const_cast<char*>(value);}
static void Chain(PrefabScene::Property* p,unsigned int count){for(unsigned int i=0;i<count;i++)p[i].next=i+1<count?p+i+1:0;}
static const PrefabScene::Property* Get(const PrefabScene::Node& n,const char* name){for(const PrefabScene::Property* p=n.properties;p;p=p->next)if(NativeAINodeLists::Exact(p->name,name))return p;return 0;}
static bool Value(const PrefabScene::Node& n,const char* name,const char* value){const PrefabScene::Property* p=Get(n,name);return p&&NativeAINodeLists::Exact(p->data,value);}
static bool Same(const void* a,const void* b,unsigned int count){const unsigned char* x=(const unsigned char*)a,*y=(const unsigned char*)b;for(unsigned int i=0;i<count;i++)if(x[i]!=y[i])return false;return true;}
static bool Write(HANDLE f,const void* p,unsigned int n){DWORD done=0;return !n||(WriteFile(f,p,n,&done,0)&&done==n);}
static bool Text(HANDLE f,const char* p){unsigned int n=lstrlenA(p);return Write(f,&n,4)&&Write(f,p,n);}
static bool Dump(HANDLE f,const PrefabScene::Node& n){unsigned int count=0;for(const PrefabScene::Property* p=n.properties;p;p=p->next)count++;if(!Write(f,&count,4))return false;for(const PrefabScene::Property* p=n.properties;p;p=p->next)if(!Text(f,p->name)||!Text(f,p->type)||!Text(f,p->data))return false;return true;}
extern "C" void HarnessEntry(){
 using namespace NativeAINodeLists;PrefabScene::Arena memory;const char* error=0;
 CHECK("dispatch supports exactly four recovered AI classes",SupportedClass("AINodeGuard")&&SupportedClass("AINodeFollow")&&SupportedClass("AINodeSearch")&&SupportedClass("AINodeSafety"));
 CHECK("dispatch class matching is exact and case sensitive",!SupportedClass("ainodeguard")&&!SupportedClass("AINodeStalk")&&!SupportedClass("AINodeGuardDerived")&&!SupportedClass(0));
 PrefabScene::Node* nodes=AllocateNodes(memory,10);const PrefabScene::Node* source[10];for(unsigned int i=0;i<10;i++)source[i]=nodes+i;
 Node(nodes[0],0,Missing,2,"AINodeGuard","Guard");Node(nodes[1],1,0,2,"AINode","DirectOne");Node(nodes[2],2,1,2,"AINode","Grandchild");Node(nodes[3],3,0,0,"","Folder");Node(nodes[4],4,3,2,"AINode","UnderFolder");Node(nodes[5],5,0,2,"Key","RemovedKey");Node(nodes[6],6,0,1,"","Brush");Node(nodes[7],7,0,2,"NonRuntimeClass","NonRuntimeDirect");Node(nodes[8],8,0,2,"Prop","OtherClass");Node(nodes[9],9,Missing,2,"AINode","UnrelatedRoot");
 static PrefabScene::Property properties[7];Prop(properties[0],"Name","Guard");Prop(properties[1],"Node3","");Prop(properties[2],"Node1","Authored");Prop(properties[3],"Node2","");Prop(properties[4],"Node4","");Prop(properties[5],"Node5","");Prop(properties[6],"Other","Preserved");properties[6].flags=0x4000;Chain(properties,7);nodes[0].properties=properties;PrefabScene::Property* before=(PrefabScene::Property*)memory.Allocate(sizeof(properties));if(!before)ExitProcess(2);memcpy(before,properties,sizeof(properties));PrefabScene::Node& target=*AllocateNodes(memory,1);target=nodes[0];
 CHECK("direct children fill numeric slots independent of property order",Apply(target,source,10,memory,error)&&Value(target,"Node2","DirectOne")&&Value(target,"Node3","NonRuntimeDirect")&&Value(target,"Node4","OtherClass"));
 CHECK("authored nonempty slot remains unchanged",Value(target,"Node1","Authored"));
 CHECK("leaf Key brush folders and deeper objects are excluded",Value(target,"Node5","")&&!Value(target,"Node5","Grandchild")&&!Value(target,"Node5","UnderFolder"));
 CHECK("nonruntime and non-AI direct objects are not filtered",Value(target,"Node3","NonRuntimeDirect")&&Value(target,"Node4","OtherClass"));
 CHECK("unrelated properties and flags survive",Value(target,"Other","Preserved")&&Get(target,"Other")->flags==0x4000);
 CHECK("source properties and links remain unchanged",Same(properties,before,sizeof(properties))&&Value(nodes[0],"Node2","")&&target.properties!=nodes[0].properties);
 PrefabScene::Node& retained=*AllocateNodes(memory,1);retained=target;
 PrefabScene::Node* small=AllocateNodes(memory,3);const PrefabScene::Node* input[3]={small,small+1,small+2};Node(small[0],0,Missing,2,"AINodeFollow","Parent");Node(small[1],1,0,2,"AINode","One");Node(small[2],2,0,2,"AINode","Two");static PrefabScene::Property p[5];
 Prop(p[0],"Node1","One");Prop(p[1],"Node2","");Prop(p[2],"Node3","");Chain(p,3);small[0].properties=p;target=small[0];
 CHECK("existing child reference does not deduplicate original parent-name quirk",Apply(target,input,2,memory,error)&&Value(target,"Node1","One")&&Value(target,"Node2","One"));
 CHECK("reapplying the original pass is deliberately not idempotent",Apply(target,input,2,memory,error)&&Value(target,"Node3","One"));
 Prop(p[0],"Node1","pArEnT");Prop(p[1],"Node2","");Chain(p,2);target=small[0];PrefabScene::Property* originalProperties=target.properties;
 CHECK("case-insensitive parent name suppresses all child insertion",Apply(target,input,3,memory,error)&&target.properties==originalProperties&&Value(target,"Node2",""));
 Prop(p[0],"Node1","");Prop(p[1],"Node2","PARENT");Chain(p,2);target=small[0];
 CHECK("later parent match wins over an earlier empty slot",Apply(target,input,3,memory,error)&&Value(target,"Node1",""));
 Prop(p[0],"Node1","");Prop(p[1],"Node2","");Prop(p[2],"Node3","");Chain(p,3);target=small[0];lstrcpyA(small[2].name,"One");
 CHECK("duplicate child names consume separate empty slots",Apply(target,input,3,memory,error)&&Value(target,"Node1","One")&&Value(target,"Node2","One"));lstrcpyA(small[2].name,"Two");
 small[1].name[0]=0;target=small[0];
 CHECK("empty child name still consumes an occupied slot",Apply(target,input,3,memory,error)&&Value(target,"Node1","")&&Value(target,"Node2","Two"));
 small[0].name[0]=0;target=small[0];
 CHECK("inserted empty name can then match empty parent name",Apply(target,input,3,memory,error)&&Value(target,"Node1","")&&Value(target,"Node2",""));lstrcpyA(small[0].name,"Parent");lstrcpyA(small[1].name,"One");
 lstrcpyA(small[1].name,"Parent");target=small[0];
 CHECK("child equal to parent name suppresses later children after insertion",Apply(target,input,3,memory,error)&&Value(target,"Node1","Parent")&&Value(target,"Node2",""));lstrcpyA(small[1].name,"One");
 Prop(p[0],"Node1","");Prop(p[1],"Node3","");Chain(p,2);target=small[0];originalProperties=target.properties;
 CHECK("missing intermediate slot ends sequence and overflow is atomic",!Apply(target,input,3,memory,error)&&error&&target.properties==originalProperties&&Value(target,"Node1",""));
 Prop(p[0],"Node01","");Chain(p,1);target=small[0];CHECK("zero-padded property is not Node1",!Apply(target,input,2,memory,error));
 Prop(p[0],"node1","");target=small[0];CHECK("slot property names are case sensitive",!Apply(target,input,2,memory,error));
 Prop(p[0],"Node1","7","real");Prop(p[1],"Node2","");Chain(p,2);target=small[0];CHECK("wrong type at first Node slot ends sequence",!Apply(target,input,2,memory,error)&&Value(target,"Node2",""));
 Prop(p[0],"Node1","");Prop(p[1],"Node2","7","real");Prop(p[2],"Node3","");Chain(p,3);target=small[0];CHECK("later wrong type terminates rather than skips",!Apply(target,input,3,memory,error)&&Value(target,"Node1",""));
 Prop(p[0],"Node1","Different");Chain(p,1);target=small[0];CHECK("full unmatched list rejects instead of silently dropping child",!Apply(target,input,2,memory,error)&&Value(target,"Node1","Different"));
 Prop(p[0],"Node1","Parent");target=small[0];CHECK("full parent-listed list needs no free slot",Apply(target,input,3,memory,error));
 small[0].properties=0;target=small[0];CHECK("no slots with no direct children is valid",Apply(target,input,1,memory,error));CHECK("no slots with a child is overflow",!Apply(target,input,2,memory,error));small[0].properties=p;
 Prop(p[0],"Node1","");Prop(p[1],"Node2","");Chain(p,2);target=small[0];small[1].kind=1;CHECK("brush direct child requires no slot",Apply(target,input,2,memory,error)&&Value(target,"Node1",""));small[1].kind=2;
 lstrcpyA(small[1].className,"Key");target=small[0];CHECK("exact leaf Key is skipped after key conversion",Apply(target,input,2,memory,error)&&Value(target,"Node1",""));
 lstrcpyA(small[1].className,"key");target=small[0];CHECK("wrong-case Key class is not removed",Apply(target,input,2,memory,error)&&Value(target,"Node1","One"));
 lstrcpyA(small[1].className,"Key");small[2].parent=1;target=small[0];originalProperties=target.properties;CHECK("non-leaf Key reparenting profile is explicitly rejected atomically",!Apply(target,input,3,memory,error)&&target.properties==originalProperties&&Value(target,"Node1",""));small[2].parent=0;lstrcpyA(small[1].className,"AINode");
 const char* classes[4]={"AINodeGuard","AINodeFollow","AINodeSearch","AINodeSafety"};bool all=true;for(unsigned int i=0;i<4;i++){lstrcpyA(small[0].className,classes[i]);target=small[0];if(!Apply(target,input,2,memory,error)||!Value(target,"Node1","One"))all=false;}CHECK("all four dispatched classes use the same fill behavior",all);
 lstrcpyA(small[0].className,"ainodeguard");target=small[0];CHECK("wrong-case dispatched class is outside exact contract",!Apply(target,input,2,memory,error));lstrcpyA(small[0].className,"AINodeSafety");target=small[0];
 small[1].parent=1;CHECK("self-parent cycle is rejected",!Apply(target,input,3,memory,error));small[1].parent=0;small[0].parent=2;CHECK("forward-parent hierarchy is rejected",!Apply(target,input,3,memory,error));small[0].parent=Missing;
 small[1].parent=99;CHECK("out-of-range parent is rejected",!Apply(target,input,3,memory,error));small[1].parent=0;input[1]=0;CHECK("missing source node is rejected",!Apply(target,input,3,memory,error));input[1]=small+1;
 small[1].index=2;CHECK("source index mismatch is rejected",!Apply(target,input,3,memory,error));small[1].index=1;
 CHECK("null source array and zero count are rejected",!Apply(target,0,3,memory,error)&&!Apply(target,input,0,memory,error));CHECK("oversized node count rejected before dereference",!Apply(target,input,MaximumNodes+1,memory,error));
 target.index=3;CHECK("target identity index is checked",!Apply(target,input,3,memory,error));target=small[0];lstrcpyA(target.className,"AINodeGuard");CHECK("target class must match source identity",!Apply(target,input,3,memory,error));target=small[0];
 p[1].next=p;CHECK("property cycle is bounded and rejected",!Apply(target,input,3,memory,error)&&target.properties==p);p[1].next=0;p[0].data=0;CHECK("null slot string is rejected",!Apply(target,input,3,memory,error));p[0].data=const_cast<char*>("");
 char* longText=(char*)memory.Allocate(4097);if(!longText)ExitProcess(2);memset(longText,'x',4096);longText[4096]=0;p[0].data=longText;CHECK("slot string above ordinary object budget rejected",!Apply(target,input,3,memory,error));longText[4095]=0;CHECK("4095 byte authored slot remains valid",Apply(target,input,1,memory,error));p[0].data=const_cast<char*>("");
 memset(small[1].name,'x',sizeof(small[1].name));CHECK("unterminated child name is rejected",!Apply(target,input,3,memory,error));lstrcpyA(small[1].name,"One");target=small[0];CHECK("helper recovers after failed controls",Apply(target,input,3,memory,error)&&!error&&Value(target,"Node1","One")&&Value(target,"Node2","Two"));
 // Original overflow (0x1000BD1F): earlier children stay written, the pass ends.
 Prop(p[0],"Node1","");Prop(p[1],"Node3","");Chain(p,2);target=small[0];{bool overflow=false;CHECK("overflow mode keeps earlier children and reports overflow",Apply(target,input,3,memory,error,&overflow)&&overflow&&Value(target,"Node1","One"));}
 Prop(p[0],"Node1","");Prop(p[1],"Node2","");Chain(p,2);target=small[0];{bool overflow=true;CHECK("overflow mode without overflow clears the flag",Apply(target,input,3,memory,error,&overflow)&&!overflow&&Value(target,"Node2","Two"));}
 {PrefabScene::Node* plan=AllocateNodes(memory,6);const PrefabScene::Node* planInput[6]={plan,plan+1,plan+2,plan+3,plan+4,plan+5};static PrefabScene::Property q[2];Prop(q[0],"Node1","");Chain(q,1);
  Node(plan[0],0,Missing,2,"AINodeSafety","Safe");Node(plan[1],1,Missing,2,"AINodeGuard","Full");Node(plan[2],2,1,2,"AINode","A");Node(plan[3],3,1,2,"AINode","B");Node(plan[4],4,Missing,2,"AINodeGuard","Later");Node(plan[5],5,Missing,2,"AINodeSearch","Search");
  plan[0].properties=q;plan[1].properties=q;plan[4].properties=q;plan[5].properties=q;unsigned char apply[6];unsigned int overflowNode=0;
  CHECK("plan stops at the first overflow in class order",Plan(planInput,6,apply,overflowNode,error)&&overflowNode==1&&apply[1]==1&&apply[4]==0&&apply[5]==0&&apply[0]==0);
  plan[3].parent=Missing;CHECK("plan without overflow processes every class",Plan(planInput,6,apply,overflowNode,error)&&overflowNode==Missing&&apply[0]&&apply[1]&&apply[4]&&apply[5]);}
 target=small[0];Prop(p[0],"Node1","");Prop(p[1],"Node2","");Chain(p,2);target=small[0];if(!Apply(target,input,3,memory,error))ExitProcess(5);
 char output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_AI_NODE_LIST_GUARDS_OUTPUT",output,MAX_PATH))ExitProcess(1);HANDLE file=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);unsigned int header[3]={0x31474c41,used,2};if(file==INVALID_HANDLE_VALUE||!Write(file,header,12)||!Write(file,results,used*4)||!Dump(file,retained)||!Dump(file,target))ExitProcess(3);CloseHandle(file);for(unsigned int i=0;i<used;i++)if(!results[i])ExitProcess(4);ExitProcess(0);
}
