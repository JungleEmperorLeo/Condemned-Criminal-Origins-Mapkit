#include "WorldPacker.cpp"
#include "ExtrusionVolume.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
static bool Read(HANDLE f,void* p,unsigned int bytes){DWORD done;return ReadFile(f,p,bytes,&done,0)&&done==bytes;}
extern "C" void HarnessEntry(){
 char input[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_EXTRUSION_SIDES_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_EXTRUSION_SIDES_OUTPUT",output,MAX_PATH))ExitProcess(1);
 HANDLE f=CreateFileA(input,0x80000000,FILE_SHARE_READ,0,3,FILE_ATTRIBUTE_NORMAL,0);if(f==INVALID_HANDLE_VALUE)ExitProcess(2);
 unsigned int header[2];WorldText::Vec3 normal;if(!Read(f,header,8)||header[0]!=0x31534253||header[1]<3||header[1]>128||!Read(f,&normal,12))ExitProcess(3);
 PrefabScene::Arena memory;WorldText::Vec3* points=(WorldText::Vec3*)memory.Allocate(header[1]*12);ExtrusionVolume::Plane* planes=(ExtrusionVolume::Plane*)memory.Allocate(header[1]*16);if(!points||!planes||!Read(f,points,header[1]*12))ExitProcess(3);
 unsigned int extra;DWORD done;if(!ReadFile(f,&extra,1,&done,0)||done)ExitProcess(3);CloseHandle(f);
 if(!ExtrusionVolume::BuildSides(points,header[1],normal,planes))ExitProcess(4);
 f=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(f==INVALID_HANDLE_VALUE)ExitProcess(5);if(!WriteFile(f,planes,header[1]*16,&done,0)||done!=header[1]*16)ExitProcess(6);CloseHandle(f);ExitProcess(0);
}
