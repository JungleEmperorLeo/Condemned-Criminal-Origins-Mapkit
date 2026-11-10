#include "WorldPacker.cpp"
#include "RenderAttributes.h"
#include "RenderFrameProjection.h"
#include "RenderFrameAggregation.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
#ifdef FEAR_VERTEX_ORACLE
extern "C" __declspec(dllimport) HMODULE WINAPI LoadLibraryA(const char*);
#pragma comment(linker,"/alternatename:__imp__LoadLibraryA@4=__imp__LoadLibraryA")
static void Invoke(void* target,void* polygon,void* outP,void* outQ){__asm {
 push ebx
 push esi
 push edi
 mov ebx,polygon
 mov edi,outP
 mov esi,outQ
 mov eax,target
 call eax
 pop edi
 pop esi
 pop ebx
}}
#endif
extern "C" void HarnessEntry(){
 char input[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_VERTEX_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_VERTEX_OUTPUT",output,MAX_PATH))ExitProcess(1);
 char perpendicular[4];bool perp=GetEnvironmentVariableA("FEAR_FRAME_PERPENDICULAR",perpendicular,4)&&perpendicular[0]=='1';
#ifdef FEAR_VERTEX_ORACLE
 LoadLibraryA("E:/fearsdk/Dev/Tools/msvcr71.dll");LoadLibraryA("E:/fearsdk/Dev/Tools/msvcp71.dll");LoadLibraryA("E:/fearsdk/Dev/Tools/MFC71.dll");unsigned char* dll=(unsigned char*)LoadLibraryA("E:/fearsdk/Dev/Tools/Packers/WorldPacker.dll");if(!dll)ExitProcess(2);
 // Private helper diagnostic only: select the original "ignore assertion"
 // state in this process, so singular-axis observations never open an MFC box.
 // No code or on-disk DLL bytes are changed; the native target has no loader.
 char ignored[4];if(GetEnvironmentVariableA("FEAR_FRAME_IGNORE_ASSERTIONS",ignored,4)&&ignored[0]=='1')dll[0x9c4bd]=1;
#endif
 HANDLE in=CreateFileA(input,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);HANDLE out=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);DWORD done;unsigned int count=0;if(in==INVALID_HANDLE_VALUE||out==INVALID_HANDLE_VALUE||!ReadFile(in,&count,4,&done,0)||done!=4||count>10000)ExitProcess(3);
 for(unsigned int i=0;i<count;i++){
 WorldText::Vec3 values[3],result[2]={};if(!ReadFile(in,values,36,&done,0)||done!=36)ExitProcess(4);
#ifdef FEAR_VERTEX_ORACLE
 if(perp){typedef WorldText::Vec3* (__thiscall* Perpendicular)(const WorldText::Vec3*,WorldText::Vec3*);((Perpendicular)(dll+0x41690))(values+2,result);}
 else {unsigned char polygon[64]={},surface[176]={},plane[20]={};*(void**)(polygon+0x38)=surface;*(void**)(surface+0xa4)=plane;memcpy(surface+0x14,values,12);memcpy(surface+0x20,values+1,12);memcpy(plane+8,values+2,12);Invoke(dll+0x41520,polygon,result,result+1);}
#else
 if(perp){if(!RenderFrameAggregation::Perpendicular(values[2],result[0]))ExitProcess(5);}else if(!RenderFrameProjection::Build(values[0],values[1],values[2],result[0],result[1]))ExitProcess(5);
#endif
 if(!WriteFile(out,result,24,&done,0)||done!=24)ExitProcess(6);
 }CloseHandle(in);CloseHandle(out);ExitProcess(0);
}
