#pragma once
typedef unsigned int uint32;typedef int int32;
struct LTVector2 {float x,y;};struct LTVector {float x,y,z;};struct LTVector4 {float x,y,z,w;};
#include "E:/fearsdk/Source/engine/sdk/inc/idatabasemgr.h"
// SDK ILTInStream ABI, memory-backed read-only implementation.
class ILTInStream {
public:
 virtual void Release()=0;virtual unsigned int Read(void*,unsigned int)=0;
 virtual unsigned int ReadString(char*,unsigned int)=0;virtual bool HasErrorOccurred()=0;
 virtual bool CanSeek()=0;virtual unsigned int SeekTo(unsigned __int64)=0;
 virtual unsigned __int64 GetPos()=0;virtual unsigned __int64 GetLen()=0;
protected:virtual ~ILTInStream(){}
};
class MemoryStream:public ILTInStream {
 const unsigned char* bytes;unsigned int size,pos;bool error;
public:
 MemoryStream(const unsigned char* b,unsigned int n):bytes(b),size(n),pos(0),error(false){}
 void Release(){}unsigned int Read(void* out,unsigned int n){if(n>size-pos){for(unsigned int i=0;i<n;i++)((unsigned char*)out)[i]=0;error=true;return 1;}for(unsigned int i=0;i<n;i++)((unsigned char*)out)[i]=bytes[pos++];return 0;}
 unsigned int ReadString(char* out,unsigned int max){unsigned short n=0;if(Read(&n,2)||!max||n>=max){error=true;return 1;}if(Read(out,n))return 1;out[n]=0;return 0;}
 bool HasErrorOccurred(){return error;}bool CanSeek(){return true;}
 unsigned int SeekTo(unsigned __int64 offset){if(offset>size){error=true;return 1;}pos=(unsigned int)offset;return 0;}
 unsigned __int64 GetPos(){return pos;}unsigned __int64 GetLen(){return size;}
};

extern "C" __declspec(dllimport) void* WINAPI LoadLibraryA(const char*);
extern "C" __declspec(dllimport) void* WINAPI GetProcAddress(void*,const char*);

#pragma comment(linker,"/alternatename:__imp__LoadLibraryA@4=__imp__LoadLibraryA")
#pragma comment(linker,"/alternatename:__imp__GetProcAddress@8=__imp__GetProcAddress")
