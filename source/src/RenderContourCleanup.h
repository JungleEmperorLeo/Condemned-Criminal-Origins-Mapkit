#pragma once
namespace RenderContourCleanup {
// Geometry counterpart of original 10047370. Removal keeps the current index.
inline float Root(float value){float result;
 __asm {fld value}
 __asm {fsqrt}
 __asm {fstp result}
 return result;}
struct GeometryPosition {const WorldText::Vec3& operator()(const WorldText::Vec3& v)const{return v;}};
template<class Vertex,class Position> inline unsigned int CleanRecords(Vertex* points,unsigned int count,Position position){
 const float threshold=0.0001f;
 unsigned int i=0;
 while(i<count){
  const WorldText::Vec3 a=position(points[i]),b=position(points[(i+1)%count]),c=position(points[(i+2)%count]);
  float x=c.x-a.x,y=c.y-a.y,z=c.z-a.z;
  float length2=(float)((double(x)*x+double(y)*y)+double(z)*z);
  bool remove=length2<threshold;
  if(!remove){
   float inverse=(float)(1.0/Root(length2));x=(float)(double(x)*inverse);y=(float)(double(y)*inverse);z=(float)(double(z)*inverse);
   float bx=b.x-a.x,by=b.y-a.y,bz=b.z-a.z;
   float projection=(float)((double(bx)*x+double(by)*y)+double(bz)*z);
   float px=(float)(double(a.x)+(float)(double(projection)*x));
   float py=(float)(double(a.y)+(float)(double(projection)*y));
   float pz=(float)(double(a.z)+(float)(double(projection)*z));
   bx=b.x-px;by=b.y-py;bz=b.z-pz;
   float distance2=(float)((double(bx)*bx+double(by)*by)+double(bz)*bz);
   remove=distance2<threshold;
  }
  if(remove){for(unsigned int j=i;j+1<count;j++)points[j]=points[j+1];if(--count<3)return 0;}
  else i++;
 }
 return count;
}
inline unsigned int Clean(WorldText::Vec3* points,unsigned int count){return CleanRecords(points,count,GeometryPosition());}
}
