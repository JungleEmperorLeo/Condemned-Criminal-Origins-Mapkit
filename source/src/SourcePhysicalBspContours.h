#pragma once
#include "SourceBlockerPlanePool.h"
#include "FearBspTree.h"
#include "NativeRawNavMeshGeneral.h"
// Source bridge for one physical BSP group. No compiled geometry is an input.
// It retains source contour identity through transient BSP cuts, as required
// by original postprocessing. Disk point/normal/material IDs are separate.
namespace SourcePhysicalBspContours {
inline bool HasNavigation(const PrefabScene::Model& model){for(const PrefabScene::Node* node=model.first;node;node=node->next)if(node->kind==2&&FearBrushFlags::Equal(node->className,"AINavMesh"))return true;return false;}
// A global AINavMeshCarver brush is deleted after Nav only when the original
// keeps its carver record: polygon count other than one and a nonempty
// projection (VA 0x100050B0, 0x10003C00, 0x10004BA0).
inline bool DiskCandidate(const PrefabScene::Node& brush,bool navigation){return SourceRenderPointTags::MainWorldCandidate(brush)&&!(navigation&&brush.geometry.polygons!=1&&FearBrushFlags::Equal(brush.type,"AINavMeshCarver")&&NativeRawNavMeshGeneral::Builder::ProjectsCarver(brush));}
inline bool RetainedGroup(const PrefabScene::Model& model,const WorldModels::Partition& partition,unsigned int group){const bool navigation=HasNavigation(model);for(unsigned int i=0;i<partition.assignmentCount;i++){const WorldModels::Assignment& item=partition.assignments[i];if(item.group==group&&DiskCandidate(*item.brush,navigation)&&!(item.brush->surfaceFlags&0x180))return true;}return false;}
struct Contour {
 FearBspSplitPolicy::Polygon polygon;
 unsigned int sourceNode,sourcePolygon,sourceFragment;
};
class Main {
 PrefabScene::Arena memory;FearBspTree::Builder tree;
 Main(const Main&);Main& operator=(const Main&);
 bool Fail(const char* reason){Clear();error=reason;return false;}
public:
 Contour* contours;unsigned int count,references;const char* error;
 Main():contours(0),count(0),references(0),error(0){}
 void Clear(){tree.Clear();memory.Clear();contours=0;count=references=0;error=0;}
 const FearBspTree::Builder& Tree()const{return tree;}
 bool Build(const PrefabScene::Model& model,const WorldModels::Partition& partition,const WorldText::Vec3& offset,bool buildTree=true){
   SourceBlockerPlanePool::Prepared planes;
  if(!planes.Build(model,partition,offset))return Fail(planes.error);
   return BuildGroup(model,partition,0,planes,1500,1000000,10000000,buildTree);
  }
  bool BuildGroup(const PrefabScene::Model& model,const WorldModels::Partition& partition,unsigned int group,const SourceBlockerPlanePool::Prepared& planes,unsigned int depthLimit=1500,unsigned int fragmentLimit=1000000,unsigned int pointLimit=10000000,bool buildTree=true /* CONDEMNED FORK: raised from 256/100000/1000000 for retail-size maps */){
   Clear();
   if(partition.error||group>=partition.groupCount||planes.error||!planes.EmitGroup(group)||model.count>100000||!depthLimit||depthLimit>4096||!fragmentLimit||fragmentLimit>1000000||pointLimit>10000000)return Fail("Invalid source physical BSP group or limits");
   // Navigation preprocessing returns before collecting carvers when the source
   // has no AINavMesh object. Surface/normal preparation precedes this removal.
   const bool navigation=HasNavigation(model);
   unsigned char* retained=(unsigned char*)memory.Allocate(partition.groupCount+1);if(!retained)return Fail("Source physical BSP group marks allocation failed");
   for(unsigned int a=0;a<partition.assignmentCount;a++){const WorldModels::Assignment& item=partition.assignments[a];if(DiskCandidate(*item.brush,navigation)&&!(item.brush->surfaceFlags&0x180))retained[item.group]=1;}
  for(unsigned int a=0;a<partition.assignmentCount;a++){
   const WorldModels::Assignment& assignment=partition.assignments[a];
    if((assignment.group!=group&&!(group==0&&!retained[assignment.group]&&planes.EmitGroup(assignment.group)))||!DiskCandidate(*assignment.brush,navigation))continue;
   const WorldText::Geometry& geometry=assignment.brush->geometry;
   for(unsigned int p=0;p<geometry.polygons;p++){
    unsigned int fragments=SourceSurfacePreparation::FragmentCount(geometry.faces[p].count);
    if(!fragments||fragments>100000-count)return Fail("Source MAIN BSP contour limit exceeded");count+=fragments;
   }
  }
  if(!count)return Fail("Source MAIN BSP has no contours");
  contours=(Contour*)memory.Allocate(count*sizeof(Contour));FearBspTree::Input* input=(FearBspTree::Input*)memory.Allocate(count*sizeof(FearBspTree::Input));
  if(!contours||!input)return Fail("Source MAIN BSP contour allocation failed");unsigned int at=0;
  for(unsigned int a=0;a<partition.assignmentCount;a++){
   const WorldModels::Assignment& assignment=partition.assignments[a];
    if((assignment.group!=group&&!(group==0&&!retained[assignment.group]&&planes.EmitGroup(assignment.group)))||!DiskCandidate(*assignment.brush,navigation))continue;
   const PrefabScene::Node& brush=*assignment.brush;const WorldText::Geometry& geometry=brush.geometry;
   for(unsigned int p=0;p<geometry.polygons;p++){
    WorldText::Vec3 normal;float distance;
    if(!planes.Normal(a,p,normal)||!planes.PlaneDistance(a,p,normal,distance))return Fail("Source MAIN BSP plane preparation failed");
    for(unsigned int f=0;f<SourceSurfacePreparation::FragmentCount(geometry.faces[p].count);f++){
     SourceSurfacePreparation::Fragment fragment;
     if(!SourceSurfacePreparation::FragmentAt(geometry.faces[p],f,fragment)||fragment.count>1000000-references)return Fail("Source MAIN BSP fragment/reference limit");
     WorldText::Vec3* points=(WorldText::Vec3*)memory.Allocate(fragment.count*12);
     if(!points)return Fail("Source MAIN BSP points allocation failed");
     for(unsigned int i=0;i<fragment.count;i++)if(!planes.Point(a,geometry.vertices[fragment.indices[i]],points[i]))return Fail("Source MAIN BSP point preparation failed");
     Contour& contour=contours[at];contour.polygon.normal=normal;contour.polygon.distance=distance;contour.polygon.points=points;contour.polygon.count=fragment.count;contour.polygon.flags=brush.surfaceFlags;
     contour.sourceNode=brush.index;contour.sourcePolygon=p;contour.sourceFragment=f;FearBspTree::Bounds(contour.polygon);
     input[at].polygon=contour.polygon;input[at].source=at;at++;references+=fragment.count;
    }
   }
  }
  if(at!=count)return Fail("Source MAIN BSP enumeration mismatch");
   unsigned int eligible=0;for(unsigned int i=0;i<count;i++)if(FearBspSplitPolicy::Type(input[i].polygon.flags)!=2)eligible++;
   // RenderOnly owners still carry cached bounds/reservation in a 48-byte block.
   // Their physical root is the initial solid-leaf sentinel, with no nodes.
   if(buildTree&&eligible&&!tree.Build(input,count,depthLimit,fragmentLimit,pointLimit))return Fail(tree.error);return true;
 }
};
}
