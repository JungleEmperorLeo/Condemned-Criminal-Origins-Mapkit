#pragma once
#include "SourceRenderPlane.h"
#include "SourcePhysicalBspContours.h"
#include "PhysicsOwnerBounds.h"
#include "SourcePhysicsVertexIndex.h"
// Shared source-owned collision input for automatic Hull and future owner Mesh.
// This is collection/indexing only: no convex construction, mass properties,
// MOPP, original record, or original DLL is an input. Empty collection is valid;
// accepting a writer's empty-record framing is a separate responsibility.
namespace SourcePhysicsOwnerVertices {
enum ErrorCode {NoError=0,InvalidInput,InvalidSettings,InvalidGeometry,NonfinitePoint,ResourceLimit,AllocationFailure,EnumerationFailure};
struct Settings {
 unsigned int version,maximumPreparedPolygons,maximumReferences,maximumUniqueVertices;
 bool generateIndices;
 // 65,535 is the tested native indexer profile, not a claimed original Hull
 // limit. All input/output indices remain u32; disk Mesh conversion is separate.
 Settings():version(1),maximumPreparedPolygons(100000),maximumReferences(1000000),maximumUniqueVertices(65535),generateIndices(false){}
};
struct Polygon {
 const WorldText::Vec3* points;unsigned int count,flags;
 unsigned int assignment,node,sourcePolygon,fragment,firstSourceCorner;
};
struct Corner {
 unsigned int polygon,corner,assignment,node,sourcePolygon,fragment,sourceCorner,sourceVertex;
};
class Builder {
 PrefabScene::Arena memory;
 Builder(const Builder&);Builder& operator=(const Builder&);
 bool Fail(ErrorCode code,const char* message,unsigned int assignment=0xffffffffu,unsigned int polygon=0xffffffffu){char saved[512];lstrcpynA(saved,message?message:"Source collision input failed",512);Clear();lstrcpyA(diagnostic,saved);error=diagnostic;errorCode=code;failedAssignment=assignment;failedPolygon=polygon;return false;}
 static bool Valid(const Settings& settings){return settings.version==1&&settings.maximumPreparedPolygons&&settings.maximumPreparedPolygons<=100000&&settings.maximumReferences&&settings.maximumReferences<=1000000&&settings.maximumUniqueVertices&&settings.maximumUniqueVertices<=65535;}
  static bool Collision(const Polygon& polygon,bool removeBlockers){return (!removeBlockers||!(polygon.flags&0x180))&&(polygon.flags&0x10)&&polygon.count>=3&&polygon.count<=30;}
  bool Finish(const Settings& settings,bool removeBlockers){
  if(!WorldText::FiniteVector(cachedCenter))return Fail(NonfinitePoint,"Nonfinite source collision cached center");
  unsigned int wanted=0,wantedIndices=0;
  for(unsigned int p=0;p<polygonCount;p++){
   const Polygon& polygon=polygons[p];
    if(removeBlockers&&(polygon.flags&0x180)){removedBlockerPolygons++;continue;}
   if(!(polygon.flags&0x10)){nonphysicalPolygons++;continue;}
   if(polygon.count<3||polygon.count>30){skippedPreparedPolygons++;continue;}
   if(polygon.count>settings.maximumReferences-wanted)return Fail(ResourceLimit,"Source collision corner limit",polygon.assignment,polygon.sourcePolygon);wanted+=polygon.count;physicsPolygons++;
   if(settings.generateIndices)wantedIndices+=(polygon.count-2)*3;
  }
  physicsReferences=wanted;indexCount=wantedIndices;mainWorldFlag=mainWorld?1u:0u;
  if(!wanted)return true;
  cornerVertexIndices=(unsigned int*)memory.Allocate(wanted*4);cornerProvenance=(Corner*)memory.Allocate(wanted*sizeof(Corner));
  representativeFirstCorner=(unsigned int*)memory.Allocate((wanted<settings.maximumUniqueVertices?wanted:settings.maximumUniqueVertices)*4);
  if(settings.generateIndices)indices=(unsigned int*)memory.Allocate(wantedIndices*4);
  if(!cornerVertexIndices||!cornerProvenance||!representativeFirstCorner||(settings.generateIndices&&!indices))return Fail(AllocationFailure,"Source collision provenance allocation failed");
  SourcePhysicsVertexIndex::Indexer indexer;const unsigned int capacity=wanted<settings.maximumUniqueVertices?wanted:settings.maximumUniqueVertices;
  if(!indexer.Begin(capacity,.01f))return Fail(AllocationFailure,"Source collision vertex tree allocation failed");
  unsigned int at=0,indexAt=0;
  for(unsigned int p=0;p<polygonCount;p++){
    const Polygon& polygon=polygons[p];if(!Collision(polygon,removeBlockers))continue;const unsigned int first=at;
   for(unsigned int i=0;i<polygon.count;i++){
    WorldText::Vec3 point=polygon.points[i];
    // Original C410 skips this subtraction only for MAIN world flag bit 1.
    if(!mainWorld){point.x-=cachedCenter.x;point.y-=cachedCenter.y;point.z-=cachedCenter.z;}
    if(!WorldText::FiniteVector(point))return Fail(NonfinitePoint,"Nonfinite centered source collision corner",polygon.assignment,polygon.sourcePolygon);
    const unsigned int before=indexer.count;unsigned int vertex=0;if(!indexer.Insert(point,vertex))return Fail(ResourceLimit,"Source collision unique vertex limit",polygon.assignment,polygon.sourcePolygon);
    cornerVertexIndices[at]=vertex;if(indexer.count!=before)representativeFirstCorner[vertex]=at;
    Corner& provenance=cornerProvenance[at];provenance.polygon=p;provenance.corner=i;provenance.assignment=polygon.assignment;provenance.node=polygon.node;provenance.sourcePolygon=polygon.sourcePolygon;provenance.fragment=polygon.fragment;
    provenance.sourceCorner=0xffffffffu;provenance.sourceVertex=0xffffffffu;
    if(sourceCorners&&polygon.firstSourceCorner!=0xffffffffu){provenance.sourceCorner=sourceCorners[polygon.firstSourceCorner+i];provenance.sourceVertex=sourceVertices[polygon.firstSourceCorner+i];}
    at++;
   }
   if(settings.generateIndices)for(unsigned int i=2;i<polygon.count;i++){indices[indexAt++]=cornerVertexIndices[first];indices[indexAt++]=cornerVertexIndices[first+i-1];indices[indexAt++]=cornerVertexIndices[first+i];}
  }
  if(at!=wanted||indexAt!=wantedIndices)return Fail(EnumerationFailure,"Source collision reference enumeration mismatch");
  pointCount=indexer.count;rawVertices=(WorldText::Vec3*)memory.Allocate(pointCount*12);scaledVertices=(WorldText::Vec3*)memory.Allocate(pointCount*12);if(!rawVertices||!scaledVertices)return Fail(AllocationFailure,"Source collision final vertex allocation failed");
  for(unsigned int i=0;i<pointCount;i++){
   rawVertices[i]=indexer.Point(i);scaledVertices[i].x=rawVertices[i].x*.01f;scaledVertices[i].y=rawVertices[i].y*.01f;scaledVertices[i].z=rawVertices[i].z*.01f;
   if(!WorldText::FiniteVector(scaledVertices[i]))return Fail(NonfinitePoint,"Nonfinite scaled collision representative");
  }
  weldedReferences=wanted-pointCount;return true;
 }
public:
 Polygon* polygons;unsigned int polygonCount,preparedReferences,sourceBrushes,sourcePolygons;
 unsigned int physicsPolygons,physicsReferences,removedBlockerPolygons,nonphysicalPolygons,skippedPreparedPolygons;
 unsigned int pointCount,weldedReferences,indexCount,mainWorldFlag;
 unsigned int* cornerVertexIndices;Corner* cornerProvenance;unsigned int* representativeFirstCorner;unsigned int* indices;
 // Prepared positions precede center subtraction. rawVertices are unscaled,
 // welded collision representatives after the MAIN/owner centering policy.
 WorldText::Vec3* rawVertices;WorldText::Vec3* scaledVertices;WorldText::Vec3 cachedCenter;
 unsigned int* sourceCorners;unsigned int* sourceVertices;bool mainWorld;ErrorCode errorCode;unsigned int failedAssignment,failedPolygon;const char* error;char diagnostic[512];
 Builder():polygons(0),polygonCount(0),preparedReferences(0),sourceBrushes(0),sourcePolygons(0),physicsPolygons(0),physicsReferences(0),removedBlockerPolygons(0),nonphysicalPolygons(0),skippedPreparedPolygons(0),pointCount(0),weldedReferences(0),indexCount(0),mainWorldFlag(0),cornerVertexIndices(0),cornerProvenance(0),representativeFirstCorner(0),indices(0),rawVertices(0),scaledVertices(0),sourceCorners(0),sourceVertices(0),mainWorld(false),errorCode(NoError),failedAssignment(0xffffffffu),failedPolygon(0xffffffffu),error(0){cachedCenter.x=cachedCenter.y=cachedCenter.z=0;diagnostic[0]=0;}
 void Clear(){memory.Clear();polygons=0;polygonCount=preparedReferences=sourceBrushes=sourcePolygons=physicsPolygons=physicsReferences=removedBlockerPolygons=nonphysicalPolygons=skippedPreparedPolygons=pointCount=weldedReferences=indexCount=mainWorldFlag=0;cornerVertexIndices=representativeFirstCorner=indices=sourceCorners=sourceVertices=0;cornerProvenance=0;rawVertices=scaledVertices=0;cachedCenter.x=cachedCenter.y=cachedCenter.z=0;mainWorld=false;errorCode=NoError;failedAssignment=failedPolygon=0xffffffffu;error=0;diagnostic[0]=0;}
 // Direct prepared CPoly controls. Counts outside 3..30 are retained as input
 // metadata but skipped by C410 indexing; raw source Build fragments first.
  // This boundary follows blocker extraction: C410 itself tests only bit0x10.
 bool BuildPrepared(const Polygon* input,unsigned int count,const WorldText::Vec3& center,bool isMainWorld,const Settings& settings=Settings()){
  Clear();if(!Valid(settings))return Fail(InvalidSettings,"Invalid source collision input settings");if((count&&!input)||count>settings.maximumPreparedPolygons||!WorldText::FiniteVector(center))return Fail(InvalidInput,"Invalid prepared source collision input");
  unsigned int total=0;for(unsigned int p=0;p<count;p++){if(input[p].count>65535||(input[p].count&&!input[p].points)||input[p].count>settings.maximumReferences-total)return Fail(InvalidGeometry,"Invalid prepared collision polygon",input[p].assignment,input[p].sourcePolygon);total+=input[p].count;}
  polygons=(Polygon*)memory.Allocate((count+1)*sizeof(Polygon));if(!polygons)return Fail(AllocationFailure,"Prepared source collision polygon allocation failed");polygonCount=count;preparedReferences=total;sourcePolygons=count;cachedCenter=center;mainWorld=isMainWorld;
  for(unsigned int p=0;p<count;p++){
   WorldText::Vec3* copied=input[p].count?(WorldText::Vec3*)memory.Allocate(input[p].count*12):0;if(input[p].count&&!copied)return Fail(AllocationFailure,"Prepared source collision corner allocation failed");
   for(unsigned int i=0;i<input[p].count;i++){if(!WorldText::FiniteVector(input[p].points[i]))return Fail(NonfinitePoint,"Nonfinite prepared source collision corner",input[p].assignment,input[p].sourcePolygon);copied[i]=input[p].points[i];}
   polygons[p]=input[p];polygons[p].points=copied;polygons[p].firstSourceCorner=0xffffffffu;
  }
   return Finish(settings,false);
 }
 bool Build(const PrefabScene::Model& model,const WorldModels::Partition& partition,unsigned int group,const WorldText::Vec3& offset,const Settings& settings=Settings()){
  Clear();if(!Valid(settings))return Fail(InvalidSettings,"Invalid source collision input settings");if(partition.error||!partition.groups||group>=partition.groupCount||(!partition.assignments&&partition.assignmentCount)||(!group?partition.groups[0].owner!=0:!partition.groups[group].owner)||!WorldText::FiniteVector(offset))return Fail(InvalidInput,"Invalid source collision input group");
  mainWorld=group==0;const PrefabScene::Node* owner=mainWorld?0:partition.groups[group].owner;const bool navigation=SourcePhysicalBspContours::HasNavigation(model);
  unsigned int count=0,total=0;
  for(unsigned int a=0;a<partition.assignmentCount;a++){
   const WorldModels::Assignment& item=partition.assignments[a];if(item.group!=group)continue;if(!item.brush||item.brush->kind!=1||!item.brush->knownType)return Fail(InvalidGeometry,"Invalid source collision brush",a);
   if(!SourcePhysicalBspContours::DiskCandidate(*item.brush,navigation))continue;const WorldText::Geometry& geometry=item.brush->geometry;
   if((geometry.points&&!geometry.vertices)||(geometry.polygons&&!geometry.faces))return Fail(InvalidGeometry,"Invalid source collision geometry",a);if(geometry.points>1000000||geometry.polygons>settings.maximumPreparedPolygons-sourcePolygons)return Fail(ResourceLimit,"Source collision source geometry limit",a);sourceBrushes++;sourcePolygons+=geometry.polygons;
   for(unsigned int p=0;p<geometry.polygons;p++){
    const unsigned int fragments=SourceSurfacePreparation::FragmentCount(geometry.faces[p].count);if(!fragments||fragments>settings.maximumPreparedPolygons-count)return Fail(ResourceLimit,"Source collision prepared polygon limit",a,p);count+=fragments;
    for(unsigned int f=0;f<fragments;f++){SourceSurfacePreparation::Fragment fragment;if(!SourceSurfacePreparation::FragmentAt(geometry.faces[p],f,fragment)||fragment.count>settings.maximumReferences-total)return Fail(ResourceLimit,"Source collision prepared corner limit",a,p);total+=fragment.count;for(unsigned int i=0;i<fragment.count;i++)if(fragment.indices[i]>=geometry.points)return Fail(InvalidGeometry,"Source collision point index",a,p);}
   }
  }
  polygons=(Polygon*)memory.Allocate((count+1)*sizeof(Polygon));sourceCorners=(unsigned int*)memory.Allocate((total+1)*4);sourceVertices=(unsigned int*)memory.Allocate((total+1)*4);if(!polygons||!sourceCorners||!sourceVertices)return Fail(AllocationFailure,"Source collision prepared tables allocation failed");polygonCount=count;preparedReferences=total;
  PhysicsOwnerBounds::Bounds bounds;unsigned int at=0,cornerAt=0;
  for(unsigned int a=0;a<partition.assignmentCount;a++){
   const WorldModels::Assignment& item=partition.assignments[a];if(item.group!=group||!SourcePhysicalBspContours::DiskCandidate(*item.brush,navigation))continue;const WorldText::Geometry& geometry=item.brush->geometry;
   for(unsigned int p=0;p<geometry.polygons;p++)for(unsigned int f=0;f<SourceSurfacePreparation::FragmentCount(geometry.faces[p].count);f++){
    SourceSurfacePreparation::Fragment fragment;if(!SourceSurfacePreparation::FragmentAt(geometry.faces[p],f,fragment))return Fail(EnumerationFailure,"Source collision fragment changed",a,p);WorldText::Vec3* corners=(WorldText::Vec3*)memory.Allocate(fragment.count*12);if(!corners)return Fail(AllocationFailure,"Source collision fragment allocation failed",a,p);
    Polygon& output=polygons[at];output.points=corners;output.count=fragment.count;output.flags=item.brush->surfaceFlags;output.assignment=a;output.node=item.brush->index;output.sourcePolygon=p;output.fragment=f;output.firstSourceCorner=cornerAt;
    const unsigned int start=f?30+(f-1)*28:0;
    for(unsigned int i=0;i<fragment.count;i++){
     const WorldText::Vec3& original=geometry.vertices[fragment.indices[i]];
     if(owner){if(!SourceOwnerRotation::Local(*owner,original,offset,corners[i]))return Fail(NonfinitePoint,"Source collision owner local point preparation failed",a,p);}else{corners[i]=original;corners[i].x-=offset.x;corners[i].y-=offset.y;corners[i].z-=offset.z;}
     if(!bounds.Add(corners[i]))return Fail(NonfinitePoint,"Nonfinite source collision prepared point",a,p);
     sourceVertices[cornerAt]=fragment.indices[i];sourceCorners[cornerAt]=!f?i:!i?0:i==1?start-1:start+i-2;cornerAt++;
    }
    at++;
   }
  }
  if(at!=count||cornerAt!=total)return Fail(EnumerationFailure,"Source collision prepared enumeration mismatch");
  if(bounds.count)for(unsigned int k=0;k<3;k++){volatile float sum=(&bounds.low.x)[k]+(&bounds.high.x)[k];(&cachedCenter.x)[k]=sum*.5f;}
   return Finish(settings,true);
 }
};
}
