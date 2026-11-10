#include "WorldPacker.cpp"
#include "SourcePhysicsOwnerVertices.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
static bool Read(HANDLE f,void* p,unsigned int n){DWORD done=0;return !n||(ReadFile(f,p,n,&done,0)&&done==n);}
static bool Write(HANDLE f,const void* p,unsigned int n){DWORD done=0;return !n||(WriteFile(f,p,n,&done,0)&&done==n);}
extern "C" void HarnessEntry(){
 char input[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_HULL_COLLECTOR_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_HULL_COLLECTOR_OUTPUT",output,MAX_PATH))ExitProcess(1);
 HANDLE source=CreateFileA(input,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);if(source==INVALID_HANDLE_VALUE||GetFileSize(source,0)>32u*1024u*1024u)ExitProcess(2);unsigned int count=0;if(!Read(source,&count,4)||!count||count>1000)ExitProcess(2);
 HANDLE target=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(target==INVALID_HANDLE_VALUE||!Write(target,&count,4))ExitProcess(3);
 for(unsigned int c=0;c<count;c++){
  unsigned int header[6];if(!Read(source,header,24)||header[0]>100000||header[1]>1||header[2]>1)ExitProcess(4);WorldText::Vec3 center;memcpy(&center,header+3,12);PrefabScene::Arena memory;
  SourcePhysicsOwnerVertices::Polygon* polygons=(SourcePhysicsOwnerVertices::Polygon*)memory.Allocate((header[0]+1)*sizeof(SourcePhysicsOwnerVertices::Polygon));if(!polygons)ExitProcess(5);unsigned int refs=0;
  for(unsigned int p=0;p<header[0];p++){
   unsigned int fields[2];if(!Read(source,fields,8)||fields[1]>65535||fields[1]>1000000-refs)ExitProcess(4);refs+=fields[1];WorldText::Vec3* points=fields[1]?(WorldText::Vec3*)memory.Allocate(fields[1]*12):0;if(fields[1]&&(!points||!Read(source,points,fields[1]*12)))ExitProcess(5);
   polygons[p].points=points;polygons[p].count=fields[1];polygons[p].flags=fields[0];polygons[p].assignment=p;polygons[p].node=0xffffffffu;polygons[p].sourcePolygon=p;polygons[p].fragment=0;polygons[p].firstSourceCorner=0xffffffffu;
  }
  SourcePhysicsOwnerVertices::Settings settings;settings.generateIndices=header[2]!=0;SourcePhysicsOwnerVertices::Builder result;if(!result.BuildPrepared(polygons,header[0],center,header[1]!=0,settings)){LogLine(result.error);ExitProcess(6);}
  unsigned int counts[2]={result.pointCount,result.indexCount};if(!Write(target,counts,8)||!Write(target,result.scaledVertices,result.pointCount*12)||!Write(target,result.indices,result.indexCount*4))ExitProcess(3);
 }
 unsigned char extra;DWORD done=0;if(!ReadFile(source,&extra,1,&done,0)||done)ExitProcess(4);CloseHandle(source);CloseHandle(target);ExitProcess(0);
}
