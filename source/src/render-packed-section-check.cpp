#include "WorldPacker.cpp"
#include "RenderPackedSection.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
static bool Read(HANDLE f,void* p,unsigned int n){DWORD done;return ReadFile(f,p,n,&done,0)&&done==n;}
static bool ReadScaffold(const char* path,unsigned int expectedNodes,PrefabScene::Arena& memory,RenderTrees::Layout& layout){
 HANDLE file=CreateFileA(path,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);unsigned int header[2];if(file==INVALID_HANDLE_VALUE||!Read(file,header,8)||header[0]!=0x31544653||!header[1]||header[1]>4096)return false;
 RenderTrees::Tree* trees=(RenderTrees::Tree*)memory.Allocate(header[1]*sizeof(RenderTrees::Tree));if(!trees)return false;unsigned int nodes=0,portals=0,vertices=0;
 for(unsigned int t=0;t<header[1];t++){
  unsigned int count;if(!Read(file,&count,4)||count>expectedNodes-nodes)return false;nodes+=count;RenderTrees::Node* records=(RenderTrees::Node*)memory.Allocate(count*sizeof(RenderTrees::Node));if(!records)return false;trees[t].nodes=records;trees[t].count=count;
  for(unsigned int n=0;n<count;n++){
   RenderTrees::Node& node=records[n];node.branches=0;node.count=0;node.skyPortals=0;node.skyPortalCount=0;
   unsigned int sky;if(!Read(file,&sky,4)||sky>100000-portals)return false;portals+=sky;if(!sky)continue;
   RenderTrees::SkyPortal* recordsSky=(RenderTrees::SkyPortal*)memory.Allocate(sky*sizeof(RenderTrees::SkyPortal));if(!recordsSky)return false;node.skyPortals=recordsSky;node.skyPortalCount=sky;
   for(unsigned int p=0;p<sky;p++){unsigned int points;if(!Read(file,&points,4)||points<3||points>255||points>1000000-vertices)return false;vertices+=points;WorldText::Vec3* positions=(WorldText::Vec3*)memory.Allocate(points*12);if(!positions||!Read(file,positions,points*12))return false;recordsSky[p].vertices=positions;recordsSky[p].count=points;}
  }
 }
 unsigned char extra;DWORD done=0;bool complete=nodes==expectedNodes&&ReadFile(file,&extra,1,&done,0)&&!done;CloseHandle(file);if(!complete)return false;layout.trees=trees;layout.count=header[1];return true;
}
extern "C" void HarnessEntry(){
 char input[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_PACKED_SECTION_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_PACKED_SECTION_OUTPUT",output,MAX_PATH))ExitProcess(1);
 HANDLE file=CreateFileA(input,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);unsigned int h[4];
 if(file==INVALID_HANDLE_VALUE||!Read(file,h,16)||h[0]!=0x31525053||h[1]>65536||h[2]>RenderPackedRanges::MaximumSections||h[3]>1)ExitProcess(2);
 PrefabScene::Arena memory;RenderPackedSection::Input* sections=(RenderPackedSection::Input*)memory.Allocate(h[2]*sizeof(RenderPackedSection::Input));if(!sections)ExitProcess(3);
 unsigned int totalVertices=0,totalTriangles=0;
 for(unsigned int i=0;i<h[2];i++){
  unsigned int s[10];if(!Read(file,s,40)||s[0]>=h[1]||s[1]>3||!s[2]||s[2]>4096||s[3]>=RenderPackedRanges::MaximumSections||s[5]<2||s[5]>64||s[6]>65535||s[7]>4000000||s[8]>100000||s[9]||!RenderPackedLimits::Fits(s[6],totalVertices,RenderPackedLimits::MaximumVertices)||!RenderPackedLimits::Fits(s[8],totalTriangles,RenderPackedLimits::MaximumTriangles))ExitProcess(3);
  totalVertices+=s[6];totalTriangles+=s[8];RenderPackedSection::Input& item=sections[i];item.node=s[0];item.category=s[1];RenderPackedRanges::Input& r=item.range;
  char* name=(char*)memory.Allocate(s[2]+1);RenderVertexPacking::Element* declaration=(RenderVertexPacking::Element*)memory.Allocate(s[5]*8);RenderAttributes::Vertex* vertices=(RenderAttributes::Vertex*)memory.Allocate(s[6]*84);unsigned int* indices=(unsigned int*)memory.Allocate(s[8]*12);
  if(!name||!declaration||!vertices||!indices||!Read(file,name,s[2])||!Read(file,declaration,s[5]*8)||!Read(file,vertices,s[6]*84)||!Read(file,indices,s[8]*12))ExitProcess(3);name[s[2]]=0;
  r.material=name;r.shaderKey=s[3];r.orderKey=s[4];r.declaration=declaration;r.elements=s[5];r.vertices=vertices;r.vertexCount=s[6];r.workingVertexStart=s[7];r.indices=indices;r.triangles=s[8];
  for(unsigned int j=0;j<i;j++)if(sections[j].range.shaderKey==r.shaderKey&&!RenderPackedRanges::SameDeclaration(sections[j].range.declaration,sections[j].range.elements,r.declaration,r.elements))ExitProcess(4);
 }
 unsigned char extra;DWORD done;if(!ReadFile(file,&extra,1,&done,0)||done)ExitProcess(3);CloseHandle(file);
 RenderPackedSection::Block block;char treeInput[MAX_PATH];unsigned int treeLength=GetEnvironmentVariableA("FEAR_PACKED_SECTION_TREE_INPUT",treeInput,MAX_PATH);
 if(treeLength>=MAX_PATH)ExitProcess(4);
 char portalInput[MAX_PATH];unsigned int portalLength=GetEnvironmentVariableA("FEAR_PACKED_SECTION_PORTAL_INPUT",portalInput,MAX_PATH);if(portalLength>=MAX_PATH)ExitProcess(4);
 RenderVisibilityPortalGraph::Graph graph;RenderGlobalSectionOrder::MainPortalGraph mainGraph={&graph,0};
 if(portalLength){
  HANDLE source=CreateFileA(portalInput,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);if(source==INVALID_HANDLE_VALUE)ExitProcess(4);DWORD bytes=GetFileSize(source,0);if(bytes<32||bytes>16*1024*1024)ExitProcess(4);
  unsigned char* data=(unsigned char*)memory.Allocate(bytes);if(!data||!Read(source,data,bytes)||!graph.Build(data,bytes))ExitProcess(4);CloseHandle(source);
  char mainTree[12];unsigned int length=GetEnvironmentVariableA("FEAR_PACKED_SECTION_MAIN_TREE",mainTree,12);if(length>=12)ExitProcess(4);if(length){for(unsigned int i=0;i<length;i++){if(mainTree[i]<'0'||mainTree[i]>'9'||mainGraph.tree>409)return ExitProcess(4);mainGraph.tree=mainGraph.tree*10+(unsigned int)(mainTree[i]-'0');}}
 }
 if(treeLength){RenderTrees::Layout scaffold={};if(!ReadScaffold(treeInput,h[1],memory,scaffold)||!block.Build(sections,h[2],scaffold,h[3]!=0,portalLength?&mainGraph:0))ExitProcess(4);}
 else if(portalLength){RenderTrees::Node* nodes=(RenderTrees::Node*)memory.Allocate(h[1]*sizeof(RenderTrees::Node));if(!nodes)ExitProcess(4);memset(nodes,0,h[1]*sizeof(RenderTrees::Node));RenderTrees::Tree tree={nodes,h[1]};RenderTrees::Layout scaffold={&tree,1};if(!block.Build(sections,h[2],scaffold,h[3]!=0,&mainGraph))ExitProcess(4);}
 else if(!block.Build(sections,h[2],h[1],h[3]!=0))ExitProcess(4);
 HANDLE out=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(out==INVALID_HANDLE_VALUE||!WriteFile(out,block.data,block.size,&done,0)||done!=block.size)ExitProcess(5);CloseHandle(out);ExitProcess(0);
}
