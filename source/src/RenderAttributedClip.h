#pragma once
#include "RenderSectorClip.h"
#include "RenderClipVertex.h"
namespace RenderAttributedClip {
using Plane=RenderSectorClip::Plane;
using Vertex=RenderClipVertex::Vertex;
struct Contour {const Vertex* points;unsigned int count;};
class Builder {
 static bool FiniteVertex(const Vertex& v){if(!WorldText::FiniteVector(v.point))return false;for(unsigned int i=0;i<7;i++)if(!WorldText::Finite(v.attributes[i]))return false;return true;}
 PrefabScene::Arena memory;
 bool Fail(const char* why){memory.Clear();inside.points=0;inside.count=0;outside=0;outsideCount=0;error=why;return false;}
 static float Distance(const Plane& p,const Vertex& v){return (float)((double(p.z)*v.point.z+double(p.y)*v.point.y)+double(p.x)*v.point.x-p.d);}
 static unsigned int Side(float d,float epsilon){return d< -epsilon?0:(d>epsilon?1:2);}
 static bool Append(Vertex* points,unsigned int& n,const Vertex& p){if(n==512)return false;points[n++]=p;return true;}
 bool Store(const Vertex* points,unsigned int n,Contour& contour){Vertex* copy=(Vertex*)memory.Allocate((n+1)*sizeof(Vertex));if(!copy)return false;memcpy(copy,points,n*sizeof(Vertex));contour.points=copy;contour.count=n;return true;}
public:
 Contour inside;Contour* outside;unsigned int outsideCount;const char* error;
 Builder():outside(0),outsideCount(0),error(0){inside.points=0;inside.count=0;}
 bool Build(const Vertex* points,unsigned int count,const WorldText::Vec3& normal,const Plane* planes,unsigned int planeCount,float epsilon=.5f){
  Fail(0);if(!points||count<3||count>128||!planes||!planeCount||planeCount>128||!WorldText::FiniteVector(normal)||(normal.x==0&&normal.y==0&&normal.z==0)||!WorldText::Finite(epsilon)||epsilon<0)return Fail("Invalid sector clipping input");
  for(unsigned int i=0;i<count;i++)if(!FiniteVertex(points[i]))return Fail("Nonfinite clipping point");
  for(unsigned int i=0;i<planeCount;i++){const Plane& p=planes[i];if(!WorldText::Finite(p.x)||!WorldText::Finite(p.y)||!WorldText::Finite(p.z)||!WorldText::Finite(p.d)||(p.x==0&&p.y==0&&p.z==0))return Fail("Invalid clipping plane");}
  outside=(Contour*)memory.Allocate(planeCount*sizeof(Contour));Vertex* current=(Vertex*)memory.Allocate(512*sizeof(Vertex));Vertex* next=(Vertex*)memory.Allocate(512*sizeof(Vertex));Vertex* rejected=(Vertex*)memory.Allocate(512*sizeof(Vertex));if(!outside||!current||!next||!rejected)return Fail("Clipping allocation failed");memcpy(current,points,count*sizeof(Vertex));
  for(unsigned int p=0;p<planeCount&&count;p++){
   const Plane& plane=planes[p];unsigned int previous=Side(Distance(plane,current[count-1]),epsilon);
   if(previous==2){for(unsigned int j=count-1;j>0&&previous==2;j--)previous=Side(Distance(plane,current[j-1]),epsilon);}
   if(previous==2){double dot=(double(plane.z)*normal.z+double(plane.y)*normal.y)+double(plane.x)*normal.x;if(dot>0)continue;if(!Store(current,count,outside[outsideCount++]))return Fail("Clipping allocation failed");count=0;break;}
   unsigned int kept=0,discarded=0;Vertex a=current[count-1];float da=Distance(plane,a);
   for(unsigned int j=0;j<count;j++){
    const Vertex& b=current[j];float db=Distance(plane,b);unsigned int side=Side(db,epsilon);if(side==2)side=previous;
    if(side!=previous){float absolute=da<0?-da:da;if(absolute<epsilon){if(side==1){if(!Append(next,kept,a))return Fail("Clipping vertex limit");}else if(!Append(rejected,discarded,a))return Fail("Clipping vertex limit");}
     else{float t=(float)(double(da)/(double(da)-db));if(t<0)t=0;if(t>1)t=1;Vertex hit;if(!RenderClipVertex::Interpolate(a,b,t,hit))return Fail("Invalid intersection attributes");if(!Append(next,kept,hit)||!Append(rejected,discarded,hit))return Fail("Clipping vertex limit");}
    }
    if(side==1){if(!Append(next,kept,b))return Fail("Clipping vertex limit");}else if(!Append(rejected,discarded,b))return Fail("Clipping vertex limit");a=b;da=db;previous=side;
   }
   if(discarded&&!Store(rejected,discarded,outside[outsideCount++]))return Fail("Clipping allocation failed");count=kept;Vertex* swap=current;current=next;next=swap;
  }
  if(!Store(current,count,inside))return Fail("Clipping allocation failed");return true;
 }
};
// Full intermediate records. Outside fragments retain plane order, including contours with
// fewer than three points; subsequent original cleanup is a separate stage.
}
