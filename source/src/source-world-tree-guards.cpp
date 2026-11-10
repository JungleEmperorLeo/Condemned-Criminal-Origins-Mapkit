#define FEAR_PRIMARY_SURFACE_PREPARATION 1
#include "WorldPacker.cpp"
#include "SourceWorldTree.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
static bool Equal(const unsigned char* a,const unsigned char* b,unsigned int bytes){for(unsigned int i=0;i<bytes;i++)if(a[i]!=b[i])return false;return true;}
extern "C" void HarnessEntry(){
 PrefabScene::Model model;PrefabScene::Node* brush=model.Add();if(!brush)ExitProcess(1);brush->kind=1;brush->knownType=true;brush->surfaceFlags=0x79;lstrcpyA(brush->type,"Normal");model.brushes=1;
 WorldText::Vec3 points[4]={{-1024,0,-1024},{-1024,0,1024},{1024,0,1024},{1024,0,-1024}};unsigned int indices[4]={0,1,2,3};WorldText::Polygon face={};face.count=4;face.indices=indices;face.normal.y=1;face.validPlane=true;
 brush->geometry.points=4;brush->geometry.vertices=points;brush->geometry.polygons=1;brush->geometry.faces=&face;
 WorldModels::Partition partition;WorldModels::Group group={};group.ownerIndex=0xffffffffu;group.brushes=1;group.polygons=1;WorldModels::Assignment assignment={brush,0};partition.groups=&group;partition.groupCount=1;partition.assignments=&assignment;partition.assignmentCount=1;
 WorldText::Vec3 offset={};SourceWorldTree::Builder tree;unsigned int checks=0;
 if(!tree.Build(model,partition,offset)||tree.nodeCount!=21)ExitProcess(2);checks++;unsigned char saved[64];unsigned int size=tree.size;memcpy(saved,tree.data,size);
 brush->surfaceFlags=0x100;lstrcpyA(brush->type,"Blocker");if(!tree.Build(model,partition,offset)||tree.nodeCount!=1||!Equal(tree.data,saved,24))ExitProcess(3);checks++;
 brush->surfaceFlags=0x1b;lstrcpyA(brush->type,"SkyPortal");if(!tree.Build(model,partition,offset)||tree.nodeCount!=1||!Equal(tree.data,saved,24))ExitProcess(4);checks++;
 brush->surfaceFlags=0x60;lstrcpyA(brush->type,"RenderOnly");if(!tree.Build(model,partition,offset)||tree.nodeCount!=1||!Equal(tree.data,saved,24))ExitProcess(5);checks++;
 brush->surfaceFlags=0x79;lstrcpyA(brush->type,"Normal");if(!tree.Build(model,partition,offset,2048)||tree.nodeCount!=1)ExitProcess(6);checks++;
 if(tree.Build(model,partition,offset,1000,4)||tree.data||tree.nodeCount||!tree.error)ExitProcess(7);checks++;
 if(tree.Build(model,partition,offset,0)||tree.data||!tree.error)ExitProcess(8);checks++;
 unsigned int invalid=0x7fc00000;float nan;memcpy(&nan,&invalid,4);offset.x=nan;if(tree.Build(model,partition,offset)||tree.data||!tree.error)ExitProcess(9);checks++;offset.x=0;
 indices[3]=4;if(tree.Build(model,partition,offset)||tree.data||!tree.error)ExitProcess(10);checks++;indices[3]=3;
 assignment.group=1;if(tree.Build(model,partition,offset)||tree.data||!tree.error)ExitProcess(11);checks++;assignment.group=0;
 if(!tree.Build(model,partition,offset)||tree.size!=size||!Equal(tree.data,saved,size))ExitProcess(12);checks++;
 // Source data no longer participates after Build, including point positions.
 points[0].x=0;if(!Equal(tree.data,saved,size))ExitProcess(13);checks++;
 tree.Clear();if(tree.data||tree.size||tree.nodeCount||tree.error)ExitProcess(14);checks++;
 partition.groups=0;partition.assignments=0;partition.groupCount=partition.assignmentCount=0;
 char output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_WORLD_TREE_GUARDS_OUTPUT",output,MAX_PATH))ExitProcess(15);
 HANDLE file=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);DWORD done=0;if(file==INVALID_HANDLE_VALUE||!WriteFile(file,&checks,4,&done,0)||done!=4)ExitProcess(16);CloseHandle(file);ExitProcess(0);
}
