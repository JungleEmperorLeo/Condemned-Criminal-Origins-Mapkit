#include "WorldPacker.cpp"
#include "FearWorldTreeIntersection.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
#ifdef FEAR_WORLD_TREE_ORACLE
extern "C" __declspec(dllimport) HMODULE WINAPI LoadLibraryA(const char*);
extern "C" __declspec(dllimport) void* WINAPI GetProcAddress(HMODULE,const char*);
#pragma comment(linker,"/alternatename:__imp__LoadLibraryA@4=__imp__LoadLibraryA")
#pragma comment(linker,"/alternatename:__imp__GetProcAddress@8=__imp__GetProcAddress")
static bool OriginalIntersects(void* target,void* polygon,void* box){unsigned int result;__asm {
 mov ecx,polygon
 push box
 mov edx,target
 call edx
 add esp,4
 movzx eax,al
 mov result,eax
}return result!=0;}
#endif
extern "C" void HarnessEntry(){
 char path[MAX_PATH],out[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_WORLD_TREE_INPUT",path,MAX_PATH)||!GetEnvironmentVariableA("FEAR_WORLD_TREE_OUTPUT",out,MAX_PATH))ExitProcess(1);
#ifdef FEAR_WORLD_TREE_ORACLE
 HMODULE kernel=LoadLibraryA("kernel32.dll");typedef DWORD (WINAPI *Mode)(DWORD);Mode mode=kernel?(Mode)GetProcAddress(kernel,"SetErrorMode"):0;if(mode)mode(3);
 LoadLibraryA("E:/fearsdk/Dev/Tools/msvcr71.dll");LoadLibraryA("E:/fearsdk/Dev/Tools/msvcp71.dll");LoadLibraryA("E:/fearsdk/Dev/Tools/MFC71.dll");unsigned char* dll=(unsigned char*)LoadLibraryA("E:/fearsdk/Dev/Tools/Packers/WorldPacker.dll");if(!dll)ExitProcess(2);
#endif
 HANDLE input=CreateFileA(path,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0),output=CreateFileA(out,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);DWORD done;unsigned int cases=0;if(input==INVALID_HANDLE_VALUE||output==INVALID_HANDLE_VALUE||!ReadFile(input,&cases,4,&done,0)||done!=4||cases>100000)ExitProcess(3);
 for(unsigned int i=0;i<cases;i++){
  unsigned int count;WorldText::Vec3 bounds[2];if(!ReadFile(input,&count,4,&done,0)||done!=4||count<3||count>64||!ReadFile(input,bounds,24,&done,0)||done!=24)ExitProcess(4);
  PrefabScene::Arena memory;WorldText::Vec3* points=(WorldText::Vec3*)memory.Allocate(count*12);if(!points||!ReadFile(input,points,count*12,&done,0)||done!=count*12)ExitProcess(4);
  FearWorldTreeIntersection::Box box;unsigned int intersects;
#ifdef FEAR_WORLD_TREE_ORACLE
  unsigned char originalBox[0x38]={};typedef void (__thiscall *Bounds)(void*,const void*,const void*);((Bounds)(dll+0x62540))(originalBox,&bounds[0],&bounds[1]);memcpy(&box,originalBox,40);
  unsigned char* polygon=(unsigned char*)memory.Allocate(0x68+count*44);if(!polygon)ExitProcess(5);*(unsigned short*)(polygon+0x60)=(unsigned short)count;for(unsigned int j=0;j<count;j++)memcpy(polygon+0x68+j*44,&points[j],12);intersects=OriginalIntersects(dll+0x14e20,polygon,originalBox)?1:0;
#else
  FearWorldTreeIntersection::SetBounds(bounds[0],bounds[1],box);
#ifdef FEAR_WORLD_TREE_WORKSPACE
  FearWorldTreeIntersection::Workspace* workspace=(FearWorldTreeIntersection::Workspace*)memory.Allocate(sizeof(FearWorldTreeIntersection::Workspace));if(!workspace)ExitProcess(5);
  intersects=FearWorldTreeIntersection::Intersects(points,count,box,*workspace)?1:0;
#else
  intersects=FearWorldTreeIntersection::Intersects(points,count,box)?1:0;
#endif
#endif
  if(!WriteFile(output,&box,40,&done,0)||done!=40||!WriteFile(output,&intersects,4,&done,0)||done!=4)ExitProcess(6);
 }
 CloseHandle(input);CloseHandle(output);ExitProcess(0);
}
