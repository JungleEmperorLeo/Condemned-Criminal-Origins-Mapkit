#define FEAR_PRIMARY_SURFACE_PREPARATION 1
#include "WorldPacker.cpp"
#include "SourceAttributedContour.h"
#include "RenderAttributedClip.h"
#include "SourceRenderPlane.h"
#include "SourceRenderPlanePool.h"
#include "SourceRenderPointTags.h"
#include "RenderAttributedOwnership.h"
#include "WorldCenter.h"
#include "SourceRenderSelection.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")

// Root main-world/white-RGB research profile, not full world compilation.
// Prefabs and unsupported object-owned geometry are refused before output.
// Streaming metadata brushes are excluded before candidate point/tag gathering.
// Tags include every non-sector candidate, including invisible candidates;
// emission follows authored root node/face/fragment order.
static const unsigned int MaximumReferences=1000000;
static RenderClipVertex::Vertex vertices[128];
static HANDLE metadata=INVALID_HANDLE_VALUE,basisOutput=INVALID_HANDLE_VALUE,frameSurfaceOutput=INVALID_HANDLE_VALUE,sectionSurfaceOutput=INVALID_HANDLE_VALUE;
static RenderTextureCoordinates::Basis currentBases[4];
static WorldText::Vec3 currentPlane;
static unsigned int currentGroup=0,currentMode=0;
static const char* currentMaterial=0;
static unsigned int currentSurfaceFlags=0;

static const WorldText::Scene* selectedScene=0;
static bool Candidate(const WorldText::SceneNode& node){
 return selectedScene&&SourceRenderSelection::Candidate(*selectedScene,(unsigned int)(&node-selectedScene->nodes));
}
static bool Save(HANDLE file,const RenderAttributedClip::Contour& contour,unsigned int owner){
 if(!contour.count)return true;unsigned int pair[2]={contour.count,owner};DWORD done;
 if(!WriteFile(metadata,pair,8,&done,0)||done!=8)return false;
 if(basisOutput!=INVALID_HANDLE_VALUE&&(!WriteFile(basisOutput,currentBases,sizeof(currentBases),&done,0)||done!=sizeof(currentBases)))return false;
 if(frameSurfaceOutput!=INVALID_HANDLE_VALUE&&(!WriteFile(frameSurfaceOutput,&currentPlane,12,&done,0)||done!=12||!WriteFile(frameSurfaceOutput,&currentGroup,4,&done,0)||done!=4||!WriteFile(frameSurfaceOutput,&currentMode,4,&done,0)||done!=4))return false;
 if(sectionSurfaceOutput!=INVALID_HANDLE_VALUE&&(!WriteFile(sectionSurfaceOutput,&currentSurfaceFlags,4,&done,0)||done!=4||!WriteFile(sectionSurfaceOutput,currentMaterial,260,&done,0)||done!=260))return false;
 return WriteFile(file,contour.points,contour.count*44,&done,0)&&done==contour.count*44;
}
static HANDLE Output(const char* name,bool required){
 char path[MAX_PATH];DWORD size=GetEnvironmentVariableA(name,path,MAX_PATH);if(size>=MAX_PATH||(required&&!size))ExitProcess(1);
 if(!size)return INVALID_HANDLE_VALUE;HANDLE file=CreateFileA(path,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(file==INVALID_HANDLE_VALUE)ExitProcess(4);return file;
}
extern "C" void HarnessEntry(){
 char input[MAX_PATH];DWORD inputSize=GetEnvironmentVariableA("FEAR_READER_INPUT",input,MAX_PATH);if(!inputSize||inputSize>=MAX_PATH)ExitProcess(1);
 PrefabScene::Source source;if(!source.Load(input,true))ExitProcess(2);WorldText::Scene& scene=source.scene;
 if(scene.prefabCount||scene.unknownTypes||scene.nodeCount>100000)ExitProcess(12);
 selectedScene=&scene;
 for(unsigned int n=0;n<scene.nodeCount;n++)if(scene.nodes[n].kind==1){
  if(SourceRenderSelection::Classify(scene,n)==SourceRenderSelection::Unsupported)ExitProcess(12);
 }
 // Other arithmetic modes are deliberately outside this verified profile.
 char normalMode[4]={},recompute[4]={};if(GetEnvironmentVariableA("FEAR_SOURCE_NORMAL_MODE",normalMode,4)&&normalMode[0]!='8')ExitProcess(8);
 if(GetEnvironmentVariableA("FEAR_SOURCE_RECOMPUTE_NORMALS",recompute,4)&&recompute[0]!='3')ExitProcess(8);
 PrefabScene::Arena memory;
 WorldText::Vec3 offset={};VisibilityCompiler::Builder* sourceVisibility=0;
 char sectorMode[4]={};if(GetEnvironmentVariableA("FEAR_SOURCE_SECTOR_OWNERSHIP",sectorMode,4)&&sectorMode[0]=='1'){
  void* expansionStorage=memory.Allocate(sizeof(PrefabScene::Expansion));void* visibilityStorage=memory.Allocate(sizeof(VisibilityCompiler::Builder));if(!expansionStorage||!visibilityStorage)ExitProcess(10);
  PrefabScene::Expansion* expansion=new(expansionStorage) PrefabScene::Expansion(true);sourceVisibility=new(visibilityStorage) VisibilityCompiler::Builder;
  if(!expansion->Build(input,scene)||!sourceVisibility->Build(expansion->Scene()))ExitProcess(10);offset=sourceVisibility->offset;
 }
 unsigned int references=0,candidates=0,tagEntries=0,planeCount=0;
 for(unsigned int n=0;n<scene.nodeCount;n++)if(Candidate(scene.nodes[n])){
  candidates++;const WorldText::Geometry& g=scene.geometry[scene.nodes[n].geometry];if(g.points>MaximumReferences-tagEntries||g.polygons>MaximumReferences-planeCount)ExitProcess(3);tagEntries+=g.points;planeCount+=g.polygons;
  for(unsigned int p=0;p<g.polygons;p++)for(unsigned int f=0;f<SourceSurfacePreparation::FragmentCount(g.faces[p].count);f++){
   SourceSurfacePreparation::Fragment fragment;if(!SourceSurfacePreparation::FragmentAt(g.faces[p],f,fragment)||references>MaximumReferences-fragment.count)ExitProcess(3);references+=fragment.count;
  }
 }
 if(!candidates||!references)ExitProcess(3);
 SourceRenderPlanePool::Pool planePool;if(!planePool.Initialize(planeCount))ExitProcess(3);
 WorldText::Vec3* points=(WorldText::Vec3*)memory.Allocate(references*12);
 unsigned int* rename=(unsigned int*)memory.Allocate(references*4);unsigned int* result=(unsigned int*)memory.Allocate(references*4);unsigned int* copied=(unsigned int*)memory.Allocate(references*4);
 unsigned int** brushTags=(unsigned int**)memory.Allocate(scene.nodeCount*sizeof(unsigned int*));if(!points||!rename||!result||!copied||!brushTags)ExitProcess(3);
 unsigned int at=0;
 for(unsigned int n=0;n<scene.nodeCount;n++)if(Candidate(scene.nodes[n])){
  const WorldText::Geometry& g=scene.geometry[scene.nodes[n].geometry];brushTags[n]=(unsigned int*)memory.Allocate(g.points*4);if(!brushTags[n])ExitProcess(3);for(unsigned int i=0;i<g.points;i++)brushTags[n][i]=0xffffffffu;
  for(unsigned int p=0;p<g.polygons;p++)for(unsigned int f=0;f<SourceSurfacePreparation::FragmentCount(g.faces[p].count);f++){
   SourceSurfacePreparation::Fragment fragment;if(!SourceSurfacePreparation::FragmentAt(g.faces[p],f,fragment))ExitProcess(3);
   for(unsigned int i=0;i<fragment.count;i++){if(fragment.indices[i]>=g.points)ExitProcess(3);WorldText::Vec3 point=g.vertices[fragment.indices[i]];point.x-=offset.x;point.y-=offset.y;point.z-=offset.z;points[at++]=point;}
  }
 }
 SourceRenderPointTags::Indexer indexer;unsigned int generated=0;
 if(at!=references||!indexer.Build(points,references)||!RenderPointRemap::Build(indexer.pointCount,indexer.indices,references,rename,result,copied,references,generated))ExitProcess(9);
 at=0;
 for(unsigned int n=0;n<scene.nodeCount;n++)if(Candidate(scene.nodes[n])){
  const WorldText::Geometry& g=scene.geometry[scene.nodes[n].geometry];
  for(unsigned int p=0;p<g.polygons;p++)for(unsigned int f=0;f<SourceSurfacePreparation::FragmentCount(g.faces[p].count);f++){
   SourceSurfacePreparation::Fragment fragment;if(!SourceSurfacePreparation::FragmentAt(g.faces[p],f,fragment))ExitProcess(3);
   for(unsigned int i=0;i<fragment.count;i++){unsigned int& tag=brushTags[n][fragment.indices[i]];if(tag!=0xffffffffu&&tag!=result[at])ExitProcess(9);tag=result[at++];}
  }
 }
 HANDLE file=Output("FEAR_SOURCE_CLIP_OUTPUT",true);metadata=Output("FEAR_SOURCE_CLIP_METADATA",true);basisOutput=Output("FEAR_SOURCE_CLIP_BASES",false);frameSurfaceOutput=Output("FEAR_SOURCE_FRAME_SURFACES",false);sectionSurfaceOutput=Output("FEAR_SOURCE_SECTION_SURFACES",false);
 char smooth[4]={};bool smoothNormals=GetEnvironmentVariableA("FEAR_SOURCE_SMOOTH_NORMALS",smooth,4)&&smooth[0]=='1';
 RenderAttributedClip::Plane plane={1,0,0,0};char noCut[4]={};if(GetEnvironmentVariableA("FEAR_SOURCE_NO_CUT",noCut,4)&&noCut[0]=='1')plane.d=-1000000;
 void* ownerStorage=memory.Allocate(sizeof(PrefabScene::Node));if(!ownerStorage)ExitProcess(3);PrefabScene::Node& owner=*(new(ownerStorage) PrefabScene::Node);
 unsigned int recordIndex=0;
 for(unsigned int n=0;n<scene.nodeCount;n++){
  const WorldText::SceneNode& node=scene.nodes[n];if(!Candidate(node)||!SourceRenderPointTags::RenderInputVisible(node.surfaceFlags))continue;
  WorldText::Geometry& g=scene.geometry[node.geometry];float creaseAngle=45;
  unsigned int prop=scene.Property(node.property,"CreaseAngle");if(prop&&!scene.doc.Number(scene.Data(prop),creaseAngle))ExitProcess(8);
  char tangent[64]="World";prop=scene.Property(node.property,"TangentMethod");if(prop)scene.doc.Copy(scene.Data(prop),tangent,64);
  if(FearBrushFlags::Equal(tangent,"World"))currentMode=0;else if(FearBrushFlags::Equal(tangent,"Smooth"))currentMode=1;else if(FearBrushFlags::Equal(tangent,"Kaldera"))currentMode=2;else ExitProcess(8);
  currentGroup=node.geometry+1;currentSurfaceFlags=node.surfaceFlags;
  for(unsigned int p=0;p<g.polygons;p++){WorldText::Polygon& face=g.faces[p];if(!SourceRenderNormals::Face(g,face,face.normal))ExitProcess(8);face.distance=(float)SourceNormalArithmetic::DotWide(face.normal,g.vertices[face.indices[0]]);}
  owner.geometry=g;SourceRenderNormals::Builder preparedNormals;if(smoothNormals&&!preparedNormals.Build(owner.geometry))ExitProcess(8);
  float threshold=0;if(smoothNormals&&!SourceRenderNormals::Threshold(PrefabScene::Matrix::Cos(creaseAngle*0.017453292519943295f),threshold))ExitProcess(8);
  for(unsigned int p=0;p<g.polygons;p++){
   const WorldText::Polygon& face=g.faces[p];SourceSurfacePreparation::Surface surface;
   currentMaterial=face.material;
   WorldText::Vec3 preparedPlane;if(!SourceSurfacePreparation::Build(owner,p,offset,surface)||!SourceRenderPlane::Build(face.normal,preparedPlane)||!planePool.Get(preparedPlane,currentPlane))ExitProcess(5);memcpy(currentBases,surface.bases,sizeof(currentBases));
   for(unsigned int ordinal=0;ordinal<SourceSurfacePreparation::FragmentCount(face.count);ordinal++){
    SourceSurfacePreparation::Fragment fragment;if(!SourceSurfacePreparation::FragmentAt(face,ordinal,fragment))ExitProcess(5);
    if(smoothNormals){if(!SourceAttributedContour::BuildPreparedSmoothWhiteRGB(surface,ordinal,offset,preparedNormals,threshold,vertices,128,brushTags[n]))ExitProcess(8);}
    else if(!SourceAttributedContour::BuildPreparedFlatWhiteRGB(surface,ordinal,offset,face.normal,vertices,128,brushTags[n]))ExitProcess(5);
    if(sourceVisibility){
     RenderAttributedOwnership::Polygon polygon={vertices,fragment.count,face.normal};RenderAttributedOwnership::Builder ownership;if(!ownership.Build(&polygon,1,*sourceVisibility))ExitProcess(11);
     unsigned int header[2]={recordIndex++,ownership.count};DWORD done;if(!WriteFile(metadata,header,8,&done,0)||done!=8)ExitProcess(7);
     for(unsigned int c=0;c<ownership.count;c++){const RenderAttributedOwnership::Contour& current=ownership.contours[c];RenderAttributedClip::Contour contour={current.vertices,current.count};if(!Save(file,contour,current.owner))ExitProcess(7);}
    }else{
     RenderAttributedClip::Builder builder;if(!builder.Build(vertices,fragment.count,face.normal,&plane,1))ExitProcess(6);
     unsigned int header[2]={recordIndex++,builder.inside.count?1u:0u};for(unsigned int i=0;i<builder.outsideCount;i++)if(builder.outside[i].count)header[1]++;
     DWORD done;if(!WriteFile(metadata,header,8,&done,0)||done!=8||!Save(file,builder.inside,1))ExitProcess(7);for(unsigned int i=0;i<builder.outsideCount;i++)if(!Save(file,builder.outside[i],0))ExitProcess(7);
    }
   }
  }
 }
 CloseHandle(file);CloseHandle(metadata);if(basisOutput!=INVALID_HANDLE_VALUE)CloseHandle(basisOutput);if(frameSurfaceOutput!=INVALID_HANDLE_VALUE)CloseHandle(frameSurfaceOutput);if(sectionSurfaceOutput!=INVALID_HANDLE_VALUE)CloseHandle(sectionSurfaceOutput);ExitProcess(0);
}
