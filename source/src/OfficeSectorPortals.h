#pragma once
#include "VisibilityTree.h"
#include "ExtrusionVolume.h"
#include "PortalBounds.h"
#include "SectorPreparation.h"
#include "PortalIntersection.h"
#include "PortalOrientation.h"
#include "PortalCandidates.h"
namespace OfficeSectorPortals {
struct PreparedSector {SectorOrientation::Face* faces;unsigned int count;};
struct Portal {unsigned int sectorA,sectorB,axis;WorldText::Vec3* vertices;unsigned int vertexCount;ExtrusionVolume::Plane plane;WorldText::Vec3 center;float radius;};
class AxisAligned {
 PrefabScene::Arena memory;
public:
 // Clipped portals rejected by the polygon check: owner and other sector (input order).
 Portal* portals;unsigned int count,*invalidOwner,*invalidOther,invalidCount,invalidCapacity;const char* error;
 AxisAligned():portals(0),count(0),invalidOwner(0),invalidOther(0),invalidCount(0),invalidCapacity(0),error(0){}
 bool Build(const StreamingSplit::Bounds* boxes,unsigned int n,const unsigned int* saveIds,const PreparedSector* prepared=0){
  memory.Clear();portals=0;count=0;error=0;invalidCount=0;invalidCapacity=4096;invalidOwner=(unsigned int*)memory.Allocate(invalidCapacity*4);invalidOther=(unsigned int*)memory.Allocate(invalidCapacity*4);if(!invalidOwner||!invalidOther)return false;if(!boxes||!saveIds||n>5000){error="Invalid rectangular sector input";return false;}
  for(unsigned int i=0;i<n;i++){if(saveIds[i]>=n){error="Invalid sector ID";return false;}for(unsigned int a=0;a<3;a++)if(!WorldText::Finite(boxes[i].low[a])||!WorldText::Finite(boxes[i].high[a])||boxes[i].low[a]>=boxes[i].high[a]){error="Invalid sector bounds";return false;}}
  if(!prepared){error="Source face pairs required";return false;}
  unsigned int* byId=(unsigned int*)memory.Allocate((n+1)*4);if(!byId)return false;for(unsigned int i=0;i<n;i++)byId[i]=0xffffffff;for(unsigned int i=0;i<n;i++){if(byId[saveIds[i]]!=0xffffffff)return false;byId[saveIds[i]]=i;}
  unsigned int capacity=0;
  for(unsigned int pass=0;pass<2;pass++){
   if(pass){portals=(Portal*)memory.Allocate((capacity+1)*sizeof(Portal));if(!portals)return false;}
   for(unsigned int left=0;left<n;left++)for(unsigned int right=left+1;right<n;right++){
    unsigned int i=byId[left],j=byId[right];const StreamingSplit::Bounds& a=boxes[i];const StreamingSplit::Bounds& b=boxes[j];
    if(!PortalCandidates::BoundsOverlap(*(const WorldText::Vec3*)a.low,*(const WorldText::Vec3*)a.high,*(const WorldText::Vec3*)b.low,*(const WorldText::Vec3*)b.high,0.1f))continue;
    const PreparedSector& owner=prepared[i];const PreparedSector& other=prepared[j];
    for(unsigned int f=0;f<owner.count;f++)for(unsigned int g=0;g<other.count;g++){
     const SectorOrientation::Face& first=owner.faces[f];const SectorOrientation::Face& second=other.faces[g];if(!PortalCandidates::FacePlanesMatch(first.plane,second.plane))continue;
     WorldText::Vec3 clipped[128];unsigned int clippedCount;bool invalid=false;
     if(!PortalIntersection::Clip(second.points,second.count,first.points,first.count,first.plane,clipped,clippedCount,&invalid))return false;
     if(invalid&&pass&&invalidCount<invalidCapacity){invalidOwner[invalidCount]=i;invalidOther[invalidCount++]=j;}
     if(!clippedCount)continue;

     if(!pass){if(++capacity>100000)return false;continue;}
     Portal& p=portals[count++];p.sectorA=left;p.sectorB=right;p.axis=3;p.vertexCount=clippedCount;p.vertices=(WorldText::Vec3*)memory.Allocate(clippedCount*12);if(!p.vertices)return false;
     if(!ExtrusionVolume::ComputeInternalPlane(clipped,clippedCount,p.plane)||!PortalOrientation::Orient(clipped,clippedCount,p.plane,owner.faces,owner.count))return false;
     for(unsigned int k=0;k<clippedCount;k++)p.vertices[k]=clipped[k];if(!PortalBounds::Compute(p.vertices,clippedCount,p.center,p.radius))return false;
    }
   }
  }return count==capacity;
 }
};
}

