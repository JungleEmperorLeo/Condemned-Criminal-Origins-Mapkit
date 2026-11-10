#include "WorldPacker.cpp"
#include "NativePhysicsAutomaticSphere.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
static WorldModels::ClassKind Resolve(const PrefabScene::Node& node,void*){return FearBrushFlags::Equal(node.className,"WorldModel")?WorldModels::WorldModel:WorldModels::Ordinary;}
static PrefabScene::Node* Brush(PrefabScene::Model& model,PrefabScene::Node& owner,const char* type,unsigned int flags,const WorldText::Vec3* points,unsigned int count){
 PrefabScene::Node* node=model.Add();if(!node)return 0;node->kind=1;node->parent=owner.index;node->knownType=true;node->surfaceFlags=flags;lstrcpyA(node->type,type);
 node->geometry.points=count;node->geometry.polygons=1;node->geometry.references=count;
 node->geometry.vertices=(WorldText::Vec3*)model.memory.Allocate(count*sizeof(WorldText::Vec3));node->geometry.faces=(WorldText::Polygon*)model.memory.Allocate(sizeof(WorldText::Polygon));node->geometry.indices=(unsigned int*)model.memory.Allocate(count*4);if(!node->geometry.vertices||!node->geometry.faces||!node->geometry.indices)return 0;
 for(unsigned int i=0;i<count;i++){node->geometry.vertices[i].x=owner.position.x+points[i].x;node->geometry.vertices[i].y=owner.position.y+points[i].y;node->geometry.vertices[i].z=owner.position.z+points[i].z;node->geometry.indices[i]=i;}
 node->geometry.faces[0].count=count;node->geometry.faces[0].indices=node->geometry.indices;model.brushes++;model.points+=count;model.polygons++;return node;
}
static bool Empty(const NativePhysicsAutomaticSphere::Builder& value){return !value.polygons&&!value.polygonCount&&!value.references&&!value.pointCount&&!value.points&&!value.fitPoints&&value.body[0]==0&&value.body[1]==0&&value.body[2]==0;}
extern "C" void HarnessEntry(){
 unsigned int results[13]={};PrefabScene::Model model;PrefabScene::Node* owner=model.Add();if(!owner)ExitProcess(1);owner->kind=2;owner->position.x=1000;owner->position.y=-2000;owner->position.z=3000;lstrcpyA(owner->className,"WorldModel");model.objects++;
 const WorldText::Vec3 physical[3]={{-2,0,0},{2,0,0},{0,2,0}},render[3]={{100,0,0},{104,0,0},{100,4,0}},blocker[3]={{-100,-100,-100},{-99,-100,-100},{-100,-99,-100}},sector[3]={{10000,10000,10000},{10001,10000,10000},{10000,10001,10000}};
 PrefabScene::Node* collision=Brush(model,*owner,"Normal",0x79,physical,3);if(!collision||!Brush(model,*owner,"RenderOnly",0x60,render,3)||!Brush(model,*owner,"Blocker",0x100,blocker,3)||!Brush(model,*owner,"Sector",0,sector,3))ExitProcess(1);
 // An unreferenced source vertex must not contribute to the cached center.
 WorldText::Vec3* replaced=(WorldText::Vec3*)model.memory.Allocate(4*sizeof(WorldText::Vec3));if(!replaced)ExitProcess(1);for(unsigned int i=0;i<3;i++)replaced[i]=collision->geometry.vertices[i];replaced[3].x=replaced[3].y=replaced[3].z=1e20f;collision->geometry.vertices=replaced;collision->geometry.points=4;model.points++;
 WorldModels::Partition partition;if(!partition.Build(model,Resolve,0))ExitProcess(1);const WorldText::Vec3 offset={1024,-2048,4096};NativePhysicsAutomaticSphere::Builder value;
 if(!value.Build(model,partition,1,offset,-3,-2))ExitProcess(2);
 results[0]=value.cachedCenter.x==2&&value.cachedCenter.y==-48&&value.cachedCenter.z==-50;
 results[1]=value.pointCount==3&&value.points[0].x==-2&&value.points[1].x==2&&value.points[2].y==2;
 results[2]=value.translation.x==-2&&value.translation.y==48&&value.translation.z==50&&value.body[2]>2&&value.body[2]<2.0001f;
 results[3]=value.body[0]==.1f&&value.body[1]==.01f;
 collision->surfaceFlags=0x60;if(!value.Build(model,partition,1,offset,1,1.1f))ExitProcess(2);
 results[4]=value.pointCount==0&&value.body[2]==0&&value.translation.x==0&&value.translation.y==0&&value.translation.z==0&&value.cachedCenter.x==2&&value.cachedCenter.y==-48;
 collision->surfaceFlags=0x79;PrefabScene::Node* carver=Brush(model,*owner,"AINavMeshCarver",0,sector,3);if(!carver)ExitProcess(1);
 // Navigation removes multi-face carvers; a one-face carver is retained.
 WorldText::Polygon* carverFaces=(WorldText::Polygon*)model.memory.Allocate(2*sizeof(WorldText::Polygon));if(!carverFaces)ExitProcess(1);carverFaces[0]=carverFaces[1]=carver->geometry.faces[0];carver->geometry.faces=carverFaces;carver->geometry.polygons=2;carver->geometry.references=6;model.polygons++;
 if(!partition.Build(model,Resolve,0)||!value.Build(model,partition,1,offset,1,1.1f))ExitProcess(2);
 results[5]=value.cachedCenter.x>4900&&value.pointCount==3;
 PrefabScene::Node* navigation=model.Add();if(!navigation)ExitProcess(1);navigation->kind=2;lstrcpyA(navigation->className,"AINavMesh");model.objects++;if(!partition.Build(model,Resolve,0)||!value.Build(model,partition,1,offset,1,1.1f))ExitProcess(2);
 results[6]=value.cachedCenter.x==2&&value.cachedCenter.y==-48&&value.cachedCenter.z==-50;
 collision->geometry.indices[0]=4;results[7]=!value.Build(model,partition,1,offset,1,1.1f)&&value.error&&Empty(value);collision->geometry.indices[0]=0;
 if(!value.Build(model,partition,1,offset,1,1.1f))ExitProcess(2);partition.Clear();model.Clear();results[8]=value.pointCount==3&&value.points[0].x==-2&&value.points[1].x==2&&value.polygons[0].points[2].y==2&&value.body[2]>2;
 WorldText::Vec3 preparedPoints[3]={{1,0,0},{-1,0,0},{0,1,0}},center={7,8,9};NativePhysicsAutomaticSphere::Polygon prepared={preparedPoints,3,0x10};
 if(!value.BuildPrepared(&prepared,1,center,2,3))ExitProcess(2);preparedPoints[0].x=100;results[9]=value.points[0].x==1&&value.polygons[0].points[0].x==1&&value.translation.x==-7;
 union Bad {unsigned int bits;float value;};Bad nan;nan.bits=0x7fc00000u;results[10]=!value.BuildPrepared(&prepared,1,center,nan.value,3)&&value.error&&Empty(value);
 // Imported large contours must retain the two shared corners between chunks.
 owner=model.Add();if(!owner)ExitProcess(1);owner->kind=2;lstrcpyA(owner->className,"WorldModel");model.objects++;WorldText::Vec3 ring[31];
 for(unsigned int i=0;i<31;i++){float angle=(float)i*(6.283185307179586f/31);ring[i].x=PrefabScene::Matrix::Cos(angle)*10;ring[i].y=PrefabScene::Matrix::Sin(angle)*10;ring[i].z=0;}
 const WorldText::Vec3 zero={};if(!Brush(model,*owner,"Normal",0x79,ring,31)||!partition.Build(model,Resolve,0)||!value.Build(model,partition,1,zero,1,1.1f))ExitProcess(2);
 results[11]=value.polygonCount==2&&value.polygons[0].count==30&&value.polygons[1].count==3&&value.pointCount==33&&value.points[30].x==value.points[0].x&&value.points[30].y==value.points[0].y&&value.points[31].x==value.points[29].x&&value.points[31].y==value.points[29].y&&value.points[32].x==ring[30].x;
 prepared.count=2;results[12]=!value.BuildPrepared(&prepared,1,center,1,1)&&value.error&&Empty(value);
 char path[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_AUTOMATIC_SPHERE_GUARDS_OUTPUT",path,MAX_PATH))ExitProcess(3);HANDLE file=CreateFileA(path,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);DWORD done=0;if(file==INVALID_HANDLE_VALUE||!WriteFile(file,results,sizeof(results),&done,0)||done!=sizeof(results))ExitProcess(4);CloseHandle(file);for(unsigned int i=0;i<13;i++)if(!results[i])ExitProcess(5);ExitProcess(0);
}
