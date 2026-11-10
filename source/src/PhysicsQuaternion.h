#pragma once
#include "ObjectsBlock.h"
namespace PhysicsQuaternion {
// Quaternion/vector product order follows original RVA 0x100067f0.
inline WorldText::Vec3 Rotate(const float* q,const WorldText::Vec3& p){
 float x=q[1]*p.z+q[3]*p.x-q[2]*p.y;
 float y=q[3]*p.y-q[0]*p.z+q[2]*p.x;
 float z=p.y*q[0]+q[3]*p.z-q[1]*p.x;
 float w=-p.x*q[0]-p.y*q[1]-p.z*q[2];
 WorldText::Vec3 r={-q[0]*w-q[2]*y+q[3]*x+q[1]*z,-q[1]*w+q[2]*x+q[3]*y-q[0]*z,-q[2]*w-q[1]*x+q[0]*y+q[3]*z};return r;
}
inline void Matrix(const float* q,PrefabScene::Matrix& m){float x=q[0],y=q[1],z=q[2],w=q[3];m.m[0][0]=1-2*(y*y+z*z);m.m[0][1]=2*(x*y-z*w);m.m[0][2]=2*(x*z+y*w);m.m[1][0]=2*(x*y+z*w);m.m[1][1]=1-2*(x*x+z*z);m.m[1][2]=2*(y*z-x*w);m.m[2][0]=2*(x*z-y*w);m.m[2][1]=2*(y*z+x*w);m.m[2][2]=1-2*(x*x+y*y);}
inline bool Local(const float* matrix,const WorldText::Vec3& pos,const WorldText::Vec3& point,char mode,WorldText::Vec3& result){PrefabScene::Matrix m;for(unsigned int i=0;i<3;i++)for(unsigned int j=0;j<4;j++)m.m[i][j]=matrix[i*4+j];float q[4];if(mode=='2'||mode=='4'){if(!ObjectsBlock::PrefabRotationReplay(m,q))return false;}else if(!ObjectsBlock::Quaternion(m,q))return false;
 WorldText::Vec3 d={point.x-pos.x,point.y-pos.y,point.z-pos.z};if(mode=='3'||mode=='4'){q[0]=-q[0];q[1]=-q[1];q[2]=-q[2];result=Rotate(q,d);}else{PrefabScene::Matrix qm;Matrix(q,qm);result.x=qm.m[0][0]*d.x+qm.m[1][0]*d.y+qm.m[2][0]*d.z;result.y=qm.m[0][1]*d.x+qm.m[1][1]*d.y+qm.m[2][1]*d.z;result.z=qm.m[0][2]*d.x+qm.m[1][2]*d.y+qm.m[2][2]*d.z;}return WorldText::FiniteVector(result);
}
}
