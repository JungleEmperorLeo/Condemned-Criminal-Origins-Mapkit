#pragma once
#include "ObjectsBlock.h"
#include "SourceSurfaceRotation.h"
namespace SourceWorldModelTransform {
// Original RVA 0x67F0: two quaternion products with float stores between them.
inline WorldText::Vec3 Rotate(const float* q,const WorldText::Vec3& p){return SourceSurfaceRotation::Rotate(q,p);}
// Main diagonal path recovered on current finite owner controls.
// Rotation sweep is exact after original LTA decimal parsing is selected.
inline bool Quaternion(const PrefabScene::Matrix& m,float* q){
 float diag=(float)((double(m.m[0][0])+m.m[1][1])+m.m[2][2]),s;
 if(diag<-.999f)return ObjectsBlock::Quaternion(m,q);
 s=ObjectsBlock::Root((float)(double(diag)+1));if(s<=0)return false;q[3]=s*.5f;s=.5f/s;
 q[0]=(float)((double(m.m[2][1])-m.m[1][2])*s);q[1]=(float)((double(m.m[0][2])-m.m[2][0])*s);q[2]=(float)((double(m.m[1][0])-m.m[0][1])*s);return true;
}
inline bool Local(const PrefabScene::Node& owner,const WorldText::Vec3& world,WorldText::Vec3& local){
 float q[4];if(!Quaternion(owner.orientation,q))return false;
 q[0]=-q[0];q[1]=-q[1];q[2]=-q[2];{WorldText::Vec3 a=Rotate(q,world),b=Rotate(q,owner.position);local.x=a.x-b.x;local.y=a.y-b.y;local.z=a.z-b.z;}return WorldText::FiniteVector(local);
}
}
