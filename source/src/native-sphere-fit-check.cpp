#include "WorldPacker.cpp"
#include "SourcePhysicsSphereFit.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
static SourcePhysicsSphereFit::Vec4 points[128];
static bool Write(HANDLE f,const void* p,unsigned int n){DWORD done=0;return !n||(WriteFile(f,p,n,&done,0)&&done==n);}
extern "C" void HarnessEntry(){
 char input[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_SPHERE_FIT_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_SPHERE_FIT_OUTPUT",output,MAX_PATH))ExitProcess(1);
 HANDLE file=CreateFileA(input,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);if(file==INVALID_HANDLE_VALUE)ExitProcess(4);unsigned int size=GetFileSize(file,0);if(size<4||size>1024u*1024u)ExitProcess(4);PrefabScene::Arena memory;unsigned char* data=(unsigned char*)memory.Allocate(size);DWORD done=0;if(!data||!ReadFile(file,data,size,&done,0)||done!=size)ExitProcess(4);CloseHandle(file);
 unsigned int count=*(unsigned int*)data,at=4;if(!count||count>5000)ExitProcess(5);file=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(file==INVALID_HANDLE_VALUE||!Write(file,&count,4))ExitProcess(6);
 for(unsigned int c=0;c<count;c++){if(size-at<8)ExitProcess(5);unsigned int n=*(unsigned int*)(data+at);float epsilon=*(float*)(data+at+4);at+=8;if(n>128||!WorldText::Finite(epsilon)||epsilon<0||n>(size-at)/12)ExitProcess(5);
  for(unsigned int i=0;i<n;i++){memcpy(&points[i],data+at,12);at+=12;points[i].w=0;}
  float sphere[4];if(!SourcePhysicsSphereFit::Fit(points,n,epsilon,sphere))ExitProcess(7);if(!Write(file,&n,4)||!Write(file,sphere,16)||!Write(file,points,n*16))ExitProcess(6);
 }
 CloseHandle(file);if(at!=size)ExitProcess(5);ExitProcess(0);
}
