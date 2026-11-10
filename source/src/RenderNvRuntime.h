#pragma once
// Single translation unit runtime for the source-backed stripifier. The regular
// partial packer keeps its existing placement-only allocation configuration.
// Faces and edges are tiny allocations. A separate VirtualAlloc reservation for
// each one wastes a 64 KiB address range and exhausts the 32-bit address space.
// HeapAlloc retains the old zero-filled storage without changing strip order.
extern "C" {
__declspec(dllimport) void* __stdcall GetProcessHeap();
__declspec(dllimport) void* __stdcall HeapAlloc(void*,unsigned int,unsigned int);
__declspec(dllimport) int __stdcall HeapFree(void*,unsigned int,void*);
}
#pragma comment(linker,"/alternatename:__imp__GetProcessHeap@0=__imp__GetProcessHeap")
#pragma comment(linker,"/alternatename:__imp__HeapAlloc@12=__imp__HeapAlloc")
#pragma comment(linker,"/alternatename:__imp__HeapFree@12=__imp__HeapFree")
#ifdef FEAR_LIBRARY_ONLY
// Inside a host process (winpacker) exhaustion must not end the host: raise a
// structured exception that the guarded compile call converts into an error.
extern "C" __declspec(dllimport) void __stdcall RaiseException(unsigned long,unsigned long,unsigned long,const unsigned long*);
#pragma comment(linker,"/alternatename:__imp__RaiseException@16=__imp__RaiseException")
static const unsigned long OutOfMemoryException=0xE04F4F4Du; // 'OOM'
void* operator new(unsigned int bytes){void* p=HeapAlloc(GetProcessHeap(),8,bytes?bytes:1);if(!p)RaiseException(OutOfMemoryException,1,0,0);return p;}
#else
void* operator new(unsigned int bytes){void* p=HeapAlloc(GetProcessHeap(),8,bytes?bytes:1);if(!p)ExitProcess(20);return p;}
#endif
void* operator new[](unsigned int bytes){return operator new(bytes);}
void operator delete(void* p){if(p)HeapFree(GetProcessHeap(),0,p);}
void operator delete[](void* p){operator delete(p);}
void operator delete(void* p,unsigned int){operator delete(p);}
void operator delete[](void* p,unsigned int){operator delete(p);}
