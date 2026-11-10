#include "WorldPacker.cpp"
#include "OfficeClassProfile.h"
#include "WorldModelSharing.h"
#include "WorldCenter.h"
#include "PhysicsExplicitShapes.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
struct Writer {unsigned char* data;unsigned int cursor,capacity;bool Put(const void* p,unsigned int bytes){if(bytes>capacity-cursor)return false;for(unsigned int i=0;i<bytes;i++)data[cursor++]=((const unsigned char*)p)[i];return true;}bool U(unsigned int v){return Put(&v,4);}};
static unsigned int BodySize(unsigned int kind){return kind==2?24:kind==3?12:kind==7?36:0;}
extern "C" void HarnessEntry(){
 char input[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_READER_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_OWNER_COMPOUNDS_OUTPUT",output,MAX_PATH))ExitProcess(1);
 PrefabScene::Source source;if(!source.Load(input))ExitProcess(2);void* expansionStorage=VirtualAlloc(0,sizeof(PrefabScene::Expansion),MEM_COMMIT|MEM_RESERVE,4);if(!expansionStorage)ExitProcess(3);PrefabScene::Expansion* e=new(expansionStorage) PrefabScene::Expansion;if(!e->Build(input,source.scene))ExitProcess(3);
 WorldModels::Partition p;if(!p.Build(e->Scene(),ResolveOffice,0))ExitProcess(4);WorldModelSharing::Identity sharing;if(!sharing.Build(e->Scene(),p))ExitProcess(5);WorldText::Vec3 offset;if(!WorldCenter::Compute(e->Scene(),offset))ExitProcess(6);
 PrefabScene::Arena memory;unsigned int* groups=(unsigned int*)memory.Allocate((e->Scene().count+1)*4);unsigned int* counts=(unsigned int*)memory.Allocate(p.groupCount*4);unsigned char* data=(unsigned char*)memory.Allocate(1048576);unsigned char* contiguous=(unsigned char*)memory.Allocate(1048576);if(!groups||!counts||!data||!contiguous)ExitProcess(7);
 for(const PrefabScene::Node* n=e->Scene().first;n;n=n->next){unsigned int group=n->parent==0xffffffffu?0:groups[n->parent];for(unsigned int i=1;i<p.groupCount;i++)if(p.groups[i].ownerIndex==n->index){group=i;break;}groups[n->index]=group;if(n->kind==2&&(FearBrushFlags::Equal(n->className,"PhysicsShapeOBB")||FearBrushFlags::Equal(n->className,"PhysicsShapeSphere")||FearBrushFlags::Equal(n->className,"PhysicsShapeCapsule")))counts[group]++;}
 Writer native={contiguous,0,1048576};Writer w={data,0,1048576};if(!w.U(0x31434f50)||!w.U(sharing.uniqueCount-1))ExitProcess(7);
 for(unsigned int i=1;i<p.groupCount;i++){if(sharing.representative[i]!=i)continue;const PrefabScene::Node& owner=*p.groups[i].owner;const PrefabScene::Property* shape=PhysicsExplicitShapes::Property(owner,"PhysicsShape");if(!shape&&!FearBrushFlags::Equal(owner.className,"LookAtTrigger"))ExitProcess(8);bool automatic=shape&&FearBrushFlags::Equal(shape->data,"OBB");if(shape&&!automatic&&!FearBrushFlags::Equal(shape->data,"None"))ExitProcess(8);unsigned int children=counts[i]+(automatic?1:0),start=w.cursor;if(!w.U(sharing.serializedIndex[i])||!w.U(0))ExitProcess(7);
  float identity[7]={0,0,0,0,0,0,1};float autoBody[6]={};WorldText::Vec3 autoTranslation={};if(automatic){float mass=0,density=0;if(!PhysicsExplicitShapes::Number(owner,"MassKg",mass)||!PhysicsExplicitShapes::Number(owner,"DensityG",density)||!PhysicsOwnerBounds::BuildGroup(p,i,mass,density,autoBody,autoTranslation,&offset))ExitProcess(9);}
  if(children==0){if(!w.Put(identity,28)||!w.U(0))ExitProcess(7);}
  else if(children>1){if(!w.Put(identity,28)||!w.U(6)||!w.U(children))ExitProcess(7);}
  if(automatic){float t[7]={autoTranslation.x,autoTranslation.y,autoTranslation.z,0,0,0,1};if(children>1&&!w.U(5))ExitProcess(7);if(!w.Put(t,28)||!w.U(2)||!w.Put(autoBody,24))ExitProcess(7);}
  for(const PrefabScene::Node* n=e->Scene().first;n;n=n->next){if(n->kind!=2||groups[n->index]!=i||!(FearBrushFlags::Equal(n->className,"PhysicsShapeOBB")||FearBrushFlags::Equal(n->className,"PhysicsShapeSphere")||FearBrushFlags::Equal(n->className,"PhysicsShapeCapsule")))continue;unsigned int kind=0;float body[9]={},t[7]={0,0,0,0,0,0,1};if(!PhysicsExplicitShapes::Build(p,i,*n,offset,kind,body,t))ExitProcess(10);
   if(children==1){if(!w.Put(kind==7?identity:t,28))ExitProcess(7);}else if(kind!=7){if(!w.U(5)||!w.Put(t,28))ExitProcess(7);}if(!w.U(kind)||!w.Put(body,BodySize(kind)))ExitProcess(7);
  }
  *(unsigned int*)(data+start+4)=w.cursor-start-8;if(!native.Put(data+start+8,w.cursor-start-8))ExitProcess(7);
 }
 HANDLE f=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);DWORD done;if(f==INVALID_HANDLE_VALUE||!WriteFile(f,data,w.cursor,&done,0)||done!=w.cursor)ExitProcess(11);CloseHandle(f);char nativeOutput[MAX_PATH];if(GetEnvironmentVariableA("FEAR_NONROOT_PHYSICS_OUTPUT",nativeOutput,MAX_PATH)){HANDLE nf=CreateFileA(nativeOutput,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(nf==INVALID_HANDLE_VALUE||!WriteFile(nf,contiguous,native.cursor,&done,0)||done!=native.cursor)ExitProcess(11);CloseHandle(nf);}ExitProcess(0);
}
