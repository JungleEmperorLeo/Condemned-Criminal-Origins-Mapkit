#include "WorldPacker.cpp"
#include "ExtrusionVolume.h"
#include "PortalContour.h"
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
 typedef bool (__thiscall *Cleanup)(void*);
 for(unsigned int t=0;t<count;t++){
  Face& f=faces[0];f.vertices=vertices[0];if(!ReadFile(in,&f.count,4,&done,0)||done!=4||f.count<3||f.count>128)ExitProcess(5);typedef void* (__cdecl *Allocate)(unsigned int);f.capacity=f.count;f.vertices=(WorldText::Vec3*)((Allocate)(dll+0x62cc4))(f.count*12);if(!f.vertices)ExitProcess(10);if(!ReadFile(in,f.vertices,f.count*12,&done,0)||done!=f.count*12)ExitProcess(6);
  unsigned int ownCount=f.count;static WorldText::Vec3 own[128];for(unsigned int j=0;j<f.count;j++)own[j]=f.vertices[j];ExtrusionVolume::Plane ownPlane;bool native=PortalContour::Clean(own,ownCount,ownPlane),original=((Cleanup)(dll+0x14260))(&f);
  if(native!=original)ExitProcess(20);if(ownCount!=f.count)ExitProcess(21);for(unsigned int j=0;j<f.count*3;j++)if(((unsigned int*)own)[j]!=((unsigned int*)f.vertices)[j])ExitProcess(22);
  if(original)for(unsigned int j=0;j<4;j++)if(((unsigned int*)&ownPlane)[j]!=((unsigned int*)&f.plane)[j])ExitProcess(23);
  unsigned int result[2]={original?1u:0u,f.count};if(!WriteFile(out,result,8,&done,0)||done!=8)ExitProcess(8);
 }
 CloseHandle(in);CloseHandle(out);ExitProcess(0);
}
