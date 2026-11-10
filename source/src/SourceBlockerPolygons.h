#pragma once
#include "WorldModels.h"
#include "SourceSurfacePreparation.h"
#include "SourceRenderPlane.h"
#include "SourceBlockerPlanePool.h"
// Ordinary physics blockers are a separate ordered polygon stream, not BSP
// polygons or render triangles. Own only serialized bytes; model/partition are
// read-only inputs. No material/shader or original compiled world is needed.
namespace SourceBlockerPolygons {
enum Code {Ok,InvalidInput,InvalidPolygon,Capacity,Allocation};
class Block {
 PrefabScene::Arena memory;
 Block(const Block&);Block& operator=(const Block&);
 bool Fail(Code reason,const char* message,unsigned int node=0xffffffffu,unsigned int polygon=0xffffffffu){
  memory.Clear();data=0;size=count=vertices=brushes=0;code=reason;error=message;sourceNode=node;sourcePolygon=polygon;return false;
 }
 bool Selected(const WorldModels::Assignment& assignment)const{return assignment.brush&&assignment.brush->kind==1&&(assignment.brush->surfaceFlags&0x100)!=0;}
 static void Put(unsigned char*& output,const void* input,unsigned int bytes){const unsigned char* p=(const unsigned char*)input;while(bytes--)*output++=*p++;}
public:
 unsigned char* data;unsigned int size,count,vertices,brushes,sourceNode,sourcePolygon;Code code;const char* error;
 Block():data(0),size(0),count(0),vertices(0),brushes(0),sourceNode(0xffffffffu),sourcePolygon(0xffffffffu),code(Ok),error(0){}
 void Clear(){Fail(Ok,0);}
 bool Build(const PrefabScene::Model& model,const WorldModels::Partition& partition,const WorldText::Vec3& offset){
  Clear();if(!WorldText::FiniteVector(offset)||!partition.groups||!partition.groupCount||partition.assignmentCount>model.brushes||(!partition.assignments&&partition.assignmentCount))return Fail(InvalidInput,"Invalid blocker scene/partition/offset");
  unsigned int byteCount=8;
  // Validate the selected geometry before the shared normal-table pass.
  for(unsigned int a=0;a<partition.assignmentCount;a++){
   const WorldModels::Assignment& assignment=partition.assignments[a];if(!Selected(assignment))continue;
   const PrefabScene::Node& node=*assignment.brush;
   const WorldText::Geometry& geometry=node.geometry;if(!node.knownType||!geometry.faces||!geometry.vertices||!geometry.points)return Fail(InvalidPolygon,"Invalid blocker brush",node.index);
   if(geometry.polygons>100000||geometry.points>1000000)return Fail(Capacity,"Blocker brush geometry limit exceeded",node.index);
   for(unsigned int p=0;p<geometry.polygons;p++){
    if(geometry.faces[p].count>1000000)return Fail(Capacity,"Blocker polygon vertex limit exceeded",node.index,p);
    WorldText::Vec3 normal;if(!SourceRenderNormals::Face(geometry,geometry.faces[p],normal,&offset))return Fail(InvalidPolygon,"Invalid blocker source plane",node.index,p);
   }
  }
  SourceBlockerPlanePool::Prepared planes;if(!planes.Build(model,partition,offset))return Fail(InvalidInput,planes.error);
  // Extraction follows preworld creation order. Shared prefab aliases reuse
  // one representative preworld and therefore contribute no duplicate stream.
  for(unsigned int g=0;g<partition.groupCount;g++)if(planes.EmitGroup(g))for(unsigned int a=0;a<partition.assignmentCount;a++){
   const WorldModels::Assignment& assignment=partition.assignments[a];if(assignment.group!=g||!Selected(assignment))continue;
   const PrefabScene::Node& node=*assignment.brush;const WorldText::Geometry& geometry=node.geometry;
   brushes++;
   for(unsigned int p=0;p<geometry.polygons;p++){
    const WorldText::Polygon& polygon=geometry.faces[p];WorldText::Vec3 normal;
    if(polygon.count>1000000)return Fail(Capacity,"Blocker polygon vertex limit exceeded",node.index,p);
    if(!planes.Normal(a,p,normal))return Fail(InvalidPolygon,"Invalid blocker plane",node.index,p);
    unsigned int fragments=SourceSurfacePreparation::FragmentCount(polygon.count);
    if(!fragments||fragments>100000-count)return Fail(Capacity,"Blocker polygon count exceeded",node.index,p);
    for(unsigned int f=0;f<fragments;f++){
     SourceSurfacePreparation::Fragment fragment;if(!SourceSurfacePreparation::FragmentAt(polygon,f,fragment))return Fail(InvalidPolygon,"Invalid blocker fragment",node.index,p);
     if(fragment.count>1000000-vertices||20+fragment.count*12>32*1024*1024-byteCount)return Fail(Capacity,"Blocker byte/vertex budget exceeded",node.index,p);
     count++;vertices+=fragment.count;byteCount+=20+fragment.count*12;
    }
   }
  }
  data=(unsigned char*)memory.Allocate(byteCount);if(!data)return Fail(Allocation,"Could not allocate blocker block");size=byteCount;unsigned char* out=data;Put(out,&count,4);Put(out,&vertices,4);
  for(unsigned int g=0;g<partition.groupCount;g++)if(planes.EmitGroup(g))for(unsigned int a=0;a<partition.assignmentCount;a++){
   const WorldModels::Assignment& assignment=partition.assignments[a];if(assignment.group!=g||!Selected(assignment))continue;
   const PrefabScene::Node& node=*assignment.brush;const WorldText::Geometry& geometry=node.geometry;
   for(unsigned int p=0;p<geometry.polygons;p++){
    const WorldText::Polygon& polygon=geometry.faces[p];WorldText::Vec3 normal;
    float distance;if(!planes.Normal(a,p,normal)||!planes.PlaneDistance(a,p,normal,distance))return Fail(InvalidPolygon,"Invalid blocker plane during emission",node.index,p);
    unsigned int fragments=SourceSurfacePreparation::FragmentCount(polygon.count);
    for(unsigned int f=0;f<fragments;f++){
     SourceSurfacePreparation::Fragment fragment;SourceSurfacePreparation::FragmentAt(polygon,f,fragment);Put(out,&normal,12);Put(out,&distance,4);Put(out,&fragment.count,4);
     for(unsigned int i=0;i<fragment.count;i++){WorldText::Vec3 point;if(!planes.Point(a,geometry.vertices[fragment.indices[i]],point))return Fail(InvalidPolygon,"Non-finite prepared blocker vertex",node.index,p);Put(out,&point,12);}
    }
   }
  }
  if(out!=data+size)return Fail(InvalidInput,"Blocker size accounting failed");return true;
 }
};
}
