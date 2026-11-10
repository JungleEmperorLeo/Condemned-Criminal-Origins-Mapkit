#pragma once
#include "RenderShadowStaging.h"
namespace RenderShadowClosedStaging {
// Working shadow records retain degenerate side triangles until the optional
// index reorder removes them. This bounded profile requires unique directed
// edges, a closed component and the recovered outward-orientation criterion.
// Open, duplicate-edge and two-sided components reject explicitly.
struct Input {WorldText::Vec3 points[3],normal;unsigned int category;};
using Triangle=RenderShadowStaging::Triangle;
static_assert(sizeof(Input)==52,"closed shadow input");
static_assert(sizeof(Triangle)==256,"closed shadow working output");
inline bool Same(const WorldText::Vec3& a,const WorldText::Vec3& b){return a.x==b.x&&a.y==b.y&&a.z==b.z;}
class Builder {
 PrefabScene::Arena memory;
 RenderShadowStaging::Builder staging;
 bool Fail(const char* reason){staging.Build(0,0);memory.Clear();triangles=0;count=0;error=reason;return false;}
 Builder(const Builder&);Builder& operator=(const Builder&);
public:
 Triangle* triangles;unsigned int count;const char* error;
 Builder():triangles(0),count(0),error(0){}
 bool Build(const Input* input,unsigned int size){
  Fail(0);if(!input||!size||size>65536)return Fail("Unsupported closed shadow input");
  int* adjacent=(int*)memory.Allocate(size*12);
  RenderShadowStaging::Input* converted=(RenderShadowStaging::Input*)memory.Allocate(size*sizeof(RenderShadowStaging::Input));
  if(!adjacent||!converted)return Fail("Closed shadow allocation failed");
  for(unsigned int i=0;i<size;i++){
   const Input& t=input[i];if(t.category>2||!WorldText::FiniteVector(t.normal)||(t.normal.x==0&&t.normal.y==0&&t.normal.z==0))return Fail("Invalid shadow category/normal");
   memcpy(converted[i].p,t.points,36);converted[i].normal=t.normal;converted[i].category=t.category;
   for(unsigned int e=0;e<3;e++){
    const WorldText::Vec3& a=t.points[e];const WorldText::Vec3& b=t.points[(e+1)%3];
    if(!WorldText::FiniteVector(a)||Same(a,b))return Fail("Invalid shadow edge");
    int found=-1;unsigned int forward=0,reverse=0;
    for(unsigned int j=0;j<size;j++)if(input[j].category==t.category)for(unsigned int q=0;q<3;q++){
     const WorldText::Vec3& c=input[j].points[q];const WorldText::Vec3& d=input[j].points[(q+1)%3];
     if(Same(a,c)&&Same(b,d))forward++;
     if(Same(a,d)&&Same(b,c)){reverse++;found=(int)j;}
    }
    if(forward!=1||reverse!=1)return Fail("Shadow profile requires unique closed directed edges");adjacent[i*3+e]=found;
   }
  }
  // Keep the legacy exact-edge contract as a restriction on the recovered
  // general topology. Classification uses every corner's component bounds and
  // the same x87 arithmetic as RenderShadowStaging, with no second heuristic.
  for(unsigned int category=0;category<3;category++){
   RenderShadowComponents::Mesh topology;
   if(!topology.Build(converted,size,category))return Fail(topology.error);
   for(unsigned int i=0;i<topology.count;i++){
    const RenderShadowComponents::Result& result=topology.results[i];
    if(!result.closed||!result.oneSided)return Fail("Two-sided shadow components are outside this profile");
    for(unsigned int e=0;e<3;e++){
     int neighbor=result.adjacent[e];
     if(neighbor<0||(unsigned int)neighbor>=topology.count||
        (int)topology.source[neighbor]!=adjacent[topology.source[i]*3+e])return Fail("Shadow topology is outside the exact closed profile");
    }
   }
  }
  if(!staging.Build(converted,size))return Fail(staging.error);
  if(staging.oneSided!=size||staging.twoSided||staging.boundaryEdges||staging.count!=size*4)return Fail("Unsupported closed shadow working result");
  triangles=staging.triangles;count=staging.count;
  return true;
 }
};
}
