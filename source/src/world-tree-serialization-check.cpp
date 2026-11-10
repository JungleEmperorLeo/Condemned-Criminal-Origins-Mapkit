#include "WorldPacker.cpp"
#include "WorldTreeSerialization.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
static bool Read(HANDLE file,void* target,unsigned int size){DWORD done=0;return !size||(ReadFile(file,target,size,&done,0)&&done==size);}
extern "C" void HarnessEntry(){
 char input[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_WORLD_TREE_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_WORLD_TREE_OUTPUT",output,MAX_PATH))ExitProcess(1);
 HANDLE file=CreateFileA(input,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);WorldText::Vec3 bounds[2];unsigned int counts[2];
 if(file==INVALID_HANDLE_VALUE||!Read(file,bounds,24)||!Read(file,counts,8)||counts[0]>WorldTreeSerialization::MaximumNodes)ExitProcess(2);
 PrefabScene::Arena memory;WorldTreeSerialization::Node* nodes=(WorldTreeSerialization::Node*)memory.Allocate((counts[0]+1)*sizeof(WorldTreeSerialization::Node));if(!nodes||!Read(file,nodes,counts[0]*16))ExitProcess(3);unsigned char tail;DWORD done=0;if(!ReadFile(file,&tail,1,&done,0)||done)ExitProcess(3);CloseHandle(file);
 WorldTreeSerialization::Block block;if(!block.Build(bounds[0],bounds[1],nodes,counts[0],counts[1])){LogLine(block.error);ExitProcess(4);}
 file=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(file==INVALID_HANDLE_VALUE||!WriteFile(file,block.data,block.size,&done,0)||done!=block.size)ExitProcess(5);CloseHandle(file);ExitProcess(0);
}
