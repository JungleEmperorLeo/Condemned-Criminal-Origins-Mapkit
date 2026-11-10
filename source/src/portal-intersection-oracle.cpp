#include "WorldPacker.cpp"
#include "ExtrusionVolume.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
extern "C" __declspec(dllimport) void* WINAPI LoadLibraryA(const char*);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
#pragma comment(linker,"/alternatename:__imp__LoadLibraryA@4=__imp__LoadLibraryA")
struct Face {WorldText::Vec3* vertices;unsigned int count,capacity,padding;ExtrusionVolume::Plane plane;unsigned int tail[3];};
static WorldText::Vec3 vertices[2][128];static Face faces[2];static unsigned int manager[48],volumes[2][40];
extern "C" void HarnessEntry(){
 LoadLibraryA("E:\\fearsdk\\Dev\\Tools\\msvcr71.dll");LoadLibraryA("E:\\fearsdk\\Dev\\Tools\\msvcp71.dll");LoadLibraryA("E:\\fearsdk\\Dev\\Tools\\MFC71.dll");unsigned char* dll=(unsigned char*)LoadLibraryA("E:\\fearsdk\\Dev\\Tools\\Packers\\WorldPacker.dll");if(!dll)ExitProcess(2);
 char input[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_READER_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_TREE_OUTPUT",output,MAX_PATH))ExitProcess(1);
 HANDLE in=CreateFileA(input,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0),out=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(in==INVALID_HANDLE_VALUE||out==INVALID_HANDLE_VALUE)ExitProcess(3);DWORD done;unsigned int count;if(!ReadFile(in,&count,4,&done,0)||done!=4)ExitProcess(4);
 typedef Face* (__thiscall *Intersect)(void*,Face*,Face*,void*,void*);
 for(unsigned int t=0;t<count;t++){
  for(unsigned int k=0;k<2;k++){Face& f=faces[k];f.vertices=vertices[k];if(!ReadFile(in,&f.count,4,&done,0)||done!=4||f.count<3||f.count>128)ExitProcess(5);f.capacity=f.count;if(!ReadFile(in,f.vertices,f.count*12,&done,0)||done!=f.count*12)ExitProcess(6);if(!ExtrusionVolume::ComputeInternalPlane(f.vertices,f.count,f.plane))ExitProcess(7);}
  Face* result=((Intersect)(dll+0x2fec0))(manager,&faces[0],&faces[1],volumes[0],volumes[1]);unsigned int n=result?result->count:0;if(!WriteFile(out,&n,4,&done,0)||done!=4)ExitProcess(8);if(n&&(!WriteFile(out,result->vertices,n*12,&done,0)||done!=n*12))ExitProcess(9);
 }
 CloseHandle(in);CloseHandle(out);ExitProcess(0);
}
