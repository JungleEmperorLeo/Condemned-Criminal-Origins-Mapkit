#pragma once
#include "PhysicsPrimitives.h"
#include "WorldModels.h"
#ifdef FEAR_QUAT_COORD_EXPERIMENT
#include "PhysicsQuaternion.h"
#endif
namespace PhysicsOwnerBounds {
struct Bounds {
 WorldText::Vec3 low,high;unsigned int count;
 Bounds():count(0){}
 bool Add(const WorldText::Vec3& p){if(!WorldText::FiniteVector(p))return false;if(!count){low=high=p;}else{if(p.x<low.x)low.x=p.x;if(p.y<low.y)low.y=p.y;if(p.z<low.z)low.z=p.z;if(p.x>high.x)high.x=p.x;if(p.y>high.y)high.y=p.y;if(p.z>high.z)high.z=p.z;}count++;return true;}
 WorldText::Vec3 Center()const{WorldText::Vec3 r={(low.x+high.x)*.5f,(low.y+high.y)*.5f,(low.z+high.z)*.5f};return r;}
 WorldText::Vec3 Half()const{WorldText::Vec3 r={(high.x-low.x)*.5f,(high.y-low.y)*.5f,(high.z-low.z)*.5f};return r;}
};
inline bool LocalPoint(const float* matrix,const WorldText::Vec3& pos,const WorldText::Vec3& world,WorldText::Vec3& local){
 if(!WorldText::FiniteVector(pos)||!WorldText::FiniteVector(world))return false;for(unsigned int i=0;i<12;i++)if(!WorldText::Finite(matrix[i]))return false;
 #ifdef FEAR_QUAT_COORD_EXPERIMENT
 char mode[4]={};if(GetEnvironmentVariableA("FEAR_OWNER_QUATERNION_EXPERIMENT",mode,4)&&mode[0]>='1'&&mode[0]<='4')return PhysicsQuaternion::Local(matrix,pos,world,mode[0],local);
 #endif
 float dx=world.x-pos.x,dy=world.y-pos.y,dz=world.z-pos.z;
 local.x=matrix[0]*dx+matrix[4]*dy+matrix[8]*dz;local.y=matrix[1]*dx+matrix[5]*dy+matrix[9]*dz;local.z=matrix[2]*dx+matrix[6]*dy+matrix[10]*dz;return WorldText::FiniteVector(local);
}
inline bool ScenePoint(const float* matrix,const WorldText::Vec3& pos,const WorldText::Vec3& world,WorldText::Vec3& local,const WorldText::Vec3* offset){
 if(!offset)return LocalPoint(matrix,pos,world,local);if(!WorldText::FiniteVector(*offset))return false;WorldText::Vec3 shiftedPos={pos.x-offset->x,pos.y-offset->y,pos.z-offset->z},shiftedWorld={world.x-offset->x,world.y-offset->y,world.z-offset->z};return LocalPoint(matrix,shiftedPos,shiftedWorld,local);
}
inline bool Referenced(const WorldText::Geometry& geometry,unsigned int point){for(unsigned int i=0;i<geometry.polygons;i++)for(unsigned int j=0;j<geometry.faces[i].count;j++)if(geometry.faces[i].indices[j]==point)return true;return false;}
inline bool Finish(const Bounds& all,const Bounds& physics,float mass,float density,float* body,WorldText::Vec3& translation){
 if(!all.count||!physics.count)return false;WorldText::Vec3 half=physics.Half();if(!PhysicsPrimitives::OBB(mass,density,half,body))return false;WorldText::Vec3 a=all.Center(),p=physics.Center();translation.x=p.x-a.x;translation.y=p.y-a.y;translation.z=p.z-a.z;return WorldText::FiniteVector(translation);
}
inline bool BuildGroup(const WorldModels::Partition& partition,unsigned int group,float mass,float density,float* body,WorldText::Vec3& translation,const WorldText::Vec3* offset=0){
 if(!group||group>=partition.groupCount||!partition.groups[group].owner)return false;const PrefabScene::Node& owner=*partition.groups[group].owner;Bounds all,physics;
 for(unsigned int i=0;i<partition.assignmentCount;i++){const WorldModels::Assignment& a=partition.assignments[i];if(a.group!=group)continue;if(!a.brush->knownType)return false;for(unsigned int j=0;j<a.brush->geometry.points;j++){if(!Referenced(a.brush->geometry,j))continue;WorldText::Vec3 local;if(!ScenePoint(&owner.orientation.m[0][0],owner.position,a.brush->geometry.vertices[j],local,offset)||!all.Add(local))return false;if((a.brush->surfaceFlags&0x10)&&!physics.Add(local))return false;}}
 return Finish(all,physics,mass,density,body,translation);
}
inline bool CapsuleGroup(const WorldModels::Partition& partition,unsigned int group,const PrefabScene::Node& capsule,float mass,float density,float radius,float length,float* body,const WorldText::Vec3* offset=0,const WorldText::Vec3* sourceLocalPosition=0){
 if(!group||group>=partition.groupCount||!partition.groups[group].owner||!WorldText::Finite(mass)||!WorldText::Finite(density)||!WorldText::Finite(radius)||!WorldText::Finite(length)||mass<0||density<0||radius<=0||length<0)return false;
 const PrefabScene::Node& owner=*partition.groups[group].owner;Bounds all;
 for(unsigned int i=0;i<partition.assignmentCount;i++){const WorldModels::Assignment& a=partition.assignments[i];if(a.group!=group)continue;for(unsigned int j=0;j<a.brush->geometry.points;j++){if(!Referenced(a.brush->geometry,j))continue;WorldText::Vec3 p;if(!ScenePoint(&owner.orientation.m[0][0],owner.position,a.brush->geometry.vertices[j],p,offset)||!all.Add(p))return false;}}
 if(!all.count)return false;WorldText::Vec3 center,model=all.Center();if(sourceLocalPosition){center=*sourceLocalPosition;if(!WorldText::FiniteVector(center))return false;}else if(!ScenePoint(&owner.orientation.m[0][0],owner.position,capsule.position,center,offset))return false;center.x-=model.x;center.y-=model.y;center.z-=model.z;
 WorldText::Vec3 axis={capsule.orientation.m[0][1],capsule.orientation.m[1][1],capsule.orientation.m[2][1]},direction;WorldText::Vec3 zero={};if(!LocalPoint(&owner.orientation.m[0][0],zero,axis,direction))return false;float h=length*.5f;
 body[0]=mass;body[1]=density;body[2]=radius;body[3]=center.x+direction.x*h;body[4]=center.y+direction.y*h;body[5]=center.z+direction.z*h;body[6]=center.x-direction.x*h;body[7]=center.y-direction.y*h;body[8]=center.z-direction.z*h;for(unsigned int i=0;i<9;i++)if(!WorldText::Finite(body[i]))return false;return true;
}
}
