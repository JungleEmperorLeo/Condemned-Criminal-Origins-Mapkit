#pragma once
// Intermediate polygon preparation. Sector clipping supplies both assigned and
// unassigned contours; the latter participate in edge matching, not emission.
namespace ShadowJunctions {
struct Polygon {const WorldText::Vec3* points;unsigned int count;WorldText::Vec3 normal;};
class Preparation {
 PrefabScene::Arena memory;
 static double Abs(double x){return x<0?-x:x;}
 static double Dot(const WorldText::Vec3& a,const WorldText::Vec3& b){return double(a.x)*b.x+double(a.y)*b.y+double(a.z)*b.z;}
 static WorldText::Vec3 Sub(const WorldText::Vec3& a,const WorldText::Vec3& b){WorldText::Vec3 v={a.x-b.x,a.y-b.y,a.z-b.z};return v;}
 static bool Near(const WorldText::Vec3& a,const WorldText::Vec3& b,double epsilon){return Abs(double(a.x)-b.x)<epsilon&&Abs(double(a.y)-b.y)<epsilon&&Abs(double(a.z)-b.z)<epsilon;}
 static double Parameter(const WorldText::Vec3& p,const WorldText::Vec3& a,const WorldText::Vec3& d,double length){return Dot(Sub(p,a),d)/length;}
 static bool Line(const WorldText::Vec3& p,const WorldText::Vec3& a,const WorldText::Vec3& d,double length,double epsilon,bool squared){
  double t=Parameter(p,a,d,length),x=double(p.x)-a.x-t*d.x,y=double(p.y)-a.y-t*d.y,z=double(p.z)-a.z-t*d.z;
  return squared?x*x+y*y+z*z<epsilon:Abs(x)<epsilon&&Abs(y)<epsilon&&Abs(z)<epsilon;
 }
 static bool Boundary(const Polygon& p,const WorldText::Vec3& a,const WorldText::Vec3& b){
  for(unsigned int i=0;i<p.count;i++){const WorldText::Vec3& first=p.points[i];WorldText::Vec3 d=Sub(p.points[(i+1)%p.count],first);double length=Dot(d,d);if(!length)continue;
   double ta=Parameter(a,first,d,length),tb=Parameter(b,first,d,length);
   if(ta>=-1e-6&&ta<=1+1e-6&&tb>=-1e-6&&tb<=1+1e-6&&Line(a,first,d,length,1e-5,false)&&Line(b,first,d,length,1e-5,false))return true;
  }return false;
 }
 static bool Less(const WorldText::Vec3& a,const WorldText::Vec3& b){return a.x<b.x||(a.x==b.x&&(a.y<b.y||(a.y==b.y&&a.z<b.z)));}
 static bool Valid(const Polygon* p,unsigned int n){
  if(!p||!n||n>6000)return false;
  for(unsigned int i=0;i<n;i++){if(!p[i].points||p[i].count<3||p[i].count>128||!WorldText::FiniteVector(p[i].normal)||Dot(p[i].normal,p[i].normal)==0)return false;
   for(unsigned int j=0;j<p[i].count;j++)if(!WorldText::FiniteVector(p[i].points[j])||Near(p[i].points[j],p[i].points[(j+1)%p[i].count],1e-7))return false;
  }return true;
 }
 bool Fail(const char* reason){memory.Clear();triangles=0;count=0;error=reason;return false;}
public:
 ShadowMesh::Triangle* triangles;unsigned int count;const char* error;
 Preparation():triangles(0),count(0),error(0){}
 bool Build(const Polygon* selected,unsigned int selectedCount,const Polygon* all,unsigned int allCount){
  memory.Clear();triangles=0;count=0;error=0;
  if(!Valid(selected,selectedCount)||!Valid(all,allCount))return Fail("Invalid junction polygons");
  unsigned int capacity=0;for(unsigned int i=0;i<allCount;i++){if(all[i].count>100000-capacity)return Fail("Too many junction vertices");capacity+=all[i].count;}
  WorldText::Vec3* pool=(WorldText::Vec3*)memory.Allocate(capacity*sizeof(WorldText::Vec3));
  ShadowMesh::Triangle* stack=(ShadowMesh::Triangle*)memory.Allocate(6000*sizeof(ShadowMesh::Triangle));triangles=(ShadowMesh::Triangle*)memory.Allocate(6000*sizeof(ShadowMesh::Triangle));
  if(!pool||!stack||!triangles)return Fail("Junction allocation failed");
  for(unsigned int polygon=0;polygon<selectedCount;polygon++){
   const Polygon& contour=selected[polygon];
   for(unsigned int fan=1;fan+1<contour.count;fan++){
    unsigned int pending=1;stack[0].p[0]=contour.points[0];stack[0].p[1]=contour.points[fan];stack[0].p[2]=contour.points[fan+1];stack[0].normal=contour.normal;
    while(pending){ShadowMesh::Triangle current=stack[--pending];bool split=false;
     for(unsigned int edge=0;edge<3&&!split;edge++){
      const WorldText::Vec3& a=current.p[edge];const WorldText::Vec3& b=current.p[(edge+1)%3];const WorldText::Vec3& c=current.p[(edge+2)%3];WorldText::Vec3 d=Sub(b,a);double length=Dot(d,d);
      if(!length||!Boundary(contour,a,b))continue;unsigned int used=0;bool shared=false;
      for(unsigned int q=0;q<allCount;q++)for(unsigned int e=0;e<all[q].count;e++){
       const WorldText::Vec3& first=all[q].points[e];const WorldText::Vec3& last=all[q].points[(e+1)%all[q].count];
       for(unsigned int k=0;k<contour.count;k++)if(Near(first,contour.points[(k+1)%contour.count],.1)&&Near(last,contour.points[k],.1)&&Line(contour.points[k],a,d,length,.01,true)&&Line(contour.points[(k+1)%contour.count],a,d,length,.01,true))shared=true;
       if(Line(first,a,d,length,.01,true)&&Line(last,a,d,length,.01,true)){
        // Each polygon contributes each endpoint at most twice. Only eligible
        // interior points are retained, deduplicated below before sorting.
        const WorldText::Vec3* endpoints[2]={&first,&last};for(unsigned int k=0;k<2;k++){const WorldText::Vec3& v=*endpoints[k];double t=Parameter(v,a,d,length);if(t<=1e-6||t>=1-1e-6||!Line(v,a,d,length,1e-5,false))continue;
         unsigned int found=0;for(;found<used;found++)if(Near(pool[found],v,1e-7))break;if(found==used){if(used==capacity)return Fail("Junction candidate overflow");pool[used++]=v;}
        }
       }
      }
      if(shared||!used)continue;
      WorldText::Vec3 point=pool[0];for(unsigned int k=1;k<used;k++)if(Less(pool[k],point))point=pool[k];
      if(pending+2>6000)return Fail("Junction stack limit");ShadowMesh::Triangle left={{a,point,c},contour.normal},right={{point,b,c},contour.normal};stack[pending++]=left;stack[pending++]=right;split=true;
     }
     if(!split){if(count==6000)return Fail("Junction triangle limit");triangles[count++]=current;}
    }
   }
  }
  return true;
 }
};
}
