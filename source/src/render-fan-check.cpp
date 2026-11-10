#include "WorldPacker.cpp"
#include "RenderFanTriangulation.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
static bool Read(HANDLE f,void* p,unsigned int bytes){DWORD done;return ReadFile(f,p,bytes,&done,0)&&done==bytes;}
static bool Put(HANDLE f,const void* p,unsigned int bytes){DWORD done;return WriteFile(f,p,bytes,&done,0)&&done==bytes;}
static bool Guards(){
 WorldText::Vec3 quad[4]={{0,0,0},{1,0,0},{1,1,0},{0,1,0}};RenderFanTriangulation::Contour in={quad,4,9};RenderFanTriangulation::Builder test;
 if(!test.Build(&in,1)||test.count!=2||test.triangles[0].edgeFlags!=3||test.triangles[1].edgeFlags!=6||test.triangles[1].owner!=9)return false;
 quad[0].x=5;if(test.triangles[0].points[0].x!=0)return false;
 WorldText::Vec3 thin[3]={{0,0,0},{10000,0,0},{10000,.05f,0}};in.vertices=thin;in.count=3;if(!test.Build(&in,1)||test.count||test.filtered!=1)return false;
 thin[1].x=.01f;thin[2].x=0;thin[2].y=1;if(!test.Build(&in,1)||test.count||test.filtered!=1)return false;
 in.vertices=0;if(test.Build(&in,1)||test.triangles||test.count||test.candidates||!test.error)return false;
 return test.Build(0,0)&&!test.count&&!test.filtered&&!test.error;
}
extern "C" void HarnessEntry(){
 char input[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_RENDER_FAN_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_RENDER_FAN_OUTPUT",output,MAX_PATH))ExitProcess(1);if(!Guards())ExitProcess(7);
 HANDLE f=CreateFileA(input,0x80000000,FILE_SHARE_READ,0,3,FILE_ATTRIBUTE_NORMAL,0);if(f==INVALID_HANDLE_VALUE)ExitProcess(2);unsigned int header[2];if(!Read(f,header,8)||header[0]!=0x31434652||header[1]>100000)ExitProcess(3);
 PrefabScene::Arena memory;RenderFanTriangulation::Contour* contours=(RenderFanTriangulation::Contour*)memory.Allocate((header[1]+1)*sizeof(RenderFanTriangulation::Contour));if(!contours)ExitProcess(4);unsigned int vertices=0;
 for(unsigned int i=0;i<header[1];i++){RenderFanTriangulation::Contour& c=contours[i];if(!Read(f,&c.count,4)||!Read(f,&c.owner,4)||c.count>65535||c.count>1000000-vertices)ExitProcess(3);vertices+=c.count;WorldText::Vec3* points=(WorldText::Vec3*)memory.Allocate((c.count+1)*12);if(!points||!Read(f,points,c.count*12))ExitProcess(4);c.vertices=points;}
 unsigned char trailing;DWORD done;if(!ReadFile(f,&trailing,1,&done,0)||done)ExitProcess(3);CloseHandle(f);RenderFanTriangulation::Builder builder;if(!builder.Build(contours,header[1]))ExitProcess(5);
 f=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(f==INVALID_HANDLE_VALUE)ExitProcess(6);unsigned int result[4]={0x314e4652,builder.count,builder.candidates,builder.filtered};if(!Put(f,result,16)||(builder.count&&!Put(f,builder.triangles,builder.count*sizeof(RenderFanTriangulation::Triangle))))ExitProcess(6);CloseHandle(f);ExitProcess(0);
}
