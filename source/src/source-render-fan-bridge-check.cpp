#include "WorldPacker.cpp"
#include "SourceRenderFanBridge.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
static bool Read(HANDLE f,void* p,unsigned int bytes){DWORD done;return ReadFile(f,p,bytes,&done,0)&&done==bytes;}
static bool Put(HANDLE f,const void* p,unsigned int bytes){DWORD done;return WriteFile(f,p,bytes,&done,0)&&done==bytes;}
static bool EqualBytes(const void* a,const void* b,unsigned int size){const unsigned char* x=(const unsigned char*)a;const unsigned char* y=(const unsigned char*)b;for(unsigned int i=0;i<size;i++)if(x[i]!=y[i])return false;return true;}
static unsigned int guards=0;
static float Bits(unsigned int bits){float value;memcpy(&value,&bits,4);return value;}
static void Assert(bool good,unsigned int code){if(!good)ExitProcess(code);}
static void Rejected(bool result,SourceRenderFanBridge::Builder& b){Assert(!result&&b.error&&!b.triangles&&!b.metadata&&!b.count&&!b.contourCount&&!b.candidates&&!b.filtered&&!b.inputEndpoints&&!b.outputEndpoints,90);guards++;}
template<class C> static void Describe(C& c,unsigned int ordinal){
 memset(&c,0,sizeof(c));c.plane.z=1;c.sourceNode=1000+ordinal;c.sourcePolygon=2000+ordinal;c.fragment=3000+ordinal;c.brushGroup=4000+ordinal;c.tangentMethod=ordinal%3;c.surfaceFlags=0x79;
 lstrcpyA(c.material,"materials\\bridge.ltm");for(unsigned int b=0;b<4;b++){c.bases[b].origin.x=(float)b;c.bases[b].u.x=1;c.bases[b].v.y=1;}
}
static void GuardTests(){
 SourceExpandedRenderContours::Contour c;Describe(c,0);RenderClipVertex::Vertex vertices[3]={};vertices[1].point.x=10;vertices[2].point.y=10;vertices[0].attributes[6]=vertices[1].attributes[6]=vertices[2].attributes[6]=1;c.vertices=vertices;c.count=3;c.owner=5;
 SourceRenderFanBridge::Builder b;Assert(b.BuildMain(&c,1)&&b.count==1,80);b.Clear();Assert(!b.error&&!b.count&&!b.metadata&&!b.triangles,81);guards++;
 Assert(b.BuildMain(0,0)&&!b.error&&!b.count&&!b.contourCount,82);guards++;
 Rejected(b.BuildMain(0,1),b);Rejected(b.BuildMain(&c,SourceRenderFanBridge::MaximumContours+1),b);
 const RenderClipVertex::Vertex* saved=c.vertices;c.vertices=0;Rejected(b.BuildMain(&c,1),b);c.vertices=saved;
 c.count=65536;Rejected(b.BuildMain(&c,1),b);c.count=3;
 PrefabScene::Arena guardMemory;SourceExpandedRenderContours::Contour* many=(SourceExpandedRenderContours::Contour*)guardMemory.Allocate(17*sizeof(SourceExpandedRenderContours::Contour));Assert(many!=0,79);for(unsigned int i=0;i<17;i++){many[i]=c;many[i].count=65535;}Rejected(b.BuildMain(many,17),b);Rejected(b.BuildMain(many,6),b);
 vertices[0].point.x=Bits(0x7fc12345);Rejected(b.BuildMain(&c,1,true),b);vertices[0].point.x=0;
 for(unsigned int a=0;a<7;a++){vertices[0].attributes[a]=Bits(0x7fc12345);Rejected(b.BuildMain(&c,1,true),b);vertices[0].attributes[a]=(a==6)?1.0f:0.0f;}
 c.plane.z=0;Rejected(b.BuildMain(&c,1),b);c.plane.z=Bits(0x7fc12345);Rejected(b.BuildMain(&c,1,true),b);c.plane.z=1;
 c.tangentMethod=3;Rejected(b.BuildMain(&c,1),b);c.tangentMethod=0;
 c.material[0]='\x80';Rejected(b.BuildMain(&c,1),b);memset(c.material,'x',260);Rejected(b.BuildMain(&c,1),b);lstrcpyA(c.material,"materials\\bridge.ltm");
 c.bases[0].origin.x=Bits(0x7fc12345);Rejected(b.BuildMain(&c,1),b);Assert(b.BuildMain(&c,1,true)&&EqualBytes(&b.metadata[0].bases[0].origin.x,&c.bases[0].origin.x,4),83);guards++;
 c.bases[0].origin.x=Bits(0x7f800000);Rejected(b.BuildMain(&c,1,true),b);c.bases[0].origin.x=Bits(0x7f812345);Rejected(b.BuildMain(&c,1,true),b);c.bases[0].origin.x=0;
 c.bases[0].u.x=Bits(0x7fc12345);Rejected(b.BuildMain(&c,1,true),b);c.bases[0].u.x=1;
 Assert(b.BuildMain(&c,1)&&b.count==1,84);vertices[1].point.x=20;c.material[0]='x';Assert(b.triangles[0].geometry.vertices[1].point.x==10&&b.metadata[0].material[0]=='m',85);guards++;
 vertices[1].point.x=10;c.material[0]='m';
 SourceWorldModelRenderContours::Contour owner;Describe(owner,0);owner.vertices=vertices;owner.count=3;owner.group=7;owner.localNode=4;
 Assert(b.BuildOwners(&owner,1)&&b.count==1&&b.metadata[0].group==7&&b.metadata[0].localNode==4&&b.metadata[0].owner==4&&b.triangles[0].geometry.owner==4,86);guards++;
 owner.bases[0].origin.x=Bits(0x7fc12345);Rejected(b.BuildOwners(&owner,1),b);Assert(b.BuildOwners(&owner,1,true),87);guards++;
 owner.vertices=0;Rejected(b.BuildOwners(&owner,1,true),b);
}
extern "C" void HarnessEntry(){
 GuardTests();char input[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_RENDER_FAN_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_RENDER_FAN_OUTPUT",output,MAX_PATH))ExitProcess(1);
 HANDLE f=CreateFileA(input,0x80000000,FILE_SHARE_READ,0,3,FILE_ATTRIBUTE_NORMAL,0);if(f==INVALID_HANDLE_VALUE)ExitProcess(2);unsigned int header[2];if(!Read(f,header,8)||header[0]!=0x31434652||header[1]>SourceRenderFanBridge::MaximumContours)ExitProcess(3);
 PrefabScene::Arena memory;unsigned int allocated=header[1]+1;
 SourceExpandedRenderContours::Contour* main=(SourceExpandedRenderContours::Contour*)memory.Allocate(allocated*sizeof(SourceExpandedRenderContours::Contour));
 SourceWorldModelRenderContours::Contour* owners=(SourceWorldModelRenderContours::Contour*)memory.Allocate(allocated*sizeof(SourceWorldModelRenderContours::Contour));
 RenderAttributedFan::Contour* legacy=(RenderAttributedFan::Contour*)memory.Allocate(allocated*sizeof(RenderAttributedFan::Contour));if(!main||!owners||!legacy)ExitProcess(4);unsigned int inputVertices=0;
 for(unsigned int i=0;i<header[1];i++){
  unsigned int count=0,owner=0;if(!Read(f,&count,4)||!Read(f,&owner,4)||count>65535||count>RenderFrameLimits::MaximumEndpoints-inputVertices)ExitProcess(3);inputVertices+=count;
  RenderClipVertex::Vertex* points=(RenderClipVertex::Vertex*)memory.Allocate((count+1)*sizeof(RenderClipVertex::Vertex));if(!points||!Read(f,points,count*sizeof(RenderClipVertex::Vertex)))ExitProcess(4);
  Describe(main[i],i);main[i].vertices=points;main[i].count=count;main[i].owner=owner;
  Describe(owners[i],i);owners[i].vertices=points;owners[i].count=count;owners[i].localNode=owner;owners[i].group=7+i;
  legacy[i].vertices=points;legacy[i].count=count;legacy[i].owner=owner;
 }
 unsigned char trailing;DWORD done;if(!ReadFile(f,&trailing,1,&done,0)||done)ExitProcess(3);CloseHandle(f);
 RenderAttributedFan::Builder prior;SourceRenderFanBridge::Builder bridge,owned;
 if(!prior.Build(legacy,header[1])||!bridge.BuildMain(main,header[1])||!owned.BuildOwners(owners,header[1]))ExitProcess(5);
 Assert(prior.count==bridge.count&&prior.candidates==bridge.candidates&&prior.filtered==bridge.filtered&&owned.count==bridge.count&&bridge.inputEndpoints==inputVertices&&bridge.outputEndpoints==bridge.count*3,88);
 for(unsigned int i=0;i<header[1];i++){SourceRenderFanBridge::Metadata comparable=bridge.metadata[i];comparable.group=7+i;Assert(EqualBytes(&comparable,&owned.metadata[i],sizeof(comparable)),92);}
 for(unsigned int i=0;i<bridge.count;i++){
  Assert(EqualBytes(&bridge.triangles[i].geometry,&prior.triangles[i],sizeof(RenderAttributedFan::Triangle))&&EqualBytes(&owned.triangles[i].geometry,&prior.triangles[i],sizeof(RenderAttributedFan::Triangle)),89);
  unsigned int contour=bridge.triangles[i].contourOrdinal;Assert(contour<header[1]&&owned.triangles[i].contourOrdinal==contour&&bridge.metadata[contour].owner==legacy[contour].owner&&owned.metadata[contour].group==7+contour&&owned.metadata[contour].owner==legacy[contour].owner,91);
 }
 f=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(f==INVALID_HANDLE_VALUE)ExitProcess(6);
 unsigned int result[8]={0x31424653,bridge.count,bridge.candidates,bridge.filtered,bridge.contourCount,bridge.inputEndpoints,bridge.outputEndpoints,guards};
 if(!Put(f,result,sizeof(result))||(bridge.count&&!Put(f,bridge.triangles,bridge.count*sizeof(SourceRenderFanBridge::LinkedTriangle)))||(bridge.contourCount&&!Put(f,bridge.metadata,bridge.contourCount*sizeof(SourceRenderFanBridge::Metadata))))ExitProcess(6);CloseHandle(f);ExitProcess(0);
}
