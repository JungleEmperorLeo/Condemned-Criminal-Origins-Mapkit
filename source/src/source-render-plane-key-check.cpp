#include "WorldPacker.cpp"
#include "SourceRenderPlaneKey.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
#ifdef FEAR_PLANE_ORACLE
extern "C" __declspec(dllimport) HMODULE WINAPI LoadLibraryA(const char*);
extern "C" __declspec(dllimport) void* WINAPI GetProcAddress(HMODULE,const char*);
#pragma comment(linker,"/alternatename:__imp__LoadLibraryA@4=__imp__LoadLibraryA")
#pragma comment(linker,"/alternatename:__imp__GetProcAddress@8=__imp__GetProcAddress")
#endif
extern "C" void HarnessEntry(){
 char input[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_VERTEX_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_VERTEX_OUTPUT",output,MAX_PATH))ExitProcess(1);
#ifdef FEAR_PLANE_ORACLE
 HMODULE system=LoadLibraryA("kernel32.dll");typedef unsigned int (WINAPI *ErrorMode)(unsigned int);ErrorMode setErrorMode=system?(ErrorMode)GetProcAddress(system,"SetErrorMode"):0;if(setErrorMode)setErrorMode(3);
 LoadLibraryA("E:/fearsdk/Dev/Tools/msvcr71.dll");LoadLibraryA("E:/fearsdk/Dev/Tools/msvcp71.dll");LoadLibraryA("E:/fearsdk/Dev/Tools/MFC71.dll");unsigned char* dll=(unsigned char*)LoadLibraryA("E:/fearsdk/Dev/Tools/Packers/WorldPacker.dll");if(!dll)ExitProcess(2);
 typedef void* (__thiscall* Quantize)(void*,const void*);
#endif
 HANDLE in=CreateFileA(input,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);HANDLE out=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);DWORD done;unsigned int count=0;if(in==INVALID_HANDLE_VALUE||out==INVALID_HANDLE_VALUE||!ReadFile(in,&count,4,&done,0)||done!=4||count>100000)ExitProcess(3);
 for(unsigned int i=0;i<count;i++){
  WorldText::Vec3 normal;SourceRenderPlaneKey::Key result;if(!ReadFile(in,&normal,12,&done,0)||done!=12)ExitProcess(4);
#ifdef FEAR_PLANE_ORACLE
  ((Quantize)(dll+0x6a50))(&result,&normal);
#else
  if(!SourceRenderPlaneKey::Build(normal,result))ExitProcess(5);
#endif
  if(!WriteFile(out,&result,12,&done,0)||done!=12)ExitProcess(6);
 }CloseHandle(in);CloseHandle(out);ExitProcess(0);
}
