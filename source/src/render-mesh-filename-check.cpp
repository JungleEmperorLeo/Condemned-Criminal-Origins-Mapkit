#include "WorldPacker.cpp"
#include "RenderMeshFilename.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
extern "C" __declspec(dllimport) HMODULE WINAPI LoadLibraryA(const char*);
extern "C" __declspec(dllimport) void* WINAPI GetProcAddress(HMODULE,const char*);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
#pragma comment(linker,"/alternatename:__imp__LoadLibraryA@4=__imp__LoadLibraryA")
#pragma comment(linker,"/alternatename:__imp__GetProcAddress@8=__imp__GetProcAddress")
static bool Read(HANDLE f,void* out,unsigned int n){DWORD done;return ReadFile(f,out,n,&done,0)&&done==n;}
extern "C" void HarnessEntry(){
 char input[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_FILENAME_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_FILENAME_OUTPUT",output,MAX_PATH))ExitProcess(1);HANDLE f=CreateFileA(input,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);unsigned int h[4];if(f==INVALID_HANDLE_VALUE||!Read(f,h,16)||h[0]!=0x314e4d46||h[1]>4096||h[2]>4096||h[3]>4096)ExitProcess(2);
 PrefabScene::Arena memory;char* text[3];for(unsigned int i=0;i<3;i++){text[i]=(char*)memory.Allocate(h[i+1]+1);if(!text[i]||!Read(f,text[i],h[i+1]))ExitProcess(3);text[i][h[i+1]]=0;}CloseHandle(f);
 char* filename=(char*)memory.Allocate(RenderMeshFilename::MaximumFilename+1);char reference[260];bool relative=false;if(!filename||!RenderMeshFilename::Build(text[0],text[1],text[2],filename,RenderMeshFilename::MaximumFilename+1,reference,sizeof(reference),&relative))ExitProcess(4);
#ifdef FEAR_FILENAME_ORACLE
 HMODULE kernel=LoadLibraryA("kernel32.dll");typedef unsigned int(WINAPI* Mode)(unsigned int);Mode errorMode=kernel?(Mode)GetProcAddress(kernel,"SetErrorMode"):0;if(!errorMode)ExitProcess(5);errorMode(3);LoadLibraryA("E:/fearsdk/Dev/Tools/msvcr71.dll");LoadLibraryA("E:/fearsdk/Dev/Tools/msvcp71.dll");LoadLibraryA("E:/fearsdk/Dev/Tools/MFC71.dll");unsigned char* dll=(unsigned char*)LoadLibraryA("E:/fearsdk/Dev/Tools/Packers/WorldPacker.dll");if(!dll||dll[0x53fb0]!=0x56)ExitProcess(5);
 unsigned char project[84]={};if(h[3]<16)memcpy(project+0x3c,text[2],h[3]+1);else *(const char**)(project+0x3c)=text[2];*(unsigned int*)(project+0x4c)=h[3];*(unsigned int*)(project+0x50)=h[3]<16?15:h[3];typedef bool(__thiscall* Reference)(void*,const char*,char*,unsigned int);relative=((Reference)(dll+0x53fb0))(project,filename,reference,sizeof(reference));
#endif
 unsigned int out[4]={0x314e4d46,(unsigned int)lstrlenA(filename),(unsigned int)lstrlenA(reference),relative?1u:0u};f=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);DWORD done;if(f==INVALID_HANDLE_VALUE||!WriteFile(f,out,16,&done,0)||done!=16||!WriteFile(f,filename,out[1],&done,0)||done!=out[1]||!WriteFile(f,reference,out[2],&done,0)||done!=out[2])ExitProcess(6);CloseHandle(f);ExitProcess(0);
}
