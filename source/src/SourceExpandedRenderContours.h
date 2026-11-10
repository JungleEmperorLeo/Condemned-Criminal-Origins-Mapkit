#pragma once
#include "WorldModels.h"
#include "SourceShadowOverrides.h"
#include "WorldCenter.h"
#include "SourceAttributedContour.h"
#include "SourceRenderPlane.h"
#include "SourceRenderPlanePool.h"
#include "SourceRenderPointTags.h"
#include "RenderAttributedOwnership.h"
#include "NativeRawNavMeshGeneral.h"
#include "StalkNodes.h"
#ifdef FEAR_OWNERSHIP_TRACE
// Research-only hook (scatter-research/ownership-trace.cpp); absent from production builds.
void FearOwnershipTrace(const RenderAttributedOwnership::Polygon&,const RenderAttributedOwnership::Builder&,unsigned int node,unsigned int polygon,unsigned int fragment);
#endif
namespace SourceExpandedRenderContours {
// Our workspace bounds, not original limits (SDK Asylum needs more than 100000 contours).
static const unsigned int MaximumReferences=2000000,MaximumRecords=200000,MaximumContours=200000;
// Consumed geometry has explicit ancestry. Ordinary object ancestry alone never
// consumes a brush; the class-metadata partition determines its render owner.
class Consumed {
 PrefabScene::Arena memory;
 // Navigation preprocessing uses _stricmp on authored class/type strings, not
 // GameServer class inheritance. Everything below an AINavMesh root and the
 // projected carver brushes of valid AIRegions are deleted before render
 // preparation (VA 0x100054D0; sp-oracle/omission-rules/OMISSION_RULES.md).
 static bool SameName(const char* a,const char* b){return FearBrushFlags::Equal(a,b);}
public:
 unsigned char* categories;unsigned int count,brushes,streamingBrushes,navigationBrushes,aiRegionBrushes,acceptedAIRegions,stalkBrushes;const char* error;
 Consumed():categories(0),count(0),brushes(0),streamingBrushes(0),navigationBrushes(0),aiRegionBrushes(0),acceptedAIRegions(0),stalkBrushes(0),error(0){}
 bool Fail(const char* message){memory.Clear();categories=0;count=brushes=streamingBrushes=navigationBrushes=aiRegionBrushes=acceptedAIRegions=stalkBrushes=0;error=message;return false;}
 bool Build(const PrefabScene::Model& model){
  Fail(0);if(model.count>100000)return Fail("Expanded source node limit");
  categories=(unsigned char*)memory.Allocate(model.count+1);if(!categories)return Fail("Consumed ancestry allocation failed");
  const PrefabScene::Node** nodes=(const PrefabScene::Node**)memory.Allocate((model.count+1)*sizeof(void*));
  unsigned char* accepted=(unsigned char*)memory.Allocate(model.count+1);if(!nodes||!accepted)return Fail("Consumed region validation allocation failed");
  memset(accepted,0,model.count+1);bool hasNavigation=false;unsigned int ordered=0;
  for(const PrefabScene::Node* n=model.first;n;n=n->next){
   if(n->index!=ordered||ordered>=model.count||(n->parent!=0xffffffffu&&n->parent>=ordered))return Fail("Invalid expanded source ancestry");
   nodes[ordered++]=n;if(n->kind==2&&SameName(n->className,"AINavMesh"))hasNavigation=true;
  }
  if(ordered!=model.count)return Fail("Incomplete expanded source ancestry");
  // No AINavMesh root causes the original navigation stage to return before
  // collecting regions; their authored brushes stay in ordinary preparation.
  // Regions in original traversal order (VA 0x10005120): a region is valid with
  // a direct Normal brush whose polygon count is not one; its descendants are not
  // searched, an invalid region's are. Only the valid region's direct brushes with
  // a polygon count other than one and a nonempty carver projection are deleted,
  // recursively, after Nav (VA 0x10005420 within 0x100054D0); other children stay.
  // `accepted` marks those deleted carver brushes.
  if(hasNavigation){
   unsigned char* inside=(unsigned char*)memory.Allocate(model.count+1);if(!inside)return Fail("Consumed region traversal allocation failed");
   for(const PrefabScene::Node* n=model.first;n;n=n->next){inside[n->index]=(unsigned char)(n->parent!=0xffffffffu&&inside[n->parent]);
    if(inside[n->index]||n->kind!=2||!SameName(n->className,"AIRegion"))continue;bool valid=false;
    for(const PrefabScene::Node* c=n->next;c&&!valid;c=c->next)if(c->parent==n->index&&c->kind==1&&c->geometry.polygons!=1&&SameName(c->type,"Normal"))valid=true;
    if(!valid)continue;inside[n->index]=1;acceptedAIRegions++;
    for(const PrefabScene::Node* c=n->next;c;c=c->next)if(c->parent==n->index&&c->kind==1&&c->geometry.polygons!=1&&NativeRawNavMeshGeneral::Builder::ProjectsCarver(*c))accepted[c->index]=1;}
  }
  // ConvertStalkNodes deletes a converted node's child brush before Nav and
  // geometry import (StalkNodes.h); category 16 removes it from every stage.
  unsigned char* stalk=(unsigned char*)memory.Allocate(model.count+1);if(!stalk)return Fail("Consumed stalk allocation failed");memset(stalk,0,model.count+1);
  for(const PrefabScene::Node* n=model.first;n;n=n->next)if(const PrefabScene::Node* c=StalkNodes::ConvertedChild(*n)){stalk[c->index]=1;stalkBrushes++;}
  unsigned int seen=0;for(const PrefabScene::Node* n=model.first;n;n=n->next){
   if(n->index!=seen||seen>=model.count||(n->parent!=0xffffffffu&&n->parent>=seen))return Fail("Invalid expanded source ancestry");
   unsigned char category=n->parent==0xffffffffu?0:categories[n->parent];
   // A converted Stalk brush is deleted before streaming (16 wins over 1/2).
   if(stalk[n->index])category=(unsigned char)((category&~3)|16);
   if(n->kind==2){
    if(FearBrushFlags::Equal(n->className,"StreamingRegion")){if(category&12)return Fail("StreamingRegion beneath consumed navigation");category|=1;}
    // A link outside every region is ignored by streaming but still deleted with its subtree (0x10036CF0).
    if(FearBrushFlags::Equal(n->className,"StreamingLink"))category|=2;
    if(FearBrushFlags::Equal(n->className,"AINavMesh")){if(category&3)return Fail("Navigation beneath consumed streaming volume");category|=4;}
   }
   if(n->kind==1&&accepted[n->index]&&!(category&8)){if(category&3)return Fail("AIRegion carver beneath consumed streaming volume");category|=8;}
   categories[seen++]=category;
   if(n->kind==1&&category){
    if(FearBrushFlags::Equal(n->type,"Sector")||FearBrushFlags::Equal(n->type,"ExtrudeSector")||FearBrushFlags::Equal(n->type,"SemiExtrudeSector"))return Fail("Consumed sector needs masked visibility preparation");
    brushes++;if(category&3)streamingBrushes++;if(category&12)navigationBrushes++;if(category&8)aiRegionBrushes++;
   }
  }
  if(seen!=model.count)return Fail("Incomplete expanded source ancestry");count=model.count;return true;
 }
 bool Ignore(const PrefabScene::Node& node)const{return node.index<count&&categories&&categories[node.index]!=0;}
};
struct Contour {
 const RenderClipVertex::Vertex* vertices;unsigned int count,owner,sourceNode,sourcePolygon,fragment;
 RenderTextureCoordinates::Basis bases[4];WorldText::Vec3 plane;
 unsigned int brushGroup,tangentMethod,surfaceFlags;char material[260];
};
struct Record {unsigned int sourceNode,sourcePolygon,fragment,firstContour,contourCount;};
struct DeferredOwner {unsigned int group,sourceNode,brushes,polygons;char name[256],className[64];};
class Builder {
 struct Link {Link* next;Contour contour;};
 PrefabScene::Arena memory;
 Builder(const Builder&);Builder& operator=(const Builder&);
 bool Fail(const char* message,unsigned int node=0xffffffffu){Clear();error=message;errorNode=node;return false;}
 static bool Candidate(const PrefabScene::Node& n){return SourceRenderPointTags::MainWorldCandidate(n);}
 static bool Number(const char* text,float& output){WorldText::Document doc;return text&&doc.Parse(text,lstrlenA(text))&&doc.Elements(0)==1&&doc.Number(doc.First(0),output)&&WorldText::Finite(output);}
 bool Emit(Link*& first,Link*& last,const RenderAttributedOwnership::Contour& input,const PrefabScene::Node& node,unsigned int polygon,unsigned int fragment,const SourceSurfacePreparation::Surface& surface,const WorldText::Vec3& plane,unsigned int tangent){
  if(contourCount==MaximumContours||input.count>MaximumReferences-outputVertices)return false;
  Link* item=(Link*)memory.Allocate(sizeof(Link));RenderClipVertex::Vertex* vertices=(RenderClipVertex::Vertex*)memory.Allocate(input.count*sizeof(RenderClipVertex::Vertex));
  if(!item||!vertices)return false;memcpy(vertices,input.vertices,input.count*sizeof(RenderClipVertex::Vertex));item->next=0;
  Contour& out=item->contour;out.vertices=vertices;out.count=input.count;out.owner=input.owner;out.sourceNode=node.index;out.sourcePolygon=polygon;out.fragment=fragment;
  memcpy(out.bases,surface.bases,sizeof(out.bases));out.plane=plane;out.brushGroup=node.index+1;out.tangentMethod=tangent;out.surfaceFlags=node.surfaceFlags;
  memcpy(out.material,node.geometry.faces[polygon].material,260);out.material[259]=0;
  if(last)last->next=item;else first=item;last=item;contourCount++;outputVertices+=input.count;return true;
 }
public:
 Record* records;Contour* contours;DeferredOwner* deferred;
 unsigned int recordCount,contourCount,outputVertices,sourceNodes,sourceBrushes,sourcePolygons;
 unsigned int mainBrushes,mainPolygons,candidateBrushes,candidatePolygons,visibleBrushes,tagReferences,initialPoints,remappedPoints;
 unsigned int deferredOwners,deferredBrushes,deferredPolygons,consumedBrushes,streamingBrushes,navigationBrushes,assignedContours,unassignedContours,smallFragments;
 bool mainWorldOnly;WorldText::Vec3 offset;const char* error;unsigned int errorNode;
 Builder():records(0),contours(0),deferred(0),recordCount(0),contourCount(0),outputVertices(0),sourceNodes(0),sourceBrushes(0),sourcePolygons(0),mainBrushes(0),mainPolygons(0),candidateBrushes(0),candidatePolygons(0),visibleBrushes(0),tagReferences(0),initialPoints(0),remappedPoints(0),deferredOwners(0),deferredBrushes(0),deferredPolygons(0),consumedBrushes(0),streamingBrushes(0),navigationBrushes(0),assignedContours(0),unassignedContours(0),smallFragments(0),mainWorldOnly(true),error(0),errorNode(0xffffffffu){offset.x=offset.y=offset.z=0;}
 void Clear(){memory.Clear();records=0;contours=0;deferred=0;recordCount=contourCount=outputVertices=sourceNodes=sourceBrushes=sourcePolygons=mainBrushes=mainPolygons=candidateBrushes=candidatePolygons=visibleBrushes=tagReferences=initialPoints=remappedPoints=deferredOwners=deferredBrushes=deferredPolygons=consumedBrushes=streamingBrushes=navigationBrushes=assignedContours=unassignedContours=smallFragments=0;mainWorldOnly=true;offset.x=offset.y=offset.z=0;error=0;errorNode=0xffffffffu;}
 // Full model and supplied partition remain caller-owned and unmodified. The
 // result owns all geometry/bases/provenance bytes and pending-owner names.
 bool Build(const PrefabScene::Model& model,const WorldModels::Partition& partition,const VisibilityCompiler::Builder& visibility,bool smooth=true){
  Clear();Consumed consumed;if(!consumed.Build(model))return Fail(consumed.error);
  if(visibility.errorCode||!visibility.data||!visibility.size||(!visibility.streamingSectors&&visibility.sectorCount))return Fail("Expanded render visibility not prepared");
  if(!WorldCenter::Compute(model,offset)||offset.x!=visibility.offset.x||offset.y!=visibility.offset.y||offset.z!=visibility.offset.z)return Fail("Expanded render full-scene center mismatch");
  if(partition.error||!partition.groups||!partition.groupCount||partition.groupCount>4096||partition.assignmentCount>model.brushes||(!partition.assignments&&partition.assignmentCount)||partition.ignoredBrushes!=consumed.brushes||partition.assignmentCount+consumed.brushes!=model.brushes)return Fail("Expanded render partition/consumed mismatch");
  SourceShadowOverrides::Policy shadow;if(!shadow.Build(model,partition))return Fail(shadow.error);
  sourceNodes=model.count;sourceBrushes=model.brushes;sourcePolygons=model.polygons;consumedBrushes=consumed.brushes;streamingBrushes=consumed.streamingBrushes;navigationBrushes=consumed.navigationBrushes;
  unsigned int* groups=(unsigned int*)memory.Allocate((model.count+1)*4);const PrefabScene::Node** nodes=(const PrefabScene::Node**)memory.Allocate((model.count+1)*sizeof(void*));
  if(!groups||!nodes)return Fail("Expanded render assignment allocation failed");
  for(unsigned int i=0;i<model.count;i++)groups[i]=0xffffffffu;
  for(const PrefabScene::Node* n=model.first;n;n=n->next)nodes[n->index]=n;
  // Main-world brushes are prepared over the post-ShadowOverride tree.
  WorldModels::CompileOrder compileOrder;if(!compileOrder.Build(model))return Fail("Expanded render traversal order failed");
  for(unsigned int i=0;i<partition.assignmentCount;i++){
   const WorldModels::Assignment& a=partition.assignments[i];if(!a.brush||a.brush->index>=model.count||nodes[a.brush->index]!=a.brush||a.brush->kind!=1||a.group>=partition.groupCount||groups[a.brush->index]!=0xffffffffu||consumed.Ignore(*a.brush))return Fail("Invalid expanded render brush assignment");
   if(!a.brush->knownType)return Fail("Unknown expanded render brush type",a.brush->index);
   bool sector=FearBrushFlags::Equal(a.brush->type,"Sector")||FearBrushFlags::Equal(a.brush->type,"ExtrudeSector")||FearBrushFlags::Equal(a.brush->type,"SemiExtrudeSector");
   if(a.group&&sector)return Fail("WorldModel-owned sector preparation not supported",a.brush->index);groups[a.brush->index]=a.group;
  }
  for(const PrefabScene::Node* n=model.first;n;n=n->next)if(n->kind==1&&!consumed.Ignore(*n)&&groups[n->index]==0xffffffffu)return Fail("Expanded render brush missing from partition",n->index);
  mainBrushes=partition.groups[0].brushes;mainPolygons=partition.groups[0].polygons;
  deferred=(DeferredOwner*)memory.Allocate((partition.groupCount+1)*sizeof(DeferredOwner));if(!deferred)return Fail("Pending owner allocation failed");
  for(unsigned int i=1;i<partition.groupCount;i++){
   const WorldModels::Group& g=partition.groups[i];if(!g.owner||g.ownerIndex>=model.count||nodes[g.ownerIndex]!=g.owner)return Fail("Invalid pending WorldModel owner");
   if(!g.brushes)continue;DeferredOwner& out=deferred[deferredOwners++];out.group=i;out.sourceNode=g.ownerIndex;out.brushes=g.brushes;out.polygons=g.polygons;lstrcpynA(out.name,g.owner->name,256);lstrcpynA(out.className,g.owner->className,64);deferredBrushes+=g.brushes;deferredPolygons+=g.polygons;
  }
  for(unsigned int k=0;k<compileOrder.count;k++)if(const PrefabScene::Node* n=compileOrder.nodes[compileOrder.order[k]])if(groups[n->index]==0&&Candidate(*n)){
   const WorldText::Geometry& g=n->geometry;candidateBrushes++;if(g.polygons>MaximumReferences-candidatePolygons)return Fail("Expanded candidate polygon limit",n->index);candidatePolygons+=g.polygons;
   if(!g.vertices||!g.faces||!g.points||g.points>MaximumReferences)return Fail("Invalid expanded candidate geometry",n->index);
   unsigned int fragments=0;for(unsigned int p=0;p<g.polygons;p++){
    unsigned int count=SourceSurfacePreparation::FragmentCount(g.faces[p].count);if(!count||count>MaximumRecords-fragments)return Fail("Expanded source fragment limit",n->index);fragments+=count;
    for(unsigned int f=0;f<count;f++){SourceSurfacePreparation::Fragment fragment;if(!SourceSurfacePreparation::FragmentAt(g.faces[p],f,fragment)||fragment.count>MaximumReferences-tagReferences)return Fail("Expanded tag reference limit",n->index);tagReferences+=fragment.count;}
   }
   if(SourceRenderPointTags::RenderInputVisible(shadow.flags[n->index])){visibleBrushes++;if(fragments>MaximumRecords-recordCount)return Fail("Expanded visible record limit",n->index);recordCount+=fragments;}
  }
  if(!tagReferences){recordCount=0;return true;}
  SourceRenderPlanePool::Pool planePool;
  // Seeded like the physical normal table (18 axis/diagonal seeds).
  if(!planePool.Initialize(candidatePolygons+18)||!planePool.Seed())return Fail("Expanded normal pool allocation failed");
  WorldText::Vec3* points=(WorldText::Vec3*)memory.Allocate(tagReferences*12);unsigned int* rename=(unsigned int*)memory.Allocate(tagReferences*4);unsigned int* result=(unsigned int*)memory.Allocate(tagReferences*4);unsigned int* copied=(unsigned int*)memory.Allocate(tagReferences*4);
  unsigned int** tags=(unsigned int**)memory.Allocate((model.count+1)*sizeof(void*));if(!points||!rename||!result||!copied||!tags)return Fail("Expanded point tag allocation failed");
  unsigned int at=0;for(unsigned int k=0;k<compileOrder.count;k++)if(const PrefabScene::Node* n=compileOrder.nodes[compileOrder.order[k]])if(groups[n->index]==0&&Candidate(*n)){
   const WorldText::Geometry& g=n->geometry;tags[n->index]=(unsigned int*)memory.Allocate(g.points*4);if(!tags[n->index])return Fail("Expanded brush tag allocation failed",n->index);for(unsigned int v=0;v<g.points;v++)tags[n->index][v]=0xffffffffu;
   for(unsigned int p=0;p<g.polygons;p++)for(unsigned int f=0;f<SourceSurfacePreparation::FragmentCount(g.faces[p].count);f++){
    SourceSurfacePreparation::Fragment fragment;if(!SourceSurfacePreparation::FragmentAt(g.faces[p],f,fragment))return Fail("Expanded fragment preparation failed",n->index);
    for(unsigned int v=0;v<fragment.count;v++){if(fragment.indices[v]>=g.points)return Fail("Expanded source point index outside brush",n->index);WorldText::Vec3 point=g.vertices[fragment.indices[v]];point.x-=offset.x;point.y-=offset.y;point.z-=offset.z;points[at++]=point;}
   }
  }
  SourceRenderPointTags::Indexer indexer;if(at!=tagReferences||!indexer.Build(points,tagReferences)||!RenderPointRemap::Build(indexer.pointCount,indexer.indices,tagReferences,rename,result,copied,tagReferences,remappedPoints))return Fail("Expanded source point remapping failed");initialPoints=indexer.pointCount;
  at=0;for(unsigned int k=0;k<compileOrder.count;k++)if(const PrefabScene::Node* n=compileOrder.nodes[compileOrder.order[k]])if(groups[n->index]==0&&Candidate(*n)){
   const WorldText::Geometry& g=n->geometry;for(unsigned int p=0;p<g.polygons;p++)for(unsigned int f=0;f<SourceSurfacePreparation::FragmentCount(g.faces[p].count);f++){
    SourceSurfacePreparation::Fragment fragment;if(!SourceSurfacePreparation::FragmentAt(g.faces[p],f,fragment))return Fail("Expanded tag fragment failed",n->index);
    for(unsigned int v=0;v<fragment.count;v++){unsigned int& tag=tags[n->index][fragment.indices[v]];if(tag!=0xffffffffu&&tag!=result[at])return Fail("Expanded point tag identity mismatch",n->index);tag=result[at++];}
   }
  }
  records=(Record*)memory.Allocate((recordCount+1)*sizeof(Record));if(!records)return Fail("Expanded contour record allocation failed");unsigned int expectedRecords=recordCount;recordCount=0;Link* first=0;Link* last=0;
  RenderClipVertex::Vertex* vertices=(RenderClipVertex::Vertex*)memory.Allocate(128*sizeof(RenderClipVertex::Vertex));if(!vertices)return Fail("Expanded fragment vertex allocation failed");
  // The shared normal pool receives faces of carvers deleted by Nav after the
  // other candidates (physical table rule, original sort input on Cafeteria).
  bool navigation=false;for(const PrefabScene::Node* x=model.first;x;x=x->next)if(x->kind==2&&FearBrushFlags::Equal(x->className,"AINavMesh")){navigation=true;break;}
  for(unsigned int k=0;k<compileOrder.count;k++){const PrefabScene::Node* n=compileOrder.nodes[compileOrder.order[k]];
   if(!n||groups[n->index]!=0||!Candidate(*n)||(navigation&&n->kind==1&&n->geometry.polygons!=1&&FearBrushFlags::Equal(n->type,"AINavMeshCarver")&&NativeRawNavMeshGeneral::Builder::ProjectsCarver(*n)))continue;
   for(unsigned int p=0;p<n->geometry.polygons;p++){WorldText::Vec3 raw,normal,pooled;if(!SourceRenderNormals::Face(n->geometry,n->geometry.faces[p],raw,&offset)||!SourceRenderPlane::Build(raw,normal)||!planePool.Get(normal,pooled))return Fail("Expanded source normal seed failed",n->index);}
  }
  for(unsigned int k=0;k<compileOrder.count;k++){const PrefabScene::Node* n=compileOrder.nodes[compileOrder.order[k]];
   if(groups[n->index]!=0||!Candidate(*n))continue;
   bool visible=SourceRenderPointTags::RenderInputVisible(shadow.flags[n->index]);
   float crease=45;unsigned int tangent=0;for(const PrefabScene::Property* p=visible?n->properties:0;p;p=p->next){
    if(FearBrushFlags::Equal(p->name,"CreaseAngle")&&!Number(p->data,crease))return Fail("Invalid expanded CreaseAngle",n->index);
    if(FearBrushFlags::Equal(p->name,"TangentMethod")){if(!p->string)return Fail("Invalid expanded TangentMethod property",n->index);if(FearBrushFlags::Equal(p->data,"World"))tangent=0;else if(FearBrushFlags::Equal(p->data,"Smooth"))tangent=1;else if(FearBrushFlags::Equal(p->data,"Kaldera"))tangent=2;else return Fail("Unsupported expanded TangentMethod",n->index);}
   }
   PrefabScene::Arena brushMemory;PrefabScene::Node prepared=*n;prepared.surfaceFlags=shadow.flags[n->index];WorldText::Geometry& g=prepared.geometry;g.faces=(WorldText::Polygon*)brushMemory.Allocate(g.polygons*sizeof(WorldText::Polygon));if(!g.faces)return Fail("Expanded source face allocation failed",n->index);memcpy(g.faces,n->geometry.faces,g.polygons*sizeof(WorldText::Polygon));
   WorldText::Vec3* pooledPlanes=(WorldText::Vec3*)brushMemory.Allocate(g.polygons*12);if(!pooledPlanes)return Fail("Expanded source plane allocation failed",n->index);
   // Surfaces are prepared before the render visibility gate. PhysicsOnly and
   // other non-render candidates can supply the first normal for a pooled key.
   for(unsigned int p=0;p<g.polygons;p++){WorldText::Polygon& face=g.faces[p];WorldText::Vec3 normal;if(!SourceRenderNormals::Face(g,face,face.normal,&offset)||!SourceRenderPlane::Build(face.normal,normal)||!planePool.Get(normal,pooledPlanes[p]))return Fail("Expanded source normal failed",n->index);face.distance=(float)SourceNormalArithmetic::DotWide(face.normal,g.vertices[face.indices[0]]);}
   if(!visible||!g.polygons)continue; // a brush emptied by the per-file weld/cleanup has no render surface
   SourceRenderNormals::Builder normals;if(smooth&&!normals.Build(g,&offset))return Fail("Expanded smooth normals failed",n->index);float threshold=0;if(smooth&&!SourceRenderNormals::Threshold(PrefabScene::Matrix::Cos(crease*0.017453292519943295f),threshold))return Fail("Expanded crease threshold failed",n->index);
   for(unsigned int p=0;p<g.polygons;p++){
    const WorldText::Polygon& face=g.faces[p];SourceSurfacePreparation::Surface surface;const WorldText::Vec3& pooled=pooledPlanes[p];
    if(!SourceSurfacePreparation::Build(prepared,p,offset,surface))return Fail("Expanded source surface preparation failed",n->index);
    for(unsigned int f=0;f<SourceSurfacePreparation::FragmentCount(face.count);f++){
     SourceSurfacePreparation::Fragment fragment;if(!SourceSurfacePreparation::FragmentAt(face,f,fragment))return Fail("Expanded source fragment failed",n->index);
     bool built=smooth?SourceAttributedContour::BuildPreparedSmoothWhiteRGB(surface,f,offset,normals,threshold,vertices,128,tags[n->index]):SourceAttributedContour::BuildPreparedFlatWhiteRGB(surface,f,offset,face.normal,vertices,128,tags[n->index]);if(!built)return Fail("Expanded attributed contour failed",n->index);
     // Ownership tests a polygon lying within the clip epsilon of a sector plane by its pooled plane normal (Facility 0.4-unit pipe slivers).
      RenderAttributedOwnership::Polygon polygon={vertices,fragment.count,pooled};RenderAttributedOwnership::Builder ownership;if(!ownership.Build(&polygon,1,visibility))return Fail(ownership.error,n->index);
#ifdef FEAR_OWNERSHIP_TRACE
      FearOwnershipTrace(polygon,ownership,n->index,p,f);
#endif
     if(recordCount>=expectedRecords)return Fail("Expanded contour record count mismatch",n->index);Record& record=records[recordCount++];record.sourceNode=n->index;record.sourcePolygon=p;record.fragment=f;record.firstContour=contourCount;record.contourCount=ownership.count;
     for(unsigned int c=0;c<ownership.count;c++)if(!Emit(first,last,ownership.contours[c],prepared,p,f,surface,pooled,tangent))return Fail("Expanded contour output allocation/bound",n->index);
     assignedContours+=ownership.assigned;unassignedContours+=ownership.unassigned;smallFragments+=ownership.smallFragments;
    }
   }
  }
  if(recordCount!=expectedRecords)return Fail("Expanded source enumeration mismatch");
  contours=(Contour*)memory.Allocate((contourCount+1)*sizeof(Contour));if(!contours)return Fail("Expanded contour vector allocation failed");unsigned int used=0;for(Link* item=first;item;item=item->next)contours[used++]=item->contour;
  if(used!=contourCount)return Fail("Expanded contour vector count mismatch");return true;
 }
};
}
