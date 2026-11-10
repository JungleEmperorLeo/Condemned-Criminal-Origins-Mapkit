#pragma once
#include "PhysicsExplicitShapes.h"
#include "SourceOwnerRotation.h"
#include "NativePhysicsProperties.h"
namespace NativePhysicsExplicit {
inline void QuaternionProduct(const float* parent,const float* child,float* result){
 result[0]=(float)(((double(child[3])*parent[0]+double(parent[1])*child[2])+double(parent[3])*child[0])-double(parent[2])*child[1]);
 result[1]=(float)(((double(parent[3])*child[1]-double(parent[0])*child[2])+double(child[3])*parent[1])+double(parent[2])*child[0]);
 result[2]=(float)(((double(parent[3])*child[2]+double(child[1])*parent[0])-double(parent[1])*child[0])+double(child[3])*parent[2]);
 result[3]=(float)(((double(parent[3])*child[3]-double(child[0])*parent[0])-double(child[1])*parent[1])-double(parent[2])*child[2]);
}
inline bool SourceQuaternion(const PrefabScene::Node& node,float* result){WorldText::Vec3 angles;PrefabScene::Matrix matrix;return PhysicsExplicitShapes::Vector(node,"Rotation",angles)&&PrefabScene::Matrix::Euler(angles,matrix)&&SourceOwnerRotation::MatrixQuaternion(matrix,result);}
inline unsigned int PrefabDepth(const PrefabScene::Model& model,const PrefabScene::Node& node){unsigned int depth=0,parent=node.parent;while(parent!=0xffffffffu){const PrefabScene::Node* found=0;for(const PrefabScene::Node* n=model.first;n;n=n->next)if(n->index==parent){found=n;break;}if(!found)return 0xffffffffu;if(found->sourceKind==3)depth++;parent=found->parent;}return depth;}
inline bool Center(const WorldModels::Partition& partition,unsigned int group,const WorldText::Vec3& offset,WorldText::Vec3& center){
 if(!group||group>=partition.groupCount||!partition.groups[group].owner)return false;
 PhysicsOwnerBounds::Bounds bounds;const PrefabScene::Node& owner=*partition.groups[group].owner;
 for(unsigned int a=0;a<partition.assignmentCount;a++){const WorldModels::Assignment& item=partition.assignments[a];if(item.group!=group)continue;const WorldText::Geometry& geometry=item.brush->geometry;for(unsigned int v=0;v<geometry.points;v++){if(!PhysicsOwnerBounds::Referenced(geometry,v))continue;WorldText::Vec3 local;if(!SourceOwnerRotation::Local(owner,geometry.vertices[v],offset,local)||!bounds.Add(local))return false;}}
 if(!bounds.count)return false;center=bounds.Center();return true;
}
inline bool Transform(const PrefabScene::Model& model,const WorldModels::Partition& partition,unsigned int group,const PrefabScene::Node& shape,const WorldText::Vec3& offset,float* transform){
 const PrefabScene::Node& owner=*partition.groups[group].owner;WorldText::Vec3 position,center;
 if(!Center(partition,group,offset,center))return false;
 const unsigned int depth=PrefabDepth(model,owner);if(depth==0xffffffffu)return false;
 float inverse[4],rotation[4];
  // Original RVA 0x1BD10 returns no prototype for an authored owner (no prefab
  // ancestor). RVA 0x1BCB0 searches ordinary children of a loaded prototype,
  // without entering a nested prefab's loaded contents. Both cases therefore
  // use the expanded, centered scene pose; a direct prototype uses raw locals.
  if(depth!=1){if(!SourceOwnerRotation::Local(owner,shape.position,offset,position)||!SourceOwnerRotation::Inverse(owner,inverse)||!SourceOwnerRotation::Quaternion(shape,rotation))return false;}
 else{
  WorldText::Vec3 point,origin;if(!FearBrushFlags::Equal(owner.source,shape.source)||!PhysicsExplicitShapes::Vector(shape,"Pos",point)||!PhysicsExplicitShapes::Vector(owner,"Pos",origin)||!SourceQuaternion(owner,inverse)||!SourceQuaternion(shape,rotation))return false;
  inverse[0]=-inverse[0];inverse[1]=-inverse[1];inverse[2]=-inverse[2];WorldText::Vec3 a=SourceWorldModelTransform::Rotate(inverse,point),b=SourceWorldModelTransform::Rotate(inverse,origin);position.x=a.x-b.x;position.y=a.y-b.y;position.z=a.z-b.z;
 }
 transform[0]=position.x-center.x;transform[1]=position.y-center.y;transform[2]=position.z-center.z;
 QuaternionProduct(inverse,rotation,transform+3);for(unsigned int i=0;i<7;i++)if(!WorldText::Finite(transform[i]))return false;return true;
}
inline bool Build(const PrefabScene::Model& model,const WorldModels::Partition& partition,unsigned int group,const PrefabScene::Node& shape,const WorldText::Vec3& offset,unsigned int& kind,float* body,float* transform){
 float mass=0,density=0;if(!NativePhysicsProperties::MassDensity(shape,false,mass,density))return false;
 if(FearBrushFlags::Equal(shape.className,"PhysicsShapeOBB")){kind=2;WorldText::Vec3 dims,fallback={100,100,100};if(!NativePhysicsProperties::Vector(shape,"Dims",fallback,dims)||!PhysicsPrimitives::OBB(mass,density,dims,body))return false;return Transform(model,partition,group,shape,offset,transform);}
 if(FearBrushFlags::Equal(shape.className,"PhysicsShapeSphere")){kind=3;float radius=0;if(!NativePhysicsProperties::Number(shape,"Radius",100,radius)||!PhysicsPrimitives::Sphere(mass,density,radius,body))return false;return Transform(model,partition,group,shape,offset,transform);}
 if(FearBrushFlags::Equal(shape.className,"PhysicsShapeCapsule")){
  kind=7;float radius=0,length=0;if(!NativePhysicsProperties::Number(shape,"Radius",100,radius)||!NativePhysicsProperties::Number(shape,"Length",100,length)||radius<=0||length<0||mass<0||density<0||!Transform(model,partition,group,shape,offset,transform))return false;
  // Up(q)*h and the endpoint sums stay at x87 precision; only endpoints are
  // stored as float (Streets screwdriver: unrounded 1-2zz gives the original bits).
  const double x=transform[3],y=transform[4],z=transform[5],w=transform[6],half=length*.5f;const double direction[3]={2*(x*y-z*w),1-2*(x*x+z*z),2*(y*z+x*w)};
  body[0]=mass;body[1]=density;body[2]=radius;for(unsigned int k=0;k<3;k++){body[3+k]=(float)(transform[k]+direction[k]*half);body[6+k]=(float)(transform[k]-direction[k]*half);}for(unsigned int k=0;k<9;k++)if(!WorldText::Finite(body[k]))return false;return true;
 }
 return false;
}
}
