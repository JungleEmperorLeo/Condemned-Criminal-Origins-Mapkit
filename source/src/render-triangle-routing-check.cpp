#include "WorldPacker.cpp"
#include "RenderTriangleRouting.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
// Opaque diagnostic payload only, not a reconstructed production triangle ABI.
struct Record {unsigned char bytes[280];};
typedef RenderTriangleRouting::Builder<Record> Router;
static bool Read(HANDLE f,void* p,unsigned int bytes){DWORD done;return ReadFile(f,p,bytes,&done,0)&&done==bytes;}
static bool Put(HANDLE f,const void* p,unsigned int bytes){DWORD done;return WriteFile(f,p,bytes,&done,0)&&done==bytes;}
static bool Guards(){
 struct Small {unsigned int value;};typedef RenderTriangleRouting::Builder<Small> SmallRouter;Small a={17},b={23};SmallRouter::Input in[4]={{&a,1},{0,RenderTriangleRouting::Unassigned},{&b,0},{&b,1}};SmallRouter test;
 if(!test.Build(in,4,3)||test.assigned!=3||test.skipped!=1||test.nodes[0].count!=1||test.nodes[1].count!=2||test.nodes[2].count||test.nodes[1].triangles[0].value!=17||test.nodes[1].triangles[1].value!=23)return false;
 a.value=99;if(test.nodes[1].triangles[0].value!=17)return false;
 in[0].owner=3;if(test.Build(in,4,3)||test.nodes||test.nodeCount||test.assigned||!test.error)return false;
 in[0].owner=1;in[0].triangle=0;if(test.Build(in,4,3)||test.nodes||!test.error)return false;
 return test.Build(0,0,3)&&test.nodeCount==3&&!test.assigned&&!test.skipped&&!test.nodes[0].count;
}
extern "C" void HarnessEntry(){
 char input[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_TRIANGLE_ROUTING_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_TRIANGLE_ROUTING_OUTPUT",output,MAX_PATH))ExitProcess(1);
 if(!Guards())ExitProcess(7);HANDLE f=CreateFileA(input,0x80000000,FILE_SHARE_READ,0,3,FILE_ATTRIBUTE_NORMAL,0);if(f==INVALID_HANDLE_VALUE)ExitProcess(2);
 unsigned int header[4];if(!Read(f,header,16)||header[0]!=0x31525453||header[1]!=280||header[2]>100000||!header[3]||header[3]>65536)ExitProcess(3);
 PrefabScene::Arena memory;Record* records=(Record*)memory.Allocate((header[2]+1)*sizeof(Record));Router::Input* assignments=(Router::Input*)memory.Allocate((header[2]+1)*sizeof(Router::Input));if(!records||!assignments)ExitProcess(4);
 for(unsigned int i=0;i<header[2];i++){if(!Read(f,&assignments[i].owner,4)||!Read(f,&records[i],280))ExitProcess(3);assignments[i].triangle=&records[i];}
 // This input contains staged records only, with no original destinations.
 unsigned char trailing;DWORD done;if(!ReadFile(f,&trailing,1,&done,0)||done)ExitProcess(3);
 CloseHandle(f);Router router;if(!router.Build(assignments,header[2],header[3]))ExitProcess(5);
 f=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(f==INVALID_HANDLE_VALUE)ExitProcess(6);unsigned int out[4]={0x314e5254,router.nodeCount,router.assigned,router.skipped};if(!Put(f,out,16))ExitProcess(6);
 for(unsigned int i=0;i<router.nodeCount;i++){const Router::Node& node=router.nodes[i];if(!Put(f,&node.count,4)||(node.count&&!Put(f,node.triangles,node.count*280)))ExitProcess(6);}CloseHandle(f);ExitProcess(0);
}
