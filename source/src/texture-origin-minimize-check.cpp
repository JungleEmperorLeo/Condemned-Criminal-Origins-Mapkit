#include "WorldPacker.cpp"
#include "TextureOriginMinimize.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
#ifdef FEAR_VERTEX_ORACLE
extern "C" __declspec(dllimport) HMODULE WINAPI LoadLibraryA(const char*);
#pragma comment(linker,"/alternatename:__imp__LoadLibraryA@4=__imp__LoadLibraryA")
#endif
extern "C" void HarnessEntry(){
 char input[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_VERTEX_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_VERTEX_OUTPUT",output,MAX_PATH))ExitProcess(1);
#ifdef FEAR_VERTEX_ORACLE
 LoadLibraryA("E:\\fearsdk\\Dev\\Tools\\msvcr71.dll");LoadLibraryA("E:\\fearsdk\\Dev\\Tools\\msvcp71.dll");LoadLibraryA("E:\\fearsdk\\Dev\\Tools\\MFC71.dll");unsigned char* dll=(unsigned char*)LoadLibraryA("E:\\fearsdk\\Dev\\Tools\\Packers\\WorldPacker.dll");if(!dll)ExitProcess(2);
 void* compute=dll+0x23890;
#endif

 HANDLE in=CreateFileA(input,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);HANDLE out=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);DWORD done;unsigned int count=0;if(in==INVALID_HANDLE_VALUE||out==INVALID_HANDLE_VALUE||!ReadFile(in,&count,4,&done,0)||done!=4||count>10000)ExitProcess(3);
 for(unsigned int i=0;i<count;i++){
 unsigned int n=0;WorldText::Vec3 points[128]={};RenderTextureCoordinates::Basis bases[4]={};
 if(!ReadFile(in,&n,4,&done,0)||done!=4||n<3||n>128||!ReadFile(in,points,n*12,&done,0)||done!=n*12||!ReadFile(in,bases,144,&done,0)||done!=144)ExitProcess(4);
#ifdef FEAR_VERTEX_ORACLE
 static unsigned char surface[160],polygon[100+128*44];memset(surface,0,sizeof(surface));memset(polygon,0,sizeof(polygon));memcpy(surface+8,bases,144);*(unsigned short*)(polygon+0x60)=(unsigned short)n;for(unsigned int j=0;j<n;j++)memcpy(polygon+0x68+j*44,points+j,12);
 __asm {push esi}
 __asm {lea eax,surface}
 __asm {lea esi,polygon}
 __asm {call compute}
 __asm {pop esi}
 memcpy(bases,surface+8,144);
#else
 if(!TextureOriginMinimize::Build(points,n,bases))ExitProcess(5);
#endif
 if(!WriteFile(out,bases,144,&done,0)||done!=144)ExitProcess(6);

 }CloseHandle(in);CloseHandle(out);ExitProcess(0);
}

