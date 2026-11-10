#define FEAR_PRIMARY_SURFACE_PREPARATION 1
#include "WorldPacker.cpp"
#include "NativeObjectHierarchyLists.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
static unsigned int results[64],used=0;
#define CHECK(name,test) results[used++]=(test)?1u:0u
static void Node(PrefabScene::Node& node,unsigned int index,unsigned int parent,unsigned int kind,const char* cls,const char* name){memset(&node,0,sizeof(node));node.index=index;node.parent=parent;node.kind=kind;lstrcpyA(node.className,cls);lstrcpyA(node.name,name);}
static void Property(PrefabScene::Property& p,const char* name,const char* value,const char* type="string"){memset(&p,0,sizeof(p));lstrcpyA(p.name,name);lstrcpyA(p.type,type);p.string=NativeObjectHierarchyLists::Exact(type,"string");p.data=const_cast<char*>(value);}
static void Chain(PrefabScene::Property* p,unsigned int count){for(unsigned int i=0;i<count;i++)p[i].next=i+1<count?p+i+1:0;}
static const PrefabScene::Property* Get(const PrefabScene::Node& node,const char* name){for(const PrefabScene::Property* p=node.properties;p;p=p->next)if(NativeObjectHierarchyLists::Exact(p->name,name))return p;return 0;}
static bool Value(const PrefabScene::Node& node,const char* name,const char* expected){const PrefabScene::Property* p=Get(node,name);return p&&NativeObjectHierarchyLists::Exact(p->data,expected);}
static bool Same(const void* a,const void* b,unsigned int count){const unsigned char* x=(const unsigned char*)a,*y=(const unsigned char*)b;for(unsigned int i=0;i<count;i++)if(x[i]!=y[i])return false;return true;}
static PrefabScene::Node* AllocateNode(PrefabScene::Arena& memory){void* place=memory.Allocate(sizeof(PrefabScene::Node));if(!place)ExitProcess(2);return new(place)PrefabScene::Node;}
static bool Write(HANDLE file,const void* data,unsigned int bytes){DWORD done=0;return !bytes||(WriteFile(file,data,bytes,&done,0)&&done==bytes);}
static bool Text(HANDLE file,const char* value){unsigned int bytes=lstrlenA(value);return Write(file,&bytes,4)&&Write(file,value,bytes);}
static bool Dump(HANDLE file,const PrefabScene::Node& node){unsigned int count=0;for(const PrefabScene::Property* p=node.properties;p;p=p->next)count++;if(!Write(file,&count,4))return false;for(const PrefabScene::Property* p=node.properties;p;p=p->next)if(!Text(file,p->name)||!Text(file,p->type)||!Text(file,p->data))return false;return true;}
extern "C" void HarnessEntry(){
 using namespace NativeObjectHierarchyLists;PrefabScene::Arena memory;const char* error=0;
 PrefabScene::Node* nodes=(PrefabScene::Node*)memory.Allocate(13*sizeof(PrefabScene::Node));if(!nodes)ExitProcess(2);const PrefabScene::Node* source[13];unsigned char runtime[13];for(unsigned int i=0;i<13;i++){new(nodes+i)PrefabScene::Node;source[i]=nodes+i;runtime[i]=1;}
 Node(nodes[0],0,NoIndex,2,"Group","Outer");Node(nodes[1],1,0,2,"Prop","ChildA");Node(nodes[2],2,0,2,"Group","Nested");Node(nodes[3],3,2,2,"Prop","Inside");Node(nodes[4],4,0,0,"","Folder");Node(nodes[5],5,4,2,"Prop","Deep");Node(nodes[6],6,0,2,"Key","ExcludedKey");runtime[6]=0;Node(nodes[7],7,0,2,"Prop","cHiLdA");Node(nodes[8],8,0,2,"Prop","Authored");Node(nodes[9],9,0,2,"Group","ExcludedGroup");runtime[9]=0;Node(nodes[10],10,9,2,"Prop","OwnedByExcludedGroup");Node(nodes[11],11,0,1,"","Brush");Node(nodes[12],12,NoIndex,2,"Prop","OtherRoot");
 static PrefabScene::Property outerProps[8],innerProps[3];Property(outerProps[0],"Name","Outer");Property(outerProps[1],"Object1","");Property(outerProps[2],"Object2","Authored");Property(outerProps[3],"Object3","");Property(outerProps[4],"Object4","");Property(outerProps[5],"Object5","");Property(outerProps[6],"Object6","");Property(outerProps[7],"Other","Preserved");outerProps[7].flags=0x4000u;Chain(outerProps,8);Property(innerProps[0],"Name","Nested");Property(innerProps[1],"Object1","");Property(innerProps[2],"Object2","");Chain(innerProps,3);nodes[0].properties=outerProps;nodes[2].properties=innerProps;
 PrefabScene::Property* outerBefore=(PrefabScene::Property*)memory.Allocate(sizeof(outerProps));if(!outerBefore)ExitProcess(2);memcpy(outerBefore,outerProps,sizeof(outerProps));PrefabScene::Node& outer=*AllocateNode(memory);PrefabScene::Node& inner=*AllocateNode(memory);outer=nodes[0];inner=nodes[2];
 CHECK("nested Group processes source preorder",ApplyGroup(outer,source,13,runtime,memory,error)&&!error);
 CHECK("first empty slot receives first eligible child",Value(outer,"Object1","ChildA"));
 CHECK("existing nonempty references are preserved",Value(outer,"Object2","Authored"));
 CHECK("nested Group itself belongs to outer Group",Value(outer,"Object3","Nested"));
 CHECK("null hierarchy folders are transparent",Value(outer,"Object4","Deep"));
 CHECK("case-insensitive child duplicates are not repeated",Value(outer,"Object5","")&&Value(outer,"Object6",""));
 CHECK("nested descendants and nonruntime branches do not leak",!Value(outer,"Object5","Inside")&&!Value(outer,"Object5","OwnedByExcludedGroup"));
 CHECK("Group output stages a separate property chain",outer.properties!=nodes[0].properties);
 CHECK("source properties and links remain immutable",Same(outerBefore,outerProps,sizeof(outerProps))&&Value(nodes[0],"Object1","")&&Value(nodes[2],"Object1",""));
 CHECK("unrelated property text and flags are preserved",Value(outer,"Other","Preserved")&&Get(outer,"Other")->flags==0x4000u);
 CHECK("nested Group owns its own descendants",ApplyGroup(inner,source,13,runtime,memory,error)&&Value(inner,"Object1","Inside")&&Value(inner,"Object2",""));
 PrefabScene::Property* retained=outer.properties;CHECK("repeat application is idempotent without republishing",ApplyGroup(outer,source,13,runtime,memory,error)&&outer.properties==retained&&!error);
 PrefabScene::Node* small=(PrefabScene::Node*)memory.Allocate(3*sizeof(PrefabScene::Node));if(!small)ExitProcess(2);for(unsigned int i=0;i<3;i++)new(small+i)PrefabScene::Node;const PrefabScene::Node* smallSource[3]={small,small+1,small+2};unsigned char smallRuntime[3]={1,1,1};Node(small[0],0,NoIndex,2,"gRoUp","Small");Node(small[1],1,0,2,"Prop","One");Node(small[2],2,0,2,"Prop","Two");
 static PrefabScene::Property props[4];Property(props[0],"Object3","");Property(props[1],"Object2","");Property(props[2],"Object1","");Property(props[3],"Other","untouched");Chain(props,4);small[0].properties=props;PrefabScene::Node& target=*AllocateNode(memory);target=small[0];
 CHECK("class comparison is case-insensitive",ApplyGroup(target,smallSource,3,smallRuntime,memory,error)&&Value(target,"Object1","One"));
 CHECK("slot lookup follows numeric names not property order",Value(target,"Object2","Two")&&Value(target,"Object3",""));
 Property(props[0],"Object1","Two");Property(props[1],"Object2","");Chain(props,2);target=small[0];
 CHECK("existing later child is preserved before filling earlier child",ApplyGroup(target,smallSource,3,smallRuntime,memory,error)&&Value(target,"Object1","Two")&&Value(target,"Object2","One"));
 Property(props[0],"Object1","one");Property(props[1],"Object2","TWO");Chain(props,2);target=small[0];retained=target.properties;
 CHECK("case-insensitive authored matches need no empty slot",ApplyGroup(target,smallSource,3,smallRuntime,memory,error)&&target.properties==retained);
 Property(props[0],"Object1","");Property(props[1],"Object3","");Chain(props,2);target=small[0];retained=target.properties;
 CHECK("missing intermediate slot ends the sequence and fails overflow",!ApplyGroup(target,smallSource,3,smallRuntime,memory,error)&&error&&target.properties==retained&&Value(target,"Object1",""));
 Property(props[0],"Object01","");Chain(props,1);target=small[0];
 CHECK("zero-padded slots are not canonical Object1",!ApplyGroup(target,smallSource,2,smallRuntime,memory,error)&&Value(target,"Object01",""));
 Property(props[0],"object1","");target=small[0];CHECK("property lookup is case-sensitive",!ApplyGroup(target,smallSource,2,smallRuntime,memory,error));
 Property(props[0],"Object1","7","real");Property(props[1],"Object2","");Chain(props,2);target=small[0];
 CHECK("nonstring slot is skipped without terminating Group scan",ApplyGroup(target,smallSource,2,smallRuntime,memory,error)&&Value(target,"Object1","7")&&Value(target,"Object2","One"));
 Property(props[0],"Object1","");Chain(props,1);target=small[0];smallRuntime[1]=smallRuntime[2]=0;
 CHECK("only CF_NORUNTIME-excluded descendants need no slots",ApplyGroup(target,smallSource,3,smallRuntime,memory,error)&&Value(target,"Object1",""));smallRuntime[1]=smallRuntime[2]=1;
 small[1].kind=1;target=small[0];CHECK("brush children do not enter Group lists",ApplyGroup(target,smallSource,2,smallRuntime,memory,error)&&Value(target,"Object1",""));small[1].kind=2;
 Property(props[0],"Object1","One");Property(props[1],"Object2","Two");Chain(props,2);target=small[0];CHECK("full but already complete list succeeds",ApplyGroup(target,smallSource,3,smallRuntime,memory,error));
 small[1].parent=1;target=small[0];CHECK("self-parent cycle is rejected",!ApplyGroup(target,smallSource,3,smallRuntime,memory,error)&&error);small[1].parent=0;
 small[0].parent=2;target=small[0];CHECK("forward ancestor cycle is rejected",!ApplyGroup(target,smallSource,3,smallRuntime,memory,error)&&error);small[0].parent=NoIndex;
 small[1].parent=99;target=small[0];CHECK("out-of-range parent is rejected",!ApplyGroup(target,smallSource,3,smallRuntime,memory,error));small[1].parent=0;
 smallSource[1]=0;target=small[0];CHECK("missing source node is rejected",!ApplyGroup(target,smallSource,3,smallRuntime,memory,error));smallSource[1]=small+1;
 small[1].index=2;CHECK("source index mismatch is rejected",!ApplyGroup(target,smallSource,3,smallRuntime,memory,error));small[1].index=1;
 smallRuntime[1]=2;CHECK("runtime mask outside zero and one is rejected",!ApplyGroup(target,smallSource,3,smallRuntime,memory,error));smallRuntime[1]=1;
 CHECK("null runtime mask is rejected",!ApplyGroup(target,smallSource,3,0,memory,error));
 CHECK("null source array and empty source are rejected",!ApplyGroup(target,0,3,smallRuntime,memory,error)&&!ApplyGroup(target,smallSource,0,smallRuntime,memory,error));
 CHECK("oversized source count is rejected before dereference",!ApplyGroup(target,smallSource,MaximumNodes+1,smallRuntime,memory,error));
 target=small[0];target.index=3;CHECK("target source index is checked",!ApplyGroup(target,smallSource,3,smallRuntime,memory,error));target=small[0];lstrcpyA(target.className,"Prop");CHECK("target must be a Group",!ApplyGroup(target,smallSource,3,smallRuntime,memory,error));
 target=small[0];props[1].next=props;CHECK("property cycle is bounded before mutation",!ApplyGroup(target,smallSource,3,smallRuntime,memory,error)&&target.properties==props);props[1].next=0;
 props[0].data=0;target=small[0];CHECK("null string slot is rejected",!ApplyGroup(target,smallSource,3,smallRuntime,memory,error));props[0].data=const_cast<char*>("One");
 target=small[0];CHECK("helper recovers after invalid inputs",ApplyGroup(target,smallSource,3,smallRuntime,memory,error)&&!error);
 char path[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_GROUP_GUARDS_OUTPUT",path,MAX_PATH))ExitProcess(1);HANDLE file=CreateFileA(path,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);unsigned int header[3]={0x31524748,used,2};if(file==INVALID_HANDLE_VALUE||!Write(file,header,12)||!Write(file,results,used*4)||!Dump(file,outer)||!Dump(file,inner))ExitProcess(3);CloseHandle(file);for(unsigned int i=0;i<used;i++)if(!results[i])ExitProcess(4);ExitProcess(0);
}
