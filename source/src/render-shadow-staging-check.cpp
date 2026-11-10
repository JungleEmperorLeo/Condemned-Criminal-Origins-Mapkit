#include "WorldPacker.cpp"
#include "RenderShadowStaging.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
extern "C" void HarnessEntry(){
 char input[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_VERTEX_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_VERTEX_OUTPUT",output,MAX_PATH))ExitProcess(1);
 HANDLE in=CreateFileA(input,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);DWORD done;unsigned int n=0;
 if(in==INVALID_HANDLE_VALUE||!ReadFile(in,&n,4,&done,0)||done!=4||n>6000)ExitProcess(2);
 PrefabScene::Arena memory;RenderShadowStaging::Input* triangles=(RenderShadowStaging::Input*)memory.Allocate((n?n:1)*52);
 if(!triangles||!ReadFile(in,triangles,n*52,&done,0)||done!=n*52)ExitProcess(3);unsigned char tail;if(!ReadFile(in,&tail,1,&done,0)||done)ExitProcess(3);CloseHandle(in);
 RenderShadowStaging::Builder builder;if(!builder.Build(triangles,n))ExitProcess(4);
 HANDLE out=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);
 if(out==INVALID_HANDLE_VALUE||!WriteFile(out,&builder.count,4,&done,0)||done!=4||!WriteFile(out,builder.triangles,builder.count*256,&done,0)||done!=builder.count*256)ExitProcess(5);CloseHandle(out);
 char statsPath[MAX_PATH];if(GetEnvironmentVariableA("FEAR_SHADOW_STAGING_STATS",statsPath,MAX_PATH)){
  unsigned int stats[4]={builder.count,builder.oneSided,builder.twoSided,builder.boundaryEdges};HANDLE file=CreateFileA(statsPath,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);
  if(file==INVALID_HANDLE_VALUE||!WriteFile(file,stats,16,&done,0)||done!=16)ExitProcess(5);CloseHandle(file);
 }
 ExitProcess(0);
}
