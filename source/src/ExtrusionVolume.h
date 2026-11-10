#pragma once
namespace ExtrusionVolume {
struct Plane {float x,y,z,d;};
inline float Length(float x,float y,float z){float squared=(float)(double(x)*x+double(y)*y+double(z)*z),result;__asm fld squared
 __asm fsqrt
 __asm fstp result
 return result;
}
inline bool Normalize(WorldText::Vec3& v){
 float length=Length(v.x,v.y,v.z);if(!WorldText::Finite(length)||length<=0)return false;
 float reciprocal=(float)(1.0/double(length));v.x=(float)(double(reciprocal)*v.x);v.y=(float)(double(reciprocal)*v.y);v.z=(float)(double(reciprocal)*v.z);return WorldText::FiniteVector(v);
}
// Recovered RVA 0x14431..0x145E6 plane construction for a cleaned contour.
// Edge/collinear cleanup in RVA 0x14260 is not performed by this helper.
inline bool ComputeInternalPlane(const WorldText::Vec3* points,unsigned int count,Plane& plane){
 if(!points||count<3||count>128)return false;
 for(unsigned int j=0;j<count;j++)if(!WorldText::FiniteVector(points[j]))return false;
 WorldText::Vec3 first={points[1].x-points[0].x,points[1].y-points[0].y,points[1].z-points[0].z};if(!Normalize(first))return false;
 WorldText::Vec3 chosen={};float smallest=100;
 for(unsigned int j=2;j<count;j++){
  WorldText::Vec3 v={points[j].x-points[0].x,points[j].y-points[0].y,points[j].z-points[0].z};if(!Normalize(v))return false;
  float dot=(float)(double(v.z)*first.z+double(v.y)*first.y+double(v.x)*first.x);float alignment=dot<0?-dot:dot;
  if(alignment<smallest){smallest=alignment;chosen=v;}
 }
 WorldText::Vec3 normal;
 normal.x=(float)(double(chosen.y)*first.z-double(chosen.z)*first.y);
 normal.y=(float)(double(chosen.z)*first.x-double(chosen.x)*first.z);
 normal.z=(float)(double(chosen.x)*first.y-double(chosen.y)*first.x);
 if(!Normalize(normal))return false;plane.x=normal.x;plane.y=normal.y;plane.z=normal.z;
 plane.d=(float)(double(normal.z)*points[0].z+double(normal.y)*points[0].y+double(normal.x)*points[0].x);return WorldText::Finite(plane.d);
}
inline bool BuildSides(const WorldText::Vec3* points,unsigned int count,const WorldText::Vec3& normal,Plane* output){
 if(!points||!output||count<3||count>128||!WorldText::FiniteVector(normal))return false;
 float normalLength=Length(normal.x,normal.y,normal.z);if(normalLength<0.9999f||normalLength>1.0001f)return false;
 for(unsigned int i=0;i<count;i++){
  const WorldText::Vec3& a=points[i];const WorldText::Vec3& b=points[(i+1)%count];if(!WorldText::FiniteVector(a)||!WorldText::FiniteVector(b))return false;
  float x=a.x-b.x,y=a.y-b.y,z=a.z-b.z;WorldText::Vec3 edge={x,y,z};if(!Normalize(edge))return false;x=edge.x;y=edge.y;z=edge.z;
  Plane& p=output[i];p.x=(float)(double(normal.y)*z-double(normal.z)*y);p.y=(float)(double(normal.z)*x-double(normal.x)*z);p.z=(float)(double(normal.x)*y-double(normal.y)*x);
  WorldText::Vec3 side={p.x,p.y,p.z};if(!Normalize(side))return false;p.x=side.x;p.y=side.y;p.z=side.z;
  // The original shifts the point toward the inward normal by +0.1.
  float sx=(float)(a.x-(float)(p.x*-0.1f)),sy=(float)(a.y-(float)(p.y*-0.1f)),sz=(float)(a.z-(float)(p.z*-0.1f));
  p.d=(float)(double(p.z)*sz+double(p.y)*sy+double(p.x)*sx);if(!WorldText::Finite(p.d))return false;
 }
 return true;
}
struct Candidate {const WorldText::Vec3* points;unsigned int count;Plane plane;};
struct Ends {float positiveDistance,negativeDistance;Plane positive,negative;bool hasPositive,hasNegative;};
inline float Distance(const Plane& p,const WorldText::Vec3& v){return (float)(double(p.z)*v.z+double(p.y)*v.y+double(p.x)*v.x-p.d);}
inline bool SelectEnds(const WorldText::Vec3* aperture,unsigned int count,const WorldText::Vec3& normal,const Candidate* candidates,unsigned int candidateCount,Ends& result){
 if(count<3||count>128||!candidates||!candidateCount||candidateCount>8192)return false;
 PrefabScene::Arena scratch;Plane* sides=(Plane*)scratch.Allocate(count*sizeof(Plane));WorldText::Vec3* current=(WorldText::Vec3*)scratch.Allocate(256*12);WorldText::Vec3* next=(WorldText::Vec3*)scratch.Allocate(256*12);
 if(!sides||!current||!next||!BuildSides(aperture,count,normal,sides))return false;
 Plane aperturePlane={normal.x,normal.y,normal.z,0};aperturePlane.d=(float)(double(normal.z)*aperture[0].z+double(normal.y)*aperture[0].y+double(normal.x)*aperture[0].x);
 result.positiveDistance=1000000;result.negativeDistance=-1000000;result.hasPositive=result.hasNegative=false;
 for(unsigned int c=0;c<candidateCount;c++){
  const Candidate& candidate=candidates[c];if(!candidate.points||candidate.count<3||candidate.count>128)return false;
  const float* pp=&candidate.plane.x;for(unsigned int k=0;k<4;k++)if(!WorldText::Finite(pp[k]))return false;
  if(candidate.plane.x==0&&candidate.plane.y==0&&candidate.plane.z==0)return false;
  unsigned int n=candidate.count;for(unsigned int j=0;j<n;j++){if(!WorldText::FiniteVector(candidate.points[j]))return false;current[j]=candidate.points[j];}
  for(unsigned int k=0;k<count&&n>=3;k++){
   unsigned int produced=0;
   for(unsigned int j=0;j<n;j++){
    const WorldText::Vec3& a=current[j];const WorldText::Vec3& b=current[(j+1)%n];float da=Distance(sides[k],a),db=Distance(sides[k],b);
    if(da>0){if(produced==256)return false;next[produced++]=a;}
    if((da>0)!=(db>0)){
     if(produced==256)return false;float ratio=(float)(1.0/(double(da)-db));WorldText::Vec3 hit;
     hit.x=a.x+(float)((float)((b.x-a.x)*da)*ratio);hit.y=a.y+(float)((float)((b.y-a.y)*da)*ratio);hit.z=a.z+(float)((float)((b.z-a.z)*da)*ratio);next[produced++]=hit;
    }
   }
   WorldText::Vec3* swap=current;current=next;next=swap;n=produced;
  }
  if(n<3)continue;
  double facing=double(normal.z)*candidate.plane.z+double(normal.y)*candidate.plane.y+double(normal.x)*candidate.plane.x;
  for(unsigned int j=0;j<n;j++){
   float distance=Distance(aperturePlane,current[j]);
   if(distance>=0){if(distance<result.positiveDistance&&facing>=0.01f){result.positiveDistance=distance;result.positive=candidate.plane;result.hasPositive=true;}}
   else if(distance>result.negativeDistance&&facing< -0.01f){result.negativeDistance=distance;result.negative=candidate.plane;result.hasNegative=true;}
  }
 }
 return true;
}

}
