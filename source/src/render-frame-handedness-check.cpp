#include "WorldPacker.cpp"
#include "RenderAttributes.h"
#include "RenderFrameHandedness.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
#ifdef FEAR_VERTEX_ORACLE
extern "C" __declspec(dllimport) HMODULE WINAPI LoadLibraryA(const char*);
#pragma comment(linker,"/alternatename:__imp__LoadLibraryA@4=__imp__LoadLibraryA")
static unsigned int Invoke(void* target,void* polygon){unsigned int result;__asm {
 push ebx
 mov ebx,polygon
 mov eax,target
 call eax
 mov result,eax
 pop ebx
}return result;}

#endif
extern "C" void HarnessEntry(){
 char input[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_VERTEX_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_VERTEX_OUTPUT",output,MAX_PATH))ExitProcess(1);
#ifdef FEAR_VERTEX_ORACLE
 LoadLibraryA("E:/fearsdk/Dev/Tools/msvcr71.dll");LoadLibraryA("E:/fearsdk/Dev/Tools/msvcp71.dll");LoadLibraryA("E:/fearsdk/Dev/Tools/MFC71.dll");unsigned char* dll=(unsigned char*)LoadLibraryA("E:/fearsdk/Dev/Tools/Packers/WorldPacker.dll");if(!dll)ExitProcess(2);
#endif
 HANDLE in=CreateFileA(input,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);HANDLE out=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);DWORD done;unsigned int count=0;if(in==INVALID_HANDLE_VALUE||out==INVALID_HANDLE_VALUE||!ReadFile(in,&count,4,&done,0)||done!=4||count>10000)ExitProcess(3);
 for(unsigned int i=0;i<count;i++){
 WorldText::Vec3 values[3];unsigned int result=0;if(!ReadFile(in,values,36,&done,0)||done!=36)ExitProcess(4);
#ifdef FEAR_VERTEX_ORACLE
 unsigned char polygon[64]={},surface[176]={},plane[20]={};*(void**)(polygon+0x38)=surface;*(void**)(surface+0xa4)=plane;memcpy(surface+0x14,values,12);memcpy(surface+0x20,values+1,12);memcpy(plane+8,values+2,12);result=Invoke(dll+0x41480,polygon);
#else
 if(!RenderFrameHandedness::Build(values[0],values[1],values[2],result))ExitProcess(5);
#endif
 if(!WriteFile(out,&result,4,&done,0)||done!=4)ExitProcess(6);
 }CloseHandle(in);CloseHandle(out);ExitProcess(0);
}
