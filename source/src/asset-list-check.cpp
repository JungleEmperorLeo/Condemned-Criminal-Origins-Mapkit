#include "WorldPacker.cpp"
#include "AssetList.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
static char path[MAX_PATH];
extern "C" void HarnessEntry(){
 if(!GetEnvironmentVariableA("FEAR_ASSET_INPUT",path,MAX_PATH))ExitProcess(1);HANDLE f=CreateFileA(path,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);if(f==INVALID_HANDLE_VALUE)ExitProcess(2);unsigned int size=GetFileSize(f,0);if(!size||size>16*1024*1024)ExitProcess(3);unsigned char* data=(unsigned char*)VirtualAlloc(0,size,MEM_COMMIT|MEM_RESERVE,4);DWORD done;if(!data||!ReadFile(f,data,size,&done,0)||done!=size)ExitProcess(4);CloseHandle(f);
 const char* regions[]={"RegionA","RegionB"};AssetList::List list;if(!list.Read(data,size,regions,2))ExitProcess(20);
 if(!GetEnvironmentVariableA("FEAR_ASSET_OUTPUT",path,MAX_PATH))ExitProcess(1);f=CreateFileA(path,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(f==INVALID_HANDLE_VALUE)ExitProcess(5);unsigned int header[]={list.count,list.warnings};if(!WriteFile(f,header,8,&done,0)||done!=8)ExitProcess(6);
 for(unsigned int i=0;i<list.count;i++){unsigned int length=0;while(list.entries[i].path[length])length++;length++;if(!WriteFile(f,&list.entries[i].region,4,&done,0)||done!=4||!WriteFile(f,list.entries[i].path,length,&done,0)||done!=length)ExitProcess(7);}CloseHandle(f);ExitProcess(0);
}
