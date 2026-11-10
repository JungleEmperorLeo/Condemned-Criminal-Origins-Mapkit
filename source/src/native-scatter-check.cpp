#define FEAR_PRIMARY_SURFACE_PREPARATION 1
#include "WorldPacker.cpp"
#include "NativeScatter.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
// Source-only Scatter producer check: FEAR_READER_INPUT/PROJECT, FEAR_SCATTER_OUTPUT
// (records concatenated as u32 bytes + payload), FEAR_SCATTER_CENTERED=0|1.
static bool Save(const char* path,const void* data,unsigned int bytes){HANDLE file=CreateFileA(path,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(file==INVALID_HANDLE_VALUE)return false;DWORD done=0;bool good=WriteFile(file,data,bytes,&done,0)&&done==bytes;CloseHandle(file);return good;}
extern "C" void HarnessEntry(){
 char source[MAX_PATH],project[MAX_PATH],output[MAX_PATH],mode[8]={};
 if(!GetEnvironmentVariableA("FEAR_READER_INPUT",source,MAX_PATH)||!GetEnvironmentVariableA("FEAR_READER_PROJECT",project,MAX_PATH)||!GetEnvironmentVariableA("FEAR_SCATTER_OUTPUT",output,MAX_PATH))ExitProcess(1);
 GetEnvironmentVariableA("FEAR_SCATTER_CENTERED",mode,8);
 PrefabScene::Arena memory;void* place=memory.Allocate(sizeof(NativePreparedRenderScene::Context));if(!place)ExitProcess(2);
 NativePreparedRenderScene::Context* scene=new(place)NativePreparedRenderScene::Context;NativeScatter::Builder scatter;NativeScatter::Settings settings;settings.centered=mode[0]!='0';char flags[32]={};if(GetEnvironmentVariableA("FEAR_SCATTER_REQUIRED_FLAGS",flags,32))for(const char* p=flags;*p;p++)settings.requiredFlags=settings.requiredFlags*16+(unsigned int)(*p<='9'?*p-'0':(*p|32)-'a'+10);char excluded[32]={};if(GetEnvironmentVariableA("FEAR_SCATTER_EXCLUDED_FLAGS",excluded,32))for(const char* p=excluded;*p;p++)settings.excludedFlags=settings.excludedFlags*16+(unsigned int)(*p<='9'?*p-'0':(*p|32)-'a'+10);
 if(!scene->Build(source,project)||!scatter.Build(*scene,settings)){char path[300];lstrcpyA(path,output);lstrcpyA(path+lstrlenA(path),".error.txt");const char* message=scatter.error?scatter.error:scene->Diagnostic().message;Save(path,message,lstrlenA(message));ExitProcess(3);}
 unsigned int total=4;for(unsigned int i=0;i<scatter.count;i++)total+=4+scatter.records[i].bytes;
 unsigned char* data=(unsigned char*)memory.Allocate(total+4*scatter.missingCount+4);if(!data)ExitProcess(4);unsigned int at=0;memcpy(data,&scatter.count,4);at=4;
 for(unsigned int i=0;i<scatter.count;i++){memcpy(data+at,&scatter.records[i].bytes,4);memcpy(data+at+4,scatter.records[i].data,scatter.records[i].bytes);at+=4+scatter.records[i].bytes;}
 memcpy(data+at,&scatter.missingCount,4);at+=4;for(unsigned int i=0;i<scatter.missingCount;i++){memcpy(data+at,scatter.missingNodes+i,4);at+=4;}
 if(!Save(output,data,at))ExitProcess(4);ExitProcess(0);
}
