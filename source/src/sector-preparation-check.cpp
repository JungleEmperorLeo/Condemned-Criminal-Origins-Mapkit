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
 typedef bool (__thiscall *Cleanup)(void*);typedef void* (__cdecl *Allocate)(unsigned int);typedef bool (__thiscall *Check)(void*,float);typedef void (__thiscall *Flip)(void*);
 for(unsigned int t=0;t<count;t++){
  unsigned int n;if(!ReadFile(in,&n,4,&done,0)||done!=4||n>16)ExitProcess(5);volume.faces=pointers;volume.count=volume.capacity=n;
  for(unsigned int k=0;k<n;k++){Face& f=faces[k];pointers[k]=&f;f.vertices=vertices[k];if(!ReadFile(in,&f.count,4,&done,0)||done!=4||f.count<3||f.count>128)ExitProcess(6);f.capacity=f.count;f.vertices=(WorldText::Vec3*)((Allocate)(dll+0x62cc4))(f.count*12);if(!f.vertices)ExitProcess(10);if(!ReadFile(in,f.vertices,f.count*12,&done,0)||done!=f.count*12)ExitProcess(7);f.plane.x=f.plane.y=f.plane.z=f.plane.d=0;own[k].points=ownPoints[k];own[k].count=f.count;own[k].plane=f.plane;for(unsigned int j=0;j<f.count;j++)ownPoints[k][j]=f.vertices[j];}
  bool original=true,originalFlipped=false;unsigned int originalFailure=0;
  for(unsigned int k=0;k<n;k++)if(!((Cleanup)(dll+0x14260))(&faces[k])){original=false;originalFailure=2;break;}
  if(original&&!((Check)(dll+0x12a90))(&volume,0.1f)){((Flip)(dll+0x12bc0))(&volume);originalFlipped=true;if(!((Check)(dll+0x12a90))(&volume,0.1f)){original=false;originalFailure=3;}}
  if(original&&!((Check)(dll+0x12bf0))(&volume,0.1f)){original=false;originalFailure=4;}
  bool ownFlipped;SectorPreparation::Failure failure;bool native=SectorPreparation::Prepare(own,n,ownFlipped,failure);if(native!=original||ownFlipped!=originalFlipped||(unsigned int)failure!=originalFailure)ExitProcess(20);
  for(unsigned int k=0;k<n;k++){if(faces[k].count!=own[k].count)ExitProcess(21);for(unsigned int j=0;j<faces[k].count*3;j++)if(((unsigned int*)faces[k].vertices)[j]!=((unsigned int*)own[k].points)[j])ExitProcess(22);if(original)for(unsigned int j=0;j<4;j++)if(((unsigned int*)&faces[k].plane)[j]!=((unsigned int*)&own[k].plane)[j])ExitProcess(23);}
  unsigned int result[3]={original?1u:0u,originalFlipped?1u:0u,originalFailure};if(!WriteFile(out,result,12,&done,0)||done!=12)ExitProcess(9);
 }
 CloseHandle(in);CloseHandle(out);ExitProcess(0);
}
