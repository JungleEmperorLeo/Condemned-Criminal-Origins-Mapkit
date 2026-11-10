#include "WorldPacker.cpp"
#include "SourcePhysicsSphereFit.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
static bool Empty(const float* sphere){return sphere[0]==0&&sphere[1]==0&&sphere[2]==0&&sphere[3]==-1;}
static float Bits(unsigned int value){union Pair {unsigned int u;float f;};Pair pair;pair.u=value;return pair.f;}
extern "C" void HarnessEntry(){
 using namespace SourcePhysicsSphereFit;
 unsigned int results[11]={};float sphere[4],epsilon=Bits(0x3727c5acu);Vec4 points[3]={{1,2,3,0},{4,7,9,0},{-3,-4,5,0}};
 results[0]=Fit(0,0,epsilon,sphere)&&Empty(sphere);
 results[1]=!Fit(0,1,epsilon,sphere)&&Empty(sphere);
 results[2]=!Fit(points,3,Bits(0x7fc00000u),sphere)&&Empty(sphere);
 results[3]=!Fit(points,3,-epsilon,sphere)&&Empty(sphere);
 points[0].x=Bits(0x7f800000u);results[4]=!Fit(points,3,epsilon,sphere)&&Empty(sphere);points[0].x=1;
 results[5]=!Fit(points,100001,epsilon,sphere)&&Empty(sphere);
 results[6]=!Fit(points,3,epsilon,sphere,0)&&Empty(sphere);
 results[7]=!Fit(points,3,epsilon,sphere,1)&&Empty(sphere);
 Vec4 single={123,-456,789,0};results[8]=Fit(&single,1,epsilon,sphere)&&sphere[0]==single.x&&sphere[1]==single.y&&sphere[2]==single.z&&sphere[3]==epsilon;
 results[9]=Fit(points,3,epsilon,sphere)&&Finite(sphere[0])&&Finite(sphere[1])&&Finite(sphere[2])&&Finite(sphere[3])&&sphere[3]>0;
 results[10]=!Fit(points,3,epsilon,0);
 char path[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_SPHERE_GUARDS_OUTPUT",path,MAX_PATH))ExitProcess(1);HANDLE file=CreateFileA(path,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);DWORD done=0;if(file==INVALID_HANDLE_VALUE||!WriteFile(file,results,sizeof(results),&done,0)||done!=sizeof(results))ExitProcess(2);CloseHandle(file);
 for(unsigned int i=0;i<11;i++)if(!results[i])ExitProcess(3);ExitProcess(0);
}
