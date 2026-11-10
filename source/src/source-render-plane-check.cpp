#include "WorldPacker.cpp"
#include "SourceRenderPlane.h"
#include "SourceSurfaceRotation.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
#ifdef FEAR_PLANE_ORACLE
extern "C" __declspec(dllimport) HMODULE WINAPI LoadLibraryA(const char*);
extern "C" __declspec(dllimport) void* WINAPI GetProcAddress(HMODULE,const char*);
#pragma comment(linker,"/alternatename:__imp__LoadLibraryA@4=__imp__LoadLibraryA")
#pragma comment(linker,"/alternatename:__imp__GetProcAddress@8=__imp__GetProcAddress")
// Original RVA 0x7630 has its source normal at EAX+4, output at EDI,
// and a transform pointer as its one caller-cleaned stack argument.
static void Invoke(void* target,void* source,void* transform,void* output){__asm {
 push edi
 mov edi,output
 mov eax,source
 push transform
 mov edx,target
 call edx
 add esp,4
 pop edi
}}
#endif
extern "C" void HarnessEntry(){
 char input[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_VERTEX_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_VERTEX_OUTPUT",output,MAX_PATH))ExitProcess(1);
#ifdef FEAR_PLANE_ORACLE
 HMODULE system=LoadLibraryA("kernel32.dll");typedef unsigned int (WINAPI *ErrorMode)(unsigned int);ErrorMode setErrorMode=system?(ErrorMode)GetProcAddress(system,"SetErrorMode"):0;if(setErrorMode)setErrorMode(3);
 LoadLibraryA("E:/fearsdk/Dev/Tools/msvcr71.dll");LoadLibraryA("E:/fearsdk/Dev/Tools/msvcp71.dll");LoadLibraryA("E:/fearsdk/Dev/Tools/MFC71.dll");unsigned char* dll=(unsigned char*)LoadLibraryA("E:/fearsdk/Dev/Tools/Packers/WorldPacker.dll");if(!dll)ExitProcess(2);
#endif
 HANDLE in=CreateFileA(input,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);HANDLE out=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);DWORD done;unsigned int count=0;if(in==INVALID_HANDLE_VALUE||out==INVALID_HANDLE_VALUE||!ReadFile(in,&count,4,&done,0)||done!=4||count>100000)ExitProcess(3);
 for(unsigned int i=0;i<count;i++){
  struct Input {WorldText::Vec3 normal;float quaternion[4];} values;
  WorldText::Vec3 result;if(!ReadFile(in,&values,28,&done,0)||done!=28)ExitProcess(4);
#ifdef FEAR_PLANE_ORACLE
  unsigned char source[16]={},transform[28]={};memcpy(source+4,&values.normal,12);memcpy(transform+12,values.quaternion,16);Invoke(dll+0x7630,source,transform,&result);
#else
  WorldText::Vec3 transformed=SourceSurfaceRotation::Rotate(values.quaternion,values.normal);if(!SourceRenderPlane::Build(transformed,result))ExitProcess(5);
#endif
  if(!WriteFile(out,&result,12,&done,0)||done!=12)ExitProcess(6);
 }CloseHandle(in);CloseHandle(out);ExitProcess(0);
}
