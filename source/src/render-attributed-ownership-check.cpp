#include "WorldPacker.cpp"
#include "RenderAttributedOwnership.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
#ifdef FEAR_OWNERSHIP_ORACLE
extern "C" __declspec(dllimport) HMODULE WINAPI LoadLibraryA(const char*);
#pragma comment(linker,"/alternatename:__imp__LoadLibraryA@4=__imp__LoadLibraryA")
struct RawVertex {unsigned int flags;WorldText::Vec3 point;float attributes[7];};
struct Vector {unsigned int unused;unsigned char* first;unsigned char* end;unsigned char* capacity;};
struct RawFace {unsigned int unused[4];ExtrusionVolume::Plane plane;};
static unsigned char rawSectors[16][112],rawPolygon[100+128*44],surface[176];static RawFace rawFaces[16][6];static RawFace* facePointers[16][6];static unsigned char* sectorPointers[16];static float planeHeader[6];
#endif
static bool Read(HANDLE f,void* p,unsigned int n){DWORD done;return ReadFile(f,p,n,&done,0)&&done==n;}
static bool Put(HANDLE f,const void* p,unsigned int n){DWORD done;return WriteFile(f,p,n,&done,0)&&done==n;}
static StreamingMetadata::External controlSectors[16];static ConvexIntersection::Face faces[16][6];static RenderClipVertex::Vertex points[128];
#ifndef FEAR_OWNERSHIP_ORACLE
static bool SameBytes(const void* a,const void* b,unsigned int n){const unsigned char* x=(const unsigned char*)a;const unsigned char* y=(const unsigned char*)b;for(unsigned int i=0;i<n;i++)if(x[i]!=y[i])return false;return true;}
static bool CheckPipeline(const RenderAttributedOwnership::Builder& ownership,unsigned int sectorCount){
 PrefabScene::Arena memory;
 RenderFanTriangulation::Contour* input=(RenderFanTriangulation::Contour*)memory.Allocate((ownership.count?ownership.count:1)*sizeof(RenderFanTriangulation::Contour));if(!input)return false;
 unsigned int valid=0;for(unsigned int i=0;i<ownership.count;i++)if(ownership.contours[i].count>=3){const RenderAttributedOwnership::Contour& c=ownership.contours[i];WorldText::Vec3* positions=(WorldText::Vec3*)memory.Allocate(c.count*12);if(!positions)return false;for(unsigned int j=0;j<c.count;j++)positions[j]=c.vertices[j].point;input[valid].vertices=positions;input[valid].count=c.count;input[valid++].owner=c.owner;}
 RenderFanTriangulation::Builder fan;if(!fan.Build(input,valid))return false;
 typedef RenderTriangleRouting::Builder<RenderFanTriangulation::Triangle> Routing;
 Routing::Input* records=(Routing::Input*)memory.Allocate((fan.count?fan.count:1)*sizeof(Routing::Input));if(!records)return false;
 unsigned int assigned=0,skipped=0;
 for(unsigned int i=0;i<fan.count;i++){records[i].triangle=&fan.triangles[i];records[i].owner=fan.triangles[i].owner;if(records[i].owner==RenderTriangleRouting::Unassigned)skipped++;else assigned++;}
 Routing routing;if(!routing.Build(records,fan.count,sectorCount)||routing.assigned!=assigned||routing.skipped!=skipped)return false;
 for(unsigned int owner=0;owner<sectorCount;owner++){unsigned int at=0;for(unsigned int i=0;i<fan.count;i++)if(fan.triangles[i].owner==owner){if(at>=routing.nodes[owner].count||!SameBytes(&routing.nodes[owner].triangles[at++],&fan.triangles[i],sizeof(RenderFanTriangulation::Triangle)))return false;}if(at!=routing.nodes[owner].count)return false;}
 return true;
}
#endif
extern "C" void HarnessEntry(){
 char input[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_OWNERSHIP_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_OWNERSHIP_OUTPUT",output,MAX_PATH))ExitProcess(1);
#ifdef FEAR_OWNERSHIP_ORACLE
 LoadLibraryA("E:\\fearsdk\\Dev\\Tools\\msvcr71.dll");LoadLibraryA("E:\\fearsdk\\Dev\\Tools\\msvcp71.dll");LoadLibraryA("E:\\fearsdk\\Dev\\Tools\\MFC71.dll");unsigned char* dll=(unsigned char*)LoadLibraryA("E:\\fearsdk\\Dev\\Tools\\Packers\\WorldPacker.dll");if(!dll)ExitProcess(2);
 typedef bool (__thiscall* Assign)(void*,void*,void*,Vector*,Vector*,Vector*);Assign assign=(Assign)(dll+0x475f0);
#endif
 HANDLE in=CreateFileA(input,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);if(in==INVALID_HANDLE_VALUE)ExitProcess(2);unsigned int samples=0;if(!Read(in,&samples,4)||samples>1000)ExitProcess(3);HANDLE out=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(out==INVALID_HANDLE_VALUE)ExitProcess(6);
 for(unsigned int sample=0;sample<samples;sample++){
  unsigned int counts[2]={};WorldText::Vec3 normal={};if(!Read(in,counts,8)||!counts[0]||counts[0]>16||counts[1]<3||counts[1]>128||!Read(in,&normal,12))ExitProcess(3);
  for(unsigned int s=0;s<counts[0];s++){
   StreamingMetadata::External& external=controlSectors[s];ConvexIntersection::Volume& v=external.volume;if(!Read(in,&external.index,4)||!Read(in,&v.low,12)||!Read(in,&v.high,12))ExitProcess(3);v.faces=faces[s];v.count=6;
   for(unsigned int axis=0;axis<3;axis++)for(unsigned int side=0;side<2;side++){ConvexIntersection::Face& f=faces[s][axis*2+side];f.points=0;f.count=0;f.plane.x=f.plane.y=f.plane.z=0;(&f.plane.x)[axis]=side?-1.f:1.f;f.plane.d=side?-(&v.high.x)[axis]:(&v.low.x)[axis];}
#ifdef FEAR_OWNERSHIP_ORACLE
   memset(rawSectors[s],0,112);*(RawFace***)(rawSectors[s])=facePointers[s];*(unsigned int*)(rawSectors[s]+4)=6;*(unsigned int*)(rawSectors[s]+8)=6;memcpy(rawSectors[s]+0x14,&v.low,12);memcpy(rawSectors[s]+0x20,&v.high,12);*(unsigned int*)(rawSectors[s]+0x6c)=external.index;sectorPointers[s]=rawSectors[s];for(unsigned int j=0;j<6;j++){rawFaces[s][j].plane=faces[s][j].plane;facePointers[s][j]=&rawFaces[s][j];}
#endif
  }
  if(!Read(in,points,counts[1]*44))ExitProcess(3);
#ifdef FEAR_OWNERSHIP_ORACLE
  memset(rawPolygon,0,sizeof(rawPolygon));memset(surface,0,sizeof(surface));*(unsigned short*)(rawPolygon+0x60)=(unsigned short)counts[1];*(unsigned char**)(rawPolygon+0x38)=surface;*(float**)(surface+0xa4)=planeHeader;planeHeader[2]=normal.x;planeHeader[3]=normal.y;planeHeader[4]=normal.z;
  for(unsigned int j=0;j<counts[1];j++){RawVertex* p=(RawVertex*)(rawPolygon+0x64+j*44);memcpy(p,points+j,44);}unsigned int manager[4]={0,(unsigned int)sectorPointers,counts[0],counts[0]};Vector vertices={},sizes={},owners={};if(!assign(0,rawPolygon,manager,&vertices,&sizes,&owners))ExitProcess(5);
  unsigned int n=(unsigned int)(sizes.end-sizes.first)/4;if(!Put(out,&n,4))ExitProcess(7);unsigned int start=0;for(unsigned int j=0;j<n;j++){unsigned char* owner=((unsigned char**)owners.first)[j];unsigned int id=owner?*(unsigned int*)(owner+0x6c):0xffffffff;unsigned int nv=((unsigned int*)sizes.first)[j];if(!Put(out,&id,4)||!Put(out,&nv,4))ExitProcess(7);for(unsigned int k=0;k<nv;k++)if(!Put(out,vertices.first+(start+k)*44,44))ExitProcess(7);start+=nv;}
#else
  RenderAttributedOwnership::Polygon polygon={points,counts[1],normal};RenderAttributedOwnership::Builder builder;if(!builder.Build(&polygon,1,controlSectors,counts[0])||!CheckPipeline(builder,counts[0]))ExitProcess(5);if(!Put(out,&builder.count,4))ExitProcess(7);for(unsigned int j=0;j<builder.count;j++){const RenderAttributedOwnership::Contour& c=builder.contours[j];if(!Put(out,&c.owner,4)||!Put(out,&c.count,4)||!Put(out,c.vertices,c.count*44))ExitProcess(7);}
  unsigned int saved=controlSectors[0].index;controlSectors[0].index=counts[0];if(builder.Build(&polygon,1,controlSectors,counts[0])||builder.count||builder.contours||!builder.error)ExitProcess(8);controlSectors[0].index=saved;
  if(!builder.Build(&polygon,1,controlSectors,counts[0])||!CheckPipeline(builder,counts[0]))ExitProcess(9);
#endif
 }CloseHandle(in);CloseHandle(out);ExitProcess(0);
}


