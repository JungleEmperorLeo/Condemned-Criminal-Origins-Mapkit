#include "WorldPacker.cpp"
#include "SourcePhysicsHullPreprocess.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
namespace HP=SourcePhysicsHullPreprocess;
static bool Read(HANDLE f,void* p,unsigned int n){DWORD done=0;return !n||(ReadFile(f,p,n,&done,0)&&done==n);}
static bool Write(HANDLE f,const void* p,unsigned int n){DWORD done=0;return !n||(WriteFile(f,p,n,&done,0)&&done==n);}
struct Packet {
 unsigned char* data;unsigned int size,at;
 bool Take(void* out,unsigned int n){if(n>size-at)return false;memcpy(out,data+at,n);at+=n;return true;}
 bool Put(const void* in,unsigned int n){if(n>size-at)return false;memcpy(data+at,in,n);at+=n;return true;}
};
extern "C" void HarnessEntry(){
 char input[MAX_PATH],output[MAX_PATH];
 if(!GetEnvironmentVariableA("FEAR_HULL_PREPROCESS_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_HULL_PREPROCESS_OUTPUT",output,MAX_PATH))ExitProcess(1);
 HANDLE source=CreateFileA(input,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);
 if(source==INVALID_HANDLE_VALUE)ExitProcess(2);
 const unsigned int length=GetFileSize(source,0);
 if(length<4||length>8u*1024u*1024u)ExitProcess(2);
 unsigned char* bytes=(unsigned char*)VirtualAlloc(0,length,MEM_COMMIT|MEM_RESERVE,4);
 unsigned char* result=(unsigned char*)VirtualAlloc(0,32u*1024u*1024u,MEM_COMMIT|MEM_RESERVE,4);
 HP::Vec4* scratch=(HP::Vec4*)VirtualAlloc(0,128*48,MEM_COMMIT|MEM_RESERVE,4);
 if(!bytes||!result||!scratch||!Read(source,bytes,length))ExitProcess(3);CloseHandle(source);
 Packet in={bytes,length,0},out={result,32u*1024u*1024u,0};unsigned int count=0;
 if(!in.Take(&count,4)||!count||count>2000||!out.Put(&count,4))ExitProcess(4);
 for(unsigned int c=0;c<count;++c){
  unsigned int n=0;float threshold=0;HP::Vec4* points=scratch;HP::Vec4* normalized=scratch+128;HP::Vec4* sorted=scratch+256;
  if(!in.Take(&n,4)||!n||n>128||!in.Take(&threshold,4)||!HP::Finite(threshold)||threshold<0||!in.Take(points,n*16))ExitProcess(4);
  HP::Array a={points,static_cast<int>(n),128};HP::Vec4 span={},center={};int retained=0;
  if(HP::Normalize(&a,&span,&center)!=HP::Success)ExitProcess(6);
  memcpy(normalized,points,n*16);
  if(HP::Sort(&a)!=HP::Success)ExitProcess(6);memcpy(sorted,points,n*16);
  if(HP::Deduplicate(&a,threshold,&retained)!=HP::Success)ExitProcess(6);
  unsigned int header[5]={n,static_cast<unsigned int>(a.size),static_cast<unsigned int>(retained),0,0};
  if(!out.Put(header,20)||!out.Put(&span,16)||!out.Put(&center,16)||!out.Put(normalized,n*16)||!out.Put(sorted,n*16)||!out.Put(points,a.size*16))ExitProcess(7);
  if(HP::Unnormalize(&a,span,center)!=HP::Success||!out.Put(points,a.size*16))ExitProcess(6);
 }
 if(in.at!=in.size)ExitProcess(4);
 HANDLE target=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);
 if(target==INVALID_HANDLE_VALUE||!Write(target,result,out.at))ExitProcess(8);
 CloseHandle(target);VirtualFree(bytes,0,MEM_RELEASE);VirtualFree(result,0,MEM_RELEASE);VirtualFree(scratch,0,MEM_RELEASE);ExitProcess(0);
}
