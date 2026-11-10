#define FEAR_PRIMARY_SURFACE_PREPARATION 1
#include "WorldPacker.cpp"
#include "NativePreparedRenderScene.h"
#include "SourcePhysicsOwnerVertices.h"
#include "WorldModelSharing.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
static bool Write(HANDLE f,const void* p,unsigned int n){DWORD done=0;return !n||(WriteFile(f,p,n,&done,0)&&done==n);}
static bool U(HANDLE f,unsigned int n){return Write(f,&n,4);}
extern "C" void HarnessEntry(){char source[MAX_PATH],project[MAX_PATH],input[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_READER_INPUT",source,MAX_PATH)||!GetEnvironmentVariableA("FEAR_READER_PROJECT",project,MAX_PATH)||!GetEnvironmentVariableA("FEAR_HULL_COLLECTOR_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_HULL_COLLECTOR_OUTPUT",output,MAX_PATH))ExitProcess(1);
 PrefabScene::Arena memory;void* place=memory.Allocate(sizeof(NativePreparedRenderScene::Context));if(!place)ExitProcess(2);NativePreparedRenderScene::Context* scene=new(place)NativePreparedRenderScene::Context;if(!scene->Build(source,project))ExitProcess(2);const PrefabScene::Model& model=scene->Model();const WorldModels::Partition& partition=scene->Partition();WorldModelSharing::Identity sharing;if(!sharing.Build(model,partition))ExitProcess(3);
 unsigned int count=0;for(unsigned int g=1;g<partition.groupCount;g++)if(SourcePhysicalBspContours::RetainedGroup(model,partition,g)&&sharing.representative[g]==g)count+=2;
 HANDLE inp=CreateFileA(input,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0),out=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(inp==INVALID_HANDLE_VALUE||out==INVALID_HANDLE_VALUE||!U(inp,count)||!U(out,count))ExitProcess(4);
 SourcePhysicsOwnerVertices::Builder vertices;
 for(unsigned int g=1;g<partition.groupCount;g++){
  if(!SourcePhysicalBspContours::RetainedGroup(model,partition,g)||sharing.representative[g]!=g)continue;
  for(unsigned int generate=0;generate<2;generate++){SourcePhysicsOwnerVertices::Settings settings;settings.generateIndices=generate!=0;if(!vertices.Build(model,partition,g,scene->Offset(),settings))ExitProcess(5);unsigned int polygons=0;
   for(unsigned int p=0;p<vertices.polygonCount;p++)if(!(vertices.polygons[p].flags&0x180))polygons++;
   if(!U(inp,polygons)||!U(inp,0)||!U(inp,generate)||!Write(inp,&vertices.cachedCenter,12))ExitProcess(4);
   for(unsigned int p=0;p<vertices.polygonCount;p++){const SourcePhysicsOwnerVertices::Polygon& poly=vertices.polygons[p];if(poly.flags&0x180)continue;if(!U(inp,poly.flags)||!U(inp,poly.count)||!Write(inp,poly.points,poly.count*12))ExitProcess(4);}
   if(!U(out,vertices.pointCount)||!U(out,vertices.indexCount)||!Write(out,vertices.scaledVertices,vertices.pointCount*12)||!Write(out,vertices.indices,vertices.indexCount*4))ExitProcess(4);
  }
 }
 CloseHandle(inp);CloseHandle(out);scene->Clear();if(vertices.error||!vertices.polygons)ExitProcess(6);scene->~Context();ExitProcess(0);
}
