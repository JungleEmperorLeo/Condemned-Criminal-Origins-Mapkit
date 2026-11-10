#include "WorldPacker.cpp"
#include "RenderAttributedFan.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
static bool Read(HANDLE f,void* p,unsigned int bytes){DWORD done;return ReadFile(f,p,bytes,&done,0)&&done==bytes;}
static bool Put(HANDLE f,const void* p,unsigned int bytes){DWORD done;return WriteFile(f,p,bytes,&done,0)&&done==bytes;}
extern "C" void HarnessEntry(){
 char input[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_RENDER_FAN_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_RENDER_FAN_OUTPUT",output,MAX_PATH))ExitProcess(1);
 HANDLE f=CreateFileA(input,0x80000000,FILE_SHARE_READ,0,3,FILE_ATTRIBUTE_NORMAL,0);if(f==INVALID_HANDLE_VALUE)ExitProcess(2);unsigned int header[2];if(!Read(f,header,8)||header[0]!=0x31434652||header[1]>100000)ExitProcess(3);
 PrefabScene::Arena memory;RenderAttributedFan::Contour* contours=(RenderAttributedFan::Contour*)memory.Allocate((header[1]+1)*sizeof(RenderAttributedFan::Contour));if(!contours)ExitProcess(4);unsigned int vertices=0;
 for(unsigned int i=0;i<header[1];i++){RenderAttributedFan::Contour& c=contours[i];if(!Read(f,&c.count,4)||!Read(f,&c.owner,4)||c.count>65535||c.count>1000000-vertices)ExitProcess(3);vertices+=c.count;RenderClipVertex::Vertex* points=(RenderClipVertex::Vertex*)memory.Allocate((c.count+1)*44);if(!points||!Read(f,points,c.count*44))ExitProcess(4);c.vertices=points;}
 unsigned char trailing;DWORD done;if(!ReadFile(f,&trailing,1,&done,0)||done)ExitProcess(3);CloseHandle(f);RenderAttributedFan::Builder builder;if(!builder.Build(contours,header[1]))ExitProcess(5);
 f=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(f==INVALID_HANDLE_VALUE)ExitProcess(6);unsigned int result[4]={0x314e4652,builder.count,builder.candidates,builder.filtered};if(!Put(f,result,16)||(builder.count&&!Put(f,builder.triangles,builder.count*sizeof(RenderAttributedFan::Triangle))))ExitProcess(6);CloseHandle(f);ExitProcess(0);
}
