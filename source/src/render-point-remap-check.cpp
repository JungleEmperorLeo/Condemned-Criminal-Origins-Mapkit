#include "WorldPacker.cpp"
#include "RenderPointRemap.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
static unsigned int refs[2048],rename[2048],result[2048],copied[2048];
extern "C" void HarnessEntry(){
 char input[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_REMAP_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_REMAP_OUTPUT",output,MAX_PATH))ExitProcess(1);
 HANDLE in=CreateFileA(input,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);DWORD done;unsigned int header[2];
 if(in==INVALID_HANDLE_VALUE||!ReadFile(in,header,8,&done,0)||done!=8||header[0]>2048||header[1]>2048||!ReadFile(in,refs,header[1]*4,&done,0)||done!=header[1]*4)ExitProcess(2);CloseHandle(in);
 unsigned int count=0;if(!RenderPointRemap::Build(header[0],refs,header[1],rename,result,copied,2048,count))ExitProcess(3);
 HANDLE out=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);
 if(out==INVALID_HANDLE_VALUE||!WriteFile(out,&count,4,&done,0)||done!=4||!WriteFile(out,result,header[1]*4,&done,0)||done!=header[1]*4||!WriteFile(out,copied,count*4,&done,0)||done!=count*4)ExitProcess(4);
 CloseHandle(out);ExitProcess(0);
}
