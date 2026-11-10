#include "WorldPacker.cpp"
#include "RenderClipVertex.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
#ifdef FEAR_VERTEX_ORACLE
extern "C" __declspec(dllimport) HMODULE WINAPI LoadLibraryA(const char*);
#pragma comment(linker,"/alternatename:__imp__LoadLibraryA@4=__imp__LoadLibraryA")
#endif
#ifndef FEAR_VERTEX_ORACLE
static bool Guards(){
 RenderClipVertex::Vertex a={},b={},out={};a.flags=123;b.flags=456;out.flags=789;
 if(RenderClipVertex::Interpolate(a,b,.5f,out)||out.flags!=789)return false;
 a.attributes[6]=2;b.attributes[6]=4;
 if(!RenderClipVertex::Interpolate(a,b,0,out)||out.flags!=123||out.attributes[6]!=1)return false;
 if(!RenderClipVertex::Interpolate(a,b,1,out)||out.flags!=123||out.attributes[6]!=1)return false;
 return true;
}
#endif
extern "C" void HarnessEntry(){
 char input[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_VERTEX_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_VERTEX_OUTPUT",output,MAX_PATH))ExitProcess(1);
#ifdef FEAR_VERTEX_ORACLE
 LoadLibraryA("E:\\fearsdk\\Dev\\Tools\\msvcr71.dll");LoadLibraryA("E:\\fearsdk\\Dev\\Tools\\msvcp71.dll");LoadLibraryA("E:\\fearsdk\\Dev\\Tools\\MFC71.dll");unsigned char* dll=(unsigned char*)LoadLibraryA("E:\\fearsdk\\Dev\\Tools\\Packers\\WorldPacker.dll");if(!dll)ExitProcess(2);
 typedef RenderClipVertex::Vertex* (__thiscall* Mix)(const RenderClipVertex::Vertex*,RenderClipVertex::Vertex*,const RenderClipVertex::Vertex*,float);Mix mix=(Mix)(dll+0x409c0);
#endif
#ifndef FEAR_VERTEX_ORACLE
 if(!Guards())ExitProcess(7);
#endif
 HANDLE in=CreateFileA(input,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);HANDLE out=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);DWORD done;unsigned int count=0;if(in==INVALID_HANDLE_VALUE||out==INVALID_HANDLE_VALUE||!ReadFile(in,&count,4,&done,0)||done!=4||count>10000)ExitProcess(3);
 for(unsigned int i=0;i<count;i++){RenderClipVertex::Vertex a={},b={},result={};float t=0;if(!ReadFile(in,&a,44,&done,0)||done!=44||!ReadFile(in,&b,44,&done,0)||done!=44||!ReadFile(in,&t,4,&done,0)||done!=4)ExitProcess(4);
#ifdef FEAR_VERTEX_ORACLE
 mix(&a,&result,&b,t);
#else
 if(!RenderClipVertex::Interpolate(a,b,t,result))ExitProcess(5);
#endif
 if(!WriteFile(out,&result,44,&done,0)||done!=44)ExitProcess(6);
 }CloseHandle(in);CloseHandle(out);ExitProcess(0);
}
