#define FEAR_PRIMARY_SURFACE_PREPARATION 1
#include "WorldPacker.cpp"
#include "NativePhysicsCompiler.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
static bool Read(const char* path,PrefabScene::Arena& memory,const void*& result,unsigned int& size){HANDLE f=CreateFileA(path,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);if(f==INVALID_HANDLE_VALUE)return false;size=GetFileSize(f,0);if(!size||size>16u*1024u*1024u){CloseHandle(f);return false;}void* data=memory.Allocate(size);DWORD got=0;bool ok=data&&ReadFile(f,data,size,&got,0)&&got==size;CloseHandle(f);result=data;return ok;}
extern "C" void HarnessEntry(){
 char source[MAX_PATH],project[MAX_PATH],mesh[MAX_PATH],code[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_READER_INPUT",source,MAX_PATH)||!GetEnvironmentVariableA("FEAR_READER_PROJECT",project,MAX_PATH)||!GetEnvironmentVariableA("FEAR_PHYSICS_ASSOCIATED_MESH",mesh,MAX_PATH)||!GetEnvironmentVariableA("FEAR_MOPP_INPUT",code,MAX_PATH)||!GetEnvironmentVariableA("FEAR_PHYSICS_COMPILER_OUTPUT",output,MAX_PATH))ExitProcess(1);
 PrefabScene::Arena memory;void* place=memory.Allocate(sizeof(NativePreparedRenderScene::Context));if(!place)ExitProcess(2);NativePreparedRenderScene::Context* scene=new(place)NativePreparedRenderScene::Context;if(!scene->Build(source,project)){LogLine(scene->Diagnostic().message);ExitProcess(2);}
 NativePhysicsCompiler::MoppInput mopp;if(!Read(mesh,memory,mopp.mesh,mopp.meshBytes)||!Read(code,memory,mopp.code,mopp.codeBytes))ExitProcess(3);
 NativePhysicsCompiler::Settings settings;NativePhysicsOwnerMeshTransport::Parser ownerMopp;char ownerInput[MAX_PATH];DWORD ownerLength=GetEnvironmentVariableA("FEAR_PHYSICS_OWNER_MOPP_INPUT",ownerInput,MAX_PATH);if(ownerLength){const void* packet=0;unsigned int packetBytes=0;if(ownerLength>=MAX_PATH||!Read(ownerInput,memory,packet,packetBytes)||!ownerMopp.Build(packet,packetBytes)){LogLine(ownerMopp.error?ownerMopp.error:"Cannot read owner Mesh MOPP associations");ExitProcess(3);}settings.owners.meshAssociations=ownerMopp.records;settings.owners.meshAssociationCount=ownerMopp.count;}
 NativePhysicsCompiler::Builder result;if(!result.Build(*scene,mopp,settings)){LogLine(result.error);ExitProcess(4);}scene->Clear();scene->~Context();ownerMopp.Clear();
 HANDLE file=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);DWORD wrote=0;if(file==INVALID_HANDLE_VALUE||!WriteFile(file,result.data,result.size,&wrote,0)||wrote!=result.size)ExitProcess(5);CloseHandle(file);ExitProcess(0);
}
