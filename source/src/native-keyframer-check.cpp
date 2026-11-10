#define FEAR_PRIMARY_SURFACE_PREPARATION 1
#include "WorldPacker.cpp"
#include "NativeKeyFramerObjects.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
static bool Write(const char* path,const void* data,unsigned int bytes){HANDLE file=CreateFileA(path,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(file==INVALID_HANDLE_VALUE)return false;DWORD done=0;bool ok=WriteFile(file,data,bytes,&done,0)&&done==bytes;CloseHandle(file);return ok;}
extern "C" void HarnessEntry(){
 char source[MAX_PATH],project[MAX_PATH],objects[MAX_PATH],blind[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_READER_INPUT",source,MAX_PATH)||!GetEnvironmentVariableA("FEAR_READER_PROJECT",project,MAX_PATH)||!GetEnvironmentVariableA("FEAR_KEYFRAMER_OBJECTS_OUTPUT",objects,MAX_PATH)||!GetEnvironmentVariableA("FEAR_KEYFRAMER_BLIND_OUTPUT",blind,MAX_PATH))ExitProcess(1);
 PrefabScene::Arena memory;void* place=memory.Allocate(sizeof(NativePreparedRenderScene::Context));if(!place)ExitProcess(2);NativePreparedRenderScene::Context* scene=new(place)NativePreparedRenderScene::Context;if(!scene->Build(source,project)){LogLine(scene->Diagnostic().message);ExitProcess(3);}
 NativeKeyFramerObjects::Builder writer;if(!writer.Build(*scene)){LogLine(writer.error);char errorPath[MAX_PATH];lstrcpynA(errorPath,blind,MAX_PATH-11);lstrcpyA(errorPath+lstrlenA(errorPath),".error.txt");Write(errorPath,writer.error,lstrlenA(writer.error));ExitProcess(4);}scene->Clear();scene->~Context();if(!Write(objects,writer.objects,writer.objectsBytes)||!Write(blind,writer.blind,writer.blindBytes))ExitProcess(5);ExitProcess(0);
}
