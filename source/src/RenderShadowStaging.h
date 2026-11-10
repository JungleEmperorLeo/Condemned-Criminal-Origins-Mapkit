#pragma once
#include "RenderShadowComponents.h"
#include "RenderShadowEdges.h"
namespace RenderShadowStaging {
using Input=RenderShadowComponents::Triangle;
struct Triangle {unsigned int category;RenderAttributes::Vertex vertices[3];};
static_assert(sizeof(Triangle)==256,"shadow working payload plus category");
// Source-ordered shadow working records, separately classified per LOD.
// Working degeneracy is retained until the indexed cache pass. Shader-specific
// exclusion and render-node selection remain the caller's responsibility.
class Builder {
 PrefabScene::Arena memory;unsigned int capacity;
 bool Fail(const char* reason){memory.Clear();triangles=0;count=oneSided=twoSided=boundaryEdges=0;capacity=0;error=reason;return false;}
 Builder(const Builder&);Builder& operator=(const Builder&);
 bool Cap(const Input& input,bool reverse){
  if(count==capacity)return false;Triangle& t=triangles[count++];memset(&t,0,sizeof(t));t.category=input.category;
  WorldText::Vec3 normal=reverse?RenderShadowEdges::Flip(input.normal):input.normal;
  for(unsigned int c=0;c<3;c++){unsigned int source=reverse&&c?3-c:c;t.vertices[c].position=input.p[source];t.vertices[c].normal=normal;}return true;
 }
 bool Edge(const RenderShadowEdges::Input& input,unsigned int category){
  RenderShadowEdges::Output output;if(!RenderShadowEdges::Build(input,output)||output.count>capacity-count)return false;
  for(unsigned int i=0;i<output.count;i++){Triangle& t=triangles[count++];t.category=category;memcpy(t.vertices,output.triangles[i].vertices,252);}return true;
 }
public:
 Triangle* triangles;unsigned int count,oneSided,twoSided,boundaryEdges;const char* error;
 Builder():capacity(0),triangles(0),count(0),oneSided(0),twoSided(0),boundaryEdges(0),error(0){}
 bool Build(const Input* input,unsigned int size){
  Fail(0);if(size>65536||(!input&&size))return Fail("Unsupported shadow working input");if(!size)return true;
  for(unsigned int i=0;i<size;i++)if(input[i].category>2||!WorldText::FiniteVector(input[i].normal)||(input[i].normal.x==0&&input[i].normal.y==0&&input[i].normal.z==0))return Fail("Invalid shadow plane/category");
  capacity=size*8;triangles=(Triangle*)memory.Allocate(capacity*sizeof(Triangle));if(!triangles)return Fail("Shadow working allocation failed");
  for(unsigned int category=0;category<3;category++){
   RenderShadowComponents::Mesh topology;if(!topology.Build(input,size,category))return Fail("Shadow component preparation failed");
   for(unsigned int i=0;i<topology.count;i++){
    const Input& source=input[topology.source[i]];const RenderShadowComponents::Result& shape=topology.results[i];bool two=shape.oneSided==0;
    if(two)twoSided++;else oneSided++;
    if(!Cap(source,false)||(two&&!Cap(source,true)))return Fail("Shadow cap overflow");
    for(unsigned int e=0;e<3;e++){
     RenderShadowEdges::Input edge={};memcpy(edge.own,source.p,36);edge.ownNormal=source.normal;edge.start=e;edge.end=(e+1)%3;edge.twoSided=two?1u:0u;
     int adjacent=shape.adjacent[e];
     if(adjacent<0){edge.kind=1;boundaryEdges++;}
     else {if((unsigned int)adjacent>=topology.count)return Fail("Shadow adjacency out of range");const Input& neighbor=input[topology.source[adjacent]];memcpy(edge.neighbor,neighbor.p,36);edge.neighborNormal=neighbor.normal;}
     if(!Edge(edge,category))return Fail("Shadow edge emission failed");
    }
   }
  }
  return true;
 }
};
}
