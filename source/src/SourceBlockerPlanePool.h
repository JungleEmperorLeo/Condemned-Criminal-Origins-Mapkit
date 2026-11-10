#pragma once
#include "SourceRenderPlanePool.h"
#include "SourceRenderPointTags.h"
#include "WorldModelSharing.h"
#include "SourceOwnerRotation.h"
#include "NativeRawNavMeshGeneral.h"
namespace SourceBlockerPlanePool {
// Brush that the original deletes after Nav (global carver with a kept record).
inline bool Deleted(const PrefabScene::Node& brush,bool navigation){return navigation&&brush.kind==1&&brush.geometry.polygons!=1&&FearBrushFlags::Equal(brush.type,"AINavMeshCarver")&&NativeRawNavMeshGeneral::Builder::ProjectsCarver(brush);}
inline bool Navigation(const PrefabScene::Model& model){for(const PrefabScene::Node* n=model.first;n;n=n->next)if(n->kind==2&&FearBrushFlags::Equal(n->className,"AINavMesh"))return true;return false;}
inline bool Distance(const WorldText::Geometry& geometry,const WorldText::Polygon& polygon,const WorldText::Vec3& offset,const WorldText::Vec3& normal,float& output){
 if(!geometry.vertices||!polygon.indices||polygon.count<3||!WorldText::FiniteVector(offset)||!WorldText::FiniteVector(normal))return false;
 double center[3]={0,0,0};
 for(unsigned int i=0;i<polygon.count;i++){
  if(polygon.indices[i]>=geometry.points)return false;WorldText::Vec3 point=geometry.vertices[polygon.indices[i]];point.x-=offset.x;point.y-=offset.y;point.z-=offset.z;if(!WorldText::FiniteVector(point))return false;
  center[0]+=point.x;center[1]+=point.y;center[2]+=point.z;
 }
 double inverse=1.0/polygon.count;output=(float)(((center[0]*inverse)*normal.x+(center[1]*inverse)*normal.y)+(center[2]*inverse)*normal.z);return WorldText::Finite(output);
}
// Surfaces share a quantized normal table before the blocker extraction pass.
// A preceding ordinary brush can supply the retained normal for a blocker.
// Plane lookup is tied to assignment/polygon provenance, never equal points.
class Prepared {
 PrefabScene::Arena memory;
 const WorldModels::Partition* partition;
 WorldText::Vec3** normals;
 unsigned int* representatives;
 WorldText::Vec3 worldOffset;
 const PrefabScene::Node** nodes;
 unsigned char* localGroups;
 Prepared(const Prepared&);Prepared& operator=(const Prepared&);
 bool Fail(const char* reason){memory.Clear();partition=0;normals=0;nodes=0;representatives=0;localGroups=0;normalTable=0;normalCount=0;error=reason;return false;}
public:
 const char* error;WorldText::Vec3* normalTable;unsigned int normalCount;
 Prepared():partition(0),normals(0),representatives(0),nodes(0),localGroups(0),error(0),normalTable(0),normalCount(0){}
 bool Build(const PrefabScene::Model& model,const WorldModels::Partition& assignments,const WorldText::Vec3& offset){
  Fail(0);normalTable=0;normalCount=0;
  if(model.count>100000||assignments.error||!assignments.groups||!assignments.groupCount||assignments.groupCount>4096||assignments.assignmentCount>model.brushes||(!assignments.assignments&&assignments.assignmentCount)||!WorldText::FiniteVector(offset))return Fail("Invalid blocker normal-table input");
  worldOffset=offset;
  nodes=(const PrefabScene::Node**)memory.Allocate((model.count+1)*sizeof(*nodes));normals=(WorldText::Vec3**)memory.Allocate((assignments.assignmentCount+1)*sizeof(*normals));unsigned char* seen=(unsigned char*)memory.Allocate(model.count+1);
  if(!nodes||!normals||!seen)return Fail("Blocker normal provenance allocation failed");
  unsigned int ordered=0,polygons=0;
  for(const PrefabScene::Node* n=model.first;n;n=n->next){if(n->index!=ordered||ordered>=model.count)return Fail("Invalid blocker source node order");nodes[ordered++]=n;}if(ordered!=model.count)return Fail("Incomplete blocker source node order");
  for(unsigned int a=0;a<assignments.assignmentCount;a++){
   const WorldModels::Assignment& item=assignments.assignments[a];
   if(!item.brush||item.brush->index>=model.count||nodes[item.brush->index]!=item.brush||seen[item.brush->index]||item.brush->kind!=1||!item.brush->knownType||item.group>=assignments.groupCount)return Fail("Invalid blocker source assignment");seen[item.brush->index]=1;
   if(!SourceRenderPointTags::MainWorldCandidate(*item.brush))continue;
   const WorldText::Geometry& geometry=item.brush->geometry;
   if(!geometry.polygons){normals[a]=0;continue;} // emptied by the per-file weld/cleanup (0x10059550): no plane
   if(!geometry.faces||!geometry.vertices||!geometry.polygons||geometry.polygons>1000000-polygons){static char why[200];lstrcpyA(why,"Invalid blocker source normal geometry: ");lstrcpynA(why+lstrlenA(why),item.brush->name?item.brush->name:"",120);{char* at=why+lstrlenA(why);const char* label=" polygons ";while(*label)*at++=*label++;unsigned int v=geometry.polygons;char digits[12];int n=0;do{digits[n++]=(char)(48+v%10);v/=10;}while(v);while(n)*at++=digits[--n];*at=0;}return Fail(why);}polygons+=geometry.polygons;
   normals[a]=(WorldText::Vec3*)memory.Allocate(geometry.polygons*12);if(!normals[a])return Fail("Blocker normal vector allocation failed");
  }
  WorldModelSharing::Identity sharing;if(!sharing.Build(model,assignments))return Fail("Blocker owner sharing failed");representatives=(unsigned int*)memory.Allocate(assignments.groupCount*4);if(!representatives)return Fail("Blocker owner sharing allocation failed");memcpy(representatives,sharing.representative,assignments.groupCount*4);
  localGroups=(unsigned char*)memory.Allocate(assignments.groupCount+1);if(!localGroups)return Fail("Blocker owner localization marks allocation failed");
  for(unsigned int g=1;g<assignments.groupCount;g++)if(assignments.groups[g].owner->importedFromPrefab)localGroups[g]=1;
  for(unsigned int a=0;a<assignments.assignmentCount;a++){const WorldModels::Assignment& item=assignments.assignments[a];if(item.group&&SourceRenderPointTags::MainWorldCandidate(*item.brush)&&!(item.brush->surfaceFlags&0x180))localGroups[item.group]=1;}
  if(!polygons){partition=&assignments;return true;}
// Seeded with the 18 axis/diagonal normals like the physical table: a first
  // near-axis face (Enterprise_bsp_export -0.99999994) takes the exact seed.
  SourceRenderPlanePool::Pool pool;if(!pool.Initialize(polygons+18)||!pool.Seed())return Fail("Blocker shared normal table allocation failed");
  // Representative groups seed the pool with Nav-deleted carver surfaces last, as
  // the physical table (SourcePhysicalBspNormals); the first retained vector wins.
  const bool navigation=Navigation(model);
  for(unsigned int group=0;group<assignments.groupCount;group++){if(representatives[group]!=group)continue;
   const PrefabScene::Node* owner=group?assignments.groups[group].owner:0;bool local=owner&&localGroups[group];float inverse[4];if(local&&!SourceOwnerRotation::Inverse(*owner,inverse))return Fail("Blocker owner inverse failed");
   for(unsigned int pass=0;pass<2;pass++)for(unsigned int a=0;a<assignments.assignmentCount;a++){const WorldModels::Assignment& item=assignments.assignments[a];if(item.group!=group||!normals[a])continue;if((pass==0)==Deleted(*item.brush,navigation))continue;const WorldText::Geometry& geometry=item.brush->geometry;
    for(unsigned int p=0;p<geometry.polygons;p++){WorldText::Vec3 normal,prepared,unused;if(!SourceRenderNormals::Face(geometry,geometry.faces[p],normal,&offset))return Fail("Blocker source normal failed");if(local)normal=SourceWorldModelTransform::Rotate(inverse,normal);if(!SourceRenderPlane::Build(normal,prepared)||!pool.Get(prepared,unused))return Fail("Blocker prepass failed");}}}
  for(unsigned int group=0;group<assignments.groupCount;group++){
   const PrefabScene::Node* owner=group?assignments.groups[group].owner:0;
   bool local=owner&&localGroups[group];float inverse[4];if(local&&!SourceOwnerRotation::Inverse(*owner,inverse))return Fail("Blocker owner inverse failed");
   for(unsigned int a=0;a<assignments.assignmentCount;a++){
    const WorldModels::Assignment& item=assignments.assignments[a];if(item.group!=group||!normals[a])continue;const WorldText::Geometry& geometry=item.brush->geometry;
    for(unsigned int p=0;p<geometry.polygons;p++){
     WorldText::Vec3 normal,prepared;if(!SourceRenderNormals::Face(geometry,geometry.faces[p],normal,&offset))return Fail("Blocker source normal failed");
     if(local)normal=SourceWorldModelTransform::Rotate(inverse,normal);
     bool insert=representatives[group]==group;
     if(!SourceRenderPlane::Build(normal,prepared)||(insert?!pool.Get(prepared,normals[a][p]):!pool.LookupOrOriginal(prepared,normals[a][p])))return Fail("Blocker shared normal insertion failed");
    }
   }
  }
  normalCount=pool.Count();normalTable=(WorldText::Vec3*)memory.Allocate((normalCount+1)*12);if(!normalTable||!pool.Ordered(normalTable,normalCount))return Fail("Blocker ordered normal table allocation failed");
  partition=&assignments;return true;
 }
 bool Normal(unsigned int assignment,unsigned int polygon,WorldText::Vec3& output)const{
  if(!partition||assignment>=partition->assignmentCount||!normals[assignment]||polygon>=partition->assignments[assignment].brush->geometry.polygons)return false;
  output=normals[assignment][polygon];return true;
 }
 bool EmitGroup(unsigned int group)const{return partition&&group<partition->groupCount&&representatives&&representatives[group]==group;}
 bool Point(unsigned int assignment,const WorldText::Vec3& source,WorldText::Vec3& output)const{
  if(!partition||assignment>=partition->assignmentCount||!WorldText::FiniteVector(source))return false;
  unsigned int group=partition->assignments[assignment].group;const PrefabScene::Node* owner=group?partition->groups[group].owner:0;
  // Loaded prototypes and authored owners with remaining ordinary contours
  // use owner coordinates. A blocker-only authored group remains in world space.
  if(owner&&localGroups[group])return SourceOwnerRotation::Local(*owner,source,worldOffset,output);
  output=source;output.x-=worldOffset.x;output.y-=worldOffset.y;output.z-=worldOffset.z;return WorldText::FiniteVector(output);
 }
 bool PlaneDistance(unsigned int assignment,unsigned int polygon,const WorldText::Vec3& normal,float& output)const{
  if(!partition||assignment>=partition->assignmentCount||!WorldText::FiniteVector(normal))return false;const WorldText::Geometry& geometry=partition->assignments[assignment].brush->geometry;if(polygon>=geometry.polygons)return false;
  const WorldText::Polygon& face=geometry.faces[polygon];if(!face.indices||face.count<3)return false;double center[3]={0,0,0};
  for(unsigned int i=0;i<face.count;i++){if(face.indices[i]>=geometry.points)return false;WorldText::Vec3 point;if(!Point(assignment,geometry.vertices[face.indices[i]],point))return false;center[0]+=point.x;center[1]+=point.y;center[2]+=point.z;}
  double inverse=1.0/face.count;output=(float)(((center[0]*inverse)*normal.x+(center[1]*inverse)*normal.y)+(center[2]*inverse)*normal.z);return WorldText::Finite(output);
 }
};
}
