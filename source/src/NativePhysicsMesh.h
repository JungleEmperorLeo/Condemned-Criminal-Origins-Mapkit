#pragma once
#include "NativePreparedRenderScene.h"
#include "PhysicsMeshPrefix.h"
#include "SourcePhysicsVertexIndex.h"
// Generic static world collision geometry from the same source-owned scene.
// Corner insertion, vertex set identity and fan order follow the original. MOPP is
// intentionally a separate caller-owned backend; no compiled world is read.
namespace NativePhysicsMesh {
struct Settings {
 unsigned int version,maximumSourcePoints,maximumReferences;float weldTolerance;
 bool allowNonPlanarFan;
 Settings():version(1),maximumSourcePoints(1000000),maximumReferences(3000000),weldTolerance(.01f),allowNonPlanarFan(true){}
};
class Builder {
 PrefabScene::Arena memory;PhysicsMeshPrefix::Block output;
 Builder(const Builder&);Builder& operator=(const Builder&);
 bool Fail(const char* message){Clear();error=message;return false;}
public:
 unsigned char* data;unsigned int size,sourcePoints,sourcePolygons,points,indices;const char* error;
 Builder():data(0),size(0),sourcePoints(0),sourcePolygons(0),points(0),indices(0),error(0){}
 void Clear(){memory.Clear();output.memory.Clear();output.data=0;output.size=0;data=0;size=sourcePoints=sourcePolygons=points=indices=0;error=0;}
 const PhysicsMeshPrefix::Block& Prefix()const{return output;}
 bool Build(const NativePreparedRenderScene::Context& scene,const Settings& settings=Settings()){
  Clear();if(!scene.Ready()||settings.version!=1||!WorldText::Finite(settings.weldTolerance)||settings.weldTolerance<0||!settings.maximumSourcePoints||settings.maximumSourcePoints>1000000||!settings.maximumReferences||settings.maximumReferences>3000000)return Fail("Invalid source physics mesh settings");
  const WorldModels::Partition& partition=scene.Partition();const WorldText::Vec3& offset=scene.Offset();unsigned int references=0;
  for(unsigned int a=0;a<partition.assignmentCount;a++){const WorldModels::Assignment& item=partition.assignments[a];if(item.group||!(item.brush->surfaceFlags&0x10))continue;const WorldText::Geometry& geometry=item.brush->geometry;
   if(item.brush->knownType&&!geometry.polygons)continue; // emptied by the per-file weld/cleanup (0x10059550): no collision faces
   if(!item.brush->knownType||!geometry.vertices||!geometry.faces||!geometry.points||!geometry.polygons||geometry.points>settings.maximumSourcePoints-sourcePoints||geometry.references>settings.maximumReferences-references||geometry.polygons>100000-sourcePolygons)return Fail("Invalid source collision geometry bounds");sourcePoints+=geometry.points;sourcePolygons+=geometry.polygons;references+=geometry.references;
  }
  if(!sourcePoints||!sourcePolygons)return Fail("No source MAIN collision geometry");
  WorldText::Geometry combined={};combined.vertices=(WorldText::Vec3*)memory.Allocate((sourcePoints+1)*12);combined.faces=(WorldText::Polygon*)memory.Allocate(sourcePolygons*sizeof(WorldText::Polygon));if(!combined.vertices||!combined.faces)return Fail("Source collision geometry allocation failed");
   SourcePhysicsVertexIndex::Indexer vertexIndex;if(!vertexIndex.Begin(sourcePoints,settings.weldTolerance))return Fail("Source collision vertex index allocation failed");
   for(unsigned int a=0;a<partition.assignmentCount;a++){
   const WorldModels::Assignment& item=partition.assignments[a];if(item.group||!(item.brush->surfaceFlags&0x10))continue;const WorldText::Geometry& geometry=item.brush->geometry;
   for(unsigned int p=0;p<geometry.polygons;p++){
     const WorldText::Polygon& face=geometry.faces[p];if(!face.indices||face.count<3||face.count>4096||(PolygonCheck::Check(geometry,face).issues&~PhysicsMeshPrefix::FanIssues(settings.allowNonPlanarFan)))return Fail("Invalid source collision contour"); // fan triangulation does not require convexity (SDK Asylum)
     WorldText::Polygon& out=combined.faces[combined.polygons++];out=face;out.indices=(unsigned int*)memory.Allocate(face.count*4);if(!out.indices)return Fail("Source collision face references allocation failed");
     for(unsigned int v=0;v<face.count;v++){if(face.indices[v]>=geometry.points)return Fail("Source collision point index outside brush");WorldText::Vec3 point=geometry.vertices[face.indices[v]];point.x-=offset.x;point.y-=offset.y;point.z-=offset.z;if(!vertexIndex.Insert(point,out.indices[v]))return Fail("Collision vertex indexing failed or exceeded U16 capacity");}combined.references+=face.count;
   }
  }
   combined.points=vertexIndex.count;for(unsigned int v=0;v<combined.points;v++)combined.vertices[v]=vertexIndex.Point(v);
  for(unsigned int p=0;p<combined.polygons;p++)combined.faces[p].distance-=WorldText::Dot(combined.faces[p].normal,offset);
  if(!output.BuildStatic(combined,.01f,1.f,1.1f,settings.allowNonPlanarFan))return Fail("Source collision mesh prefix compilation failed");
  data=output.data;size=output.size;points=combined.points;indices=*(unsigned int*)(data+104);return true;
 }
};
}
