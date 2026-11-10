#include "WorldPacker.cpp"
#include "ExtrusionVolume.h"
#include "SectorOrientation.h"
#include "SectorClosure.h"
#include "SectorPreparation.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
extern "C" __declspec(dllimport) void* WINAPI LoadLibraryA(const char*);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
#pragma comment(linker,"/alternatename:__imp__LoadLibraryA@4=__imp__LoadLibraryA")
struct Face {WorldText::Vec3* vertices;unsigned int count,capacity,padding;ExtrusionVolume::Plane plane;unsigned int tail[3];};
struct Volume {Face** faces;unsigned int count,capacity,padding;unsigned int tail[30];};
static WorldText::Vec3 vertices[16][128],ownPoints[16][128];static Face faces[16];static Face* pointers[16];static SectorOrientation::Face own[16];static Volume volume;
extern "C" void HarnessEntry(){
 LoadLibraryA("E:\\fearsdk\\Dev\\Tools\\msvcr71.dll");LoadLibraryA("E:\\fearsdk\\Dev\\Tools\\msvcp71.dll");LoadLibraryA("E:\\fearsdk\\Dev\\Tools\\MFC71.dll");unsigned char* dll=(unsigned char*)LoadLibraryA("E:\\fearsdk\\Dev\\Tools\\Packers\\WorldPacker.dll");if(!dll)ExitProcess(2);
 char input[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_READER_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_TREE_OUTPUT",output,MAX_PATH))ExitProcess(1);
 HANDLE in=CreateFileA(input,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0),out=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(in==INVALID_HANDLE_VALUE||out==INVALID_HANDLE_VALUE)ExitProcess(3);DWORD done;unsigned int count;if(!ReadFile(in,&count,4,&done,0)||done!=4)ExitProcess(4);
 typedef bool (__thiscall *Check)(void*,float);typedef void (__thiscall *Flip)(void*);
 for(unsigned int t=0;t<count;t++){
  unsigned int n;if(!ReadFile(in,&n,4,&done,0)||done!=4||n>16)ExitProcess(5);volume.faces=pointers;volume.count=volume.capacity=n;
  for(unsigned int k=0;k<n;k++){Face& f=faces[k];pointers[k]=&f;f.vertices=vertices[k];if(!ReadFile(in,&f.count,4,&done,0)||done!=4||f.count<3||f.count>128)ExitProcess(6);f.capacity=f.count;if(!ReadFile(in,f.vertices,f.count*12,&done,0)||done!=f.count*12)ExitProcess(7);if(!ExtrusionVolume::ComputeInternalPlane(f.vertices,f.count,f.plane))ExitProcess(8);own[k].points=ownPoints[k];own[k].count=f.count;own[k].plane=f.plane;for(unsigned int j=0;j<f.count;j++)ownPoints[k][j]=f.vertices[j];}
  static const float tolerances[]={0,0.01f,0.1f,1.0f};unsigned int result[4];
  for(unsigned int test=0;test<4;test++){bool original=((Check)(dll+0x12bf0))(&volume,tolerances[test]);bool native=SectorClosure::Check(own,n,tolerances[test]);if(original!=native)ExitProcess(20+test);result[test]=original?1u:0u;}
  if(!WriteFile(out,result,16,&done,0)||done!=16)ExitProcess(9);
 }
 CloseHandle(in);CloseHandle(out);ExitProcess(0);
}
