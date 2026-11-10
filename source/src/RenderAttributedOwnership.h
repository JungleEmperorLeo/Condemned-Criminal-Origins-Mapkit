#pragma once
#include "VisibilityCompiler.h"
#include "RenderAttributedClip.h"
#include "RenderFanTriangulation.h"
#include "RenderTriangleRouting.h"
#include "RenderContourCleanup.h"
#ifdef FEAR_SECTOR_PLANES_DUMP
void FearSectorPlanesDump(const float*,unsigned int);
void FearSectorSourceDump(unsigned int,unsigned int);
#endif
namespace RenderAttributedOwnership {
using Vertex=RenderClipVertex::Vertex;
struct Position {const WorldText::Vec3& operator()(const Vertex& v)const{return v.point;}};
struct Polygon {const Vertex* points;unsigned int count;WorldText::Vec3 normal;};
struct Contour {const Vertex* vertices;unsigned int count,owner;WorldText::Vec3 normal;unsigned int sourcePolygon;};
class Builder {
 struct Sector {RenderAttributedClip::Plane* planes;unsigned int count,index;};
 struct Work {const Vertex* points;unsigned int count;};
 PrefabScene::Arena memory;
 bool Fail(const char* why){memory.Clear();contours=0;count=assigned=unassigned=smallFragments=0;error=why;return false;}
 bool Copy(const Vertex* points,unsigned int n,Work& work){Vertex* copy=(Vertex*)memory.Allocate(n*sizeof(Vertex));if(!copy)return false;memcpy(copy,points,n*sizeof(Vertex));work.points=copy;work.count=n;return true;}
 bool Emit(const Work& work,const WorldText::Vec3& normal,unsigned int polygon,unsigned int owner){
  if(work.count<3){smallFragments++;return true;}if(count==100000)return false;Work own;if(!Copy(work.points,work.count,own))return false;own.count=RenderContourCleanup::CleanRecords((Vertex*)own.points,own.count,Position());if(own.count<3){smallFragments++;return true;}Contour& out=contours[count++];out.vertices=own.points;out.count=own.count;out.owner=owner;out.normal=normal;out.sourcePolygon=polygon;if(owner==RenderTriangleRouting::Unassigned)unassigned++;else assigned++;return true;
 }
public:
 Contour* contours;unsigned int count,assigned,unassigned,smallFragments;const char* error;
 Builder():contours(0),count(0),assigned(0),unassigned(0),smallFragments(0),error(0){}
 bool Build(const Polygon* input,unsigned int polygons,const StreamingMetadata::External* external,unsigned int sectors,const unsigned char* extrusion=0){
  // Without sectors every fragment stays unassigned (empty main render tree).
  Fail(0);if(polygons>100000||(polygons&&!input)||(sectors&&!external)||sectors>4096)return Fail("Invalid render ownership input");
  Sector* prepared=(Sector*)memory.Allocate((sectors?sectors:1)*sizeof(Sector));unsigned char* seen=(unsigned char*)memory.Allocate(sectors?sectors:1);Work* stack=(Work*)memory.Allocate(100000*sizeof(Work));contours=(Contour*)memory.Allocate(100000*sizeof(Contour));if(!prepared||!seen||!stack||!contours)return Fail("Render ownership allocation failed");for(unsigned int i=0;i<sectors;i++)seen[i]=0;
  for(unsigned int i=0;i<sectors;i++){
   const StreamingMetadata::External& source=external[i];const ConvexIntersection::Volume& v=source.volume;if(source.index>=sectors||seen[source.index]||!v.faces||!v.count||v.count>128||!WorldText::FiniteVector(v.low)||!WorldText::FiniteVector(v.high)||v.low.x>v.high.x||v.low.y>v.high.y||v.low.z>v.high.z)return Fail("Invalid prepared ownership sector");seen[source.index]=1;
   Sector& s=prepared[i];s.index=source.index;s.count=v.count;s.planes=(RenderAttributedClip::Plane*)memory.Allocate(v.count*sizeof(RenderAttributedClip::Plane));if(!s.planes)return Fail("Ownership plane allocation failed");
   // Extrusion sectors list their two end caps in the opposite order (original
   // clip input, framesort-proxy trace on Labs); plane values are the same.
   for(unsigned int j=0;j<v.count;j++){const ExtrusionVolume::Plane& p=v.faces[extrusion&&extrusion[i]&&v.count>=2&&j<2?1-j:j].plane;RenderAttributedClip::Plane& out=s.planes[j];out.x=p.x;out.y=p.y;out.z=p.z;out.d=p.d;if(!WorldText::Finite(out.x)||!WorldText::Finite(out.y)||!WorldText::Finite(out.z)||!WorldText::Finite(out.d)||(out.x==0&&out.y==0&&out.z==0))return Fail("Invalid ownership sector plane");}
  }
#ifdef FEAR_SECTOR_PLANES_DUMP
  {static bool dumped=false;if(!dumped){dumped=true;for(unsigned int i=0;i<sectors;i++)FearSectorPlanesDump(&prepared[i].planes[0].x,prepared[i].count);}}
#endif
  for(unsigned int polygon=0;polygon<polygons;polygon++){
   const Polygon& source=input[polygon];if(!source.points||source.count<3||source.count>128||!WorldText::FiniteVector(source.normal)||(source.normal.x==0&&source.normal.y==0&&source.normal.z==0))return Fail("Invalid ownership polygon");for(unsigned int j=0;j<source.count;j++)if(!WorldText::FiniteVector(source.points[j].point))return Fail("Nonfinite ownership point");
   unsigned int pending=1,search=0;stack[0].points=source.points;stack[0].count=source.count;
   while(pending){Work work=stack[--pending];if(work.count<3){smallFragments++;continue;}bool owned=false;
    for(unsigned int sector=search;sector<sectors;sector++){
     RenderAttributedClip::Builder clip;const Sector& s=prepared[sector];if(!clip.Build(work.points,work.count,source.normal,s.planes,s.count))return Fail(clip.error);
     if(!clip.inside.count)continue;
     Work inside={clip.inside.points,clip.inside.count};if(!Emit(inside,source.normal,polygon,s.index))return Fail("Ownership output limit");
     unsigned int pushed=0;for(unsigned int j=0;j<clip.outsideCount;j++){const RenderAttributedClip::Contour& outside=clip.outside[j];if(outside.count<3){smallFragments++;continue;}if(pending==100000||!Copy(outside.points,outside.count,stack[pending]))return Fail("Ownership work limit");pending++;pushed++;}
     // Original 10047CAD/10047CD0 append outside fragments in plane order;
     // 100477BC pops the last contour; 10047CD5 advances the sector search.
     search=pushed?sector+1:0;owned=true;break;
    }
    if(!owned){if(!Emit(work,source.normal,polygon,RenderTriangleRouting::Unassigned))return Fail("Ownership output limit");search=0;}
   }
  }return true;
 }
 bool Build(const Polygon* input,unsigned int polygons,const VisibilityCompiler::Builder& visibility){
  if(visibility.errorCode||!visibility.data||!visibility.size||(!visibility.streamingSectors&&visibility.sectorCount)||visibility.sectorCount>4096)return Fail("Source visibility preparation failed");
  // The low-level clipper consumes the caller's ordered volumes. The source
  // visibility bridge supplies serialized SaveID order, not construction order.
  const unsigned int slots=visibility.sectorCount?visibility.sectorCount:1;
  PrefabScene::Arena orderedMemory;StreamingMetadata::External* ordered=(StreamingMetadata::External*)orderedMemory.Allocate(slots*sizeof(StreamingMetadata::External));unsigned char* seen=(unsigned char*)orderedMemory.Allocate(slots);
  if(!ordered||!seen)return Fail("Source ownership ordering allocation failed");memset(seen,0,visibility.sectorCount);
  for(unsigned int i=0;i<visibility.sectorCount;i++){const StreamingMetadata::External& sector=visibility.streamingSectors[i];if(sector.index>=visibility.sectorCount||seen[sector.index])return Fail("Invalid source ownership SaveID");seen[sector.index]=1;ordered[sector.index]=sector;}
#ifdef FEAR_SECTOR_PLANES_DUMP
  {static bool once=false;if(!once){once=true;for(unsigned int i=0;i<visibility.sectorCount;i++)FearSectorSourceDump(visibility.streamingSectors[i].index,visibility.sectorSources?visibility.sectorSources[i]:0xffffffffu);}}
#endif
  unsigned char* extrusion=(unsigned char*)orderedMemory.Allocate(slots);if(!extrusion)return Fail("Source ownership extrusion allocation failed");memset(extrusion,0,visibility.sectorCount);
  // Extrusions are constructed after all Sector brushes (VisibilityCompiler).
  const unsigned int firstExtrusion=visibility.sectorCount-visibility.acceptedExtrusions;for(unsigned int i=firstExtrusion;i<visibility.sectorCount;i++)extrusion[visibility.streamingSectors[i].index]=1;
  return Build(input,polygons,ordered,visibility.sectorCount,extrusion);
 }
};
// Inputs must already belong to the main world and use VisibilityCompiler's
// centered coordinate system. Full source selection remains separate.
}

