#pragma once
#include "SourcePhysicalBspPointTable.h"
#include "SourcePhysicalBspMetadata.h"
#include "SourcePhysicalBspNormals.h"
#include "PhysicalBspSerialization.h"
// One physical BSP disk block. Global normals and resource resolution are shared
// by the caller across MAIN and representative owners; reference files are absent.
namespace SourcePhysicalBspDisk {
class Builder {
 PrefabScene::Arena memory;PhysicalBspSerialization::Block output;
 Builder(const Builder&);Builder& operator=(const Builder&);
 static bool SameNormal(const WorldText::Vec3& a,const WorldText::Vec3& b){const unsigned char* x=(const unsigned char*)&a;const unsigned char* y=(const unsigned char*)&b;for(unsigned int i=0;i<12;i++)if(x[i]!=y[i])return false;return true;}
 bool Fail(const char* reason){char saved[512];lstrcpynA(saved,reason?reason:"Source BSP disk compilation failed",512);Clear();lstrcpyA(diagnostic,saved);error=diagnostic;return false;}
public:
 unsigned char* data;unsigned int size;const char* error;char diagnostic[512];
 unsigned int internalPolygons,materialPolygons,collisionPolygons; // contours entering the BSP, those with a material (packer log statistics only)
 Builder():data(0),size(0),error(0),internalPolygons(0),materialPolygons(0),collisionPolygons(0){diagnostic[0]=0;}
 void Clear(){output.Clear();memory.Clear();data=0;size=0;error=0;diagnostic[0]=0;}
 const PhysicalBspSerialization::Block& Block()const{return output;}
 bool Build(const PrefabScene::Model& model,const SourcePhysicalBspContours::Main& source,const WorldText::Vec3* normals,unsigned int normalCount,SourcePhysicalBspMetadata::Resolver& metadata,unsigned int flags){
  Clear();if(source.error||!source.contours||!source.count||!normals||!normalCount||metadata.error)return Fail("Invalid source BSP disk inputs");
  SourcePhysicalBspPointTable::Table points;if(!points.Build(source))return Fail(points.error);
  const PrefabScene::Node** nodes=(const PrefabScene::Node**)memory.Allocate((model.count+1)*sizeof(void*));unsigned int* saved=(unsigned int*)memory.Allocate(source.count*4);
  PhysicalBspSerialization::Polygon* polygons=(PhysicalBspSerialization::Polygon*)memory.Allocate(source.count*sizeof(PhysicalBspSerialization::Polygon));
  BspNodeTable::Record* records=(BspNodeTable::Record*)memory.Allocate((source.Tree().nodeCount+1)*12);if(!nodes||!saved||!polygons||!records)return Fail("Source BSP serialization tables allocation failed");
  unsigned int ordinal=0;for(const PrefabScene::Node* node=model.first;node;node=node->next){if(node->index!=ordinal||ordinal>=model.count)return Fail("Source BSP disk hierarchy mismatch");nodes[ordinal++]=node;}if(ordinal!=model.count)return Fail("Source BSP disk hierarchy incomplete");
  internalPolygons=source.count;materialPolygons=0;collisionPolygons=0;for(unsigned int d=0;d<source.count;d++)if(!(source.contours[d].polygon.flags&0x180))collisionPolygons++;
  unsigned int polygonCount=0,reservation=0;WorldText::Vec3 low={},high={},initialLow={},initialHigh={};bool hasBounds=false,hasInitialBounds=false;
  for(unsigned int i=0;i<source.count;i++){
   const SourcePhysicalBspContours::Contour& contour=source.contours[i];const FearBspSplitPolicy::Polygon& polygon=contour.polygon;saved[i]=0xffffffffu;
   if(contour.sourceNode>=model.count||contour.sourcePolygon>=nodes[contour.sourceNode]->geometry.polygons)return Fail("Invalid source BSP disk provenance");
   // "Imported %d texture flags" (0x1001D4B0) walks the BSP world polygons before
   // the original splits faces above 30 vertices: one count per source face.
   if(!i||source.contours[i-1].sourceNode!=contour.sourceNode||source.contours[i-1].sourcePolygon!=contour.sourcePolygon){bool present=false;if(!metadata.Present(nodes[contour.sourceNode]->geometry.faces[contour.sourcePolygon].material,present))return Fail(metadata.error);if(present)materialPolygons++;}
   // Cached center is established before blocker extraction. Half extents and
   // allocation reservation are updated afterward; these are distinct bounds.
   for(unsigned int p=0;p<polygon.count;p++){
    const WorldText::Vec3& point=polygon.points[p];if(!hasInitialBounds){initialLow=initialHigh=point;hasInitialBounds=true;}
    else for(unsigned int k=0;k<3;k++){if((&point.x)[k]<(&initialLow.x)[k])(&initialLow.x)[k]=(&point.x)[k];if((&point.x)[k]>(&initialHigh.x)[k])(&initialHigh.x)[k]=(&point.x)[k];}
   }
   if(!(polygon.flags&0x180)){
    if(polygon.count>PhysicalBspSerialization::MaximumReferences-reservation)return Fail("Source BSP reservation limit");reservation+=polygon.count;
    for(unsigned int p=0;p<polygon.count;p++){
     const WorldText::Vec3& point=polygon.points[p];if(!hasBounds){low=high=point;hasBounds=true;}
     else for(unsigned int k=0;k<3;k++){if((&point.x)[k]<(&low.x)[k])(&low.x)[k]=(&point.x)[k];if((&point.x)[k]>(&high.x)[k])(&high.x)[k]=(&point.x)[k];}
    }
   }
   if(!(polygon.flags&8))continue;SourcePhysicalBspMetadata::Value value;
   if(!metadata.Resolve(nodes[contour.sourceNode]->geometry.faces[contour.sourcePolygon],value))return Fail(metadata.error);
   unsigned int normal=0;for(;normal<normalCount;normal++)if(SameNormal(normals[normal],polygon.normal))break;
   if(normal==normalCount){char why[400];lstrcpyA(why,"Source BSP normal missing from global table: ");lstrcpynA(why+lstrlenA(why),nodes[contour.sourceNode]->name?nodes[contour.sourceNode]->name:"",120);char* at=why+lstrlenA(why);
    for(unsigned int k=0;k<3;k++){unsigned int bits=*(const unsigned int*)(&polygon.normal.x+k);*at++=' ';for(int s=28;s>=0;s-=4)*at++="0123456789abcdef"[(bits>>s)&15];}*at=0;return Fail(why);}saved[i]=polygonCount;PhysicalBspSerialization::Polygon& outputPolygon=polygons[polygonCount++];
   outputPolygon.indices=points.references+points.firstReference[i];outputPolygon.count=polygon.count;outputPolygon.normal=normal;outputPolygon.distance=polygon.distance;outputPolygon.material=value.material;outputPolygon.occlusion=value.occlusion;outputPolygon.flags=(unsigned char)(polygon.flags&7);
  }
  unsigned int n=0;for(const FearBspTree::Node* node=source.Tree().first;node;node=node->next){if(node->polygon->source>=source.count||saved[node->polygon->source]==0xffffffffu)return Fail("Source BSP node references an unsaved polygon");records[n].polygon=(int)saved[node->polygon->source];records[n].negative=node->negative;records[n].positive=node->positive;n++;}
  if(n!=source.Tree().nodeCount||(!hasBounds&&(n||polygonCount)))return Fail("Source BSP serialized topology/bounds mismatch");
  PhysicalBspSerialization::Input input;input.points=points.points;input.pointCount=points.pointCount;input.polygons=polygons;input.polygonCount=polygonCount;input.nodes=records;input.nodeCount=n;input.normalCount=normalCount;input.referenceReservation=reservation;input.flags=flags;input.root=source.Tree().root;
  for(unsigned int k=0;k<3;k++){volatile float span=(&high.x)[k]-(&low.x)[k],sum=(&initialHigh.x)[k]+(&initialLow.x)[k];(&input.half.x)[k]=span*0.5f;(&input.center.x)[k]=sum*0.5f;}
  if(!output.Build(input))return Fail(output.error);data=output.data;size=output.size;return true;
 }
};
}
