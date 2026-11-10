#include "WorldPacker.cpp"
#include "StreamingTree.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
static unsigned char originalOut[4096];static unsigned int written;static bool overflow;
static void __fastcall WriteValue(void*,void*,unsigned int value){if(written+4>sizeof(originalOut)){overflow=true;return;}*(unsigned int*)(originalOut+written)=value;written+=4;}
struct Convex {unsigned int dummy[4],id;};
struct Reference {unsigned int dummy[2];Convex* convex;};
struct OriginalNode {void* vtable;unsigned int axis;float split;unsigned int id;OriginalNode* a;OriginalNode* b;Reference** refs;unsigned int count,capacity,padding;};
static char path[MAX_PATH];
extern "C" void HarnessEntry(){
 if(!GetEnvironmentVariableA("FEAR_STREAM_TREE_CODE",path,MAX_PATH))ExitProcess(1);
 HANDLE f=CreateFileA(path,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);if(f==INVALID_HANDLE_VALUE)ExitProcess(2);
 unsigned int bytes=GetFileSize(f,0);if(bytes!=288)ExitProcess(3);unsigned char* code=(unsigned char*)VirtualAlloc(0,bytes,MEM_COMMIT|MEM_RESERVE,0x40);DWORD done;
 if(!code||!ReadFile(f,code,bytes,&done,0)||done!=bytes)ExitProcess(4);CloseHandle(f);
 if(code[0]!=0x56||code[0x70]!=0x53)ExitProcess(5);
 void* methods[24]={};methods[14]=(void*)&WriteValue;methods[18]=(void*)&WriteValue;void** stream=methods;
 typedef void (__thiscall *Writer)(OriginalNode*,void*);Writer writer=(Writer)(code+0x70);
 unsigned int ids[]={2,0};Convex convex[2]={};convex[0].id=2;convex[1].id=0;Reference references[2]={};Reference* pointers[2];
 for(unsigned int i=0;i<2;i++){references[i].convex=&convex[i];pointers[i]=&references[i];}
 StreamingTree::Block block;
 for(unsigned int axis=0;axis<3;axis++){
  // Deliberately use a non-preorder input array to verify child index remapping.
  StreamingTree::Node nodes[3]={{3,0,-1,-1,ids,1},{axis,12.5f,2,0,ids,2},{3,0,-1,-1,ids+1,1}};
  OriginalNode runtime[3]={};runtime[0].axis=3;runtime[1].axis=axis;runtime[1].split=12.5f;runtime[2].axis=3;
  runtime[1].a=&runtime[2];runtime[1].b=&runtime[0];runtime[1].id=0;runtime[2].id=1;runtime[0].id=2;
  runtime[1].refs=pointers;runtime[1].count=2;runtime[2].refs=pointers+1;runtime[2].count=1;runtime[0].refs=pointers;runtime[0].count=1;
  written=0;overflow=false;writer(&runtime[1],&stream);if(overflow||!block.Build(nodes,3,1,3)||block.size!=written)ExitProcess(20+axis);
  for(unsigned int j=0;j<written;j++)if(block.data[j]!=originalOut[j])ExitProcess(30+axis);
 }
 StreamingTree::Node leaf={3,0,-1,-1,0,0};OriginalNode single={};single.axis=3;single.id=0;written=0;writer(&single,&stream);if(!block.Build(&leaf,1,0,0)||block.size!=written)ExitProcess(40);for(unsigned int j=0;j<written;j++)if(block.data[j]!=originalOut[j])ExitProcess(41);
 leaf.axis=4;if(block.Build(&leaf,1,0,0))ExitProcess(50);leaf.axis=3;leaf.childA=0;if(block.Build(&leaf,1,0,0))ExitProcess(51);leaf.axis=0;if(block.Build(&leaf,1,0,0))ExitProcess(52);leaf.childA=-1;if(block.Build(&leaf,1,0,0))ExitProcess(53);leaf.axis=3;leaf.convexIds=ids;leaf.convexCount=1;if(block.Build(&leaf,1,0,2))ExitProcess(54);
 const unsigned char* result=originalOut;
 if(GetEnvironmentVariableA("FEAR_STREAM_TREE_INPUT",path,MAX_PATH)){
  f=CreateFileA(path,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);if(f==INVALID_HANDLE_VALUE)ExitProcess(70);
  unsigned int size=GetFileSize(f,0);if(size<12||size>16*1024*1024)ExitProcess(71);unsigned char* input=(unsigned char*)VirtualAlloc(0,size,MEM_COMMIT|MEM_RESERVE,4);
  if(!input||!ReadFile(f,input,size,&done,0)||done!=size)ExitProcess(72);CloseHandle(f);
  unsigned int count=*(unsigned int*)input,limit=*(unsigned int*)(input+4),root=*(unsigned int*)(input+8),cursor=12;if(!count||count>100000)ExitProcess(73);
  StreamingTree::Node* nodes=(StreamingTree::Node*)VirtualAlloc(0,count*sizeof(StreamingTree::Node),MEM_COMMIT|MEM_RESERVE,4);if(!nodes)ExitProcess(74);
  for(unsigned int i=0;i<count;i++){
   if(size-cursor<20)ExitProcess(75);nodes[i].axis=*(unsigned int*)(input+cursor);nodes[i].split=*(float*)(input+cursor+4);nodes[i].childA=*(int*)(input+cursor+8);nodes[i].childB=*(int*)(input+cursor+12);nodes[i].convexCount=*(unsigned int*)(input+cursor+16);cursor+=20;
   if(nodes[i].convexCount>(size-cursor)/4)ExitProcess(76);nodes[i].convexIds=(unsigned int*)(input+cursor);cursor+=nodes[i].convexCount*4;
  }
  if(cursor!=size||!block.Build(nodes,count,root,limit))ExitProcess(77);result=block.data;written=block.size;
 }
 if(!GetEnvironmentVariableA("FEAR_STREAM_TREE_OUTPUT",path,MAX_PATH))ExitProcess(1);f=CreateFileA(path,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(f==INVALID_HANDLE_VALUE||!WriteFile(f,result,written,&done,0)||done!=written)ExitProcess(60);CloseHandle(f);ExitProcess(0);
}
