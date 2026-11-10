#include "WorldPacker.cpp"
#include "RenderFrameSort.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
#ifdef FEAR_VERTEX_ORACLE
extern "C" __declspec(dllimport) HMODULE WINAPI LoadLibraryA(const char*);
#pragma comment(linker,"/alternatename:__imp__LoadLibraryA@4=__imp__LoadLibraryA")
#endif
extern "C" void HarnessEntry(){char input[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_VERTEX_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_VERTEX_OUTPUT",output,MAX_PATH))ExitProcess(1);HANDLE in=CreateFileA(input,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);DWORD done;unsigned int count=0;if(in==INVALID_HANDLE_VALUE||!ReadFile(in,&count,4,&done,0)||done!=4||count>100000)ExitProcess(2);PrefabScene::Arena memory;RenderFrameSort::Record* records=(RenderFrameSort::Record*)memory.Allocate((count+1)*72);if(!records||!ReadFile(in,records,count*72,&done,0)||done!=count*72)ExitProcess(3);CloseHandle(in);unsigned int ideal=count;char budget[8];if(GetEnvironmentVariableA("FEAR_SORT_IDEAL",budget,8)){if(budget[0]<'0'||budget[0]>'9'||budget[1])ExitProcess(4);ideal=(unsigned int)(budget[0]-'0');}
#ifdef FEAR_VERTEX_ORACLE
LoadLibraryA("E:/fearsdk/Dev/Tools/msvcr71.dll");LoadLibraryA("E:/fearsdk/Dev/Tools/msvcp71.dll");LoadLibraryA("E:/fearsdk/Dev/Tools/MFC71.dll");unsigned char* dll=(unsigned char*)LoadLibraryA("E:/fearsdk/Dev/Tools/Packers/WorldPacker.dll");if(!dll)ExitProcess(4);typedef void (__cdecl* Sort)(void*,void*,int,void*);((Sort)(dll+0x45cf0))(records,records+count,(int)ideal,dll+0x3f680);
#else
if(!RenderFrameSort::Range(records,records+count,ideal))ExitProcess(4);
#endif
HANDLE out=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(out==INVALID_HANDLE_VALUE||!WriteFile(out,records,count*72,&done,0)||done!=count*72)ExitProcess(5);CloseHandle(out);ExitProcess(0);}
