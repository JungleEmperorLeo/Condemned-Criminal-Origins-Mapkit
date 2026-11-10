#include "WorldPacker.cpp"
#include "ExtrusionVolume.h"
#include "PortalBounds.h"
#include "PortalRecord.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
extern "C" __declspec(dllimport) void* WINAPI LoadLibraryA(const char*);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
#pragma comment(linker,"/alternatename:__imp__LoadLibraryA@4=__imp__LoadLibraryA")
struct Face {WorldText::Vec3* vertices;unsigned int count,capacity,padding;ExtrusionVolume::Plane plane;unsigned int tail[3];};
static WorldText::Vec3 vertices[2][128];static Face faces[2];static unsigned int manager[48],volumes[2][40];
struct Writer {void** table;};static void* table[21];static unsigned char record[2048];static unsigned int written;
static void Copy(const void* p,unsigned int n){const unsigned char* b=(const unsigned char*)p;for(unsigned int i=0;i<n;i++)record[written++]=b[i];}
static void __fastcall U16(Writer*,void*,unsigned int v){unsigned short value=(unsigned short)v;Copy(&value,2);}
static void __fastcall Vector(Writer*,void*,const void* p){Copy(p,12);}
static void __fastcall Plane(Writer*,void*,const void* p){Copy(p,16);}
static void __fastcall Float(Writer*,void*,float v){Copy(&v,4);}
extern "C" void HarnessEntry(){
 LoadLibraryA("E:\\fearsdk\\Dev\\Tools\\msvcr71.dll");LoadLibraryA("E:\\fearsdk\\Dev\\Tools\\msvcp71.dll");LoadLibraryA("E:\\fearsdk\\Dev\\Tools\\MFC71.dll");unsigned char* dll=(unsigned char*)LoadLibraryA("E:\\fearsdk\\Dev\\Tools\\Packers\\WorldPacker.dll");if(!dll)ExitProcess(2);
 char input[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_READER_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_TREE_OUTPUT",output,MAX_PATH))ExitProcess(1);
 HANDLE in=CreateFileA(input,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0),out=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(in==INVALID_HANDLE_VALUE||out==INVALID_HANDLE_VALUE)ExitProcess(3);DWORD done;unsigned int count;if(!ReadFile(in,&count,4,&done,0)||done!=4)ExitProcess(4);
 table[0x50/4]=(void*)U16;table[0x2c/4]=(void*)Vector;table[0x18/4]=(void*)Plane;table[0x38/4]=(void*)Float;Writer writer={table};typedef void (__thiscall *Serialize)(void*,void*);
 for(unsigned int t=0;t<count;t++){
  Face& face=faces[0];face.vertices=vertices[0];if(!ReadFile(in,&face.count,4,&done,0)||done!=4||!face.count||face.count>128)ExitProcess(5);face.capacity=face.count;
  if(!ReadFile(in,face.vertices,face.count*12,&done,0)||done!=face.count*12||!ReadFile(in,&face.plane,16,&done,0)||done!=16)ExitProcess(6);
  written=0;((Serialize)(dll+0x2c9f0))(&face,&writer);if(written!=2+12*face.count+32)ExitProcess(20);
  WorldText::Vec3 center;float radius;if(!PortalBounds::Compute(face.vertices,face.count,center,radius))ExitProcess(21);const unsigned char* c=(const unsigned char*)&center;for(unsigned int j=0;j<12;j++)if(record[written-16+j]!=c[j])ExitProcess(22);c=(const unsigned char*)&radius;for(unsigned int j=0;j<4;j++)if(record[written-4+j]!=c[j])ExitProcess(23);
  static unsigned char nativeRecord[2048];unsigned int nativeBytes;if(!PortalRecord::Write(nativeRecord,sizeof(nativeRecord),face.vertices,face.count,face.plane,nativeBytes)||nativeBytes!=written)ExitProcess(24);for(unsigned int j=0;j<written;j++)if(nativeRecord[j]!=record[j])ExitProcess(25);
  if(!WriteFile(out,record,written,&done,0)||done!=written)ExitProcess(8);
 }
 CloseHandle(in);CloseHandle(out);ExitProcess(0);
}

