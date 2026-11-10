#define FEAR_PRIMARY_SURFACE_PREPARATION 1
#include "WorldPacker.cpp"
#include "SourceBlockerPolygons.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
static WorldModels::ClassKind Ordinary(const PrefabScene::Node&,void*){return WorldModels::Ordinary;}
static bool Empty(const SourceBlockerPolygons::Block& b){return b.data&&b.size==8&&!b.count&&!b.vertices&&!*(unsigned int*)b.data&&!*(unsigned int*)(b.data+4);}
static bool Failed(const SourceBlockerPolygons::Block& b,SourceBlockerPolygons::Code c){return !b.data&&!b.size&&!b.count&&!b.vertices&&b.code==c&&b.error;}
static bool Equal(const unsigned char* a,const unsigned char* b,unsigned int n){while(n--)if(*a++!=*b++)return false;return true;}
static void Require(bool condition,unsigned int number){if(!condition)ExitProcess(number);}
struct GuardState {PrefabScene::Model model;WorldModels::Partition partition;SourceBlockerPolygons::Block block;};
extern "C" void HarnessEntry(){
 PrefabScene::Arena scratch;void* memory=scratch.Allocate(sizeof(GuardState));Require(memory!=0,1);GuardState& s=*new(memory)GuardState;WorldText::Vec3 offset={};
 Require(s.partition.Build(s.model,Ordinary,0)&&s.block.Build(s.model,s.partition,offset)&&Empty(s.block),2);
 PrefabScene::Node* node=s.model.Add();Require(node!=0,3);node->kind=1;node->knownType=true;node->surfaceFlags=0x102;lstrcpyA(node->type,"Blocker");
 s.model.brushes=1;s.model.polygons=1;s.model.points=4;WorldText::Geometry& geometry=node->geometry;geometry.points=4;geometry.polygons=1;geometry.references=4;
 WorldText::Vec3 points[4]={{0,10,0},{10,10,0},{10,10,10},{0,10,10}};unsigned int indices[4]={0,1,2,3};WorldText::Polygon polygon={};polygon.count=4;polygon.indices=indices;geometry.vertices=points;geometry.faces=&polygon;
 Require(s.partition.Build(s.model,Ordinary,0)&&s.block.Build(s.model,s.partition,offset)&&s.block.size==76&&s.block.count==1&&s.block.vertices==4,4);
 unsigned char original[76];memcpy(original,s.block.data,76);node->hidden=node->frozen=node->effectiveHidden=node->effectiveFrozen=true;
 Require(s.block.Build(s.model,s.partition,offset)&&Equal(s.block.data,original,76),5);
 node->surfaceFlags=0x82;Require(s.block.Build(s.model,s.partition,offset)&&Empty(s.block),6);node->surfaceFlags=0x102;
 offset.x=2;offset.y=3;offset.z=4;Require(s.block.Build(s.model,s.partition,offset),7);
 const float* plane=(const float*)(s.block.data+8);const WorldText::Vec3* first=(const WorldText::Vec3*)(s.block.data+28);
 Require(plane[0]==0&&plane[1]==-1&&plane[2]==0&&plane[3]==-7&&first->x==-2&&first->y==7&&first->z==-4,8);offset.x=offset.y=offset.z=0;
 union {unsigned int bits;float value;} infinity;infinity.bits=0x7f800000u;offset.x=infinity.value;Require(!s.block.Build(s.model,s.partition,offset)&&Failed(s.block,SourceBlockerPolygons::InvalidInput),9);offset.x=0;
 indices[3]=4;Require(!s.block.Build(s.model,s.partition,offset)&&Failed(s.block,SourceBlockerPolygons::InvalidPolygon),10);indices[3]=3;
 points[2]=points[1];points[3]=points[0];Require(!s.block.Build(s.model,s.partition,offset)&&Failed(s.block,SourceBlockerPolygons::InvalidPolygon),11);points[2].x=10;points[2].y=10;points[2].z=10;points[3].x=0;points[3].y=10;points[3].z=10;
 geometry.faces=0;Require(!s.block.Build(s.model,s.partition,offset)&&Failed(s.block,SourceBlockerPolygons::InvalidPolygon),12);geometry.faces=&polygon;
 geometry.polygons=100001;Require(!s.block.Build(s.model,s.partition,offset)&&Failed(s.block,SourceBlockerPolygons::Capacity),13);geometry.polygons=1;
 geometry.points=1000001;Require(!s.block.Build(s.model,s.partition,offset)&&Failed(s.block,SourceBlockerPolygons::Capacity),14);geometry.points=4;
 polygon.count=1000001;Require(!s.block.Build(s.model,s.partition,offset)&&Failed(s.block,SourceBlockerPolygons::Capacity),15);polygon.count=4;
 s.partition.assignmentCount=2;Require(!s.block.Build(s.model,s.partition,offset)&&Failed(s.block,SourceBlockerPolygons::InvalidInput),16);s.partition.assignmentCount=1;
 s.partition.assignments[0].group=1;Require(!s.block.Build(s.model,s.partition,offset)&&Failed(s.block,SourceBlockerPolygons::InvalidInput),17);s.partition.assignments[0].group=0;
 Require(s.block.Build(s.model,s.partition,offset)&&Equal(s.block.data,original,76),18);
 s.block.Clear();Require(!s.block.data&&!s.block.size&&!s.block.error&&s.block.code==SourceBlockerPolygons::Ok,19);
 s.~GuardState();ExitProcess(0);
}
