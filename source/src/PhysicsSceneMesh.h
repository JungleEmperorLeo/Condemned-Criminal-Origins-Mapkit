#pragma once
#include "PhysicsMeshPrefix.h"
namespace PhysicsSceneMesh {
class MainWorld {
 PrefabScene::Arena memory;
 bool Fail(const char* msg){memory.Clear();prefix.memory.Clear();prefix.data=0;prefix.size=0;error=msg;return false;}
public:
 PhysicsMeshPrefix::Block prefix;const char* error;
 MainWorld():error(0){}
 bool Build(const WorldModels::Partition& partition,WorldText::Vec3 worldOffset=WorldText::Vec3{},bool allowNonPlanarFan=false,float weldTolerance=0){
  memory.Clear();prefix.memory.Clear();prefix.data=0;prefix.size=0;error=0;if(!WorldText::Finite(weldTolerance)||weldTolerance<0)return Fail("Invalid physics weld tolerance");unsigned int points=0,polygons=0,references=0;
  for(unsigned int i=0;i<partition.assignmentCount;i++){const WorldModels::Assignment& a=partition.assignments[i];if(a.group)continue;if(!a.brush->knownType)return Fail("Unknown physics brush type");if(!(a.brush->surfaceFlags&0x10))continue;const WorldText::Geometry& g=a.brush->geometry;points+=g.points;polygons+=g.polygons;references+=g.references;if(points>=65536||polygons>100000||references>1000000)return Fail("Physics source size limit exceeded");}
  if(!points||!polygons)return Fail("No main-world physics geometry");WorldText::Geometry combined={};combined.vertices=(WorldText::Vec3*)memory.Allocate(points*12);combined.faces=(WorldText::Polygon*)memory.Allocate(polygons*sizeof(WorldText::Polygon));if(!combined.vertices||!combined.faces)return Fail("Physics geometry allocation failed");
  for(unsigned int i=0;i<partition.assignmentCount;i++){
   const WorldModels::Assignment& a=partition.assignments[i];if(a.group||!(a.brush->surfaceFlags&0x10))continue;const WorldText::Geometry& g=a.brush->geometry;unsigned int* remap=(unsigned int*)memory.Allocate(g.points*4);if(!remap)return Fail("Physics remap allocation failed");
   for(unsigned int j=0;j<g.points;j++){bool used=false;for(unsigned int face=0;face<g.polygons&&!used;face++)for(unsigned int k=0;k<g.faces[face].count;k++)if(g.faces[face].indices[k]==j){used=true;break;}if(!used){remap[j]=0xffffffffu;continue;}
    const WorldText::Vec3& p=g.vertices[j];unsigned int found=0;for(;found<combined.points;found++){const WorldText::Vec3& q=combined.vertices[found];float dx=p.x-q.x,dy=p.y-q.y,dz=p.z-q.z;
    if((p.x==q.x&&p.y==q.y&&p.z==q.z)||(weldTolerance>0&&dx*dx+dy*dy+dz*dz<weldTolerance*weldTolerance))break;}if(found==combined.points)combined.vertices[combined.points++]=p;remap[j]=found;}
   for(unsigned int j=0;j<g.polygons;j++){if(PolygonCheck::Check(g,g.faces[j]).issues&~(allowNonPlanarFan?(unsigned int)PolygonCheck::NonPlanar:0u))return Fail("Invalid physics contour");WorldText::Polygon& p=combined.faces[combined.polygons++];p=g.faces[j];p.indices=(unsigned int*)memory.Allocate(p.count*4);if(!p.indices)return Fail("Physics index allocation failed");for(unsigned int k=0;k<p.count;k++)p.indices[k]=remap[g.faces[j].indices[k]];combined.references+=p.count;}
  }
  if(!WorldText::FiniteVector(worldOffset))return Fail("Nonfinite physics world offset");
  for(unsigned int i=0;i<combined.points;i++){combined.vertices[i].x-=worldOffset.x;combined.vertices[i].y-=worldOffset.y;combined.vertices[i].z-=worldOffset.z;}
  for(unsigned int i=0;i<combined.polygons;i++)combined.faces[i].distance-=WorldText::Dot(combined.faces[i].normal,worldOffset);
  // Static main-world profile. Owner shapes are retained
  // by the diagnostic world assembler; this does not implement all physics types.
  if(!prefix.BuildStatic(combined,.01f,1.f,1.1f,allowNonPlanarFan))return Fail("Could not build source physics mesh prefix");return true;
 }
};
}
