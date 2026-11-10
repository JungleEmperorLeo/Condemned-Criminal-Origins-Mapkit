#pragma once
namespace SourceNormalArithmetic {
inline double DotWide(const WorldText::Vec3& a,const WorldText::Vec3& b){return (double(a.y)*b.y+double(a.z)*b.z)+double(a.x)*b.x;}
inline bool Normalize(WorldText::Vec3& v){
 float squared=(float)((double(v.x)*v.x+double(v.y)*v.y)+double(v.z)*v.z);
 if(!WorldText::Finite(squared)||squared<=0)return false;
 float length=WorldText::Sqrt(squared);float inverse=(float)(1.0/double(length));
 v.x=(float)(double(v.x)*inverse);v.y=(float)(double(v.y)*inverse);v.z=(float)(double(v.z)*inverse);
 return WorldText::FiniteVector(v);
}
inline bool Sum(const WorldText::Vec3& face,float cosine,const WorldText::Vec3* adjacent,unsigned int count,WorldText::Vec3& output){
 if(!adjacent||!count||!WorldText::FiniteVector(face)||!WorldText::Finite(cosine))return false;
 WorldText::Vec3 result={};for(unsigned int i=0;i<count;i++){
  if(!WorldText::FiniteVector(adjacent[i]))return false;
  if(DotWide(face,adjacent[i])<cosine)continue;
  result.x=(float)(double(result.x)+adjacent[i].x);result.y=(float)(double(result.y)+adjacent[i].y);result.z=(float)(double(result.z)+adjacent[i].z);
 }
 if(!Normalize(result))return false;output=result;return true;
}
}
