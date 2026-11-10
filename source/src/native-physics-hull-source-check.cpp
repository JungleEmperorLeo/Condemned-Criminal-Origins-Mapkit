#define FEAR_PRIMARY_SURFACE_PREPARATION 1
#include "WorldPacker.cpp"
#include "NativePhysicsOwners.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
static bool Write(HANDLE f,const void* p,unsigned int n){DWORD done=0;return !n||(WriteFile(f,p,n,&done,0)&&done==n);}
static bool U(HANDLE f,unsigned int value){return Write(f,&value,4);}
extern "C" void HarnessEntry(){
 char source[MAX_PATH],project[MAX_PATH],input[MAX_PATH],output[MAX_PATH];
 if(!GetEnvironmentVariableA("FEAR_READER_INPUT",source,MAX_PATH)||!GetEnvironmentVariableA("FEAR_READER_PROJECT",project,MAX_PATH)||!GetEnvironmentVariableA("FEAR_HULL_SOURCE_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_HULL_SOURCE_OUTPUT",output,MAX_PATH))ExitProcess(1);
 PrefabScene::Arena memory;void* place=memory.Allocate(sizeof(NativePreparedRenderScene::Context));if(!place)ExitProcess(2);
 NativePreparedRenderScene::Context* scene=new(place)NativePreparedRenderScene::Context;
 if(!scene->Build(source,project)){LogLine(scene->Diagnostic().message);ExitProcess(2);}
 NativePhysicsOwners::Builder owners;if(!owners.Build(*scene)){LogLine(owners.error);ExitProcess(3);}
 HANDLE inp=CreateFileA(input,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0),out=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);
 if(inp==INVALID_HANDLE_VALUE||out==INVALID_HANDLE_VALUE||!owners.hullCount||!U(inp,owners.hullCount)||!U(out,owners.hullCount))ExitProcess(4);
 unsigned int written=0;
 for(const NativePhysicsOwners::Record* record=owners.first;record;record=record->next){
  const PrefabScene::Node& owner=*scene->Partition().groups[record->group].owner;
  if(NativePhysicsProperties::Automatic(owner)!=NativePhysicsProperties::Hull)continue;
  SourcePhysicsOwnerVertices::Builder collector;if(!collector.Build(scene->Model(),scene->Partition(),record->group,scene->Offset()))ExitProcess(5);
  unsigned int polygons=0;for(unsigned int p=0;p<collector.polygonCount;++p)if(!(collector.polygons[p].flags&0x180))++polygons;
  const bool wrapper=record->type==6;float values[5];if(!NativePhysicsProperties::MassDensity(owner,false,values[0],values[1]))ExitProcess(5);memcpy(values+2,&collector.cachedCenter,12);
  if(!U(inp,polygons)||!U(inp,wrapper?1u:0u)||!Write(inp,values,20))ExitProcess(4);
  for(unsigned int p=0;p<collector.polygonCount;++p){const SourcePhysicsOwnerVertices::Polygon& poly=collector.polygons[p];if(poly.flags&0x180)continue;if(!U(inp,poly.flags)||!U(inp,poly.count)||!Write(inp,poly.points,poly.count*12))ExitProcess(4);}
  const unsigned char* raw=record->data+(wrapper?36u:0u);const unsigned int prefix=wrapper?4u:0u;
  if(record->bytes<104+prefix+(wrapper?36u:0u)||*(const unsigned int*)(raw+28+prefix)!=4)ExitProcess(6);
  unsigned int nv=*(const unsigned int*)(raw+96+prefix),np=*(const unsigned int*)(raw+100+prefix),size=104+prefix+nv*12+np*16;
  if(size>record->bytes-(wrapper?36u:0u)||!U(out,record->group)||!U(out,size)||!Write(out,raw,size))ExitProcess(6);++written;
 }
 if(written!=owners.hullCount)ExitProcess(7);CloseHandle(inp);CloseHandle(out);
 scene->Clear();if(!owners.data||!owners.size||!owners.hullCount)ExitProcess(7);scene->~Context();ExitProcess(0);
}
