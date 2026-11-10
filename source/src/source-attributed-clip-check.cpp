#define FEAR_PRIMARY_SURFACE_PREPARATION 1
#include "WorldPacker.cpp"
#include "SourceAttributedContour.h"
#include "RenderAttributedClip.h"
#include "RenderAttributes.h"
#include "SourceNormalArithmetic.h"
#include "SourceRenderNormals.h"
#include "SourceRenderPlane.h"
#include "SourceRenderPointTags.h"
#include "RenderAttributedOwnership.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
static RenderClipVertex::Vertex vertices[128];
static unsigned int tags[512];
static HANDLE metadata=INVALID_HANDLE_VALUE;
static HANDLE basisOutput=INVALID_HANDLE_VALUE;
static HANDLE frameSurfaceOutput=INVALID_HANDLE_VALUE;
static WorldText::Vec3 currentPlane;
static unsigned int currentGroup=0,currentMode=0;
static RenderTextureCoordinates::Basis currentBases[4];
static HANDLE normalInputs=INVALID_HANDLE_VALUE;
static bool CornerNormal(const WorldText::Geometry& geometry,const WorldText::Polygon& face,unsigned int point,float cosine,unsigned int mode,WorldText::Vec3& normal){
 if(mode==8){WorldText::Vec3 adjacent[64];unsigned int ids[64];unsigned int count=0;
  for(unsigned int p=0;p<geometry.polygons;p++){const WorldText::Polygon& other=geometry.faces[p];for(unsigned int j=0;j<other.count;j++)if(other.indices[j]==point){if(count==64)return false;ids[count]=p;adjacent[count++]=other.normal;break;}}
  if(!SourceNormalArithmetic::Sum(face.normal,cosine,adjacent,count,normal))return false;
  if(normalInputs!=INVALID_HANDLE_VALUE){unsigned int header[3]={0,point,count};DWORD done;if(!WriteFile(normalInputs,header,12,&done,0)||done!=12||!WriteFile(normalInputs,&face.normal,12,&done,0)||done!=12||!WriteFile(normalInputs,&cosine,4,&done,0)||done!=4||!WriteFile(normalInputs,&normal,12,&done,0)||done!=12)return false;for(unsigned int i=0;i<count;i++)if(!WriteFile(normalInputs,ids+i,4,&done,0)||done!=4||!WriteFile(normalInputs,adjacent+i,12,&done,0)||done!=12)return false;}
  return true;
 }
 normal=WorldText::Vec3();for(unsigned int p=0;p<geometry.polygons;p++){
  const WorldText::Polygon& other=geometry.faces[p];if(WorldText::Dot(face.normal,other.normal)<cosine)continue;
  for(unsigned int j=0;j<other.count;j++)if(other.indices[j]==point){RenderAttributes::Add(normal,other.normal);break;}
 }
 if(mode==0)return RenderAttributes::Normalize(normal);
 float squared=(mode==3||mode==4||mode==5)?(float)((double(normal.x)*normal.x+double(normal.y)*normal.y)+double(normal.z)*normal.z):WorldText::Dot(normal,normal);
 if(mode>=4){double wideSquared=mode==7?((double(normal.x)*normal.x+double(normal.y)*normal.y)+double(normal.z)*normal.z):double(squared);double length=RenderAttributes::Root(wideSquared);if(length<=0)return false;double inverse=mode==6?1.0/(float)length:1.0/length;if(mode==4)inverse=(float)inverse;normal.x=(float)(double(normal.x)*inverse);normal.y=(float)(double(normal.y)*inverse);normal.z=(float)(double(normal.z)*inverse);return true;}
 float length=WorldText::Sqrt(squared);if(length<=0)return false;
 if(mode==1){normal.x/=length;normal.y/=length;normal.z/=length;}
 else {float inverse=1.f/length;normal.x*=inverse;normal.y*=inverse;normal.z*=inverse;}
 return true;
}
static bool Save(HANDLE f,const RenderAttributedClip::Contour& contour,unsigned int owner){
 if(!contour.count)return true;
 unsigned int pair[2]={contour.count,owner};DWORD written;
 if(!WriteFile(metadata,pair,8,&written,0)||written!=8)return false;
 if(basisOutput!=INVALID_HANDLE_VALUE&&(!WriteFile(basisOutput,currentBases,sizeof(currentBases),&written,0)||written!=sizeof(currentBases)))return false;
 if(frameSurfaceOutput!=INVALID_HANDLE_VALUE){if(!WriteFile(frameSurfaceOutput,&currentPlane,12,&written,0)||written!=12||!WriteFile(frameSurfaceOutput,&currentGroup,4,&written,0)||written!=4||!WriteFile(frameSurfaceOutput,&currentMode,4,&written,0)||written!=4)return false;}
 DWORD done;for(unsigned int i=0;i<contour.count;i++){
  const RenderClipVertex::Vertex& v=contour.points[i];
  if(!WriteFile(f,&v,44,&done,0)||done!=44)return false;
 }return true;
}
extern "C" void HarnessEntry(){
 char input[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_READER_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_SOURCE_CLIP_OUTPUT",output,MAX_PATH))ExitProcess(1);
 PrefabScene::Source source;if(!source.Load(input,true))ExitProcess(2);
 PrefabScene::Arena sectorMemory;VisibilityCompiler::Builder* sourceVisibility=0;
 char sectorMode[4]={};if(GetEnvironmentVariableA("FEAR_SOURCE_SECTOR_OWNERSHIP",sectorMode,4)&&sectorMode[0]=='1'){
  void* expansionStorage=sectorMemory.Allocate(sizeof(PrefabScene::Expansion));void* visibilityStorage=sectorMemory.Allocate(sizeof(VisibilityCompiler::Builder));if(!expansionStorage||!visibilityStorage)ExitProcess(10);
  PrefabScene::Expansion* scene=new(expansionStorage) PrefabScene::Expansion(true);sourceVisibility=new(visibilityStorage) VisibilityCompiler::Builder;
  if(!scene->Build(input,source.scene)||!sourceVisibility->Build(scene->Scene()))ExitProcess(10);
 }
 float creaseAngle=45;const WorldText::Geometry* geometry=0;for(unsigned int i=0;i<source.scene.nodeCount;i++){const WorldText::SceneNode& node=source.scene.nodes[i];if(node.kind==1&&node.knownType&&(node.surfaceFlags&8)){geometry=&source.scene.geometry[node.geometry];currentGroup=node.geometry+1;char tangent[64]="World";unsigned int tangentProp=source.scene.Property(node.property,"TangentMethod");if(tangentProp)source.scene.doc.Copy(source.scene.Data(tangentProp),tangent,64);if(FearBrushFlags::Equal(tangent,"World"))currentMode=0;else if(FearBrushFlags::Equal(tangent,"Smooth"))currentMode=1;else if(FearBrushFlags::Equal(tangent,"Kaldera"))currentMode=2;else ExitProcess(8);unsigned int prop=source.scene.Property(node.property,"CreaseAngle");if(prop&&!source.scene.doc.Number(source.scene.Data(prop),creaseAngle))ExitProcess(8);break;}}
 if(!geometry||geometry->points>512)ExitProcess(3);for(unsigned int i=0;i<geometry->points;i++)tags[i]=i;
 char pointTags[4]={};SourceRenderPointTags::Builder sourceTags;if(GetEnvironmentVariableA("FEAR_SOURCE_POINT_TAGS",pointTags,4)&&pointTags[0]=='1'){if(!sourceTags.Build(*geometry))ExitProcess(9);for(unsigned int i=0;i<geometry->points;i++)tags[i]=sourceTags.tags[i];}
 HANDLE file=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(file==INVALID_HANDLE_VALUE)ExitProcess(4);
 char metadataPath[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_SOURCE_CLIP_METADATA",metadataPath,MAX_PATH))ExitProcess(1);
 metadata=CreateFileA(metadataPath,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(metadata==INVALID_HANDLE_VALUE)ExitProcess(4);
 char basisPath[MAX_PATH];if(GetEnvironmentVariableA("FEAR_SOURCE_CLIP_BASES",basisPath,MAX_PATH)){basisOutput=CreateFileA(basisPath,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(basisOutput==INVALID_HANDLE_VALUE)ExitProcess(4);}
 char framePath[MAX_PATH];if(GetEnvironmentVariableA("FEAR_SOURCE_FRAME_SURFACES",framePath,MAX_PATH)){frameSurfaceOutput=CreateFileA(framePath,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(frameSurfaceOutput==INVALID_HANDLE_VALUE)ExitProcess(4);}
 RenderAttributedClip::Plane plane={1,0,0,0};WorldText::Vec3 offset={};
 if(sourceVisibility)offset=sourceVisibility->offset;
 char noCut[4]={};if(GetEnvironmentVariableA("FEAR_SOURCE_NO_CUT",noCut,4)&&noCut[0]=='1')plane.d=-1000000;
 char smooth[4]={};bool smoothNormals=GetEnvironmentVariableA("FEAR_SOURCE_SMOOTH_NORMALS",smooth,4)&&smooth[0]=='1';
 char normalMode[4]={};unsigned int normalizeMode=GetEnvironmentVariableA("FEAR_SOURCE_NORMAL_MODE",normalMode,4)?(unsigned int)(normalMode[0]-'0'):0;if(normalizeMode>8)ExitProcess(8);
 char normalPath[MAX_PATH];if(GetEnvironmentVariableA("FEAR_SOURCE_NORMAL_INPUTS",normalPath,MAX_PATH)){normalInputs=CreateFileA(normalPath,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(normalInputs==INVALID_HANDLE_VALUE)ExitProcess(4);unsigned int magic=0x314e5352;DWORD done;if(!WriteFile(normalInputs,&magic,4,&done,0)||done!=4)ExitProcess(7);}
 char recompute[4]={};if(GetEnvironmentVariableA("FEAR_SOURCE_RECOMPUTE_NORMALS",recompute,4)&&recompute[0]=='1')for(unsigned int i=0;i<geometry->polygons;i++){WorldText::Polygon& f=geometry->faces[i];if(!WorldText::ComputePlane(geometry->vertices,f.indices,f.count,f.normal,f.distance))ExitProcess(8);}
 if(recompute[0]=='3')for(unsigned int p=0;p<geometry->polygons;p++){WorldText::Polygon& f=geometry->faces[p];if(!SourceRenderNormals::Face(*geometry,f,f.normal))ExitProcess(8);f.distance=(float)SourceNormalArithmetic::DotWide(f.normal,geometry->vertices[f.indices[0]]);}
 if(recompute[0]=='2'||recompute[0]=='4')for(unsigned int p=0;p<geometry->polygons;p++){WorldText::Polygon& f=geometry->faces[p];double x=0,y=0,z=0;for(unsigned int i=0,j=f.count-1;i<f.count;j=i++){const WorldText::Vec3& a=geometry->vertices[f.indices[j]];const WorldText::Vec3& b=geometry->vertices[f.indices[i]];x+=(double(a.y)-b.y)*(double(a.z)+b.z);y+=(double(a.z)-b.z)*(double(a.x)+b.x);z+=(double(a.x)-b.x)*(double(a.y)+b.y);}f.normal.x=(float)x;f.normal.y=(float)y;f.normal.z=(float)z;if(recompute[0]=='2'){if(!SourceNormalArithmetic::Normalize(f.normal))ExitProcess(8);}else{if(recompute[0]=='4'){x=f.normal.x;y=f.normal.y;z=f.normal.z;}double length=RenderAttributes::Root((x*x+y*y)+z*z);if(length<=0)ExitProcess(8);f.normal.x=(float)(x/length);f.normal.y=(float)(y/length);f.normal.z=(float)(z/length);}f.distance=(float)SourceNormalArithmetic::DotWide(f.normal,geometry->vertices[f.indices[0]]);}
 PrefabScene::Arena ownerArena;void* ownerStorage=ownerArena.Allocate(sizeof(PrefabScene::Node));if(!ownerStorage)ExitProcess(3);
 PrefabScene::Node& owner=*(new(ownerStorage) PrefabScene::Node);owner.geometry=*geometry;
 SourceRenderNormals::Builder preparedNormals;if(smoothNormals&&normalizeMode==8&&!preparedNormals.Build(owner.geometry))ExitProcess(8);
 unsigned int recordIndex=0;for(unsigned int i=0;i<geometry->polygons;i++){
  const WorldText::Polygon& face=geometry->faces[i];
  SourceSurfacePreparation::Surface surface;
  if(!SourceSurfacePreparation::Build(owner,i,offset,surface))ExitProcess(5);
  memcpy(currentBases,surface.bases,sizeof(currentBases));currentPlane=face.normal;
  if(!SourceRenderPlane::Build(face.normal,currentPlane))ExitProcess(8);
  for(unsigned int ordinal=0;ordinal<SourceSurfacePreparation::FragmentCount(face.count);ordinal++){
  SourceSurfacePreparation::Fragment fragment;if(!SourceSurfacePreparation::FragmentAt(face,ordinal,fragment)||!SourceAttributedContour::BuildPreparedFlatWhiteRGB(surface,ordinal,offset,face.normal,vertices,128,tags))ExitProcess(5);
  if(smoothNormals){float cosine=PrefabScene::Matrix::Cos(creaseAngle*0.017453292519943295f);if(normalizeMode!=8&&cosine<0)cosine=0;if(normalizeMode==8&&!SourceRenderNormals::Threshold(cosine,cosine))ExitProcess(8);for(unsigned int j=0;j<fragment.count;j++){WorldText::Vec3 normal;if(!CornerNormal(*geometry,face,fragment.indices[j],cosine,normalizeMode,normal))ExitProcess(8);vertices[j].attributes[4]=normal.x;vertices[j].attributes[5]=normal.y;vertices[j].attributes[6]=normal.z;}}
  if(smoothNormals&&normalizeMode==8){float threshold=0;if(!SourceRenderNormals::Threshold(PrefabScene::Matrix::Cos(creaseAngle*0.017453292519943295f),threshold)||!SourceAttributedContour::BuildPreparedSmoothWhiteRGB(surface,ordinal,offset,preparedNormals,threshold,vertices,128,tags))ExitProcess(8);}
  if(sourceVisibility){
   RenderAttributedOwnership::Polygon polygon={vertices,fragment.count,face.normal};RenderAttributedOwnership::Builder ownership;
   if(!ownership.Build(&polygon,1,*sourceVisibility))ExitProcess(11);
   unsigned int header[2]={recordIndex++,ownership.count};DWORD written;
   if(!WriteFile(metadata,header,8,&written,0)||written!=8)ExitProcess(7);
   for(unsigned int c=0;c<ownership.count;c++){const RenderAttributedOwnership::Contour& current=ownership.contours[c];RenderAttributedClip::Contour contour={current.vertices,current.count};if(!Save(file,contour,current.owner))ExitProcess(7);}
   continue;
  }
  RenderAttributedClip::Builder builder;if(!builder.Build(vertices,fragment.count,face.normal,&plane,1))ExitProcess(6);
  unsigned int header[2]={recordIndex++,builder.inside.count?1u:0u};for(unsigned int j=0;j<builder.outsideCount;j++)if(builder.outside[j].count)header[1]++;
  DWORD written;if(!WriteFile(metadata,header,8,&written,0)||written!=8)ExitProcess(7);
  if(!Save(file,builder.inside,true))ExitProcess(7);for(unsigned int j=0;j<builder.outsideCount;j++)if(!Save(file,builder.outside[j],false))ExitProcess(7);
 }}CloseHandle(file);CloseHandle(metadata);if(basisOutput!=INVALID_HANDLE_VALUE)CloseHandle(basisOutput);if(frameSurfaceOutput!=INVALID_HANDLE_VALUE)CloseHandle(frameSurfaceOutput);if(normalInputs!=INVALID_HANDLE_VALUE)CloseHandle(normalInputs);ExitProcess(0);
}

