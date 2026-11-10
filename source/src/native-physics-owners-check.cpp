#define FEAR_PRIMARY_SURFACE_PREPARATION 1
#include "WorldPacker.cpp"
#include "NativePhysicsOwners.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
static bool Write(HANDLE file,const void* data,unsigned int size){DWORD done=0;return !size||(WriteFile(file,data,size,&done,0)&&done==size);}
extern "C" void HarnessEntry(){
 char source[MAX_PATH],project[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_READER_INPUT",source,MAX_PATH)||!GetEnvironmentVariableA("FEAR_READER_PROJECT",project,MAX_PATH)||!GetEnvironmentVariableA("FEAR_PHYSICS_OWNERS_OUTPUT",output,MAX_PATH))ExitProcess(1);
 PrefabScene::Arena memory;void* place=memory.Allocate(sizeof(NativePreparedRenderScene::Context));if(!place)ExitProcess(2);NativePreparedRenderScene::Context* scene=new(place)NativePreparedRenderScene::Context;if(!scene->Build(source,project)){LogLine(scene->Diagnostic().message);ExitProcess(2);}
 NativePhysicsOwners::Settings settings;char mode[4];if(GetEnvironmentVariableA("FEAR_PHYSICS_MATRIX_DIAGNOSTIC",mode,4)&&mode[0]=='1')settings.exactOwnerCoordinates=false;
 NativePhysicsOwners::Builder result;if(!result.Build(*scene,settings)){char errorPath[MAX_PATH];lstrcpynA(errorPath,output,MAX_PATH-11);lstrcpyA(errorPath+lstrlenA(errorPath),".error.txt");HANDLE f=CreateFileA(errorPath,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(f!=INVALID_HANDLE_VALUE){Write(f,&result.failedGroup,4);Write(f,result.error,lstrlenA(result.error));CloseHandle(f);}ExitProcess(3);}
 scene->Clear();unsigned int header[10]={0x31575050u,3,result.count,result.size,result.noneCount,result.obbCount,result.sphereCount,result.compoundCount,result.explicitCount,result.unownedExplicitShapes};
 HANDLE file=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(file==INVALID_HANDLE_VALUE||!Write(file,header,sizeof(header)))ExitProcess(4);
 for(const NativePhysicsOwners::Record* record=result.first;record;record=record->next){unsigned int nameBytes=lstrlenA(record->name)+1;unsigned int fields[6]={record->group,record->modelIndex,record->type,record->bytes,record->children,nameBytes};if(!Write(file,fields,24)||!Write(file,record->name,nameBytes)||!Write(file,record->data,record->bytes))ExitProcess(4);}CloseHandle(file);scene->~Context();ExitProcess(0);
}
