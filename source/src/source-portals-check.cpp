#include "WorldPacker.cpp"
#include "StreamingConvex.h"
#include "VisibilityTree.h"
#include "SectorPortals.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
extern "C" void HarnessEntry(){
 char input[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_READER_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_TREE_OUTPUT",output,MAX_PATH))ExitProcess(1);
 HANDLE in=CreateFileA(input,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);DWORD read;unsigned int count=0;if(in==INVALID_HANDLE_VALUE||!ReadFile(in,&count,4,&read,0)||read!=4||!count||count>50000)ExitProcess(2);StreamingSplit::Bounds* bounds=(StreamingSplit::Bounds*)VirtualAlloc(0,count*sizeof(StreamingSplit::Bounds),MEM_COMMIT|MEM_RESERVE,4);if(!bounds||!ReadFile(in,bounds,count*sizeof(StreamingSplit::Bounds),&read,0)||read!=count*sizeof(StreamingSplit::Bounds))ExitProcess(3);CloseHandle(in);VisibilityTree::Builder tree;if(!tree.Build(bounds,count))ExitProcess(7);SectorPortals::AxisAligned portals;if(!portals.Build(bounds,count,tree.saveIds))ExitProcess(8);HANDLE f=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);DWORD done;if(f==INVALID_HANDLE_VALUE||!WriteFile(f,&portals.count,4,&done,0)||done!=4||!WriteFile(f,portals.portals,portals.count*sizeof(SectorPortals::Portal),&done,0)||done!=portals.count*sizeof(SectorPortals::Portal))ExitProcess(6);CloseHandle(f);ExitProcess(0);
}
