#include "WorldPacker.cpp"
#include "SourceNormalArithmetic.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
#ifdef FEAR_NORMAL_ORACLE
extern "C" __declspec(dllimport) HMODULE WINAPI LoadLibraryA(const char*);
#pragma comment(linker,"/alternatename:__imp__LoadLibraryA@4=__imp__LoadLibraryA")
struct Face {unsigned int unused;WorldText::Vec3 normal;};
struct Generator {unsigned int* offsets;Face** faces;};
static Face faces[64];static Face* pointers[64];
#endif
static WorldText::Vec3 adjacent[64];
static bool Read(HANDLE f,void* p,unsigned int n){DWORD done;return ReadFile(f,p,n,&done,0)&&done==n;}
extern "C" void HarnessEntry(){
 char input[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_NORMAL_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_NORMAL_OUTPUT",output,MAX_PATH))ExitProcess(1);
#ifdef FEAR_NORMAL_ORACLE
 LoadLibraryA("E:\\fearsdk\\Dev\\Tools\\msvcr71.dll");LoadLibraryA("E:\\fearsdk\\Dev\\Tools\\msvcp71.dll");LoadLibraryA("E:\\fearsdk\\Dev\\Tools\\MFC71.dll");
 unsigned char* dll=(unsigned char*)LoadLibraryA("E:\\fearsdk\\Dev\\Tools\\Packers\\WorldPacker.dll");if(!dll)ExitProcess(2);
 typedef WorldText::Vec3* (__thiscall* Sum)(Generator*,WorldText::Vec3*,const WorldText::Vec3*,float,unsigned int);Sum sum=(Sum)(dll+0x6c60);
#endif
 HANDLE in=CreateFileA(input,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);HANDLE out=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);
 unsigned int cases=0;if(in==INVALID_HANDLE_VALUE||out==INVALID_HANDLE_VALUE||!Read(in,&cases,4)||cases>10000)ExitProcess(3);
 for(unsigned int i=0;i<cases;i++){
  unsigned int count=0;WorldText::Vec3 face={},result={};float cosine=0;
  if(!Read(in,&count,4)||!count||count>64||!Read(in,&face,12)||!Read(in,&cosine,4)||!Read(in,adjacent,count*12))ExitProcess(4);
#ifdef FEAR_NORMAL_ORACLE
  for(unsigned int j=0;j<count;j++){faces[j].normal=adjacent[j];pointers[j]=faces+j;}unsigned int offsets[2]={0,count};Generator generator={offsets,pointers};sum(&generator,&result,&face,cosine,0);
#else
  if(!SourceNormalArithmetic::Sum(face,cosine,adjacent,count,result))ExitProcess(5);
#endif
  DWORD done;if(!WriteFile(out,&result,12,&done,0)||done!=12)ExitProcess(6);
 }CloseHandle(in);CloseHandle(out);ExitProcess(0);
}
