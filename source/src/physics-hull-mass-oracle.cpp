#include "WorldPacker.cpp"
// Research-only bounded original integration call; no convex builder or Process.
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
extern "C" __declspec(dllimport) HMODULE WINAPI LoadLibraryA(const char*);
extern "C" __declspec(dllimport) void* WINAPI GetProcAddress(HMODULE,const char*);
#pragma comment(linker,"/alternatename:__imp__LoadLibraryA@4=__imp__LoadLibraryA")
#pragma comment(linker,"/alternatename:__imp__GetProcAddress@8=__imp__GetProcAddress")
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
struct __declspec(align(16)) Point {float x,y,z,w;};
struct Triangle {unsigned int a,b,c;};
struct Geometry {Point* points;unsigned int pointCount,pointFlags;Triangle* triangles;unsigned int triangleCount,triangleFlags;};
static Point points[128];static Triangle triangles[256];
static bool Write(HANDLE f,const void* p,unsigned int n){DWORD done=0;return !n||(WriteFile(f,p,n,&done,0)&&done==n);}
extern "C" void HarnessEntry(){
 char input[MAX_PATH],output[MAX_PATH];DWORD ni=GetEnvironmentVariableA("FEAR_HULL_MASS_INPUT",input,MAX_PATH),no=GetEnvironmentVariableA("FEAR_HULL_MASS_OUTPUT",output,MAX_PATH);if(!ni||ni>=MAX_PATH||!no||no>=MAX_PATH)ExitProcess(1);
 HANDLE file=CreateFileA(input,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);if(file==INVALID_HANDLE_VALUE)ExitProcess(4);unsigned int size=GetFileSize(file,0);if(size<4||size>4u*1024u*1024u)ExitProcess(4);PrefabScene::Arena memory;unsigned char* data=(unsigned char*)memory.Allocate(size);DWORD done=0;if(!data||!ReadFile(file,data,size,&done,0)||done!=size)ExitProcess(4);CloseHandle(file);
 unsigned int count=*(unsigned int*)data,at=4;if(!count||count>5000)ExitProcess(5);
 for(unsigned int c=0;c<count;c++){if(size-at<12)ExitProcess(5);unsigned int nv=*(unsigned int*)(data+at),nt=*(unsigned int*)(data+at+4);float mass=*(float*)(data+at+8);at+=12;if(nv>128||nt>256||!WorldText::Finite(mass)||nv>(size-at)/16)ExitProcess(5);
  for(unsigned int i=0;i<nv;i++){Point p;memcpy(&p,data+at,16);at+=16;if(!WorldText::Finite(p.x)||!WorldText::Finite(p.y)||!WorldText::Finite(p.z)||!WorldText::Finite(p.w))ExitProcess(5);}
  if(nt>(size-at)/12)ExitProcess(5);for(unsigned int i=0;i<nt;i++){Triangle t;memcpy(&t,data+at,12);at+=12;if(t.a>=nv||t.b>=nv||t.c>=nv)ExitProcess(5);}
 }if(at!=size)ExitProcess(5);
 HMODULE kernel=LoadLibraryA("kernel32.dll");typedef DWORD (WINAPI *Mode)(DWORD);Mode mode=kernel?(Mode)GetProcAddress(kernel,"SetErrorMode"):0;if(mode)mode(3);
 LoadLibraryA("E:/fearsdk/Dev/Tools/msvcr71.dll");LoadLibraryA("E:/fearsdk/Dev/Tools/msvcp71.dll");LoadLibraryA("E:/fearsdk/Dev/Tools/MFC71.dll");unsigned char* dll=(unsigned char*)LoadLibraryA("E:/fearsdk/Dev/Tools/Packers/WorldPacker.dll");if(!dll)ExitProcess(2);
 const unsigned char signature[12]={0x55,0x8b,0xec,0x83,0xe4,0xf0,0x83,0xec,0x58,0xd9,0x45,0x0c};for(unsigned int i=0;i<12;i++)if(dll[0x64160+i]!=signature[i])ExitProcess(3);
 typedef unsigned int (__cdecl *Integrate)(Geometry*,float,void*);Integrate integrate=(Integrate)(dll+0x64160);
 file=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(file==INVALID_HANDLE_VALUE||!Write(file,&count,4))ExitProcess(6);at=4;
 for(unsigned int c=0;c<count;c++){unsigned int nv=*(unsigned int*)(data+at),nt=*(unsigned int*)(data+at+4);float mass=*(float*)(data+at+8);at+=12;memcpy(points,data+at,nv*16);at+=nv*16;memcpy(triangles,data+at,nt*12);at+=nt*12;
  Geometry geometry={points,nv,0x80000000u|nv,triangles,nt,0x80000000u|nt};__declspec(align(16)) unsigned char result[80];memset(result,0x5a,80);unsigned int status=integrate(&geometry,mass,result);if(!Write(file,&status,4)||!Write(file,result,80))ExitProcess(6);
 }CloseHandle(file);ExitProcess(0);
}
