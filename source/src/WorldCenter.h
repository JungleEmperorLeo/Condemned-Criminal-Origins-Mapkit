#pragma once
#include "PhysicsOwnerBounds.h"
namespace WorldCenter {
inline bool Eligible(unsigned int flags){return !(flags&2)&&(flags&0x18);}
inline bool Snap(float coordinate,float& output){
 if(!WorldText::Finite(coordinate)||coordinate>2000000000.f||coordinate< -2000000000.f)return false;int integer;
 __asm movss xmm0,coordinate
 __asm cvttss2si eax,xmm0
 __asm mov integer,eax
 integer=((integer+128)/256)*256;output=(float)integer;return WorldText::Finite(output);
}
inline bool Compute(const PrefabScene::Model& model,WorldText::Vec3& offset,unsigned int* included=0){
 if(model.uncentered){offset.x=offset.y=offset.z=0;if(included)*included=0;return true;}
 // Original unknown Type fallback (RVA 0x1C022) returns zero base flags.
 // Ineligible brushes must not prevent centering of otherwise valid geometry.
 PhysicsOwnerBounds::Bounds bounds;unsigned int brushes=0;for(const PrefabScene::Node* n=model.first;n;n=n->next){if(n->kind!=1)continue;if(!Eligible(n->surfaceFlags))continue;if(!n->knownType)return false;brushes++;for(unsigned int i=0;i<n->geometry.points;i++)if(!bounds.Add(n->geometry.vertices[i]))return false;}
 if(included)*included=brushes;if(!bounds.count){offset.x=offset.y=offset.z=0;return true;}WorldText::Vec3 center=bounds.Center();return Snap(center.x,offset.x)&&Snap(center.y,offset.y)&&Snap(center.z,offset.z);
}
}
