#pragma once
#include "ShaderDependencyPaths.h"
// Source equivalent of original CIncludeResolver (0x7CCA0, 0x7CD00, 0x7CE20).
// ID3DXInclude has two stdcall methods and deliberately no IUnknown methods.
namespace ShaderEffectInclude {
class Resolver {
 struct Allocation {void* data;};
 char directories[128][264];Allocation allocations[128];unsigned int allocationCount;
 Resolver(const Resolver&);Resolver& operator=(const Resolver&);
public:
 unsigned int depth,opens,closes,maximumDepth;const char* error;bool unsupported;
 Resolver(const char* root):allocationCount(0),depth(0),opens(0),closes(0),maximumDepth(0),error(0),unsupported(false){
  if(ShaderDependencyPaths::Directory(root,directories[0],261)){depth=maximumDepth=1;}else{error="Invalid include root";unsupported=true;}
 }
 // Type and parentData do not participate in the original lookup. The last
 // open directory governs both local and system includes; Close pops it.
 virtual long WINAPI Open(unsigned int,const char* filename,const void*,const void** data,unsigned int* size){
  if(!filename||!data||!size||!depth){error="Invalid include arguments";return (long)0x80070057u;}
  if(depth>=128||allocationCount>=128){error="Include nesting limit";unsupported=true;return (long)0x8007000eu;}
  char path[264],directory[264];if(!ShaderDependencyPaths::Resolve(directories[depth-1],filename,path,261)||!ShaderDependencyPaths::Directory(path,directory,261)){error="Oversized include path";unsupported=true;return (long)0x80070057u;}
  HANDLE file=CreateFileA(path,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);if(file==INVALID_HANDLE_VALUE){error="Include file unavailable";return (long)0x80070057u;}
  DWORD high=0,bytes=GetFileSize(file,&high);if(high||bytes>16*1024*1024){CloseHandle(file);error="Unsupported include size";unsupported=true;return (long)0x80070057u;}
  void* buffer=VirtualAlloc(0,bytes?bytes:1,MEM_COMMIT|MEM_RESERVE,4);if(!buffer){CloseHandle(file);error="Include allocation failed";unsupported=true;return (long)0x8007000eu;}
  DWORD done=0;bool read=!bytes||ReadFile(file,buffer,bytes,&done,0)&&done==bytes;CloseHandle(file);if(!read){VirtualFree(buffer,0,MEM_RELEASE);error="Incomplete include read";unsupported=true;return (long)0x80070057u;}
  allocations[allocationCount++].data=buffer;lstrcpyA(directories[depth++],directory);if(depth>maximumDepth)maximumDepth=depth;opens++;*data=buffer;*size=bytes;return 0;
 }
 virtual long WINAPI Close(const void* data){
  if(!depth){error="Invalid include close";return (long)0x80070057u;}
  for(unsigned int i=0;i<allocationCount;i++)if(allocations[i].data==data){VirtualFree(allocations[i].data,0,MEM_RELEASE);allocations[i]=allocations[--allocationCount];break;}
  // Original Close frees the supplied pointer and pops the latest directory,
  // independent of that pointer's Open. Valid D3DX calls are properly nested.
  depth--;closes++;return 0;
 }
 const char* CurrentDirectory()const{return depth?directories[depth-1]:0;}
 ~Resolver(){for(unsigned int i=0;i<allocationCount;i++)VirtualFree(allocations[i].data,0,MEM_RELEASE);}
};
}
