#pragma once
// Original polygon/sector precheck, including clipping of spanning contours.
namespace ExtrusionBoundaryCheck {
struct Plane {float x,y,z,d;};
inline bool Evaluate(const WorldText::Vec3* points,unsigned int count,const WorldText::Vec3& originalNormal,const Plane* planes,unsigned int planeCount,float tolerance,bool& intersects){
 intersects=false;
 if(!points||count<3||count>128||!planes||!planeCount||planeCount>128||!WorldText::FiniteVector(originalNormal)||!WorldText::Finite(tolerance))return false;
 if(originalNormal.x==0&&originalNormal.y==0&&originalNormal.z==0)return false;
 for(unsigned int i=0;i<count;i++)if(!WorldText::FiniteVector(points[i]))return false;
 for(unsigned int i=0;i<planeCount;i++){const float* p=&planes[i].x;for(unsigned int k=0;k<4;k++)if(!WorldText::Finite(p[k]))return false;if(p[0]==0&&p[1]==0&&p[2]==0)return false;}
 PrefabScene::Arena scratch;
 WorldText::Vec3* current=(WorldText::Vec3*)scratch.Allocate(256*12);
 WorldText::Vec3* clipped=(WorldText::Vec3*)scratch.Allocate(256*12);
 if(!current||!clipped)return false;
 for(unsigned int j=0;j<count;j++)current[j]=points[j];
 float epsilon=tolerance<0?-tolerance:tolerance;
 for(unsigned int i=0;i<planeCount;i++){
  const Plane& p=planes[i];unsigned int front=0,back=0;
  for(unsigned int j=0;j<count;j++){const WorldText::Vec3& v=current[j];float d=(float)(double(v.x)*p.x+double(v.y)*p.y+double(v.z)*p.z-p.d);if(d>=epsilon)front++;else if(d<=-epsilon)back++;}
  if(front+back==0){double facing=double(originalNormal.x)*p.x+double(originalNormal.y)*p.y+double(originalNormal.z)*p.z;if(facing>0)front++;else back++;}
  if(!back)continue;
  if(!front)return true;
  unsigned int produced=0;
  for(unsigned int j=0;j<count;j++){
   const WorldText::Vec3& a=current[j];const WorldText::Vec3& b=current[(j+1)%count];
   float da=(float)(double(a.x)*p.x+double(a.y)*p.y+double(a.z)*p.z-p.d+tolerance);
   float db=(float)(double(b.x)*p.x+double(b.y)*p.y+double(b.z)*p.z-p.d+tolerance);
   if(da>=0){if(produced==256)return false;clipped[produced++]=a;}
   if((da>=0)!=(db>=0)){
    if(produced==256)return false;
    float reciprocal=(float)(1.0/(double(da)-db));
    WorldText::Vec3 v;
    v.x=a.x+(float)((float)((b.x-a.x)*da)*reciprocal);
    v.y=a.y+(float)((float)((b.y-a.y)*da)*reciprocal);
    v.z=a.z+(float)((float)((b.z-a.z)*da)*reciprocal);
    if(!WorldText::FiniteVector(v))return false;clipped[produced++]=v;
   }
  }
  if(produced<3)return true;
  WorldText::Vec3* swap=current;current=clipped;clipped=swap;count=produced;
 }
 intersects=true;return true;
}
}
