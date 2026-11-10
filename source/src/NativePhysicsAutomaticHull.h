#pragma once
#include "SourcePhysicsOwnerVertices.h"
#include "NativeHullConvex.h"
#include "NativePhysicsHullRecord.h"
// Automatic owner Hull: source collection -> native graph twice -> native mass
// and complete record. All temporary source/graph storage belongs to this build.
namespace NativePhysicsAutomaticHull {
enum ErrorCode {NoError=0,InvalidInput,SourceCollectionFailed,EmptyGeometry,ConvexFailed,AllocationFailure,RecordFailed};
class Builder {
 PrefabScene::Arena memory;
 SourcePhysicsOwnerVertices::Builder input;
 NativePhysicsHullRecord::Builder record;
 Builder(const Builder&);Builder& operator=(const Builder&);
 bool Fail(ErrorCode code,const char* message){char saved[512];lstrcpynA(saved,message?message:"Automatic Hull build failed",512);Clear();errorCode=code;lstrcpyA(diagnostic,saved);error=diagnostic;return false;}
 static const char* ConvexMessage(NativeHullConvex::Status s){return s==NativeHullConvex::InvalidInput?"Automatic Hull collision point limit or invalid input":s==NativeHullConvex::ValidationFailed?"Automatic Hull convex graph did not validate":s==NativeHullConvex::NonfiniteResult?"Automatic Hull produced nonfinite convex geometry":s==NativeHullConvex::AllocationLimit?"Automatic Hull convex memory budget":"Automatic Hull convex preparation failed";}
 NativePhysicsHullRecord::Vec4* CopyPoints(const NativeHullConvex::Array<NativeHullConvex::Vec4>& source){
  NativePhysicsHullRecord::Vec4* output=(NativePhysicsHullRecord::Vec4*)memory.Allocate(source.size*16);if(!output)return 0;
  for(int i=0;i<source.size;++i){output[i].x=source.data[i].x;output[i].y=source.data[i].y;output[i].z=source.data[i].z;output[i].w=0;}
  return output;
 }
 NativePhysicsHullRecord::Triangle* CopyTriangles(const NativeHullConvex::Array<NativeHullConvex::Triangle>& source){
  NativePhysicsHullRecord::Triangle* output=(NativePhysicsHullRecord::Triangle*)memory.Allocate(source.size*12);if(!output)return 0;
  for(int i=0;i<source.size;++i){output[i].a=source.data[i].a;output[i].b=source.data[i].b;output[i].c=source.data[i].c;}return output;
 }
 bool Finish(float mass,float density,bool wrapper){
  if(!WorldText::Finite(mass)||!WorldText::Finite(density))return Fail(InvalidInput,"Nonfinite automatic Hull mass or density");
  if(!input.pointCount)return Fail(EmptyGeometry,"Empty automatic Hull has no complete record framing");
  NativeHullConvex::Vec3* cloud=(NativeHullConvex::Vec3*)memory.Allocate(input.pointCount*12);if(!cloud)return Fail(AllocationFailure,"Automatic Hull point allocation failed");
  for(unsigned int i=0;i<input.pointCount;++i){cloud[i].x=input.scaledVertices[i].x;cloud[i].y=input.scaledVertices[i].y;cloud[i].z=input.scaledVertices[i].z;}
  unsigned char* raw=(unsigned char*)memory.Allocate(NativeHullConvex::WorkspaceBytes+16);if(!raw)return Fail(AllocationFailure,"Automatic Hull workspace allocation failed");
  void* workspace=(void*)((reinterpret_cast<unsigned int>(raw)+15u)&~15u);
  NativeHullConvex::Context context;NativeHullConvex::Result geometry;
  NativeHullConvex::Status status=NativeHullConvex::BuildForWriter(cloud,input.pointCount,workspace,NativeHullConvex::WorkspaceBytes,&context,&geometry);
  if(status!=NativeHullConvex::Success){Fail(ConvexFailed,ConvexMessage(status));convexStatus=status;return false;}
  unsigned int fallbacks=geometry.validated?0u:1u;
  NativePhysicsHullRecord::Input supplied;supplied.points=CopyPoints(geometry.geometry.vertices);supplied.pointCount=geometry.geometry.vertices.size;
  supplied.triangles=CopyTriangles(geometry.geometry.triangles);supplied.triangleCount=geometry.geometry.triangles.size;
  NativePhysicsHullRecord::Vec4* planes=(NativePhysicsHullRecord::Vec4*)memory.Allocate(geometry.planes.size*16);supplied.planes=planes;supplied.planeCount=geometry.planes.size;
  if(!supplied.points||!supplied.triangles||!planes)return Fail(AllocationFailure,"Automatic Hull geometry copy failed");
  for(int i=0;i<geometry.planes.size;++i){planes[i].x=geometry.planes.data[i].x;planes[i].y=geometry.planes.data[i].y;planes[i].z=geometry.planes.data[i].z;planes[i].w=geometry.planes.data[i].w;}
  // Original 0x64270 rebuilds from the same welded/scaled cloud for integration.
  // Reuse workspace only after owning every first-graph field needed on disk.
  status=NativeHullConvex::BuildForWriter(cloud,input.pointCount,workspace,NativeHullConvex::WorkspaceBytes,&context,&geometry);
  if(status!=NativeHullConvex::Success){Fail(ConvexFailed,ConvexMessage(status));convexStatus=status;return false;}
  if(!geometry.validated)++fallbacks;
  supplied.massPoints=CopyPoints(geometry.geometry.vertices);supplied.massPointCount=geometry.geometry.vertices.size;
  supplied.massTriangles=CopyTriangles(geometry.geometry.triangles);supplied.massTriangleCount=geometry.geometry.triangles.size;
  if(!supplied.massPoints||!supplied.massTriangles)return Fail(AllocationFailure,"Automatic Hull mass geometry copy failed");
  supplied.mass=mass;supplied.density=density;supplied.wrapper=wrapper;
  if(!record.Build(supplied))return Fail(RecordFailed,record.error);
  data=record.data;size=record.size;pointCount=input.pointCount;vertexCount=record.vertexCount;triangleCount=record.triangleCount;planeCount=record.planeCount;fallbackGraphBuilds=fallbacks;return true;
 }
public:
 unsigned char* data;unsigned int size,pointCount,vertexCount,triangleCount,planeCount,fallbackGraphBuilds;
 NativeHullConvex::Status convexStatus;ErrorCode errorCode;const char* error;char diagnostic[512];
 Builder():data(0),size(0),pointCount(0),vertexCount(0),triangleCount(0),planeCount(0),fallbackGraphBuilds(0),convexStatus(NativeHullConvex::Success),errorCode(NoError),error(0){diagnostic[0]=0;}
 void Clear(){record.Clear();input.Clear();memory.Clear();data=0;size=pointCount=vertexCount=triangleCount=planeCount=fallbackGraphBuilds=0;convexStatus=NativeHullConvex::Success;errorCode=NoError;error=0;diagnostic[0]=0;}
 bool Build(const PrefabScene::Model& model,const WorldModels::Partition& partition,unsigned int group,const WorldText::Vec3& offset,float mass,float density,bool wrapper=false){
  Clear();if(!group||group>=partition.groupCount)return Fail(InvalidInput,"Invalid automatic Hull owner group");
  if(!input.Build(model,partition,group,offset))return Fail(SourceCollectionFailed,input.error);return Finish(mass,density,wrapper);
 }
 bool BuildPrepared(const SourcePhysicsOwnerVertices::Polygon* polygons,unsigned int count,const WorldText::Vec3& center,bool isMainWorld,float mass,float density,bool wrapper=false){
  Clear();if(!input.BuildPrepared(polygons,count,center,isMainWorld))return Fail(SourceCollectionFailed,input.error);return Finish(mass,density,wrapper);
 }
};
}
