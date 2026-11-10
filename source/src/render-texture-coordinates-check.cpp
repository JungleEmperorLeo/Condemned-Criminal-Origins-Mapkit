#include "WorldPacker.cpp"
#include "RenderTextureCoordinates.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
#ifdef FEAR_VERTEX_ORACLE
extern "C" __declspec(dllimport) HMODULE WINAPI LoadLibraryA(const char*);
#pragma comment(linker,"/alternatename:__imp__LoadLibraryA@4=__imp__LoadLibraryA")
#endif
extern "C" void HarnessEntry(){
 char input[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_VERTEX_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_VERTEX_OUTPUT",output,MAX_PATH))ExitProcess(1);
#ifdef FEAR_VERTEX_ORACLE
 LoadLibraryA("E:\\fearsdk\\Dev\\Tools\\msvcr71.dll");LoadLibraryA("E:\\fearsdk\\Dev\\Tools\\msvcp71.dll");LoadLibraryA("E:\\fearsdk\\Dev\\Tools\\MFC71.dll");unsigned char* dll=(unsigned char*)LoadLibraryA("E:\\fearsdk\\Dev\\Tools\\Packers\\WorldPacker.dll");if(!dll)ExitProcess(2);
 typedef void (__thiscall* Compute)(void*,const WorldText::Vec3*,void*);Compute compute=(Compute)(dll+0x3fd00);
#endif

 HANDLE in=CreateFileA(input,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);HANDLE out=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);DWORD done;unsigned int count=0;if(in==INVALID_HANDLE_VALUE||out==INVALID_HANDLE_VALUE||!ReadFile(in,&count,4,&done,0)||done!=4||count>10000)ExitProcess(3);
 for(unsigned int i=0;i<count;i++){
 WorldText::Vec3 position={};RenderTextureCoordinates::Basis basis[4]={};float result[8]={};
 if(!ReadFile(in,&position,12,&done,0)||done!=12||!ReadFile(in,basis,144,&done,0)||done!=144)ExitProcess(4);
#ifdef FEAR_VERTEX_ORACLE
 unsigned char surface[160]={},polygon[64]={},vertex[84]={};memcpy(surface+8,basis,144);*(void**)(polygon+0x38)=surface;compute(vertex,&position,polygon);memcpy(result,vertex+12,32);
#else
 if(!RenderTextureCoordinates::Build(position,basis,result))ExitProcess(5);
#endif
 if(!WriteFile(out,result,32,&done,0)||done!=32)ExitProcess(6);
 }CloseHandle(in);CloseHandle(out);ExitProcess(0);
}
