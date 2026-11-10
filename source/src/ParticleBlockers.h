#pragma once
#include "WorldModels.h"
#include "SourceSurfacePreparation.h"
#include "SourceRenderPlane.h"
#include "SourceBlockerPlanePool.h"
// Particle blockers have their own ordered polygon stream. RVA 0x253E0
// extracts surface flag 0x80 from every preworld; RVA 0x51620 writes this
// count, the planes/points, and one unconditional zero terminator.
namespace ParticleBlockers {
enum Code {Ok,InvalidInput,InvalidPolygon,Capacity,Allocation};
class Block {
 PrefabScene::Arena memory;
 Block(const Block&);Block& operator=(const Block&);
 bool Fail(Code reason,const char* message,unsigned int node=0xffffffffu,unsigned int polygon=0xffffffffu){
  memory.Clear();data=0;size=count=vertices=brushes=0;code=reason;error=message;sourceNode=node;sourcePolygon=polygon;return false;
 }
 static bool Selected(const WorldModels::Assignment& assignment){return assignment.brush&&assignment.brush->kind==1&&(assignment.brush->surfaceFlags&0x80)!=0;}
 static void Put(unsigned char*& output,const void* input,unsigned int bytes){const unsigned char* p=(const unsigned char*)input;while(bytes--)*output++=*p++;}
public:
 unsigned char* data;unsigned int size,count,vertices,brushes,sourceNode,sourcePolygon;Code code;const char* error;
 Block():data(0),size(0),count(0),vertices(0),brushes(0),sourceNode(0xffffffffu),sourcePolygon(0xffffffffu),code(Ok),error(0){}
 void Clear(){Fail(Ok,0);}
 bool Build(const PrefabScene::Model& model,const WorldModels::Partition& partition,const WorldText::Vec3& offset){
  Clear();if(!WorldText::FiniteVector(offset)||!partition.groups||!partition.groupCount||partition.assignmentCount>model.brushes||(!partition.assignments&&partition.assignmentCount))return Fail(InvalidInput,"Invalid particle blocker scene/partition/offset");
  SourceBlockerPlanePool::Prepared planes;if(!planes.Build(model,partition,offset))return Fail(InvalidInput,planes.error);
  unsigned int byteCount=8;
  // Follow preworld creation order, then each preworld's source polygon order.
  for(unsigned int g=0;g<partition.groupCount;g++)for(unsigned int a=0;a<partition.assignmentCount;a++){
   const WorldModels::Assignment& assignment=partition.assignments[a];if(assignment.group!=g||!planes.EmitGroup(g)||!Selected(assignment))continue;
   const PrefabScene::Node& node=*assignment.brush;const WorldText::Geometry& geometry=node.geometry;
   if(!node.knownType||!geometry.faces||!geometry.vertices)return Fail(InvalidPolygon,"Invalid particle blocker brush",node.index);
   brushes++;
   for(unsigned int p=0;p<geometry.polygons;p++){
    const WorldText::Polygon& polygon=geometry.faces[p];WorldText::Vec3 normal;
    if(!planes.Normal(a,p,normal))return Fail(InvalidPolygon,"Invalid particle blocker plane",node.index,p);
    unsigned int fragments=SourceSurfacePreparation::FragmentCount(polygon.count);
    if(!fragments||fragments>100000-count)return Fail(Capacity,"Particle blocker polygon count exceeded",node.index,p);
    for(unsigned int f=0;f<fragments;f++){
     SourceSurfacePreparation::Fragment fragment;if(!SourceSurfacePreparation::FragmentAt(polygon,f,fragment))return Fail(InvalidPolygon,"Invalid particle blocker fragment",node.index,p);
     if(fragment.count>1000000-vertices||20+fragment.count*12>32*1024*1024-byteCount)return Fail(Capacity,"Particle blocker byte/vertex budget exceeded",node.index,p);
     count++;vertices+=fragment.count;byteCount+=20+fragment.count*12;
    }
   }
  }
  data=(unsigned char*)memory.Allocate(byteCount);if(!data)return Fail(Allocation,"Could not allocate particle blocker block");size=byteCount;unsigned char* out=data;Put(out,&count,4);
  for(unsigned int g=0;g<partition.groupCount;g++)for(unsigned int a=0;a<partition.assignmentCount;a++){
   const WorldModels::Assignment& assignment=partition.assignments[a];if(assignment.group!=g||!planes.EmitGroup(g)||!Selected(assignment))continue;
   const PrefabScene::Node& node=*assignment.brush;const WorldText::Geometry& geometry=node.geometry;
   for(unsigned int p=0;p<geometry.polygons;p++){
    const WorldText::Polygon& polygon=geometry.faces[p];WorldText::Vec3 normal;
    if(!planes.Normal(a,p,normal))return Fail(InvalidPolygon,"Invalid particle blocker plane during emission",node.index,p);
    float distance;if(!planes.PlaneDistance(a,p,normal,distance))return Fail(InvalidPolygon,"Invalid particle blocker distance",node.index,p);
    unsigned int fragments=SourceSurfacePreparation::FragmentCount(polygon.count);
    for(unsigned int f=0;f<fragments;f++){
     SourceSurfacePreparation::Fragment fragment;SourceSurfacePreparation::FragmentAt(polygon,f,fragment);Put(out,&normal,12);Put(out,&distance,4);Put(out,&fragment.count,4);
     for(unsigned int i=0;i<fragment.count;i++){WorldText::Vec3 point;if(!planes.Point(a,geometry.vertices[fragment.indices[i]],point))return Fail(InvalidPolygon,"Non-finite prepared particle blocker vertex",node.index,p);Put(out,&point,12);}
    }
   }
  }
  unsigned int terminator=0;Put(out,&terminator,4);if(out!=data+size)return Fail(InvalidInput,"Particle blocker size accounting failed");return true;
 }
};
}
