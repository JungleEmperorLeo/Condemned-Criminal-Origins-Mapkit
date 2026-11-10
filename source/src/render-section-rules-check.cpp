#include "WorldPacker.cpp"
#include "RenderSectionRules.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
#ifdef FEAR_SECTION_ORACLE
extern "C" __declspec(dllimport) HMODULE WINAPI LoadLibraryA(const char*);
#pragma comment(linker,"/alternatename:__imp__LoadLibraryA@4=__imp__LoadLibraryA")
static void* matchEntry;
// Enter only the original CountSection matching scan, after candidate material
// preparation/lookup. This deliberately does not exercise resource loading.
__declspec(naked) static unsigned int __fastcall MatchContinuation(void*,void*,const RenderSectionRules::Record*,unsigned int,int){
 __asm {
  mov eax,ecx
  sub esp,28h
  push ebx
  push ebp
  push esi
  push edi
  mov ebx,eax
  mov esi,[esp+3ch]
  lea edi,[esp+10h]
  mov ecx,10
  rep movsd
  mov edi,ebx
  jmp dword ptr [matchEntry]
 }
}
#endif
static bool Read(HANDLE h,void* p,unsigned int n){DWORD done;return ReadFile(h,p,n,&done,0)&&done==n;}
static bool Write(HANDLE h,const void* p,unsigned int n){DWORD done;return WriteFile(h,p,n,&done,0)&&done==n;}
extern "C" void HarnessEntry(){
 char input[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_VERTEX_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_VERTEX_OUTPUT",output,MAX_PATH))ExitProcess(1);
 HANDLE in=CreateFileA(input,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);unsigned int header[4];
 if(in==INVALID_HANDLE_VALUE||!Read(in,header,16)||header[0]>1||header[1]>100000||header[3]>100000||header[1]+header[3]>100000)ExitProcess(2);
 unsigned int mode=header[0],count=header[1],ideal=header[2],queries=header[3],capacity=count+queries+1;PrefabScene::Arena memory;
 RenderSectionRules::Record* records=(RenderSectionRules::Record*)memory.Allocate(capacity*40);RenderSectionRules::Record* query=(RenderSectionRules::Record*)memory.Allocate((queries+1)*40);char* strings=(char*)memory.Allocate(capacity*64);unsigned int* indices=(unsigned int*)memory.Allocate((queries+1)*4);
 if(!records||!query||!strings||!indices)ExitProcess(3);
 for(unsigned int i=0;i<count+queries;i++){
  RenderSectionRules::Record& r=i<count?records[i]:query[i-count];if(!Read(in,r.bytes,40)||!Read(in,strings+i*64,64)||strings[i*64+63])ExitProcess(3);
  const char* name=*(unsigned int*)r.bytes?strings+i*64:0;memcpy(r.bytes,&name,4);
 }
 CloseHandle(in);
#ifdef FEAR_SECTION_ORACLE
 LoadLibraryA("E:/fearsdk/Dev/Tools/msvcr71.dll");LoadLibraryA("E:/fearsdk/Dev/Tools/msvcp71.dll");LoadLibraryA("E:/fearsdk/Dev/Tools/MFC71.dll");unsigned char* dll=(unsigned char*)LoadLibraryA("E:/fearsdk/Dev/Tools/Packers/WorldPacker.dll");if(!dll)ExitProcess(4);
 // Suppress the original null-name assertion UI; 0x59F0 then implements its
 // explicit null ordering. The original file on disk is never changed.
 dll[0x9c48d]=1;
 if(mode==0){const unsigned char signature[3]={0x83,0xec,0x08};for(unsigned int i=0;i<3;i++)if(dll[0x46950+i]!=signature[i])ExitProcess(5);typedef void (__cdecl* Sort)(void*,void*,int);((Sort)(dll+0x46950))(records,records+count,(int)ideal);}
 else {
  const unsigned char signature[4]={0x33,0xdb,0x33,0xed};for(unsigned int i=0;i<4;i++)if(dll[0x46c44+i]!=signature[i])ExitProcess(5);matchEntry=dll+0x46c44;
  unsigned char node[0x234];memset(node,0,sizeof(node));*(RenderSectionRules::Record**)(node+4)=records;*(RenderSectionRules::Record**)(node+8)=records+count;*(RenderSectionRules::Record**)(node+12)=records+capacity;
  for(unsigned int i=0;i<queries;i++){unsigned int add;memcpy(&add,query[i].bytes+24,4);indices[i]=MatchContinuation(node,0,query+i,add,RenderSectionRules::Kind(query[i]));if(indices[i]>=capacity)ExitProcess(6);}
  count=(unsigned int)(*(RenderSectionRules::Record**)(node+8)-records);
 }
#else
 if(mode==0){if(!RenderSectionRules::Range(records,records+count,ideal))ExitProcess(4);}
 else for(unsigned int i=0;i<queries;i++){unsigned int add;memcpy(&add,query[i].bytes+24,4);if(!RenderSectionRules::Count(records,count,capacity,query[i],add,indices[i]))ExitProcess(6);}
#endif
 for(unsigned int i=0;i<count;i++){const char* name=RenderSectionRules::Material(records[i]);unsigned int reference=name?(unsigned int)(name-strings)/64+1:0;memcpy(records[i].bytes,&reference,4);}
 HANDLE out=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);unsigned int result[2]={count,queries};if(out==INVALID_HANDLE_VALUE||!Write(out,result,8)||!Write(out,indices,queries*4)||!Write(out,records,count*40))ExitProcess(7);CloseHandle(out);ExitProcess(0);
}
