#include "WorldPacker.cpp"
#include "RenderBlock.h"
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
 const WorldText::Geometry* g=0;for(unsigned int i=0;i<source.scene.nodeCount;i++){const WorldText::SceneNode& n=source.scene.nodes[i];if(n.kind==1&&n.knownType&&(n.surfaceFlags&8)){g=&source.scene.geometry[n.geometry];break;}}
 if(!g||!GetEnvironmentVariableA("FEAR_RENDER_SOURCE",path,MAX_PATH))ExitProcess(3);
 HANDLE file=CreateFileA(path,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(file==INVALID_HANDLE_VALUE)ExitProcess(4);DWORD done;
 WriteFile(file,&g->polygons,4,&done,0);
 for(unsigned int i=0;i<g->polygons;i++){
  const WorldText::Polygon& p=g->faces[i];WriteFile(file,&p.count,4,&done,0);WriteFile(file,&p.normal,12,&done,0);WriteFile(file,&p.mappingCount,4,&done,0);
  for(unsigned int j=0;j<p.mappingCount;j++){const WorldText::Mapping& m=p.mappings[j];if(!m.valid)ExitProcess(5);WriteFile(file,&m.channel,4,&done,0);WriteFile(file,&m.origin,12,&done,0);WriteFile(file,&m.p,12,&done,0);WriteFile(file,&m.q,12,&done,0);}
  for(unsigned int j=0;j<p.count;j++)WriteFile(file,&g->vertices[p.indices[j]],12,&done,0);
 }
 CloseHandle(file);ExitProcess(0);
}
