#include "WorldPacker.cpp"
#include "StreamingBlock.h"
#include "StreamingTables.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
static char path[MAX_PATH];static unsigned char* input;static unsigned int bytes,cursor;static PrefabScene::Arena* arena;
static unsigned int U(){if(cursor>bytes||bytes-cursor<4)ExitProcess(10);unsigned int v=*(unsigned int*)(input+cursor);cursor+=4;return v;}
static const char* S(){unsigned int start=cursor;while(cursor<bytes&&input[cursor])cursor++;if(cursor==bytes||cursor-start>4096)ExitProcess(11);cursor++;return (const char*)(input+start);}
static StreamingBlock::Asset* A(unsigned int n){if(n>100000)ExitProcess(12);StreamingBlock::Asset* a=(StreamingBlock::Asset*)arena->Allocate((n+1)*sizeof(StreamingBlock::Asset));if(!a)ExitProcess(13);for(unsigned int i=0;i<n;i++){a[i].folder=S();a[i].file=S();}return a;}
extern "C" void HarnessEntry(){ PrefabScene::Arena localArena;arena=&localArena;
 if(!GetEnvironmentVariableA("FEAR_TABLE_INPUT",path,MAX_PATH))ExitProcess(1);HANDLE f=CreateFileA(path,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);if(f==INVALID_HANDLE_VALUE)ExitProcess(2);bytes=GetFileSize(f,0);if(bytes<12||bytes>16*1024*1024)ExitProcess(3);input=(unsigned char*)VirtualAlloc(0,bytes,MEM_COMMIT|MEM_RESERVE,4);DWORD done;if(!input||!ReadFile(f,input,bytes,&done,0)||done!=bytes)ExitProcess(4);CloseHandle(f);
 unsigned int convex=U(),globalCount=U();StreamingBlock::Asset* global=A(globalCount);unsigned int count=U();if(count>100000)ExitProcess(12);StreamingTables::Region* regions=(StreamingTables::Region*)arena->Allocate((count+1)*sizeof(StreamingTables::Region));if(!regions)ExitProcess(13);
 for(unsigned int i=0;i<count;i++){StreamingTables::Region& r=regions[i];r.name=S();r.budget=U();r.referenceCount=U();if(r.referenceCount>100000)ExitProcess(12);unsigned int* ids=(unsigned int*)arena->Allocate((r.referenceCount+1)*4);if(!ids)ExitProcess(13);for(unsigned int j=0;j<r.referenceCount;j++)ids[j]=U();r.references=ids;r.assetCount=U();r.assets=A(r.assetCount);}
 StreamingTables::Tables tables;if(cursor!=bytes||!tables.Build(global,globalCount,regions,count,convex))ExitProcess(20);
 if(!GetEnvironmentVariableA("FEAR_TABLE_OUTPUT",path,MAX_PATH))ExitProcess(1);f=CreateFileA(path,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);unsigned int header[]={tables.poolBytes,tables.totalAssetReferences,tables.aggregate};if(f==INVALID_HANDLE_VALUE||!WriteFile(f,header,12,&done,0)||done!=12||!WriteFile(f,tables.data,tables.size,&done,0)||done!=tables.size)ExitProcess(5);CloseHandle(f);ExitProcess(0);
}

