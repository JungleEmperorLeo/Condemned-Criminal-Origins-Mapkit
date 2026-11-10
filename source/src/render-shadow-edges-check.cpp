#include "WorldPacker.cpp"
#include "RenderShadowEdges.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
#ifdef FEAR_SHADOW_EDGE_ORACLE
extern "C" __declspec(dllimport) HMODULE WINAPI LoadLibraryA(const char*);
extern "C" __declspec(dllimport) void* WINAPI GetProcAddress(HMODULE,const char*);
#pragma comment(linker,"/alternatename:__imp__LoadLibraryA@4=__imp__LoadLibraryA")
#pragma comment(linker,"/alternatename:__imp__GetProcAddress@8=__imp__GetProcAddress")
struct OriginalTriangle {RenderAttributes::Vertex corners[3];unsigned char trailer[28];};
static_assert(sizeof(OriginalTriangle)==280,"original triangle stride");
static void Prepare(OriginalTriangle& t,const WorldText::Vec3* points,unsigned char* polygon,unsigned char* surface,unsigned char* plane,const WorldText::Vec3& normal){
 memset(&t,0,sizeof(t));memset(polygon,0,60);memset(surface,0,172);memset(plane,0,24);for(unsigned int i=0;i<3;i++)t.corners[i].position=points[i];
 *(void**)((unsigned char*)&t+0x108)=polygon;*(void**)(polygon+0x38)=surface;*(void**)(surface+0xA4)=plane;memcpy(plane+8,&normal,12);
}
#endif
extern "C" void HarnessEntry(){
 char input[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_SHADOW_EDGE_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_SHADOW_EDGE_OUTPUT",output,MAX_PATH))ExitProcess(1);
 HANDLE in=CreateFileA(input,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);DWORD done;unsigned int size=0;
 if(in==INVALID_HANDLE_VALUE||!ReadFile(in,&size,4,&done,0)||done!=4||!size||size>10000)ExitProcess(2);
 PrefabScene::Arena memory;RenderShadowEdges::Input* records=(RenderShadowEdges::Input*)memory.Allocate(size*112);RenderShadowEdges::Output* results=(RenderShadowEdges::Output*)memory.Allocate(size*sizeof(RenderShadowEdges::Output));
 if(!records||!results||!ReadFile(in,records,size*112,&done,0)||done!=size*112)ExitProcess(3);CloseHandle(in);
#ifdef FEAR_SHADOW_EDGE_ORACLE
 HMODULE system=LoadLibraryA("kernel32.dll");typedef unsigned int (WINAPI *ErrorMode)(unsigned int);ErrorMode setErrorMode=system?(ErrorMode)GetProcAddress(system,"SetErrorMode"):0;if(!setErrorMode)ExitProcess(4);setErrorMode(3);
 LoadLibraryA("E:/fearsdk/Dev/Tools/msvcr71.dll");LoadLibraryA("E:/fearsdk/Dev/Tools/msvcp71.dll");LoadLibraryA("E:/fearsdk/Dev/Tools/MFC71.dll");unsigned char* dll=(unsigned char*)LoadLibraryA("E:/fearsdk/Dev/Tools/Packers/WorldPacker.dll");if(!dll)ExitProcess(4);
 const unsigned char connectedSignature[10]={0x8b,0x44,0x24,0x0c,0x81,0xec,0x1c,0x01,0x00,0x00};const unsigned char boundarySignature[9]={0x81,0xec,0x20,0x01,0x00,0x00,0x53,0x55,0x56};
 for(unsigned int i=0;i<10;i++)if(dll[0x49b50+i]!=connectedSignature[i])ExitProcess(5);for(unsigned int i=0;i<9;i++)if(dll[0x49de0+i]!=boundarySignature[i])ExitProcess(5);
 // Suppress the helper's one-time assertion UI for deliberate missing-match
 // controls. This affects this process's loaded image, never the DLL file.
 dll[0x9d40e]=1;
 typedef void (__stdcall* Connected)(void*,void*,unsigned int,unsigned int,bool,void*,void*);
 typedef void (__stdcall* Boundary)(void*,void*,unsigned int,unsigned int,void*);
 Connected connected=(Connected)(dll+0x49b50);Boundary boundary=(Boundary)(dll+0x49de0);
 for(unsigned int i=0;i<size;i++){
  RenderShadowEdges::Output validated;if(!RenderShadowEdges::Build(records[i],validated))ExitProcess(7);
  OriginalTriangle own,neighbor,working,emitted[2];unsigned char ownPoly[60],neighborPoly[60],ownSurface[172],neighborSurface[172],ownPlane[24],neighborPlane[24];
  Prepare(own,records[i].own,ownPoly,ownSurface,ownPlane,records[i].ownNormal);Prepare(neighbor,records[i].neighbor,neighborPoly,neighborSurface,neighborPlane,records[i].neighborNormal);memset(&working,0,sizeof(working));*(unsigned int*)((unsigned char*)&working+0x110)=7;memset(emitted,0,sizeof(emitted));
  unsigned int vector[4]={0,(unsigned int)emitted,(unsigned int)emitted,(unsigned int)(emitted+2)};
  if(records[i].kind==0)connected(&own,&neighbor,records[i].start,records[i].end,records[i].twoSided!=0,&working,vector);else boundary(&own,&working,records[i].start,records[i].end,vector);
  unsigned int count=(vector[2]-vector[1])/280;if(count>2)ExitProcess(8);memset(&results[i],0,sizeof(results[i]));results[i].count=count;for(unsigned int q=0;q<count;q++)memcpy(results[i].triangles[q].vertices,emitted[q].corners,252);
 }
#else
 for(unsigned int i=0;i<size;i++)if(!RenderShadowEdges::Build(records[i],results[i]))ExitProcess(4);
#endif
 HANDLE out=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);unsigned int header[2]={0x31454753,size};if(out==INVALID_HANDLE_VALUE||!WriteFile(out,header,8,&done,0)||done!=8||!WriteFile(out,results,size*sizeof(RenderShadowEdges::Output),&done,0)||done!=size*sizeof(RenderShadowEdges::Output))ExitProcess(10);CloseHandle(out);ExitProcess(0);
}
