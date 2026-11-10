#include "WorldPacker.cpp"
#include "OfficeClassProfile.h"
#include "WorldModelSharing.h"
#include "WorldCenter.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
static const PrefabScene::Property* Prop(const PrefabScene::Node& n,const char* name){for(const PrefabScene::Property* p=n.properties;p;p=p->next)if(FearBrushFlags::Equal(p->name,name))return p;return 0;}
static bool Number(const PrefabScene::Node& n,const char* key,float& value){const PrefabScene::Property* p=Prop(n,key);WorldText::Document d;return p&&d.Parse(p->data,lstrlenA(p->data))&&d.Elements(0)==1&&d.Number(d.First(0),value);}
extern "C" void HarnessEntry(){
 char input[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_READER_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_OWNER_RECORDS_OUTPUT",output,MAX_PATH))ExitProcess(1);
 PrefabScene::Source source;if(!source.Load(input))ExitProcess(2);void* expansionStorage=VirtualAlloc(0,sizeof(PrefabScene::Expansion),MEM_COMMIT|MEM_RESERVE,4);if(!expansionStorage)ExitProcess(3);PrefabScene::Expansion* e=new(expansionStorage) PrefabScene::Expansion;if(!e->Build(input,source.scene))ExitProcess(3);
 WorldModels::Partition p;if(!p.Build(e->Scene(),ResolveOffice,0))ExitProcess(4);WorldModelSharing::Identity sharing;if(!sharing.Build(e->Scene(),p))ExitProcess(5);WorldText::Vec3 offset;if(!WorldCenter::Compute(e->Scene(),offset))ExitProcess(6);
 PrefabScene::Arena memory;unsigned int count=e->Scene().count;unsigned int* groups=(unsigned int*)memory.Allocate((count+1)*4);unsigned char* explicitShape=(unsigned char*)memory.Allocate(p.groupCount);unsigned char* data=(unsigned char*)memory.Allocate(8+p.groupCount*64);if(!groups||!explicitShape||!data)ExitProcess(7);
 for(const PrefabScene::Node* n=e->Scene().first;n;n=n->next){unsigned int group=n->parent==0xffffffffu?0:groups[n->parent];for(unsigned int i=1;i<p.groupCount;i++)if(p.groups[i].ownerIndex==n->index){group=i;break;}groups[n->index]=group;if(n->kind==2&&(FearBrushFlags::Equal(n->className,"PhysicsShapeOBB")||FearBrushFlags::Equal(n->className,"PhysicsShapeSphere")||FearBrushFlags::Equal(n->className,"PhysicsShapeCapsule")))explicitShape[group]=1;}
 unsigned int writtenCount=0,cursor=8;*(unsigned int*)data=0x31524f50;
 for(unsigned int i=1;i<p.groupCount;i++){if(sharing.representative[i]!=i||explicitShape[i])continue;const PrefabScene::Node& n=*p.groups[i].owner;const PrefabScene::Property* shape=Prop(n,"PhysicsShape");if(!shape&&!FearBrushFlags::Equal(n.className,"LookAtTrigger"))ExitProcess(8);bool none=!shape||FearBrushFlags::Equal(shape->data,"None"),obb=shape&&FearBrushFlags::Equal(shape->data,"OBB");if(!none&&!obb)continue;
  unsigned int bytes=none?32:56;unsigned char* row=data+cursor;*(unsigned int*)row=sharing.serializedIndex[i];*(unsigned int*)(row+4)=bytes;float* transform=(float*)(row+8);for(unsigned int k=0;k<7;k++)transform[k]=0;transform[6]=1;*(unsigned int*)(row+36)=none?0:2;
  if(obb){float mass=0,density=0;WorldText::Vec3 translation={};if(!Number(n,"MassKg",mass)||!Number(n,"DensityG",density)||!PhysicsOwnerBounds::BuildGroup(p,i,mass,density,(float*)(row+40),translation,&offset))ExitProcess(9);transform[0]=translation.x;transform[1]=translation.y;transform[2]=translation.z;}
  cursor+=8+bytes;writtenCount++;
 }
 *(unsigned int*)(data+4)=writtenCount;HANDLE f=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);DWORD done;if(f==INVALID_HANDLE_VALUE||!WriteFile(f,data,cursor,&done,0)||done!=cursor)ExitProcess(10);CloseHandle(f);ExitProcess(0);
}
