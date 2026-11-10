#include "WorldPacker.cpp"
#include "VisibilityBlock.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
static char path[MAX_PATH];
static unsigned char* readEnv(const char* name,unsigned int* size){
 if(!GetEnvironmentVariableA(name,path,MAX_PATH))ExitProcess(1);
 HANDLE file=CreateFileA(path,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);if(file==INVALID_HANDLE_VALUE)ExitProcess(2);
 *size=GetFileSize(file,0);if(!*size||*size>16*1024*1024)ExitProcess(3);
 unsigned char* data=(unsigned char*)VirtualAlloc(0,*size,MEM_COMMIT|MEM_RESERVE,4);DWORD done;
 if(!data||!ReadFile(file,data,*size,&done,0)||done!=*size)ExitProcess(4);CloseHandle(file);return data;
}
extern "C" void HarnessEntry(){
 unsigned int bytes;unsigned char* input=readEnv("FEAR_VISIBILITY_INPUT",&bytes);if(bytes<4)ExitProcess(30);unsigned int count=*(unsigned int*)input;if(!count||count>5000||bytes<4+count*24)ExitProcess(31);
 const StreamingSplit::Bounds* bounds=(const StreamingSplit::Bounds*)(input+4);const char** names=(const char**)VirtualAlloc(0,count*4,MEM_COMMIT|MEM_RESERVE,4);if(!names)ExitProcess(32);unsigned int cursor=4+count*24;
 for(unsigned int i=0;i<count;i++){if(cursor>=bytes)ExitProcess(33);names[i]=(const char*)(input+cursor);while(cursor<bytes&&input[cursor])cursor++;if(cursor==bytes)ExitProcess(34);cursor++;}if(cursor!=bytes)ExitProcess(35);
 VisibilityBlock::Rectangular block;if(!block.Build(bounds,names,count))ExitProcess(36);if(!GetEnvironmentVariableA("FEAR_VISIBILITY_OUTPUT",path,MAX_PATH))ExitProcess(37);HANDLE f=CreateFileA(path,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);DWORD done;if(f==INVALID_HANDLE_VALUE||!WriteFile(f,block.data,block.size,&done,0)||done!=block.size)ExitProcess(38);CloseHandle(f);ExitProcess(0);
}
