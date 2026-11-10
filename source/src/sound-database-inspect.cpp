#include "WorldPacker.cpp"
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
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
extern "C" __declspec(dllimport) void* WINAPI LoadLibraryA(const char*);
extern "C" __declspec(dllimport) void* WINAPI GetProcAddress(void*,const char*);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
#pragma comment(linker,"/alternatename:__imp__LoadLibraryA@4=__imp__LoadLibraryA")
#pragma comment(linker,"/alternatename:__imp__GetProcAddress@8=__imp__GetProcAddress")
static HANDLE output;
static void U(unsigned int n){DWORD done;if(!WriteFile(output,&n,4,&done,0)||done!=4)ExitProcess(9);}
static void S(const char* s){if(!s)s="";unsigned int n=lstrlenA(s)+1;DWORD done;if(n>4097||!WriteFile(output,s,n,&done,0)||done!=n)ExitProcess(9);}
extern "C" void HarnessEntry(){
 LoadLibraryA("E:\\fearsdk\\Dev\\Tools\\msvcr71.dll");LoadLibraryA("E:\\fearsdk\\Dev\\Tools\\msvcp71.dll");
 void* dll=LoadLibraryA("E:\\fearsdk\\Dev\\Runtime\\GameDatabase.dll");if(!dll)ExitProcess(2);
 typedef IDatabaseMgr* (*Factory)();Factory factory=(Factory)GetProcAddress(dll,"GetIDatabaseMgr");if(!factory)ExitProcess(3);IDatabaseMgr* mgr=factory();if(!mgr)ExitProcess(3);
 const char* path="E:\\fearsdk\\Dev\\Runtime\\Game\\Database\\FEAR.Gamdb00p";
 HANDLE file=CreateFileA(path,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);if(file==INVALID_HANDLE_VALUE)ExitProcess(4);unsigned int size=GetFileSize(file,0);if(size>64*1024*1024)ExitProcess(4);unsigned char* bytes=(unsigned char*)VirtualAlloc(0,size,MEM_COMMIT|MEM_RESERVE,4);DWORD done;if(!bytes||!ReadFile(file,bytes,size,&done,0)||done!=size)ExitProcess(4);CloseHandle(file);
 MemoryStream stream(bytes,size);HDATABASE db=mgr->OpenNewDatabase(path,stream);if(!db||stream.HasErrorOccurred())ExitProcess(5);char categoryName[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_DATABASE_CATEGORY",categoryName,MAX_PATH))lstrcpyA(categoryName,"Sound/Sound");HCATEGORY category=mgr->GetCategory(db,categoryName);if(!category)ExitProcess(6);
 char target[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_SOUND_DATABASE_OUTPUT",target,MAX_PATH))ExitProcess(7);output=CreateFileA(target,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(output==INVALID_HANDLE_VALUE)ExitProcess(8);
 unsigned int count=mgr->GetNumRecords(category);if(count>100000)ExitProcess(10);U(count);
 for(unsigned int r=0;r<count;r++){HRECORD rec=mgr->GetRecordByIndex(category,r);if(!rec)ExitProcess(11);S(mgr->GetRecordName(rec));unsigned int attributes=mgr->GetNumAttributes(rec);if(attributes>4096)ExitProcess(10);U(attributes);
  for(unsigned int a=0;a<attributes;a++){HATTRIBUTE attr=mgr->GetAttributeByIndex(rec,a);unsigned int type=mgr->GetAttributeType(attr),usage=mgr->GetAttributeUsage(attr);S(mgr->GetAttributeName(attr));U(type);U(usage);unsigned int n=(type==eAttributeType_String||type==eAttributeType_RecordLink)?mgr->GetNumValues(attr):0;if(n>100000)ExitProcess(10);U(n);for(unsigned int i=0;i<n;i++)S(type==eAttributeType_RecordLink?mgr->GetRecordName(mgr->GetRecordLink(attr,i,0)):mgr->GetString(attr,i,""));}
 }
 CloseHandle(output);mgr->ReleaseDatabase(db);VirtualFree(bytes,0,MEM_RELEASE);ExitProcess(0);
}
