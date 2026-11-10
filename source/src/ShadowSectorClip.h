#pragma once
namespace ShadowSectorClip {
struct Plane {double x,y,z,d;};
struct Volume {const Plane* planes;unsigned int count;};
class Preparation {
 struct Point {double x,y,z;};
 struct Contour {Point* points;unsigned int count;WorldText::Vec3 normal;};
 PrefabScene::Arena memory;
 static double Abs(double x){return x<0?-x:x;}
 static double Distance(const Plane& p,const Point& v){return p.x*v.x+p.y*v.y+p.z*v.z-p.d;}
 static bool Near(const Point& a,const Point& b){return Abs(a.x-b.x)<1e-6&&Abs(a.y-b.y)<1e-6&&Abs(a.z-b.z)<1e-6;}
 bool Append(Contour& c,const Point& p){if(c.count&&Near(c.points[c.count-1],p))return true;if(c.count==256)return false;c.points[c.count++]=p;return true;}
 bool Cut(const Contour& source,const Plane& plane,bool positive,bool forward,Contour& result){
  result.count=0;result.normal=source.normal;result.points=(Point*)memory.Allocate(256*sizeof(Point));if(!result.points)return false;
  for(unsigned int i=0;i<source.count;i++){
   const Point& a=source.points[forward?i:(i+source.count-1)%source.count];const Point& b=source.points[forward?(i+1)%source.count:i];double da=Distance(plane,a),db=Distance(plane,b);
   bool ia=positive?da>=-1e-6:da<=1e-6,ib=positive?db>=-1e-6:db<=1e-6;
   if(ia!=ib){double t=da/(da-db);Point hit={a.x+t*(b.x-a.x),a.y+t*(b.y-a.y),a.z+t*(b.z-a.z)};if(!Append(result,hit))return false;}
   if(ib&&!Append(result,b))return false;
  }
  if(result.count>1&&Near(result.points[0],result.points[result.count-1]))result.count--;if(result.count<3)result.count=0;return true;
 }
 bool Clip(const Contour& input,const Volume& v,bool forward,Contour& result){
  result=input;
  for(unsigned int i=0;i<v.count&&result.count;i++){
   bool coplanar=true;for(unsigned int k=0;k<result.count;k++)if(Abs(Distance(v.planes[i],result.points[k]))>=1e-6){coplanar=false;break;}
   if(coplanar){const Plane& p=v.planes[i];if(p.x*result.normal.x+p.y*result.normal.y+p.z*result.normal.z<0)result.count=0;continue;}
   Contour next;if(!Cut(result,v.planes[i],true,forward,next))return false;result=next;
  }return true;
 }
 bool Store(const Contour& c,ShadowJunctions::Polygon* list,unsigned int& count){
  if(!c.count)return true;if(count==6000||c.count>128)return false;
  WorldText::Vec3* points=(WorldText::Vec3*)memory.Allocate(c.count*sizeof(WorldText::Vec3));if(!points)return false;
  for(unsigned int i=0;i<c.count;i++){points[i].x=(float)c.points[i].x;points[i].y=(float)c.points[i].y;points[i].z=(float)c.points[i].z;}
  ShadowJunctions::Polygon& p=list[count++];p.points=points;p.count=c.count;p.normal=c.normal;return true;
 }
 bool Fail(const char* reason){memory.Clear();selected=all=0;selectedCount=allCount=0;error=reason;return false;}
public:
 ShadowJunctions::Polygon* selected;ShadowJunctions::Polygon* all;unsigned int selectedCount,allCount;const char* error;
 Preparation():selected(0),all(0),selectedCount(0),allCount(0),error(0){}
 bool Build(const ShadowJunctions::Polygon* source,unsigned int sourceCount,const Volume* volumes,unsigned int volumeCount,unsigned int target,bool forward){
  memory.Clear();selected=all=0;selectedCount=allCount=0;error=0;
  if(!source||!sourceCount||sourceCount>6000||!volumes||!volumeCount||volumeCount>64||target>=volumeCount)return Fail("Invalid clipping inputs");
  for(unsigned int i=0;i<volumeCount;i++){
   const Volume& v=volumes[i];if(!v.planes||v.count<4||v.count>128)return Fail("Invalid sector plane count");
   for(unsigned int j=0;j<v.count;j++){const Plane& p=v.planes[j];const double* values=&p.x;for(unsigned int k=0;k<4;k++)if(!(values[k]>=-3.402823466e38&&values[k]<=3.402823466e38))return Fail("Nonfinite sector plane");if(p.x*p.x+p.y*p.y+p.z*p.z==0)return Fail("Zero sector normal");}
  }
  selected=(ShadowJunctions::Polygon*)memory.Allocate(6000*sizeof(ShadowJunctions::Polygon));all=(ShadowJunctions::Polygon*)memory.Allocate(6000*sizeof(ShadowJunctions::Polygon));
  Contour* roots=(Contour*)memory.Allocate(sourceCount*sizeof(Contour));Contour* remaining=(Contour*)memory.Allocate(6000*sizeof(Contour));Contour* next=(Contour*)memory.Allocate(6000*sizeof(Contour));
  if(!selected||!all||!roots||!remaining||!next)return Fail("Clipping allocation failed");
  for(unsigned int i=0;i<sourceCount;i++){
   const ShadowJunctions::Polygon& p=source[i];if(!p.points||p.count<3||p.count>128||!WorldText::FiniteVector(p.normal)||p.normal.x==0&&p.normal.y==0&&p.normal.z==0)return Fail("Invalid source contour");
   Contour& c=roots[i];c.count=p.count;c.normal=p.normal;c.points=(Point*)memory.Allocate(p.count*sizeof(Point));if(!c.points)return Fail("Clipping allocation failed");
   for(unsigned int j=0;j<p.count;j++){if(!WorldText::FiniteVector(p.points[j]))return Fail("Nonfinite source vertex");c.points[j].x=p.points[j].x;c.points[j].y=p.points[j].y;c.points[j].z=p.points[j].z;}
  }
  // Match source-sector order, preserving contour start vertices for fan triangulation.
  for(unsigned int v=0;v<volumeCount;v++)for(unsigned int i=0;i<sourceCount;i++){Contour c;if(!Clip(roots[i],volumes[v],forward,c)||!Store(c,all,allCount))return Fail("Sector clipping limit");if(v==target&&!Store(c,selected,selectedCount))return Fail("Selected clipping limit");}
  for(unsigned int i=0;i<sourceCount;i++){
   unsigned int remainingCount=1;remaining[0]=roots[i];
   for(unsigned int v=0;v<volumeCount;v++){
    unsigned int nextCount=0;
    for(unsigned int r=0;r<remainingCount;r++){
     Contour intersection;if(!Clip(remaining[r],volumes[v],forward,intersection))return Fail("Difference clipping limit");
     if(!intersection.count){if(nextCount==6000)return Fail("Difference fragment limit");next[nextCount++]=remaining[r];continue;}
     Contour inside=remaining[r];
     for(unsigned int p=0;p<volumes[v].count&&inside.count;p++){
      const Plane& plane=volumes[v].planes[p];bool coplanar=true;for(unsigned int k=0;k<inside.count;k++)if(Abs(Distance(plane,inside.points[k]))>=1e-6){coplanar=false;break;}if(coplanar)continue;
      Contour outside,after;if(!Cut(inside,plane,false,false,outside)||!Cut(inside,plane,true,false,after))return Fail("Difference contour limit");
      if(outside.count){if(nextCount==6000)return Fail("Difference fragment limit");next[nextCount++]=outside;}inside=after;
     }
    }
    Contour* swap=remaining;remaining=next;next=swap;remainingCount=nextCount;
   }
   for(unsigned int r=0;r<remainingCount;r++)if(!Store(remaining[r],all,allCount))return Fail("Unassigned contour limit");
  }
  if(!selectedCount)return Fail("Selected sector has no contours");return true;
 }
};
}
