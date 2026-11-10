#include "WorldPacker.cpp"
#include "RenderPackedRanges.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
extern "C" __declspec(dllimport) HMODULE WINAPI LoadLibraryA(const char*);
extern "C" __declspec(dllimport) void* WINAPI GetProcAddress(HMODULE,const char*);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
#pragma comment(linker,"/alternatename:__imp__LoadLibraryA@4=__imp__LoadLibraryA")
#pragma comment(linker,"/alternatename:__imp__GetProcAddress@8=__imp__GetProcAddress")
// One-section original writer oracle with an actual NULL section Shader*.
// Input/output match render_packed_ranges.py; the supplied declaration must
// be the Working declaration, but it is never installed in the mock Shader*.
struct Bytes {unsigned char* data;unsigned int count,capacity;bool failed;
 void Put(const void* value,unsigned int size){if(size>capacity-count){failed=true;return;}memcpy(data+count,value,size);count+=size;}
 void U32(unsigned int value){Put(&value,4);}void U16(unsigned short value){Put(&value,2);}void U8(unsigned char value){Put(&value,1);}
};
struct MockStream {void** table;Bytes* bytes;};
static void __fastcall Write32(MockStream* stream,void*,unsigned int value){stream->bytes->U32(value);}
static void __fastcall Write16(MockStream* stream,void*,unsigned int value){stream->bytes->U16((unsigned short)value);}
static void __fastcall Write8(MockStream* stream,void*,unsigned int value){stream->bytes->U8((unsigned char)value);}
static void __fastcall Unexpected(MockStream*,void*){ExitProcess(19);}
struct Pair {void* node;void* section;};
static bool Read(HANDLE file,void* out,unsigned int size){DWORD done;return ReadFile(file,out,size,&done,0)&&done==size;}
static void CallWriter(void* target,MockStream* stream,void* mesh,unsigned int* declarations,unsigned int* allocation,unsigned int* declarationBytes){
 __asm {
  push esi
  mov esi,stream
  push declarationBytes
  push allocation
  push declarations
  push mesh
  call target
  add esp,16
  pop esi
 }
}
extern "C" void HarnessEntry(){
 char input[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_RANGES_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_RANGES_OUTPUT",output,MAX_PATH))ExitProcess(1);
 HANDLE file=CreateFileA(input,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);unsigned int header[3],h[8];
 if(file==INVALID_HANDLE_VALUE||!Read(file,header,12)||header[0]!=0x31474e52||header[1]!=1||header[2]||!Read(file,h,32))ExitProcess(2);
 if(!h[0]||h[0]>4096||h[1]||h[3]!=10||!h[4]||h[4]>65535||h[5]>1000||!h[6]||h[6]>100000||h[7]>1000)ExitProcess(3);
 PrefabScene::Arena memory;char* name=(char*)memory.Allocate(h[0]+1);RenderVertexPacking::Element declaration[10];
 RenderAttributes::Vertex* vertices=(RenderAttributes::Vertex*)memory.Allocate((h[5]+h[4])*84);unsigned int* indices=(unsigned int*)memory.Allocate((h[7]+h[6])*12);
 if(!name||!vertices||!indices||!Read(file,name,h[0])||!Read(file,declaration,80))ExitProcess(3);name[h[0]]=0;
 if(!RenderPackedRanges::Name(name)||!RenderPackedRanges::SameDeclaration(declaration,10,RenderVertexPacking::Working,10))ExitProcess(4);
 memset(vertices,0,(h[5]+h[4])*84);memset(indices,0,(h[7]+h[6])*12);
 if(!Read(file,vertices+h[5],h[4]*84)||!Read(file,indices+h[7]*3,h[6]*12))ExitProcess(3);
 unsigned char extra;DWORD done;if(!ReadFile(file,&extra,1,&done,0)||done)ExitProcess(3);CloseHandle(file);
 for(unsigned int q=0;q<h[6]*3;q++)if(indices[h[7]*3+q]<h[5]||indices[h[7]*3+q]-h[5]>=h[4])ExitProcess(4);
 HMODULE system=LoadLibraryA("kernel32.dll");typedef unsigned int (WINAPI *ErrorMode)(unsigned int);ErrorMode setErrorMode=system?(ErrorMode)GetProcAddress(system,"SetErrorMode"):0;if(!setErrorMode)ExitProcess(6);setErrorMode(3);
 LoadLibraryA("E:/fearsdk/Dev/Tools/msvcr71.dll");LoadLibraryA("E:/fearsdk/Dev/Tools/msvcp71.dll");LoadLibraryA("E:/fearsdk/Dev/Tools/MFC71.dll");unsigned char* dll=(unsigned char*)LoadLibraryA("E:/fearsdk/Dev/Tools/Packers/WorldPacker.dll");if(!dll)ExitProcess(6);
 if(dll[0x4e880]!=0x6a||dll[0x4e881]!=0xff||dll[0x4e882]!=0x68)ExitProcess(7);
 unsigned char node[0x74]={},records[80]={},mesh[48]={};unsigned char* section=records+40;
 *(void**)(node+4)=records;*(void**)(node+8)=records+80;*(void**)(node+12)=records+80;
 *(void**)(node+0x14)=indices;*(void**)(node+0x18)=indices+(h[7]+h[6])*3;*(void**)(node+0x1c)=indices+(h[7]+h[6])*3;
 *(void**)(node+0x24)=vertices;*(void**)(node+0x28)=vertices+h[5]+h[4];*(void**)(node+0x2c)=vertices+h[5]+h[4];
 *(unsigned int*)(records+24)=h[7];*(const char**)section=name;*(void**)(section+4)=0;
 *(unsigned int*)(section+8)=h[2];*(unsigned int*)(section+24)=h[6];*(unsigned int*)(section+32)=h[5];*(unsigned int*)(section+36)=h[4];
 Pair pair={node,section};*(void**)(mesh+0x24)=&pair;*(void**)(mesh+0x28)=&pair+1;*(void**)(mesh+0x2c)=&pair+1;
 Bytes bytes={(unsigned char*)memory.Allocate(16*1024*1024),0,16*1024*1024,false};if(!bytes.data)ExitProcess(5);
 void* table[24];for(unsigned int i=0;i<24;i++)table[i]=(void*)Unexpected;table[18]=(void*)Write32;table[20]=(void*)Write16;table[22]=(void*)Write8;MockStream stream={table,&bytes};
 unsigned int declarations=0,allocation=0,declarationElements=0;CallWriter(dll+0x4e880,&stream,mesh,&declarations,&declarationElements,&allocation);
 if(bytes.failed)ExitProcess(8);HANDLE out=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);unsigned int result[8]={0x31474e52,1,bytes.count,declarations,allocation,declarationElements,0,0};
 if(out==INVALID_HANDLE_VALUE||!WriteFile(out,result,32,&done,0)||done!=32||!WriteFile(out,bytes.data,bytes.count,&done,0)||done!=bytes.count)ExitProcess(9);CloseHandle(out);ExitProcess(0);
}
