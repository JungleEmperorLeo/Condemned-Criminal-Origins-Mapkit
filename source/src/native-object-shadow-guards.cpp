#define FEAR_PRIMARY_SURFACE_PREPARATION 1
#include "WorldPacker.cpp"
#include "NativeObjectShadow.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
static unsigned int results[64],used=0;
#define CHECK(name,test) results[used++]=(test)?1u:0u
static void Property(PrefabScene::Property& p,const char* name,const char* value,const char* type="string"){memset(&p,0,sizeof(p));lstrcpyA(p.name,name);lstrcpyA(p.type,type);p.string=NativeObjectShadow::Exact(type,"string");p.data=const_cast<char*>(value);}
static const PrefabScene::Property* Get(const PrefabScene::Node& n,const char* name){return NativeObjectShadow::Property(n,name);}
static bool Value(const PrefabScene::Node& n,const char* name,const char* expected){const PrefabScene::Property* p=Get(n,name);return p&&NativeObjectShadow::Exact(p->data,expected);}
static bool Same(const void* a,const void* b,unsigned int n){const unsigned char* x=(const unsigned char*)a,*y=(const unsigned char*)b;for(unsigned int i=0;i<n;i++)if(x[i]!=y[i])return false;return true;}
static PrefabScene::Node* Allocate(PrefabScene::Arena& memory){void* place=memory.Allocate(sizeof(PrefabScene::Node));if(!place)ExitProcess(2);return new(place)PrefabScene::Node;}
static bool Write(HANDLE file,const void* data,unsigned int bytes){DWORD done=0;return !bytes||(WriteFile(file,data,bytes,&done,0)&&done==bytes);}
static bool Text(HANDLE file,const char* value){unsigned int n=lstrlenA(value);return Write(file,&n,4)&&Write(file,value,n);}
static bool Dump(HANDLE file,const PrefabScene::Node& n){unsigned int count=0;for(const PrefabScene::Property* p=n.properties;p;p=p->next)count++;if(!Write(file,&count,4))return false;for(const PrefabScene::Property* p=n.properties;p;p=p->next)if(!Text(file,p->name)||!Text(file,p->type)||!Text(file,p->data))return false;return true;}
extern "C" void HarnessEntry(){
 using namespace NativeObjectShadow;PrefabScene::Arena memory;PrefabScene::Model model;PrefabScene::Node* nodes[19];
 const unsigned int parents[19]={NoIndex,0,1,2,1,4,1,6,1,8,0,NoIndex,0,12,0,14,0,16,0};
 for(unsigned int i=0;i<19;i++){nodes[i]=model.Add();if(!nodes[i])ExitProcess(2);nodes[i]->parent=parents[i];nodes[i]->kind=2;lstrcpyA(nodes[i]->className,"DynamicWorldModel");}
 lstrcpyA(nodes[0]->className,"ShadowOverride");nodes[1]->kind=0;nodes[1]->className[0]=0;lstrcpyA(nodes[4]->className,"Ordinary");lstrcpyA(nodes[6]->className,"ShadowOverride");lstrcpyA(nodes[8]->className,"ShadowOverride");lstrcpyA(nodes[12]->className,"UnflaggedDerived");lstrcpyA(nodes[14]->className,"ShadowOverride");nodes[16]->kind=1;nodes[16]->className[0]=0;
 static PrefabScene::Property outer[2],clearProp,innerProp,mediumProp,owner[3];char outerValue[]="Never";Property(outer[0],"BrushOverride","High");Property(outer[1],"WMOverride",outerValue);outer[0].next=outer+1;nodes[0]->properties=outer;Property(clearProp,"WMOverride","No Override");nodes[6]->properties=&clearProp;Property(innerProp,"WMOverride","High");nodes[8]->properties=&innerProp;Property(mediumProp,"WMOverride","Medium");nodes[14]->properties=&mediumProp;
 Property(owner[0],"Name","ZeroBrushOwner");Property(owner[1],"ShadowLOD","Low");Property(owner[2],"Other","Preserved");owner[0].next=owner+1;owner[1].next=owner+2;owner[1].flags=0x4000u;nodes[2]->properties=owner;
 GameClassMetadata::Catalog catalog;GameClassMetadata::Class definitions[4];memset(definitions,0,sizeof(definitions));definitions[0].name="DynamicWorldModel";definitions[0].flags=0x10;definitions[1].name="Ordinary";definitions[2].name="UnflaggedDerived";definitions[2].parentIndex=0;definitions[2].parentName="DynamicWorldModel";definitions[3].name="ShadowOverride";definitions[3].flags=0x10;catalog.classes=definitions;catalog.count=4;
 Policy policy;CHECK("ordered source policy builds without a geometry partition",policy.Build(model,catalog)&&policy.count==19&&!policy.error);
 CHECK("zero brush custom WorldModel receives inherited selector",policy.worldModelLOD[2]&&Exact(policy.worldModelLOD[2],"Never"));
 CHECK("WorldModel consumes WM override for its descendants",policy.worldModelLOD[3]==0);
 CHECK("ordinary objects do not consume inherited WM override",policy.worldModelLOD[4]==0&&Exact(policy.worldModelLOD[5],"Never"));
 CHECK("nested No Override clears the outer selector",policy.worldModelLOD[7]==0);
 CHECK("nested override replaces the outer selector",Exact(policy.worldModelLOD[9],"High"));
 CHECK("sibling branches keep their outer selector",Exact(policy.worldModelLOD[10],"Never"));
 CHECK("independent roots do not inherit a selector",policy.worldModelLOD[11]==0);
 CHECK("parent WorldModel class flag is not inherited",policy.worldModelLOD[12]==0&&Exact(policy.worldModelLOD[13],"Never"));
 CHECK("ShadowOverride traversal takes priority over its class flags",Exact(policy.worldModelLOD[15],"Medium"));
 CHECK("brush traversal does not consume WM override",Exact(policy.worldModelLOD[17],"Never"));
 CHECK("selector text is owned independently from source",policy.worldModelLOD[2]!=outerValue);outerValue[0]='n';CHECK("source selector mutation cannot alter prepared output",Exact(policy.worldModelLOD[2],"Never"));outerValue[0]='N';
 PrefabScene::Node& changed=*Allocate(memory);changed=*nodes[2];PrefabScene::Property before[3];memcpy(before,owner,sizeof(owner));const char* error=0;
 CHECK("Objects application replaces existing ShadowLOD",Apply(changed,policy.worldModelLOD[2],memory,error)&&!error&&Value(changed,"ShadowLOD","Never"));
 CHECK("source property text and links remain immutable",Same(before,owner,sizeof(owner))&&Value(*nodes[2],"ShadowLOD","Low"));
 CHECK("replacement preserves surrounding order and flags",Get(changed,"ShadowLOD")->flags==0x4000u&&Exact(changed.properties->name,"Name")&&Exact(changed.properties->next->next->name,"Other"));
 const PrefabScene::Property* retained=changed.properties;CHECK("repeat application is idempotent without republishing",Apply(changed,"Never",memory,error)&&changed.properties==retained);
 PrefabScene::Node& appended=*Allocate(memory);appended=*nodes[18];static PrefabScene::Property name;Property(name,"Name","MissingLOD");appended.properties=&name;
 CHECK("missing ShadowLOD is appended after existing properties",Apply(appended,"Medium",memory,error)&&Exact(appended.properties->name,"Name")&&Exact(appended.properties->next->name,"ShadowLOD")&&Value(appended,"ShadowLOD","Medium")&&!appended.properties->next->next);
 CHECK("new ShadowLOD is an ordinary flagless string",appended.properties->next->string&&Exact(appended.properties->next->type,"string")&&appended.properties->next->flags==0);
 PrefabScene::Node& nonstring=*Allocate(memory);nonstring=*nodes[18];static PrefabScene::Property typed;Property(typed,"ShadowLOD","1","real");nonstring.properties=&typed;
 CHECK("wrong typed first property retains itself and appends a string",Apply(nonstring,"High",memory,error)&&Exact(nonstring.properties->type,"real")&&nonstring.properties->next&&Exact(nonstring.properties->next->type,"string")&&Exact(nonstring.properties->next->data,"High"));
 PrefabScene::Node& skipped=*Allocate(memory);skipped=*nodes[2];retained=skipped.properties;CHECK("null selector leaves authored properties unchanged",Apply(skipped,0,memory,error)&&skipped.properties==retained);
 CHECK("unknown selector fails atomically",!Apply(skipped,"FutureLOD",memory,error)&&error&&skipped.properties==retained);
 CHECK("No Override is a policy sentinel not an emitted LOD",!Apply(skipped,"No Override",memory,error)&&skipped.properties==retained);
 skipped.kind=1;CHECK("non object target cannot receive a generated property",!Apply(skipped,"High",memory,error)&&skipped.properties==retained);skipped.kind=2;
 lstrcpyA(outer[1].name,"wmoverride");CHECK("property lookup remains case sensitive",policy.Build(model,catalog)&&policy.worldModelLOD[2]==0);lstrcpyA(outer[1].name,"WMOverride");
 lstrcpyA(nodes[0]->className,"sHaDoWoVeRrIdE");CHECK("ShadowOverride class detection is case insensitive",policy.Build(model,catalog)&&Exact(policy.worldModelLOD[2],"Never"));lstrcpyA(nodes[0]->className,"ShadowOverride");
 Property(clearProp,"BrushOverride","Low");CHECK("nested missing WM selector also clears inheritance",policy.Build(model,catalog)&&policy.worldModelLOD[7]==0);Property(clearProp,"WMOverride","No Override");
 outer[1].data=const_cast<char*>("Unknown");CHECK("unsupported LOD fails with no partially published table",!policy.Build(model,catalog)&&policy.error&&!policy.worldModelLOD&&!policy.count);outer[1].data=outerValue;
 outer[1].data=0;CHECK("null selector text is rejected without dereference",!policy.Build(model,catalog));outer[1].data=outerValue;
 nodes[2]->parent=2;CHECK("self parent cycle is rejected",!policy.Build(model,catalog));nodes[2]->parent=1;
 nodes[2]->parent=99;CHECK("out of range parent is rejected",!policy.Build(model,catalog));nodes[2]->parent=1;
 nodes[2]->index=19;CHECK("source index mismatch is rejected",!policy.Build(model,catalog));nodes[2]->index=2;
 owner[2].next=owner;CHECK("property cycle is bounded",!policy.Build(model,catalog));owner[2].next=0;
 unsigned int saved=model.count;model.count=MaximumNodes+1;CHECK("oversized scene count fails before traversal",!policy.Build(model,catalog));model.count=saved;
 model.count++;CHECK("incomplete scene count is rejected",!policy.Build(model,catalog));model.count--;
 CHECK("policy recovers after invalid inputs",policy.Build(model,catalog)&&!policy.error&&Exact(policy.worldModelLOD[2],"Never"));
 char path[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_OBJECT_SHADOW_GUARDS_OUTPUT",path,MAX_PATH))ExitProcess(1);HANDLE file=CreateFileA(path,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);unsigned int header[3]={0x3147534f,used,3};if(file==INVALID_HANDLE_VALUE||!Write(file,header,12)||!Write(file,results,used*4)||!Dump(file,changed)||!Dump(file,appended)||!Dump(file,nonstring))ExitProcess(3);CloseHandle(file);for(unsigned int i=0;i<used;i++)if(!results[i])ExitProcess(4);ExitProcess(0);
}
