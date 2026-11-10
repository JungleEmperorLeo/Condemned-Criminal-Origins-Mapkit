#include "WorldPacker.cpp"
#include "NativePhysicsAutomaticHull.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
static bool Read(HANDLE f,void* p,unsigned int n){DWORD done=0;return !n||(ReadFile(f,p,n,&done,0)&&done==n);}
static bool Write(HANDLE f,const void* p,unsigned int n){DWORD done=0;return !n||(WriteFile(f,p,n,&done,0)&&done==n);}
extern "C" void HarnessEntry(){
 char input[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_AUTOMATIC_HULL_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_AUTOMATIC_HULL_OUTPUT",output,MAX_PATH))ExitProcess(1);
 HANDLE source=CreateFileA(input,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);unsigned int count=0;
 if(source==INVALID_HANDLE_VALUE||GetFileSize(source,0)>32u*1024u*1024u||!Read(source,&count,4)||!count||count>2000)ExitProcess(2);
 HANDLE target=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(target==INVALID_HANDLE_VALUE||!Write(target,&count,4))ExitProcess(3);
 NativePhysicsAutomaticHull::Builder result;
 for(unsigned int c=0;c<count;++c){
  PrefabScene::Arena memory;unsigned int h[7];if(!Read(source,h,28)||h[0]>100000||h[1]>1)ExitProcess(4);
  SourcePhysicsOwnerVertices::Polygon* polygons=(SourcePhysicsOwnerVertices::Polygon*)memory.Allocate((h[0]+1)*sizeof(SourcePhysicsOwnerVertices::Polygon));if(!polygons)ExitProcess(5);
  unsigned int refs=0;
  for(unsigned int p=0;p<h[0];++p){unsigned int fields[2];if(!Read(source,fields,8)||fields[1]>65535||fields[1]>1000000-refs)ExitProcess(4);refs+=fields[1];WorldText::Vec3* points=(WorldText::Vec3*)memory.Allocate((fields[1]+1)*12);if(!points||!Read(source,points,fields[1]*12))ExitProcess(5);memset(polygons+p,0,sizeof(*polygons));polygons[p].points=points;polygons[p].count=fields[1];polygons[p].flags=fields[0];}
  WorldText::Vec3 center;memcpy(&center,h+4,12);float mass,density;memcpy(&mass,h+2,4);memcpy(&density,h+3,4);
  bool built=result.BuildPrepared(polygons,h[0],center,false,mass,density,h[1]!=0);
  unsigned int fields[6]={built?1u:0u,result.size,static_cast<unsigned int>(result.errorCode),static_cast<unsigned int>(result.convexStatus),result.pointCount,result.fallbackGraphBuilds};
  if(!built&&(result.data||result.size||result.pointCount||result.vertexCount||result.triangleCount||result.planeCount||result.fallbackGraphBuilds||!result.error))ExitProcess(6);
  if(!Write(target,fields,24)||!Write(target,result.data,result.size))ExitProcess(3);
 }
 unsigned char extra;DWORD done=0;if(!ReadFile(source,&extra,1,&done,0)||done)ExitProcess(4);CloseHandle(source);CloseHandle(target);ExitProcess(0);
}
