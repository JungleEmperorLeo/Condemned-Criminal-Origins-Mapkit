#pragma once
#include "BspTree.h"
#include "BspNodeTable.h"
// Disk writer for prepared physical BSP geometry. This layer does not select
// splitters, infer solid leaves, reorder polygons or recreate saved indices.
// Those are explicit inputs from the physical compiler, rather than guesses.
namespace PhysicalBspSerialization {
static const unsigned int MaximumPoints=1000000u,MaximumPolygons=100000u,MaximumNodes=1000000u,MaximumReferences=10000000u,MaximumBytes=128u*1024u*1024u;
struct Polygon {const unsigned int* indices;unsigned int count,normal;float distance;unsigned short material;unsigned char flags,occlusion;};
struct Header {unsigned int flags,points,polygons,references,nodes;WorldText::Vec3 half,center;int root;};
static_assert(sizeof(Header)==48,"physical BSP disk header");
struct Input {
 const WorldText::Vec3* points;unsigned int pointCount;const Polygon* polygons;unsigned int polygonCount;
 const BspNodeTable::Record* nodes;unsigned int nodeCount,normalCount,referenceReservation,flags;
 WorldText::Vec3 half,center;int root;
 Input():points(0),pointCount(0),polygons(0),polygonCount(0),nodes(0),nodeCount(0),normalCount(0),referenceReservation(0),flags(0),root(-1){half.x=half.y=half.z=center.x=center.y=center.z=0;}
};
class Block {
 PrefabScene::Arena memory;unsigned int cursor;
 Block(const Block&);Block& operator=(const Block&);
 bool Fail(const char* reason){Clear();error=reason;return false;}
 bool Put(const void* source,unsigned int bytes){if(bytes>size-cursor||(!source&&bytes))return false;if(bytes)memcpy(data+cursor,source,bytes);cursor+=bytes;return true;}
 static bool Child(int value,unsigned int nodes){return value==-1||value==-2||(value>=0&&(unsigned int)value<nodes);}
 bool Topology(const Input& in){
  if(!Child(in.root,in.nodeCount))return Fail("Invalid physical BSP root");
  if(!in.nodeCount)return in.root<0||Fail("Non-leaf root without physical BSP nodes");
  if(in.root<0)return Fail("Unreachable physical BSP nodes behind leaf root");
  PrefabScene::Arena temporary;unsigned int* parents=(unsigned int*)temporary.Allocate(in.nodeCount*4);unsigned int* stack=(unsigned int*)temporary.Allocate(in.nodeCount*4);
  if(!parents||!stack)return Fail("Physical BSP topology allocation failed");memset(parents,0,in.nodeCount*4);
  for(unsigned int i=0;i<in.nodeCount;i++){
   const BspNodeTable::Record& node=in.nodes[i];if(node.polygon<0||(unsigned int)node.polygon>=in.polygonCount||!Child(node.negative,in.nodeCount)||!Child(node.positive,in.nodeCount))return Fail("Invalid physical BSP node reference");
   const int children[2]={node.negative,node.positive};for(unsigned int c=0;c<2;c++)if(children[c]>=0){unsigned int child=(unsigned int)children[c];if(child==i||++parents[child]!=1)return Fail("Physical BSP node cycle or multiple parents");}
  }
  if(parents[in.root])return Fail("Physical BSP root has a parent");for(unsigned int i=0;i<in.nodeCount;i++)if(i!=(unsigned int)in.root&&parents[i]!=1)return Fail("Unreachable physical BSP node");
  unsigned int used=0,visited=0;stack[used++]=(unsigned int)in.root;
  while(used){unsigned int index=stack[--used];if(++visited>in.nodeCount)return Fail("Physical BSP topology cycle");const BspNodeTable::Record& node=in.nodes[index];if(node.negative>=0)stack[used++]=(unsigned int)node.negative;if(node.positive>=0)stack[used++]=(unsigned int)node.positive;}
  return visited==in.nodeCount||Fail("Disconnected physical BSP node cycle");
 }
public:
 unsigned char* data;unsigned int size,savedReferences;Header header;const char* error;
 Block():cursor(0),data(0),size(0),savedReferences(0),error(0){memset(&header,0,sizeof(header));}
 void Clear(){memory.Clear();data=0;size=cursor=savedReferences=0;memset(&header,0,sizeof(header));error=0;}
 bool Build(const Input& in){
  Clear();
  if(in.pointCount>MaximumPoints||in.polygonCount>MaximumPolygons||in.nodeCount>MaximumNodes||in.normalCount>MaximumPolygons||in.referenceReservation>MaximumReferences||(!in.points&&in.pointCount)||(!in.polygons&&in.polygonCount)||(!in.nodes&&in.nodeCount))return Fail("Invalid physical BSP input bounds");
  if(!WorldText::FiniteVector(in.half)||!WorldText::FiniteVector(in.center)||in.half.x<0||in.half.y<0||in.half.z<0)return Fail("Invalid cached physical BSP bounds");
  for(unsigned int i=0;i<in.pointCount;i++)if(!WorldText::FiniteVector(in.points[i]))return Fail("Non-finite physical BSP point");
  for(unsigned int i=0;i<in.polygonCount;i++){
   const Polygon& polygon=in.polygons[i];if(!polygon.indices||polygon.count<3||polygon.count>255||polygon.normal>=in.normalCount||!WorldText::Finite(polygon.distance)||polygon.count>MaximumReferences-savedReferences)return Fail("Invalid saved physical BSP polygon");
   savedReferences+=polygon.count;for(unsigned int j=0;j<polygon.count;j++)if(polygon.indices[j]>=in.pointCount)return Fail("Physical BSP polygon point reference out of range");
  }
  // The original count is an allocation reservation over internal polygons;
  // it can exceed references actually serialized after the flag filter.
  if(in.referenceReservation<savedReferences)return Fail("Physical BSP reference reservation is too small");
  if(!Topology(in))return false;
  unsigned int needed=48u+in.polygonCount+in.polygonCount*12u+savedReferences*4u+in.nodeCount*12u+in.pointCount*12u;
  if(needed>MaximumBytes)return Fail("Physical BSP serialized byte bound");
  header.flags=in.flags;header.points=in.pointCount;header.polygons=in.polygonCount;header.references=in.referenceReservation;header.nodes=in.nodeCount;header.half=in.half;header.center=in.center;header.root=in.root;
  size=needed;data=(unsigned char*)memory.Allocate(size);if(!data)return Fail("Physical BSP output allocation failed");
  if(!Put(&header,48))return Fail("Physical BSP write overflow");
  for(unsigned int i=0;i<in.polygonCount;i++){unsigned char count=(unsigned char)in.polygons[i].count;if(!Put(&count,1))return Fail("Physical BSP vertex-count write overflow");}
  for(unsigned int i=0;i<in.polygonCount;i++){
   const Polygon& polygon=in.polygons[i];if(!Put(&polygon.flags,1)||!Put(&polygon.occlusion,1)||!Put(&polygon.material,2)||!Put(&polygon.normal,4)||!Put(&polygon.distance,4)||!Put(polygon.indices,polygon.count*4u))return Fail("Physical BSP polygon write overflow");
  }
  if(!Put(in.nodes,in.nodeCount*12u)||!Put(in.points,in.pointCount*12u)||cursor!=size)return Fail("Physical BSP node/point write overflow");return true;
 }
};
}
