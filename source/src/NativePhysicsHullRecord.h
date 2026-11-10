#pragma once
#include "NativeHullMassProperties.h"
// Owned serialization from supplied convex geometry. Convex construction and
// source collection belong to the caller; no original DLL or corpus is used.
namespace NativePhysicsHullRecord {
typedef NativeHullMassProperties::Vec4 Vec4;
typedef NativeHullMassProperties::Triangle Triangle;
enum ErrorCode {NoError=0,InvalidInput,InvalidSettings,EmptyGeometry,NonfiniteGeometry,MassIntegrationFailed,ResourceLimit,AllocationFailure,NonfiniteVolume};
struct Input {
 const Vec4* points;unsigned int pointCount;
 const Triangle* triangles;unsigned int triangleCount;
 const Vec4* planes;unsigned int planeCount;
 // Original automatic Hull reruns convex construction from the welded source
 // cloud for mass. Optional independent geometry preserves that boundary.
 const Vec4* massPoints;unsigned int massPointCount;
 const Triangle* massTriangles;unsigned int massTriangleCount;
 float mass,density;bool wrapper;
 Input():points(0),pointCount(0),triangles(0),triangleCount(0),planes(0),planeCount(0),massPoints(0),massPointCount(0),massTriangles(0),massTriangleCount(0),mass(1),density(1.1f),wrapper(false){}
};
struct Settings {
 unsigned int version,maximumVertices,maximumTriangles,maximumPlanes,maximumBytes;
 Settings():version(1),maximumVertices(100000),maximumTriangles(200000),maximumPlanes(200000),maximumBytes(16u*1024u*1024u){}
};
class Builder {
 PrefabScene::Arena memory;
 Builder(const Builder&);Builder& operator=(const Builder&);
 bool Fail(ErrorCode code,const char* message){Clear();errorCode=code;error=message;return false;}
 static void Put(unsigned char* target,unsigned int& at,const void* source,unsigned int count){const unsigned char* p=(const unsigned char*)source;for(unsigned int i=0;i<count;i++)target[at+i]=p[i];at+=count;}
 static void Word(unsigned char* target,unsigned int& at,unsigned int value){Put(target,at,&value,4);}
 static bool VectorFinite(const Vec4& p){return NativeHullMassProperties::Finite(p.x)&&NativeHullMassProperties::Finite(p.y)&&NativeHullMassProperties::Finite(p.z)&&NativeHullMassProperties::Finite(p.w);}
 static float Volume(float value){
  float result;const float units=1000000.f;
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
public:
 unsigned char* data;unsigned int size,vertexCount,triangleCount,planeCount;
 float mass,density,scaledVolume;bool wrapper;NativeHullMassProperties::Result properties;
 NativeHullMassProperties::Status integrationStatus;ErrorCode errorCode;const char* error;
 Builder():data(0),size(0),vertexCount(0),triangleCount(0),planeCount(0),mass(0),density(0),scaledVolume(0),wrapper(false),integrationStatus(NativeHullMassProperties::Ok),errorCode(NoError),error(0){unsigned char* p=(unsigned char*)&properties;for(unsigned int i=0;i<sizeof(properties);i++)p[i]=0;}
 void Clear(){memory.Clear();data=0;size=vertexCount=triangleCount=planeCount=0;mass=density=scaledVolume=0;wrapper=false;integrationStatus=NativeHullMassProperties::Ok;errorCode=NoError;error=0;unsigned char* p=(unsigned char*)&properties;for(unsigned int i=0;i<sizeof(properties);i++)p[i]=0;}
 bool Build(const Input& input,const Settings& settings=Settings()){
  Clear();
  if(settings.version!=1||!settings.maximumVertices||settings.maximumVertices>1000000||!settings.maximumTriangles||settings.maximumTriangles>2000000||!settings.maximumPlanes||settings.maximumPlanes>2000000||settings.maximumBytes<104||settings.maximumBytes>64u*1024u*1024u)return Fail(InvalidSettings,"Invalid Hull record limits");
  if(!NativeHullMassProperties::Finite(input.mass)||!NativeHullMassProperties::Finite(input.density))return Fail(InvalidInput,"Nonfinite Hull mass or density");
  // The original empty writer emits only four bytes; this API deliberately
  // rejects that incomplete standalone framing and clears all record output.
  if(!input.pointCount&&!input.triangleCount&&!input.planeCount)return Fail(EmptyGeometry,"Empty Hull has no complete record framing");
  if(!input.points||!input.triangles||!input.planes||!input.pointCount||!input.triangleCount||!input.planeCount)return Fail(InvalidInput,"Incomplete supplied Hull geometry");
  if(input.pointCount>settings.maximumVertices||input.triangleCount>settings.maximumTriangles||input.planeCount>settings.maximumPlanes)return Fail(ResourceLimit,"Hull record geometry limit");
  unsigned long long wanted=104ull+(input.wrapper?4ull:0ull)+(unsigned long long)input.pointCount*12+(unsigned long long)input.planeCount*16;
  if(wanted>settings.maximumBytes)return Fail(ResourceLimit,"Hull record byte limit");
  for(unsigned int i=0;i<input.pointCount;i++)if(!VectorFinite(input.points[i]))return Fail(NonfiniteGeometry,"Nonfinite Hull vertex");
  for(unsigned int i=0;i<input.planeCount;i++)if(!VectorFinite(input.planes[i]))return Fail(NonfiniteGeometry,"Nonfinite Hull plane");
  for(unsigned int i=0;i<input.triangleCount;i++)if(input.triangles[i].a>=input.pointCount||input.triangles[i].b>=input.pointCount||input.triangles[i].c>=input.pointCount)return Fail(InvalidInput,"Hull triangle index out of range");
  const bool separateMass=input.massPoints||input.massPointCount||input.massTriangles||input.massTriangleCount;
  if(separateMass&&(!input.massPoints||!input.massPointCount||!input.massTriangles||!input.massTriangleCount))return Fail(InvalidInput,"Incomplete supplied Hull mass geometry");
  const Vec4* massPoints=separateMass?input.massPoints:input.points;const Triangle* massTriangles=separateMass?input.massTriangles:input.triangles;
  const unsigned int massPointCount=separateMass?input.massPointCount:input.pointCount,massTriangleCount=separateMass?input.massTriangleCount:input.triangleCount;
  if(massPointCount>settings.maximumVertices||massTriangleCount>settings.maximumTriangles)return Fail(ResourceLimit,"Hull mass geometry limit");
  if(separateMass){
   for(unsigned int i=0;i<massPointCount;i++)if(!VectorFinite(massPoints[i]))return Fail(NonfiniteGeometry,"Nonfinite Hull mass vertex");
   for(unsigned int i=0;i<massTriangleCount;i++)if(massTriangles[i].a>=massPointCount||massTriangles[i].b>=massPointCount||massTriangles[i].c>=massPointCount)return Fail(InvalidInput,"Hull mass triangle index out of range");
  }
  float clampedMass=input.mass<.1f?.1f:input.mass,clampedDensity=input.density<.01f?.01f:input.density;
  NativeHullMassProperties::Result integrated;
  NativeHullMassProperties::Status status=NativeHullMassProperties::Integrate(massPoints,massPointCount,massTriangles,massTriangleCount,clampedMass,&integrated,settings.maximumVertices,settings.maximumTriangles);
  if(status!=NativeHullMassProperties::Ok){Fail(MassIntegrationFailed,"Hull mass integration failed");integrationStatus=status;return false;}
  float volume=Volume(integrated.volume);if(!NativeHullMassProperties::Finite(volume))return Fail(NonfiniteVolume,"Hull serialized volume overflow");
  unsigned char* output=(unsigned char*)memory.Allocate((unsigned int)wanted);if(!output)return Fail(AllocationFailure,"Hull record allocation failed");
  unsigned int at=0;if(input.wrapper)Word(output,at,5);const float pose[7]={0,0,0,0,0,0,1};Put(output,at,pose,28);Word(output,at,4);
  Put(output,at,&clampedMass,4);Put(output,at,&clampedDensity,4);Put(output,at,&integrated.center,12);
  const unsigned int matrixOrder[9]={0,4,8,1,5,9,2,6,10};for(unsigned int i=0;i<9;i++)Put(output,at,integrated.inertia+matrixOrder[i],4);
  const float radius=3.f;Put(output,at,&volume,4);Put(output,at,&radius,4);Word(output,at,input.pointCount);Word(output,at,input.planeCount);
  for(unsigned int i=0;i<input.pointCount;i++)Put(output,at,input.points+i,12);
  for(unsigned int i=0;i<input.planeCount;i++)Put(output,at,input.planes+i,16);
  if(at!=(unsigned int)wanted)return Fail(InvalidInput,"Hull record serialization mismatch");
  data=output;size=at;vertexCount=input.pointCount;triangleCount=input.triangleCount;planeCount=input.planeCount;
  mass=clampedMass;density=clampedDensity;scaledVolume=volume;wrapper=input.wrapper;properties=integrated;return true;
 }
};
}
