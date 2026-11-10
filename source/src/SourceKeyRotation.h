#pragma once
#include "SourceOwnerRotation.h"
// FEAR ConvertKeyData's Euler constructor is distinct from the authored
// gr_SetupMatrixEuler path. This header owns its arithmetic, without DLL calls.
namespace SourceKeyRotation {
inline void Clear(float* q){if(q)for(unsigned int i=0;i<4;i++)q[i]=0;}
inline float Product(float a,float b){float r;__asm fld a
 __asm fmul b
 __asm fstp r
 return r;}
inline float AddProducts(float a,float b,float c,float d){float r;__asm fld a
 __asm fmul b
 __asm fld c
 __asm fmul d
 __asm faddp st(1),st(0)
 __asm fstp r
 return r;}
inline float SubProducts(float a,float b,float c,float d){float r;__asm fld a
 __asm fmul b
 __asm fld c
 __asm fmul d
 __asm fsubp st(1),st(0)
 __asm fstp r
 return r;}
// RVA 0xC940: six float-stored trig values; rs*ps and rc*ps are also
// float-stored. The remaining two-product sums/subtractions are stored once.
// In particular m01 and m20 use subtraction, preserving identity +0 results.
inline bool EulerQuaternion(const WorldText::Vec3& angles,float* q){
 if(!q)return false;Clear(q);
 if(!WorldText::FiniteVector(angles)||angles.x< -1000000||angles.x>1000000||angles.y< -1000000||angles.y>1000000||angles.z< -1000000||angles.z>1000000)return false;
 float yc=PrefabScene::Matrix::Cos(angles.y),ys=PrefabScene::Matrix::Sin(angles.y);
 float pc=PrefabScene::Matrix::Cos(angles.x),ps=PrefabScene::Matrix::Sin(angles.x);
 float rc=PrefabScene::Matrix::Cos(angles.z),rs=PrefabScene::Matrix::Sin(angles.z);
 float rsps=Product(rs,ps),rcps=Product(rc,ps);PrefabScene::Matrix m;
 m.m[0][0]=AddProducts(rsps,ys,rc,yc);m.m[0][1]=SubProducts(rcps,ys,rs,yc);m.m[0][2]=Product(pc,ys);
 m.m[1][0]=Product(rs,pc);m.m[1][1]=Product(rc,pc);m.m[1][2]=-ps;
 m.m[2][0]=SubProducts(rsps,yc,rc,ys);m.m[2][1]=AddProducts(rcps,yc,rs,ys);m.m[2][2]=Product(pc,yc);
 float result[4];if(!SourceOwnerRotation::MatrixQuaternion(m,result))return false;
 for(unsigned int i=0;i<4;i++)q[i]=result[i];return true;
}
// Same generic WorldNode::Rotate preparation as SourceOwnerRotation. This
// produces the prepared Euler triple so the Key-specific constructor follows.
inline bool PreparedEuler(const PrefabScene::Node& node,WorldText::Vec3& angles){
 angles.x=angles.y=angles.z=0;
 if(!WorldText::FiniteVector(node.position))return false;
 if(!node.importedFromPrefab){
  unsigned int seen=0;for(const PrefabScene::Property* p=node.properties;p;p=p->next){
   if(seen++==4096)return false;if(!FearBrushFlags::Equal(p->name,"Rotation"))continue;
   if(!FearBrushFlags::Equal(p->type,"rotation")||!p->data)return false;
   unsigned int length=0;while(length<=4095&&p->data[length])length++;if(length>4095)return false;
   WorldText::Document doc;if(!doc.Parse(p->data,length))return false;
   unsigned int form=doc.First(0),inner=doc.Next(doc.First(form));
   return inner<doc.count&&doc.tokens[inner].list&&doc.Vector(inner,angles)&&WorldText::FiniteVector(angles);
  }return true;
 }
 PrefabScene::Matrix prepared=node.orientation;
 for(unsigned int r=0;r<3;r++)for(unsigned int c=0;c<4;c++)if(!WorldText::Finite(prepared.m[r][c]))return false;
 volatile float bottom=0;
 for(unsigned int r=0;r<3;r++)for(unsigned int c=0;c<3;c++)prepared.m[r][c]=(float)(double(prepared.m[r][c])+double(node.orientation.m[r][3])*bottom);
 angles.y=ObjectsBlock::Atan2(prepared.m[0][2],prepared.m[2][2]);
 float base=ObjectsBlock::Root((float)(double(prepared.m[0][2])*prepared.m[0][2]+double(prepared.m[2][2])*prepared.m[2][2]));
 angles.x=-ObjectsBlock::Atan2(prepared.m[1][2],base);
 PrefabScene::Matrix pitch,yaw;WorldText::Vec3 axisX={1,0,0},axisY={0,1,0};
 if(!SourceOwnerRotation::AxisRotation(axisX,-angles.x,pitch)||!SourceOwnerRotation::AxisRotation(axisY,-angles.y,yaw))return false;
 // Explicit wide arithmetic matches the primary preparation profile even if
 // this header is included by an independently configured diagnostic harness.
 PrefabScene::Matrix reverse;for(unsigned int r=0;r<3;r++){
  for(unsigned int c=0;c<3;c++)reverse.m[r][c]=(float)((double(pitch.m[r][0])*yaw.m[0][c]+double(pitch.m[r][1])*yaw.m[1][c])+double(pitch.m[r][2])*yaw.m[2][c]);
  reverse.m[r][3]=(float)(((double(pitch.m[r][0])*yaw.m[0][3]+double(pitch.m[r][1])*yaw.m[1][3])+double(pitch.m[r][2])*yaw.m[2][3])+pitch.m[r][3]);
 }
 WorldText::Vec3 up={prepared.m[0][1],prepared.m[1][1],prepared.m[2][1]},local;
 for(unsigned int r=0;r<3;r++)(&local.x)[r]=(float)(((double(reverse.m[r][0])*up.x+double(reverse.m[r][1])*up.y)+double(reverse.m[r][2])*up.z)+reverse.m[r][3]);
 angles.z=-ObjectsBlock::Atan2(local.x,local.y);
 return WorldText::FiniteVector(angles);
}
inline bool Quaternion(const PrefabScene::Node& node,float* q){
 if(!q)return false;Clear(q);WorldText::Vec3 angles;
 return PreparedEuler(node,angles)&&EulerQuaternion(angles,q);
}
}
