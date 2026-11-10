#include "WorldPacker.cpp"
#include "NativeHullLeafGraph.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
static void Guard(bool value,unsigned int number){if(!value)ExitProcess(100+number);}
extern "C" void HarnessEntry(){using namespace NativeHullLeafGraph;
 Vec4 p[4]={{0,0,0,0},{1,0,0,0},{2,0,0,0},{3,0,0,1}};unsigned int ids[4]={0,1,2,3};Leaf leaf;
 Guard(Build(0,0,0,0,&leaf)==Success&&leaf.count==0,1);
 Guard(Build(p,4,ids,1,&leaf)==Success&&leaf.count==1&&leaf.edges[0].origin==0&&leaf.edges[0].twin==0&&leaf.edges[0].next==0,2);
 Guard(Build(p,4,ids,2,&leaf)==Success&&leaf.count==2&&leaf.edges[0].twin==1&&leaf.edges[1].next==0,3);
 Guard(Build(p,4,ids,3,&leaf)==Success&&leaf.count==2&&leaf.edges[0].origin==2&&leaf.edges[1].origin==0,4);
 p[2].y=1;Guard(Build(p,4,ids,3,&leaf)==Success&&leaf.count==6,5);
 for(unsigned int e=0;e<leaf.count;e++){Guard(leaf.edges[e].state==0&&leaf.edges[e].twin<6&&leaf.edges[e].next<6,6);Guard(leaf.edges[leaf.edges[e].twin].twin==e,7);unsigned int next=leaf.edges[e].next;Guard(leaf.edges[leaf.edges[next].next].next==e,8);}
 Guard(Build(p,4,ids,4,&leaf)==TooManyActivePoints&&leaf.count==0,9);
 Guard(Build(p,4,ids,3,0)==InvalidInput,10);ids[2]=4;Guard(Build(p,4,ids,3,&leaf)==IndexOutOfProfile&&leaf.count==0,11);ids[2]=1;Guard(Build(p,4,ids,3,&leaf)==InvalidInput,12);ids[2]=2;
 Guard(Build(p,4,ids+3,1,&leaf)==InvalidInput,13);
 Guard(BuildRange(p,4,0,3,&leaf)==Success&&leaf.count==6,14);p[3].w=0;Guard(BuildRange(p,4,0,3,&leaf)==TooManyActivePoints&&leaf.count==0,15);p[3].w=1;
 Guard(BuildRange(p,4,3,3,&leaf)==Success&&leaf.count==0,16);Guard(BuildRange(p,4,3,2,&leaf)==InvalidInput,17);
 union B {float f;unsigned int u;};B bad;bad.u=0x7fc00000u;p[0].x=bad.f;Guard(Build(p,4,ids,3,&leaf)==InvalidInput&&leaf.count==0,18);p[0].x=0;bad.u=0x7f800000u;p[0].w=bad.f;Guard(BuildRange(p,4,0,2,&leaf)==InvalidInput,19);p[0].w=-0.0f;
 Guard(BuildRange(p,4,0,2,&leaf)==Success&&leaf.count==6,20);
 B huge;huge.u=0x7f7fffffu;p[0].x=huge.f;p[1].x=-huge.f;Guard(Build(p,4,ids,3,&leaf)==NonfiniteResult&&leaf.count==0,21);
 ExitProcess(0);
}
