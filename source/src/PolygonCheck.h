#pragma once
namespace PolygonCheck {
enum Issue {Degenerate=1,RepeatedPoint=2,NonPlanar=4,SelfIntersection=8,Concave=16,TooComplex=32};
struct Result {unsigned int issues;float residual;};
inline double Axis(const WorldText::Vec3& v,unsigned int axis){return axis==0?v.x:axis==1?v.y:v.z;}
inline double Turn(const WorldText::Vec3& a,const WorldText::Vec3& b,const WorldText::Vec3& c,unsigned int x,unsigned int y){return (Axis(b,x)-Axis(a,x))*(Axis(c,y)-Axis(a,y))-(Axis(b,y)-Axis(a,y))*(Axis(c,x)-Axis(a,x));}
inline bool Between(double a,double b,double p){return p>=(a<b?a:b)&&p<=(a>b?a:b);}
inline bool OnSegment(const WorldText::Vec3& a,const WorldText::Vec3& b,const WorldText::Vec3& p,unsigned int x,unsigned int y){return Turn(a,b,p,x,y)==0&&Between(Axis(a,x),Axis(b,x),Axis(p,x))&&Between(Axis(a,y),Axis(b,y),Axis(p,y));}
inline bool Intersects(const WorldText::Vec3& a,const WorldText::Vec3& b,const WorldText::Vec3& c,const WorldText::Vec3& d,unsigned int x,unsigned int y){
 double ac=Turn(a,b,c,x,y),ad=Turn(a,b,d,x,y),ca=Turn(c,d,a,x,y),cb=Turn(c,d,b,x,y);
 if(((ac>0&&ad<0)||(ac<0&&ad>0))&&((ca>0&&cb<0)||(ca<0&&cb>0)))return true;
 return OnSegment(a,b,c,x,y)||OnSegment(a,b,d,x,y)||OnSegment(c,d,a,x,y)||OnSegment(c,d,b,x,y);
}
inline Result Check(const WorldText::Geometry& g,const WorldText::Polygon& p){
 Result result={0,0};if(!p.validPlane)result.issues|=Degenerate;
 if(p.count>4096){result.issues|=TooComplex;return result;}
 float largest=0;unsigned int drop=2;
 if(p.validPlane){const float components[]={p.normal.x,p.normal.y,p.normal.z};for(unsigned int i=0;i<3;i++){float value=components[i]<0?-components[i]:components[i];if(value>largest){largest=value;drop=i;}}}
 else {
  double smallest=1e300;for(unsigned int axis=0;axis<3;axis++){double low=Axis(g.vertices[p.indices[0]],axis),high=low;for(unsigned int i=1;i<p.count;i++){double value=Axis(g.vertices[p.indices[i]],axis);if(value<low)low=value;if(value>high)high=value;}if(high-low<smallest){smallest=high-low;drop=axis;}}
 }
 unsigned int x=drop==0?1:0,y=drop==2?1:2;bool positive=false,negative=false;
 for(unsigned int i=0;i<p.count;i++){
  const WorldText::Vec3& a=g.vertices[p.indices[i]];
  if(p.validPlane){float distance=WorldText::Dot(p.normal,a)-p.distance;if(distance<0)distance=-distance;if(distance>result.residual)result.residual=distance;}
  double turn=Turn(a,g.vertices[p.indices[(i+1)%p.count]],g.vertices[p.indices[(i+2)%p.count]],x,y);if(turn>0)positive=true;if(turn<0)negative=true;
  for(unsigned int j=i+1;j<p.count;j++){
   const WorldText::Vec3& b=g.vertices[p.indices[j]];if(a.x==b.x&&a.y==b.y&&a.z==b.z)result.issues|=RepeatedPoint;
   if(j==i+1||(i==0&&j==p.count-1))continue;
   if(Intersects(a,g.vertices[p.indices[(i+1)%p.count]],b,g.vertices[p.indices[(j+1)%p.count]],x,y))result.issues|=SelfIntersection;
  }
 }
 // This is our explicit preparation policy, not a recovered FEAR tolerance.
 if(result.residual>0.01f)result.issues|=NonPlanar;
 if(positive&&negative)result.issues|=Concave;
 return result;
}
}
