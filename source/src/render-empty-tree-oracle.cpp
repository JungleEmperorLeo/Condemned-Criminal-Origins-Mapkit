#include "WorldPacker.cpp"
#include "RenderAttributes.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
extern "C" __declspec(dllimport) HMODULE WINAPI LoadLibraryA(const char*);
extern "C" __declspec(dllimport) void* WINAPI GetProcAddress(HMODULE,const char*);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
#pragma comment(linker,"/alternatename:__imp__LoadLibraryA@4=__imp__LoadLibraryA")
#pragma comment(linker,"/alternatename:__imp__GetProcAddress@8=__imp__GetProcAddress")
// Original42200 node branch serializer over bounded caller-owned sections.
// It never visits shader loading, conversion, Process, or original source data.
struct Stream {void** table;unsigned char* bytes;unsigned int used,capacity;};
static void Put(Stream* stream,const void* bytes,unsigned int count){if(count>stream->capacity-stream->used)ExitProcess(10);memcpy(stream->bytes+stream->used,bytes,count);stream->used+=count;}
static void __fastcall U32(Stream* stream,void*,unsigned int value){Put(stream,&value,4);}
static void __fastcall Vector(Stream* stream,void*,const void* value){Put(stream,value,12);}
static void __fastcall Unexpected(Stream*,void*){ExitProcess(11);}
static bool Read(HANDLE file,void* bytes,unsigned int count){DWORD done;return ReadFile(file,bytes,count,&done,0)&&done==count;}
extern "C" void HarnessEntry(){
 char input[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_EMPTY_TREE_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_EMPTY_TREE_OUTPUT",output,MAX_PATH))ExitProcess(1);
 HANDLE file=CreateFileA(input,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);unsigned int h[3];if(file==INVALID_HANDLE_VALUE||!Read(file,h,12)||h[0]!=0x31425445||h[1]>64||h[2]>1000)ExitProcess(2);
 PrefabScene::Arena memory;unsigned char* sections=(unsigned char*)memory.Allocate((h[1]+1)*40);RenderAttributes::Vertex* vertices=(RenderAttributes::Vertex*)memory.Allocate((h[2]+1)*84);if(!sections||!vertices)ExitProcess(3);memset(sections,0,(h[1]+1)*40);memset(vertices,0,(h[2]+1)*84);
 for(unsigned int i=0;i<h[1];i++){unsigned int s[5];if(!Read(file,s,20)||s[0]>h[2]||s[1]>h[2]-s[0]||s[2]>3||s[3]>1000||s[4]>1000)ExitProcess(3);unsigned char* section=sections+i*40;*(unsigned int*)(section+32)=s[0];*(unsigned int*)(section+36)=s[1];*(unsigned int*)(section+20)=s[2];*(unsigned int*)(section+12)=s[3];*(unsigned int*)(section+16)=s[4];}
 if(!Read(file,vertices,h[2]*84))ExitProcess(3);for(unsigned int i=0;i<h[2];i++)if(!WorldText::FiniteVector(vertices[i].position))ExitProcess(3);unsigned char extra;DWORD done;if(!ReadFile(file,&extra,1,&done,0)||done)ExitProcess(3);CloseHandle(file);
 HMODULE system=LoadLibraryA("kernel32.dll");typedef unsigned int (WINAPI *ErrorMode)(unsigned int);ErrorMode setErrorMode=system?(ErrorMode)GetProcAddress(system,"SetErrorMode"):0;if(!setErrorMode)ExitProcess(4);setErrorMode(3);
 LoadLibraryA("E:/fearsdk/Dev/Tools/msvcr71.dll");LoadLibraryA("E:/fearsdk/Dev/Tools/msvcp71.dll");LoadLibraryA("E:/fearsdk/Dev/Tools/MFC71.dll");unsigned char* dll=(unsigned char*)LoadLibraryA("E:/fearsdk/Dev/Tools/Packers/WorldPacker.dll");if(!dll||dll[0x42200]!=0x83||dll[0x42201]!=0xec||dll[0x42202]!=0x20)ExitProcess(4);
 unsigned char node[0x74]={};*(void**)(node+4)=sections;*(void**)(node+8)=sections+h[1]*40;*(void**)(node+12)=sections+h[1]*40;*(void**)(node+0x24)=vertices;*(void**)(node+0x28)=vertices+h[2];*(void**)(node+0x2c)=vertices+h[2];
 void* table[24];for(unsigned int i=0;i<24;i++)table[i]=(void*)Unexpected;table[11]=(void*)Vector;table[18]=(void*)U32;Stream stream={table,(unsigned char*)memory.Allocate(4096),0,4096};if(!stream.bytes)ExitProcess(3);
 typedef bool (__thiscall *Save)(void*,void*);if(!((Save)(dll+0x42200))(node,&stream))ExitProcess(5);
 HANDLE out=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(out==INVALID_HANDLE_VALUE||!WriteFile(out,stream.bytes,stream.used,&done,0)||done!=stream.used)ExitProcess(6);CloseHandle(out);ExitProcess(0);
}
