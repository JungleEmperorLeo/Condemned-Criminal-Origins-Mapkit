#pragma once
// Owned memory and private calling convention for the scalar convex core.
// XMM7 carries this invocation's Context only inside private assembly calls.
// Every allocator callback preserves it. Public entry saves/restores FP/SSE state.
// No mutable process global, original address, allocator or DLL is consumed.
namespace NativeHullConvex {
struct Vec3 {float x,y,z;};
struct Vec4 {float x,y,z,w;};
struct Triangle {unsigned int a,b,c;};
template<class T> struct Array {T* data;int size;unsigned int capacity;};
struct Geometry {Array<Vec4> vertices;Array<Triangle> triangles;};
struct Descriptor {const void* data;int count;int stride;};
enum Status {Success=0,InvalidInput,WorkspaceTooSmall,AllocationLimit,InternalInvariant,ValidationFailed,NonfiniteResult};
enum {MaximumAllocationCalls=200000,MaximumAllocationBytes=64*1024*1024};
struct Context {
 const void* const* table;unsigned int reserved[7];
 unsigned char* stackCurrent;unsigned int stackRemaining,unused40,stackMask;
 unsigned char* memory;unsigned int memoryBytes,used,allocationCalls,allocationBytes;
 unsigned int escapeStack,escapeFrame,escapeResume,status,validated,validationCalls;
};
struct Result {Geometry geometry;Array<Vec4> planes;bool validated;unsigned int attempts;};
inline bool Finite(float f){union B {float f;unsigned int u;};B b;b.f=f;return (b.u&0x7f800000u)!=0x7f800000u;}
#if defined(_MSC_VER) && defined(_M_IX86)
namespace detail {
static void CoreBlock();
static __declspec(naked) void Abort(Context*,unsigned int){
 __asm {
  mov ecx,dword ptr [esp+4]
  mov eax,dword ptr [esp+8]
  mov dword ptr [ecx+80],eax
  mov esp,dword ptr [ecx+68]
  mov ebp,dword ptr [ecx+72]
  jmp dword ptr [ecx+76]
 }
}
static void* __cdecl AllocateImpl(Context* c,unsigned int bytes,unsigned int){
 if(bytes>8u*1024u*1024u||c->allocationCalls>=MaximumAllocationCalls||c->allocationBytes>MaximumAllocationBytes||bytes>MaximumAllocationBytes-c->allocationBytes||c->used>c->memoryBytes)Abort(c,AllocationLimit);
 unsigned int aligned=(c->used+15u)&~15u;
 if(aligned<c->used||aligned>c->memoryBytes||bytes>c->memoryBytes-aligned)Abort(c,AllocationLimit);
 unsigned char* p=c->memory+aligned;c->used=aligned+bytes;++c->allocationCalls;c->allocationBytes+=bytes;
 for(unsigned int i=0;i<bytes;++i)p[i]=0;
 return p;
}
static void __cdecl StackReleaseImpl(Context* c,void* pointer,unsigned int bytes){
 unsigned char* p=(unsigned char*)pointer;
 unsigned char* first=c->memory-8u*1024u*1024u;
 if(p<first||p>c->memory||bytes>static_cast<unsigned int>(c->memory-p)||p+bytes!=c->stackCurrent)Abort(c,InternalInvariant);
 c->stackCurrent=p;c->stackRemaining+=bytes;
}
static __declspec(naked) void AllocateCallback(){
 __asm {
  sub esp,16
  movups xmmword ptr [esp],xmm7
  push dword ptr [esp+24]
  push dword ptr [esp+24]
  push ecx
  call AllocateImpl
  add esp,12
  movups xmm7,xmmword ptr [esp]
  add esp,16
  ret 8
 }
}
static __declspec(naked) void ReleaseCallback(){__asm {ret 4}}
static __declspec(naked) void ReleaseChunkCallback(){__asm {ret 12}}
static __declspec(naked) void StackReleaseCallback(){
 __asm {
  sub esp,16
  movups xmmword ptr [esp],xmm7
  push dword ptr [esp+24]
  push dword ptr [esp+24]
  push ecx
  call StackReleaseImpl
  add esp,12
  movups xmm7,xmmword ptr [esp]
  add esp,16
  ret 8
 }
}
static __declspec(naked) void TrapCallback(){__asm {push InternalInvariant}
 __asm {push ecx}
 __asm {call Abort}
}
static __declspec(naked) void StackAllocateCallback(){__asm {push WorkspaceTooSmall}
 __asm {push ecx}
 __asm {call Abort}
}
static const void* const Callbacks[12]={
 (const void*)AllocateCallback,(const void*)ReleaseCallback,(const void*)TrapCallback,(const void*)TrapCallback,
 (const void*)AllocateCallback,(const void*)ReleaseChunkCallback,(const void*)TrapCallback,(const void*)TrapCallback,
 (const void*)TrapCallback,(const void*)StackAllocateCallback,(const void*)StackReleaseCallback,(const void*)TrapCallback};
// Restores the caller's FP/SSE environment even when a bounded allocation aborts.
static __declspec(naked) void Invoke(Context*,const Descriptor*,Geometry*,Array<Vec4>*){
 __asm {
  push ebp
  mov ebp,esp
  push ebx
  push esi
  push edi
  and esp,0fffffff0h
  sub esp,512
  fxsave [esp]
  mov eax,dword ptr [ebp+8]
  mov dword ptr [eax+68],esp
  mov dword ptr [eax+72],ebp
  lea edx,Restore
  mov dword ptr [eax+76],edx
  movd xmm7,eax
  fninit
  fldcw word ptr [esp]
  push dword ptr [ebp+20]
  push dword ptr [ebp+16]
  push dword ptr [ebp+12]
  call CoreBlock
  add esp,12
 Restore:
  fxrstor [esp]
  lea esp,[ebp-12]
  pop edi
  pop esi
  pop ebx
  pop ebp
  ret
 }
}
}
#else
#error NativeHullConvex requires the recovered MSVC x86 arithmetic profile.
#endif
}
