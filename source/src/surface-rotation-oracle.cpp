#include "WorldPacker.cpp"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
extern "C" __declspec(dllimport) void* WINAPI LoadLibraryA(const char*);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
#pragma comment(linker,"/alternatename:__imp__LoadLibraryA@4=__imp__LoadLibraryA")
#include "SourceSurfaceRotation.h"
extern "C" void HarnessEntry(){
 LoadLibraryA("E:\\fearsdk\\Dev\\Tools\\msvcr71.dll");LoadLibraryA("E:\\fearsdk\\Dev\\Tools\\msvcp71.dll");LoadLibraryA("E:\\fearsdk\\Dev\\Tools\\MFC71.dll");
 unsigned char* dll=(unsigned char*)LoadLibraryA("E:\\fearsdk\\Dev\\Tools\\Packers\\WorldPacker.dll");if(!dll)ExitProcess(2);
 char output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_ROTATION_OUTPUT",output,MAX_PATH))ExitProcess(1);
 HANDLE out=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(out==INVALID_HANDLE_VALUE)ExitProcess(3);
 typedef void* (__fastcall* Rotate)(void*,void*,void*,const void*);
 float quaternion[4]={0,0,0,1};unsigned int seed=0x183727;char mode[4]={};
 bool nonfinite=GetEnvironmentVariableA("FEAR_ROTATION_NONFINITE",mode,4)&&mode[0]=='1';
 const unsigned int exceptional[8]={0x7fc00000,0xffc00000,0x7fc01234,0xffc01234,0x7f800000,0xff800000,0x7f800001,0xff800001};
 for(unsigned int i=0;i<(nonfinite?24u:2048u);i++){
  union {unsigned int bits[3];WorldText::Vec3 vector;} input;
  for(unsigned int axis=0;axis<3;axis++){
   if(nonfinite)input.bits[axis]=axis==i%3?exceptional[i/3]:0;
   else if(i<8)input.bits[axis]=(i&(1u<<axis))?0x80000000:0;
   else if(i<24)input.bits[axis]=(i&1)?0x80000001:1;
   else {seed=seed*1664525u+1013904223u;input.bits[axis]=seed;if((seed&0x7f800000)==0x7f800000)input.bits[axis]^=0x00800000;}
  }
  WorldText::Vec3 original;((Rotate)(dll+0x67f0))(quaternion,0,&original,&input.vector);
  WorldText::Vec3 native=SourceSurfaceRotation::Identity(input.vector);DWORD done;
  if(!WriteFile(out,&input,12,&done,0)||done!=12||!WriteFile(out,&original,12,&done,0)||done!=12||!WriteFile(out,&native,12,&done,0)||done!=12)ExitProcess(4);
 }CloseHandle(out);ExitProcess(0);
}
