#include "WorldPacker.cpp"
#include "CompleteRender.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
class Input {
 PrefabScene::Arena memory;const unsigned char* data;unsigned int size,cursor;
public:
 bool Open(const char* env){char path[MAX_PATH];if(!GetEnvironmentVariableA(env,path,MAX_PATH))return false;HANDLE f=CreateFileA(path,GENERIC_READ,FILE_SHARE_READ,0,3,FILE_ATTRIBUTE_NORMAL,0);if(f==INVALID_HANDLE_VALUE)return false;size=GetFileSize(f,0);cursor=0;if(size<12||size>32*1024*1024){CloseHandle(f);return false;}unsigned char* bytes=(unsigned char*)memory.Allocate(size);DWORD done;bool ok=bytes&&ReadFile(f,bytes,size,&done,0)&&done==size;CloseHandle(f);data=bytes;return ok;}
 const unsigned char* Take(unsigned int bytes){if(bytes>size-cursor)return 0;const unsigned char* p=data+cursor;cursor+=bytes;return p;}
 bool Header(unsigned int* h,unsigned int n){const unsigned char* p=Take(n*4);if(!p)return false;for(unsigned int i=0;i<n*4;i++)((unsigned char*)h)[i]=p[i];return true;}
 bool End(){return cursor==size;}
};
extern "C" void HarnessEntry(){
 char topologyInput[MAX_PATH];if(GetEnvironmentVariableA("FEAR_RENDER_TREE_INPUT",topologyInput,MAX_PATH)){
  Input input;if(!input.Open("FEAR_RENDER_TREE_INPUT"))ExitProcess(30);unsigned int h[3];if(!input.Header(h,3)||h[0]!=0x31545252||!h[1]||h[1]>4096||!h[2]||h[2]>100000)ExitProcess(31);
  PrefabScene::Arena arena;RenderTrees::Tree* trees=(RenderTrees::Tree*)arena.Allocate(h[1]*sizeof(RenderTrees::Tree));if(!trees)ExitProcess(32);
  for(unsigned int t=0;t<h[1];t++){unsigned int count;if(!input.Header(&count,1)||!count||count>65536)ExitProcess(33);RenderTrees::Node* nodes=(RenderTrees::Node*)arena.Allocate(count*sizeof(RenderTrees::Node));if(!nodes)ExitProcess(32);trees[t].nodes=nodes;trees[t].count=count;
   for(unsigned int n=0;n<count;n++){unsigned int nh[2];if(!input.Header(nh,2)||nh[0]>100000||nh[1]>100000)ExitProcess(33);nodes[n].count=nh[0];nodes[n].branches=(const RenderTrees::Branch*)input.Take(nh[0]*36);if(nh[0]&&!nodes[n].branches)ExitProcess(33);
    nodes[n].skyPortalCount=nh[1];RenderTrees::SkyPortal* skyPortals=nh[1]?(RenderTrees::SkyPortal*)arena.Allocate(nh[1]*sizeof(RenderTrees::SkyPortal)):0;if(nh[1]&&!skyPortals)ExitProcess(32);nodes[n].skyPortals=skyPortals;
    for(unsigned int o=0;o<nh[1];o++){const unsigned char* size=input.Take(1);if(!size)ExitProcess(33);skyPortals[o].count=*size;skyPortals[o].vertices=(const WorldText::Vec3*)input.Take(*size*12);if(*size&&!skyPortals[o].vertices)ExitProcess(33);}
   }
  }
  if(!input.End())ExitProcess(33);RenderTrees::Layout layout={trees,h[1]};RenderTrees::Block block;if(!block.Build(layout,h[2]))ExitProcess(34);
  char output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_RENDER_COMPLETE",output,MAX_PATH))ExitProcess(35);HANDLE f=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);DWORD done;if(f==INVALID_HANDLE_VALUE||!WriteFile(f,block.data,block.size,&done,0)||done!=block.size)ExitProcess(36);CloseHandle(f);ExitProcess(0);
 }
 char generic[4]={};if(GetEnvironmentVariableA("FEAR_RENDER_GENERIC_CONTROL",generic,4)&&generic[0]=='1'){
  const char* names[]={"Tests\\Stone.Mat00","Tests\\Metal.Mat00","Tests\\Glass.Mat00","Tests\\Water.Mat00","Tests\\Wood.Mat00","Tests\\Paint.Mat00"};
  const void* decls[]={CompleteRender::declarations0,CompleteRender::declarations1,CompleteRender::declarations2,CompleteRender::declarations0};
  unsigned int sizes[]={sizeof(CompleteRender::declarations0),sizeof(CompleteRender::declarations1),sizeof(CompleteRender::declarations2),sizeof(CompleteRender::declarations0)};
  const CompleteRender::Profile profile={names,6,decls,sizes,4};
  unsigned char vertices[3][168]={};unsigned short indices[]={0,1,2};CompleteRender::Surface surfaces[3]={};unsigned int strides[]={24,56,32};
  for(unsigned int i=0;i<3;i++){for(unsigned int j=0;j<3;j++){WorldText::Vec3 pos={(float)(i*10+j),(float)(j==1),(float)i};memcpy(vertices[i]+j*strides[i],&pos,12);}surfaces[i].vertices=vertices[i];surfaces[i].indices=indices;surfaces[i].count=3;surfaces[i].triangles=1;surfaces[i].stride=strides[i];surfaces[i].definition=i==2?3:i;surfaces[i].material=i==2?5:i;surfaces[i].tree=i==1?2:0;surfaces[i].branchType=i;}
  CompleteRender::Block block;surfaces[0].material=6;if(block.Build(surfaces,3,3,profile)||block.data)ExitProcess(20);surfaces[0].material=0;
  surfaces[0].definition=4;if(block.Build(surfaces,3,3,profile)||block.data)ExitProcess(21);surfaces[0].definition=0;
  RenderTrees::Branch branches[3];for(unsigned int i=0;i<3;i++){branches[i].low.x=(float)(i*10);branches[i].low.y=0;branches[i].low.z=(float)i;branches[i].high.x=(float)(i*10+2);branches[i].high.y=1;branches[i].high.z=(float)i;branches[i].type=i;branches[i].meshIndex=0;branches[i].surface=i;}
  RenderTrees::Node nodes[]={{branches+2,1},{branches,1},{0,0},{branches+1,1}};RenderTrees::Tree trees[]={{nodes,2},{nodes+2,1},{nodes+3,1}};RenderTrees::Layout layout={trees,3};char mode[4]={};bool multi=GetEnvironmentVariableA("FEAR_RENDER_MULTINODE_CONTROL",mode,4)&&mode[0]=='1';
  WorldText::Vec3 quad[]={{0,0,0},{2,0,0},{2,2,0},{0,2,0}},triangle[]={{0,0,1},{1,0,1},{0,1,1}};RenderTrees::SkyPortal skyPortals[]={{quad,4},{triangle,3}};
  char skyPortalMode[4]={};bool occlusion=GetEnvironmentVariableA("FEAR_RENDER_SKYPORTAL_CONTROL",skyPortalMode,4)&&skyPortalMode[0]=='1';
  if(occlusion){multi=true;nodes[0].skyPortals=skyPortals;nodes[0].skyPortalCount=1;nodes[2].skyPortals=skyPortals+1;nodes[2].skyPortalCount=1;
   skyPortals[0].count=256;if(block.Build(surfaces,3,3,profile,&layout)||block.data)ExitProcess(26);skyPortals[0].count=4;
   skyPortals[1].count=2;if(block.Build(surfaces,3,3,profile,&layout)||block.data)ExitProcess(27);skyPortals[1].count=3;
  }
  if(multi){branches[2].meshIndex=1;if(block.Build(surfaces,3,3,profile,&layout)||block.data)ExitProcess(28);branches[2].meshIndex=0;branches[2].surface=0;if(block.Build(surfaces,3,3,profile,&layout)||block.data)ExitProcess(25);branches[2].surface=2;}
  if(!block.Build(surfaces,3,3,profile,multi?&layout:0))ExitProcess(22);char target[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_RENDER_COMPLETE",target,MAX_PATH))ExitProcess(23);
  HANDLE f=CreateFileA(target,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);DWORD done;if(f==INVALID_HANDLE_VALUE||!WriteFile(f,block.data,block.size,&done,0)||done!=block.size)ExitProcess(24);CloseHandle(f);ExitProcess(0);
 }
 Input visible,shadow;if(!visible.Open("FEAR_RENDER_EXPANDED")||!shadow.Open("FEAR_SHADOW_EXPANDED"))ExitProcess(1);unsigned int vh[3],sh[3];if(!visible.Header(vh,3)||!shadow.Header(sh,3)||vh[0]!=0x31534752||sh[0]!=0x31534853||vh[1]!=4||sh[1]>1024||vh[2]!=sh[2])ExitProcess(2);
 PrefabScene::Arena arena;CompleteRender::Surface* surfaces=(CompleteRender::Surface*)arena.Allocate((sh[1]+4)*sizeof(CompleteRender::Surface));if(!surfaces)ExitProcess(3);unsigned int count=0;
 for(unsigned int i=0;i<sh[1];i++){unsigned int h[4];if(!shadow.Header(h,4)||h[1]>2||!h[2]||h[2]>65535||!h[3]||h[3]>1000000)ExitProcess(4);CompleteRender::Surface& s=surfaces[count++];s.tree=h[0];s.branchType=h[1]+1;s.count=h[2];s.triangles=h[3];s.stride=24;s.definition=s.material=0;s.vertices=shadow.Take(s.count*24);s.indices=(const unsigned short*)shadow.Take(s.triangles*6);if(!s.vertices||!s.indices)ExitProcess(4);}if(!shadow.End())ExitProcess(4);
 CompleteRender::Surface lit[4]={};bool seen[4]={};
 for(unsigned int i=0;i<4;i++){unsigned int h[4];if(!visible.Header(h,4)||!h[1]||h[1]>65535||!h[2]||h[2]>1000000||!h[3]||h[3]>4096)ExitProcess(5);const unsigned char* name=visible.Take(h[3]);if(!name)ExitProcess(5);unsigned int mat=1;for(;mat<5;mat++){if((unsigned int)lstrlenA(CompleteRender::materials[mat])!=h[3])continue;unsigned int k=0;for(;k<h[3];k++)if(name[k]!=CompleteRender::materials[mat][k])break;if(k==h[3])break;}if(mat==5||seen[mat-1])ExitProcess(5);seen[mat-1]=true;CompleteRender::Surface& s=lit[mat-1];s.tree=h[0];s.count=h[1];s.triangles=h[2];s.material=mat;s.stride=mat==1?24:56;s.definition=mat==1?1:2;const unsigned char* raw=visible.Take(s.count*84);s.indices=(const unsigned short*)visible.Take(s.triangles*6);unsigned char* packed=(unsigned char*)arena.Allocate(s.count*s.stride);if(!raw||!s.indices||!packed)ExitProcess(5);s.vertices=packed;
  for(unsigned int v=0;v<s.count;v++){const unsigned char* src=raw+v*84;unsigned char* dst=packed+v*s.stride;for(unsigned int k=0;k<12;k++)dst[k]=src[k];if(mat==1){for(unsigned int k=0;k<4;k++)dst[12+k]=src[44+k];for(unsigned int k=0;k<8;k++)dst[16+k]=src[12+k];}else{for(unsigned int k=0;k<12;k++)dst[12+k]=src[48+k];for(unsigned int k=0;k<8;k++)dst[24+k]=src[12+k];for(unsigned int k=0;k<24;k++)dst[32+k]=src[60+k];}}
 }
 if(!visible.End())ExitProcess(5);for(unsigned int i=0;i<4;i++)surfaces[count++]=lit[i];CompleteRender::Block block;if(!block.Build(surfaces,count,vh[2]))ExitProcess(6);char output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_RENDER_COMPLETE",output,MAX_PATH))ExitProcess(7);HANDLE f=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);DWORD done;if(f==INVALID_HANDLE_VALUE||!WriteFile(f,block.data,block.size,&done,0)||done!=block.size)ExitProcess(8);CloseHandle(f);ExitProcess(0);
}
