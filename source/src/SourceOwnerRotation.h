#pragma once
#include "SourceWorldModelTransform.h"
#include "RenderExceptionalUV.h"
namespace SourceOwnerRotation {
// Owner preparation has a wider negative-diagonal conversion than the raw
// SourceWorldModelTransform primitive. Keep that primitive's API unchanged.
inline bool MatrixQuaternion(const PrefabScene::Matrix& m,float* q){
 if(!q)return false;
 for(unsigned int r=0;r<3;r++)for(unsigned int c=0;c<3;c++)if(!WorldText::Finite(m.m[r][c]))return false;
 float diag=(float)((double(m.m[0][0])+m.m[1][1])+m.m[2][2]),s;
 if(diag<-.999f){
  unsigned int i=0;if(m.m[1][1]>m.m[0][0])i=1;if(m.m[2][2]>m.m[i][i])i=2;
  unsigned int j=(i+1)%3,k=(j+1)%3;
  s=ObjectsBlock::Root((float)((double(m.m[i][i])-(double(m.m[j][j])+m.m[k][k]))+1));
  if(s<=0)return false;q[i]=s*.5f;s=.5f/s;
  q[3]=(float)((double(m.m[k][j])-m.m[j][k])*s);
  q[j]=(float)((double(m.m[j][i])+m.m[i][j])*s);
  q[k]=(float)((double(m.m[k][i])+m.m[i][k])*s);
 }else {
  s=ObjectsBlock::Root((float)(double(diag)+1));if(s<=0)return false;q[3]=s*.5f;s=.5f/s;
  q[0]=(float)((double(m.m[2][1])-m.m[1][2])*s);
  q[1]=(float)((double(m.m[0][2])-m.m[2][0])*s);
  q[2]=(float)((double(m.m[1][0])-m.m[0][1])*s);
 }
 return WorldText::Finite(q[0])&&WorldText::Finite(q[1])&&WorldText::Finite(q[2])&&WorldText::Finite(q[3]);
}
// Original LTMatrix::SetupRot, RVA 0x5FCB0. Euler setup is not interchangeable
// with the axis-angle inverse matrices used while recovering imported angles.
inline bool AxisRotation(const WorldText::Vec3& axis,float angle,PrefabScene::Matrix& m){
 if(!WorldText::FiniteVector(axis)||!WorldText::Finite(angle)||angle< -1000000||angle>1000000)return false;
 float s=PrefabScene::Matrix::Sin(angle),c=PrefabScene::Matrix::Cos(angle),t=1.f-c;
 float tx=t*axis.x,ty=t*axis.y,tz=t*axis.z,sx=s*axis.x,sy=s*axis.y,sz=s*axis.z;
 m.m[0][0]=(float)(double(tx)*axis.x+c);m.m[1][0]=(float)(double(tx)*axis.y+sz);m.m[2][0]=(float)(double(tx)*axis.z-sy);
 m.m[0][1]=(float)(double(tx)*axis.y-sz);m.m[1][1]=(float)(double(ty)*axis.y+c);m.m[2][1]=(float)(double(ty)*axis.z+sx);
 m.m[0][2]=(float)(double(tx)*axis.z+sy);m.m[1][2]=(float)(double(ty)*axis.z-sx);m.m[2][2]=(float)(double(tz)*axis.z+c);
 m.m[0][3]=m.m[1][3]=m.m[2][3]=0;
 for(unsigned int r=0;r<3;r++)for(unsigned int col=0;col<3;col++)if(!WorldText::Finite(m.m[r][col]))return false;
 return true;
}
// Imported object WorldNode::Rotate recovers Euler angles before rebuilding
// its matrix. Authored owners retain their directly constructed matrix.
inline bool Quaternion(const PrefabScene::Node& owner,float* q){
 if(!WorldText::FiniteVector(owner.position))return false;
 if(!owner.importedFromPrefab)return MatrixQuaternion(owner.orientation,q);
 PrefabScene::Matrix prepared=owner.orientation;
 for(unsigned int r=0;r<3;r++)for(unsigned int c=0;c<4;c++)if(!WorldText::Finite(prepared.m[r][c]))return false;
 // WorldNode::Rotate uses a 4x4 product, RVA 0x2D420. The owned affine matrix
 // has already accumulated its three direction terms; replay the fourth
 // translation*+0 term before recovering Euler angles. Retain its zero sign.
 volatile float bottom=0;
 for(unsigned int r=0;r<3;r++)for(unsigned int c=0;c<3;c++)
  prepared.m[r][c]=(float)(double(prepared.m[r][c])+double(owner.orientation.m[r][3])*bottom);
 const PrefabScene::Matrix& m=prepared;
 WorldText::Vec3 angles;angles.y=ObjectsBlock::Atan2(m.m[0][2],m.m[2][2]);
 float base=ObjectsBlock::Root((float)(double(m.m[0][2])*m.m[0][2]+double(m.m[2][2])*m.m[2][2]));
 angles.x=-ObjectsBlock::Atan2(m.m[1][2],base);
 PrefabScene::Matrix pitch,yaw;WorldText::Vec3 axisX={1,0,0},axisY={0,1,0};
 if(!AxisRotation(axisX,-angles.x,pitch)||!AxisRotation(axisY,-angles.y,yaw))return false;
 PrefabScene::Matrix reverse=pitch.Times(yaw);WorldText::Vec3 up={m.m[0][1],m.m[1][1],m.m[2][1]};
 up=reverse.Apply(up);angles.z=-ObjectsBlock::Atan2(up.x,up.y);
 PrefabScene::Matrix reconstructed;
 return PrefabScene::Matrix::Euler(angles,reconstructed)&&MatrixQuaternion(reconstructed,q);
}
inline bool Inverse(const PrefabScene::Node& owner,float* q){
 if(!Quaternion(owner,q))return false;q[0]=-q[0];q[1]=-q[1];q[2]=-q[2];return true;
}
// Center both operands with float stores before the two quaternion products.
// Rotate(point)-Rotate(owner) preserves original arithmetic and zero signs.
inline bool LocalArithmetic(const PrefabScene::Node& owner,const WorldText::Vec3& world,const WorldText::Vec3& offset,WorldText::Vec3& local){
 if(!WorldText::FiniteVector(offset))return false;
 float inverse[4];if(!Inverse(owner,inverse))return false;
 WorldText::Vec3 centeredWorld=world,centeredOwner=owner.position;
 centeredWorld.x-=offset.x;centeredWorld.y-=offset.y;centeredWorld.z-=offset.z;
 centeredOwner.x-=offset.x;centeredOwner.y-=offset.y;centeredOwner.z-=offset.z;
 WorldText::Vec3 a=SourceWorldModelTransform::Rotate(inverse,centeredWorld),b=SourceWorldModelTransform::Rotate(inverse,centeredOwner);
 local.x=a.x-b.x;local.y=a.y-b.y;local.z=a.z-b.z;
 return true;
}
inline bool Local(const PrefabScene::Node& owner,const WorldText::Vec3& world,const WorldText::Vec3& offset,WorldText::Vec3& local){
 return WorldText::FiniteVector(world)&&LocalArithmetic(owner,world,offset,local)&&WorldText::FiniteVector(local);
}
// Texture origins use the same centered inverse arithmetic as positions, with
// an explicit source policy for quiet NaNs. Geometry callers stay finite-only.
inline bool LocalTextureOrigin(const PrefabScene::Node& owner,const WorldText::Vec3& world,const WorldText::Vec3& offset,WorldText::Vec3& local,bool allowExceptional){
 return RenderExceptionalUV::Origin(world,allowExceptional)&&LocalArithmetic(owner,world,offset,local)&&RenderExceptionalUV::Origin(local,allowExceptional);
}
}
