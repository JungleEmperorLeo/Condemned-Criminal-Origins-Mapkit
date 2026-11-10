#define FEAR_PRIMARY_SURFACE_PREPARATION 1
#include "WorldPacker.cpp"
#include "NativeWorldModelsStructure.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
static bool Write(HANDLE file,const void* data,unsigned int size){DWORD done=0;return !size||(WriteFile(file,data,size,&done,0)&&done==size);}
static bool Cancel(void* context){return *(bool*)context;}
extern "C" void HarnessEntry(){
 char source[MAX_PATH],project[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_READER_INPUT",source,MAX_PATH)||!GetEnvironmentVariableA("FEAR_READER_PROJECT",project,MAX_PATH)||!GetEnvironmentVariableA("FEAR_MODELS_STRUCTURE_OUTPUT",output,MAX_PATH))ExitProcess(1);
 PrefabScene::Arena memory;void* sceneStorage=memory.Allocate(sizeof(NativePreparedRenderScene::Context));if(!sceneStorage)ExitProcess(2);NativePreparedRenderScene::Context* scene=new(sceneStorage)NativePreparedRenderScene::Context;
 NativeWorldModelsStructure::Builder builder;unsigned int guards=0;
 if(builder.Build(*scene)||builder.Ready()||builder.Models().data||!builder.Error().message[0])ExitProcess(10);guards++;
 if(!scene->Build(source,project)){LogLine(scene->Diagnostic().message);ExitProcess(2);}
 NativeWorldModelsStructure::Settings settings;settings.physical.fragmentLimit=500000;settings.physical.pointLimit=4000000;
 bool cancelled=true;NativeWorldModelsStructure::Observer events;events.cancel=Cancel;events.context=&cancelled;
 if(builder.Build(*scene,settings,events)||builder.Ready()||builder.Models().data||builder.Structural().Tree().data||!builder.Error().message[0])ExitProcess(11);guards++;
 if(!builder.Build(*scene,settings)){LogLine(builder.Error().message);ExitProcess(3);}
 scene->Clear();
 const NativeWorldStructure::Builder& structure=builder.Structural();const SourceWorldModels::Builder& models=builder.Models();
 const SourceWorldTree::Builder& tree=structure.Tree();const SourceBlockerPolygons::Block& blockers=structure.OrdinaryBlockers();const ParticleBlockers::Block& particles=structure.ParticleBlockersData();
 unsigned int h[12]={0x324d5357u,1,tree.size,models.size,blockers.size,particles.size,tree.nodeCount,models.modelCount,models.nameCount,blockers.count,particles.count,guards};
 HANDLE file=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(file==INVALID_HANDLE_VALUE||!Write(file,h,48)||!Write(file,tree.data,tree.size)||!Write(file,models.data,models.size)||!Write(file,blockers.data,blockers.size)||!Write(file,particles.data,particles.size))ExitProcess(4);CloseHandle(file);
 builder.Clear();if(builder.Ready()||builder.Models().data||builder.Structural().Tree().data)ExitProcess(12);scene->~Context();ExitProcess(0);
}
