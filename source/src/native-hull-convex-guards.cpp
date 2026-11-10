#define FEAR_PRIMARY_SURFACE_PREPARATION 1
#include "WorldPacker.cpp"
#include "NativeHullConvex.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
namespace HullConvexGuards {
using namespace NativeHullConvex;
using NativeHullConvex::detail::Invoke;
static const unsigned short Control=0x077fu;
static const unsigned int SseControl=0x00003f80u;
static const unsigned int Pattern[4]={0x01234567u,0x89abcdefu,0x76543210u,0xfedcba98u};
struct __declspec(align(16)) Snapshot {unsigned char bytes[512];};
static Snapshot saved,before,after;
static bool Equal(const void* a,const void* b,unsigned int bytes){const unsigned char* x=(const unsigned char*)a;const unsigned char* y=(const unsigned char*)b;for(unsigned int i=0;i<bytes;i++)if(x[i]!=y[i])return false;return true;}
static float Bits(unsigned int bits){float value;memcpy(&value,&bits,4);return value;}
static bool Empty(const Result& r){return !r.geometry.vertices.data&&!r.geometry.vertices.size&&r.geometry.vertices.capacity==0x80000000u&&!r.geometry.triangles.data&&!r.geometry.triangles.size&&r.geometry.triangles.capacity==0x80000000u&&!r.planes.data&&!r.planes.size&&r.planes.capacity==0x80000000u&&!r.validated&&!r.attempts;}
static void Dirty(Result& r){memset(&r,0x5a,sizeof(r));}
static bool Failure(Status wanted,const Vec3* p,unsigned int count,void* w,unsigned int bytes,Context* c){Result r;Dirty(r);return Build(p,count,w,bytes,c,&r)==wanted&&Empty(r);}
static bool Within(const void* pointer,unsigned int count,unsigned int stride,const unsigned char* workspace,unsigned int bytes){unsigned int p=(unsigned int)pointer,b=(unsigned int)workspace;if(!count)return !pointer||p>=b&&p<=b+bytes;return p>=b&&p<=b+bytes&&count<=(b+bytes-p)/stride;}
static bool Framing(const Result& r,const unsigned char* workspace){if(!r.validated||r.attempts<1||r.attempts>4||r.geometry.vertices.size<4||r.geometry.triangles.size<4||r.planes.size<4)return false;if(!Within(r.geometry.vertices.data,(unsigned int)r.geometry.vertices.size,16,workspace,WorkspaceBytes)||!Within(r.geometry.triangles.data,(unsigned int)r.geometry.triangles.size,12,workspace,WorkspaceBytes)||!Within(r.planes.data,(unsigned int)r.planes.size,16,workspace,WorkspaceBytes))return false;for(int i=0;i<r.geometry.triangles.size;i++){const Triangle& t=r.geometry.triangles.data[i];if(t.a>=(unsigned int)r.geometry.vertices.size||t.b>=(unsigned int)r.geometry.vertices.size||t.c>=(unsigned int)r.geometry.vertices.size||t.a==t.b||t.a==t.c||t.b==t.c)return false;}return true;}
static unsigned int CopyGeometry(const Result& r,unsigned char* output){unsigned int at=0;for(int i=0;i<r.geometry.vertices.size;i++){memcpy(output+at,&r.geometry.vertices.data[i],12);at+=12;}memcpy(output+at,r.geometry.triangles.data,r.geometry.triangles.size*12);at+=r.geometry.triangles.size*12;memcpy(output+at,r.planes.data,r.planes.size*16);return at+r.planes.size*16;}
static void Initialize(Context& c,unsigned char* workspace,unsigned int memoryBytes){memset(&c,0,sizeof(c));c.table=NativeHullConvex::detail::Callbacks;c.stackCurrent=workspace;c.stackRemaining=ScratchBytes;c.memory=workspace+ScratchBytes;c.memoryBytes=memoryBytes;}
// Full SIMD register preservation is a private Invoke contract. Public C++ ABI
// permits volatile XMM registers to change during parameter/result validation.
static __declspec(naked) void InvokeWithEnvironment(Context*,const Descriptor*,Geometry*,Array<Vec4>*,Snapshot*,Snapshot*,Snapshot*){
 __asm {
  push ebp
  mov ebp,esp
  mov eax,dword ptr [ebp+24]
  fxsave [eax]
  fninit
  fldcw word ptr [Control]
  ldmxcsr dword ptr [SseControl]
  fld1
  fldz
  movups xmm0,xmmword ptr [Pattern]
  movups xmm1,xmmword ptr [Pattern]
  movups xmm2,xmmword ptr [Pattern]
  movups xmm3,xmmword ptr [Pattern]
  movups xmm4,xmmword ptr [Pattern]
  movups xmm5,xmmword ptr [Pattern]
  movups xmm6,xmmword ptr [Pattern]
  movups xmm7,xmmword ptr [Pattern]
  mov eax,dword ptr [ebp+28]
  fxsave [eax]
  push dword ptr [ebp+20]
  push dword ptr [ebp+16]
  push dword ptr [ebp+12]
  push dword ptr [ebp+8]
  call Invoke
  add esp,16
  mov eax,dword ptr [ebp+32]
  fxsave [eax]
  mov eax,dword ptr [ebp+24]
  fxrstor [eax]
  pop ebp
  ret
 }
}
static __declspec(naked) void SetEnvironment(Snapshot*){__asm {mov eax,dword ptr [esp+4]}
 __asm {fxsave [eax]}
 __asm {fninit}
 __asm {fldcw word ptr [Control]}
 __asm {ldmxcsr dword ptr [SseControl]}
 __asm {fld1}
 __asm {fldz}
 __asm {ret}
}
static __declspec(naked) void SaveEnvironment(Snapshot*){__asm {mov eax,dword ptr [esp+4]}
 __asm {fxsave [eax]}
 __asm {ret}
}
static __declspec(naked) void RestoreEnvironment(Snapshot*){__asm {mov eax,dword ptr [esp+4]}
 __asm {fxrstor [eax]}
 __asm {ret}
}
static bool PublicEnvironment(const Vec3* p,unsigned char* w,Context& c,Result& r){SetEnvironment(&saved);SaveEnvironment(&before);Status status=Build(p,4,w,WorkspaceBytes,&c,&r);SaveEnvironment(&after);RestoreEnvironment(&saved);return status==Success&&Equal(before.bytes,after.bytes,160);}
static bool PrivateEnvironment(Context& c,const Descriptor& descriptor,Result& result){Clear(&result);InvokeWithEnvironment(&c,&descriptor,&result.geometry,&result.planes,&saved,&before,&after);return Equal(&before,&after,512);}
static bool NonfiniteRejected(unsigned int bits,unsigned char* w,Context& c){Vec3 p[4]={{0,0,0},{1,0,0},{0,1,0},{0,0,1}};for(unsigned int axis=0;axis<3;axis++){float* values=(float*)&p[3];float old=values[axis];values[axis]=Bits(bits);if(!Failure(InvalidInput,p,4,w,WorkspaceBytes,&c))return false;values[axis]=old;}return true;}
static bool Write(HANDLE f,const void* data,unsigned int bytes){DWORD done=0;return WriteFile(f,data,bytes,&done,0)&&done==bytes;}
}
extern "C" void HarnessEntry(){using namespace HullConvexGuards;using namespace NativeHullConvex;
 PrefabScene::Arena memory;unsigned char* allocation=(unsigned char*)memory.Allocate(WorkspaceBytes*2u+32u);if(!allocation)ExitProcess(2);unsigned char* workspace=(unsigned char*)(((unsigned int)allocation+15)&~15u);unsigned char* second=workspace+WorkspaceBytes;
 unsigned int checks[128]={},count=0;
#define CHECK(label,expression) checks[count++]=(expression)?1u:0u
 const Vec3 tetra[4]={{0,0,0},{1,0,0},{0,1,0},{0,0,1}};Vec3 source[4];memcpy(source,tetra,sizeof(source));Context c,d;Result result;
 Clear(&result);CHECK("explicit clear initializes all result fields",Empty(result));Clear(0);CHECK("null result clear is safe",true);
 CHECK("null output rejected",Build(tetra,4,workspace,WorkspaceBytes,&c,0)==InvalidInput);
 CHECK("null context rejects and clears old output",Failure(InvalidInput,tetra,4,workspace,WorkspaceBytes,0));
 CHECK("null input rejects and clears old output",Failure(InvalidInput,0,4,workspace,WorkspaceBytes,&c));
 CHECK("null workspace rejects and clears old output",Failure(InvalidInput,tetra,4,0,WorkspaceBytes,&c));
 CHECK("zero points reject before reading input",Failure(InvalidInput,tetra,0,workspace,WorkspaceBytes,&c));
 CHECK("three points reject before reading input",Failure(InvalidInput,tetra,3,workspace,WorkspaceBytes,&c));
  CHECK("maximum plus one points reject before reading input",Failure(InvalidInput,tetra,MaximumPoints+1,workspace,WorkspaceBytes,&c));
 CHECK("huge point count rejects before multiplication",Failure(InvalidInput,tetra,0xffffffffu,workspace,WorkspaceBytes,&c));
 CHECK("one byte workspace rejected before access",Failure(WorkspaceTooSmall,tetra,4,workspace,1,&c));
 CHECK("workspace lower bound excludes one missing byte",Failure(WorkspaceTooSmall,tetra,4,workspace,WorkspaceBytes-1,&c));
 CHECK("misaligned workspace rejected",Failure(InvalidInput,tetra,4,workspace+1,WorkspaceBytes,&c));
 CHECK("NaN in each xyz component rejects",NonfiniteRejected(0x7fc12345u,workspace,c));
 CHECK("positive infinite xyz components reject",NonfiniteRejected(0x7f800000u,workspace,c));
 CHECK("negative infinite xyz components reject",NonfiniteRejected(0xff800000u,workspace,c));
 CHECK("tetra succeeds after all invalid calls",Build(source,4,workspace,WorkspaceBytes,&c,&result)==Success&&Framing(result,workspace));
 CHECK("source points are never modified",Equal(source,tetra,sizeof(source)));
 CHECK("tetra produces four independent vertices triangles planes",result.geometry.vertices.size==4&&result.geometry.triangles.size==4&&result.planes.size==4);
  CHECK("allocator stays within owned byte and call budgets",c.used<=c.memoryBytes&&c.allocationBytes<=MaximumAllocationBytes&&c.allocationCalls>0&&c.allocationCalls<=MaximumAllocationCalls&&c.status==Success);
  static unsigned char savedGeometry[2048],nextGeometry[2048];unsigned int bytes=CopyGeometry(result,savedGeometry),calls=c.allocationCalls,used=c.used,allocated=c.allocationBytes,attempts=result.attempts;
 CHECK("fresh invocation reproduces geometry and allocation order",Build(source,4,workspace,WorkspaceBytes,&c,&result)==Success&&CopyGeometry(result,nextGeometry)==bytes&&Equal(savedGeometry,nextGeometry,bytes)&&c.allocationCalls==calls&&c.used==used&&c.allocationBytes==allocated&&result.attempts==attempts);
 const Vec3 translated[4]={{10,-20,30},{12,-20,30},{10,-17,30},{10,-20,34}};Result other;CHECK("second independent context succeeds",Build(translated,4,second,WorkspaceBytes,&d,&other)==Success&&Framing(other,second));
 CHECK("interleaved build cannot overwrite first workspace",CopyGeometry(result,nextGeometry)==bytes&&Equal(savedGeometry,nextGeometry,bytes));
 CHECK("context cursors and storage remain independent",c.memory==workspace+ScratchBytes&&d.memory==second+ScratchBytes&&c.memory!=d.memory&&c.stackCurrent>=workspace&&c.stackCurrent<=workspace+ScratchBytes&&d.stackCurrent>=second&&d.stackCurrent<=second+ScratchBytes);
 Result view=result;Clear(&result);CHECK("clearing result does not invalidate caller owned geometry",Empty(result)&&CopyGeometry(view,nextGeometry)==bytes&&Equal(savedGeometry,nextGeometry,bytes));
 CHECK("public build preserves x87 stack control and MXCSR",PublicEnvironment(source,workspace,c,result));
 Initialize(c,workspace,WorkspaceBytes-ScratchBytes);Descriptor descriptor={tetra,4,12};CHECK("private entry preserves all x87 and SSE register state",PrivateEnvironment(c,descriptor,result)&&c.status==Success);
 CHECK("private successful geometry stays in caller workspace",result.geometry.vertices.size==4&&Within(result.geometry.vertices.data,4,16,workspace,WorkspaceBytes));
 Initialize(c,workspace,1);CHECK("allocator failure escapes without crashing",PrivateEnvironment(c,descriptor,result)&&c.status==AllocationLimit&&c.used<=1&&c.allocationBytes<=1);
 Clear(&result);CHECK("caller can discard escaped partial graph",Empty(result));
 CHECK("public API reuses context after allocation escape",Build(source,4,workspace,WorkspaceBytes,&c,&result)==Success&&Framing(result,workspace));
 Initialize(c,workspace,WorkspaceBytes-ScratchBytes);c.stackRemaining=0;CHECK("scratch exhaustion escapes with workspace status",PrivateEnvironment(c,descriptor,result)&&c.status==WorkspaceTooSmall);
 CHECK("public API reuses context after scratch escape",Build(source,4,workspace,WorkspaceBytes,&c,&result)==Success&&Framing(result,workspace));
 Vec3 planar[4]={{0,0,0},{1,0,0},{0,1,0},{1,1,0}};CHECK("strict builder rejects planar cloud with cleared output",Failure(ValidationFailed,planar,4,workspace,WorkspaceBytes,&c));
 Vec3 collinear[4]={{0,0,0},{1,0,0},{2,0,0},{3,0,0}};CHECK("strict builder rejects collinear cloud with cleared output",Failure(ValidationFailed,collinear,4,workspace,WorkspaceBytes,&c));
 Vec3 repeated[128];for(unsigned int i=0;i<128;i++)repeated[i]=tetra[i%4];CHECK("maximum accepted source count handles duplicate cloud",Build(repeated,128,workspace,WorkspaceBytes,&c,&result)==Success&&Framing(result,workspace)&&result.geometry.vertices.size==4);
 for(unsigned int i=0;i<128;i++)repeated[i]=tetra[0];CHECK("strict builder rejects identical point cloud with cleared output",Failure(ValidationFailed,repeated,128,workspace,WorkspaceBytes,&c));
 CHECK("failure does not poison another context output",Framing(other,second)&&other.geometry.vertices.size==4&&other.geometry.vertices.data[0].x>=10);
 CHECK("normal source remains reusable after degenerate failures",Build(source,4,workspace,WorkspaceBytes,&c,&result)==Success&&CopyGeometry(result,nextGeometry)==bytes&&Equal(savedGeometry,nextGeometry,bytes));
 CHECK("one extra workspace byte is accepted safely",Build(source,4,workspace,WorkspaceBytes+1,&c,&result)==Success&&Framing(result,workspace)&&c.used<=c.memoryBytes);
  Dirty(result);CHECK("writer entry null input clears output",BuildForWriter(0,4,workspace,WorkspaceBytes,&c,&result)==InvalidInput&&Empty(result));
  CHECK("writer preserves the finite last graph after four failed validations",BuildForWriter(planar,4,workspace,WorkspaceBytes,&c,&result)==Success&&!result.validated&&result.attempts==4&&result.geometry.vertices.size>0&&result.geometry.triangles.size>0&&result.planes.size>0);
  CHECK("strict authoring predicate still rejects the same planar graph",Failure(ValidationFailed,planar,4,workspace,WorkspaceBytes,&c));
  CHECK("writer normal tetra matches strict builder output",BuildForWriter(source,4,workspace,WorkspaceBytes,&c,&result)==Success&&result.validated&&CopyGeometry(result,nextGeometry)==bytes&&Equal(savedGeometry,nextGeometry,bytes));
  CHECK("writer never mutates source cloud",Equal(source,tetra,sizeof(source)));
  Initialize(c,workspace,WorkspaceBytes-ScratchBytes);c.allocationCalls=MaximumAllocationCalls;CHECK("exact allocation call cap escapes before allocating",PrivateEnvironment(c,descriptor,result)&&c.status==AllocationLimit&&c.used==0&&c.allocationCalls==MaximumAllocationCalls);
  Initialize(c,workspace,WorkspaceBytes-ScratchBytes);c.allocationCalls=MaximumAllocationCalls-1;CHECK("last available allocator call is counted before next call rejects",PrivateEnvironment(c,descriptor,result)&&c.status==AllocationLimit&&c.allocationCalls==MaximumAllocationCalls&&c.used>0);
  Initialize(c,workspace,WorkspaceBytes-ScratchBytes);c.allocationBytes=MaximumAllocationBytes;CHECK("exact cumulative byte cap escapes before allocating",PrivateEnvironment(c,descriptor,result)&&c.status==AllocationLimit&&c.used==0&&c.allocationBytes==MaximumAllocationBytes);
 char output[MAX_PATH];DWORD length=GetEnvironmentVariableA("FEAR_HULL_CONVEX_GUARDS_OUTPUT",output,MAX_PATH);if(!length||length>=MAX_PATH)ExitProcess(1);HANDLE file=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);unsigned int header[2]={0x31474348u,count};if(file==INVALID_HANDLE_VALUE||!Write(file,header,8)||!Write(file,checks,count*4))ExitProcess(3);CloseHandle(file);for(unsigned int i=0;i<count;i++)if(!checks[i])ExitProcess(9);ExitProcess(0);
}
