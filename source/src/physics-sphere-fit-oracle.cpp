#include "WorldPacker.cpp"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
extern "C" __declspec(dllimport) HMODULE WINAPI LoadLibraryA(const char*);
extern "C" __declspec(dllimport) void* WINAPI GetProcAddress(HMODULE,const char*);
#pragma comment(linker,"/alternatename:__imp__LoadLibraryA@4=__imp__LoadLibraryA")
#pragma comment(linker,"/alternatename:__imp__GetProcAddress@8=__imp__GetProcAddress")
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
struct __declspec(align(16)) Point {float x,y,z,w;};
static Point points[128];
static bool Write(HANDLE f,const void* p,unsigned int n){DWORD done=0;return WriteFile(f,p,n,&done,0)&&done==n;}
extern "C" void HarnessEntry(){
 char input[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_SPHERE_FIT_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_SPHERE_FIT_OUTPUT",output,MAX_PATH))ExitProcess(1);
 HMODULE kernel=LoadLibraryA("kernel32.dll");typedef DWORD (WINAPI *Mode)(DWORD);Mode mode=kernel?(Mode)GetProcAddress(kernel,"SetErrorMode"):0;if(mode)mode(3);
 LoadLibraryA("E:/fearsdk/Dev/Tools/msvcr71.dll");LoadLibraryA("E:/fearsdk/Dev/Tools/msvcp71.dll");LoadLibraryA("E:/fearsdk/Dev/Tools/MFC71.dll");unsigned char* dll=(unsigned char*)LoadLibraryA("E:/fearsdk/Dev/Tools/Packers/WorldPacker.dll");if(!dll)ExitProcess(2);
 const unsigned char signature[12]={0x55,0x8b,0xec,0x83,0xe4,0xf0,0x83,0xec,0x34,0x8b,0x45,0x14};for(unsigned int i=0;i<12;i++)if(dll[0x64a20+i]!=signature[i])ExitProcess(3);
 typedef float* (__cdecl *Fit)(float*,Point*,int,unsigned int,float);Fit fit=(Fit)(dll+0x64a20);
 HANDLE file=CreateFileA(input,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);if(file==INVALID_HANDLE_VALUE)ExitProcess(4);unsigned int size=GetFileSize(file,0);if(size<4||size>1024u*1024u)ExitProcess(4);PrefabScene::Arena memory;unsigned char* data=(unsigned char*)memory.Allocate(size);DWORD done=0;if(!data||!ReadFile(file,data,size,&done,0)||done!=size)ExitProcess(4);CloseHandle(file);
 unsigned int count=*(unsigned int*)data,at=4;if(!count||count>5000)ExitProcess(5);file=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(file==INVALID_HANDLE_VALUE||!Write(file,&count,4))ExitProcess(6);
 for(unsigned int c=0;c<count;c++){if(size-at<8)ExitProcess(5);unsigned int n=*(unsigned int*)(data+at);float epsilon=*(float*)(data+at+4);at+=8;if(n>128||!WorldText::Finite(epsilon)||epsilon<0||n>(size-at)/12)ExitProcess(5);
  for(unsigned int i=0;i<n;i++){memcpy(&points[i],data+at,12);at+=12;points[i].w=0;if(!WorldText::Finite(points[i].x)||!WorldText::Finite(points[i].y)||!WorldText::Finite(points[i].z))ExitProcess(5);}
  __declspec(align(16)) float sphere[4];fit(sphere,points,(int)n,0,epsilon);if(!Write(file,&n,4)||!Write(file,sphere,16)||!Write(file,points,n*16))ExitProcess(6);
 }
 CloseHandle(file);if(at!=size)ExitProcess(5);ExitProcess(0);
}
