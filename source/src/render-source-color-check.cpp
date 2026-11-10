#include "WorldPacker.cpp"
#include "SourceRenderColor.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
extern "C" void HarnessEntry(){
 char input[MAX_PATH],output[MAX_PATH];
 if(!GetEnvironmentVariableA("FEAR_COLOR_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_COLOR_OUTPUT",output,MAX_PATH))ExitProcess(1);
 HANDLE in=CreateFileA(input,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);
 HANDLE out=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);
 DWORD done;unsigned int count=0;
 if(in==INVALID_HANDLE_VALUE||out==INVALID_HANDLE_VALUE||!ReadFile(in,&count,4,&done,0)||done!=4||count>100000)ExitProcess(2);
 for(unsigned int i=0;i<count;i++){
  float alpha;unsigned int color;
  if(!ReadFile(in,&alpha,4,&done,0)||done!=4||!SourceRenderColor::PackWhiteRGB(alpha,color))ExitProcess(3);
  if(!WriteFile(out,&color,4,&done,0)||done!=4)ExitProcess(4);
 }
 CloseHandle(in);CloseHandle(out);ExitProcess(0);
}
