#pragma once
#ifdef FEAR_SHARED_PICK
unsigned int FearSharedPick(unsigned int,const unsigned int*,unsigned int,const unsigned int*);
#endif
#include "NativePreparedRenderScene.h"
#include "SourceRenderFanBridge.h"
#include "SourceRenderStaging.h"
#include "SourceRenderTJunctions.h"
#include "RenderShadowStaging.h"
#include "RenderIndexedSections.h"
#include "NativeMaterialLayouts.h"
#include "RenderSourceGlobalMeshes.h"
namespace NativeRenderCompiler {
enum Stage {Idle,MainContours,MainFrames,MainJunctions,MainNodes,OwnerContours,OwnerFrames,OwnerJunctions,OwnerNodes,MaterialLayouts,Serialize,Complete};
enum Code {Ok,InvalidArgument,Unsupported,Capacity,Allocation,Preparation,MaterialFailure,Cancelled,Reentrant};
struct Diagnostic {Code code;Stage stage;unsigned int node,group;char message[512];};
struct Settings {
 // cacheReorder: the original OptimizeVertices option gates its per-section
 // NvTriStrip pass (0x10049740); without it sections keep their input order
 // and collapsed shadow triangles. Vertices are welded either way.
 unsigned int version;bool splitMeshes,exceptionalUV,cacheReorder;const char* worldStem;const char* projectRoot;unsigned int ownedBudget;
 Settings():version(1),splitMeshes(false),exceptionalUV(true),cacheReorder(true),worldStem(0),projectRoot(0),ownedBudget(128u*1024u*1024u){}
};
// The callback result is copied before another callback. It need only remain
// valid during this call; the compiler never borrows its names/declaration.
struct Resolver {bool(*resolve)(void*,const char*,const NativeMaterialLayouts::Layout**);void* context;Resolver():resolve(0),context(0){}};
struct Observer {bool(*cancel)(void*);void(*progress)(void*,Stage,unsigned int,unsigned int);void* context;Observer():cancel(0),progress(0),context(0){}};
struct NodeStats {unsigned int node,group,sourceTriangles,fixedTriangles,indexedTriangles,vertices,surfaces,shadowTriangles,oneSided,twoSided,boundaryEdges,retainedTriangles,shadowBase;SourceRenderTJunctions::Stats junction;};
struct Stats {unsigned int trees,nodes,surfaces,mainFanTriangles,ownerFanTriangles,filteredMainFan,filteredOwnerFan,aliases,representatives,materials,shaders,workingVertices,indexedTriangles,ownedBytes;};
class Builder {
 struct Material {char name[260],shader[264];RenderVertexPacking::Element declaration[64];unsigned int elements,shaderKey;};
 PrefabScene::Arena owned;RenderSourceGlobalMeshes::Block block;Material* materials;const char** shaders;RenderPackedSection::Input* sections;NodeStats* reports;
 unsigned int materialCount,shaderCount,sectionCount,bytes,vertexCount,triangleCount;bool running,ready;
 Settings settings;Resolver resolver;Observer observer;Diagnostic diagnostic;Stats stats;
 Builder(const Builder&);Builder& operator=(const Builder&);
 void Release(){block.Clear();owned.Clear();materials=0;shaders=0;sections=0;reports=0;materialCount=shaderCount=sectionCount=bytes=vertexCount=triangleCount=0;ready=false;memset(&stats,0,sizeof(stats));}
 bool Fail(Code code,Stage stage,const char* message,unsigned int node=0xffffffffu,unsigned int group=0xffffffffu){
  diagnostic.code=code;diagnostic.stage=stage;diagnostic.node=node;diagnostic.group=group;lstrcpynA(diagnostic.message,message?message:"Native render compilation failed",512);return false;
 }
 void* Allocate(unsigned int count,unsigned int stride){
  if(stride&&count>settings.ownedBudget/stride)return 0;unsigned int size=count*stride;
  if(bytes>settings.ownedBudget||size>settings.ownedBudget-bytes)return 0;
  void* result=owned.Allocate(size?size:1);if(result)bytes+=size;return result;
 }
 bool Step(Stage stage,unsigned int current,unsigned int total){
  if(observer.cancel&&observer.cancel(observer.context))return Fail(Cancelled,stage,"Native render compilation cancelled");
  if(observer.progress)observer.progress(observer.context,stage,current,total);
  if(observer.cancel&&observer.cancel(observer.context))return Fail(Cancelled,stage,"Native render compilation cancelled");return true;
 }
 bool CanonicalMaterials(SourceRenderFanBridge::Builder& fan,Stage stage){
  // Resource-root separators are removed before material/category matching,
  // just as in the verified source adapter. Slash spelling inside the virtual
  // name is retained; shader filename identities have their own policy.
  for(unsigned int i=0;i<fan.contourCount;i++){
   char* name=fan.metadata[i].material;const char* source=name;while(*source=='/'||*source=='\\')++source;
   if(name[0]&&!source[0])return Fail(Preparation,stage,"Empty material resource name after root separators");
   if(source!=name){unsigned int at=0;do{name[at]=source[at];}while(name[at++]);}
  }return true;
 }
 Material* Layout(const char* name,unsigned int node,unsigned int group){
  for(unsigned int i=0;i<materialCount;i++)if(!RenderSectionRules::TextCompare(name,materials[i].name))return materials+i;
  if(materialCount==RenderPackedRanges::MaximumSections){Fail(Capacity,MaterialLayouts,"Native material catalog capacity",node,group);return 0;}
  if(!Step(MaterialLayouts,materialCount,RenderPackedRanges::MaximumSections))return 0;
  const NativeMaterialLayouts::Layout* result=0;
  if(!resolver.resolve(resolver.context,name,&result)||!result||(result->state!=NativeMaterialLayouts::Ready&&result->state!=NativeMaterialLayouts::Failed)||!result->shader||(result->state==NativeMaterialLayouts::Ready&&!result->shader[0])||!result->declaration||!result->count||result->count>64){Fail(MaterialFailure,MaterialLayouts,"Material layout resolver failed or returned an unsupported declaration",node,group);return 0;}
  unsigned int n=0,s=0;while(n<260&&name[n])n++;while(s<264&&result->shader[s])s++;
  if(!n||n==260||s==264){Fail(MaterialFailure,MaterialLayouts,"Material or shader name exceeds native layout bound",node,group);return 0;}
  Material& material=materials[materialCount];memcpy(material.name,name,n+1);memcpy(material.shader,result->shader,s+1);material.elements=result->count;memcpy(material.declaration,result->declaration,result->count*sizeof(RenderVertexPacking::Element));
  unsigned int identity=0;for(;identity<shaderCount;identity++)if(!RenderSectionRules::TextCompare(shaders[identity],material.shader))break;
  if(identity==shaderCount){if(shaderCount==RenderPackedRanges::MaximumSections){Fail(Capacity,MaterialLayouts,"Native shader identity capacity",node,group);return 0;}shaders[shaderCount++]=material.shader;}
  material.shaderKey=identity;++materialCount;return &material;
 }
 bool Append(const RenderIndexedSections::Builder& indexed,unsigned int node,unsigned int group){
  if(indexed.sectionCount>RenderPackedRanges::MaximumSections-sectionCount)return Fail(Capacity,MaterialLayouts,"Native packed section capacity",node,group);
  unsigned int at=0;
  for(unsigned int i=0;i<indexed.sectionCount;i++){
   const RenderSectionRules::Record& record=indexed.sections[i];unsigned int triangles,start,vertices;
   memcpy(&triangles,record.bytes+24,4);memcpy(&start,record.bytes+32,4);memcpy(&vertices,record.bytes+36,4);
   if(start>indexed.vertexCount||vertices>indexed.vertexCount-start||at>indexed.triangleCount||triangles>indexed.triangleCount-at||!RenderPackedLimits::Fits(vertices,vertexCount,RenderPackedLimits::MaximumVertices)||!RenderPackedLimits::Fits(triangles,triangleCount,RenderPackedLimits::MaximumTriangles))return Fail(Capacity,MaterialLayouts,"Native section range or aggregate bound",node,group);
   Material* material=Layout(RenderSectionRules::Material(record),node,group);if(!material)return false;
   RenderAttributes::Vertex* vertexCopy=(RenderAttributes::Vertex*)Allocate(vertices,sizeof(RenderAttributes::Vertex));unsigned int* indexCopy=(unsigned int*)Allocate(triangles*3,4);
   if(!vertexCopy||!indexCopy)return Fail(Allocation,MaterialLayouts,"Native owned section allocation/budget failed",node,group);
   if(vertices)memcpy(vertexCopy,indexed.vertices+start,vertices*sizeof(RenderAttributes::Vertex));if(triangles)memcpy(indexCopy,indexed.indices+at*3,triangles*12);
   RenderPackedSection::Input& out=sections[sectionCount];memset(&out,0,sizeof(out));out.node=node;out.category=(unsigned int)RenderSectionRules::Kind(record);
   out.range.material=material->name;out.range.shaderKey=material->shaderKey;out.range.orderKey=sectionCount;out.range.declaration=material->declaration;out.range.elements=material->elements;
   out.range.vertices=vertexCopy;out.range.vertexCount=vertices;out.range.workingVertexStart=start;out.range.indices=indexCopy;out.range.triangles=triangles;
   ++sectionCount;vertexCount+=vertices;triangleCount+=triangles;at+=triangles;
  }
  return at==indexed.triangleCount||Fail(Preparation,MaterialLayouts,"Native reordered triangle cursor mismatch",node,group);
 }
 bool PackNode(const SourceRenderFanBridge::Builder& fan,const SourceRenderTJunctions::Builder& fixed,const unsigned int* ordinals,unsigned int count,unsigned int node,unsigned int group){
  const Stage stage=group?OwnerNodes:MainNodes;PrefabScene::Arena scratch;unsigned int ordinary=0,shadows=0,casters[3]={},hidden[3]={};
  for(unsigned int j=0;j<count;j++){
   unsigned int t=ordinals?ordinals[j]:j;if(t>=fixed.count||fixed.vertices[t].origin>=fan.count)return Fail(Preparation,stage,"Invalid routed triangle provenance",node,group);
   const SourceRenderFanBridge::Metadata& source=fan.metadata[fan.triangles[fixed.vertices[t].origin].contourOrdinal];
   if(RenderSectionRules::RetainTriangle(source.surfaceFlags,source.material))++ordinary;if(source.surfaceFlags&0x40)++shadows;
   if(source.surfaceFlags&0x40){unsigned int c=(source.surfaceFlags>>16)&3u;if(c<3){++casters[c];if(!(source.surfaceFlags&0x22))++hidden[c];}}
  }
  if(shadows>65536||ordinary>RenderIndexedLimits::MaximumTriangles)return Fail(Capacity,stage,"Node shadow or ordinary triangle bound",node,group);
  RenderShadowStaging::Input* shadowInput=(RenderShadowStaging::Input*)scratch.Allocate((shadows?shadows:1)*sizeof(RenderShadowStaging::Input));if(!shadowInput)return Fail(Allocation,stage,"Node shadow input allocation failed",node,group);
  unsigned int at=0;
  for(unsigned int j=0;j<count;j++){
   const SourceRenderTJunctions::Triangle& triangle=fixed.vertices[ordinals?ordinals[j]:j];const SourceRenderFanBridge::Metadata& source=fan.metadata[fan.triangles[triangle.origin].contourOrdinal];
   if(!(source.surfaceFlags&0x40))continue;RenderShadowStaging::Input& shadow=shadowInput[at++];for(unsigned int c=0;c<3;c++)shadow.p[c]=triangle.vertices[c].position;shadow.normal=source.plane;shadow.category=(source.surfaceFlags>>16)&3u;
  }
  RenderShadowStaging::Builder shadow;if(!shadow.Build(shadowInput,shadows))return Fail(Preparation,stage,shadow.error,node,group);
  if(shadow.count>RenderIndexedLimits::MaximumTriangles-ordinary)return Fail(Capacity,stage,"Node staged triangle bound",node,group);
  unsigned int entries=ordinary+shadow.count;RenderIndexedSections::Triangle* input=(RenderIndexedSections::Triangle*)scratch.Allocate((entries?entries:1)*sizeof(RenderIndexedSections::Triangle));if(!input)return Fail(Allocation,stage,"Node indexed input allocation failed",node,group);
  at=0;for(unsigned int j=0;j<count;j++){
   const SourceRenderTJunctions::Triangle& triangle=fixed.vertices[ordinals?ordinals[j]:j];const SourceRenderFanBridge::Metadata& source=fan.metadata[fan.triangles[triangle.origin].contourOrdinal];
   if(!RenderSectionRules::RetainTriangle(source.surfaceFlags,source.material))continue;RenderIndexedSections::Triangle& out=input[at++];memcpy(out.vertices,triangle.vertices,sizeof(out.vertices));out.material=source.material;out.category=3;
  }
  for(unsigned int j=0;j<shadow.count;j++){RenderIndexedSections::Triangle& out=input[at++];memcpy(out.vertices,shadow.triangles[j].vertices,sizeof(out.vertices));out.material="engine\\shadowvolume.Mat00";out.category=shadow.triangles[j].category;}
  RenderIndexedSections::Builder indexed;if(at!=entries||!indexed.Build(input,entries,settings.exceptionalUV)||(settings.cacheReorder&&!indexed.Reorder()))return Fail(Preparation,stage,indexed.error?indexed.error:"Node cache reorder failed",node,group);
  if(!Append(indexed,node,group))return false;
  // Shadow-line base (0x1004B37B): each category pass 0x1004A450(0..2) with
  // casters adds the node's current triangle array (shadows of earlier passes
  // already appended) minus its casters that are not visible (0x40 without 0x22).
  unsigned int base=0,appended=0;
  for(unsigned int c=0;c<3;c++){if(!casters[c])continue;base+=count+appended-hidden[c];for(unsigned int j=0;j<shadow.count;j++)if(shadow.triangles[j].category==c)++appended;}
  NodeStats& report=reports[node];report.retainedTriangles=ordinary;report.shadowBase=base;report.fixedTriangles=count;report.indexedTriangles=indexed.triangleCount;report.vertices=indexed.vertexCount;report.surfaces=indexed.sectionCount;report.shadowTriangles=shadow.count;report.oneSided=shadow.oneSided;report.twoSided=shadow.twoSided;report.boundaryEdges=shadow.boundaryEdges;return true;
 }
 bool Fix(const SourceRenderFanBridge::Builder& fan,const unsigned int* ordinals,unsigned int count,SourceRenderTJunctions::Builder& fixed,bool main){
  if(!count)return true;if(count>SourceRenderTJunctions::MaximumInputTriangles)return Fail(Capacity,main?MainFrames:OwnerFrames,"Source T-junction input bound");
  PrefabScene::Arena trianglesMemory;SourceRenderTJunctions::Triangle* input=(SourceRenderTJunctions::Triangle*)trianglesMemory.Allocate(count*sizeof(SourceRenderTJunctions::Triangle));if(!input)return Fail(Allocation,main?MainFrames:OwnerFrames,"Source working triangle allocation failed");
  {
   PrefabScene::Arena frameMemory;SourceRenderStaging::Input* endpoints=(SourceRenderStaging::Input*)frameMemory.Allocate(count*3*sizeof(SourceRenderStaging::Input));if(!endpoints)return Fail(Allocation,main?MainFrames:OwnerFrames,"Source frame input allocation failed");
   for(unsigned int i=0;i<count;i++){
    unsigned int ordinal=ordinals?ordinals[i]:i;if(ordinal>=fan.count||fan.triangles[ordinal].contourOrdinal>=fan.contourCount)return Fail(Preparation,main?MainFrames:OwnerFrames,"Source frame provenance outside fan");
    const SourceRenderFanBridge::LinkedTriangle& triangle=fan.triangles[ordinal];const SourceRenderFanBridge::Metadata& source=fan.metadata[triangle.contourOrdinal];
    input[i].origin=ordinal;input[i].edgeFlags=triangle.geometry.edgeFlags;
    for(unsigned int c=0;c<3;c++){SourceRenderStaging::Input& out=endpoints[i*3+c];out.vertex=triangle.geometry.vertices[c];memcpy(out.bases,source.bases,sizeof(out.bases));out.plane=source.plane;out.group=source.brushGroup;out.mode=source.tangentMethod;}
   }
   if(!Step(main?MainFrames:OwnerFrames,0,count))return false;
   SourceRenderStaging::Builder staging;if(!staging.Build(endpoints,count*3,settings.exceptionalUV))return Fail(Preparation,main?MainFrames:OwnerFrames,staging.error);
   for(unsigned int i=0;i<count;i++)memcpy(input[i].vertices,staging.vertices+i*3,sizeof(input[i].vertices));
  }
  if(!Step(main?MainJunctions:OwnerJunctions,0,count))return false;
  if(!fixed.Build(input,count,settings.exceptionalUV))return Fail(Preparation,main?MainJunctions:OwnerJunctions,fixed.error);return true;
 }
 bool Main(const NativePreparedRenderScene::Context& scene){
  if(!Step(MainContours,0,1))return false;SourceRenderFanBridge::Builder fan;
  {SourceExpandedRenderContours::Builder contours;if(!contours.Build(scene.Model(),scene.Partition(),scene.Visibility(),true))return Fail(Preparation,MainContours,contours.error);
   if(!fan.BuildMain(contours.contours,contours.contourCount,settings.exceptionalUV))return Fail(Preparation,MainContours,fan.error);}
  if(!CanonicalMaterials(fan,MainContours))return false;stats.mainFanTriangles=fan.count;stats.filteredMainFan=fan.filtered;
  SourceRenderTJunctions::Builder fixed;if(!Fix(fan,0,fan.count,fixed,true))return false;
  PrefabScene::Arena routing;unsigned int nodes=scene.Visibility().sectorCount;
  unsigned int* counts=(unsigned int*)routing.Allocate(nodes*4);unsigned int* begins=(unsigned int*)routing.Allocate((nodes+1)*4);unsigned int* cursor=(unsigned int*)routing.Allocate(nodes*4);unsigned int* ordinals=(unsigned int*)routing.Allocate((fixed.count?fixed.count:1)*4);
  if(!counts||!begins||!cursor||!ordinals)return Fail(Allocation,MainNodes,"Source node routing allocation failed");memset(counts,0,nodes*4);
  for(unsigned int i=0;i<fixed.count;i++){unsigned int origin=fixed.vertices[i].origin;if(origin>=fan.count)return Fail(Preparation,MainNodes,"Source routing origin outside fan");unsigned int owner=fan.metadata[fan.triangles[origin].contourOrdinal].owner;if(owner==0xffffffffu)continue;if(owner>=nodes)return Fail(Preparation,MainNodes,"Source routing sector outside scaffold");++counts[owner];}
  begins[0]=0;for(unsigned int n=0;n<nodes;n++){begins[n+1]=begins[n]+counts[n];cursor[n]=begins[n];reports[n].node=n;reports[n].group=0;reports[n].junction=fixed.stats;}
  for(unsigned int i=0;i<fan.count;i++){unsigned int owner=fan.metadata[fan.triangles[i].contourOrdinal].owner;if(owner!=0xffffffffu){if(owner>=nodes)return Fail(Preparation,MainNodes,"Source fan sector outside scaffold");++reports[owner].sourceTriangles;}}
  for(unsigned int i=0;i<fixed.count;i++){unsigned int owner=fan.metadata[fan.triangles[fixed.vertices[i].origin].contourOrdinal].owner;if(owner!=0xffffffffu)ordinals[cursor[owner]++]=i;}
  for(unsigned int n=0;n<nodes;n++)if(!Step(MainNodes,n,nodes)||!PackNode(fan,fixed,ordinals+begins[n],counts[n],n,0))return false;
  return true;
 }
 bool Owners(const NativePreparedRenderScene::Context& scene){
  if(!Step(OwnerContours,0,1))return false;SourceRenderFanBridge::Builder fan;
  {SourceWorldModelRenderContours::Builder contours;if(!contours.Build(scene.Model(),scene.Partition(),true,settings.exceptionalUV,true))return Fail(Preparation,OwnerContours,contours.error);
   if(!fan.BuildOwners(contours.contours,contours.contourCount,settings.exceptionalUV))return Fail(Preparation,OwnerContours,fan.error);}
  if(!CanonicalMaterials(fan,OwnerContours))return false;stats.ownerFanTriangles=fan.count;stats.filteredOwnerFan=fan.filtered;const SourceRenderAllTrees::Builder& trees=scene.Trees();
  PrefabScene::Arena routing;unsigned int* counts=(unsigned int*)routing.Allocate(trees.groupCount*4);unsigned int* begin=(unsigned int*)routing.Allocate((trees.groupCount+1)*4);unsigned int* cursor=(unsigned int*)routing.Allocate(trees.groupCount*4);unsigned int* ordinals=(unsigned int*)routing.Allocate((fan.count?fan.count:1)*4);
  if(!counts||!begin||!cursor||!ordinals)return Fail(Allocation,OwnerNodes,"Owner source routing allocation failed");memset(counts,0,trees.groupCount*4);
  for(unsigned int i=0;i<fan.count;i++){unsigned int group=fan.metadata[fan.triangles[i].contourOrdinal].group;if(!group||group>=trees.groupCount)return Fail(Preparation,OwnerNodes,"Owner fan group outside source scaffold");++counts[group];}
  begin[0]=0;for(unsigned int g=0;g<trees.groupCount;g++){begin[g+1]=begin[g]+counts[g];cursor[g]=begin[g];}
  for(unsigned int i=0;i<fan.count;i++){unsigned int group=fan.metadata[fan.triangles[i].contourOrdinal].group;ordinals[cursor[group]++]=i;}
  for(unsigned int g=1;g<trees.groupCount;g++){
   if(trees.groupNode[g]==0xffffffffu){if(counts[g])return Fail(Preparation,OwnerNodes,"Empty owner has render geometry",0xffffffffu,g);continue;}
   if(trees.groupRepresentative[g]!=g)continue;unsigned int node=trees.groupNode[g];if(node>=trees.totalNodeCount||node<trees.mainNodeCount)return Fail(Preparation,OwnerNodes,"Owner node outside prepared scaffold",node,g);
   ++stats.representatives;reports[node].node=node;reports[node].group=g;reports[node].sourceTriangles=counts[g];
   if(!Step(OwnerNodes,g,trees.groupCount))return false;if(!counts[g])continue;
   unsigned int sourceGroup=g;
#ifdef FEAR_SHARED_PICK
   sourceGroup=FearSharedPick(g,trees.groupRepresentative,trees.groupCount,counts);
#endif
   SourceRenderTJunctions::Builder fixed;if(!Fix(fan,ordinals+begin[sourceGroup],counts[sourceGroup],fixed,false)||!PackNode(fan,fixed,0,fixed.count,node,g))return false;reports[node].junction=fixed.stats;
  }return true;
 }
 bool Compile(const NativePreparedRenderScene::Context& scene){
  if(!scene.Ready()||settings.version!=1||!settings.worldStem||!settings.worldStem[0]||!settings.projectRoot||!settings.projectRoot[0]||!resolver.resolve||settings.ownedBudget<1024u*1024u||settings.ownedBudget>256u*1024u*1024u)return Fail(InvalidArgument,Idle,"Invalid native render context/settings/resolver");
  const SourceRenderAllTrees::Builder& trees=scene.Trees();if(!trees.totalNodeCount||trees.totalNodeCount>65536)return Fail(Capacity,Idle,"Native render node capacity");
  sections=(RenderPackedSection::Input*)Allocate(RenderPackedRanges::MaximumSections,sizeof(RenderPackedSection::Input));materials=(Material*)Allocate(RenderPackedRanges::MaximumSections,sizeof(Material));shaders=(const char**)Allocate(RenderPackedRanges::MaximumSections,sizeof(char*));reports=(NodeStats*)Allocate(trees.totalNodeCount,sizeof(NodeStats));
  if(!sections||!materials||!shaders||!reports)return Fail(Allocation,Idle,"Native render catalog allocation/budget failed");memset(reports,0,trees.totalNodeCount*sizeof(NodeStats));
  if(!Main(scene)||!Owners(scene)||!Step(Serialize,0,sectionCount))return false;
  const SourceSkyPortals::Builder* sky=scene.Sky().count?&scene.Sky():0;
  if(!block.Build(scene.Model(),scene.Visibility(),sections,sectionCount,scene.Offset(),settings.splitMeshes,settings.worldStem,settings.projectRoot,sky,true,&trees))return Fail(Preparation,Serialize,block.error);
  if(!Step(Complete,sectionCount,sectionCount))return false;
  stats.trees=trees.layout.count;stats.nodes=trees.totalNodeCount;stats.surfaces=sectionCount;stats.aliases=trees.aliasCount;stats.materials=materialCount;stats.shaders=shaderCount;stats.workingVertices=vertexCount;stats.indexedTriangles=triangleCount;stats.ownedBytes=bytes;memset(&diagnostic,0,sizeof(diagnostic));ready=true;return true;
 }
public:
 Builder():materials(0),shaders(0),sections(0),reports(0),materialCount(0),shaderCount(0),sectionCount(0),bytes(0),vertexCount(0),triangleCount(0),running(false),ready(false){memset(&diagnostic,0,sizeof(diagnostic));memset(&stats,0,sizeof(stats));}
 void Clear(){if(running)return;Release();memset(&diagnostic,0,sizeof(diagnostic));}
 bool Build(const NativePreparedRenderScene::Context& scene,const Settings& options,const Resolver& resources,const Observer& events=Observer()){
  if(running)return Fail(Reentrant,Idle,"Reentrant native render compilation is unsupported");Clear();settings=options;resolver=resources;observer=events;running=true;bool success=Compile(scene);running=false;if(!success)Release();return success;
 }
 bool Ready()const{return ready;}const RenderSourceGlobalMeshes::Block& Result()const{return block;}const Diagnostic& Error()const{return diagnostic;}const Stats& Statistics()const{return stats;}
 const NodeStats* NodeReports()const{return reports;}
};
}
