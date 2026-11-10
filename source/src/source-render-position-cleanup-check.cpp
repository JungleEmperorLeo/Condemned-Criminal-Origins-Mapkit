#include "WorldPacker.cpp"
#include "SourceRenderPositionCleanup.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
#ifdef FEAR_VERTEX_ORACLE
extern "C" __declspec(dllimport) HMODULE WINAPI LoadLibraryA(const char*);
#pragma comment(linker,"/alternatename:__imp__LoadLibraryA@4=__imp__LoadLibraryA")
#endif
extern "C" void HarnessEntry(){
 char input[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_VERTEX_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_VERTEX_OUTPUT",output,MAX_PATH))ExitProcess(1);
 HANDLE in=CreateFileA(input,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);DWORD done;unsigned int count=0;
 if(in==INVALID_HANDLE_VALUE||!ReadFile(in,&count,4,&done,0)||done!=4||!count||count>100000)ExitProcess(2);
 PrefabScene::Arena memory;WorldText::Vec3* points=(WorldText::Vec3*)memory.Allocate(count*12);unsigned int* results=(unsigned int*)memory.Allocate(count*16);
 if(!points||!results||!ReadFile(in,points,count*12,&done,0)||done!=count*12)ExitProcess(3);CloseHandle(in);
 SourceRenderPositionCleanup::Pool pool;if(!pool.Initialize(points,count))ExitProcess(4);
#ifdef FEAR_VERTEX_ORACLE
 LoadLibraryA("E:/fearsdk/Dev/Tools/msvcr71.dll");LoadLibraryA("E:/fearsdk/Dev/Tools/msvcp71.dll");LoadLibraryA("E:/fearsdk/Dev/Tools/MFC71.dll");unsigned char* dll=(unsigned char*)LoadLibraryA("E:/fearsdk/Dev/Tools/Packers/WorldPacker.dll");if(!dll)ExitProcess(5);
 const unsigned char sig[7]={0x83,0xec,0x28,0x8b,0x44,0x24,0x30};for(unsigned int i=0;i<7;i++)if(dll[0x53330+i]!=sig[i])ExitProcess(6);
 SourceRenderPositionCleanup::Range bounds={points[0],points[0]};
 for(unsigned int i=0;i<count;i++){WorldText::Vec3 p=points[i];if(p.x<bounds.minimum.x)bounds.minimum.x=p.x;if(p.y<bounds.minimum.y)bounds.minimum.y=p.y;if(p.z<bounds.minimum.z)bounds.minimum.z=p.z;if(p.x>bounds.maximum.x)bounds.maximum.x=p.x;if(p.y>bounds.maximum.y)bounds.maximum.y=p.y;if(p.z>bounds.maximum.z)bounds.maximum.z=p.z;}
 unsigned int node[3]={};
 typedef bool (__thiscall* Find)(void*,const void*,const void*,float,void*);Find find=(Find)(dll+0x53330);
 typedef void (__thiscall* Add)(void*,const void*,const void*,unsigned int);Add add=(Add)(dll+0x52c70);
#endif
 for(unsigned int i=0;i<count;i++){WorldText::Vec3 point=points[i];bool found=false;
#ifdef FEAR_VERTEX_ORACLE
  WorldText::Vec3 closest={};found=find(node,&bounds,&point,0.1f,&closest);if(found)point=closest;add(node,&bounds,&point,i/3);
#else
  if(!pool.Snap(point,i/3,found))ExitProcess(7);
#endif
  results[i*4]=found?1u:0u;memcpy(results+i*4+1,&point,12);
 }
 HANDLE out=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(out==INVALID_HANDLE_VALUE||!WriteFile(out,results,count*16,&done,0)||done!=count*16)ExitProcess(8);CloseHandle(out);ExitProcess(0);
}
