#include "WorldPacker.cpp"
#include "RenderVisibilityPortalGraph.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
extern "C" void HarnessEntry(){
 char input[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_PORTAL_GRAPH_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_PORTAL_GRAPH_OUTPUT",output,MAX_PATH))ExitProcess(1);HANDLE file=CreateFileA(input,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);if(file==INVALID_HANDLE_VALUE)ExitProcess(2);DWORD bytes=GetFileSize(file,0),done=0;PrefabScene::Arena memory;unsigned char* data=(unsigned char*)memory.Allocate(bytes+1);if(!data||bytes>16*1024*1024||!ReadFile(file,data,bytes,&done,0)||done!=bytes)ExitProcess(3);CloseHandle(file);
 RenderVisibilityPortalGraph::Graph graph;if(!graph.Build(data,bytes))ExitProcess(4);HANDLE out=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);unsigned int header[3]={0x31475056,graph.count,graph.edgeCount};if(out==INVALID_HANDLE_VALUE||!WriteFile(out,header,12,&done,0)||done!=12||!WriteFile(out,graph.nodes,graph.count*8,&done,0)||done!=graph.count*8||!WriteFile(out,graph.neighbors,graph.edgeCount*4,&done,0)||done!=graph.edgeCount*4)ExitProcess(5);CloseHandle(out);ExitProcess(0);
}
