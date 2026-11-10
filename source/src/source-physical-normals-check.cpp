#define FEAR_PRIMARY_SURFACE_PREPARATION 1
#include "WorldPacker.cpp"
#include "NativePreparedRenderScene.h"
#include "SourcePhysicalBspNormals.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
static bool Write(HANDLE f,const void* p,unsigned int size){DWORD done=0;return WriteFile(f,p,size,&done,0)&&done==size;}
extern "C" void HarnessEntry(){
 char source[MAX_PATH],project[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_READER_INPUT",source,MAX_PATH)||!GetEnvironmentVariableA("FEAR_READER_PROJECT",project,MAX_PATH)||!GetEnvironmentVariableA("FEAR_MAIN_BSP_OUTPUT",output,MAX_PATH))ExitProcess(1);
 PrefabScene::Arena memory;void* sceneStorage=memory.Allocate(sizeof(NativePreparedRenderScene::Context));if(!sceneStorage)ExitProcess(2);NativePreparedRenderScene::Context* scene=new(sceneStorage)NativePreparedRenderScene::Context;
 if(!scene->Build(source,project))ExitProcess(3);SourcePhysicalBspNormals::Table table;if(!table.Build(scene->Model(),scene->Partition(),scene->Offset()))ExitProcess(4);
 HANDLE file=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);unsigned int header[5]={0x314e5042,table.count,table.initialCount,table.surfaces,table.surfaceCount};
 if(file==INVALID_HANDLE_VALUE||!Write(file,header,20)||!Write(file,table.normals,table.count*12)||!Write(file,table.uses,table.count*4)||!Write(file,table.initialNormals,table.initialCount*12)||!Write(file,table.initialUses,table.initialCount*4))ExitProcess(5);
 for(unsigned int i=0;i<table.surfaceCount;i++){const SourcePhysicalBspNormals::Surface& surface=table.sourceSurfaces[i];const WorldModels::Assignment& item=scene->Partition().assignments[surface.assignment];unsigned int fields[8]={surface.assignment,item.group,item.brush->index,surface.polygon,surface.initialIdentity,surface.finalIdentity,surface.active?1u:0u,item.brush->surfaceFlags};if(!Write(file,fields,32))ExitProcess(5);}
 CloseHandle(file);scene->~Context();ExitProcess(0);
}
