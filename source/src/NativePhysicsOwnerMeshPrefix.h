#pragma once
#include "SourcePhysicsOwnerVertices.h"
#include "NativeHullConvex.h"
#include "NativeHullMassProperties.h"
#include "PhysicsMeshPrefix.h"
// One owned mesh transport prefix, deliberately stopping before MOPP CodeInfo.
// The serialized mesh remains the source collector's original fan topology;
// convex construction is used only for non-MAIN mass properties.
namespace NativePhysicsOwnerMeshPrefix {
enum ErrorCode {NoError=0,InvalidInput,CollectionFailed,EmptyGeometry,ResourceLimit,AllocationFailure,ConvexMassFailed,IntegrationFailed,NonfiniteResult};
struct Settings {
 unsigned int version,maximumBytes;
 SourcePhysicsOwnerVertices::Settings collector;
 Settings():version(1),maximumBytes(16u*1024u*1024u){collector.generateIndices=true;}
};
class Builder {
 PhysicsMeshPrefix::Block prefix;
 Builder(const Builder&);Builder& operator=(const Builder&);
 static bool Finite(float f){return NativeHullMassProperties::Finite(f);}
 static void Put(unsigned char* output,unsigned int& at,const void* source,unsigned int bytes){const unsigned char* p=(const unsigned char*)source;for(unsigned int i=0;i<bytes;i++)output[at+i]=p[i];at+=bytes;}
 static void Word(unsigned char* output,unsigned int& at,unsigned int word){Put(output,at,&word,4);}
 static float ScaledVolume(float value){float result;const float units=1000000.f;
#if defined(_MSC_VER) && defined(_M_IX86)
  __asm {
   fld value
   fmul units
   fstp result
  }
#else
  result=value*units;
#endif
  return result;
 }
 bool Fail(ErrorCode code,const char* text){Clear();errorCode=code;error=text;return false;}
 bool Finish(const SourcePhysicsOwnerVertices::Builder& collected,bool isMain,float requestedMass,float requestedDensity,const Settings& settings){
  if(!Finite(requestedMass)||!Finite(requestedDensity))return Fail(InvalidInput,"Nonfinite owner Mesh mass or density");
  if(!collected.pointCount||!collected.indexCount)return Fail(EmptyGeometry,"Empty owner Mesh has no complete record framing");
  if(collected.pointCount>=65536||collected.indexCount%3||collected.indexCount>3000000||!collected.scaledVertices||!collected.indices)return Fail(ResourceLimit,"Owner Mesh u16 profile exceeded");
  const unsigned long long wanted=108ull+(unsigned long long)collected.pointCount*12+(unsigned long long)collected.indexCount*2;
  if(wanted>settings.maximumBytes)return Fail(ResourceLimit,"Owner Mesh prefix byte limit");
  for(unsigned int i=0;i<collected.pointCount;i++)if(!WorldText::FiniteVector(collected.scaledVertices[i]))return Fail(NonfiniteResult,"Nonfinite owner Mesh vertex");
  for(unsigned int i=0;i<collected.indexCount;i++)if(collected.indices[i]>=collected.pointCount)return Fail(InvalidInput,"Owner Mesh index out of range");
  const float actualMass=requestedMass<.1f?.1f:requestedMass,actualDensity=requestedDensity<.01f?.01f:requestedDensity;
  NativeHullMassProperties::Result integrated={};float volume=0;
  if(!isMain){
   if(collected.pointCount<4||collected.pointCount>NativeHullConvex::MaximumPoints)return Fail(ResourceLimit,"Dynamic owner Mesh convex mass vertex limit");
   PrefabScene::Arena work;
   NativeHullConvex::Vec3* cloud=(NativeHullConvex::Vec3*)work.Allocate(collected.pointCount*sizeof(NativeHullConvex::Vec3));
   void* workspace=work.Allocate(NativeHullConvex::WorkspaceBytes);if(!cloud||!workspace)return Fail(AllocationFailure,"Owner Mesh mass workspace allocation failed");
   for(unsigned int i=0;i<collected.pointCount;i++){cloud[i].x=collected.scaledVertices[i].x;cloud[i].y=collected.scaledVertices[i].y;cloud[i].z=collected.scaledVertices[i].z;}
   NativeHullConvex::Context context;NativeHullConvex::Result convex;
   NativeHullConvex::Status built=NativeHullConvex::BuildForWriter(cloud,collected.pointCount,workspace,NativeHullConvex::WorkspaceBytes,&context,&convex);
   if(built!=NativeHullConvex::Success){Fail(ConvexMassFailed,"Owner Mesh convex mass construction failed");convexStatus=built;return false;}
   NativeHullMassProperties::Vec4* massPoints=(NativeHullMassProperties::Vec4*)work.Allocate(convex.geometry.vertices.size*16);
   NativeHullMassProperties::Triangle* massTriangles=(NativeHullMassProperties::Triangle*)work.Allocate(convex.geometry.triangles.size*12);
   if(!massPoints||!massTriangles)return Fail(AllocationFailure,"Owner Mesh mass geometry allocation failed");
   for(int i=0;i<convex.geometry.vertices.size;i++){const NativeHullConvex::Vec4& p=convex.geometry.vertices.data[i];massPoints[i].x=p.x;massPoints[i].y=p.y;massPoints[i].z=p.z;massPoints[i].w=0;}
   for(int i=0;i<convex.geometry.triangles.size;i++){const NativeHullConvex::Triangle& t=convex.geometry.triangles.data[i];massTriangles[i].a=t.a;massTriangles[i].b=t.b;massTriangles[i].c=t.c;}
   NativeHullMassProperties::Status status=NativeHullMassProperties::Integrate(massPoints,(unsigned int)convex.geometry.vertices.size,massTriangles,(unsigned int)convex.geometry.triangles.size,actualMass,&integrated);
   if(status!=NativeHullMassProperties::Ok){Fail(IntegrationFailed,"Owner Mesh mass integration failed");integrationStatus=status;return false;}
   volume=ScaledVolume(integrated.volume);if(!Finite(volume))return Fail(NonfiniteResult,"Owner Mesh serialized volume overflow");
   convexValidated=convex.validated;convexAttempts=convex.attempts;massVertexCount=(unsigned int)convex.geometry.vertices.size;massTriangleCount=(unsigned int)convex.geometry.triangles.size;
  }
  prefix.size=(unsigned int)wanted;prefix.data=(unsigned char*)prefix.memory.Allocate(prefix.size);if(!prefix.data)return Fail(AllocationFailure,"Owner Mesh prefix allocation failed");
  unsigned int at=0;Word(prefix.data,at,1);const float pose[7]={0,0,0,0,0,0,1};Put(prefix.data,at,pose,28);Word(prefix.data,at,1);
  float values[16]={};values[0]=actualMass;values[1]=actualDensity;values[15]=1;
  if(!isMain){values[2]=integrated.center.x;values[3]=integrated.center.y;values[4]=integrated.center.z;const unsigned int order[9]={0,4,8,1,5,9,2,6,10};for(unsigned int i=0;i<9;i++)values[5+i]=integrated.inertia[order[i]];values[14]=volume;}
  Put(prefix.data,at,values,64);Word(prefix.data,at,collected.pointCount);Word(prefix.data,at,collected.indexCount);Put(prefix.data,at,collected.scaledVertices,collected.pointCount*12);
  for(unsigned int i=0;i<collected.indexCount;i++){unsigned short index=(unsigned short)collected.indices[i];Put(prefix.data,at,&index,2);}
  if(at!=prefix.size)return Fail(InvalidInput,"Owner Mesh prefix serialization mismatch");
  data=prefix.data;size=prefix.size;pointCount=collected.pointCount;indexCount=collected.indexCount;triangleCount=indexCount/3;mainWorld=isMain;mass=actualMass;density=actualDensity;scaledVolume=volume;properties=integrated;
  return true;
 }
 static bool Valid(const Settings& s){return s.version==1&&s.maximumBytes>=108&&s.maximumBytes<=64u*1024u*1024u;}
public:
 unsigned char* data;unsigned int size,pointCount,indexCount,triangleCount,massVertexCount,massTriangleCount,convexAttempts;
 float mass,density,scaledVolume;bool mainWorld,convexValidated;
 NativeHullMassProperties::Result properties;NativeHullMassProperties::Status integrationStatus;NativeHullConvex::Status convexStatus;
 ErrorCode errorCode;const char* error;
 Builder():data(0),size(0),pointCount(0),indexCount(0),triangleCount(0),massVertexCount(0),massTriangleCount(0),convexAttempts(0),mass(0),density(0),scaledVolume(0),mainWorld(false),convexValidated(false),integrationStatus(NativeHullMassProperties::Ok),convexStatus(NativeHullConvex::Success),errorCode(NoError),error(0){unsigned char* p=(unsigned char*)&properties;for(unsigned int i=0;i<sizeof(properties);i++)p[i]=0;}
 void Clear(){prefix.memory.Clear();prefix.data=0;prefix.size=0;data=0;size=pointCount=indexCount=triangleCount=massVertexCount=massTriangleCount=convexAttempts=0;mass=density=scaledVolume=0;mainWorld=convexValidated=false;integrationStatus=NativeHullMassProperties::Ok;convexStatus=NativeHullConvex::Success;errorCode=NoError;error=0;unsigned char* p=(unsigned char*)&properties;for(unsigned int i=0;i<sizeof(properties);i++)p[i]=0;}
 const PhysicsMeshPrefix::Block& Prefix()const{return prefix;}
 bool BuildPrepared(const SourcePhysicsOwnerVertices::Polygon* polygons,unsigned int polygonCount,const WorldText::Vec3& center,bool isMain,float requestedMass,float requestedDensity,const Settings& settings=Settings()){
  Clear();if(!Valid(settings))return Fail(InvalidInput,"Invalid owner Mesh prefix settings");SourcePhysicsOwnerVertices::Settings collectorSettings=settings.collector;collectorSettings.generateIndices=true;
  SourcePhysicsOwnerVertices::Builder collected;if(!collected.BuildPrepared(polygons,polygonCount,center,isMain,collectorSettings))return Fail(CollectionFailed,"Owner Mesh prepared collection failed");
  return Finish(collected,isMain,requestedMass,requestedDensity,settings);
 }
 bool Build(const PrefabScene::Model& model,const WorldModels::Partition& partition,unsigned int group,const WorldText::Vec3& offset,float requestedMass,float requestedDensity,const Settings& settings=Settings()){
  Clear();if(!Valid(settings))return Fail(InvalidInput,"Invalid owner Mesh prefix settings");SourcePhysicsOwnerVertices::Settings collectorSettings=settings.collector;collectorSettings.generateIndices=true;
  SourcePhysicsOwnerVertices::Builder collected;if(!collected.Build(model,partition,group,offset,collectorSettings))return Fail(CollectionFailed,"Owner Mesh source collection failed");
  return Finish(collected,group==0,requestedMass,requestedDensity,settings);
 }
};
}
