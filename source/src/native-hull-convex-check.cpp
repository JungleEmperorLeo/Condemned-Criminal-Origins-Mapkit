#include "WorldPacker.cpp"
#include "NativeHullConvex.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
extern "C" __declspec(dllimport) HMODULE WINAPI LoadLibraryA(const char*);
extern "C" __declspec(dllimport) void* WINAPI GetProcAddress(HMODULE,const char*);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
#pragma comment(linker,"/alternatename:__imp__LoadLibraryA@4=__imp__LoadLibraryA")
#pragma comment(linker,"/alternatename:__imp__GetProcAddress@8=__imp__GetProcAddress")
namespace HC=NativeHullConvex;
static bool Read(HANDLE f,void* p,unsigned int n){DWORD done=0;return !n||(ReadFile(f,p,n,&done,0)&&done==n);}
static bool Write(HANDLE f,const void* p,unsigned int n){DWORD done=0;return !n||(WriteFile(f,p,n,&done,0)&&done==n);}
static long WINAPI Exception(void* pointers){
 void** pair=(void**)pointers;unsigned int* context=(unsigned int*)pair[1];char path[MAX_PATH];
 if(GetEnvironmentVariableA("FEAR_NATIVE_HULL_OUTPUT",path,MAX_PATH)&&lstrlenA(path)<MAX_PATH-7){lstrcpyA(path+lstrlenA(path),".crash");HANDLE f=CreateFileA(path,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(f!=INVALID_HANDLE_VALUE){DWORD done=0;WriteFile(f,pair[0],20,&done,0);WriteFile(f,context+39,48,&done,0);CloseHandle(f);}}
 ExitProcess(25);return 1;
}
extern "C" void HarnessEntry(){
 HMODULE kernel=LoadLibraryA("kernel32.dll");typedef DWORD(WINAPI* Mode)(DWORD);typedef void*(WINAPI* Filter)(void*);
 Mode mode=(Mode)GetProcAddress(kernel,"SetErrorMode");if(mode)mode(3);Filter filter=(Filter)GetProcAddress(kernel,"SetUnhandledExceptionFilter");if(filter)filter((void*)Exception);
 char input[MAX_PATH],output[MAX_PATH],meta[MAX_PATH];
 if(!GetEnvironmentVariableA("FEAR_NATIVE_HULL_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_NATIVE_HULL_OUTPUT",output,MAX_PATH)||!GetEnvironmentVariableA("FEAR_NATIVE_HULL_META",meta,MAX_PATH))ExitProcess(1);
 HANDLE source=CreateFileA(input,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);
 if(source==INVALID_HANDLE_VALUE||GetFileSize(source,0)>8u*1024u*1024u)ExitProcess(2);
 unsigned int count=0;if(!Read(source,&count,4)||!count||count>2000)ExitProcess(3);
 HANDLE target=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);HANDLE metadata=CreateFileA(meta,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);
 if(target==INVALID_HANDLE_VALUE||metadata==INVALID_HANDLE_VALUE||!Write(target,&count,4)||!Write(metadata,&count,4))ExitProcess(4);
 void* workspace=VirtualAlloc(0,HC::WorkspaceBytes,MEM_COMMIT|MEM_RESERVE,4);if(!workspace)ExitProcess(5);
 for(unsigned int c=0;c<count;++c){
  unsigned int n=0;if(!Read(source,&n,4)||n<4||n>HC::MaximumPoints)ExitProcess(6);PrefabScene::Arena inputMemory;HC::Vec3* points=(HC::Vec3*)inputMemory.Allocate(n*12);if(!points||!Read(source,points,n*12))ExitProcess(6);
  unsigned int* zero=(unsigned int*)workspace;for(unsigned int j=0;j<HC::ScratchBytes/4;++j)zero[j]=0;
  HC::Context context;HC::Result result;HC::Status status=HC::Observe(points,n,workspace,HC::WorkspaceBytes,&context,&result);
  if(status!=HC::Success)ExitProcess(10+status);
  unsigned int header[5]={static_cast<unsigned int>(result.geometry.vertices.size),static_cast<unsigned int>(result.geometry.triangles.size),static_cast<unsigned int>(result.planes.size),context.allocationCalls,context.allocationBytes};
  unsigned int fields[2]={result.validated?1u:0u,result.attempts};
  if(!Write(metadata,fields,8)||!Write(target,header,20))ExitProcess(4);
  for(int v=0;v<result.geometry.vertices.size;++v)if(!Write(target,result.geometry.vertices.data+v,12))ExitProcess(4);
  if(!Write(target,result.geometry.triangles.data,result.geometry.triangles.size*12)||!Write(target,result.planes.data,result.planes.size*16))ExitProcess(4);
 }
 unsigned char extra;DWORD done=0;if(!ReadFile(source,&extra,1,&done,0)||done)ExitProcess(6);
 CloseHandle(source);CloseHandle(target);CloseHandle(metadata);VirtualFree(workspace,0,MEM_RELEASE);ExitProcess(0);
}
