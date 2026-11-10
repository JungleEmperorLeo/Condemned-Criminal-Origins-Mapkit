#define FEAR_PRIMARY_SURFACE_PREPARATION 1
#include "WorldPacker.cpp"
#include "NativePreparedRenderScene.h"
#include "SourceWorldTree.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
extern "C" void HarnessEntry(){
 char input[MAX_PATH],project[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_READER_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_READER_PROJECT",project,MAX_PATH)||!GetEnvironmentVariableA("FEAR_WORLD_TREE_OUTPUT",output,MAX_PATH))ExitProcess(1);
 PrefabScene::Arena memory;void* contextStorage=memory.Allocate(sizeof(NativePreparedRenderScene::Context));if(!contextStorage)ExitProcess(2);NativePreparedRenderScene::Context* scene=new(contextStorage)NativePreparedRenderScene::Context;
 if(!scene->Build(input,project)){LogLine(scene->Diagnostic().message);ExitProcess(2);}SourceWorldTree::Builder tree;if(!tree.Build(scene->Model(),scene->Partition(),scene->Offset())){LogLine(tree.error);ExitProcess(3);}
 HANDLE file=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);DWORD done=0;if(file==INVALID_HANDLE_VALUE||!WriteFile(file,tree.data,tree.size,&done,0)||done!=tree.size)ExitProcess(4);CloseHandle(file);scene->~Context();ExitProcess(0);
}
