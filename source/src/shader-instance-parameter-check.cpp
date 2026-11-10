#include "WorldPacker.cpp"
#include "ShaderInstanceParameter.h"
extern "C" __declspec(dllimport) HMODULE WINAPI LoadLibraryA(const char*);
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__LoadLibraryA@4=__imp__LoadLibraryA")
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
extern "C" void HarnessEntry(){
 char input[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_INSTANCE_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_INSTANCE_OUTPUT",output,MAX_PATH))ExitProcess(1);
 HANDLE file=CreateFileA(input,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);DWORD done;unsigned int count=0;if(file==INVALID_HANDLE_VALUE||!ReadFile(file,&count,4,&done,0)||done!=4||!count||count>100000)ExitProcess(2);
 PrefabScene::Arena memory;ShaderInstanceParameter::Descriptor* p=(ShaderInstanceParameter::Descriptor*)memory.Allocate(count*44);unsigned int* kinds=(unsigned int*)memory.Allocate(count*4);if(!p||!kinds||!ReadFile(file,p,count*44,&done,0)||done!=count*44)ExitProcess(3);CloseHandle(file);
#ifdef FEAR_INSTANCE_ORACLE
 LoadLibraryA("E:/fearsdk/Dev/Tools/msvcr71.dll");LoadLibraryA("E:/fearsdk/Dev/Tools/msvcp71.dll");LoadLibraryA("E:/fearsdk/Dev/Tools/MFC71.dll");unsigned char* original=(unsigned char*)LoadLibraryA("E:/fearsdk/Dev/Tools/Packers/WorldPacker.dll");if(!original)ExitProcess(4);const unsigned char signature[4]={0x57,0x8b,0x79,0x08};for(unsigned int i=0;i<4;i++)if(original[0x7b1c0+i]!=signature[i])ExitProcess(5);typedef unsigned int (__thiscall *Classify)(const void*);Classify classify=(Classify)(original+0x7b1c0);
#endif
 for(unsigned int i=0;i<count;i++){
#ifdef FEAR_INSTANCE_ORACLE
  kinds[i]=classify(p+i);
#else
  kinds[i]=ShaderInstanceParameter::Classify(p[i]);
#endif
 }
 file=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(file==INVALID_HANDLE_VALUE||!WriteFile(file,&count,4,&done,0)||done!=4||!WriteFile(file,kinds,count*4,&done,0)||done!=count*4)ExitProcess(6);CloseHandle(file);ExitProcess(0);
}
