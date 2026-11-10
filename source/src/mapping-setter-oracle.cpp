#include "WorldPacker.cpp"
#include "SourceMappingSkewRepair.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
extern "C" __declspec(dllimport) void* WINAPI LoadLibraryA(const char*);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
#pragma comment(linker,"/alternatename:__imp__LoadLibraryA@4=__imp__LoadLibraryA")
extern "C" void HarnessEntry(){
 LoadLibraryA("E:\\fearsdk\\Dev\\Tools\\msvcr71.dll");LoadLibraryA("E:\\fearsdk\\Dev\\Tools\\msvcp71.dll");LoadLibraryA("E:\\fearsdk\\Dev\\Tools\\MFC71.dll");
 unsigned char* dll=(unsigned char*)LoadLibraryA("E:\\fearsdk\\Dev\\Tools\\Packers\\WorldPacker.dll");if(!dll)ExitProcess(2);
 char output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_SETTER_OUTPUT",output,MAX_PATH))ExitProcess(1);
 HANDLE out=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(out==INVALID_HANDLE_VALUE)ExitProcess(3);
 typedef void (__fastcall* Setter)(void*,void*,const void*,const void*,const void*,const void*);
 const unsigned int normal[3]={0,0x3f800000,0};
 char mode[4]={};
 if(GetEnvironmentVariableA("FEAR_SETTER_SKEW",mode,4)&&mode[0]=='1'){
  const WorldText::Vec3 normals[4]={{0,1,0},{1,0,0},{0.3f,0.4f,0.5f},{1,2,3}};unsigned int seed=0x5f750;
  for(unsigned int i=0;i<256;i++){
   WorldText::Mapping mapping={};mapping.valid=true;float* input=(float*)&mapping.origin;
   for(unsigned int j=0;j<9;j++){seed=seed*1664525u+1013904223u;input[j]=float(int((seed>>16)&255)-128)/32;}
   WorldText::Mapping native=mapping;float original[9];const WorldText::Vec3& n=normals[i%4];
   ((Setter)(dll+0x5f750))(original,0,&n,&mapping.origin,&mapping.p,&mapping.q);
   PrefabScene::SourceMappingSkewRepair::Repair(n,native);DWORD done;
   if(!WriteFile(out,&n,12,&done,0)||done!=12||!WriteFile(out,&mapping.origin,36,&done,0)||done!=36||!WriteFile(out,original,36,&done,0)||done!=36||!WriteFile(out,&native.origin,36,&done,0)||done!=36)ExitProcess(4);
  }
  CloseHandle(out);ExitProcess(0);
 }
 for(unsigned int mask=0;mask<128;mask++){
  unsigned int input[9]={0,0,0,0x3c23d70a,0,0,0,0,0x3c23d70a};
  const unsigned int positions[7]={0,1,2,4,5,6,7};
  for(unsigned int k=0;k<7;k++)if(mask&(1u<<k))input[positions[k]]=0x80000000;
  unsigned int result[9]={};((Setter)(dll+0x5f750))(result,0,normal,input,input+3,input+6);DWORD done;
  if(!WriteFile(out,input,36,&done,0)||done!=36||!WriteFile(out,result,36,&done,0)||done!=36)ExitProcess(4);
 }CloseHandle(out);ExitProcess(0);
}
