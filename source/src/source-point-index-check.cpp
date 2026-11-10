#include "WorldPacker.cpp"
#include "SourceRenderPointTags.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
#ifdef FEAR_POINT_INDEX_ORACLE
extern "C" __declspec(dllimport) HMODULE WINAPI LoadLibraryA(const char*);
#pragma comment(linker,"/alternatename:__imp__LoadLibraryA@4=__imp__LoadLibraryA")
static unsigned int world[32],polygon[25+64*11];
#endif
static WorldText::Vec3 positions[64];
static bool Read(HANDLE f,void* p,unsigned int n){DWORD done;return ReadFile(f,p,n,&done,0)&&done==n;}
static bool Write(HANDLE f,const void* p,unsigned int n){DWORD done;return WriteFile(f,p,n,&done,0)&&done==n;}
extern "C" void HarnessEntry(){
 char input[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_INDEX_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_INDEX_OUTPUT",output,MAX_PATH))ExitProcess(1);
#ifdef FEAR_POINT_INDEX_ORACLE
 LoadLibraryA("E:\\fearsdk\\Dev\\Tools\\msvcr71.dll");LoadLibraryA("E:\\fearsdk\\Dev\\Tools\\msvcp71.dll");LoadLibraryA("E:\\fearsdk\\Dev\\Tools\\MFC71.dll");
 unsigned char* dll=(unsigned char*)LoadLibraryA("E:\\fearsdk\\Dev\\Tools\\Packers\\WorldPacker.dll");if(!dll)ExitProcess(2);
 if(dll[0x266a0]!=0x6a||dll[0x266a1]!=0xff||dll[0x266a2]!=0x68)ExitProcess(2);
 typedef void (__thiscall* Index)(void*);Index index=(Index)(dll+0x266a0);
#endif
 HANDLE in=CreateFileA(input,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);HANDLE out=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);unsigned int cases=0;
 if(in==INVALID_HANDLE_VALUE||out==INVALID_HANDLE_VALUE||!Read(in,&cases,4)||cases>10000)ExitProcess(3);
 for(unsigned int c=0;c<cases;c++){unsigned int count;if(!Read(in,&count,4)||!count||count>64||!Read(in,positions,count*12))ExitProcess(4);
#ifdef FEAR_POINT_INDEX_ORACLE
  for(unsigned int i=0;i<32;i++)world[i]=0;for(unsigned int i=0;i<25+64*11;i++)polygon[i]=0;
  unsigned int head=(unsigned int)polygon+4;world[0x20/4]=1;world[0x24/4]=head;polygon[0]=polygon[2]=head;*(unsigned short*)((unsigned char*)polygon+0x60)=(unsigned short)count;
  for(unsigned int i=0;i<count;i++){unsigned int* v=polygon+25+i*11;v[1]=((unsigned int*)(positions+i))[0];v[2]=((unsigned int*)(positions+i))[1];v[3]=((unsigned int*)(positions+i))[2];}
  index(world);unsigned int generated=world[0x54/4];if(generated>count||!Write(out,&generated,4))ExitProcess(5);
  for(unsigned int i=0;i<count;i++)if(!Write(out,polygon+25+i*11,4))ExitProcess(5);
  if(!Write(out,(void*)world[0x50/4],generated*12))ExitProcess(5);
#else
  SourceRenderPointTags::Indexer indexer;if(!indexer.Build(positions,count)||!Write(out,&indexer.pointCount,4)||!Write(out,indexer.indices,count*4)||!Write(out,indexer.points,indexer.pointCount*12))ExitProcess(5);
#endif
 }CloseHandle(in);CloseHandle(out);ExitProcess(0);
}

