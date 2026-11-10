#define MAX_PATH 260
typedef void* HANDLE;
extern "C" {
__declspec(dllimport) unsigned int __stdcall SetErrorMode(unsigned int);
__declspec(dllimport) unsigned int __stdcall GetEnvironmentVariableA(const char*,char*,unsigned int);
__declspec(dllimport) void* __stdcall LoadLibraryA(const char*);
__declspec(dllimport) void __stdcall ExitProcess(unsigned int);
__declspec(dllimport) void* __stdcall VirtualAlloc(void*,unsigned int,unsigned int,unsigned int);
__declspec(dllimport) int __stdcall VirtualFree(void*,unsigned int,unsigned int);
__declspec(dllimport) HANDLE __stdcall CreateFileA(const char*,unsigned int,unsigned int,void*,unsigned int,unsigned int,HANDLE);
__declspec(dllimport) int __stdcall ReadFile(HANDLE,void*,unsigned int,unsigned int*,void*);
__declspec(dllimport) int __stdcall WriteFile(HANDLE,const void*,unsigned int,unsigned int*,void*);
__declspec(dllimport) int __stdcall CloseHandle(HANDLE);
}
#pragma comment(linker,"/alternatename:__imp__SetErrorMode@4=__imp__SetErrorMode")
#pragma comment(linker,"/alternatename:__imp__GetEnvironmentVariableA@12=__imp__GetEnvironmentVariableA")
#pragma comment(linker,"/alternatename:__imp__LoadLibraryA@4=__imp__LoadLibraryA")
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
#pragma comment(linker,"/alternatename:__imp__VirtualAlloc@16=__imp__VirtualAlloc")
#pragma comment(linker,"/alternatename:__imp__VirtualFree@12=__imp__VirtualFree")
#pragma comment(linker,"/alternatename:__imp__CreateFileA@28=__imp__CreateFileA")
#pragma comment(linker,"/alternatename:__imp__ReadFile@20=__imp__ReadFile")
#pragma comment(linker,"/alternatename:__imp__WriteFile@20=__imp__WriteFile")
#pragma comment(linker,"/alternatename:__imp__CloseHandle@4=__imp__CloseHandle")
#include "RenderNvRuntime.h"
extern "C" {int _fltused=0;}
extern "C" void* __cdecl memset(void* target,int value,unsigned int n) {unsigned char* p=(unsigned char*)target;while(n--)*p++=(unsigned char)value;return target;}
extern "C" void* __cdecl memcpy(void* target,const void* source,unsigned int n) {unsigned char* p=(unsigned char*)target;const unsigned char* q=(const unsigned char*)source;while(n--)*p++=*q++;return target;}
#include "RenderIndexReorder.h"
extern "C" void HarnessEntry() {
    SetErrorMode(3);
    char input[MAX_PATH],output[MAX_PATH];
    if(!GetEnvironmentVariableA("FEAR_VERTEX_INPUT",input,MAX_PATH) || !GetEnvironmentVariableA("FEAR_VERTEX_OUTPUT",output,MAX_PATH)) ExitProcess(1);
    HANDLE in=CreateFileA(input,0x80000000,1,0,3,128,0);unsigned int done,count=0;
    if(in==(HANDLE)-1 || !ReadFile(in,&count,4,&done,0) || done!=4 || count>100000) ExitProcess(2);
    NvCompat::Vector<RenderIndexReorder::Triangle> triangles(count);
    if(count && (!ReadFile(in,triangles.data(),count*12,&done,0) || done!=count*12)) ExitProcess(4);
    CloseHandle(in);
    NvCompat::Vector<RenderIndexReorder::Triangle> result;
#ifdef FEAR_VERTEX_ORACLE
    LoadLibraryA("E:/fearsdk/Dev/Tools/msvcr71.dll");LoadLibraryA("E:/fearsdk/Dev/Tools/msvcp71.dll");LoadLibraryA("E:/fearsdk/Dev/Tools/MFC71.dll");
    unsigned char* dll=(unsigned char*)LoadLibraryA("E:/fearsdk/Dev/Tools/Packers/WorldPacker.dll");if(!dll) ExitProcess(5);
    // Node sections +4/+8/+12; triangles +20/+24/+28. Section stride40,count+24.
    unsigned int section[10]={0};section[6]=count;
    unsigned int node[8]={0};node[1]=(unsigned int)section;node[2]=(unsigned int)(section+10);node[3]=node[2];
    typedef void* (__cdecl* Allocate)(unsigned int);
    typedef void (__cdecl* Free)(void*);
    void* allocated=((Allocate)(dll+0x62da0))((count+1)*12);
    if(!allocated) ExitProcess(6);
    if(count) memcpy(allocated,triangles.data(),count*12);
    node[5]=(unsigned int)allocated;node[6]=node[5]+count*12;node[7]=node[5]+(count+1)*12;
    typedef void (__thiscall* Reorder)(void*);((Reorder)(dll+0x49740))(node);
    unsigned int reorderedCount=(node[6]-node[5])/12;
    if(reorderedCount>count || reorderedCount!=section[6]) ExitProcess(7);
    result.assign((RenderIndexReorder::Triangle*)node[5],(RenderIndexReorder::Triangle*)node[5]+reorderedCount);
    ((Free)(dll+0x62c60))((void*)node[5]);
#else
    if(!RenderIndexReorder::Section(triangles.data(),count,result)) ExitProcess(5);
#endif
    HANDLE out=CreateFileA(output,0x40000000,1,0,2,128,0);if(out==(HANDLE)-1)ExitProcess(8);
    unsigned int returned=(unsigned int)result.size();
    if(!WriteFile(out,&returned,4,&done,0)||done!=4 || !WriteFile(out,result.data(),returned*12,&done,0)||done!=returned*12) ExitProcess(9);
    CloseHandle(out);ExitProcess(0);
}
