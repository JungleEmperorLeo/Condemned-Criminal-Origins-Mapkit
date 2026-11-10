#include "WorldPacker.cpp"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
extern "C" __declspec(dllimport) HMODULE WINAPI LoadLibraryA(const char*);
extern "C" __declspec(dllimport) void* WINAPI GetProcAddress(HMODULE,const char*);
#pragma comment(linker,"/alternatename:__imp__LoadLibraryA@4=__imp__LoadLibraryA")
#pragma comment(linker,"/alternatename:__imp__GetProcAddress@8=__imp__GetProcAddress")
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
// Bounded caller-owned CPoly lists supplied directly to the original Sphere
// serializer. No Process, filesystem conversion, or original world is involved.
struct Stream {void** table;unsigned char data[64];unsigned int size;};
static void Put(Stream* s,const void* p,unsigned int n){if(n>64-s->size)ExitProcess(20);memcpy(s->data+s->size,p,n);s->size+=n;}
static void __fastcall Word(Stream* s,void*,unsigned int v){Put(s,&v,4);}
static void __fastcall Pose(Stream* s,void*,const void* p){Put(s,p,28);}
static void __fastcall Unexpected(void*,void*){ExitProcess(21);}
static bool __fastcall Log(void*,void*,unsigned int,const char*){return true;}
static PrefabScene::Arena* activeMemory=0;
static void* __fastcall AlignedAllocate(void*,void*,unsigned int bytes,unsigned int tag){if(!activeMemory||bytes>1600000||tag!=7)ExitProcess(22);void* p=activeMemory->Allocate(bytes+16);if(!p)ExitProcess(7);return (void*)(((unsigned int)p+15)&~15u);}
static void __fastcall AlignedRelease(void*,void*,void*){}
static bool Read(HANDLE f,void* p,unsigned int n){DWORD done=0;return !n||(ReadFile(f,p,n,&done,0)&&done==n);}
static bool Write(HANDLE f,const void* p,unsigned int n){DWORD done=0;return !n||(WriteFile(f,p,n,&done,0)&&done==n);}
extern "C" void HarnessEntry(){
 char input[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_SPHERE_WRITER_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_SPHERE_WRITER_OUTPUT",output,MAX_PATH))ExitProcess(1);
 HMODULE kernel=LoadLibraryA("kernel32.dll");typedef DWORD (WINAPI *Mode)(DWORD);Mode mode=kernel?(Mode)GetProcAddress(kernel,"SetErrorMode"):0;if(mode)mode(3);
 LoadLibraryA("E:/fearsdk/Dev/Tools/msvcr71.dll");LoadLibraryA("E:/fearsdk/Dev/Tools/msvcp71.dll");LoadLibraryA("E:/fearsdk/Dev/Tools/MFC71.dll");unsigned char* dll=(unsigned char*)LoadLibraryA("E:/fearsdk/Dev/Tools/Packers/WorldPacker.dll");if(!dll)ExitProcess(2);
 const unsigned char signature[]={0x55,0x8b,0xec,0x83,0xe4,0xf0,0x6a,0xff,0x68,0x38,0x52,0x08,0x10};for(unsigned int i=0;i<sizeof(signature);i++)if(dll[0x3c190+i]!=signature[i])ExitProcess(3);
 // The direct call bypasses Process's Havok allocator setup. Supply only its
 // two observed allocation slots, without replacing any geometry arithmetic.
 void* allocationTable[2]={(void*)AlignedAllocate,(void*)AlignedRelease};void* allocator[]={allocationTable};void* oldAllocator=*(void**)(dll+0x9f430);*(void**)(dll+0x9f430)=allocator;
 void* loggerTable[8];for(unsigned int i=0;i<8;i++)loggerTable[i]=(void*)Unexpected;loggerTable[6]=(void*)Log;void* logger[]={loggerTable};typedef void(__cdecl *SetOutput)(void*);((SetOutput)(dll+0x53580))(logger);
 HANDLE file=CreateFileA(input,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);if(file==INVALID_HANDLE_VALUE||GetFileSize(file,0)>16u*1024u*1024u)ExitProcess(4);unsigned int count=0;if(!Read(file,&count,4)||!count||count>1000)ExitProcess(4);
 HANDLE out=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(out==INVALID_HANDLE_VALUE||!Write(out,&count,4))ExitProcess(5);
 for(unsigned int c=0;c<count;c++){
  PrefabScene::Arena memory;activeMemory=&memory;unsigned int header[7];if(!Read(file,header,28))ExitProcess(6);unsigned int polygons=header[0],wrapper=header[1];if(polygons>10000||wrapper>1)ExitProcess(6);float* values=(float*)(header+2);for(unsigned int j=0;j<5;j++)if(!WorldText::Finite(values[j]))ExitProcess(6);
  unsigned char world[0xc0]={};lstrcpyA((char*)world+0x64,"bounded-sphere");memcpy(world+0xa8,values+2,12);unsigned char* first=0;unsigned char* previous=0;unsigned int corners=0;
  for(unsigned int p=0;p<polygons;p++){
   unsigned int ph[2];if(!Read(file,ph,8)||ph[1]>512||ph[1]>100000-corners)ExitProcess(6);unsigned char* polygon=(unsigned char*)memory.Allocate(0x68+ph[1]*0x2c);unsigned char* surface=(unsigned char*)memory.Allocate(0xb0);if(!polygon||!surface)ExitProcess(7);*(void**)(polygon+0x38)=surface;*(unsigned int*)(surface+0xac)=ph[0];*(unsigned short*)(polygon+0x60)=(unsigned short)ph[1];
   for(unsigned int v=0;v<ph[1];v++){float pos[3];if(!Read(file,pos,12)||!WorldText::Finite(pos[0])||!WorldText::Finite(pos[1])||!WorldText::Finite(pos[2]))ExitProcess(6);memcpy(polygon+0x68+v*0x2c,pos,12);}corners+=ph[1];if(!first)first=polygon;if(previous)*(void**)(previous+8)=polygon+4;previous=polygon;
  }
  if(first){*(void**)(previous+8)=first+4;*(void**)(world+0x24)=first+4;}*(unsigned int*)(world+0x54)=corners;
  void* table[24];for(unsigned int i=0;i<24;i++)table[i]=(void*)Unexpected;table[8]=(void*)Pose;table[14]=table[18]=(void*)Word;Stream stream;stream.table=table;stream.size=0;
  typedef bool(__cdecl *Writer)(void*,void*,float,float,bool);if(!((Writer)(dll+0x3c190))(world,&stream,values[0],values[1],wrapper!=0)||stream.size!=(wrapper?48u:44u)||!Write(out,&stream.size,4)||!Write(out,stream.data,stream.size))ExitProcess(8);
 }
 unsigned char extra;DWORD done=0;if(!ReadFile(file,&extra,1,&done,0)||done)ExitProcess(6);CloseHandle(file);CloseHandle(out);*(void**)(dll+0x9f430)=oldAllocator;((SetOutput)(dll+0x53580))(0);ExitProcess(0);
}
