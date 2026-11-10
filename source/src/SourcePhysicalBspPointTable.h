#pragma once
#include "SourcePhysicalBspContours.h"
// Save-point compaction is distinct from the earlier shared point tags.
// Only flag0x08 polygons mark used points in FEAR SetupIndexMap (RVA0x24800).
namespace SourcePhysicalBspPointTable {
class Table {
 PrefabScene::Arena memory;
 Table(const Table&);Table& operator=(const Table&);
 bool Fail(const char* reason){Clear();error=reason;return false;}
public:
 WorldText::Vec3* points;unsigned int* references;unsigned int* firstReference;
 unsigned int pointCount,referenceCount,initialPoints,internalPoints;const char* error;
 Table():points(0),references(0),firstReference(0),pointCount(0),referenceCount(0),initialPoints(0),internalPoints(0),error(0){}
 void Clear(){memory.Clear();points=0;references=firstReference=0;pointCount=referenceCount=initialPoints=internalPoints=0;error=0;}
 bool Build(const SourcePhysicalBspContours::Main& source){
  Clear();if(source.error||!source.contours||!source.count||source.count>100000||!source.references||source.references>1000000)return Fail("Invalid source BSP point-table input");
  PrefabScene::Arena temporary;WorldText::Vec3* input=(WorldText::Vec3*)temporary.Allocate(source.references*12);
  unsigned int* offsets=(unsigned int*)temporary.Allocate((source.count+1)*4);
  if(!input||!offsets)return Fail("Source BSP point preparation allocation failed");unsigned int at=0;
  for(unsigned int p=0;p<source.count;p++){
   const FearBspSplitPolicy::Polygon& polygon=source.contours[p].polygon;offsets[p]=at;
   if(!polygon.points||polygon.count<3||polygon.count>30||polygon.count>source.references-at)return Fail("Invalid source BSP point contour");
   memcpy(input+at,polygon.points,polygon.count*12);at+=polygon.count;if(polygon.flags&8)referenceCount+=polygon.count;
  }
   offsets[source.count]=at;if(at!=source.references)return Fail("Source BSP reference enumeration mismatch");
   if(!referenceCount){firstReference=(unsigned int*)memory.Allocate((source.count+1)*4);return firstReference!=0||Fail("Empty source BSP point-table allocation failed");}
  SourceRenderPointTags::Indexer indexer;if(!indexer.Build(input,at))return Fail("Source BSP initial point indexing failed");initialPoints=indexer.pointCount;
  unsigned int* rename=(unsigned int*)temporary.Allocate(at*4);unsigned int* remapped=(unsigned int*)temporary.Allocate(at*4);unsigned int* copied=(unsigned int*)temporary.Allocate(at*4);
  if(!rename||!remapped||!copied||!RenderPointRemap::Build(initialPoints,indexer.indices,at,rename,remapped,copied,at,internalPoints))return Fail("Source BSP unused-point remapping failed");
  unsigned char* used=(unsigned char*)temporary.Allocate(internalPoints);unsigned int* saved=(unsigned int*)temporary.Allocate(internalPoints*4);
  points=(WorldText::Vec3*)memory.Allocate(internalPoints*12);references=(unsigned int*)memory.Allocate(referenceCount*4);firstReference=(unsigned int*)memory.Allocate((source.count+1)*4);
  if(!used||!saved||!points||!references||!firstReference)return Fail("Source BSP saved-point allocation failed");memset(used,0,internalPoints);
  for(unsigned int p=0;p<source.count;p++)if(source.contours[p].polygon.flags&8)for(unsigned int i=offsets[p];i<offsets[p+1];i++)used[remapped[i]]=1;
  for(unsigned int i=0;i<internalPoints;i++){saved[i]=pointCount;if(used[i])points[pointCount++]=indexer.points[copied[i]];}
  at=0;for(unsigned int p=0;p<source.count;p++){
   firstReference[p]=at;
   if(source.contours[p].polygon.flags&8)for(unsigned int i=offsets[p];i<offsets[p+1];i++)references[at++]=saved[remapped[i]];
  }
  firstReference[source.count]=at;if(at!=referenceCount)return Fail("Source BSP saved-reference enumeration mismatch");return true;
 }
};
}
