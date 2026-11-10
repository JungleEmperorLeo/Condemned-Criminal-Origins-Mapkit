#include "WorldPacker.cpp"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
extern "C" __declspec(dllimport) HMODULE WINAPI LoadLibraryA(const char*);
extern "C" __declspec(dllimport) void* WINAPI GetProcAddress(HMODULE,const char*);
extern "C" __declspec(dllimport) BOOL WINAPI VirtualProtect(void*,unsigned int,DWORD,DWORD*);
#pragma comment(linker,"/alternatename:__imp__VirtualProtect@16=__imp__VirtualProtect")
#pragma comment(linker,"/alternatename:__imp__LoadLibraryA@4=__imp__LoadLibraryA")
#pragma comment(linker,"/alternatename:__imp__GetProcAddress@8=__imp__GetProcAddress")
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
static char dllPath[MAX_PATH],meshPath[MAX_PATH],outPath[MAX_PATH];
static float axisTolerance=.05f;
struct Triangle {unsigned short a,b,c;};
extern "C" void HarnessEntry(){
 if(!GetEnvironmentVariableA("MOPPER_DLL",dllPath,MAX_PATH)||!GetEnvironmentVariableA("MOPPER_MESH",meshPath,MAX_PATH)||!GetEnvironmentVariableA("MOPPER_OUT",outPath,MAX_PATH))ExitProcess(1);
 HANDLE f=CreateFileA(meshPath,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);DWORD done;
 if(f==INVALID_HANDLE_VALUE)ExitProcess(2);unsigned int bytes=GetFileSize(f,0);if(bytes<108||bytes>16*1024*1024)ExitProcess(2);
 unsigned char* mesh=(unsigned char*)VirtualAlloc(0,bytes,MEM_COMMIT|MEM_RESERVE,4);if(!mesh||!ReadFile(f,mesh,bytes,&done,0)||done!=bytes)ExitProcess(2);CloseHandle(f);
 unsigned int points=*(unsigned int*)(mesh+100),indices=*(unsigned int*)(mesh+104);
 if(!points||points>=65536||!indices||indices%3||indices>3000000||bytes!=108+points*12+indices*2||*(unsigned int*)(mesh+32)!=1)ExitProcess(3);
 WorldText::Vec3* vertices=(WorldText::Vec3*)(mesh+108);Triangle* triangles=(Triangle*)(mesh+108+points*12);
 for(unsigned int i=0;i<points;i++)if(!WorldText::FiniteVector(vertices[i]))ExitProcess(3);
 for(unsigned int i=0;i<indices/3;i++)if(triangles[i].a>=points||triangles[i].b>=points||triangles[i].c>=points)ExitProcess(3);
 HMODULE module=LoadLibraryA(dllPath);if(!module)ExitProcess(4);
 char radiusMode[4]={};if(GetEnvironmentVariableA("MOPPER_RADIUS_003",radiusMode,4)&&radiusMode[0]=='1'){
  unsigned int* radius=(unsigned int*)((unsigned char*)module+0xbd94c);if(*radius!=0x3c23d70au)ExitProcess(10);
  DWORD old;if(!VirtualProtect(radius,4,4,&old))ExitProcess(11);*radius=0x3cf5c28fu;DWORD unused;VirtualProtect(radius,4,old,&unused);
 }
 char configMode[4]={};if(GetEnvironmentVariableA("MOPPER_FEAR_CONFIG",configMode,4)&&configMode[0]=='1'){
  unsigned char* instruction=(unsigned char*)module+0x1283;unsigned int* operand=(unsigned int*)(instruction+2);
  if(instruction[0]!=0xd9||instruction[1]!=0x05||*operand!=(unsigned int)((unsigned char*)module+0xbd958))ExitProcess(12);
  DWORD old;if(!VirtualProtect(operand,4,0x40,&old))ExitProcess(13);*operand=(unsigned int)&axisTolerance;DWORD unused;VirtualProtect(operand,4,old,&unused);
 }
 char tinyMode[4]={};if(GetEnvironmentVariableA("MOPPER_KEEP_SMALL_TRIANGLES",tinyMode,4)&&tinyMode[0]=='1'){
  // Explicit compatibility profile; this is not a recovered FEAR constant.
  // Validate all three archived triangle-validity constant loads before patching.
  unsigned int* cutoff=(unsigned int*)((unsigned char*)module+0xc289c);if(*cutoff!=0x33d6bf95u)ExitProcess(14);
  const unsigned int refs[3]={0x24743,0x247c3,0x2906d};for(unsigned int i=0;i<3;i++){unsigned char* instruction=(unsigned char*)module+refs[i];if(instruction[0]!=0xd9||instruction[1]!=0x05||*(unsigned int*)(instruction+2)!=(unsigned int)cutoff)ExitProcess(14);}
  DWORD old;if(!VirtualProtect(cutoff,4,4,&old))ExitProcess(15);*cutoff=0x2edbe6ffu;DWORD unused;VirtualProtect(cutoff,4,old,&unused);
 }
 typedef int (WINAPI *Generate)(int,const WorldText::Vec3*,int,const Triangle*);
 typedef int (WINAPI *Retrieve)(int,char*);typedef int (WINAPI *GetScale)(float*);typedef int (WINAPI *GetOrigin)(WorldText::Vec3*);
 Generate generate=(Generate)GetProcAddress(module,"GenerateMoppCode");Retrieve retrieve=(Retrieve)GetProcAddress(module,"RetrieveMoppCode");GetScale scale=(GetScale)GetProcAddress(module,"RetrieveMoppScale");GetOrigin origin=(GetOrigin)GetProcAddress(module,"RetrieveMoppOrigin");
 if(!generate||!retrieve||!scale||!origin)ExitProcess(5);int size=generate(points,vertices,indices/3,triangles);if(size<=0||size>16*1024*1024)ExitProcess(6);
 unsigned char* result=(unsigned char*)VirtualAlloc(0,size+20,MEM_COMMIT|MEM_RESERVE,4);if(!result)ExitProcess(7);
 if(!retrieve(size,(char*)result+20)||!scale((float*)(result+12))||!origin((WorldText::Vec3*)result))ExitProcess(8);*(unsigned int*)(result+16)=(unsigned int)size;
 f=CreateFileA(outPath,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(f==INVALID_HANDLE_VALUE||!WriteFile(f,result,size+20,&done,0)||done!=(unsigned int)size+20)ExitProcess(9);CloseHandle(f);ExitProcess(0);
}
