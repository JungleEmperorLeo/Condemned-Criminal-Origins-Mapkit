#pragma once
#include "VisibilityTree.h"
#include "ExtrusionVolume.h"
#include "PortalBounds.h"
#include "SectorPreparation.h"
#include "PortalIntersection.h"
#include "PortalOrientation.h"
namespace SectorPortals {
struct PreparedSector {SectorOrientation::Face* faces;unsigned int count;};
struct Portal {unsigned int sectorA,sectorB,axis;WorldText::Vec3 vertices[4];ExtrusionVolume::Plane plane;WorldText::Vec3 center;float radius;};
class AxisAligned {
 PrefabScene::Arena memory;
public:
 Portal* portals;unsigned int count;const char* error;
 AxisAligned():portals(0),count(0),error(0){}
 bool Build(const StreamingSplit::Bounds* boxes,unsigned int n,const unsigned int* saveIds,const PreparedSector* prepared=0){
  memory.Clear();portals=0;count=0;error=0;if(!boxes||!saveIds||n>5000){error="Invalid rectangular sector input";return false;}
  for(unsigned int i=0;i<n;i++){if(saveIds[i]>=n){error="Invalid sector ID";return false;}for(unsigned int a=0;a<3;a++)if(!WorldText::Finite(boxes[i].low[a])||!WorldText::Finite(boxes[i].high[a])||boxes[i].low[a]>=boxes[i].high[a]){error="Invalid sector bounds";return false;}}
  // Exact shared-face profile only; positive-area rectangles, no near-plane snapping.
  unsigned int* byId=(unsigned int*)memory.Allocate((n+1)*4);if(!byId)return false;for(unsigned int i=0;i<n;i++)byId[i]=0xffffffff;for(unsigned int i=0;i<n;i++){if(byId[saveIds[i]]!=0xffffffff){error="Duplicate sector ID";return false;}byId[saveIds[i]]=i;}
  unsigned int capacity=0;
  for(unsigned int pass=0;pass<2;pass++){
   if(pass){portals=(Portal*)memory.Allocate((capacity+1)*sizeof(Portal));if(!portals)return false;}
   for(unsigned int left=0;left<n;left++)for(unsigned int right=left+1;right<n;right++)for(unsigned int axis=0;axis<3;axis++){
    unsigned int i=byId[left],j=byId[right];
    const StreamingSplit::Bounds& a=boxes[i];const StreamingSplit::Bounds& b=boxes[j];float coordinate,sign;
    if(a.high[axis]==b.low[axis]){coordinate=a.high[axis];sign=-1;}else if(b.high[axis]==a.low[axis]){coordinate=b.high[axis];sign=1;}else continue;
    unsigned int y=(axis+1)%3,z=(axis+2)%3;float ly=a.low[y]>b.low[y]?a.low[y]:b.low[y],hy=a.high[y]<b.high[y]?a.high[y]:b.high[y];float lz=a.low[z]>b.low[z]?a.low[z]:b.low[z],hz=a.high[z]<b.high[z]?a.high[z]:b.high[z];if(ly>=hy||lz>=hz)continue;
    if(!pass){if(++capacity>100000){error="Too many portals";return false;}continue;}
    Portal& p=portals[count++];p.sectorA=saveIds[i];p.sectorB=saveIds[j];p.axis=axis;
    for(unsigned int k=0;k<4;k++){float* v=&p.vertices[k].x;v[axis]=coordinate;unsigned int corner=sign<0?k:(4-k)%4;v[y]=(corner==1||corner==2)?hy:ly;v[z]=corner>=2?hz:lz;}
    if(prepared){
     const PreparedSector& owner=prepared[i];const PreparedSector& other=prepared[j];if(owner.count!=6||other.count!=6){error="Prepared profile requires six rectangular faces";return false;}
     const SectorOrientation::Face* first=0;const SectorOrientation::Face* second=0;
     for(unsigned int face=0;face<6;face++){
      if(owner.faces[face].count!=4||other.faces[face].count!=4)return false;bool allFirst=true,allSecond=true;
      for(unsigned int k=0;k<4;k++){if((&owner.faces[face].points[k].x)[axis]!=coordinate)allFirst=false;if((&other.faces[face].points[k].x)[axis]!=coordinate)allSecond=false;}
      if(allFirst)first=&owner.faces[face];if(allSecond)second=&other.faces[face];
     }
     if(!first||!second){error="No source face at shared plane";return false;}WorldText::Vec3 clipped[128];unsigned int clippedCount;
     if(!PortalIntersection::Clip(second->points,second->count,first->points,first->count,first->plane,clipped,clippedCount)||clippedCount!=4){error="Prepared intersection is not rectangular";return false;}
     if(!ExtrusionVolume::ComputeInternalPlane(clipped,clippedCount,p.plane)||!PortalOrientation::Orient(clipped,clippedCount,p.plane,owner.faces,owner.count))return false;
     for(unsigned int k=0;k<4;k++)p.vertices[k]=clipped[k];
    }else{
    p.plane.x=p.plane.y=p.plane.z=0;(&p.plane.x)[axis]=sign;p.plane.d=sign*coordinate;
    }
    if(!PortalBounds::Compute(p.vertices,4,p.center,p.radius)){error="Invalid portal bounds";return false;}
   }
  }return count==capacity;
 }
};
}
