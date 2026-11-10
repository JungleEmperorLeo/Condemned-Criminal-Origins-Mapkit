#include "WorldPacker.cpp"
#include "ClientFxFile.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
extern "C" void HarnessEntry(){
 char path[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_FX_INPUT",path,MAX_PATH))ExitProcess(1);
 HANDLE f=CreateFileA(path,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);if(f==INVALID_HANDLE_VALUE)ExitProcess(2);
 unsigned int size=GetFileSize(f,0);DWORD done=0;unsigned char* data=size&&size<32*1024*1024?(unsigned char*)VirtualAlloc(0,size,MEM_COMMIT|MEM_RESERVE,4):0;
 bool loaded=data&&ReadFile(f,data,size,&done,0)&&done==size;CloseHandle(f);if(!loaded)ExitProcess(3);
 ClientFxFile::Reader reader;if(!reader.Read(data,size))ExitProcess(4);
 if(!GetEnvironmentVariableA("FEAR_FX_OUTPUT",path,MAX_PATH))ExitProcess(1);
 f=CreateFileA(path,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(f==INVALID_HANDLE_VALUE)ExitProcess(5);
 unsigned int stats[3]={reader.count,reader.keyCount,reader.propertyCount};bool written=WriteFile(f,stats,12,&done,0)&&done==12;CloseHandle(f);ExitProcess(written?0:6);
}
