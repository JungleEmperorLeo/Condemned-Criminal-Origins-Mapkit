#include "WorldPacker.cpp"
#include "StreamingConvex.h"
#include "StreamingTreeBuilder.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
extern "C" void HarnessEntry(){
 char input[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_READER_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_TREE_OUTPUT",output,MAX_PATH))ExitProcess(1);
 PrefabScene::Source source;if(!source.Load(input))ExitProcess(2);void* convexStorage=VirtualAlloc(0,sizeof(PrefabScene::Expansion),MEM_COMMIT|MEM_RESERVE,4);if(!convexStorage)ExitProcess(3);PrefabScene::Expansion* scene=new(convexStorage) PrefabScene::Expansion;if(!scene->Build(input,source.scene))ExitProcess(4);
 StreamingConvex::Block block;if(!block.Build(scene->Scene()))ExitProcess(5);StreamingTreeBuilder::Builder tree;if(!tree.Build(block.bounds,block.volumeCount))ExitProcess(7);HANDLE f=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);DWORD done;unsigned int header[]={tree.nodeCount,block.volumeCount,block.polygonCount,block.vertexCount};if(f==INVALID_HANDLE_VALUE||!WriteFile(f,header,16,&done,0)||done!=16||!WriteFile(f,&block.offset,12,&done,0)||done!=12||!WriteFile(f,tree.block.data,tree.block.size,&done,0)||done!=tree.block.size)ExitProcess(6);CloseHandle(f);ExitProcess(0);
}


