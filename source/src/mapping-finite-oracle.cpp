#include "WorldPacker.cpp"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
extern "C" __declspec(dllimport) void* WINAPI LoadLibraryA(const char*);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
#pragma comment(linker,"/alternatename:__imp__LoadLibraryA@4=__imp__LoadLibraryA")
extern "C" void HarnessEntry(){
 LoadLibraryA("E:\\fearsdk\\Dev\\Tools\\msvcr71.dll");LoadLibraryA("E:\\fearsdk\\Dev\\Tools\\msvcp71.dll");LoadLibraryA("E:\\fearsdk\\Dev\\Tools\\MFC71.dll");
 unsigned char* dll=(unsigned char*)LoadLibraryA("E:\\fearsdk\\Dev\\Tools\\Packers\\WorldPacker.dll");if(!dll)ExitProcess(2);
 char output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_FINITE_OUTPUT",output,MAX_PATH))ExitProcess(1);
 HANDLE out=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(out==INVALID_HANDLE_VALUE)ExitProcess(3);
 const unsigned int values[]={0,0x80000000,1,0x80000001,0x7f7fffff,0xff7fffff,0x7f800000,0xff800000,0x7fc00000,0xffc00000,0x7f800001};
 for(unsigned int i=0;i<sizeof(values)/sizeof(values[0]);i++)for(unsigned int axis=0;axis<3;axis++){
  unsigned int vector[3]={0x3f800000,0x40000000,0x40400000};vector[axis]=values[i];
  unsigned int result=((unsigned int (__cdecl*)(const void*))(dll+0x5f4f0))(vector);DWORD done;
  unsigned int record[5]={values[i],axis,vector[0],vector[1],vector[2]};
  if(!WriteFile(out,record,sizeof(record),&done,0)||done!=sizeof(record)||!WriteFile(out,&result,4,&done,0)||done!=4)ExitProcess(4);
 }CloseHandle(out);ExitProcess(0);
}
