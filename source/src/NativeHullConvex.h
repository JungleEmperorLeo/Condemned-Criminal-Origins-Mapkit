#pragma once
#include "NativeHullConvexSupport.h"
#include "NativeHullConvexCore.h"
namespace NativeHullConvex {
enum {ScratchBytes=8*1024*1024,WorkspaceBytes=72*1024*1024,MaximumPoints=4096,MaximumTriangles=8192,MaximumPlanes=8192};
// Input, Context, Result and workspace must be distinct live caller-owned buffers.
// Geometry views stay valid until their workspace is reused or released.
template<class T> inline bool OwnedArray(const Array<T>& a,const Context& c,unsigned int maximum){
 if(a.size<0||static_cast<unsigned int>(a.size)>maximum||(a.capacity&0x7fffffffu)<static_cast<unsigned int>(a.size))return false;
 if(!a.size)return true;
 const unsigned int p=reinterpret_cast<unsigned int>(a.data),begin=reinterpret_cast<unsigned int>(c.memory);
 return a.data&&p>=begin&&p-begin<=c.used&&static_cast<unsigned int>(a.size)<=(c.used-(p-begin))/sizeof(T);
}
inline void Clear(Result* r){if(r){r->geometry.vertices.data=0;r->geometry.vertices.size=0;r->geometry.vertices.capacity=0x80000000u;r->geometry.triangles.data=0;r->geometry.triangles.size=0;r->geometry.triangles.capacity=0x80000000u;r->planes.data=0;r->planes.size=0;r->planes.capacity=0x80000000u;r->validated=false;r->attempts=0;}}
inline Status Observe(const Vec3* points,unsigned int count,void* workspace,unsigned int workspaceBytes,Context* c,Result* out){
 Clear(out);
 if(!out||!c||!points||count<4||count>MaximumPoints||!workspace||(reinterpret_cast<unsigned int>(workspace)&15u))return InvalidInput;
 if(workspaceBytes<WorkspaceBytes)return WorkspaceTooSmall;
 for(unsigned int i=0;i<count;++i)if(!Finite(points[i].x)||!Finite(points[i].y)||!Finite(points[i].z))return InvalidInput;
 unsigned int* words=reinterpret_cast<unsigned int*>(c);for(unsigned int i=0;i<sizeof(Context)/4;++i)words[i]=0;
 c->table=detail::Callbacks;c->stackCurrent=(unsigned char*)workspace;c->stackRemaining=ScratchBytes;
 c->memory=(unsigned char*)workspace+ScratchBytes;c->memoryBytes=workspaceBytes-ScratchBytes;
 Descriptor desc={points,static_cast<int>(count),12};
 detail::Invoke(c,&desc,&out->geometry,&out->planes);
 if(c->status){Status s=static_cast<Status>(c->status);Clear(out);return s;}
 out->validated=c->validated!=0;out->attempts=c->validationCalls;
 if(!OwnedArray(out->geometry.vertices,*c,MaximumPoints)||!OwnedArray(out->geometry.triangles,*c,MaximumTriangles)||!OwnedArray(out->planes,*c,MaximumPlanes)){Clear(out);return InternalInvariant;}
 return Success;
}
inline Status FiniteGeometry(Result* out){
 if(!out->geometry.vertices.size||!out->geometry.triangles.size||!out->planes.size){Clear(out);return ValidationFailed;}
 for(int i=0;i<out->geometry.vertices.size;++i){const Vec4& p=out->geometry.vertices.data[i];if(!Finite(p.x)||!Finite(p.y)||!Finite(p.z)){Clear(out);return NonfiniteResult;}}
 for(int i=0;i<out->planes.size;++i){const Vec4& p=out->planes.data[i];if(!Finite(p.x)||!Finite(p.y)||!Finite(p.z)||!Finite(p.w)){Clear(out);return NonfiniteResult;}}
 for(int i=0;i<out->geometry.triangles.size;++i){const Triangle& t=out->geometry.triangles.data[i];if(t.a>=static_cast<unsigned int>(out->geometry.vertices.size)||t.b>=static_cast<unsigned int>(out->geometry.vertices.size)||t.c>=static_cast<unsigned int>(out->geometry.vertices.size)){Clear(out);return InternalInvariant;}}
 return Success;
}
// Research/authoring predicate: only a graph accepted by the validator succeeds.
inline Status Build(const Vec3* points,unsigned int count,void* workspace,unsigned int workspaceBytes,Context* c,Result* out){
 Status status=Observe(points,count,workspace,workspaceBytes,c,out);if(status!=Success)return status;
 if(!out->validated||out->geometry.vertices.size<4||out->geometry.triangles.size<4||out->planes.size<4){Clear(out);return ValidationFailed;}
 return FiniteGeometry(out);
}
// Original 0x6A620 retries four times; 0x64F70/0x3CDE0 still consume the final
// graph when validation remains false. Preserve that last result, not an extra
// rejection invented by the replacement. Nonfinite/incomplete output is rejected
// before serialization; validated/attempts retain the observed diagnostic state.
inline Status BuildForWriter(const Vec3* points,unsigned int count,void* workspace,unsigned int workspaceBytes,Context* c,Result* out){
 Status status=Observe(points,count,workspace,workspaceBytes,c,out);if(status!=Success)return status;
 return FiniteGeometry(out);
}
}
