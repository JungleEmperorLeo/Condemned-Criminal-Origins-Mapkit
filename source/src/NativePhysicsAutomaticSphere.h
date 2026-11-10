#pragma once
#include "SourceRenderPlane.h"
#include "SourcePhysicalBspContours.h"
#include "PhysicsOwnerBounds.h"
#include "SourcePhysicsSphereFit.h"
// Source automatic Sphere preparation. Owner serialization/integration is
// deliberately separate. Prepared polygons and fit inputs remain owned after
// the source context is cleared; no compiled-world input is accepted.
namespace NativePhysicsAutomaticSphere {
struct Polygon {const WorldText::Vec3* points;unsigned int count,flags;};
class Builder {
 PrefabScene::Arena memory;
 Builder(const Builder&);Builder& operator=(const Builder&);
 bool Fail(const char* message){char saved[512];lstrcpynA(saved,message?message:"Automatic Sphere preparation failed",512);Clear();lstrcpyA(diagnostic,saved);error=diagnostic;return false;}
 bool Finish(float mass,float density){
  if(!WorldText::Finite(mass)||!WorldText::Finite(density)||!WorldText::FiniteVector(cachedCenter))return Fail("Invalid automatic Sphere mass/density/center");
  // Original shared automatic-shape sanitation, RVA 0x3A960.
  if(mass<.1f)mass=.1f;if(density<.01f)density=.01f;
  body[0]=mass;body[1]=density;body[2]=0;translation.x=translation.y=translation.z=0;
  unsigned int wanted=0;
  for(unsigned int p=0;p<polygonCount;p++){
   const Polygon& polygon=polygons[p];if((polygon.flags&0x180)||!(polygon.flags&0x10))continue;
   if(polygon.count>100000-wanted)return Fail("Automatic Sphere fit point limit");wanted+=polygon.count;
  }
  pointCount=wanted;
  // The original writer skips fitting an empty collision list and retains its
  // identity transform/radius zero; fit's -1 sentinel is never serialized.
  if(!wanted)return true;
  points=(SourcePhysicsSphereFit::Vec4*)memory.Allocate(wanted*sizeof(SourcePhysicsSphereFit::Vec4));
  fitPoints=(SourcePhysicsSphereFit::Vec4*)memory.Allocate(wanted*sizeof(SourcePhysicsSphereFit::Vec4));
  if(!points||!fitPoints)return Fail("Automatic Sphere point allocation failed");
  unsigned int at=0;
  for(unsigned int p=0;p<polygonCount;p++){
   const Polygon& polygon=polygons[p];if((polygon.flags&0x180)||!(polygon.flags&0x10))continue;
   for(unsigned int i=0;i<polygon.count;i++){const WorldText::Vec3& point=polygon.points[i];points[at].x=point.x;points[at].y=point.y;points[at].z=point.z;points[at].w=0;fitPoints[at]=points[at];at++;}
  }
  if(at!=wanted)return Fail("Automatic Sphere point enumeration mismatch");
  union Epsilon {unsigned int bits;float value;};Epsilon epsilon;epsilon.bits=0x3727c5acu;
  float fitted[4];if(!SourcePhysicsSphereFit::Fit(fitPoints,wanted,epsilon.value,fitted))return Fail("Automatic Sphere fit failed or work limit exceeded");
  for(unsigned int i=0;i<4;i++)sphere[i]=fitted[i];body[2]=fitted[3];
  translation.x=fitted[0]-cachedCenter.x;translation.y=fitted[1]-cachedCenter.y;translation.z=fitted[2]-cachedCenter.z;
  return WorldText::FiniteVector(translation)||Fail("Nonfinite automatic Sphere translation");
 }
public:
 Polygon* polygons;unsigned int polygonCount,references,pointCount;
 SourcePhysicsSphereFit::Vec4* points;SourcePhysicsSphereFit::Vec4* fitPoints;
 WorldText::Vec3 cachedCenter,translation;float sphere[4],body[3];const char* error;char diagnostic[512];
 Builder():polygons(0),polygonCount(0),references(0),pointCount(0),points(0),fitPoints(0),error(0){cachedCenter.x=cachedCenter.y=cachedCenter.z=translation.x=translation.y=translation.z=0;for(unsigned int i=0;i<4;i++)sphere[i]=0;for(unsigned int i=0;i<3;i++)body[i]=0;diagnostic[0]=0;}
 void Clear(){memory.Clear();polygons=0;polygonCount=references=pointCount=0;points=fitPoints=0;cachedCenter.x=cachedCenter.y=cachedCenter.z=translation.x=translation.y=translation.z=0;for(unsigned int i=0;i<4;i++)sphere[i]=0;for(unsigned int i=0;i<3;i++)body[i]=0;error=0;diagnostic[0]=0;}
 // Already localized source polygons, with the separately established cached
 // model center. This interface is also used for bounded writer controls.
 bool BuildPrepared(const Polygon* input,unsigned int count,const WorldText::Vec3& center,float mass,float density){
  Clear();if((count&&!input)||count>100000||!WorldText::FiniteVector(center))return Fail("Invalid prepared automatic Sphere input");
  unsigned int total=0;
  for(unsigned int p=0;p<count;p++){if(input[p].count<3||input[p].count>65535||!input[p].points||input[p].count>1000000-total)return Fail("Invalid prepared automatic Sphere polygon");total+=input[p].count;}
  polygons=(Polygon*)memory.Allocate((count+1)*sizeof(Polygon));if(!polygons)return Fail("Prepared automatic Sphere polygon allocation failed");
  polygonCount=count;references=total;cachedCenter=center;
  for(unsigned int p=0;p<count;p++){
   WorldText::Vec3* copied=(WorldText::Vec3*)memory.Allocate(input[p].count*sizeof(WorldText::Vec3));if(!copied)return Fail("Prepared automatic Sphere corner allocation failed");
   for(unsigned int i=0;i<input[p].count;i++){if(!WorldText::FiniteVector(input[p].points[i]))return Fail("Nonfinite prepared automatic Sphere corner");copied[i]=input[p].points[i];}
   polygons[p].points=copied;polygons[p].count=input[p].count;polygons[p].flags=input[p].flags;
  }
  return Finish(mass,density);
 }
 bool Build(const PrefabScene::Model& model,const WorldModels::Partition& partition,unsigned int group,const WorldText::Vec3& offset,float mass,float density){
  Clear();if(partition.error||!partition.groups||!group||group>=partition.groupCount||!partition.groups[group].owner||(!partition.assignments&&partition.assignmentCount)||!WorldText::FiniteVector(offset))return Fail("Invalid automatic Sphere source group");
  const PrefabScene::Node& owner=*partition.groups[group].owner;const bool navigation=SourcePhysicalBspContours::HasNavigation(model);
  unsigned int count=0,total=0;
  for(unsigned int a=0;a<partition.assignmentCount;a++){
   const WorldModels::Assignment& item=partition.assignments[a];if(item.group!=group)continue;
   if(!item.brush||item.brush->kind!=1||!item.brush->knownType)return Fail("Invalid automatic Sphere source brush");
   if(!SourcePhysicalBspContours::DiskCandidate(*item.brush,navigation))continue;const WorldText::Geometry& geometry=item.brush->geometry;
   if((geometry.points&&!geometry.vertices)||(geometry.polygons&&!geometry.faces))return Fail("Invalid automatic Sphere source geometry");
   for(unsigned int p=0;p<geometry.polygons;p++){
    unsigned int fragments=SourceSurfacePreparation::FragmentCount(geometry.faces[p].count);if(!fragments||fragments>100000-count)return Fail("Automatic Sphere source polygon limit");count+=fragments;
    for(unsigned int f=0;f<fragments;f++){SourceSurfacePreparation::Fragment fragment;if(!SourceSurfacePreparation::FragmentAt(geometry.faces[p],f,fragment)||fragment.count>1000000-total)return Fail("Automatic Sphere source corner limit");total+=fragment.count;for(unsigned int i=0;i<fragment.count;i++)if(fragment.indices[i]>=geometry.points)return Fail("Automatic Sphere source point index");}
   }
  }
  polygons=(Polygon*)memory.Allocate((count+1)*sizeof(Polygon));if(!polygons)return Fail("Automatic Sphere source polygon allocation failed");polygonCount=count;references=total;
  PhysicsOwnerBounds::Bounds all;unsigned int at=0;
  for(unsigned int a=0;a<partition.assignmentCount;a++){
   const WorldModels::Assignment& item=partition.assignments[a];if(item.group!=group||!SourcePhysicalBspContours::DiskCandidate(*item.brush,navigation))continue;const WorldText::Geometry& geometry=item.brush->geometry;
   for(unsigned int p=0;p<geometry.polygons;p++)for(unsigned int f=0;f<SourceSurfacePreparation::FragmentCount(geometry.faces[p].count);f++){
    SourceSurfacePreparation::Fragment fragment;if(!SourceSurfacePreparation::FragmentAt(geometry.faces[p],f,fragment))return Fail("Automatic Sphere source fragment changed");
    WorldText::Vec3* corners=(WorldText::Vec3*)memory.Allocate(fragment.count*sizeof(WorldText::Vec3));if(!corners)return Fail("Automatic Sphere source corner allocation failed");
    for(unsigned int i=0;i<fragment.count;i++)if(!SourceOwnerRotation::Local(owner,geometry.vertices[fragment.indices[i]],offset,corners[i])||!all.Add(corners[i]))return Fail("Automatic Sphere source local point preparation failed");
    polygons[at].points=corners;polygons[at].count=fragment.count;polygons[at].flags=item.brush->surfaceFlags;at++;
   }
  }
  if(at!=count)return Fail("Automatic Sphere source polygon enumeration mismatch");
  // Cached center is established before blocker extraction, from every source
  // candidate corner. Preserve the float sum store before halving each axis.
  if(all.count)for(unsigned int k=0;k<3;k++){volatile float sum=(&all.low.x)[k]+(&all.high.x)[k];(&cachedCenter.x)[k]=sum*.5f;}
  return Finish(mass,density);
 }
};
}
