#include "WorldPacker.cpp"
#include "ResourceOrder.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
extern "C" __declspec(dllimport) void* WINAPI LoadLibraryA(const char*);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
#pragma comment(linker,"/alternatename:__imp__LoadLibraryA@4=__imp__LoadLibraryA")
static char path[MAX_PATH];
extern "C" void HarnessEntry(){
 if(!LoadLibraryA("E:\\fearsdk\\Dev\\Tools\\msvcr71.dll")||!LoadLibraryA("E:\\fearsdk\\Dev\\Tools\\msvcp71.dll")||!LoadLibraryA("E:\\fearsdk\\Dev\\Tools\\MFC71.dll"))ExitProcess(1);
 unsigned char* dll=(unsigned char*)LoadLibraryA("E:\\fearsdk\\Dev\\Tools\\Packers\\WorldPacker.dll");if(!dll)ExitProcess(2);
 if(!GetEnvironmentVariableA("FEAR_ORDER_INPUT",path,MAX_PATH))ExitProcess(3);HANDLE f=CreateFileA(path,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);if(f==INVALID_HANDLE_VALUE)ExitProcess(4);unsigned int bytes=GetFileSize(f,0);if(bytes<4||bytes>1024*1024)ExitProcess(5);unsigned char* input=(unsigned char*)VirtualAlloc(0,bytes,MEM_COMMIT|MEM_RESERVE,4);DWORD done;if(!input||!ReadFile(f,input,bytes,&done,0)||done!=bytes)ExitProcess(6);CloseHandle(f);unsigned int count=*(unsigned int*)input;if(count>10000)ExitProcess(7);
 if(!GetEnvironmentVariableA("FEAR_ORDER_OUTPUT",path,MAX_PATH))ExitProcess(3);f=CreateFileA(path,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(f==INVALID_HANDLE_VALUE)ExitProcess(8);
 PrefabScene::Arena arena;ResourceOrder::List own;own.Init(&arena);unsigned int original[10]={};typedef void* (__thiscall *Ctor)(void*);typedef void* (__thiscall *Insert)(void*,void*,void*);((Ctor)(dll+0x2c490))(original);unsigned int cursor=4;
 for(unsigned int i=0;i<count;i++){
  unsigned int begin=cursor;while(cursor<bytes&&input[cursor])cursor++;if(cursor==bytes||cursor-begin>4096)ExitProcess(9);const char* s=(const char*)(input+begin);cursor++;unsigned int pair[]={(unsigned int)s,i+1},result[2]={};((Insert)(dll+0x2bde0))(original,result,pair);bool inserted;if(!own.Insert(s,i+1,&inserted))ExitProcess(10);if(own.count!=original[3])ExitProcess(20);
  unsigned int header[]={i+1,original[3],original[8],original[9]};if(!WriteFile(f,header,16,&done,0)||done!=16)ExitProcess(11);
  unsigned int* sentinel=(unsigned int*)original[2];unsigned int* node=(unsigned int*)sentinel[0];ResourceOrder::Node* native=own.head;unsigned int visited=0;
  while(node!=sentinel){if(!native||visited++>=own.count||node[3]!=native->value)ExitProcess(21);if(!WriteFile(f,node+3,4,&done,0)||done!=4)ExitProcess(11);node=(unsigned int*)node[0];native=native->next;}
  if(native||visited!=own.count)ExitProcess(22);
 }
 if(cursor!=bytes)ExitProcess(12);CloseHandle(f);ExitProcess(0);
}
