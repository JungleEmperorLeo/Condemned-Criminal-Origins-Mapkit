#include "WorldPacker.cpp"
#include "RenderVertexWeld.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
#ifdef FEAR_VERTEX_ORACLE
extern "C" __declspec(dllimport) HMODULE WINAPI LoadLibraryA(const char*);
#pragma comment(linker,"/alternatename:__imp__LoadLibraryA@4=__imp__LoadLibraryA")
#endif
extern "C" void HarnessEntry(){
 char input[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_VERTEX_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_VERTEX_OUTPUT",output,MAX_PATH))ExitProcess(1);
 HANDLE in=CreateFileA(input,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);DWORD done;unsigned int size=0;int origin[3];
 if(in==INVALID_HANDLE_VALUE||!ReadFile(in,&size,4,&done,0)||done!=4||!size||!ReadFile(in,origin,12,&done,0)||done!=12)ExitProcess(2);
#ifdef FEAR_VERTEX_ORACLE
 if(size>100000)ExitProcess(2);
#else
 if(size>RenderIndexedLimits::MaximumCorners)ExitProcess(2);
#endif
 PrefabScene::Arena memory;RenderVertexWeld::Input* corners=(RenderVertexWeld::Input*)memory.Allocate(size*88);if(!corners||!ReadFile(in,corners,size*88,&done,0)||done!=size*88)ExitProcess(3);CloseHandle(in);
 RenderAttributes::Vertex* vertices=0;unsigned int* indices=0;unsigned int count=0,resets=0;
#ifdef FEAR_VERTEX_ORACLE
 LoadLibraryA("E:/fearsdk/Dev/Tools/msvcr71.dll");LoadLibraryA("E:/fearsdk/Dev/Tools/msvcp71.dll");LoadLibraryA("E:/fearsdk/Dev/Tools/MFC71.dll");unsigned char* dll=(unsigned char*)LoadLibraryA("E:/fearsdk/Dev/Tools/Packers/WorldPacker.dll");if(!dll)ExitProcess(4);
 const unsigned char signature[8]={0x8b,0x44,0x24,0x08,0x53,0x55,0x56,0x57};for(unsigned int i=0;i<8;i++)if(dll[0x4b480+i]!=signature[i])ExitProcess(5);
 // The vector layouts are observed from 0x4B480/0x488D0. Preallocation keeps
 // the oracle away from STL allocation, constructors and destructors.
 unsigned char* hash=(unsigned char*)memory.Allocate(0x100c);unsigned int* bucketStorage=(unsigned int*)memory.Allocate(size*256*4);
 vertices=(RenderAttributes::Vertex*)memory.Allocate(size*84);indices=(unsigned int*)memory.Allocate(size*4);if(!hash||!bucketStorage||!vertices||!indices)ExitProcess(6);memset(hash,0,0x100c);memcpy(hash+0x1000,origin,12);
 unsigned int vector[4]={0,(unsigned int)vertices,(unsigned int)vertices,(unsigned int)(vertices+size)};
 typedef unsigned int (__cdecl* Insert)(void*,const void*,void*);Insert insert=(Insert)(dll+0x4b480);
 for(unsigned int i=0;i<size;i++){
  if(corners[i].reset>1||!RenderVertexWeld::Valid(corners[i].vertex)||(!i&&!corners[i].reset))ExitProcess(7);
  if(corners[i].reset){for(unsigned int b=0;b<256;b++){unsigned int* slot=(unsigned int*)(hash+b*16);slot[0]=0;slot[1]=slot[2]=(unsigned int)(bucketStorage+b*size);slot[3]=(unsigned int)(bucketStorage+(b+1)*size);}resets++;}
  indices[i]=insert(hash,&corners[i].vertex,vector);if(indices[i]>=size)ExitProcess(8);
 }
 count=(vector[2]-vector[1])/84;if(count>size)ExitProcess(9);
#else
 RenderVertexWeld::Mesh mesh;if(!mesh.Build(corners,size,origin))ExitProcess(4);vertices=mesh.vertices;indices=mesh.indices;count=mesh.count;resets=mesh.resets;
#endif
 HANDLE out=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);unsigned int header[4]={0x31444c57,size,count,resets};if(out==INVALID_HANDLE_VALUE||!WriteFile(out,header,16,&done,0)||done!=16||!WriteFile(out,indices,size*4,&done,0)||done!=size*4||!WriteFile(out,vertices,count*84,&done,0)||done!=count*84)ExitProcess(10);CloseHandle(out);ExitProcess(0);
}
