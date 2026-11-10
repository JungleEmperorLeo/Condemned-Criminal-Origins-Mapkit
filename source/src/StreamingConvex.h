#pragma once
#include "ExtrusionVolume.h"
#include "WorldCenter.h"
#include "StreamingSplit.h"
#include "StreamingStructure.h"
namespace StreamingConvex {
struct Face {ExtrusionVolume::Plane plane;WorldText::Vec3* points;unsigned int count;};
// moved: a brush of an invalid link appended to its region (0x10037740).
struct Volume {unsigned int owner;int linked;Face* faces;unsigned int count;bool moved;};
class Block {
 bool Fail(const char* message){data=0;size=0;error=message;return false;}
 const char* Property(const PrefabScene::Node* n,const char* name){for(PrefabScene::Property* p=n->properties;p;p=p->next)if(FearBrushFlags::Equal(p->name,name))return p->data;return 0;}
public:
 PrefabScene::Arena memory;StreamingSplit::Bounds* bounds;Volume* volumes;unsigned char* data;unsigned int size,volumeCount,polygonCount,vertexCount,regionCount;WorldText::Vec3 offset;const char* error;
 Block():bounds(0),volumes(0),data(0),size(0),volumeCount(0),polygonCount(0),vertexCount(0),regionCount(0),error(0){offset.x=offset.y=offset.z=0;}
 // StreamingStructure inside test (0x10012A30): a centered point is inside a
 // brush when no internal face plane has it behind; context is the world offset.
 static bool InsideBrush(void* context,const PrefabScene::Node& brush,const WorldText::Vec3& centered){
  const WorldText::Vec3& o=*(const WorldText::Vec3*)context;const WorldText::Geometry& g=brush.geometry;if(!g.polygons)return false;
  for(unsigned int i=0;i<g.polygons;i++){const WorldText::Polygon& p=g.faces[i];if(p.count<3||p.count>128)return false;WorldText::Vec3 points[128];
   for(unsigned int j=0;j<p.count;j++){if(p.indices[j]>=g.points)return false;WorldText::Vec3 q=g.vertices[p.indices[j]];q.x-=o.x;q.y-=o.y;q.z-=o.z;points[j]=q;}
   ExtrusionVolume::Plane plane;if(!ExtrusionVolume::ComputeInternalPlane(points,p.count,plane)||ExtrusionVolume::Distance(plane,centered)<0)return false;}
  return true;
 }
 bool Build(const PrefabScene::Model& model,const WorldText::Vec3* suppliedOffset=0){
  memory.Clear();bounds=0;volumes=0;data=0;size=volumeCount=polygonCount=vertexCount=regionCount=0;error=0;offset.x=offset.y=offset.z=0;if(model.count>100000)return Fail("Invalid streaming source");
  if(suppliedOffset){if(!WorldText::FiniteVector(*suppliedOffset))return Fail("Invalid streaming center offset");offset=*suppliedOffset;}else if(!WorldCenter::Compute(model,offset))return Fail("Invalid streaming source");
  volumes=(Volume*)memory.Allocate((model.brushes+1)*sizeof(Volume));bounds=(StreamingSplit::Bounds*)memory.Allocate((model.brushes+1)*sizeof(StreamingSplit::Bounds));
  StreamingStructure::Volume* order=(StreamingStructure::Volume*)memory.Allocate((model.brushes+1)*sizeof(StreamingStructure::Volume));if(!bounds||!volumes||!order)return Fail("Allocation failed");
  // Region table and volume membership/order of the original stage (StreamingStructure.h).
  StreamingStructure::Builder structure;if(!structure.Build(model,offset))return Fail(structure.error);if(structure.fatal)return Fail("Streaming region names are reserved or duplicated");
  regionCount=structure.TableCount();unsigned int ordered=structure.Volumes(order,model.brushes+1);if(ordered==StreamingStructure::None)return Fail("Streaming volume count");
  for(unsigned int o=0;o<ordered;o++){const PrefabScene::Node* n=order[o].brush;
   StreamingSplit::Bounds& box=bounds[volumeCount];for(unsigned int a=0;a<3;a++){box.low[a]=3.402823466e38f;box.high[a]=-3.402823466e38f;}Volume& v=volumes[volumeCount++];v.owner=order[o].region;v.linked=order[o].linked;v.moved=order[o].moved;
   const WorldText::Geometry& g=n->geometry;if(g.polygons<4||g.polygons>4096)return Fail("Invalid convex polygon count");v.count=g.polygons;v.faces=(Face*)memory.Allocate(v.count*sizeof(Face));if(!v.faces)return Fail("Allocation failed");
   if(size>16*1024*1024-12-v.count*20)return Fail("Convex section too large");size+=12+v.count*20;polygonCount+=v.count;
   for(unsigned int i=0;i<v.count;i++){
    const WorldText::Polygon& p=g.faces[i];Face& f=v.faces[i];if(p.count<3||p.count>128)return Fail("Unsupported convex contour");f.count=p.count;vertexCount+=p.count;if(size>16*1024*1024-p.count*12)return Fail("Convex section too large");size+=p.count*12;f.points=(WorldText::Vec3*)memory.Allocate(p.count*12);if(!f.points)return Fail("Allocation failed");
    for(unsigned int j=0;j<p.count;j++){if(p.indices[j]>=g.points)return Fail("Invalid convex vertex index");WorldText::Vec3 q=g.vertices[p.indices[j]];q.x-=offset.x;q.y-=offset.y;q.z-=offset.z;f.points[j]=q;const float* xyz=&q.x;for(unsigned int a=0;a<3;a++){if(xyz[a]<box.low[a])box.low[a]=xyz[a];if(xyz[a]>box.high[a])box.high[a]=xyz[a];}}
    if(!ExtrusionVolume::ComputeInternalPlane(f.points,f.count,f.plane))return Fail("Invalid convex plane");for(unsigned int j=0;j<f.count;j++){float d=ExtrusionVolume::Distance(f.plane,f.points[j]);if(d<-.01f||d>.01f)return Fail("Nonplanar region contour");}
   }
   // Verified profile: already-clean closed convex brushes, inward-facing planes.
   for(unsigned int i=0;i<v.count;i++)for(unsigned int j=0;j<v.count;j++)for(unsigned int k=0;k<v.faces[j].count;k++)if(ExtrusionVolume::Distance(v.faces[i].plane,v.faces[j].points[k])<-.01f)return Fail("Nonconvex or inconsistent region brush");
  }
  data=(unsigned char*)memory.Allocate(size);if(!data)return Fail("Allocation failed");unsigned int* out=(unsigned int*)data;
  for(unsigned int i=0;i<volumeCount;i++){const Volume& v=volumes[i];*out++=v.owner;*out++=(unsigned int)v.linked;*out++=v.count;for(unsigned int j=0;j<v.count;j++){const unsigned int* plane=(const unsigned int*)&v.faces[j].plane;for(unsigned int k=0;k<4;k++)*out++=plane[k];}for(unsigned int j=0;j<v.count;j++){*out++=v.faces[j].count;const unsigned int* points=(const unsigned int*)v.faces[j].points;for(unsigned int k=0;k<v.faces[j].count*3;k++)*out++=points[k];}}
  return ((unsigned int)((unsigned char*)out-data)==size)||Fail("Invalid serialized size");
 }
};
// Source brush hierarchy and native plane generation. Cleanup, nonconvex decomposition,
// nested region precedence and unusual links still need a broader original corpus.
}




