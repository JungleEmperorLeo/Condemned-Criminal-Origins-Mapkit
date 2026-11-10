#pragma once
#include "SourceExpandedRenderContours.h"
#include "SourceOwnerRotation.h"
#include "WorldModelSharing.h"
namespace SourceWorldModelRenderContours {
static const unsigned int MaximumReferences=1000000,MaximumRecords=100000,MaximumContours=100000;
struct Record {unsigned int group,sourceNode,sourcePolygon,fragment,firstContour,contourCount;};
struct Contour {
 const RenderClipVertex::Vertex* vertices;unsigned int count,group,localNode,sourceNode,sourcePolygon,fragment;
 RenderTextureCoordinates::Basis bases[4];WorldText::Vec3 plane;
 unsigned int brushGroup,tangentMethod,surfaceFlags;char material[260];
};
struct Owner {
 unsigned int group,sourceNode,brushes,polygons,candidateBrushes,candidatePolygons,visibleBrushes;
 unsigned int tagReferences,initialPoints,remappedPoints,firstRecord,recordCount,firstContour,contourCount,outputVertices;
 char name[256],className[64];
};
class Builder {
 struct Link {Link* next;Contour contour;};
 PrefabScene::Arena memory;Builder(const Builder&);Builder& operator=(const Builder&);
 bool Fail(const char* message,unsigned int node=0xffffffffu,unsigned int group=0xffffffffu){Clear();error=message;errorNode=node;errorGroup=group;return false;}
 static bool Number(const char* text,float& value){WorldText::Document doc;return text&&doc.Parse(text,lstrlenA(text))&&doc.Elements(0)==1&&doc.Number(doc.First(0),value)&&WorldText::Finite(value);}
 static bool Properties(const PrefabScene::Node& node,float& crease,unsigned int& tangent){
  crease=45;tangent=0;for(const PrefabScene::Property* p=node.properties;p;p=p->next){
   if(FearBrushFlags::Equal(p->name,"CreaseAngle")&&!Number(p->data,crease))return false;
   if(FearBrushFlags::Equal(p->name,"TangentMethod")){if(!p->string)return false;
    if(FearBrushFlags::Equal(p->data,"World"))tangent=0;else if(FearBrushFlags::Equal(p->data,"Smooth"))tangent=1;else if(FearBrushFlags::Equal(p->data,"Kaldera"))tangent=2;else return false;
   }
  }return true;
 }
 // These mappings have already passed through the owner inverse. Retain their
 // zero signs and do not replay the surface identity quaternion a second time.
 static bool DirectOwnerSurface(const PrefabScene::Node& node,unsigned int polygon,SourceSurfacePreparation::Surface& output,bool allowExceptional){
  const WorldText::Geometry& geometry=node.geometry;
  if(!geometry.vertices||!geometry.faces||polygon>=geometry.polygons)return false;
  const WorldText::Polygon& face=geometry.faces[polygon];SourceSurfacePreparation::Fragment first;
  if(!SourceSurfacePreparation::FragmentAt(face,0,first)||!face.mappings||face.mappingCount>32)return false;
  for(unsigned int i=0;i<face.count;i++)if(face.indices[i]>=geometry.points||!WorldText::FiniteVector(geometry.vertices[face.indices[i]]))return false;
  SourceSurfacePreparation::Surface prepared={};prepared.owner=&node;prepared.polygonIndex=polygon;unsigned int seen=0;
  for(unsigned int i=0;i<face.mappingCount;i++){
   const WorldText::Mapping& mapping=face.mappings[i];if(mapping.channel>=4)continue;unsigned int bit=1u<<mapping.channel;
   bool exceptional=allowExceptional&&!WorldText::FiniteVector(mapping.origin)&&RenderExceptionalUV::Origin(mapping.origin,true);
   if((seen&bit)||(!mapping.valid&&!exceptional)||!RenderExceptionalUV::Origin(mapping.origin,allowExceptional)||!WorldText::FiniteVector(mapping.p)||!WorldText::FiniteVector(mapping.q))return false;
   seen|=bit;RenderTextureCoordinates::Basis& basis=prepared.bases[mapping.channel];basis.origin=mapping.origin;basis.u=mapping.p;basis.v=mapping.q;
  }
  if(seen!=15)return false;WorldText::Vec3 points[30];for(unsigned int i=0;i<first.count;i++)points[i]=geometry.vertices[first.indices[i]];
  if(!TextureOriginMinimize::Build(points,first.count,prepared.bases))return false;output=prepared;return true;
 }
 bool Localize(const PrefabScene::Node& node,const PrefabScene::Node& owner,PrefabScene::Arena& arena,PrefabScene::Node& prepared,bool allowExceptional){
  prepared=node;WorldText::Geometry& g=prepared.geometry;
  g.vertices=(WorldText::Vec3*)arena.Allocate(g.points*12);g.faces=(WorldText::Polygon*)arena.Allocate(g.polygons*sizeof(WorldText::Polygon));
  if(!g.vertices||!g.faces)return false;
  for(unsigned int i=0;i<g.points;i++)if(!SourceOwnerRotation::Local(owner,node.geometry.vertices[i],offset,g.vertices[i]))return false;
  memcpy(g.faces,node.geometry.faces,g.polygons*sizeof(WorldText::Polygon));
  float inverse[4];if(!SourceOwnerRotation::Quaternion(owner,inverse))return false;inverse[0]=-inverse[0];inverse[1]=-inverse[1];inverse[2]=-inverse[2];
  for(unsigned int p=0;p<g.polygons;p++){
   WorldText::Polygon& face=g.faces[p];if(!face.mappings||!face.mappingCount||face.mappingCount>32)return false;
   WorldText::Mapping* mappings=(WorldText::Mapping*)arena.Allocate(face.mappingCount*sizeof(WorldText::Mapping));if(!mappings)return false;
   memcpy(mappings,face.mappings,face.mappingCount*sizeof(WorldText::Mapping));face.mappings=mappings;
   for(unsigned int m=0;m<face.mappingCount;m++)if(mappings[m].channel<4){
    WorldText::Mapping& mapping=mappings[m];if(!SourceOwnerRotation::LocalTextureOrigin(owner,mapping.origin,offset,mapping.origin,allowExceptional))return false;
    mapping.p=SourceWorldModelTransform::Rotate(inverse,mapping.p);mapping.q=SourceWorldModelTransform::Rotate(inverse,mapping.q);
    if(!WorldText::FiniteVector(mapping.p)||!WorldText::FiniteVector(mapping.q))return false;
   }
   WorldText::Vec3 centeredWorldNormal;if(!SourceRenderNormals::Face(node.geometry,node.geometry.faces[p],centeredWorldNormal,&offset))return false;face.normal=SourceWorldModelTransform::Rotate(inverse,centeredWorldNormal);if(!WorldText::FiniteVector(face.normal))return false;
   face.distance=(float)SourceNormalArithmetic::DotWide(face.normal,g.vertices[face.indices[0]]);face.validPlane=true;
  }
  return true;
 }
 bool Emit(Link*& first,Link*& last,const RenderClipVertex::Vertex* vertices,unsigned int count,const PrefabScene::Node& node,unsigned int group,unsigned int polygon,unsigned int fragment,const SourceSurfacePreparation::Surface& surface,const WorldText::Vec3& plane,unsigned int tangent){
  if(contourCount==MaximumContours||count>MaximumReferences-outputVertices)return false;
  Link* item=(Link*)memory.Allocate(sizeof(Link));RenderClipVertex::Vertex* copy=(RenderClipVertex::Vertex*)memory.Allocate(count*sizeof(RenderClipVertex::Vertex));if(!item||!copy)return false;
  memcpy(copy,vertices,count*sizeof(RenderClipVertex::Vertex));item->next=0;Contour& out=item->contour;
  out.vertices=copy;out.count=count;out.group=group;out.localNode=0;out.sourceNode=node.index;out.sourcePolygon=polygon;out.fragment=fragment;
  memcpy(out.bases,surface.bases,sizeof(out.bases));out.plane=plane;out.brushGroup=node.index+1;out.tangentMethod=tangent;out.surfaceFlags=node.surfaceFlags;
  memcpy(out.material,node.geometry.faces[polygon].material,260);out.material[259]=0;
  if(last)last->next=item;else first=item;last=item;contourCount++;outputVertices+=count;return true;
 }
public:
 Record* records;Contour* contours;Owner* owners;
 unsigned int recordCount,contourCount,outputVertices,ownerCount,nonemptyOwners,sourceNodes,sourceBrushes,sourcePolygons;
 unsigned int mainBrushes,mainPolygons,ownerBrushes,ownerPolygons,candidateBrushes,candidatePolygons,visibleBrushes,tagReferences,initialPoints,remappedPoints;
 unsigned int consumedBrushes,streamingBrushes,navigationBrushes;bool sharedPrototypePlaneNormals;WorldText::Vec3 offset;const char* error;unsigned int errorNode,errorGroup;
 Builder():records(0),contours(0),owners(0),recordCount(0),contourCount(0),outputVertices(0),ownerCount(0),nonemptyOwners(0),sourceNodes(0),sourceBrushes(0),sourcePolygons(0),mainBrushes(0),mainPolygons(0),ownerBrushes(0),ownerPolygons(0),candidateBrushes(0),candidatePolygons(0),visibleBrushes(0),tagReferences(0),initialPoints(0),remappedPoints(0),consumedBrushes(0),streamingBrushes(0),navigationBrushes(0),sharedPrototypePlaneNormals(false),error(0),errorNode(0xffffffffu),errorGroup(0xffffffffu){offset.x=offset.y=offset.z=0;}
 void Clear(){memory.Clear();records=0;contours=0;owners=0;recordCount=contourCount=outputVertices=ownerCount=nonemptyOwners=sourceNodes=sourceBrushes=sourcePolygons=mainBrushes=mainPolygons=ownerBrushes=ownerPolygons=candidateBrushes=candidatePolygons=visibleBrushes=tagReferences=initialPoints=remappedPoints=consumedBrushes=streamingBrushes=navigationBrushes=0;sharedPrototypePlaneNormals=false;offset.x=offset.y=offset.z=0;error=0;errorNode=errorGroup=0xffffffffu;}
 // Pure source preparation, no filesystem or metadata/module calls. Groups stay
 // partition identities; sharing/emission/tree mapping belongs to the caller.
 // Prepare every source group, including aliases. With prototype sharing,
 // only emitted representatives can introduce global plane-normal keys.
 // Raw/unshared callers retain the default of inserting every owner.
 bool Build(const PrefabScene::Model& model,const WorldModels::Partition& partition,bool smooth=true,bool allowExceptionalUV=false,bool sharedPrototypeNormals=false){
  Clear();SourceExpandedRenderContours::Consumed consumed;if(!consumed.Build(model))return Fail(consumed.error);
  if(!WorldCenter::Compute(model,offset))return Fail("WorldModel render full-scene center failed");
  if(partition.error||!partition.groups||!partition.groupCount||partition.groupCount>4096||partition.assignmentCount>model.brushes||(!partition.assignments&&partition.assignmentCount)||partition.ignoredBrushes!=consumed.brushes||partition.assignmentCount+consumed.brushes!=model.brushes)return Fail("WorldModel render partition/consumed mismatch");
  SourceShadowOverrides::Policy shadow;if(!shadow.Build(model,partition))return Fail(shadow.error);
  sourceNodes=model.count;sourceBrushes=model.brushes;sourcePolygons=model.polygons;mainBrushes=partition.groups[0].brushes;mainPolygons=partition.groups[0].polygons;consumedBrushes=consumed.brushes;streamingBrushes=consumed.streamingBrushes;navigationBrushes=consumed.navigationBrushes;
  unsigned int* groups=(unsigned int*)memory.Allocate((model.count+1)*4);const PrefabScene::Node** nodes=(const PrefabScene::Node**)memory.Allocate((model.count+1)*sizeof(void*));
  unsigned int* actualBrushes=(unsigned int*)memory.Allocate(partition.groupCount*4);unsigned int* actualPolygons=(unsigned int*)memory.Allocate(partition.groupCount*4);
  if(!groups||!nodes||!actualBrushes||!actualPolygons)return Fail("WorldModel render assignment allocation failed");
  for(unsigned int i=0;i<model.count;i++)groups[i]=0xffffffffu;for(const PrefabScene::Node* n=model.first;n;n=n->next)nodes[n->index]=n;
  for(unsigned int i=0;i<partition.assignmentCount;i++){
   const WorldModels::Assignment& a=partition.assignments[i];if(!a.brush||a.brush->index>=model.count||nodes[a.brush->index]!=a.brush||a.brush->kind!=1||a.group>=partition.groupCount||groups[a.brush->index]!=0xffffffffu||consumed.Ignore(*a.brush))return Fail("Invalid WorldModel render brush assignment");
   if(!a.brush->knownType)return Fail("Unknown WorldModel render brush type",a.brush->index,a.group);
   bool sector=FearBrushFlags::Equal(a.brush->type,"Sector")||FearBrushFlags::Equal(a.brush->type,"ExtrudeSector")||FearBrushFlags::Equal(a.brush->type,"SemiExtrudeSector");
   if(a.group&&sector)return Fail("WorldModel-owned sector preparation unsupported",a.brush->index,a.group);
   if(a.group&&FearBrushFlags::Equal(a.brush->type,"SkyPortal"))return Fail("WorldModel-owned SkyPortal preparation unsupported",a.brush->index,a.group);
   groups[a.brush->index]=a.group;
   if(a.brush->geometry.polygons>0xffffffffu-actualPolygons[a.group])return Fail("WorldModel render assignment polygon count overflow",a.brush->index,a.group);actualBrushes[a.group]++;actualPolygons[a.group]+=a.brush->geometry.polygons;
  }
  for(unsigned int group=0;group<partition.groupCount;group++)if(actualBrushes[group]!=partition.groups[group].brushes||actualPolygons[group]!=partition.groups[group].polygons)return Fail("WorldModel render partition group count mismatch",partition.groups[group].ownerIndex,group);
  for(const PrefabScene::Node* n=model.first;n;n=n->next)if(n->kind==1&&!consumed.Ignore(*n)&&groups[n->index]==0xffffffffu)return Fail("WorldModel render brush missing from partition",n->index);
  ownerCount=partition.groupCount-1;owners=(Owner*)memory.Allocate((ownerCount+1)*sizeof(Owner));if(!owners)return Fail("WorldModel render owner allocation failed");
  for(unsigned int group=1;group<partition.groupCount;group++){
   const WorldModels::Group& g=partition.groups[group];if(!g.owner||g.ownerIndex>=model.count||nodes[g.ownerIndex]!=g.owner||g.owner->kind!=2)return Fail("Invalid WorldModel render owner",g.ownerIndex,group);
   Owner& out=owners[group-1];out.group=group;out.sourceNode=g.ownerIndex;out.brushes=g.brushes;out.polygons=g.polygons;lstrcpynA(out.name,g.owner->name,256);lstrcpynA(out.className,g.owner->className,64);
   if(!g.brushes)continue;float ownerQ[4];if(!WorldText::FiniteVector(g.owner->position)||!SourceOwnerRotation::Quaternion(*g.owner,ownerQ)||!WorldText::Finite(ownerQ[0])||!WorldText::Finite(ownerQ[1])||!WorldText::Finite(ownerQ[2])||!WorldText::Finite(ownerQ[3]))return Fail("Invalid WorldModel render owner quaternion",g.ownerIndex,group);nonemptyOwners++;
   if(g.brushes>MaximumReferences-ownerBrushes||g.polygons>MaximumReferences-ownerPolygons)return Fail("WorldModel render owner count limit",g.ownerIndex,group);ownerBrushes+=g.brushes;ownerPolygons+=g.polygons;
  }
  for(const PrefabScene::Node* n=model.first;n;n=n->next)if(groups[n->index]!=0xffffffffu&&groups[n->index]&&SourceRenderPointTags::MainWorldCandidate(*n)){
   Owner& owner=owners[groups[n->index]-1];const WorldText::Geometry& g=n->geometry;
   if(!g.polygons)continue; // emptied by the per-file weld/cleanup: no render surface
   if(!g.vertices||!g.faces||!g.points||g.points>MaximumReferences||!g.polygons||g.polygons>MaximumRecords||g.polygons>MaximumReferences-candidatePolygons)return Fail("Invalid WorldModel render candidate geometry",n->index,owner.group);
   candidateBrushes++;owner.candidateBrushes++;candidatePolygons+=g.polygons;owner.candidatePolygons+=g.polygons;
   unsigned int fragments=0;for(unsigned int p=0;p<g.polygons;p++){
    unsigned int count=SourceSurfacePreparation::FragmentCount(g.faces[p].count);if(!count||count>MaximumRecords-fragments)return Fail("WorldModel render source fragment limit",n->index,owner.group);fragments+=count;
    for(unsigned int f=0;f<count;f++){SourceSurfacePreparation::Fragment fragment;if(!SourceSurfacePreparation::FragmentAt(g.faces[p],f,fragment)||fragment.count>MaximumReferences-tagReferences)return Fail("WorldModel render tag reference limit",n->index,owner.group);tagReferences+=fragment.count;owner.tagReferences+=fragment.count;}
   }
   if(SourceRenderPointTags::RenderInputVisible(shadow.flags[n->index])){
    float crease;unsigned int tangent;if(!Properties(*n,crease,tangent))return Fail("Invalid WorldModel render crease/tangent properties",n->index,owner.group);
    visibleBrushes++;owner.visibleBrushes++;if(fragments>MaximumRecords-recordCount)return Fail("WorldModel render visible record limit",n->index,owner.group);recordCount+=fragments;
   }
  }
  WorldModelSharing::Identity sharing;if(sharedPrototypeNormals&&!sharing.Build(model,partition))return Fail("WorldModel render prototype plane identity failed");sharedPrototypePlaneNormals=sharedPrototypeNormals;
  if(sharedPrototypeNormals)for(unsigned int group=1;group<partition.groupCount;group++)if(sharing.representative[group]!=group&&!shadow.CompatibleBrushFlags(partition,group,sharing.representative[group]))return Fail("WorldModel render shared prototype shadow flags differ",partition.groups[group].ownerIndex,group);
  SourceRenderPlanePool::Pool planes;
  // Seeded like the physical normal table (18 axis/diagonal seeds).
  if(!planes.Initialize((model.polygons?model.polygons:1)+18)||!planes.Seed())return Fail("WorldModel render shared plane pool failed");
  // MAIN faces seed the shared pool in the post-ShadowOverride compile order, like the
  // main-world contours (Streets WorldModel shadow normals take a chair face's key first).
  WorldModels::CompileOrder seedOrder;if(!seedOrder.Build(model))return Fail("WorldModel render MAIN seed order failed");
  // Faces of carvers deleted by Nav enter after the other MAIN faces (physical table rule).
  bool navigation=false;for(const PrefabScene::Node* x=model.first;x;x=x->next)if(x->kind==2&&FearBrushFlags::Equal(x->className,"AINavMesh")){navigation=true;break;}
  for(unsigned int pass=0;pass<2;pass++)for(unsigned int k=0;k<seedOrder.count;k++)if(const PrefabScene::Node* n=seedOrder.nodes[seedOrder.order[k]])if(groups[n->index]==0&&SourceRenderPointTags::MainWorldCandidate(*n)&&(pass==1)==(navigation&&n->kind==1&&n->geometry.polygons!=1&&FearBrushFlags::Equal(n->type,"AINavMeshCarver")&&NativeRawNavMeshGeneral::Builder::ProjectsCarver(*n))){
   for(unsigned int p=0;p<n->geometry.polygons;p++){
    WorldText::Vec3 raw,normal,pooled;if(!SourceRenderNormals::Face(n->geometry,n->geometry.faces[p],raw,&offset)||!SourceRenderPlane::Build(raw,normal)||!planes.Get(normal,pooled))return Fail("WorldModel render MAIN plane pool seed failed",n->index);
   }
  }
  unsigned int expectedRecords=recordCount;records=(Record*)memory.Allocate((recordCount+1)*sizeof(Record));if(!records)return Fail("WorldModel render record allocation failed");recordCount=0;Link* first=0;Link* last=0;WorldText::Vec3 zero={};
  for(unsigned int group=1;group<partition.groupCount;group++){
   Owner& ownerStats=owners[group-1];const PrefabScene::Node& owner=*partition.groups[group].owner;ownerStats.firstRecord=recordCount;ownerStats.firstContour=contourCount;unsigned int verticesBefore=outputVertices;
   if(!ownerStats.tagReferences)continue;PrefabScene::Arena groupMemory;
   WorldText::Vec3* points=(WorldText::Vec3*)groupMemory.Allocate(ownerStats.tagReferences*12);unsigned int* rename=(unsigned int*)groupMemory.Allocate(ownerStats.tagReferences*4);unsigned int* result=(unsigned int*)groupMemory.Allocate(ownerStats.tagReferences*4);unsigned int* copied=(unsigned int*)groupMemory.Allocate(ownerStats.tagReferences*4);unsigned int** tags=(unsigned int**)groupMemory.Allocate((model.count+1)*sizeof(void*));
   if(!points||!rename||!result||!copied||!tags)return Fail("WorldModel render tag allocation failed",owner.index,group);
   unsigned int at=0;for(const PrefabScene::Node* n=model.first;n;n=n->next)if(groups[n->index]==group&&SourceRenderPointTags::MainWorldCandidate(*n)){
    const WorldText::Geometry& g=n->geometry;tags[n->index]=(unsigned int*)groupMemory.Allocate(g.points*4);if(!tags[n->index])return Fail("WorldModel render brush tags allocation failed",n->index,group);for(unsigned int v=0;v<g.points;v++)tags[n->index][v]=0xffffffffu;
    for(unsigned int p=0;p<g.polygons;p++)for(unsigned int f=0;f<SourceSurfacePreparation::FragmentCount(g.faces[p].count);f++){
     SourceSurfacePreparation::Fragment fragment;if(!SourceSurfacePreparation::FragmentAt(g.faces[p],f,fragment))return Fail("WorldModel render tag fragment failed",n->index,group);
     for(unsigned int v=0;v<fragment.count;v++){if(fragment.indices[v]>=g.points||at>=ownerStats.tagReferences||!SourceOwnerRotation::Local(owner,g.vertices[fragment.indices[v]],offset,points[at++]))return Fail("WorldModel render local tag point failed",n->index,group);}
    }
   }
   SourceRenderPointTags::Indexer indexer;if(at!=ownerStats.tagReferences||!indexer.Build(points,at)||!RenderPointRemap::Build(indexer.pointCount,indexer.indices,at,rename,result,copied,at,ownerStats.remappedPoints))return Fail("WorldModel render tag remapping failed",owner.index,group);ownerStats.initialPoints=indexer.pointCount;initialPoints+=indexer.pointCount;remappedPoints+=ownerStats.remappedPoints;
   at=0;for(const PrefabScene::Node* n=model.first;n;n=n->next)if(groups[n->index]==group&&SourceRenderPointTags::MainWorldCandidate(*n)){
    const WorldText::Geometry& g=n->geometry;for(unsigned int p=0;p<g.polygons;p++)for(unsigned int f=0;f<SourceSurfacePreparation::FragmentCount(g.faces[p].count);f++){
     SourceSurfacePreparation::Fragment fragment;if(!SourceSurfacePreparation::FragmentAt(g.faces[p],f,fragment))return Fail("WorldModel render tag assignment fragment failed",n->index,group);
     for(unsigned int v=0;v<fragment.count;v++){unsigned int& tag=tags[n->index][fragment.indices[v]];if(at>=ownerStats.tagReferences||(tag!=0xffffffffu&&tag!=result[at]))return Fail("WorldModel render tag identity mismatch",n->index,group);tag=result[at++];}
    }
   }
   
   RenderClipVertex::Vertex* vertices=(RenderClipVertex::Vertex*)groupMemory.Allocate(128*sizeof(RenderClipVertex::Vertex));if(!vertices)return Fail("WorldModel render attributed vertex allocation failed",owner.index,group);
   for(const PrefabScene::Node* n=model.first;n;n=n->next){
    if(groups[n->index]!=group||!SourceRenderPointTags::MainWorldCandidate(*n)||!n->geometry.polygons)continue;
    PrefabScene::Arena brushMemory;PrefabScene::Node prepared;if(!Localize(*n,owner,brushMemory,prepared,allowExceptionalUV))return Fail("WorldModel render local surface preparation failed",n->index,group);prepared.surfaceFlags=shadow.flags[n->index];
    const WorldText::Geometry& geometry=prepared.geometry;
    WorldText::Vec3* pooledPlanes=(WorldText::Vec3*)brushMemory.Allocate(geometry.polygons*12);if(!pooledPlanes)return Fail("WorldModel render prepared plane allocation failed",n->index,group);
    bool insertPlanes=!sharedPrototypeNormals||sharing.representative[group]==group;
    for(unsigned int p=0;p<geometry.polygons;p++){WorldText::Vec3 normal;if(!SourceRenderPlane::Build(geometry.faces[p].normal,normal)||(insertPlanes?!planes.Get(normal,pooledPlanes[p]):!planes.LookupOrOriginal(normal,pooledPlanes[p])))return Fail("WorldModel render shared owner plane insertion failed",n->index,group);}
    if(!SourceRenderPointTags::RenderInputVisible(shadow.flags[n->index]))continue;float crease;unsigned int tangent;if(!Properties(prepared,crease,tangent))return Fail("WorldModel render properties changed",n->index,group);
    SourceRenderNormals::Builder worldNormals;if(smooth&&!worldNormals.Build(n->geometry,&offset))return Fail("WorldModel render centered-world normals failed",n->index,group);
    float ownerInverse[4];if(!SourceOwnerRotation::Inverse(owner,ownerInverse))return Fail("WorldModel render inverse owner quaternion failed",n->index,group);
    SourceRenderNormals::Builder normals;if(smooth&&!normals.Build(geometry))return Fail("WorldModel render smooth normals failed",n->index,group);
    float threshold=0;if(smooth&&!SourceRenderNormals::Threshold(PrefabScene::Matrix::Cos(crease*0.017453292519943295f),threshold))return Fail("WorldModel render crease threshold failed",n->index,group);
    for(unsigned int p=0;p<geometry.polygons;p++){
     const WorldText::Polygon& face=geometry.faces[p];SourceSurfacePreparation::Surface surface;
     if(!DirectOwnerSurface(prepared,p,surface,allowExceptionalUV))return Fail("WorldModel render local basis/plane failed",n->index,group);
     for(unsigned int f=0;f<SourceSurfacePreparation::FragmentCount(face.count);f++){
      SourceSurfacePreparation::Fragment fragment;if(!SourceSurfacePreparation::FragmentAt(face,f,fragment))return Fail("WorldModel render attributed fragment failed",n->index,group);
      bool built=smooth?SourceAttributedContour::BuildPreparedSmoothWhiteRGB(surface,f,zero,normals,threshold,vertices,128,tags[n->index]):SourceAttributedContour::BuildPreparedFlatWhiteRGB(surface,f,zero,face.normal,vertices,128,tags[n->index]);if(!built)return Fail("WorldModel render attributed contour failed",n->index,group);
      if(smooth)for(unsigned int v=0;v<fragment.count;v++){
       WorldText::Vec3 worldCorner;if(!worldNormals.Corner(p,fragment.indices[v],threshold,worldCorner))return Fail("WorldModel render centered-world corner normal failed",n->index,group);
       WorldText::Vec3 localCorner=SourceWorldModelTransform::Rotate(ownerInverse,worldCorner);
       if(!WorldText::FiniteVector(localCorner))return Fail("WorldModel render inverse corner normal is not finite",n->index,group);
       vertices[v].attributes[4]=localCorner.x;vertices[v].attributes[5]=localCorner.y;vertices[v].attributes[6]=localCorner.z;
      }

      if(recordCount>=expectedRecords)return Fail("WorldModel render record count mismatch",n->index,group);Record& record=records[recordCount++];record.group=group;record.sourceNode=n->index;record.sourcePolygon=p;record.fragment=f;record.firstContour=contourCount;record.contourCount=1;
      if(!Emit(first,last,vertices,fragment.count,prepared,group,p,f,surface,pooledPlanes[p],tangent))return Fail("WorldModel render contour output allocation/bound",n->index,group);
     }
    }
   }
   ownerStats.recordCount=recordCount-ownerStats.firstRecord;ownerStats.contourCount=contourCount-ownerStats.firstContour;ownerStats.outputVertices=outputVertices-verticesBefore;
  }
  if(recordCount!=expectedRecords)return Fail("WorldModel render source enumeration mismatch");contours=(Contour*)memory.Allocate((contourCount+1)*sizeof(Contour));if(!contours)return Fail("WorldModel render contour vector failed");unsigned int used=0;for(Link* item=first;item;item=item->next)contours[used++]=item->contour;if(used!=contourCount)return Fail("WorldModel render contour vector mismatch");return true;
 }
};
}
