#define FEAR_PRIMARY_SURFACE_PREPARATION 1
#include "WorldPacker.cpp"
#include "NativeObjectsBlindData.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
static bool Save(const char* path,const void* data,unsigned int bytes){HANDLE file=CreateFileA(path,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(file==INVALID_HANDLE_VALUE)return false;DWORD done=0;bool good=WriteFile(file,data,bytes,&done,0)&&done==bytes;CloseHandle(file);return good;}
extern "C" void HarnessEntry(){char source[MAX_PATH],project[MAX_PATH],objects[MAX_PATH],blind[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_READER_INPUT",source,MAX_PATH)||!GetEnvironmentVariableA("FEAR_READER_PROJECT",project,MAX_PATH)||!GetEnvironmentVariableA("FEAR_KEYFRAMER_OBJECTS_OUTPUT",objects,MAX_PATH)||!GetEnvironmentVariableA("FEAR_KEYFRAMER_BLIND_OUTPUT",blind,MAX_PATH))ExitProcess(1);PrefabScene::Arena memory;void* place=memory.Allocate(sizeof(NativePreparedRenderScene::Context));if(!place)ExitProcess(2);NativePreparedRenderScene::Context* scene=new(place)NativePreparedRenderScene::Context;NativeObjectsBlindData::Builder output;
 NativeObjectsBlindData::Settings settings;char navPath[MAX_PATH];unsigned char* nav=0;DWORD navNameBytes=GetEnvironmentVariableA("FEAR_NAVMESH_PROCESSED_INPUT",navPath,MAX_PATH);if(navNameBytes>=MAX_PATH)ExitProcess(5);
 if(navNameBytes){
  HANDLE f=CreateFileA(navPath,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);if(f==INVALID_HANDLE_VALUE)ExitProcess(5);DWORD size=GetFileSize(f,0),done=0;if(size<16||size>32u*1024u*1024u){CloseHandle(f);ExitProcess(5);}nav=(unsigned char*)memory.Allocate(size);if(!nav||!ReadFile(f,nav,size,&done,0)||done!=size){CloseHandle(f);ExitProcess(5);}CloseHandle(f);unsigned int header[4];memcpy(header,nav,16);if(header[0]!=0x3150564eu||header[1]!=1||header[2]>size-16||header[3]!=size-16-header[2])ExitProcess(5);settings.navigation.sourceRaw=nav+16;settings.navigation.sourceRawBytes=header[2];settings.navigation.processed=nav+16+header[2];settings.navigation.bytes=header[3];
 }
 if(!scene->Build(source,project)||!output.BuildKnownTypes(*scene,settings)){char path[300];lstrcpyA(path,blind);lstrcpyA(path+lstrlenA(path),".error.txt");const char* message=output.error?output.error:scene->Diagnostic().message;Save(path,message,lstrlenA(message));ExitProcess(3);}scene->Clear();scene->~Context();if(!Save(objects,output.objects,output.objectsBytes)||!Save(blind,output.blind,output.blindBytes))ExitProcess(4);ExitProcess(0);
}
