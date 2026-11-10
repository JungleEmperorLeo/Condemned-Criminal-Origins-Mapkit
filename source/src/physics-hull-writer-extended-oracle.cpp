#include "OriginalPhysicsExtendedOracle.h"
struct HullStream {void** table;unsigned char* data;unsigned int size,capacity;};
static void Put(HullStream* s,const void* p,unsigned int n){if(n>s->capacity-s->size)ExitProcess(24);memcpy(s->data+s->size,p,n);s->size+=n;}
static void __fastcall Word(HullStream* s,void*,unsigned int v){Put(s,&v,4);}
static void __fastcall Vector(HullStream* s,void*,const void* p){Put(s,p,12);}
static void __fastcall Pose(HullStream* s,void*,const void* p){Put(s,p,28);}
static void __fastcall Plane(HullStream* s,void*,const void* p){Put(s,p,16);}
static bool HullWrite(void* target,void* world,HullStream* stream,float mass,float density,unsigned int wrapper){unsigned int result;__asm {mov ebx,stream
 push wrapper
 push density
 push mass
 push world
 call target
 add esp,16
 movzx eax,al
 mov result,eax
 }return result!=0;}
static bool Dispatch(void* target,void* world,HullStream* stream){unsigned int result;__asm {mov edi,world
 mov eax,stream
 call target
 movzx eax,al
 mov result,eax
 }return result!=0;}
extern "C" void HarnessEntry(){using namespace OriginalPhysicsOracle;char input[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_HULL_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_HULL_OUTPUT",output,MAX_PATH))ExitProcess(1);
 unsigned char* dll=Load();Install(dll);char mode[4];const bool dispatch=GetEnvironmentVariableA("FEAR_HULL_DISPATCH",mode,4)&&mode[0]=='1';const unsigned char signature[]={0x55,0x8b,0xec,0x83,0xe4,0xf0,0x6a,0xff};for(unsigned int i=0;i<sizeof(signature);i++)if(dll[0x3cde0+i]!=signature[i])ExitProcess(3);if(dispatch&&(dll[0x3d190]!=0x83||dll[0x3d191]!=0xec||dll[0x3d192]!=0x2c))ExitProcess(3);
 HANDLE file=CreateFileA(input,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);if(file==INVALID_HANDLE_VALUE||GetFileSize(file,0)>16u*1024u*1024u)ExitProcess(4);unsigned int count;if(!Read(file,&count,4)||!count||count>1000)ExitProcess(4);HANDLE out=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(out==INVALID_HANDLE_VALUE||!Write(out,&count,4))ExitProcess(5);
 for(unsigned int c=0;c<count;c++){PrefabScene::Arena memory;Begin(memory);unsigned int h[7];if(!Read(file,h,28)||h[1]>1)ExitProcess(6);for(unsigned int i=2;i<7;i++)if(!WorldText::Finite(((float*)h)[i]))ExitProcess(6);unsigned char world[0xc0]={};lstrcpyA((char*)world+0x64,"bounded-hull");memcpy(world+0xa8,h+4,12);if(!Polygons(file,world,h[0],memory))ExitProcess(6);*(unsigned int*)world=3;memcpy(world+4,h+2,8);
  void* table[24];for(unsigned int i=0;i<24;i++)table[i]=(void*)Unexpected;table[8]=(void*)Pose;table[10]=table[11]=(void*)Vector;table[14]=table[18]=(void*)Word;table[9]=(void*)Plane;HullStream stream={table,(unsigned char*)memory.Allocate(1024*1024),0,1024*1024};if(!stream.data)ExitProcess(23);unsigned int status=(dispatch?Dispatch(dll+0x3d190,world,&stream):HullWrite(dll+0x3cde0,world,&stream,((float*)h)[2],((float*)h)[3],h[1]))?1u:0u;unsigned int header[4]={status,stream.size,allocationCalls,allocationBytes};if(!Write(out,header,16)||!Write(out,stream.data,stream.size))ExitProcess(7);
 }unsigned char extra;DWORD done=0;if(!ReadFile(file,&extra,1,&done,0)||done)ExitProcess(6);CloseHandle(file);CloseHandle(out);ExitProcess(0);
}
