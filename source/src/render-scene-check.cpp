#include "WorldPacker.cpp"
#include "RenderBlock.h"
#include "RenderAttributes.h"
#include "RenderSector.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
static char path[MAX_PATH];
static unsigned char* readEnv(const char* name,unsigned int* size){
 if(!GetEnvironmentVariableA(name,path,MAX_PATH))ExitProcess(1);
 HANDLE file=CreateFileA(path,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);if(file==INVALID_HANDLE_VALUE)ExitProcess(2);
 *size=GetFileSize(file,0);if(!*size||*size>16*1024*1024)ExitProcess(3);
 unsigned char* data=(unsigned char*)VirtualAlloc(0,*size,MEM_COMMIT|MEM_RESERVE,4);DWORD done;
 if(!data||!ReadFile(file,data,*size,&done,0)||done!=*size)ExitProcess(4);CloseHandle(file);return data;
}
extern "C" void HarnessEntry(){
 if(!GetEnvironmentVariableA("FEAR_READER_INPUT",path,MAX_PATH))ExitProcess(1);PrefabScene::Source source;if(!source.Load(path))ExitProcess(2);
 if(!GetEnvironmentVariableA("FEAR_RENDER_SOURCE",path,MAX_PATH))ExitProcess(1);HANDLE file=CreateFileA(path,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(file==INVALID_HANDLE_VALUE)ExitProcess(3);DWORD done;
 char mode[4]={};bool sectorProbe=GetEnvironmentVariableA("FEAR_RENDER_SECTOR_PROBE",mode,4)&&mode[0]=='1';
 unsigned int records=0;for(unsigned int i=0;i<source.scene.nodeCount;i++){const WorldText::SceneNode& n=source.scene.nodes[i];if(n.kind!=1||(sectorProbe&&!FearBrushFlags::Equal(n.type,"Normal")))continue;const WorldText::Geometry& g=source.scene.geometry[n.geometry];for(unsigned int p=0;p<g.polygons;p++)if(!sectorProbe||RenderSector::Assigned(source.scene,g,g.faces[p],0.01f))records++;}WriteFile(file,&records,4,&done,0);
 for(unsigned int i=0;i<source.scene.nodeCount;i++){
  const WorldText::SceneNode& node=source.scene.nodes[i];if(node.kind!=1||(sectorProbe&&!FearBrushFlags::Equal(node.type,"Normal")))continue;const WorldText::Geometry& g=source.scene.geometry[node.geometry];for(unsigned int face=0;face<g.polygons;face++){if(sectorProbe&&!RenderSector::Assigned(source.scene,g,g.faces[face],0.01f))continue;WorldText::Geometry single=g;single.polygons=1;single.faces=&g.faces[face];single.references=g.faces[face].count;RenderAttributes::Mesh mesh;
  if(!mesh.Build(single,0xffffffff))ExitProcess(4);unsigned int values[3]={node.id,mesh.count,mesh.triangles};WriteFile(file,values,12,&done,0);WriteFile(file,mesh.vertices,mesh.count*84,&done,0);WriteFile(file,mesh.indices,mesh.triangles*6,&done,0);}
 }
 CloseHandle(file);ExitProcess(0);
}
