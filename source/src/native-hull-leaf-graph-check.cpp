#if defined(HULL_LEAF_ORIGINAL)
#include "OriginalPhysicsOracle.h"
#else
#include "WorldPacker.cpp"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
#endif
#include "NativeHullLeafGraph.h"
static bool LeafRead(HANDLE f,void* p,unsigned int n){DWORD done=0;return !n||(ReadFile(f,p,n,&done,0)&&done==n);}
static bool LeafWrite(HANDLE f,const void* p,unsigned int n){DWORD done=0;return !n||(WriteFile(f,p,n,&done,0)&&done==n);}
extern "C" void HarnessEntry(){
 char input[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_HULL_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_HULL_OUTPUT",output,MAX_PATH))ExitProcess(1);
#if defined(HULL_LEAF_ORIGINAL)
 unsigned char* dll=OriginalPhysicsOracle::Load();
 if(dll[0x6a770]!=0x83||dll[0x6a771]!=0xec||dll[0x6a772]!=0x08||dll[0x6a7d0]!=0x83||dll[0x6a7d2]!=0x10||dll[0x6a880]!=0x55)ExitProcess(3);
#endif
 HANDLE file=CreateFileA(input,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);if(file==INVALID_HANDLE_VALUE||GetFileSize(file,0)>8u*1024u*1024u)ExitProcess(4);
 unsigned int cases;if(!LeafRead(file,&cases,4)||!cases||cases>10000)ExitProcess(4);HANDLE out=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(out==INVALID_HANDLE_VALUE||!LeafWrite(out,&cases,4))ExitProcess(5);
 for(unsigned int c=0;c<cases;c++){
  unsigned int n=0,activeCount=0,active[3]={};NativeHullLeafGraph::Vec4 points[16];if(!LeafRead(file,&n,4)||n>16||!LeafRead(file,points,n*16)||!LeafRead(file,&activeCount,4)||activeCount>3||!LeafRead(file,active,activeCount*4))ExitProcess(6);
  NativeHullLeafGraph::Leaf leaf;
#if defined(HULL_LEAF_ORIGINAL)
  unsigned int graph[0x110]={};NativeHullLeafGraph::Edge original[6]={};graph[0]=(unsigned int)points;graph[1]=(unsigned int)original;graph[3]=0x80000006u;
  for(unsigned int i=0;i<activeCount;i++)if(active[i]>=n||!NativeHullLeafGraph::Finite(points[active[i]].x)||!NativeHullLeafGraph::Finite(points[active[i]].y)||!NativeHullLeafGraph::Finite(points[active[i]].z)||points[active[i]].w!=0)ExitProcess(6);
  if(activeCount==1){typedef void(__thiscall* One)(void*,unsigned int);((One)(dll+0x6a770))(graph,active[0]);}
  else if(activeCount==2){typedef void(__thiscall* Two)(void*,unsigned int,unsigned int);((Two)(dll+0x6a7d0))(graph,active[0],active[1]);}
  else if(activeCount==3){typedef void(__thiscall* Three)(void*,unsigned int,unsigned int,unsigned int);((Three)(dll+0x6a880))(graph,active[0],active[1],active[2]);}
  if(graph[1]!=(unsigned int)original||graph[2]>6||graph[3]!=0x80000006u)ExitProcess(7);leaf.count=graph[2];for(unsigned int i=0;i<leaf.count;i++)leaf.edges[i]=original[i];
#else
  if(NativeHullLeafGraph::Build(points,n,active,activeCount,&leaf)!=NativeHullLeafGraph::Success)ExitProcess(7);
#endif
  if(!LeafWrite(out,&leaf.count,4))ExitProcess(8);for(unsigned int i=0;i<leaf.count;i++)if(!LeafWrite(out,&leaf.edges[i],6))ExitProcess(8);
 }
 unsigned char extra;DWORD done=0;if(!ReadFile(file,&extra,1,&done,0)||done)ExitProcess(6);CloseHandle(file);CloseHandle(out);ExitProcess(0);
}
