#define FEAR_PRIMARY_SURFACE_PREPARATION 1
#include "WorldPacker.cpp"
#include "NativePreparedRenderScene.h"
#include "SourceWorldModels.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
static bool Write(HANDLE file,const void* data,unsigned int size){DWORD done=0;return WriteFile(file,data,size,&done,0)&&done==size;}
extern "C" void HarnessEntry(){
 char source[MAX_PATH],project[MAX_PATH],output[MAX_PATH];
 if(!GetEnvironmentVariableA("FEAR_READER_INPUT",source,MAX_PATH)||!GetEnvironmentVariableA("FEAR_READER_PROJECT",project,MAX_PATH)||!GetEnvironmentVariableA("FEAR_WORLDMODELS_OUTPUT",output,MAX_PATH))ExitProcess(1);
 PrefabScene::Arena memory;void* sceneStorage=memory.Allocate(sizeof(NativePreparedRenderScene::Context));if(!sceneStorage)ExitProcess(2);NativePreparedRenderScene::Context* scene=new(sceneStorage)NativePreparedRenderScene::Context;
 if(!scene->Build(source,project)){LogLine(scene->Diagnostic().message);ExitProcess(2);}
 SourceWorldModels::Settings settings;settings.fragmentLimit=500000;settings.pointLimit=4000000;
 SourceWorldModels::Builder worlds;
 if(!worlds.Build(scene->Model(),scene->Partition(),scene->Offset(),scene->Project(),settings)){
  LogLine(worlds.error);char errorPath[MAX_PATH];lstrcpynA(errorPath,output,MAX_PATH-11);lstrcpyA(errorPath+lstrlenA(errorPath),".error.txt");HANDLE f=CreateFileA(errorPath,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(f!=INVALID_HANDLE_VALUE){Write(f,&worlds.failedGroup,4);if(worlds.error)Write(f,worlds.error,lstrlenA(worlds.error));CloseHandle(f);}ExitProcess(3);
 }
 scene->Clear();
 HANDLE file=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(file==INVALID_HANDLE_VALUE||!Write(file,worlds.data,worlds.size))ExitProcess(4);CloseHandle(file);scene->~Context();ExitProcess(0);
}
