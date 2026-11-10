#pragma once
#include "WorldModels.h"
#include "RenderSector.h"
// Explicit experimental class/sector profile shared by visible and shadow paths.
static WorldModels::ClassKind Resolve(const PrefabScene::Node& n,void*){
 // FEAR Source/Game/ObjectDLL: these derived classes explicitly carry CF_WORLDMODEL.
 const char* owners[]={"WorldModel","SlidingDoor","RotatingDoor","SlidingSwitch"};for(unsigned int i=0;i<4;i++)if(FearBrushFlags::Equal(n.className,owners[i]))return WorldModels::WorldModel;
 const char* ordinary[]={"LightPoint","WorldProperties","GameStartPoint","PhysicsShapeCapsule","PhysicsShapeOBB","KeyFramer","Key"};for(unsigned int i=0;i<7;i++)if(FearBrushFlags::Equal(n.className,ordinary[i]))return WorldModels::Ordinary;return WorldModels::Unknown;
}
static bool Assigned(const PrefabScene::Model& model,const WorldText::Geometry& g,const WorldText::Polygon& face){
 double p[3]={};for(unsigned int i=0;i<face.count;i++){const WorldText::Vec3& v=g.vertices[face.indices[i]];p[0]+=v.x;p[1]+=v.y;p[2]+=v.z;}
 WorldText::Vec3 probe={(float)(p[0]/face.count+face.normal.x*.01),(float)(p[1]/face.count+face.normal.y*.01),(float)(p[2]/face.count+face.normal.z*.01)};
 for(const PrefabScene::Node* n=model.first;n;n=n->next)if(n->kind==1&&FearBrushFlags::Equal(n->type,"Sector")&&RenderSector::Contains(n->geometry,probe))return true;return false;
}
