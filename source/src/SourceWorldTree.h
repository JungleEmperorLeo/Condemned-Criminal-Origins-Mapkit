#pragma once
#include "SourceRenderPointTags.h"
#include "WorldModels.h"
#include "WorldCenter.h"
#include "WorldTreeSerialization.h"
#include "FearWorldTreeIntersection.h"
#include "SourcePhysicalBspContours.h"
// Source quadtree construction follows create_world_tree.cpp's polygon-volume
// callback. Default node size and FEAR arithmetic are verified independently;
// this does not construct physical BSP nodes or change the owned source model.
namespace SourceWorldTree {
struct Contour {WorldText::Vec3* points;unsigned int count;};
class Builder {
 PrefabScene::Arena memory;WorldTreeSerialization::Block output;
 WorldTreeSerialization::Node* topology;unsigned int capacity;
 FearWorldTreeIntersection::Workspace* clipScratch;
 Builder(const Builder&);Builder& operator=(const Builder&);
 bool Fail(const char* reason){Clear();error=reason;return false;}
 __declspec(noinline) bool Recurse(const Contour* const* contours,unsigned int contourCount,const WorldText::Vec3& nodeLow,const WorldText::Vec3& nodeHigh,unsigned int depth,unsigned int& index){
  if(nodeCount==capacity)return Fail("World tree node capacity");index=nodeCount++;WorldTreeSerialization::Node& node=topology[index];for(unsigned int c=0;c<4;c++)node.children[c]=-1;if(depth>maxDepth)maxDepth=depth;
  FearWorldTreeIntersection::Box box;FearWorldTreeIntersection::SetBounds(nodeLow,nodeHigh,box);
  if(depth>=16||box.minimumDimension<=nodeSize)return true;
  PrefabScene::Arena scratch;const Contour** inside=(const Contour**)scratch.Allocate((contourCount+1)*sizeof(void*));if(!inside)return Fail("World tree callback allocation failed");unsigned int count=0;
  for(unsigned int p=0;p<contourCount;p++)if(FearWorldTreeIntersection::Intersects(contours[p]->points,contours[p]->count,box,*clipScratch))inside[count++]=contours[p];
  if(!count)return true;
  float centerX=box.center.x,centerZ=box.center.z;
  for(unsigned int c=0;c<4;c++){WorldText::Vec3 childLow=nodeLow,childHigh=nodeHigh;if(c&2)childLow.x=centerX;else childHigh.x=centerX;if(c&1)childLow.z=centerZ;else childHigh.z=centerZ;unsigned int child=0;if(!Recurse(inside,count,childLow,childHigh,depth+1,child))return false;node.children[c]=(int)child;}
  return true;
 }
public:
 unsigned char* data;unsigned int size,nodeCount,maxDepth,polygons;WorldText::Vec3 low,high;float nodeSize;const char* error;
 Builder():topology(0),capacity(0),clipScratch(0),data(0),size(0),nodeCount(0),maxDepth(0),polygons(0),nodeSize(1000),error(0){low.x=low.y=low.z=high.x=high.y=high.z=0;}
 void Clear(){output.Clear();memory.Clear();topology=0;capacity=0;clipScratch=0;data=0;size=nodeCount=maxDepth=polygons=0;error=0;low.x=low.y=low.z=high.x=high.y=high.z=0;}
 bool Build(const PrefabScene::Model& model,const WorldModels::Partition& partition,const WorldText::Vec3& offset,float worldNodeSize=1000.0f,unsigned int maximumNodes=100000){
  Clear();nodeSize=worldNodeSize;
  if(!WorldText::FiniteVector(offset)||!WorldText::Finite(worldNodeSize)||worldNodeSize<1||worldNodeSize>20000||!maximumNodes||maximumNodes>WorldTreeSerialization::MaximumNodes||model.count>100000||partition.error||!partition.groups||!partition.groupCount||partition.groupCount>4096||partition.assignmentCount>model.brushes||(!partition.assignments&&partition.assignmentCount))return Fail("Invalid source world tree settings/partition");
  const PrefabScene::Node** sourceNodes=(const PrefabScene::Node**)memory.Allocate((model.count+1)*sizeof(void*));unsigned char* seen=(unsigned char*)memory.Allocate(model.count+1);if(!sourceNodes||!seen)return Fail("World tree source provenance allocation failed");memset(seen,0,model.count+1);unsigned int ordered=0;
  for(const PrefabScene::Node* node=model.first;node;node=node->next){if(node->index!=ordered||ordered>=model.count)return Fail("Invalid world tree source node order");sourceNodes[ordered++]=node;}if(ordered!=model.count)return Fail("Incomplete world tree source node order");
  for(unsigned int a=0;a<partition.assignmentCount;a++){const WorldModels::Assignment& item=partition.assignments[a];if(!item.brush||item.brush->index>=model.count||sourceNodes[item.brush->index]!=item.brush||item.brush->kind!=1||item.group>=partition.groupCount||seen[item.brush->index])return Fail("Invalid world tree source assignment");seen[item.brush->index]=1;}
  // The tree is built from the BSP world after Nav deleted its projected global
  // carvers (VA 0x100054D0), so they take no part in the root bounds either.
  unsigned int selectedPolygons=0;const bool navigation=SourcePhysicalBspContours::HasNavigation(model);
  for(unsigned int a=0;a<partition.assignmentCount;a++)if(partition.assignments[a].group==0&&partition.assignments[a].brush&&SourcePhysicalBspContours::DiskCandidate(*partition.assignments[a].brush,navigation)){
   const PrefabScene::Node& brush=*partition.assignments[a].brush;if(brush.geometry.polygons&&!brush.geometry.faces)return Fail("Invalid world tree source faces");
   for(unsigned int p=0;p<brush.geometry.polygons;p++){unsigned int fragments=SourceSurfacePreparation::FragmentCount(brush.geometry.faces[p].count);if(!fragments||fragments>100000-polygons)return Fail("World tree source polygon capacity");polygons+=fragments;}
  }
  Contour* contours=(Contour*)memory.Allocate((polygons+1)*sizeof(Contour));const Contour** pointers=(const Contour**)memory.Allocate((polygons+1)*sizeof(void*));topology=(WorldTreeSerialization::Node*)memory.Allocate(maximumNodes*sizeof(WorldTreeSerialization::Node));clipScratch=(FearWorldTreeIntersection::Workspace*)memory.Allocate(sizeof(FearWorldTreeIntersection::Workspace));if(!contours||!pointers||!topology||!clipScratch)return Fail("World tree source allocation failed");capacity=maximumNodes;unsigned int ordinal=0;bool hasBounds=false;
  for(unsigned int a=0;a<partition.assignmentCount;a++)if(partition.assignments[a].group==0&&partition.assignments[a].brush&&SourcePhysicalBspContours::DiskCandidate(*partition.assignments[a].brush,navigation)){
   const WorldText::Geometry& geometry=partition.assignments[a].brush->geometry;if((geometry.polygons&&!geometry.faces)||(geometry.points&&!geometry.vertices))return Fail("Invalid world tree source geometry");
   for(unsigned int p=0;p<geometry.polygons;p++){
    const WorldText::Polygon& face=geometry.faces[p];if(!face.indices||face.count<3||face.count>65535)return Fail("Invalid world tree polygon");
    for(unsigned int f=0;f<SourceSurfacePreparation::FragmentCount(face.count);f++){
    SourceSurfacePreparation::Fragment fragment;if(!SourceSurfacePreparation::FragmentAt(face,f,fragment))return Fail("World tree source fragment failed");Contour& contour=contours[ordinal];contour.count=fragment.count;contour.points=(WorldText::Vec3*)memory.Allocate(fragment.count*12);if(!contour.points)return Fail("World tree source contour allocation failed");
    for(unsigned int i=0;i<fragment.count;i++){if(fragment.indices[i]>=geometry.points)return Fail("World tree polygon index out of range");WorldText::Vec3 point=geometry.vertices[fragment.indices[i]];point.x-=offset.x;point.y-=offset.y;point.z-=offset.z;if(!WorldText::FiniteVector(point))return Fail("Non-finite world tree point");contour.points[i]=point;
     if(!hasBounds){low=high=point;hasBounds=true;}else {if(point.x<low.x)low.x=point.x;if(point.y<low.y)low.y=point.y;if(point.z<low.z)low.z=point.z;if(point.x>high.x)high.x=point.x;if(point.y>high.y)high.y=point.y;if(point.z>high.z)high.z=point.z;}}
    // FEAR CreateWorldTree 0x1549A..0x154BE includes every main polygon in
    // root bounds, but only !(flags&2)&&(flags&0x18) in the subdivision callback.
    if(WorldCenter::Eligible(partition.assignments[a].brush->surfaceFlags))pointers[selectedPolygons++]=&contour;ordinal++;
    }
   }
  }
  if(ordinal!=polygons)return Fail("World tree source enumeration mismatch");unsigned int root=0;if(!Recurse(pointers,selectedPolygons,low,high,0,root))return false;
  if(!output.Build(low,high,topology,nodeCount,root))return Fail(output.error);data=output.data;size=output.size;return true;
 }
};
}
