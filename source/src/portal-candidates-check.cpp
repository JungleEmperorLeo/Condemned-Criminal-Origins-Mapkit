#include "WorldPacker.cpp"
#include "PortalCandidates.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
extern "C" __declspec(dllimport) void* WINAPI LoadLibraryA(const char*);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
#pragma comment(linker,"/alternatename:__imp__LoadLibraryA@4=__imp__LoadLibraryA")
extern "C" void HarnessEntry(){
 LoadLibraryA("E:\\fearsdk\\Dev\\Tools\\msvcr71.dll");LoadLibraryA("E:\\fearsdk\\Dev\\Tools\\msvcp71.dll");LoadLibraryA("E:\\fearsdk\\Dev\\Tools\\MFC71.dll");unsigned char* dll=(unsigned char*)LoadLibraryA("E:\\fearsdk\\Dev\\Tools\\Packers\\WorldPacker.dll");if(!dll)ExitProcess(2);
 char input[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_CANDIDATES_INPUT",input,MAX_PATH))ExitProcess(1);HANDLE f=CreateFileA(input,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);DWORD done;unsigned int count=0;if(f==INVALID_HANDLE_VALUE||!ReadFile(f,&count,4,&done,0)||done!=4)ExitProcess(3);typedef bool (__thiscall *Test)(void*,void*,float);
 for(unsigned int i=0;i<count;i++){float values[13];if(!ReadFile(f,values,52,&done,0)||done!=52)ExitProcess(4);unsigned int a[16]={},b[16]={};for(unsigned int j=0;j<6;j++){((float*)a)[5+j]=values[j];((float*)b)[5+j]=values[6+j];}bool own=PortalCandidates::BoundsOverlap(*(WorldText::Vec3*)values,*(WorldText::Vec3*)(values+3),*(WorldText::Vec3*)(values+6),*(WorldText::Vec3*)(values+9),values[12]);bool original=((Test)(dll+0x124a0))(a,b,values[12]);if(own!=original)ExitProcess(20);}
 CloseHandle(f);ExitProcess(0);
}

