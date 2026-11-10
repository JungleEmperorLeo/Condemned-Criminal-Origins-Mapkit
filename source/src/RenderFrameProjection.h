#pragma once
namespace RenderFrameProjection {
// Original RVA 0x11e20 stores squared length, root and inverse to float.
inline bool Normalize(WorldText::Vec3& v){
 float squared=(float)((double(v.x)*v.x+double(v.y)*v.y)+double(v.z)*v.z);
 float length=(float)RenderAttributes::Root(squared);
 // A zero (exported zero texture axis) or NaN vector passes the original's
 // assertion: 1/0 = +inf, 0*inf = x87 indefinite; NaN propagates.
 if(length==0||length!=length){unsigned int indefinite=0xffc00000u;float inverse=length==0?1e30f*1e30f:length;
  for(unsigned int k=0;k<3;k++){float& c=(&v.x)[k];if(c==0&&length==0)memcpy(&c,&indefinite,4);else c=(float)(double(inverse)*c);}return true;}
 if(!WorldText::Finite(length)||length<0)return false;
 float inverse=(float)(1.0/double(length));v.x=(float)(double(inverse)*v.x);v.y=(float)(double(inverse)*v.y);v.z=(float)(double(inverse)*v.z);return WorldText::FiniteVector(v);
}
inline bool Axis(const WorldText::Vec3& input,const WorldText::Vec3& normal,WorldText::Vec3& output){
 WorldText::Vec3 v=input;if(!Normalize(v))return false;
 float d=(float)((double(normal.z)*v.z+double(normal.y)*v.y)+double(normal.x)*v.x);
 float x=(float)(double(d)*normal.x),y=(float)(double(d)*normal.y),z=(float)(double(d)*normal.z);
 v.x=(float)(double(v.x)-x);v.y=(float)(double(v.y)-y);v.z=(float)(double(v.z)-z);
 // Original zero-length projection continues after its assertion and produces
 // x87 indefinite NaNs. Preserve this working record: aggregation can discard
 // a singular Q axis when P still defines a finite final tangent frame.
 if(v.x==0&&v.y==0&&v.z==0){unsigned int indefinite=0xffc00000u;memcpy(&v.x,&indefinite,4);memcpy(&v.y,&indefinite,4);memcpy(&v.z,&indefinite,4);output=v;return true;}
 if(!Normalize(v))return false;output=v;return true;
}
inline bool Build(const WorldText::Vec3& p,const WorldText::Vec3& q,const WorldText::Vec3& normal,WorldText::Vec3& u,WorldText::Vec3& v){return Axis(p,normal,u)&&Axis(q,normal,v);}
}
