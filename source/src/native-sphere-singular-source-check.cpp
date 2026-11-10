#include "WorldPacker.cpp"
#include "SourcePhysicsSphereFit.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
static bool Write(HANDLE f,const void* p,unsigned int n){DWORD done=0;return !n||(WriteFile(f,p,n,&done,0)&&done==n);}
extern "C" void HarnessEntry(){
 char input[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_SPHERE_FIT_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_SPHERE_FIT_OUTPUT",output,MAX_PATH))ExitProcess(1);
 HANDLE file=CreateFileA(input,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);if(file==INVALID_HANDLE_VALUE)ExitProcess(2);unsigned int size=GetFileSize(file,0);if(size<4||size>16u*1024u*1024u)ExitProcess(2);PrefabScene::Arena packetMemory;unsigned char* bytes=(unsigned char*)packetMemory.Allocate(size);DWORD done=0;if(!bytes||!ReadFile(file,bytes,size,&done,0)||done!=size)ExitProcess(2);CloseHandle(file);
 unsigned int count=*(unsigned int*)bytes,at=4;if(!count||count>1000)ExitProcess(3);file=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(file==INVALID_HANDLE_VALUE||!Write(file,&count,4))ExitProcess(4);
 for(unsigned int c=0;c<count;c++){
  if(size-at<8)ExitProcess(3);unsigned int n=*(unsigned int*)(bytes+at);float epsilon=*(float*)(bytes+at+4);at+=8;if(!n||n>100000||n>(size-at)/12||!WorldText::Finite(epsilon)||epsilon<0)ExitProcess(3);
  PrefabScene::Arena memory;SourcePhysicsSphereFit::Vec4* points=(SourcePhysicsSphereFit::Vec4*)memory.Allocate(n*sizeof(SourcePhysicsSphereFit::Vec4));SourcePhysicsSphereFit::Vec4* rawPoints=(SourcePhysicsSphereFit::Vec4*)memory.Allocate(n*sizeof(SourcePhysicsSphereFit::Vec4));if(!points||!rawPoints)ExitProcess(5);
  for(unsigned int i=0;i<n;i++){memcpy(points+i,bytes+at,12);at+=12;points[i].w=0;rawPoints[i]=points[i];}
  float checked[4],raw[4];unsigned int results[4]={};results[0]=SourcePhysicsSphereFit::Fit(points,n,epsilon,checked)?1u:0u;
  SourcePhysicsSphereFit::Work observation(50000000ull);results[1]=observation.Recurse(rawPoints,n,0,epsilon,raw)?1u:0u;
  bool finite=true;for(unsigned int i=0;i<4;i++)if(!SourcePhysicsSphereFit::Finite(raw[i]))finite=false;results[2]=finite?1u:0u;results[3]=n;
  if(!Write(file,results,16)||!Write(file,checked,16)||!Write(file,raw,16))ExitProcess(4);
 }
 CloseHandle(file);if(at!=size)ExitProcess(3);ExitProcess(0);
}
