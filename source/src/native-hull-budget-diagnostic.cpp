#include "WorldPacker.cpp"
#include "NativeHullConvex.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
namespace HC=NativeHullConvex;
static unsigned int researchCallLimit=100000;
static bool TextEqual(const char* a,const char* b){while(*a&&*a==*b){++a;++b;}return *a==*b;}
static void* __cdecl ResearchAllocate(HC::Context* c,unsigned int bytes,unsigned int){if(bytes>8u*1024u*1024u||c->allocationCalls>=researchCallLimit||bytes>64u*1024u*1024u-c->allocationBytes||c->used>c->memoryBytes)HC::detail::Abort(c,HC::AllocationLimit);unsigned int aligned=(c->used+15u)&~15u;if(aligned<c->used||aligned>c->memoryBytes||bytes>c->memoryBytes-aligned)HC::detail::Abort(c,HC::AllocationLimit);unsigned char* p=c->memory+aligned;c->used=aligned+bytes;++c->allocationCalls;c->allocationBytes+=bytes;for(unsigned int i=0;i<bytes;i++)p[i]=0;return p;}
static __declspec(naked) void ResearchAllocateCallback(){__asm {
 sub esp,16
 movups xmmword ptr [esp],xmm7
 push dword ptr [esp+24]
 push dword ptr [esp+24]
 push ecx
 call ResearchAllocate
 add esp,12
 movups xmm7,xmmword ptr [esp]
 add esp,16
 ret 8
}}
static bool Read(HANDLE f,void* data,unsigned int bytes){DWORD done=0;return ReadFile(f,data,bytes,&done,0)&&done==bytes;}
static bool Write(HANDLE f,const void* data,unsigned int bytes){DWORD done=0;return WriteFile(f,data,bytes,&done,0)&&done==bytes;}
extern "C" void HarnessEntry(){
 char input[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_NATIVE_HULL_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_NATIVE_HULL_OUTPUT",output,MAX_PATH))ExitProcess(1);HANDLE in=CreateFileA(input,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);if(in==INVALID_HANDLE_VALUE||GetFileSize(in,0)>1024u*1024u)ExitProcess(2);unsigned int count=0,n=0;if(!Read(in,&count,4)||count!=1||!Read(in,&n,4)||n<4||n>4096)ExitProcess(3);PrefabScene::Arena memory;HC::Vec3* points=(HC::Vec3*)memory.Allocate(n*12);if(!points||!Read(in,points,n*12))ExitProcess(3);CloseHandle(in);
 char limit[16];if(GetEnvironmentVariableA("FEAR_HULL_RESEARCH_CALL_LIMIT",limit,16)){if(TextEqual(limit,"200000"))researchCallLimit=200000;else if(TextEqual(limit,"1000000"))researchCallLimit=1000000;else ExitProcess(3);}
 void* workspace=VirtualAlloc(0,HC::WorkspaceBytes,MEM_RESERVE|MEM_COMMIT,4);if(!workspace)ExitProcess(4);HC::Context context;HC::Result result;HC::Status status;
 if(researchCallLimit==100000)status=HC::Observe(points,n,workspace,HC::WorkspaceBytes,&context,&result);
 else {memset(&context,0,sizeof(context));HC::Clear(&result);const void* table[12];for(unsigned int i=0;i<12;i++)table[i]=HC::detail::Callbacks[i];table[0]=table[4]=(const void*)ResearchAllocateCallback;context.table=table;context.stackCurrent=(unsigned char*)workspace;context.stackRemaining=HC::ScratchBytes;context.memory=(unsigned char*)workspace+HC::ScratchBytes;context.memoryBytes=HC::WorkspaceBytes-HC::ScratchBytes;HC::Descriptor descriptor={points,(int)n,12};HC::detail::Invoke(&context,&descriptor,&result.geometry,&result.planes);status=(HC::Status)context.status;if(status!=HC::Success)HC::Clear(&result);else {result.validated=context.validated!=0;result.attempts=context.validationCalls;}}
 unsigned int fields[12]={(unsigned int)status,context.status,context.used,context.memoryBytes,context.allocationCalls,context.allocationBytes,context.stackRemaining,context.validated,context.validationCalls,(unsigned int)result.geometry.vertices.size,(unsigned int)result.geometry.triangles.size,(unsigned int)result.planes.size};HANDLE out=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(out==INVALID_HANDLE_VALUE||!Write(out,fields,sizeof(fields)))ExitProcess(5);CloseHandle(out);
 char geometryPath[MAX_PATH];if(status==HC::Success&&GetEnvironmentVariableA("FEAR_NATIVE_HULL_GEOMETRY",geometryPath,MAX_PATH)){if(!HC::OwnedArray(result.geometry.vertices,context,4096)||!HC::OwnedArray(result.geometry.triangles,context,8192)||!HC::OwnedArray(result.planes,context,8192))ExitProcess(6);HANDLE graph=CreateFileA(geometryPath,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);unsigned int h[6]={1,(unsigned int)result.geometry.vertices.size,(unsigned int)result.geometry.triangles.size,(unsigned int)result.planes.size,context.allocationCalls,context.allocationBytes};if(graph==INVALID_HANDLE_VALUE||!Write(graph,h,24))ExitProcess(5);for(int i=0;i<result.geometry.vertices.size;i++)if(!Write(graph,result.geometry.vertices.data+i,12))ExitProcess(5);if(!Write(graph,result.geometry.triangles.data,result.geometry.triangles.size*12)||!Write(graph,result.planes.data,result.planes.size*16))ExitProcess(5);CloseHandle(graph);}
 VirtualFree(workspace,0,MEM_RELEASE);ExitProcess(0);
}
