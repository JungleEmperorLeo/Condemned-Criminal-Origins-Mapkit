#include "WorldPacker.cpp"
#include "ShadowMesh.h"
#include "ShadowJunctions.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
static bool Read(HANDLE f,void* data,unsigned int bytes){DWORD done;return ReadFile(f,data,bytes,&done,0)&&done==bytes;}
static void Save(HANDLE f,const void* data,unsigned int bytes){DWORD done;if(!WriteFile(f,data,bytes,&done,0)||done!=bytes)ExitProcess(7);}
static ShadowJunctions::Polygon* Load(HANDLE f,unsigned int count,PrefabScene::Arena& arena){
 ShadowJunctions::Polygon* polygons=(ShadowJunctions::Polygon*)arena.Allocate(count*sizeof(ShadowJunctions::Polygon));if(!polygons)ExitProcess(4);
 for(unsigned int i=0;i<count;i++){ShadowJunctions::Polygon& p=polygons[i];if(!Read(f,&p.count,4)||p.count<3||p.count>128||!Read(f,&p.normal,12))ExitProcess(3);
  WorldText::Vec3* points=(WorldText::Vec3*)arena.Allocate(p.count*12);if(!points||!Read(f,points,p.count*12))ExitProcess(4);p.points=points;
 }return polygons;
}
extern "C" void HarnessEntry(){
 char input[MAX_PATH],output[MAX_PATH],prepared[MAX_PATH];
 if(!GetEnvironmentVariableA("FEAR_JUNCTION_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_SHADOW_OUTPUT",output,MAX_PATH)||!GetEnvironmentVariableA("FEAR_JUNCTION_TRIANGLES",prepared,MAX_PATH))ExitProcess(1);
 HANDLE f=CreateFileA(input,0x80000000,FILE_SHARE_READ,0,3,FILE_ATTRIBUTE_NORMAL,0);if(f==INVALID_HANDLE_VALUE)ExitProcess(2);
 unsigned int header[3];WorldText::Vec3 offset;
 if(!Read(f,header,12)||header[0]!=0x31504a53||!header[1]||!header[2]||header[1]>6000||header[2]>6000||!Read(f,&offset,12))ExitProcess(3);
 PrefabScene::Arena arena;ShadowJunctions::Polygon* selected=Load(f,header[1],arena);ShadowJunctions::Polygon* all=Load(f,header[2],arena);unsigned char trailing;DWORD done;if(!ReadFile(f,&trailing,1,&done,0)||done)ExitProcess(3);CloseHandle(f);
 ShadowJunctions::Preparation junctions;if(!junctions.Build(selected,header[1],all,header[2]))ExitProcess(5);
 ShadowMesh::Mesh mesh;if(!mesh.Build(junctions.triangles,junctions.count,offset))ExitProcess(5);
 // Both calculations complete before either output is opened.
 f=CreateFileA(prepared,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(f==INVALID_HANDLE_VALUE)ExitProcess(6);
 unsigned int triHeader[2]={0x31574853,junctions.count};Save(f,triHeader,8);Save(f,&offset,12);Save(f,junctions.triangles,junctions.count*sizeof(ShadowMesh::Triangle));CloseHandle(f);
 f=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(f==INVALID_HANDLE_VALUE)ExitProcess(6);unsigned int counts[2]={mesh.count,mesh.triangles};Save(f,counts,8);Save(f,mesh.vertices,mesh.count*24);Save(f,mesh.indices,mesh.triangles*6);CloseHandle(f);ExitProcess(0);
}
