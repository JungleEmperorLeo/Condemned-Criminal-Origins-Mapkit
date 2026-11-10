#pragma once
#include "VisibilityCompiler.h"
#include "RenderSectorClip.h"
#include "RenderFanTriangulation.h"
#include "RenderTriangleRouting.h"
#include "RenderContourCleanup.h"
namespace RenderSectorOwnership {
struct Polygon {const WorldText::Vec3* points;unsigned int count;WorldText::Vec3 normal;};
struct Contour {RenderFanTriangulation::Contour fan;WorldText::Vec3 normal;unsigned int sourcePolygon;};
class Builder {
 struct Sector {RenderSectorClip::Plane* planes;unsigned int count,index;};
 struct Work {const WorldText::Vec3* points;unsigned int count;};
 PrefabScene::Arena memory;
 bool Fail(const char* why){memory.Clear();contours=0;count=assigned=unassigned=smallFragments=0;error=why;return false;}
 bool Copy(const WorldText::Vec3* points,unsigned int n,Work& work){WorldText::Vec3* copy=(WorldText::Vec3*)memory.Allocate(n*12);if(!copy)return false;memcpy(copy,points,n*12);work.points=copy;work.count=n;return true;}
 bool Emit(const Work& work,const WorldText::Vec3& normal,unsigned int polygon,unsigned int owner){
  if(work.count<3){smallFragments++;return true;}if(count==100000)return false;Work own;if(!Copy(work.points,work.count,own))return false;own.count=RenderContourCleanup::Clean((WorldText::Vec3*)own.points,own.count);if(own.count<3){smallFragments++;return true;}Contour& out=contours[count++];out.fan.vertices=own.points;out.fan.count=own.count;out.fan.owner=owner;out.normal=normal;out.sourcePolygon=polygon;if(owner==RenderTriangleRouting::Unassigned)unassigned++;else assigned++;return true;
 }
public:
 Contour* contours;unsigned int count,assigned,unassigned,smallFragments;const char* error;
 Builder():contours(0),count(0),assigned(0),unassigned(0),smallFragments(0),error(0){}
 bool Build(const Polygon* input,unsigned int polygons,const StreamingMetadata::External* external,unsigned int sectors){
  Fail(0);if(polygons>100000||(polygons&&!input)||!external||!sectors||sectors>4096)return Fail("Invalid render ownership input");
  Sector* prepared=(Sector*)memory.Allocate(sectors*sizeof(Sector));unsigned char* seen=(unsigned char*)memory.Allocate(sectors);Work* stack=(Work*)memory.Allocate(100000*sizeof(Work));contours=(Contour*)memory.Allocate(100000*sizeof(Contour));if(!prepared||!seen||!stack||!contours)return Fail("Render ownership allocation failed");for(unsigned int i=0;i<sectors;i++)seen[i]=0;
  for(unsigned int i=0;i<sectors;i++){
   const StreamingMetadata::External& source=external[i];const ConvexIntersection::Volume& v=source.volume;if(source.index>=sectors||seen[source.index]||!v.faces||!v.count||v.count>128||!WorldText::FiniteVector(v.low)||!WorldText::FiniteVector(v.high)||v.low.x>v.high.x||v.low.y>v.high.y||v.low.z>v.high.z)return Fail("Invalid prepared ownership sector");seen[source.index]=1;
   Sector& s=prepared[i];s.index=source.index;s.count=v.count;s.planes=(RenderSectorClip::Plane*)memory.Allocate(v.count*sizeof(RenderSectorClip::Plane));if(!s.planes)return Fail("Ownership plane allocation failed");
   for(unsigned int j=0;j<v.count;j++){const ExtrusionVolume::Plane& p=v.faces[j].plane;RenderSectorClip::Plane& out=s.planes[j];out.x=p.x;out.y=p.y;out.z=p.z;out.d=p.d;if(!WorldText::Finite(out.x)||!WorldText::Finite(out.y)||!WorldText::Finite(out.z)||!WorldText::Finite(out.d)||(out.x==0&&out.y==0&&out.z==0))return Fail("Invalid ownership sector plane");}
  }
  for(unsigned int polygon=0;polygon<polygons;polygon++){
   const Polygon& source=input[polygon];if(!source.points||source.count<3||source.count>128||!WorldText::FiniteVector(source.normal)||(source.normal.x==0&&source.normal.y==0&&source.normal.z==0))return Fail("Invalid ownership polygon");for(unsigned int j=0;j<source.count;j++)if(!WorldText::FiniteVector(source.points[j]))return Fail("Nonfinite ownership point");
   unsigned int pending=1,search=0;stack[0].points=source.points;stack[0].count=source.count;
   while(pending){Work work=stack[--pending];if(work.count<3){smallFragments++;continue;}bool owned=false;
    for(unsigned int sector=search;sector<sectors;sector++){
     RenderSectorClip::Builder clip;const Sector& s=prepared[sector];if(!clip.Build(work.points,work.count,source.normal,s.planes,s.count))return Fail(clip.error);
     if(!clip.inside.count)continue;
     Work inside={clip.inside.points,clip.inside.count};if(!Emit(inside,source.normal,polygon,s.index))return Fail("Ownership output limit");
     unsigned int pushed=0;for(unsigned int j=0;j<clip.outsideCount;j++){const RenderSectorClip::Contour& outside=clip.outside[j];if(outside.count<3){smallFragments++;continue;}if(pending==100000||!Copy(outside.points,outside.count,stack[pending]))return Fail("Ownership work limit");pending++;pushed++;}
     // Original 10047CAD/10047CD0 append outside fragments in plane order;
     // 100477BC pops the last contour; 10047CD5 advances the sector search.
     search=pushed?sector+1:0;owned=true;break;
    }
    if(!owned){if(!Emit(work,source.normal,polygon,RenderTriangleRouting::Unassigned))return Fail("Ownership output limit");search=0;}
   }
  }return true;
 }
 bool Build(const Polygon* input,unsigned int polygons,const VisibilityCompiler::Builder& visibility){
  if(visibility.errorCode||!visibility.data||!visibility.size)return Fail("Source visibility preparation failed");return Build(input,polygons,visibility.streamingSectors,visibility.sectorCount);
 }
};
// Inputs must already belong to the main world and use VisibilityCompiler's
// centered coordinate system. Full source selection and cleanup remain separate.
}

