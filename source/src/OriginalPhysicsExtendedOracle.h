#pragma once
// Research-only extended support: 400000 allocation calls; byte and scratch caps unchanged.
// Mechanically copied from OriginalPhysicsOracle.h. Existing support remains unchanged.
#include "WorldPacker.cpp"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
extern "C" __declspec(dllimport) HMODULE WINAPI LoadLibraryA(const char*);
extern "C" __declspec(dllimport) void* WINAPI GetProcAddress(HMODULE,const char*);
#pragma comment(linker,"/alternatename:__imp__LoadLibraryA@4=__imp__LoadLibraryA")
#pragma comment(linker,"/alternatename:__imp__GetProcAddress@8=__imp__GetProcAddress")
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
namespace OriginalPhysicsOracle {
static PrefabScene::Arena* active=0;static unsigned int allocationBytes=0,allocationCalls=0;
static unsigned int allocatorObject[12]={};
static unsigned int scratchBegin=0,scratchEnd=0;
static void __fastcall Unexpected(void*,void*){ExitProcess(21);}
template<unsigned int Slot> static void __fastcall Trap(void*,void*){ExitProcess(100+Slot);}
static void* __fastcall Allocate(void*,void*,unsigned int bytes,unsigned int){if(!active||bytes>8u*1024u*1024u||bytes>64u*1024u*1024u-allocationBytes||allocationCalls>=400000)ExitProcess(22);allocationBytes+=bytes;allocationCalls++;void* p=active->Allocate(bytes+16);if(!p)ExitProcess(23);return (void*)(((unsigned int)p+15)&~15u);}
static void __fastcall Release(void*,void*,void*){}
static void __fastcall ReleaseChunk(void*,void*,void*,unsigned int,unsigned int){}
static void* __fastcall StackAllocate(void*,void*,unsigned int){ExitProcess(26);return 0;}
static void __fastcall StackRelease(void*,void*,void* p,unsigned int bytes){unsigned int address=(unsigned int)p;if(address<scratchBegin||address>scratchEnd||bytes>scratchEnd-address||address+bytes!=allocatorObject[8])ExitProcess(27);allocatorObject[8]=address;allocatorObject[9]+=bytes;}
static bool __fastcall Log(void*,void*,unsigned int,const char*){return true;}
static long WINAPI Exception(void* pointers){void** pair=(void**)pointers;unsigned int* context=(unsigned int*)pair[1];char path[MAX_PATH];if(GetEnvironmentVariableA("FEAR_HULL_OUTPUT",path,MAX_PATH)&&lstrlenA(path)<MAX_PATH-7){lstrcpyA(path+lstrlenA(path),".crash");HANDLE f=CreateFileA(path,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(f!=INVALID_HANDLE_VALUE){DWORD done=0;WriteFile(f,pair[0],20,&done,0);WriteFile(f,context+39,48,&done,0);CloseHandle(f);}}ExitProcess(25);return 1;}
static unsigned char* Load(){HMODULE kernel=LoadLibraryA("kernel32.dll");typedef DWORD (WINAPI *Mode)(DWORD);Mode mode=kernel?(Mode)GetProcAddress(kernel,"SetErrorMode"):0;if(mode)mode(3);typedef void*(WINAPI *Filter)(void*);Filter filter=kernel?(Filter)GetProcAddress(kernel,"SetUnhandledExceptionFilter"):0;if(filter)filter((void*)Exception);LoadLibraryA("E:/fearsdk/Dev/Tools/msvcr71.dll");LoadLibraryA("E:/fearsdk/Dev/Tools/msvcp71.dll");LoadLibraryA("E:/fearsdk/Dev/Tools/MFC71.dll");unsigned char* dll=(unsigned char*)LoadLibraryA("E:/fearsdk/Dev/Tools/Packers/WorldPacker.dll");if(!dll)ExitProcess(2);return dll;}
static void Install(unsigned char* dll){static void* allocatorTable[12]={(void*)Allocate,(void*)Release,(void*)Trap<2>,(void*)Trap<3>,(void*)Allocate,(void*)ReleaseChunk,(void*)Trap<6>,(void*)Trap<7>,(void*)Trap<8>,(void*)StackAllocate,(void*)StackRelease,(void*)Trap<11>};allocatorObject[0]=(unsigned int)allocatorTable;*(void**)(dll+0x9f430)=allocatorObject;static void* loggerTable[8]={(void*)Unexpected,(void*)Unexpected,(void*)Unexpected,(void*)Unexpected,(void*)Unexpected,(void*)Unexpected,(void*)Log,(void*)Unexpected};static void* logger[]={loggerTable};typedef void(__cdecl *SetOutput)(void*);((SetOutput)(dll+0x53580))(logger);}
static bool Read(HANDLE f,void* p,unsigned int n){DWORD done=0;return !n||(ReadFile(f,p,n,&done,0)&&done==n);}
static bool Write(HANDLE f,const void* p,unsigned int n){DWORD done=0;return !n||(WriteFile(f,p,n,&done,0)&&done==n);}
inline void Begin(PrefabScene::Arena& memory){active=&memory;allocationBytes=allocationCalls=0;void* p=memory.Allocate(8u*1024u*1024u+16);if(!p)ExitProcess(23);scratchBegin=((unsigned int)p+15)&~15u;scratchEnd=scratchBegin+8u*1024u*1024u;allocatorObject[8]=scratchBegin;allocatorObject[9]=8u*1024u*1024u;allocatorObject[11]=0;}
inline bool Polygons(HANDLE f,unsigned char* world,unsigned int n,PrefabScene::Arena& memory){unsigned char* first=0;unsigned char* last=0;unsigned int references=0;if(n>10000)return false;
 for(unsigned int i=0;i<n;i++){unsigned int h[2];if(!Read(f,h,8)||h[1]>512||h[1]>100000-references)return false;unsigned char* polygon=(unsigned char*)memory.Allocate(0x68+h[1]*0x2c);unsigned char* surface=(unsigned char*)memory.Allocate(0xb0);if(!polygon||!surface)return false;*(void**)(polygon+0x38)=surface;*(unsigned int*)(surface+0xac)=h[0];*(unsigned short*)(polygon+0x60)=(unsigned short)h[1];
  for(unsigned int v=0;v<h[1];v++){WorldText::Vec3 point;if(!Read(f,&point,12)||!WorldText::FiniteVector(point))return false;memcpy(polygon+0x68+v*0x2c,&point,12);}references+=h[1];if(!first)first=polygon;if(last)*(void**)(last+8)=polygon+4;last=polygon;
 }if(first){*(void**)(last+8)=first+4;*(void**)(world+0x24)=first+4;}*(unsigned int*)(world+0x54)=references;return true;}
inline void Collect(void* target,void* world,void* vertices,void* indices,unsigned int generate){__asm {mov esi,indices
 push generate
 push world
 push vertices
 call target
 add esp,12
 }}
}
