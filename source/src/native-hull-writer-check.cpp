#include "WorldPacker.cpp"
#include "SourcePhysicsOwnerVertices.h"
#include "NativeHullConvex.h"
#include "NativePhysicsHullRecord.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
namespace HC=NativeHullConvex;
namespace HR=NativePhysicsHullRecord;
static_assert(sizeof(HC::Vec3)==12&&sizeof(HC::Vec4)==16&&sizeof(HR::Vec4)==16&&sizeof(HC::Triangle)==12&&sizeof(HR::Triangle)==12,"Hull boundary layouts");
static bool Write(HANDLE f,const void* p,unsigned int n){DWORD done=0;return !n||(WriteFile(f,p,n,&done,0)&&done==n);}
static bool Read(HANDLE f,void* p,unsigned int n){DWORD done=0;return !n||(ReadFile(f,p,n,&done,0)&&done==n);}
struct OwnedGeometry {HR::Vec4* points;unsigned int pointCount;HR::Triangle* triangles;unsigned int triangleCount;HR::Vec4* planes;unsigned int planeCount;};
static bool CopyGeometry(PrefabScene::Arena& memory,const HC::Result& graph,OwnedGeometry& out){
 out.pointCount=(unsigned int)graph.geometry.vertices.size;out.triangleCount=(unsigned int)graph.geometry.triangles.size;out.planeCount=(unsigned int)graph.planes.size;
 out.points=(HR::Vec4*)memory.Allocate((out.pointCount+1)*16);out.triangles=(HR::Triangle*)memory.Allocate((out.triangleCount+1)*12);out.planes=(HR::Vec4*)memory.Allocate((out.planeCount+1)*16);if(!out.points||!out.triangles||!out.planes)return false;
 // Graph vertex w is scratch padding. Only xyz is part of its input/output
 // contract; explicitly initialize w in the owned integration geometry.
 for(unsigned int i=0;i<out.pointCount;i++){const HC::Vec4& p=graph.geometry.vertices.data[i];out.points[i].x=p.x;out.points[i].y=p.y;out.points[i].z=p.z;out.points[i].w=0;}
 for(unsigned int i=0;i<out.triangleCount;i++){const HC::Triangle& t=graph.geometry.triangles.data[i];out.triangles[i].a=t.a;out.triangles[i].b=t.b;out.triangles[i].c=t.c;}
 for(unsigned int i=0;i<out.planeCount;i++){const HC::Vec4& p=graph.planes.data[i];out.planes[i].x=p.x;out.planes[i].y=p.y;out.planes[i].z=p.z;out.planes[i].w=p.w;}
 return true;
}
struct Metadata {unsigned int error,bytes,massStatus,collectedPoints,firstGraphStatus,secondGraphStatus,firstValidated,secondValidated,firstAttempts,secondAttempts;};
static bool Case(HANDLE source,void* workspace,HR::Builder& record,Metadata& info){
 PrefabScene::Arena memory;unsigned int h[7];if(!Read(source,h,28)||h[0]>1000||h[1]>1)return false;
 SourcePhysicsOwnerVertices::Polygon* polygons=(SourcePhysicsOwnerVertices::Polygon*)memory.Allocate((h[0]+1)*sizeof(SourcePhysicsOwnerVertices::Polygon));if(!polygons)return false;
 for(unsigned int p=0;p<h[0];p++){unsigned int ph[2];if(!Read(source,ph,8)||ph[1]>512)return false;WorldText::Vec3* points=(WorldText::Vec3*)memory.Allocate((ph[1]+1)*12);if(!points||!Read(source,points,ph[1]*12))return false;memset(polygons+p,0,sizeof(*polygons));polygons[p].points=points;polygons[p].count=ph[1];polygons[p].flags=ph[0];}
 WorldText::Vec3 center;memcpy(&center,h+4,12);SourcePhysicsOwnerVertices::Builder collector;
 if(!collector.BuildPrepared(polygons,h[0],center,false)){info.error=200+(unsigned int)collector.errorCode;return true;}info.collectedPoints=collector.pointCount;
 HR::Input input;memcpy(&input.mass,h+2,4);memcpy(&input.density,h+3,4);input.wrapper=h[1]!=0;
 if(!collector.pointCount){record.Build(input);info.error=(unsigned int)record.errorCode;return true;}
 if(collector.pointCount<4||collector.pointCount>128){info.error=100+HC::InvalidInput;return true;}
 HC::Vec3 cloud[128];for(unsigned int i=0;i<collector.pointCount;i++){cloud[i].x=collector.scaledVertices[i].x;cloud[i].y=collector.scaledVertices[i].y;cloud[i].z=collector.scaledVertices[i].z;}
 unsigned int* scratch=(unsigned int*)workspace;for(unsigned int i=0;i<HC::ScratchBytes/4;i++)scratch[i]=0;
  HC::Context firstContext;HC::Result first;HC::Status firstStatus=HC::BuildForWriter(cloud,collector.pointCount,workspace,HC::WorkspaceBytes,&firstContext,&first);
 info.firstGraphStatus=(unsigned int)firstStatus;info.firstValidated=first.validated?1u:0u;info.firstAttempts=first.attempts;
 if(firstStatus!=HC::Success){info.error=100+(unsigned int)firstStatus;return true;}
 OwnedGeometry one;if(!CopyGeometry(memory,first,one))return false;
 // Match the writer/64270 boundary: rebuild from the original collected cloud,
 // never from the first graph's reduced vertex output. Reuse owned workspace.
  HC::Context secondContext;HC::Result second;HC::Status secondStatus=HC::BuildForWriter(cloud,collector.pointCount,workspace,HC::WorkspaceBytes,&secondContext,&second);
 info.secondGraphStatus=(unsigned int)secondStatus;info.secondValidated=second.validated?1u:0u;info.secondAttempts=second.attempts;
 if(secondStatus!=HC::Success){info.error=100+(unsigned int)secondStatus;return true;}
 OwnedGeometry two;if(!CopyGeometry(memory,second,two))return false;
 input.points=one.points;input.pointCount=one.pointCount;input.triangles=one.triangles;input.triangleCount=one.triangleCount;input.planes=one.planes;input.planeCount=one.planeCount;
 input.massPoints=two.points;input.massPointCount=two.pointCount;input.massTriangles=two.triangles;input.massTriangleCount=two.triangleCount;
 record.Build(input);info.error=(unsigned int)record.errorCode;info.massStatus=(unsigned int)record.integrationStatus;info.bytes=record.size;return true;
}
extern "C" void HarnessEntry(){
 char input[MAX_PATH],output[MAX_PATH];DWORD ni=GetEnvironmentVariableA("FEAR_NATIVE_HULL_WRITER_INPUT",input,MAX_PATH),no=GetEnvironmentVariableA("FEAR_NATIVE_HULL_WRITER_OUTPUT",output,MAX_PATH);if(!ni||ni>=MAX_PATH||!no||no>=MAX_PATH)ExitProcess(1);
 HANDLE source=CreateFileA(input,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);if(source==INVALID_HANDLE_VALUE||GetFileSize(source,0)>16u*1024u*1024u)ExitProcess(4);unsigned int count=0;if(!Read(source,&count,4)||!count||count>2000)ExitProcess(5);
 HANDLE out=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(out==INVALID_HANDLE_VALUE||!Write(out,&count,4))ExitProcess(6);
 void* workspace=VirtualAlloc(0,HC::WorkspaceBytes,MEM_COMMIT|MEM_RESERVE,4);if(!workspace)ExitProcess(7);
 for(unsigned int c=0;c<count;c++){
  HR::Builder record;Metadata info={};info.firstGraphStatus=info.secondGraphStatus=0xffffffffu;
  if(!Case(source,workspace,record,info))ExitProcess(5);
  // Case has released collector and both copied source geometries. Record owns
  // all bytes written here, independently of source arrays and graph workspace.
  if(!Write(out,&info,sizeof(info))||!Write(out,record.data,record.size))ExitProcess(6);
 }
 unsigned char extra;DWORD done=0;if(!ReadFile(source,&extra,1,&done,0)||done)ExitProcess(5);CloseHandle(source);CloseHandle(out);VirtualFree(workspace,0,MEM_RELEASE);ExitProcess(0);
}
