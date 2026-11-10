#define FEAR_PRIMARY_SURFACE_PREPARATION 1
#include "WorldPacker.cpp"
#include "NativePreparedRenderScene.h"
#include "SourceWorldModels.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
namespace WorldModelsGuards {
static bool Equal(const unsigned char* a,const unsigned char* b,unsigned int size){for(unsigned int i=0;i<size;i++)if(a[i]!=b[i])return false;return true;}
static bool Empty(const SourceWorldModels::Builder& value){const WorldModelsSerialization::Block& block=value.Block();if(value.data||value.size||value.modelCount||value.nameCount||value.normalCount||value.emptyCount||value.materials||block.data||block.size)return false;for(unsigned int i=0;i<8;i++)if(block.counts[i])return false;return true;}
static bool Rejected(SourceWorldModels::Builder& value,const PrefabScene::Model& model,const WorldModels::Partition& partition,const WorldText::Vec3& offset,const PrefabScene::Project& project,const SourceWorldModels::Settings& settings=SourceWorldModels::Settings()){
 return !value.Build(model,partition,offset,project,settings)&&Empty(value)&&value.error&&value.error[0]&&value.error==value.diagnostic;
}
static bool Write(HANDLE file,const void* data,unsigned int bytes){DWORD done=0;return WriteFile(file,data,bytes,&done,0)&&done==bytes;}
static unsigned int Word(const unsigned char* data){unsigned int result;memcpy(&result,data,4);return result;}
static bool TextEqual(const char* a,const char* b){for(;;a++,b++){if(*a!=*b)return false;if(!*a)return true;}}
// Validate output references independently of the serializer. Name table model
// indices must address emitted blocks, not source owner groups or sorted names.
static bool Names(const SourceWorldModels::Builder& value,const char* a,const char* b,const char* c){
 if(!value.data||value.size<32||value.nameCount!=3||value.modelCount!=2)return false;
 unsigned int count=Word(value.data)^399u,strings=Word(value.data+4)^399u,models=Word(value.data+12)^399u;
 if(count!=3||models!=2||strings>value.size-32||count*8>value.size-32-strings)return false;
 const unsigned char* entries=value.data+32+strings;const char* expected[3]={a,b,c};unsigned int expectedIds[3]={1,0,1};
 for(unsigned int i=0;i<count;i++){unsigned int offset=Word(entries+i*8),index=Word(entries+i*8+4);if(offset>=strings||index>=models||index!=expectedIds[i])return false;const char* text=(const char*)value.data+32+offset;unsigned int length=0;while(length<strings-offset&&text[length])length++;if(length==strings-offset||!TextEqual(text,expected[i]))return false;}
 return true;
}
static PrefabScene::Node* Brush(PrefabScene::Model& model,const PrefabScene::Node& source,unsigned int parent){
 PrefabScene::Node* node=model.Add();if(!node)return 0;unsigned int index=node->index;*node=source;node->next=0;node->index=index;node->parent=parent;node->properties=0;node->importedFromPrefab=parent!=0xffffffffu;model.brushes++;model.points+=node->geometry.points;model.polygons+=node->geometry.polygons;return node;
}
}
extern "C" void HarnessEntry(){
 using namespace WorldModelsGuards;
 char source[MAX_PATH],project[MAX_PATH],output[MAX_PATH];
 if(!GetEnvironmentVariableA("FEAR_WORLDMODELS_GUARDS_SOURCE",source,MAX_PATH)||!GetEnvironmentVariableA("FEAR_WORLDMODELS_GUARDS_PROJECT",project,MAX_PATH)||!GetEnvironmentVariableA("FEAR_WORLDMODELS_GUARDS_OUTPUT",output,MAX_PATH))ExitProcess(1);
 PrefabScene::Arena memory;void* place=memory.Allocate(sizeof(NativePreparedRenderScene::Context));if(!place)ExitProcess(2);
 NativePreparedRenderScene::Context* scene=new(place)NativePreparedRenderScene::Context;
 SourceWorldModels::Builder worlds;unsigned int checks[48]={},count=0;
 checks[count++]=Empty(worlds)&&!worlds.error&&worlds.failedGroup==0xffffffffu;
 checks[count++]=Rejected(worlds,scene->Model(),scene->Partition(),scene->Offset(),scene->Project());
 if(!scene->Build(source,project)){LogLine(scene->Diagnostic().message);ExitProcess(3);}
 PrefabScene::Model& model=const_cast<PrefabScene::Model&>(scene->Model());WorldModels::Partition& partition=const_cast<WorldModels::Partition&>(scene->Partition());
 const WorldText::Vec3& offset=scene->Offset();const PrefabScene::Project& selected=scene->Project();
 if(!worlds.Build(model,partition,offset,selected)){LogLine(worlds.error);ExitProcess(4);}
 unsigned int referenceSize=worlds.size;unsigned char* reference=(unsigned char*)memory.Allocate(referenceSize);if(!reference)ExitProcess(2);memcpy(reference,worlds.data,referenceSize);
 checks[count++]=worlds.modelCount==6&&worlds.nameCount==6&&worlds.normalCount&&worlds.Block().data==worlds.data&&worlds.Block().size==worlds.size;
 SourceWorldModels::Settings settings;settings.depthLimit=0;checks[count++]=Rejected(worlds,model,partition,offset,selected,settings);
 settings=SourceWorldModels::Settings();settings.depthLimit=4097;checks[count++]=Rejected(worlds,model,partition,offset,selected,settings);
 settings=SourceWorldModels::Settings();settings.fragmentLimit=0;checks[count++]=Rejected(worlds,model,partition,offset,selected,settings);
 settings=SourceWorldModels::Settings();settings.fragmentLimit=1000001;checks[count++]=Rejected(worlds,model,partition,offset,selected,settings);
 settings=SourceWorldModels::Settings();settings.pointLimit=10000001;checks[count++]=Rejected(worlds,model,partition,offset,selected,settings);
 settings=SourceWorldModels::Settings();settings.pointLimit=0;checks[count++]=Rejected(worlds,model,partition,offset,selected,settings);
 settings=SourceWorldModels::Settings();settings.pointLimit=1;checks[count++]=Rejected(worlds,model,partition,offset,selected,settings);
 PrefabScene::Project missing;checks[count++]=Rejected(worlds,model,partition,offset,missing)&&worlds.failedGroup==0xffffffffu;
 PrefabScene::Project failedProject=selected;failedProject.error="guard project retained failure";checks[count++]=Rejected(worlds,model,partition,offset,failedProject);
 WorldText::Vec3 invalid=offset;unsigned int bits=0x7fc00000;memcpy(&invalid.x,&bits,4);checks[count++]=Rejected(worlds,model,partition,invalid,selected);
 unsigned int savedCount=partition.groupCount;partition.groupCount=0;checks[count++]=Rejected(worlds,model,partition,offset,selected);partition.groupCount=savedCount;
 WorldModels::Group* savedGroups=partition.groups;partition.groups=0;checks[count++]=Rejected(worlds,model,partition,offset,selected);partition.groups=savedGroups;
 WorldModels::Assignment* savedAssignments=partition.assignments;partition.assignments=0;checks[count++]=Rejected(worlds,model,partition,offset,selected);partition.assignments=savedAssignments;
 const char* savedError=partition.error;partition.error="guard source partition error";checks[count++]=Rejected(worlds,model,partition,offset,selected);partition.error=savedError;
 unsigned int savedGroup=partition.assignments[0].group;partition.assignments[0].group=partition.groupCount;checks[count++]=Rejected(worlds,model,partition,offset,selected);partition.assignments[0].group=savedGroup;
 const PrefabScene::Node* savedBrush=partition.assignments[0].brush;partition.assignments[0].brush=0;checks[count++]=Rejected(worlds,model,partition,offset,selected);partition.assignments[0].brush=savedBrush;
 const PrefabScene::Node* secondBrush=partition.assignments[1].brush;partition.assignments[1].brush=partition.assignments[0].brush;checks[count++]=Rejected(worlds,model,partition,offset,selected);partition.assignments[1].brush=secondBrush;
 unsigned int modelCount=model.count;model.count--;checks[count++]=Rejected(worlds,model,partition,offset,selected);model.count=modelCount;
 unsigned int firstIndex=model.first->index;model.first->index=1;checks[count++]=Rejected(worlds,model,partition,offset,selected);model.first->index=firstIndex;
 const PrefabScene::Node* savedOwner=partition.groups[1].owner;partition.groups[1].owner=0;checks[count++]=Rejected(worlds,model,partition,offset,selected);partition.groups[1].owner=savedOwner;
 PrefabScene::Node* brush=0;for(unsigned int i=0;i<partition.assignmentCount;i++)if(!partition.assignments[i].group&&SourceRenderPointTags::MainWorldCandidate(*partition.assignments[i].brush)&&!(partition.assignments[i].brush->surfaceFlags&0x180)){brush=const_cast<PrefabScene::Node*>(partition.assignments[i].brush);break;}if(!brush)ExitProcess(7);
 WorldText::Polygon* savedFaces=brush->geometry.faces;brush->geometry.faces=0;checks[count++]=Rejected(worlds,model,partition,offset,selected);brush->geometry.faces=savedFaces;
 unsigned int vertex=brush->geometry.faces[0].indices[0];brush->geometry.faces[0].indices[0]=brush->geometry.points;checks[count++]=Rejected(worlds,model,partition,offset,selected);brush->geometry.faces[0].indices[0]=vertex;
 float point=brush->geometry.vertices[0].x;memcpy(&brush->geometry.vertices[0].x,&bits,4);checks[count++]=Rejected(worlds,model,partition,offset,selected);brush->geometry.vertices[0].x=point;
 PrefabScene::Node* owner=const_cast<PrefabScene::Node*>(savedOwner);char savedName[256];memcpy(savedName,owner->name,256);
 owner->name[0]=0;checks[count++]=Rejected(worlds,model,partition,offset,selected);memcpy(owner->name,savedName,256);
 lstrcpyA(owner->name,"pHySiCsBsP");checks[count++]=Rejected(worlds,model,partition,offset,selected);memcpy(owner->name,savedName,256);
 memset(owner->name,'x',256);checks[count++]=Rejected(worlds,model,partition,offset,selected);memcpy(owner->name,savedName,256);
 checks[count++]=worlds.Build(model,partition,offset,selected)&&worlds.size==referenceSize&&Equal(worlds.data,reference,referenceSize)&&!worlds.error&&worlds.failedGroup==0xffffffffu;
 // Two direct prefab instances share one physical owner BSP while both names
 // survive alphabetic sorting. Geometry is an input, never a compiled fixture.
 PrefabScene::Model aliasModel;WorldModels::Partition aliasPartition;WorldModels::Group groups[3]={};WorldModels::Assignment assignments[3]={};
 PrefabScene::Node* mainBrush=Brush(aliasModel,*brush,0xffffffffu);if(!mainBrush)ExitProcess(2);
 for(unsigned int i=0;i<2;i++){
  PrefabScene::Node* prefab=aliasModel.Add();PrefabScene::Node* aliasOwner=aliasModel.Add();if(!prefab||!aliasOwner)ExitProcess(2);prefab->sourceKind=3;
  aliasOwner->kind=aliasOwner->sourceKind=2;aliasOwner->parent=prefab->index;aliasOwner->sourceId=7;aliasOwner->importedFromPrefab=true;lstrcpyA(aliasOwner->source,"GuardPrototype.World00a");lstrcpyA(aliasOwner->name,i?"AlphaOwner":"ZetaOwner");
  PrefabScene::Node* ownedBrush=Brush(aliasModel,*brush,aliasOwner->index);if(!ownedBrush)ExitProcess(2);groups[i+1].owner=aliasOwner;groups[i+1].ownerIndex=aliasOwner->index;groups[i+1].brushes=1;groups[i+1].polygons=ownedBrush->geometry.polygons;assignments[i+1].brush=ownedBrush;assignments[i+1].group=i+1;
 }
 groups[0].ownerIndex=0xffffffffu;groups[0].brushes=1;groups[0].polygons=mainBrush->geometry.polygons;assignments[0].brush=mainBrush;
 aliasPartition.groups=groups;aliasPartition.groupCount=3;aliasPartition.assignments=assignments;aliasPartition.assignmentCount=3;
 WorldText::Vec3 zero={};checks[count++]=worlds.Build(aliasModel,aliasPartition,zero,selected)&&Names(worlds,"AlphaOwner","PhysicsBSP","ZetaOwner");
 unsigned int savedFlags[3];for(unsigned int i=0;i<3;i++){PrefabScene::Node* renderBrush=const_cast<PrefabScene::Node*>(assignments[i].brush);savedFlags[i]=renderBrush->surfaceFlags;renderBrush->surfaceFlags=0x60;}
 checks[count++]=worlds.Build(aliasModel,aliasPartition,zero,selected)&&worlds.emptyCount==2&&Names(worlds,"AlphaOwner","PhysicsBSP","ZetaOwner");
 settings=SourceWorldModels::Settings();settings.pointLimit=0;checks[count++]=Rejected(worlds,aliasModel,aliasPartition,zero,selected,settings);
 for(unsigned int i=0;i<3;i++)const_cast<PrefabScene::Node*>(assignments[i].brush)->surfaceFlags=savedFlags[i];
 if(!worlds.Build(aliasModel,aliasPartition,zero,selected))ExitProcess(8);
 unsigned int aliasSize=worlds.size;unsigned char* aliasCopy=(unsigned char*)memory.Allocate(aliasSize);if(!aliasCopy)ExitProcess(2);memcpy(aliasCopy,worlds.data,aliasSize);
 const unsigned char* aliasData=worlds.data;aliasPartition.Clear();aliasModel.Clear();checks[count++]=worlds.data==aliasData&&worlds.size==aliasSize&&Equal(worlds.data,aliasCopy,aliasSize)&&Names(worlds,"AlphaOwner","PhysicsBSP","ZetaOwner");
 checks[count++]=worlds.Build(model,partition,offset,selected)&&worlds.size==referenceSize&&Equal(worlds.data,reference,referenceSize);
 scene->Clear();checks[count++]=worlds.size==referenceSize&&Equal(worlds.data,reference,referenceSize);
 scene->~Context();checks[count++]=worlds.size==referenceSize&&Equal(worlds.data,reference,referenceSize);
 worlds.Clear();checks[count++]=Empty(worlds)&&!worlds.error&&worlds.failedGroup==0xffffffffu;
 worlds.Clear();checks[count++]=Empty(worlds)&&!worlds.error&&worlds.failedGroup==0xffffffffu;
 HANDLE file=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(file==INVALID_HANDLE_VALUE)ExitProcess(5);unsigned int header[3]={0x31474d57,count,referenceSize};bool passed=Write(file,header,12)&&Write(file,checks,count*4)&&Write(file,reference,referenceSize);CloseHandle(file);for(unsigned int i=0;i<count;i++)if(!checks[i])passed=false;ExitProcess(passed?0:6);
}
