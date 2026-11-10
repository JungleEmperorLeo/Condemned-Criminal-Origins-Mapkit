#include "OriginalPhysicsOracle.h"
#include "OriginalHullAttemptTrace.h"
extern "C" void HarnessEntry(){using namespace OriginalPhysicsOracle;char input[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_HULL_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_HULL_OUTPUT",output,MAX_PATH))ExitProcess(1);
 unsigned char* dll=Load();Install(dll);char tracePath[MAX_PATH];bool trace=GetEnvironmentVariableA("FEAR_HULL_TRACE",tracePath,MAX_PATH)!=0;if(trace)OriginalHullAttemptTrace::Install(dll);const unsigned char signature[]={0x81,0xec,0x48,4,0,0,0x56};for(unsigned int i=0;i<sizeof(signature);i++)if(dll[0x64f70+i]!=signature[i])ExitProcess(3);
 HANDLE file=CreateFileA(input,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);if(file==INVALID_HANDLE_VALUE||GetFileSize(file,0)>8u*1024u*1024u)ExitProcess(4);unsigned int count;if(!Read(file,&count,4)||!count||count>2000)ExitProcess(4);HANDLE out=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(out==INVALID_HANDLE_VALUE||!Write(out,&count,4))ExitProcess(5);
 HANDLE traceFile=trace?CreateFileA(tracePath,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0):INVALID_HANDLE_VALUE;if(trace&&(traceFile==INVALID_HANDLE_VALUE||!Write(traceFile,&count,4)))ExitProcess(5);
 for(unsigned int c=0;c<count;c++){PrefabScene::Arena memory;Begin(memory);OriginalHullAttemptTrace::count=0;unsigned int n;if(!Read(file,&n,4)||n<4||n>128)ExitProcess(6);WorldText::Vec3* points=(WorldText::Vec3*)memory.Allocate(n*12);if(!points||!Read(file,points,n*12))ExitProcess(6);for(unsigned int i=0;i<n;i++)if(!WorldText::FiniteVector(points[i]))ExitProcess(6);
  unsigned int descriptor[3]={(unsigned int)points,n,12},geometry[6]={0,0,0x80000000u,0,0,0x80000000u},planes[3]={0,0,0x80000000u};typedef void(__cdecl *Graph)(void*,void*,void*);((Graph)(dll+0x64f70))(descriptor,geometry,planes);
  if(geometry[1]>10000||geometry[4]>20000||planes[1]>20000||(!geometry[0]&&geometry[1])||(!geometry[3]&&geometry[4])||(!planes[0]&&planes[1]))ExitProcess(7);unsigned int h[5]={geometry[1],geometry[4],planes[1],allocationCalls,allocationBytes};if(!Write(out,h,20))ExitProcess(8);
  for(unsigned int v=0;v<geometry[1];v++)if(!Write(out,(void*)(geometry[0]+v*16),12))ExitProcess(8);
  if(!Write(out,(void*)geometry[3],geometry[4]*12)||!Write(out,(void*)planes[0],planes[1]*16))ExitProcess(8);
  if(trace&&(!Write(traceFile,&OriginalHullAttemptTrace::count,4)||!Write(traceFile,OriginalHullAttemptTrace::attempts,OriginalHullAttemptTrace::count*sizeof(OriginalHullAttemptTrace::Attempt))))ExitProcess(8);
 }unsigned char extra;DWORD done=0;if(!ReadFile(file,&extra,1,&done,0)||done)ExitProcess(6);CloseHandle(file);CloseHandle(out);if(trace)CloseHandle(traceFile);ExitProcess(0);
}
