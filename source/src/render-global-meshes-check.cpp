#include "WorldPacker.cpp"
#include "RenderGlobalMeshes.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
static bool Read(HANDLE file,void* data,unsigned int bytes){DWORD done;return !bytes||(ReadFile(file,data,bytes,&done,0)&&done==bytes);}
static bool Eof(HANDLE file){unsigned char extra;DWORD done;return ReadFile(file,&extra,1,&done,0)&&!done;}
static bool Write(HANDLE file,const void* data,unsigned int bytes){DWORD done;return !bytes||(WriteFile(file,data,bytes,&done,0)&&done==bytes);}
static unsigned int MainTree(){char number[12];unsigned int n=GetEnvironmentVariableA("FEAR_GLOBAL_RENDER_MAIN_TREE",number,12),value=0;if(n>=12)ExitProcess(1);for(unsigned int i=0;i<n;i++){if(number[i]<'0'||number[i]>'9'||value>409)ExitProcess(1);value=value*10+(unsigned int)(number[i]-'0');}return value;}
extern "C" void HarnessEntry(){
 char input[MAX_PATH],scaffoldPath[MAX_PATH],output[MAX_PATH],stem[MAX_PATH];
 if(!GetEnvironmentVariableA("FEAR_GLOBAL_RENDER_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_GLOBAL_RENDER_SCAFFOLD",scaffoldPath,MAX_PATH)||!GetEnvironmentVariableA("FEAR_GLOBAL_RENDER_OUTPUT",output,MAX_PATH)||!GetEnvironmentVariableA("FEAR_GLOBAL_RENDER_WORLD_STEM",stem,MAX_PATH))ExitProcess(1);
 PrefabScene::Arena memory;HANDLE file=CreateFileA(input,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);unsigned int h[4];if(file==INVALID_HANDLE_VALUE||!Read(file,h,16)||h[0]!=0x31525053||h[1]>65536||h[2]>4096||h[3]>1)ExitProcess(2);
 unsigned int count=h[2],nodeCount=h[1],mainTree=MainTree(),totalVertices=0,totalIndices=0;RenderPackedSection::Input* sections=(RenderPackedSection::Input*)memory.Allocate(count*sizeof(RenderPackedSection::Input));if(!sections)ExitProcess(3);
 for(unsigned int i=0;i<count;i++){
  unsigned int s[10];if(!Read(file,s,40)||s[0]>=nodeCount||s[1]>3||!s[2]||s[2]>4096||s[3]>=4096||s[5]<2||s[5]>64||s[6]>65535||s[7]>4000000||s[8]>100000||s[9]||!RenderPackedLimits::Fits(s[6],totalVertices,RenderPackedLimits::MaximumVertices)||!RenderPackedLimits::Fits(s[8]*3,totalIndices,RenderPackedLimits::MaximumIndices))ExitProcess(3);totalVertices+=s[6];totalIndices+=s[8]*3;
  char* name=(char*)memory.Allocate(s[2]+1);RenderVertexPacking::Element* declaration=(RenderVertexPacking::Element*)memory.Allocate(s[5]*8);RenderAttributes::Vertex* vertices=(RenderAttributes::Vertex*)memory.Allocate(s[6]*84);unsigned int* indices=(unsigned int*)memory.Allocate(s[8]*12);if(!name||!declaration||!vertices||!indices||!Read(file,name,s[2])||!Read(file,declaration,s[5]*8)||!Read(file,vertices,s[6]*84)||!Read(file,indices,s[8]*12))ExitProcess(3);name[s[2]]=0;
  RenderPackedSection::Input& section=sections[i];section.node=s[0];section.category=s[1];RenderPackedRanges::Input& range=section.range;range.material=name;range.shaderKey=s[3];range.orderKey=s[4];range.declaration=declaration;range.elements=s[5];range.vertices=vertices;range.vertexCount=s[6];range.workingVertexStart=s[7];range.indices=indices;range.triangles=s[8];
  for(unsigned int j=0;j<i;j++)if(sections[j].range.shaderKey==range.shaderKey&&!RenderPackedRanges::SameDeclaration(sections[j].range.declaration,sections[j].range.elements,declaration,range.elements))ExitProcess(3);
 }
 if(!Eof(file))ExitProcess(3);CloseHandle(file);
 file=CreateFileA(scaffoldPath,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);unsigned int treeHeader[2];if(file==INVALID_HANDLE_VALUE||!Read(file,treeHeader,8)||treeHeader[0]!=0x31544653||!treeHeader[1]||treeHeader[1]>4096||mainTree>=treeHeader[1])ExitProcess(4);
 RenderTrees::Tree* trees=(RenderTrees::Tree*)memory.Allocate(treeHeader[1]*sizeof(RenderTrees::Tree));if(!trees)ExitProcess(4);unsigned int totalNodes=0,totalSky=0,totalSkyVertices=0;
 for(unsigned int t=0;t<treeHeader[1];t++){
  unsigned int nodes;if(!Read(file,&nodes,4)||nodes>nodeCount-totalNodes)ExitProcess(4);totalNodes+=nodes;RenderTrees::Node* records=(RenderTrees::Node*)memory.Allocate(nodes*sizeof(RenderTrees::Node));if(!records)ExitProcess(4);trees[t].nodes=records;trees[t].count=nodes;
  for(unsigned int n=0;n<nodes;n++){RenderTrees::Node& node=records[n];memset(&node,0,sizeof(node));unsigned int sky;if(!Read(file,&sky,4)||sky>100000-totalSky)ExitProcess(4);totalSky+=sky;RenderTrees::SkyPortal* portals=(RenderTrees::SkyPortal*)memory.Allocate(sky*sizeof(RenderTrees::SkyPortal));if(!portals)ExitProcess(4);node.skyPortals=portals;node.skyPortalCount=sky;
   for(unsigned int p=0;p<sky;p++){unsigned int points;if(!Read(file,&points,4)||points<3||points>255||points>1000000-totalSkyVertices)ExitProcess(4);totalSkyVertices+=points;WorldText::Vec3* positions=(WorldText::Vec3*)memory.Allocate(points*12);if(!positions||!Read(file,positions,points*12))ExitProcess(4);portals[p].vertices=positions;portals[p].count=points;}
  }
 }
 if(totalNodes!=nodeCount||!Eof(file))ExitProcess(4);CloseHandle(file);RenderTrees::Layout scaffold={trees,treeHeader[1]};
 RenderMeshGroups::Region* regions=0;unsigned int regionCount=0;char regionPath[MAX_PATH];unsigned int regionLength=GetEnvironmentVariableA("FEAR_GLOBAL_RENDER_REGIONS",regionPath,MAX_PATH);if(regionLength>=MAX_PATH)ExitProcess(5);
 if(regionLength){file=CreateFileA(regionPath,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);unsigned int header[2];if(file==INVALID_HANDLE_VALUE||!Read(file,header,8)||header[0]!=0x31474d52||header[1]>4096)ExitProcess(5);regionCount=header[1];regions=(RenderMeshGroups::Region*)memory.Allocate(regionCount*sizeof(RenderMeshGroups::Region));if(!regions)ExitProcess(5);unsigned int refs=0;
  for(unsigned int r=0;r<regionCount;r++){unsigned int counts[2];if(!Read(file,counts,8)||counts[0]>4096||counts[1]>1000000-refs)ExitProcess(5);refs+=counts[1];char* name=(char*)memory.Allocate(counts[0]+1);unsigned int* ids=(unsigned int*)memory.Allocate(counts[1]*4);if(!name||!ids||!Read(file,name,counts[0])||!Read(file,ids,counts[1]*4))ExitProcess(5);name[counts[0]]=0;if(lstrlenA(name)!=(int)counts[0])ExitProcess(5);regions[r].name=name;regions[r].sectors=ids;regions[r].count=counts[1];}if(!Eof(file))ExitProcess(5);CloseHandle(file);
 }
 RenderVisibilityPortalGraph::Graph graph;char portalPath[MAX_PATH];unsigned int portalLength=GetEnvironmentVariableA("FEAR_GLOBAL_RENDER_PORTALS",portalPath,MAX_PATH);if(portalLength>=MAX_PATH)ExitProcess(6);
 if(portalLength){file=CreateFileA(portalPath,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);if(file==INVALID_HANDLE_VALUE)ExitProcess(6);DWORD bytes=GetFileSize(file,0);if(bytes<32||bytes>16*1024*1024)ExitProcess(6);unsigned char* data=(unsigned char*)memory.Allocate(bytes);if(!data||!Read(file,data,bytes)||!graph.Build(data,bytes))ExitProcess(6);CloseHandle(file);}
 char projectRoot[MAX_PATH];unsigned int rootLength=GetEnvironmentVariableA("FEAR_GLOBAL_RENDER_PROJECT_ROOT",projectRoot,MAX_PATH);if(rootLength>=MAX_PATH)ExitProcess(7);if(!rootLength)projectRoot[0]=0;
 RenderGlobalMeshes::Block block;if(!block.Build(sections,count,scaffold,mainTree,regions,regionCount,stem,h[3]!=0,portalLength?&graph:0,projectRoot))ExitProcess(7);
 // Diagnostic archive only. Semantic filenames are stored, never interpreted
 // as filesystem paths. No SDK or external target file is written here.
 file=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);unsigned int out[3]={0x31475257,block.size,block.meshCount};if(file==INVALID_HANDLE_VALUE||!Write(file,out,12)||!Write(file,block.data,block.size))ExitProcess(8);
 for(unsigned int m=0;m<block.meshCount;m++){const RenderGlobalMeshes::Mesh& mesh=block.meshes[m];unsigned int length=mesh.name?lstrlenA(mesh.name):0,record[3]={m,length,mesh.externalBytes};if(!Write(file,record,12)||!Write(file,mesh.name,length)||!Write(file,mesh.external,mesh.externalBytes))ExitProcess(8);}CloseHandle(file);ExitProcess(0);
}
