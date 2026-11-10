// Research only. Logs bounded allocator calls before invoking unchanged original
// allocation support, so an Exit22 leaves the precise failed request on disk.
#include "OriginalPhysicsOracle.h"
#include "OriginalHullAttemptTrace.h"
static HANDLE traceFile=INVALID_HANDLE_VALUE;
static HANDLE attemptsFile=INVALID_HANDLE_VALUE;
static unsigned char* originalBase=0;
static unsigned int callLimit=100000;
static bool ObserveAttempts=false;
static bool TextEqual(const char* a,const char* b){while(*a&&*a==*b){++a;++b;}return *a==*b;}
extern "C" void* _ReturnAddress(void);
#pragma intrinsic(_ReturnAddress)
static void* __fastcall TraceAllocate(void* object,void* ignored,unsigned int bytes,unsigned int flags){
 unsigned int fields[5]={bytes,flags,OriginalPhysicsOracle::allocationCalls,OriginalPhysicsOracle::allocationBytes,(unsigned int)_ReturnAddress()-(unsigned int)originalBase};
 if(!OriginalPhysicsOracle::Write(traceFile,fields,20))ExitProcess(8);
 if(callLimit==100000)return OriginalPhysicsOracle::Allocate(object,ignored,bytes,flags);
 using namespace OriginalPhysicsOracle;
 if(!active||bytes>8u*1024u*1024u||bytes>64u*1024u*1024u-allocationBytes||allocationCalls>=callLimit){if(ObserveAttempts){Write(attemptsFile,&OriginalHullAttemptTrace::count,4);Write(attemptsFile,OriginalHullAttemptTrace::attempts,OriginalHullAttemptTrace::count*sizeof(OriginalHullAttemptTrace::Attempt));}ExitProcess(22);}
 allocationBytes+=bytes;allocationCalls++;void* p=active->Allocate(bytes+16);if(!p)ExitProcess(23);return (void*)(((unsigned int)p+15)&~15u);
}
extern "C" void HarnessEntry(){using namespace OriginalPhysicsOracle;
 char input[MAX_PATH],output[MAX_PATH],trace[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_HULL_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_HULL_OUTPUT",output,MAX_PATH)||!GetEnvironmentVariableA("FEAR_HULL_ALLOCATION_TRACE",trace,MAX_PATH))ExitProcess(1);
 char limit[16];if(GetEnvironmentVariableA("FEAR_HULL_RESEARCH_CALL_LIMIT",limit,16)){if(TextEqual(limit,"200000"))callLimit=200000;else if(TextEqual(limit,"1000000"))callLimit=1000000;else ExitProcess(6);}char attemptPath[MAX_PATH];ObserveAttempts=GetEnvironmentVariableA("FEAR_HULL_RESEARCH_ATTEMPTS",attemptPath,MAX_PATH)!=0;
 unsigned char* dll=Load();originalBase=dll;Install(dll);if(ObserveAttempts){OriginalHullAttemptTrace::Install(dll);attemptsFile=CreateFileA(attemptPath,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(attemptsFile==INVALID_HANDLE_VALUE)ExitProcess(5);}const unsigned char signature[]={0x81,0xec,0x48,4,0,0,0x56};for(unsigned int i=0;i<sizeof(signature);i++)if(dll[0x64f70+i]!=signature[i])ExitProcess(3);
 void** table=(void**)allocatorObject[0];table[0]=(void*)TraceAllocate;table[4]=(void*)TraceAllocate;
 HANDLE file=CreateFileA(input,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);if(file==INVALID_HANDLE_VALUE||GetFileSize(file,0)>1024u*1024u)ExitProcess(4);unsigned int cases=0,n=0;if(!Read(file,&cases,4)||cases!=1||!Read(file,&n,4)||n<4||n>4096)ExitProcess(6);
 PrefabScene::Arena memory;Begin(memory);WorldText::Vec3* points=(WorldText::Vec3*)memory.Allocate(n*12);if(!points||!Read(file,points,n*12))ExitProcess(6);for(unsigned int i=0;i<n;i++)if(!WorldText::FiniteVector(points[i]))ExitProcess(6);unsigned char extra;DWORD done=0;if(!ReadFile(file,&extra,1,&done,0)||done)ExitProcess(6);CloseHandle(file);
 traceFile=CreateFileA(trace,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);HANDLE out=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(traceFile==INVALID_HANDLE_VALUE||out==INVALID_HANDLE_VALUE||!Write(out,&cases,4))ExitProcess(5);
 unsigned int descriptor[3]={(unsigned int)points,n,12},geometry[6]={0,0,0x80000000u,0,0,0x80000000u},planes[3]={0,0,0x80000000u};typedef void(__cdecl* Graph)(void*,void*,void*);((Graph)(dll+0x64f70))(descriptor,geometry,planes);
 if(geometry[1]>8192||geometry[4]>32768||planes[1]>32768)ExitProcess(7);unsigned int h[5]={geometry[1],geometry[4],planes[1],allocationCalls,allocationBytes};if(!Write(out,h,20))ExitProcess(8);for(unsigned int v=0;v<geometry[1];v++)if(!Write(out,(void*)(geometry[0]+v*16),12))ExitProcess(8);if(!Write(out,(void*)geometry[3],geometry[4]*12)||!Write(out,(void*)planes[0],planes[1]*16))ExitProcess(8);if(ObserveAttempts){Write(attemptsFile,&OriginalHullAttemptTrace::count,4);Write(attemptsFile,OriginalHullAttemptTrace::attempts,OriginalHullAttemptTrace::count*sizeof(OriginalHullAttemptTrace::Attempt));CloseHandle(attemptsFile);}CloseHandle(traceFile);CloseHandle(out);ExitProcess(0);
}
