#pragma once
#define WINAPI __stdcall
#define MAX_PATH 260
#define FILE_APPEND_DATA 4
#define GENERIC_READ 0x80000000
#define FILE_SHARE_READ 1
#define OPEN_EXISTING 3
#define OPEN_ALWAYS 4
#define FILE_ATTRIBUTE_NORMAL 128
#define MEM_COMMIT 0x1000
#define MEM_RESERVE 0x2000
#define MEM_RELEASE 0x8000
#define DLL_PROCESS_ATTACH 1
#define TRUE 1
typedef void* HANDLE; typedef void* HMODULE; typedef void* HINSTANCE; typedef void* LPVOID;
typedef unsigned long DWORD; typedef int BOOL;
struct FindDataA {DWORD fields[11];char name[260];char alternate[14];};
#define INVALID_HANDLE_VALUE ((HANDLE)-1)
extern "C" {
__declspec(dllimport) DWORD WINAPI GetModuleFileNameA(HMODULE,char*,DWORD);
__declspec(dllimport) char* WINAPI lstrcpyA(char*,const char*);
__declspec(dllimport) char* WINAPI lstrcpynA(char*,const char*,int);
__declspec(dllimport) int WINAPI lstrlenA(const char*);
__declspec(dllimport) HANDLE WINAPI CreateFileA(const char*,DWORD,DWORD,void*,DWORD,DWORD,HANDLE);
__declspec(dllimport) BOOL WINAPI WriteFile(HANDLE,const void*,DWORD,DWORD*,void*);
__declspec(dllimport) BOOL WINAPI ReadFile(HANDLE,void*,DWORD,DWORD*,void*);
__declspec(dllimport) DWORD WINAPI GetFileSize(HANDLE,DWORD*);
__declspec(dllimport) void* WINAPI VirtualAlloc(void*,unsigned int,DWORD,DWORD);
__declspec(dllimport) BOOL WINAPI VirtualFree(void*,unsigned int,DWORD);
__declspec(dllimport) BOOL WINAPI CloseHandle(HANDLE);
__declspec(dllimport) DWORD WINAPI GetEnvironmentVariableA(const char*,char*,DWORD);
__declspec(dllimport) DWORD WINAPI GetFullPathNameA(const char*,DWORD,char*,char**);
__declspec(dllimport) HANDLE WINAPI FindFirstFileA(const char*,FindDataA*);
__declspec(dllimport) BOOL WINAPI FindNextFileA(HANDLE,FindDataA*);
__declspec(dllimport) BOOL WINAPI FindClose(HANDLE);
__declspec(dllimport) DWORD WINAPI GetPrivateProfileStringA(const char*,const char*,const char*,char*,DWORD,const char*);
}
#pragma comment(linker,"/alternatename:__imp__GetFullPathNameA@16=__imp__GetFullPathNameA")
#pragma comment(linker,"/alternatename:__imp__FindFirstFileA@8=__imp__FindFirstFileA")
#pragma comment(linker,"/alternatename:__imp__FindNextFileA@8=__imp__FindNextFileA")
#pragma comment(linker,"/alternatename:__imp__FindClose@4=__imp__FindClose")
#pragma comment(linker,"/alternatename:__imp__GetPrivateProfileStringA@24=__imp__GetPrivateProfileStringA")
#pragma comment(linker,"/alternatename:__imp__GetEnvironmentVariableA@12=__imp__GetEnvironmentVariableA")
inline void* operator new(unsigned int,void* p){return p;}
#ifndef FEAR_NATIVE_NV_RUNTIME
void operator delete(void*){}
#endif

