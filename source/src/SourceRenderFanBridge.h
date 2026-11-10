#pragma once
#include "SourceWorldModelRenderContours.h"
#include "RenderAttributedFan.h"
#include "RenderFrameLimits.h"
namespace SourceRenderFanBridge {
// Owned bridge between typed source contours and the unchanged attributed fan.
// Triangle order is source contour order, then original fan corner order.
// Every retained triangle keeps its contour ordinal even when earlier fans
// were rejected as degenerate. No search by equal geometry reconstructs it.
static const unsigned int MaximumContours=200000u; // matches SourceExpandedRenderContours (SDK Asylum)
static const unsigned int MaximumCandidates=RenderFrameLimits::MaximumEndpoints/3u;
struct Metadata {
 RenderTextureCoordinates::Basis bases[4];WorldText::Vec3 plane;
 unsigned int owner,group,localNode,sourceNode,sourcePolygon,fragment;
 unsigned int brushGroup,tangentMethod,surfaceFlags;char material[260];
};
struct LinkedTriangle {RenderAttributedFan::Triangle geometry;unsigned int contourOrdinal;};
static_assert(sizeof(LinkedTriangle)==144,"source-linked attributed fan triangle");
static_assert(MaximumContours<=0x7fffffffu/sizeof(Metadata),"source fan metadata byte bound");
static_assert(MaximumCandidates<=0x7fffffffu/sizeof(LinkedTriangle),"source fan output byte bound");
static_assert(MaximumCandidates<=0x7fffffffu/sizeof(RenderAttributedFan::Triangle),"source fan temporary byte bound");
class Builder {
 PrefabScene::Arena memory;
 Builder(const Builder&);Builder& operator=(const Builder&);
 bool Fail(const char* reason){memory.Clear();triangles=0;metadata=0;count=contourCount=candidates=filtered=inputEndpoints=outputEndpoints=0;error=reason;return false;}
 static unsigned int Owner(const SourceExpandedRenderContours::Contour& c){return c.owner;}
 static unsigned int Owner(const SourceWorldModelRenderContours::Contour& c){return c.localNode;}
 static void Provenance(const SourceExpandedRenderContours::Contour& c,Metadata& m){m.owner=c.owner;m.group=0;m.localNode=c.owner;}
 static void Provenance(const SourceWorldModelRenderContours::Contour& c,Metadata& m){m.owner=c.localNode;m.group=c.group;m.localNode=c.localNode;}
 template<class Contour> static bool Describe(const Contour& c,Metadata& out,bool allowExceptionalUV){
  if(!WorldText::FiniteVector(c.plane)||(c.plane.x==0&&c.plane.y==0&&c.plane.z==0)||c.tangentMethod>2)return false;
  unsigned int length=0;while(length<260&&c.material[length]){if((unsigned char)c.material[length]>=128)return false;length++;}if(length==260)return false;
  for(unsigned int b=0;b<4;b++)if(!RenderExceptionalUV::Origin(c.bases[b].origin,allowExceptionalUV)||!WorldText::FiniteVector(c.bases[b].u)||!WorldText::FiniteVector(c.bases[b].v))return false;
  memset(&out,0,sizeof(out));memcpy(out.bases,c.bases,sizeof(out.bases));out.plane=c.plane;Provenance(c,out);
  out.sourceNode=c.sourceNode;out.sourcePolygon=c.sourcePolygon;out.fragment=c.fragment;
  out.brushGroup=c.brushGroup;out.tangentMethod=c.tangentMethod;out.surfaceFlags=c.surfaceFlags;memcpy(out.material,c.material,length+1);return true;
 }
 template<class Contour> bool Build(const Contour* input,unsigned int size,bool allowExceptionalUV){
  Fail(0);if(size>MaximumContours||(size&&!input))return Fail("Invalid source fan contour input");
  // Validate all count arithmetic before allocating or invoking the old fan.
  for(unsigned int i=0;i<size;i++){
   const Contour& c=input[i];if(c.count>65535||(c.count&&!c.vertices)||c.count>RenderFrameLimits::MaximumEndpoints-inputEndpoints)return Fail("Source fan endpoint limit");inputEndpoints+=c.count;
   unsigned int fans=c.count>2?c.count-2:0;if(fans>MaximumCandidates-candidates)return Fail("Source fan candidate endpoint limit");candidates+=fans;
  }
  if(!size)return true;
  metadata=(Metadata*)memory.Allocate(size*sizeof(Metadata));RenderAttributedFan::Contour* contours=(RenderAttributedFan::Contour*)memory.Allocate(size*sizeof(RenderAttributedFan::Contour));
  if(!metadata||!contours)return Fail("Source fan input allocation failed");
  for(unsigned int i=0;i<size;i++){
   const Contour& c=input[i];if(!Describe(c,metadata[i],allowExceptionalUV))return Fail("Invalid source fan metadata");
   for(unsigned int v=0;v<c.count;v++){
    const RenderClipVertex::Vertex& vertex=c.vertices[v];if(!WorldText::FiniteVector(vertex.point))return Fail("Nonfinite source fan geometry");
    for(unsigned int a=0;a<7;a++)if(!WorldText::Finite(vertex.attributes[a]))return Fail("Nonfinite source fan attribute");
   }
   contours[i].vertices=c.vertices;contours[i].count=c.count;contours[i].owner=Owner(c);
  }
  RenderAttributedFan::Builder fan;if(!fan.Build(contours,size)||fan.candidates!=candidates||fan.count>candidates||fan.filtered!=candidates-fan.count)return Fail(fan.error?fan.error:"Source fan count mismatch");
  if(fan.count){triangles=(LinkedTriangle*)memory.Allocate(fan.count*sizeof(LinkedTriangle));if(!triangles)return Fail("Source fan output allocation failed");}
  // Reuse the same viability predicate solely to transport provenance. The
  // geometry/arithmetic/edge flags come from the unchanged attributed fan.
  unsigned int retained=0;
  for(unsigned int i=0;i<size;i++)for(unsigned int corner=2;corner<input[i].count;corner++){
   const RenderClipVertex::Vertex* v=input[i].vertices;
   if(!RenderFanTriangulation::Builder::Viable(v[0].point,v[corner-1].point,v[corner].point))continue;
   if(retained>=fan.count)return Fail("Source fan provenance count mismatch");
   triangles[retained].geometry=fan.triangles[retained];triangles[retained].contourOrdinal=i;retained++;
  }
  if(retained!=fan.count)return Fail("Source fan provenance incomplete");
  count=fan.count;filtered=fan.filtered;contourCount=size;outputEndpoints=count*3u;return true;
 }
public:
 LinkedTriangle* triangles;Metadata* metadata;
 unsigned int count,contourCount,candidates,filtered,inputEndpoints,outputEndpoints;const char* error;
 Builder():triangles(0),metadata(0),count(0),contourCount(0),candidates(0),filtered(0),inputEndpoints(0),outputEndpoints(0),error(0){}
 void Clear(){Fail(0);}
 bool BuildMain(const SourceExpandedRenderContours::Contour* input,unsigned int size,bool allowExceptionalUV=false){return Build(input,size,allowExceptionalUV);}
 bool BuildOwners(const SourceWorldModelRenderContours::Contour* input,unsigned int size,bool allowExceptionalUV=false){return Build(input,size,allowExceptionalUV);}
};
}
