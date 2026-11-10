#define FEAR_PRIMARY_SURFACE_PREPARATION 1
#include "WorldPacker.cpp"
#include "NativePhysicsMainExplicit.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
static bool Write(HANDLE file,const void* data,unsigned int size){DWORD done=0;return !size||(WriteFile(file,data,size,&done,0)&&done==size);}
extern "C" void HarnessEntry(){
 char source[MAX_PATH],project[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_READER_INPUT",source,MAX_PATH)||!GetEnvironmentVariableA("FEAR_READER_PROJECT",project,MAX_PATH)||!GetEnvironmentVariableA("FEAR_PHYSICS_MAIN_EXPLICIT_OUTPUT",output,MAX_PATH))ExitProcess(1);
 PrefabScene::Arena memory;void* place=memory.Allocate(sizeof(NativePreparedRenderScene::Context));if(!place)ExitProcess(2);NativePreparedRenderScene::Context* scene=new(place)NativePreparedRenderScene::Context;
 if(!scene->Build(source,project)){LogLine(scene->Diagnostic().message);ExitProcess(2);}
 NativePhysicsMainExplicit::Builder result;if(!result.Build(*scene)){LogLine(result.error);ExitProcess(3);}scene->Clear();
 unsigned int header[9]={0x31454d50u,1,result.count,result.size,result.obbCount,result.sphereCount,result.capsuleCount,result.nonMainExplicitCount,1};
 HANDLE file=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(file==INVALID_HANDLE_VALUE||!Write(file,header,36))ExitProcess(4);
 for(const NativePhysicsMainExplicit::Record* record=result.first;record;record=record->next){unsigned int nameBytes=lstrlenA(record->name)+1;unsigned int fields[4]={record->sourceNode,record->type,record->bytes,nameBytes};if(!Write(file,fields,16)||!Write(file,record->name,nameBytes)||!Write(file,record->data,record->bytes))ExitProcess(4);}CloseHandle(file);scene->~Context();ExitProcess(0);
}
