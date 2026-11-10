#include "WorldPacker.cpp"
#include "FearBspTree.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
extern "C" void HarnessEntry(){
 WorldText::Vec3 p[4]={{0,-1,-1},{0,-1,1},{0,1,1},{0,1,-1}};
 FearBspTree::Input inputs[2]={};inputs[0].polygon.normal.x=1;inputs[0].polygon.points=p;inputs[0].polygon.count=4;inputs[0].polygon.flags=8;inputs[0].source=41;inputs[1]=inputs[0];inputs[1].source=42;
 FearBspTree::Builder tree;unsigned int tests=0;
 if(!tree.Build(inputs,2)||tree.nodeCount!=2||tree.root!=0||tree.first->polygon->source!=41||tree.first->positive!=1||tree.first->negative!=-2||tree.last->positive!=-1||tree.last->negative!=-2)ExitProcess(1);tests++;
 p[0].y=17;if(tree.first->polygon->polygon.points[0].y!=-1)ExitProcess(2);p[0].y=-1;tests++;
 if(tree.Build(inputs,2,1)||tree.root!=-2||tree.nodeCount||tree.first||!tree.error)ExitProcess(3);tests++;
 if(!tree.Build(inputs,2))ExitProcess(4);tests++;
 if(tree.Build(inputs,2,128,1)||tree.root!=-2||tree.nodeCount||!tree.error)ExitProcess(5);tests++;
 if(tree.Build(inputs,2,128,100000,7)||tree.root!=-2||!tree.error)ExitProcess(6);tests++;
 inputs[0].polygon.flags=inputs[1].polygon.flags=0;if(tree.Build(inputs,2)||tree.root!=-2||!tree.error)ExitProcess(7);tests++;
 inputs[0].polygon.flags=8;inputs[0].polygon.normal.x=0;if(tree.Build(inputs,1)||!tree.error)ExitProcess(8);tests++;
 inputs[0].polygon.normal.x=1;union {unsigned int bits;float f;} bad;bad.bits=0x7f800000;p[0].x=bad.f;if(tree.Build(inputs,1)||!tree.error)ExitProcess(9);p[0].x=0;tests++;
 if(tree.Build(0,1)||tree.Build(inputs,0)||tree.Build(inputs,1,0)||tree.Build(inputs,1,4097))ExitProcess(10);tests++;
 if(!tree.Build(inputs,1)||tree.root!=0||tree.nodeCount!=1)ExitProcess(11);tests++;
 char path[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_BSP_POLICY_OUTPUT",path,MAX_PATH))ExitProcess(12);HANDLE out=CreateFileA(path,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);DWORD done;if(out==INVALID_HANDLE_VALUE||!WriteFile(out,&tests,4,&done,0)||done!=4)ExitProcess(13);CloseHandle(out);ExitProcess(0);
}
