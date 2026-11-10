#include "WorldPacker.cpp"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
extern "C" __declspec(dllimport) void* WINAPI LoadLibraryA(const char*);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
#pragma comment(linker,"/alternatename:__imp__LoadLibraryA@4=__imp__LoadLibraryA")
extern "C" void HarnessEntry(){
 LoadLibraryA("E:\\fearsdk\\Dev\\Tools\\msvcr71.dll");LoadLibraryA("E:\\fearsdk\\Dev\\Tools\\msvcp71.dll");LoadLibraryA("E:\\fearsdk\\Dev\\Tools\\MFC71.dll");
 unsigned char* dll=(unsigned char*)LoadLibraryA("E:\\fearsdk\\Dev\\Tools\\Packers\\WorldPacker.dll");if(!dll)ExitProcess(2);
 char output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_NUMBER_OUTPUT",output,MAX_PATH))ExitProcess(1);
 HANDLE out=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(out==INVALID_HANDLE_VALUE)ExitProcess(3);
 const char* cases[]={"0x00000000","0x80000000","0","-0","0.000000","-0.000000","0x3F800000","0xBF800000","1.25","-1.25","0x00000001","0x80000001"};
 for(unsigned int i=0;i<sizeof(cases)/sizeof(cases[0]);i++){
  double value=((double (__cdecl*)(const char*))(dll+0x75560))(cases[i]);float narrowed=(float)value;DWORD done;
  if(!WriteFile(out,&value,8,&done,0)||done!=8||!WriteFile(out,&narrowed,4,&done,0)||done!=4)ExitProcess(4);
 }CloseHandle(out);ExitProcess(0);
}
