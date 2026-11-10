#define FEAR_PRIMARY_SURFACE_PREPARATION 1
#include "WorldPacker.cpp"
#include "NativeWorldStructure.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
static bool Write(HANDLE file,const void* data,unsigned int size){DWORD done=0;return !size||(WriteFile(file,data,size,&done,0)&&done==size);}
static bool Cancel(void* context){return *(bool*)context;}
extern "C" void HarnessEntry(){
 char source[MAX_PATH],project[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_READER_INPUT",source,MAX_PATH)||!GetEnvironmentVariableA("FEAR_READER_PROJECT",project,MAX_PATH)||!GetEnvironmentVariableA("FEAR_WORLD_STRUCTURE_OUTPUT",output,MAX_PATH))ExitProcess(1);
 PrefabScene::Arena memory;void* sceneStorage=memory.Allocate(sizeof(NativePreparedRenderScene::Context));if(!sceneStorage)ExitProcess(2);NativePreparedRenderScene::Context* scene=new(sceneStorage)NativePreparedRenderScene::Context;
 NativeWorldStructure::Builder builder;if(builder.Build(*scene)||builder.Ready()||builder.Tree().data||!builder.Error().message[0])ExitProcess(10);
 if(!scene->Build(source,project)){LogLine(scene->Diagnostic().message);ExitProcess(2);}
 bool cancelled=true;NativeWorldStructure::Observer observer;observer.cancel=Cancel;observer.context=&cancelled;
 if(builder.Build(*scene,NativeWorldStructure::Settings(),observer)||builder.Ready()||builder.OrdinaryBlockers().data||builder.ParticleBlockersData().data||!builder.Error().message[0])ExitProcess(11);
 if(!builder.Build(*scene)){LogLine(builder.Error().message);ExitProcess(3);}
 const SourceWorldTree::Builder& tree=builder.Tree();const SourceBlockerPolygons::Block& blockers=builder.OrdinaryBlockers();const ParticleBlockers::Block& particles=builder.ParticleBlockersData();
 unsigned int header[8]={0x31535357u,1,tree.size,blockers.size,particles.size,tree.nodeCount,blockers.count,particles.count};
 // Complete all supported structural streams before opening a diagnostic output.
 // This archive contains three separate streams, not a complete world prefix.
 HANDLE file=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(file==INVALID_HANDLE_VALUE||!Write(file,header,32)||!Write(file,tree.data,tree.size)||!Write(file,blockers.data,blockers.size)||!Write(file,particles.data,particles.size))ExitProcess(4);CloseHandle(file);
 builder.Clear();scene->~Context();ExitProcess(0);
}
