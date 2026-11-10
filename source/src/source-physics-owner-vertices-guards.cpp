#include "WorldPacker.cpp"
#include "SourcePhysicsOwnerVertices.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
static WorldModels::ClassKind Resolve(const PrefabScene::Node& node,void*){return FearBrushFlags::Equal(node.className,"WorldModel")?WorldModels::WorldModel:WorldModels::Ordinary;}
static PrefabScene::Node* Brush(PrefabScene::Model& model,PrefabScene::Node* owner,const char* type,unsigned int flags,const WorldText::Vec3* points,unsigned int count,const WorldText::Vec3& origin){
 PrefabScene::Node* node=model.Add();if(!node)return 0;node->kind=1;node->parent=owner?owner->index:0xffffffffu;node->knownType=true;node->surfaceFlags=flags;lstrcpyA(node->type,type);
 node->geometry.points=count;node->geometry.polygons=1;node->geometry.references=count;
 node->geometry.vertices=(WorldText::Vec3*)model.memory.Allocate(count*12);node->geometry.faces=(WorldText::Polygon*)model.memory.Allocate(sizeof(WorldText::Polygon));node->geometry.indices=(unsigned int*)model.memory.Allocate(count*4);if(!node->geometry.vertices||!node->geometry.faces||!node->geometry.indices)return 0;
 for(unsigned int i=0;i<count;i++){node->geometry.vertices[i].x=origin.x+points[i].x;node->geometry.vertices[i].y=origin.y+points[i].y;node->geometry.vertices[i].z=origin.z+points[i].z;node->geometry.indices[i]=i;}
 node->geometry.faces[0].count=count;node->geometry.faces[0].indices=node->geometry.indices;model.brushes++;model.points+=count;model.polygons++;return node;
}
static float Bits(unsigned int bits){union Pair {unsigned int bits;float value;};Pair result;result.bits=bits;return result.value;}
static bool Empty(const SourcePhysicsOwnerVertices::Builder& result){return !result.polygons&&!result.polygonCount&&!result.pointCount&&!result.rawVertices&&!result.scaledVertices&&!result.cornerVertexIndices&&!result.cornerProvenance&&!result.representativeFirstCorner&&!result.indices&&!result.indexCount;}
extern "C" void HarnessEntry(){
 using namespace SourcePhysicsOwnerVertices;
 unsigned int results[20]={};Builder value;Settings settings;settings.generateIndices=true;WorldText::Vec3 center={7,8,9},zero={};
 WorldText::Vec3 threshold[3]={{0,0,0},{.01f,.01f,.01f},{Bits(0x3c23d70bu),0,0}};Polygon polygon={threshold,3,0x10,0,0,0,0,0xffffffffu};
 if(!value.BuildPrepared(&polygon,1,zero,true,settings))ExitProcess(1);
 results[0]=value.pointCount==2&&value.cornerVertexIndices[0]==0&&value.cornerVertexIndices[1]==0&&value.cornerVertexIndices[2]==1&&value.rawVertices[0].x==0&&value.rawVertices[1].x==threshold[2].x;
 results[1]=value.indexCount==3&&value.indices[0]==0&&value.indices[1]==0&&value.indices[2]==1;
 WorldText::Vec3 beforeScale[3]={{0,0,0},{.02f,0,0},{0,.02f,0}};polygon.points=beforeScale;if(!value.BuildPrepared(&polygon,1,zero,true,settings))ExitProcess(1);
 results[2]=value.pointCount==3&&value.scaledVertices[1].x<.01f&&value.scaledVertices[2].y<.01f;
 polygon.points=threshold;if(!value.BuildPrepared(&polygon,1,center,true,settings))ExitProcess(1);results[3]=value.mainWorldFlag==1&&value.rawVertices[0].x==0&&value.cachedCenter.x==7;
 WorldText::Vec3 ordinary[3]={{8,8,9},{7,9,9},{7,8,10}};polygon.points=ordinary;if(!value.BuildPrepared(&polygon,1,center,false,settings))ExitProcess(1);results[4]=value.mainWorldFlag==0&&value.rawVertices[0].x==1&&value.rawVertices[0].y==0&&value.rawVertices[1].y==1;
 ordinary[0].x=100;results[5]=value.rawVertices[0].x==1&&value.polygons[0].points[0].x==8;
 WorldText::Vec3 dummy[31];for(unsigned int i=0;i<31;i++){dummy[i].x=(float)i;dummy[i].y=dummy[i].z=0;}Polygon skipped[3]={{0,0,0x10,0,0,0,0,0},{dummy,2,0x10,0,0,0,0,0},{dummy,31,0x10,0,0,0,0,0}};
 if(!value.BuildPrepared(skipped,3,zero,true,settings))ExitProcess(1);results[6]=value.pointCount==0&&value.indexCount==0&&value.skippedPreparedPolygons==3&&value.physicsReferences==0;
 if(!value.BuildPrepared(0,0,zero,true,settings))ExitProcess(1);results[7]=value.pointCount==0&&value.indexCount==0&&value.errorCode==NoError;
 Settings invalid=settings;invalid.maximumUniqueVertices=65536;results[8]=!value.BuildPrepared(0,0,zero,true,invalid)&&value.errorCode==InvalidSettings&&Empty(value);
 invalid=settings;invalid.maximumUniqueVertices=1;polygon.points=beforeScale;results[9]=!value.BuildPrepared(&polygon,1,zero,true,invalid)&&value.errorCode==ResourceLimit&&Empty(value);
 beforeScale[0].x=Bits(0x7fc00000u);results[10]=!value.BuildPrepared(&polygon,1,zero,true,settings)&&value.errorCode==NonfinitePoint&&Empty(value);beforeScale[0].x=0;
 PrefabScene::Model model;PrefabScene::Node* owner=model.Add();if(!owner)ExitProcess(1);owner->kind=2;owner->position.x=1000;owner->position.y=-2000;owner->position.z=3000;lstrcpyA(owner->className,"WorldModel");model.objects++;
 const WorldText::Vec3 physical[3]={{-2,0,0},{2,0,0},{0,2,0}},render[3]={{100,0,0},{104,0,0},{100,4,0}},blocker[3]={{-100,-100,-100},{-99,-100,-100},{-100,-99,-100}},sector[3]={{10000,10000,10000},{10001,10000,10000},{10000,10001,10000}};
 PrefabScene::Node* collision=Brush(model,owner,"Normal",0x79,physical,3,owner->position);if(!collision||!Brush(model,owner,"RenderOnly",0x60,render,3,owner->position)||!Brush(model,owner,"Blocker",0x100,blocker,3,owner->position)||!Brush(model,owner,"Sector",0,sector,3,owner->position))ExitProcess(1);
 WorldText::Vec3* expanded=(WorldText::Vec3*)model.memory.Allocate(4*12);if(!expanded)ExitProcess(1);for(unsigned int i=0;i<3;i++)expanded[i]=collision->geometry.vertices[i];expanded[3].x=expanded[3].y=expanded[3].z=1e20f;collision->geometry.vertices=expanded;collision->geometry.points=4;model.points++;
 WorldModels::Partition partition;if(!partition.Build(model,Resolve,0))ExitProcess(1);const WorldText::Vec3 offset={1024,-2048,4096};if(!value.Build(model,partition,1,offset,settings))ExitProcess(1);
 results[11]=value.cachedCenter.x==2&&value.cachedCenter.y==-48&&value.cachedCenter.z==-50&&value.pointCount==3&&value.removedBlockerPolygons==1&&value.nonphysicalPolygons==1&&value.rawVertices[0].x==-4&&value.rawVertices[0].y==48&&value.rawVertices[0].z==50;
 results[12]=value.cornerProvenance[0].assignment==0&&value.cornerProvenance[0].node==collision->index&&value.cornerProvenance[0].sourcePolygon==0&&value.cornerProvenance[0].fragment==0&&value.cornerProvenance[0].sourceCorner==0&&value.cornerProvenance[0].sourceVertex==0&&value.representativeFirstCorner[1]==1;
 collision->geometry.indices[0]=4;results[13]=!value.Build(model,partition,1,offset,settings)&&value.errorCode==InvalidGeometry&&value.failedAssignment==0&&value.failedPolygon==0&&Empty(value);collision->geometry.indices[0]=0;
 if(!value.Build(model,partition,1,offset,settings))ExitProcess(1);partition.Clear();model.Clear();results[14]=value.pointCount==3&&value.rawVertices[0].x==-4&&value.cornerProvenance[2].sourceVertex==2&&value.polygons[0].points[2].y==2;
 const WorldText::Vec3 mainOrigin={1050,-2020,4130};if(!Brush(model,0,"Normal",0x79,physical,3,mainOrigin)||!partition.Build(model,Resolve,0)||!value.Build(model,partition,0,offset,settings))ExitProcess(1);
 results[15]=value.mainWorldFlag==1&&value.cachedCenter.x==26&&value.rawVertices[0].x==24&&value.rawVertices[0].y==28&&value.rawVertices[0].z==34;
 partition.Clear();model.Clear();owner=model.Add();if(!owner)ExitProcess(1);owner->kind=2;lstrcpyA(owner->className,"WorldModel");model.objects++;WorldText::Vec3 ring[31];for(unsigned int i=0;i<31;i++){float angle=(float)i*(6.283185307179586f/31);ring[i].x=PrefabScene::Matrix::Cos(angle)*10;ring[i].y=PrefabScene::Matrix::Sin(angle)*10;ring[i].z=0;}
 if(!Brush(model,owner,"Normal",0x79,ring,31,zero)||!partition.Build(model,Resolve,0)||!value.Build(model,partition,1,zero,settings))ExitProcess(1);
 results[16]=value.polygonCount==2&&value.polygons[0].count==30&&value.polygons[1].count==3&&value.physicsReferences==33&&value.pointCount==31&&value.indexCount==87&&value.cornerProvenance[30].sourceCorner==0&&value.cornerProvenance[31].sourceCorner==29&&value.cornerProvenance[32].sourceCorner==30&&value.cornerVertexIndices[30]==value.cornerVertexIndices[0]&&value.cornerVertexIndices[31]==value.cornerVertexIndices[29];
 results[17]=value.indices[84]==value.cornerVertexIndices[0]&&value.indices[85]==value.cornerVertexIndices[29]&&value.indices[86]==value.cornerVertexIndices[32];
 const WorldText::Vec3 nonfiniteOffset={Bits(0x7f800000u),0,0};results[18]=!value.Build(model,partition,1,nonfiniteOffset,settings)&&value.errorCode==InvalidInput&&Empty(value);
 polygon.points=beforeScale;polygon.flags=0x110;results[19]=value.BuildPrepared(&polygon,1,zero,true,settings)&&value.pointCount==3&&value.indexCount==3&&value.removedBlockerPolygons==0;
 char path[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_OWNER_VERTICES_GUARDS_OUTPUT",path,MAX_PATH))ExitProcess(2);HANDLE file=CreateFileA(path,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);DWORD done=0;if(file==INVALID_HANDLE_VALUE||!WriteFile(file,results,sizeof(results),&done,0)||done!=sizeof(results))ExitProcess(3);CloseHandle(file);for(unsigned int i=0;i<20;i++)if(!results[i])ExitProcess(4);ExitProcess(0);
}
