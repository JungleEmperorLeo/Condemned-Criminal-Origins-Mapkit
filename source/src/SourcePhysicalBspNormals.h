#pragma once
#include "SourceBlockerPlanePool.h"
#include "SourcePhysicalBspContours.h"
#include "PhysicalNormalOrder.h"
// Physical save table is distinct from the blocker-extraction plane pool:
// Authored owners enter local coordinates when contours remain after blocker
// extraction. Blocker-only owners retain their initial world surfaces; loaded
// prefab owners are already local. Blocker surfaces participate in counts.
#ifdef FEAR_PHYSNORMAL_TRACE
void FearPhysNormalTrace(unsigned int,unsigned int,unsigned int,unsigned int,const WorldText::Vec3&);
void FearNodeInfo(const PrefabScene::Model&,const PrefabScene::Node&);
#endif
namespace SourcePhysicalBspNormals {
struct Surface {unsigned int assignment,polygon,initialIdentity,finalIdentity;bool active;};
class Table {
 PrefabScene::Arena memory;
 Table(const Table&);Table& operator=(const Table&);
 bool Fail(const char* reason){memory.Clear();normals=0;uses=0;initialNormals=0;initialUses=0;sourceSurfaces=0;count=initialCount=surfaces=surfaceCount=0;error=reason;return false;}
public:
 WorldText::Vec3* normals;unsigned int* uses;unsigned int count,initialCount,surfaces;const char* error;
 const WorldText::Vec3* initialNormals;unsigned int* initialUses;Surface* sourceSurfaces;unsigned int surfaceCount;
 Table():normals(0),uses(0),count(0),initialCount(0),surfaces(0),error(0),initialNormals(0),initialUses(0),sourceSurfaces(0),surfaceCount(0){}
 bool Build(const PrefabScene::Model& model,const WorldModels::Partition& partition,const WorldText::Vec3& offset){
  Fail(0);SourceBlockerPlanePool::Prepared preliminary;if(!preliminary.Build(model,partition,offset))return Fail(preliminary.error);
  unsigned int faces=0;for(unsigned int a=0;a<partition.assignmentCount;a++)if(preliminary.EmitGroup(partition.assignments[a].group)&&SourceRenderPointTags::MainWorldCandidate(*partition.assignments[a].brush)){
   unsigned int n=partition.assignments[a].brush->geometry.polygons;if(n>400000-faces)return Fail("Physical normal surface limit exceeded");faces+=n;
  }
  SourceRenderPlanePool::Pool pool;if(!pool.Initialize(faces*2+18))return Fail("Physical normal pool allocation failed");
  float diagonal;unsigned int bits=0x3fb504f3u;memcpy(&diagonal,&bits,4);
  const WorldText::Vec3 seeds[18]={{0,0,1},{0,0,-1},{0,1,0},{0,-1,0},{1,0,0},{-1,0,0},
   {0,diagonal,diagonal},{0,diagonal,-diagonal},{0,-diagonal,-diagonal},{0,-diagonal,diagonal},
   {diagonal,0,diagonal},{diagonal,0,-diagonal},{-diagonal,0,-diagonal},{-diagonal,0,diagonal},
   {diagonal,diagonal,0},{diagonal,-diagonal,0},{-diagonal,-diagonal,0},{-diagonal,diagonal,0}};
  WorldText::Vec3 retained;unsigned int identity=0;for(unsigned int i=0;i<18;i++)if(!pool.Get(seeds[i],retained,identity)||identity!=i)return Fail("Physical normal seed identity failed");
  sourceSurfaces=(Surface*)memory.Allocate((faces+1)*sizeof(Surface));if(!sourceSurfaces)return Fail("Physical normal surface provenance allocation failed");
  PhysicalNormalOrder::Entry* order=(PhysicalNormalOrder::Entry*)memory.Allocate((faces*2+18)*sizeof(PhysicalNormalOrder::Entry));
  WorldText::Vec3* vectors=(WorldText::Vec3*)memory.Allocate((faces*2+18)*12);
  if(!order||!vectors)return Fail("Physical normal cleanup allocation failed");
  unsigned char* localizedOwners=(unsigned char*)memory.Allocate(partition.groupCount+1);if(!localizedOwners)return Fail("Physical owner eligibility allocation failed");
  for(unsigned int a=0;a<partition.assignmentCount;a++)if(SourceRenderPointTags::MainWorldCandidate(*partition.assignments[a].brush)&&!(partition.assignments[a].brush->surfaceFlags&0x180))localizedOwners[partition.assignments[a].group]=1;
#ifdef FEAR_PHYSNORMAL_TRACE
  for(unsigned int group=1;group<partition.groupCount;group++){const PrefabScene::Node* owner=partition.groups[group].owner;float inverse[4];if(!owner||!SourceOwnerRotation::Inverse(*owner,inverse))continue;
   for(unsigned int a=0;a<partition.assignmentCount;a++){const WorldModels::Assignment& item=partition.assignments[a];if(item.group!=group||!SourceRenderPointTags::MainWorldCandidate(*item.brush))continue;
    PrefabScene::Arena traceMemory;WorldText::Geometry localGeometry=item.brush->geometry;localGeometry.vertices=(WorldText::Vec3*)traceMemory.Allocate(localGeometry.points*12+12);bool localOk=localGeometry.vertices!=0;
    for(unsigned int v=0;localOk&&v<localGeometry.points;v++)localOk=SourceOwnerRotation::Local(*owner,item.brush->geometry.vertices[v],offset,localGeometry.vertices[v]);
    for(unsigned int p=0;p<item.brush->geometry.polygons;p++){WorldText::Vec3 world,normal;if(!SourceRenderNormals::Face(item.brush->geometry,item.brush->geometry.faces[p],world,&offset))continue;WorldText::Vec3 local=SourceWorldModelTransform::Rotate(inverse,world);if(!SourceRenderPlane::Build(local,normal))continue;FearPhysNormalTrace(group,preliminary.EmitGroup(group)?1u:0u,owner->index,p,normal);
     WorldText::Vec3 fromLocal,key2;if(localOk&&SourceRenderNormals::Face(localGeometry,localGeometry.faces[p],fromLocal,0)&&SourceRenderPlane::Build(fromLocal,key2))FearPhysNormalTrace(group,2u|(preliminary.EmitGroup(group)?1u:0u),owner->index,p,key2);}}}
#endif
  // Within each emitted group, surfaces of carvers that Nav deletes enter the pool
  // after the other brushes (original sort input at call 0x1002659E: Cafeteria all
  // 4213 identities equal; Office, without Nav, keeps its carver in place). Counts at
  // 0x100264E0 include only polygons still alive; deleted normals keep identities.
  const bool navigation=SourcePhysicalBspContours::HasNavigation(model);
  for(unsigned int group=0;group<partition.groupCount;group++){
   if(!preliminary.EmitGroup(group))continue;const PrefabScene::Node* owner=group?partition.groups[group].owner:0;
   float inverse[4];if(owner&&!SourceOwnerRotation::Inverse(*owner,inverse))return Fail("Physical owner inverse failed");
   for(unsigned int pass=0;pass<2;pass++)for(unsigned int a=0;a<partition.assignmentCount;a++){
    const WorldModels::Assignment& item=partition.assignments[a];if(item.group!=group||!SourceRenderPointTags::MainWorldCandidate(*item.brush))continue;
    if((pass==0)!=SourcePhysicalBspContours::DiskCandidate(*item.brush,navigation))continue;
    const WorldText::Geometry& geometry=item.brush->geometry;
    for(unsigned int p=0;p<geometry.polygons;p++){WorldText::Vec3 world,normal;if(!SourceRenderNormals::Face(geometry,geometry.faces[p],world,&offset))return Fail("Physical source face normal failed");bool local=owner&&(localizedOwners[group]||owner->importedFromPrefab);if(local)world=SourceWorldModelTransform::Rotate(inverse,world);if(!SourceRenderPlane::Build(world,normal)||!pool.Get(normal,retained,identity))return Fail("Physical prepass normal failed");}
   }
  }
  for(unsigned int group=0;group<partition.groupCount;group++){
   if(!preliminary.EmitGroup(group))continue;const PrefabScene::Node* owner=group?partition.groups[group].owner:0;
   float inverse[4];if(owner&&!SourceOwnerRotation::Inverse(*owner,inverse))return Fail("Physical owner inverse failed");
   for(unsigned int a=0;a<partition.assignmentCount;a++){
    const WorldModels::Assignment& item=partition.assignments[a];if(item.group!=group||!SourceRenderPointTags::MainWorldCandidate(*item.brush))continue;
    const WorldText::Geometry& geometry=item.brush->geometry;
    for(unsigned int p=0;p<geometry.polygons;p++){
     WorldText::Vec3 world,normal;if(!SourceRenderNormals::Face(geometry,geometry.faces[p],world,&offset))return Fail("Physical source face normal failed");
     bool local=owner&&(localizedOwners[group]||owner->importedFromPrefab);
     if(local)world=SourceWorldModelTransform::Rotate(inverse,world);
     if(!SourceRenderPlane::Build(world,normal))return Fail("Physical owner normal preparation failed");
     if(!pool.Get(normal,retained,identity))return Fail("Physical final normal insertion failed");
#ifdef FEAR_PHYSNORMAL_TRACE
     FearPhysNormalTrace(group,4u|identity<<8,item.brush->index,p,normal);if(!p)FearNodeInfo(model,*item.brush);
#endif

     Surface& surface=sourceSurfaces[surfaceCount++];surface.assignment=a;surface.polygon=p;surface.initialIdentity=surface.finalIdentity=identity;surface.active=SourcePhysicalBspContours::DiskCandidate(*item.brush,navigation);
     if(surface.active)order[identity].uses++;surfaces++;
    }
   }
  }
  initialCount=pool.Count();if(!pool.Ordered(vectors,initialCount))return Fail("Physical initial normal export failed");
  initialNormals=vectors;initialUses=(unsigned int*)memory.Allocate((initialCount+1)*4);if(!initialUses||surfaceCount!=faces)return Fail("Physical normal usage/provenance export failed");
  for(unsigned int i=0;i<initialCount;i++)initialUses[i]=order[i].uses;
  for(unsigned int i=0;i<initialCount;i++)order[i].identity=i;
#ifdef FEAR_PHYSNORMAL_TRACE
  for(unsigned int i=0;i<initialCount;i++)FearPhysNormalTrace(0xfffffffeu,order[i].uses,order[i].identity,i,vectors[order[i].identity]);
#endif
  if(!PhysicalNormalOrder::Build(order,initialCount))return Fail("Physical normal ordering failed");
  for(unsigned int i=0;i<initialCount&&order[i].uses;i++)count++;
  normals=(WorldText::Vec3*)memory.Allocate((count+1)*12);uses=(unsigned int*)memory.Allocate((count+1)*4);if(!normals||!uses)return Fail("Physical saved normal allocation failed");
  for(unsigned int i=0;i<count;i++){normals[i]=vectors[order[i].identity];uses[i]=order[i].uses;}return true;
 }
};
}
