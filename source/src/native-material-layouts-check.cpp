#include "WorldPacker.cpp"
#include "NativeMaterialLayouts.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
struct CancelState {unsigned int calls,after;};
static bool Cancelled(void* opaque){CancelState* state=(CancelState*)opaque;return ++state->calls>=state->after&&state->after;}
static bool Write(HANDLE file,const void* data,unsigned int size){DWORD done=0;return !size||WriteFile(file,data,size,&done,0)&&done==size;}
extern "C" void HarnessEntry(){
 HMODULE kernel=LoadLibraryA("kernel32.dll");
 typedef unsigned int (WINAPI *SetErrors)(unsigned int);
 SetErrors setErrors=kernel?(SetErrors)GetProcAddress(kernel,"SetErrorMode"):0;
 if(setErrors)setErrors(3);
 char root[MAX_PATH],input[MAX_PATH],output[MAX_PATH];
 if(!GetEnvironmentVariableA("FEAR_MATERIAL_CACHE_ROOT",root,MAX_PATH)||!GetEnvironmentVariableA("FEAR_MATERIAL_CACHE_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_MATERIAL_CACHE_OUTPUT",output,MAX_PATH))ExitProcess(1);
 HANDLE file=CreateFileA(input,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);if(file==INVALID_HANDLE_VALUE)ExitProcess(2);
 DWORD high=0,bytes=GetFileSize(file,&high),done=0;if(high||bytes<16||bytes>8*1024*1024){CloseHandle(file);ExitProcess(2);}
 PrefabScene::Arena storage;unsigned char* data=(unsigned char*)storage.Allocate(bytes);if(!data){CloseHandle(file);ExitProcess(3);}bool read=ReadFile(file,data,bytes,&done,0)&&done==bytes;CloseHandle(file);if(!read)ExitProcess(3);
 const unsigned int* header=(const unsigned int*)data;unsigned int count=header[1];if(header[0]!=0x31514c4eu||count>32768)ExitProcess(4);
 unsigned int at=16;const char** names=(const char**)storage.Allocate(count*4);const NativeMaterialLayouts::Layout** layouts=(const NativeMaterialLayouts::Layout**)storage.Allocate(count*4);if(!names||!layouts)ExitProcess(3);
 for(unsigned int i=0;i<count;i++){if(bytes-at<4)ExitProcess(4);unsigned int n=*(unsigned int*)(data+at);at+=4;if(!n||n>=260||n>bytes-at)ExitProcess(4);char* name=(char*)storage.Allocate(n+1);if(!name)ExitProcess(3);for(unsigned int c=0;c<n;c++){if(!data[at+c])ExitProcess(4);name[c]=(char)data[at+c];}name[n]=0;names[i]=name;at+=n;}if(at!=bytes)ExitProcess(4);
 void* cacheStorage=storage.Allocate(sizeof(NativeMaterialLayouts::Cache));if(!cacheStorage)ExitProcess(3);NativeMaterialLayouts::Cache* cache=new(cacheStorage)NativeMaterialLayouts::Cache();CancelState cancel={0,header[3]};
 if(!cache->Initialize(root,Cancelled,&cancel,header[2]?header[2]:32u*1024u*1024u)){unsigned int result=10u+cache->Error().code;cache->~Cache();ExitProcess(result);}
 for(unsigned int i=0;i<count;i++)if(!cache->Resolve(names[i],layouts+i)){unsigned int result=10u+cache->Error().code;cache->~Cache();ExitProcess(result);}
 file=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(file==INVALID_HANDLE_VALUE){cache->~Cache();ExitProcess(5);}
 unsigned int resultHeader[6]={0x314c4d4eu,count,cache->Count(),cache->ShaderCount(),cache->ShaderEdges(),cache->OwnedBytes()};bool ok=Write(file,resultHeader,24);
 for(unsigned int i=0;i<count&&ok;i++){const NativeMaterialLayouts::Layout& l=*layouts[i];unsigned int m=lstrlenA(l.material),s=lstrlenA(l.shader);unsigned int record[7]={(unsigned int)l.state,l.count,l.warnings,(unsigned int)l.compilerResult,l.shaderIdentity,m,s};ok=Write(file,record,28)&&Write(file,l.material,m)&&Write(file,l.shader,s)&&Write(file,l.declaration,l.count*8);}
 CloseHandle(file);cache->~Cache();ExitProcess(ok?0:5);
}
