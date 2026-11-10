#include "WorldPacker.cpp"
#include "ExtrusionBoundaryCheck.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
static bool Read(HANDLE f,void* p,unsigned int bytes){DWORD done;return ReadFile(f,p,bytes,&done,0)&&done==bytes;}
extern "C" void HarnessEntry(){
 char input[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_EXTRUSION_CHECK_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_EXTRUSION_CHECK_OUTPUT",output,MAX_PATH))ExitProcess(1);
 HANDLE f=CreateFileA(input,0x80000000,FILE_SHARE_READ,0,3,FILE_ATTRIBUTE_NORMAL,0);if(f==INVALID_HANDLE_VALUE)ExitProcess(2);
 unsigned int magic;if(!Read(f,&magic,4)||magic!=0x32505845)ExitProcess(3);
 PrefabScene::Arena memory;unsigned int* results=(unsigned int*)memory.Allocate(100001*4);if(!results)ExitProcess(4);unsigned int count=0;WorldText::Vec3* points=(WorldText::Vec3*)memory.Allocate(128*12);ExtrusionBoundaryCheck::Plane* planes=(ExtrusionBoundaryCheck::Plane*)memory.Allocate(128*16);if(!points||!planes)ExitProcess(4);
 while(true){unsigned int header[8];DWORD done;if(!ReadFile(f,header,32,&done,0))ExitProcess(3);if(!done)break;if(done!=32||count==100000||header[0]<3||header[0]>128||!header[7]||header[7]>128)ExitProcess(3);
  WorldText::Vec3 normal={};if(!Read(f,points,header[0]*12)||!Read(f,planes,header[7]*16))ExitProcess(3);
  unsigned int* normalBits=(unsigned int*)&normal;for(unsigned int k=0;k<3;k++)normalBits[k]=header[2+k];float tolerance=*(float*)&header[5];bool intersects;
  // Header[6] is the recorded original answer and is deliberately not used.
  if(!ExtrusionBoundaryCheck::Evaluate(points,header[0],normal,planes,header[7],tolerance,intersects))ExitProcess(5);results[++count]=intersects?1:0;
 }
 CloseHandle(f);results[0]=count;f=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(f==INVALID_HANDLE_VALUE)ExitProcess(6);DWORD done;if(!WriteFile(f,results,(count+1)*4,&done,0)||done!=(count+1)*4)ExitProcess(7);CloseHandle(f);ExitProcess(0);
}
