#include "WorldPacker.cpp"
#include "StreamingBlock.h"
#include "StreamingTables.h"
#include "AssetList.h"
#include "StreamingResources.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
static char path[MAX_PATH];static unsigned char* input;static unsigned int bytes,cursor;
static unsigned int U(){if(cursor>bytes||bytes-cursor<4)ExitProcess(10);unsigned int v=*(unsigned int*)(input+cursor);cursor+=4;return v;}
static const char* S(){unsigned int start=cursor;while(cursor<bytes&&input[cursor])cursor++;if(cursor==bytes||cursor-start>4096)ExitProcess(11);cursor++;return (const char*)(input+start);}
extern "C" void HarnessEntry(){
 PrefabScene::Arena arena;
 if(!GetEnvironmentVariableA("FEAR_RESOURCE_INPUT",path,MAX_PATH))ExitProcess(1);HANDLE f=CreateFileA(path,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);if(f==INVALID_HANDLE_VALUE)ExitProcess(2);bytes=GetFileSize(f,0);if(bytes<12||bytes>16*1024*1024)ExitProcess(3);input=(unsigned char*)VirtualAlloc(0,bytes,MEM_COMMIT|MEM_RESERVE,4);DWORD done;if(!input||!ReadFile(f,input,bytes,&done,0)||done!=bytes)ExitProcess(4);CloseHandle(f);
 const char* stem=S();unsigned int count=U();if(count>10000)ExitProcess(12);const char** names=(const char**)arena.Allocate((count+1)*sizeof(char*));unsigned int* budgets=(unsigned int*)arena.Allocate((count+1)*4);if(!names||!budgets)ExitProcess(13);for(unsigned int i=0;i<count;i++){names[i]=S();budgets[i]=U();}
 unsigned int seedCount=U();if(seedCount>100000)ExitProcess(12);StreamingResources::Seed* seeds=(StreamingResources::Seed*)arena.Allocate((seedCount+1)*sizeof(StreamingResources::Seed));if(!seeds)ExitProcess(13);for(unsigned int i=0;i<seedCount;i++){seeds[i].region=U();seeds[i].path=S();}
 unsigned int textBytes=U();if(textBytes>bytes-cursor||cursor+textBytes!=bytes)ExitProcess(14);AssetList::List list;if(!list.Read(input+cursor,textBytes,names,count))ExitProcess(20);
 StreamingResources::Sets sets;if(!sets.Build(stem,names,budgets,count,seeds,seedCount,list))ExitProcess(21);StreamingTables::Tables tables;if(!tables.Build(sets.global,sets.globalCount,sets.regions,count,0))ExitProcess(22);
 if(!GetEnvironmentVariableA("FEAR_RESOURCE_OUTPUT",path,MAX_PATH))ExitProcess(1);f=CreateFileA(path,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);unsigned int header[]={tables.poolBytes,tables.totalAssetReferences,tables.aggregate,list.warnings};if(f==INVALID_HANDLE_VALUE||!WriteFile(f,header,16,&done,0)||done!=16||!WriteFile(f,tables.data,tables.size,&done,0)||done!=tables.size)ExitProcess(5);CloseHandle(f);ExitProcess(0);
}
