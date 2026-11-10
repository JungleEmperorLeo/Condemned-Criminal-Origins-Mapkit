#define FEAR_SOURCE_GEOMETRY_REPAIR_DEFAULT 1
#include "WorldPacker.cpp"
#include "RenderSourceGlobalMeshes.h"
#include "SourceExpandedRenderContours.h"
#include "GameClassMetadata.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
static bool Read(HANDLE file,void* data,unsigned int bytes){DWORD done;return !bytes||(ReadFile(file,data,bytes,&done,0)&&done==bytes);}
static bool Eof(HANDLE file){unsigned char extra;DWORD done;return ReadFile(file,&extra,1,&done,0)&&!done;}
static bool Write(HANDLE file,const void* data,unsigned int bytes){DWORD done;return !bytes||(WriteFile(file,data,bytes,&done,0)&&done==bytes);}
static bool Path(const char* name,char* result,bool required=true){unsigned int n=GetEnvironmentVariableA(name,result,MAX_PATH);if(n>=MAX_PATH)return false;if(!n)result[0]=0;return n||!required;}
static bool Option(const char* name,bool fallback){char value[8];unsigned int n=GetEnvironmentVariableA(name,value,8);if(!n)return fallback;if(n>=8)ExitProcess(1);if(FearBrushFlags::Equal(value,"TRUE")||FearBrushFlags::Equal(value,"T"))return true;if(FearBrushFlags::Equal(value,"FALSE")||FearBrushFlags::Equal(value,"F"))return false;ExitProcess(1);return false;}
static void Error(const char* stage,const char* message,unsigned int code){if(stage)LogLine(stage);if(message)LogLine(message);ExitProcess(code);}
struct ClassContext {GameClassMetadata::Catalog* catalog;SourceExpandedRenderContours::Consumed* consumed;};
static WorldModels::ClassKind ResolveClass(const PrefabScene::Node& n,void* c){return GameClassMetadata::Catalog::Resolve(n,((ClassContext*)c)->catalog);}
static bool IgnoreClassBrush(const PrefabScene::Node& n,void* c){return ((ClassContext*)c)->consumed->Ignore(n);}
extern "C" void HarnessEntry(){
 char sourcePath[MAX_PATH],input[MAX_PATH],offsetPath[MAX_PATH],output[MAX_PATH],metadataPath[MAX_PATH],stem[MAX_PATH],projectRoot[MAX_PATH];
 if(!Path("FEAR_READER_INPUT",sourcePath)||!Path("FEAR_SOURCE_GLOBAL_INPUT",input)||!Path("FEAR_SOURCE_GLOBAL_OFFSET_INPUT",offsetPath)||!Path("FEAR_SOURCE_GLOBAL_OUTPUT",output)||!Path("FEAR_SOURCE_GLOBAL_METADATA",metadataPath)||!Path("FEAR_SOURCE_GLOBAL_WORLD_STEM",stem)||!Path("FEAR_SOURCE_GLOBAL_PROJECT_ROOT",projectRoot,false))ExitProcess(1);
 bool split=Option("FEAR_SOURCE_GLOBAL_SPLIT_MESHES",false),center=Option("FEAR_SOURCE_GLOBAL_CENTER_WORLD",true),all=Option("FEAR_SOURCE_GLOBAL_ALL_TREES",false),shared=Option("FEAR_SOURCE_GLOBAL_SHARED",false);
 if(shared&&!all)ExitProcess(1);
 PrefabScene::Arena memory;WorldText::Vec3 packetOffset;
 HANDLE file=CreateFileA(offsetPath,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);if(file==INVALID_HANDLE_VALUE||GetFileSize(file,0)!=12||!Read(file,&packetOffset,12)||!WorldText::FiniteVector(packetOffset))ExitProcess(2);CloseHandle(file);
 file=CreateFileA(input,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);unsigned int h[4];if(file==INVALID_HANDLE_VALUE||!Read(file,h,16)||h[0]!=0x31525053||!h[1]||h[1]>65536||h[2]>4096||h[3]>1)ExitProcess(2);
 unsigned int count=h[2],nodeCount=h[1],totalVertices=0,totalIndices=0;RenderPackedSection::Input* sections=(RenderPackedSection::Input*)memory.Allocate(count*sizeof(RenderPackedSection::Input));if(!sections)ExitProcess(3);
 for(unsigned int i=0;i<count;i++){
  unsigned int s[10];if(!Read(file,s,40)||s[0]>=nodeCount||s[1]>3||!s[2]||s[2]>4096||s[3]>=4096||s[5]<2||s[5]>64||s[6]>65535||s[7]>4000000||s[8]>100000||s[9]||!RenderPackedLimits::Fits(s[6],totalVertices,RenderPackedLimits::MaximumVertices)||!RenderPackedLimits::Fits(s[8]*3,totalIndices,RenderPackedLimits::MaximumIndices))ExitProcess(3);totalVertices+=s[6];totalIndices+=s[8]*3;
  char* name=(char*)memory.Allocate(s[2]+1);RenderVertexPacking::Element* declaration=(RenderVertexPacking::Element*)memory.Allocate(s[5]*8);RenderAttributes::Vertex* vertices=(RenderAttributes::Vertex*)memory.Allocate(s[6]*84);unsigned int* indices=(unsigned int*)memory.Allocate(s[8]*12);if(!name||!declaration||!vertices||!indices||!Read(file,name,s[2])||!Read(file,declaration,s[5]*8)||!Read(file,vertices,s[6]*84)||!Read(file,indices,s[8]*12))ExitProcess(3);name[s[2]]=0;if(lstrlenA(name)!=(int)s[2])ExitProcess(3);
  RenderPackedSection::Input& section=sections[i];section.node=s[0];section.category=s[1];RenderPackedRanges::Input& range=section.range;range.material=name;range.shaderKey=s[3];range.orderKey=s[4];range.declaration=declaration;range.elements=s[5];range.vertices=vertices;range.vertexCount=s[6];range.workingVertexStart=s[7];range.indices=indices;range.triangles=s[8];
  for(unsigned int j=0;j<i;j++)if(sections[j].range.shaderKey==range.shaderKey&&!RenderPackedRanges::SameDeclaration(sections[j].range.declaration,sections[j].range.elements,declaration,range.elements))ExitProcess(3);
 }
 if(!Eof(file))ExitProcess(3);CloseHandle(file);
 void* stageStorage=memory.Allocate(sizeof(PrefabScene::Expansion));if(!stageStorage)ExitProcess(4);PrefabScene::Expansion* expansion=new(stageStorage)PrefabScene::Expansion(true);
 {PrefabScene::Source source;if(!source.Load(sourcePath,true)||!expansion->Build(sourcePath,source.scene))Error("Source prefab preparation failed",source.scene.doc.error,4);}
 stageStorage=memory.Allocate(sizeof(VisibilityCompiler::Builder));if(!stageStorage)ExitProcess(4);VisibilityCompiler::Builder* visibility=new(stageStorage)VisibilityCompiler::Builder();WorldText::Vec3 zero={};
 if(!visibility->Build(expansion->Scene(),center?0:&zero))Error("Source visibility preparation failed",visibility->errorSector,4);
 SourceSkyPortals::Builder sky;if(!sky.Build(expansion->Scene(),visibility))Error("Source SkyPortal preparation failed",sky.error,4);
 const SourceSkyPortals::Builder* skyInput=sky.count?&sky:0;if(sky.count&&(sky.offset.x!=visibility->offset.x||sky.offset.y!=visibility->offset.y||sky.offset.z!=visibility->offset.z))Error("Source SkyPortal explicit center profile not implemented",0,5);
 const SourceRenderAllTrees::Builder* treesInput=0;
 if(all){
  void* catalogStorage=memory.Allocate(sizeof(GameClassMetadata::Catalog));void* consumedStorage=memory.Allocate(sizeof(SourceExpandedRenderContours::Consumed));void* partitionStorage=memory.Allocate(sizeof(WorldModels::Partition));void* treesStorage=memory.Allocate(sizeof(SourceRenderAllTrees::Builder));if(!catalogStorage||!consumedStorage||!partitionStorage||!treesStorage)ExitProcess(4);
  GameClassMetadata::Catalog* catalog=new(catalogStorage)GameClassMetadata::Catalog;SourceExpandedRenderContours::Consumed* consumed=new(consumedStorage)SourceExpandedRenderContours::Consumed;WorldModels::Partition* partition=new(partitionStorage)WorldModels::Partition;SourceRenderAllTrees::Builder* allTrees=new(treesStorage)SourceRenderAllTrees::Builder;
  if(!catalog->LoadProject(sourcePath))Error("Source game class metadata failed",catalog->error,4);
  if(!consumed->Build(expansion->Scene()))Error("Source consumed geometry failed",consumed->error,4);
  ClassContext context={catalog,consumed};if(!partition->Build(expansion->Scene(),ResolveClass,&context,IgnoreClassBrush))Error("Source owner partition failed",partition->error,4);
  if(!allTrees->Build(expansion->Scene(),*partition,*visibility,skyInput,shared))Error("Source model scaffold failed",allTrees->error,4);
  treesInput=allTrees;
 }
 if(nodeCount!=(treesInput?treesInput->totalNodeCount:visibility->sectorCount))Error("Packed surface/source scaffold node count mismatch",0,5);
 RenderSourceGlobalMeshes::Block block;
 if(!block.Build(expansion->Scene(),*visibility,sections,count,packetOffset,split,stem,projectRoot,skyInput,h[3]!=0,treesInput))Error(block.errorStage,block.error,6);
 // Archive paths are explicit local caller targets. Embedded semantic Mesh00p
 // filenames are metadata and are never used as filesystem destinations here.
 file=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);unsigned int out[3]={0x31475257,block.size,block.meshCount};if(file==INVALID_HANDLE_VALUE||!Write(file,out,12)||!Write(file,block.data,block.size))ExitProcess(7);
 for(unsigned int m=0;m<block.meshCount;m++){const RenderGlobalMeshes::Mesh& mesh=block.meshes[m];unsigned int length=mesh.name?lstrlenA(mesh.name):0,record[3]={m,length,mesh.externalBytes};if(!Write(file,record,12)||!Write(file,mesh.name,length)||!Write(file,mesh.external,mesh.externalBytes))ExitProcess(7);}CloseHandle(file);
 file=CreateFileA(metadataPath,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);unsigned int report[7]={0x314d4753,block.regionCount,block.nodeCount,block.unassigned,block.ambiguous,block.splitMeshes?1u:0u,block.skyPortalCount};if(file==INVALID_HANDLE_VALUE||!Write(file,report,28)||!Write(file,&block.offset,12)||!Write(file,block.constructionIndexBySaveID,block.nodeCount*4)||!Write(file,block.ownersByConstruction,block.nodeCount*4))ExitProcess(8);
 for(unsigned int r=0;r<block.regionCount;r++){const RenderMeshGroups::Region& region=block.regions[r];unsigned int record[2]={(unsigned int)lstrlenA(region.name),region.count};if(!Write(file,record,8)||!Write(file,region.name,record[0])||!Write(file,region.sectors,region.count*4))ExitProcess(8);}CloseHandle(file);
 block.Clear();visibility->~Builder();expansion->~Expansion();ExitProcess(0);
}
