#include "WorldPacker.cpp"
#include "ExtrusionVolume.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
static bool Read(HANDLE f,void* p,unsigned int bytes){DWORD done;return ReadFile(f,p,bytes,&done,0)&&done==bytes;}
extern "C" void HarnessEntry(){
 char input[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_EXTRUSION_ENDS_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_EXTRUSION_ENDS_OUTPUT",output,MAX_PATH))ExitProcess(1);
 HANDLE f=CreateFileA(input,0x80000000,FILE_SHARE_READ,0,3,FILE_ATTRIBUTE_NORMAL,0);if(f==INVALID_HANDLE_VALUE)ExitProcess(2);
 unsigned int header[3];WorldText::Vec3 normal;if(!Read(f,header,12)||header[0]!=0x31534245||header[1]<3||header[1]>128||!header[2]||header[2]>8192||!Read(f,&normal,12))ExitProcess(3);
 PrefabScene::Arena memory;WorldText::Vec3* points=(WorldText::Vec3*)memory.Allocate(header[1]*12);ExtrusionVolume::Plane* planes=(ExtrusionVolume::Plane*)memory.Allocate(header[1]*16);if(!points||!planes||!Read(f,points,header[1]*12))ExitProcess(3);
 ExtrusionVolume::Candidate* candidates=(ExtrusionVolume::Candidate*)memory.Allocate(header[2]*sizeof(ExtrusionVolume::Candidate));if(!candidates)ExitProcess(3);
 for(unsigned int i=0;i<header[2];i++){
  ExtrusionVolume::Candidate& c=candidates[i];if(!Read(f,&c.count,4)||c.count<3||c.count>128||!Read(f,&c.plane,16))ExitProcess(3);
  WorldText::Vec3* vertices=(WorldText::Vec3*)memory.Allocate(c.count*12);if(!vertices||!Read(f,vertices,c.count*12))ExitProcess(3);c.points=vertices;
 }
 unsigned int extra;DWORD done;if(!ReadFile(f,&extra,1,&done,0)||done)ExitProcess(3);CloseHandle(f);
 ExtrusionVolume::Plane recomputed={};char recompute[4]={};bool computed=GetEnvironmentVariableA("FEAR_EXTRUSION_RECOMPUTE",recompute,4)&&recompute[0]=='1';
 if(computed){if(!ExtrusionVolume::ComputeInternalPlane(points,header[1],recomputed))ExitProcess(4);normal.x=recomputed.x;normal.y=recomputed.y;normal.z=recomputed.z;}
 ExtrusionVolume::Ends ends={};if(!ExtrusionVolume::SelectEnds(points,header[1],normal,candidates,header[2],ends))ExitProcess(4);
 unsigned int flags[2]={ends.hasPositive?1u:0u,ends.hasNegative?1u:0u};float values[10]={ends.positiveDistance,ends.negativeDistance,ends.positive.x,ends.positive.y,ends.positive.z,ends.positive.d,ends.negative.x,ends.negative.y,ends.negative.z,ends.negative.d};
 f=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(f==INVALID_HANDLE_VALUE)ExitProcess(5);if(!WriteFile(f,flags,8,&done,0)||done!=8||!WriteFile(f,values,40,&done,0)||done!=40)ExitProcess(6);if(computed&&(!WriteFile(f,&recomputed,16,&done,0)||done!=16))ExitProcess(6);CloseHandle(f);ExitProcess(0);
}
