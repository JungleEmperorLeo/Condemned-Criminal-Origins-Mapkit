// Headless host for the in-process world compiler (the monolithic DLL core).
// Usage: native-monolith-host --source PATH --project PATH --world-stem STEM --mopp DLL --output WORLD00P
#define FEAR_PRIMARY_SURFACE_PREPARATION 1
#define FEAR_NATIVE_NV_RUNTIME 1
#define FEAR_RENDER_INDEX_REORDER 1
#define FEAR_SOURCE_GEOMETRY_REPAIR_DEFAULT 1
#include "WorldPacker.cpp"
extern "C" {
__declspec(dllimport) void WINAPI ExitProcess(unsigned int);
__declspec(dllimport) unsigned int WINAPI SetErrorMode(unsigned int);
__declspec(dllimport) char* WINAPI GetCommandLineA();
__declspec(dllimport) HANDLE WINAPI GetStdHandle(unsigned int);
__declspec(dllimport) unsigned int WINAPI GetCurrentProcessId();
__declspec(dllimport) int WINAPI MoveFileExA(const char*,const char*,unsigned int);
__declspec(dllimport) int WINAPI DeleteFileA(const char*);
__declspec(dllimport) int WINAPI MultiByteToWideChar(unsigned int,unsigned int,const char*,int,unsigned short*,int);
__declspec(dllimport) int WINAPI CompareStringOrdinal(const unsigned short*,int,const unsigned short*,int,BOOL);
__declspec(dllimport) BOOL WINAPI CreateDirectoryA(const char*,void*);
__declspec(dllimport) char* WINAPI lstrcatA(char*,const char*);
}
#pragma comment(linker,"/alternatename:__imp__CreateDirectoryA@8=__imp__CreateDirectoryA")
#pragma comment(linker,"/alternatename:__imp__lstrcatA@8=__imp__lstrcatA")
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
#pragma comment(linker,"/alternatename:__imp__SetErrorMode@4=__imp__SetErrorMode")
#pragma comment(linker,"/alternatename:__imp__GetCommandLineA@0=__imp__GetCommandLineA")
#pragma comment(linker,"/alternatename:__imp__GetStdHandle@4=__imp__GetStdHandle")
#pragma comment(linker,"/alternatename:__imp__GetCurrentProcessId@0=__imp__GetCurrentProcessId")
#pragma comment(linker,"/alternatename:__imp__MoveFileExA@12=__imp__MoveFileExA")
#pragma comment(linker,"/alternatename:__imp__DeleteFileA@4=__imp__DeleteFileA")
#pragma comment(linker,"/alternatename:__imp__MultiByteToWideChar@24=__imp__MultiByteToWideChar")
#pragma comment(linker,"/alternatename:__imp__CompareStringOrdinal@20=__imp__CompareStringOrdinal")
#pragma comment(linker,"/alternatename:__imp__LoadLibraryExA@12=__imp__LoadLibraryExA")
#pragma comment(linker,"/alternatename:__imp__FreeLibrary@4=__imp__FreeLibrary")
#pragma comment(linker,"/alternatename:__imp__VirtualQuery@12=__imp__VirtualQuery")
#include "NativeWorldCompiler.h"
namespace MonolithHost {
static bool Token(const char*& at,char out[260]){while(*at==' '||*at=='\t')++at;if(!*at)return false;bool quoted=false;unsigned int n=0;while(*at&&(quoted||(*at!=' '&&*at!='\t'))){if(*at=='"'){quoted=!quoted;++at;continue;}if(n==259)return false;out[n++]=*at++;}out[n]=0;return !quoted&&n!=0;}
static void Console(const char* text){DWORD done;HANDLE h=GetStdHandle((unsigned int)-11);WriteFile(h,text,lstrlenA(text),&done,0);WriteFile(h,"\r\n",2,&done,0);}
static void Stage(void*,const char* name){char line[128];lstrcpyA(line,"stage ");lstrcpynA(line+6,name,100);Console(line);}
}
extern "C" void MonolithHostEntry(){
 SetErrorMode(3);using namespace MonolithHost;char source[260]={},project[260]={},stem[260]={},mopp[260]={},output[260]={},repeat[16]={},split[16]={},post[16]={},key[260],value[260];const char* command=GetCommandLineA();if(!Token(command,key))ExitProcess(1);
 while(*command){while(*command==' '||*command=='\t')++command;if(!*command)break;if(!Token(command,key)||!Token(command,value))ExitProcess(1);char* target=0;
  if(FearBrushFlags::Equal(key,"--source"))target=source;else if(FearBrushFlags::Equal(key,"--project"))target=project;else if(FearBrushFlags::Equal(key,"--world-stem"))target=stem;else if(FearBrushFlags::Equal(key,"--mopp"))target=mopp;else if(FearBrushFlags::Equal(key,"--output"))target=output;else if(FearBrushFlags::Equal(key,"--repeat"))target=repeat;else if(FearBrushFlags::Equal(key,"--split-meshes"))target=split;else if(FearBrushFlags::Equal(key,"--post-profile"))target=post;else ExitProcess(1);if((target==repeat||target==split||target==post)&&lstrlenA(value)>=16)ExitProcess(1);lstrcpyA(target,value);}
 // --mopp is accepted and ignored (the MOPP is compiled natively).
 if(!source[0]||!project[0]||!stem[0]||!output[0]){Console("Usage: native-monolith-host --source PATH --project PATH --world-stem STEM --output WORLD00P");ExitProcess(1);}
 PrefabScene::Arena storage;void* place=storage.Allocate(sizeof(NativeWorldCompiler::Compiler));if(!place)ExitProcess(2);NativeWorldCompiler::Compiler* compiler=new(place)NativeWorldCompiler::Compiler;
 NativeWorldCompiler::Request request;request.source=source;request.project=project;request.stem=stem;request.splitMeshes=FearBrushFlags::Equal(split,"true");request.postProfile=post[0]?post:"auto";request.stage=Stage;
 if(!compiler->Compile(request)){Console(compiler->error);ExitProcess(3);}
 // --repeat N: compile again in the same process (state isolation across runs,
 // as in one winpacker session); every result must equal the first byte for byte.
 unsigned int runs=0;for(const char* p=repeat;*p>='0'&&*p<='9';p++)runs=runs*10+(unsigned int)(*p-'0');
 if(runs>1){unsigned int size=compiler->size;unsigned char* first=(unsigned char*)storage.Allocate(size);if(!first)ExitProcess(2);memcpy(first,compiler->world,size);
  for(unsigned int r=1;r<runs;r++){if(!compiler->Compile(request)){Console(compiler->error);ExitProcess(3);}bool same=compiler->size==size;for(unsigned int i=0;same&&i<size;i++)same=compiler->world[i]==first[i];Console(same?"repeat: identical":"repeat: DIFFERENT");if(!same)ExitProcess(6);}}
 // External meshes go below the folder of the world file, by Game-relative path.
 char folder[260];lstrcpyA(folder,output);{char* last=0;for(char* p=folder;*p;p++)if(*p=='\\'||*p=='/')last=p;if(last)*last=0;}
 for(unsigned int m=0;m<compiler->meshCount;m++){char path[600];lstrcpyA(path,folder);lstrcatA(path,"\\");lstrcatA(path,compiler->meshes[m].name);for(char* p=path;*p;p++)if(*p=='/')*p='\\';
  for(char* p=path+lstrlenA(folder)+1;*p;p++)if(*p=='\\'){*p=0;CreateDirectoryA(path,0);*p='\\';}
  HANDLE g=CreateFileA(path,0x40000000,0,0,2,FILE_ATTRIBUTE_NORMAL,0);DWORD written=0;if(g==INVALID_HANDLE_VALUE||!WriteFile(g,compiler->meshes[m].data,compiler->meshes[m].size,&written,0)||written!=compiler->meshes[m].size)ExitProcess(4);CloseHandle(g);}
 {char line[64];lstrcpyA(line,"post-profile ");lstrcatA(line,compiler->postProfile);Console(line);lstrcpyA(line,"external meshes ");*Decimal(line+16,compiler->meshCount)=0;Console(line);}
 HANDLE f=CreateFileA(output,0x40000000,0,0,2,FILE_ATTRIBUTE_NORMAL,0);DWORD done=0;if(f==INVALID_HANDLE_VALUE||!WriteFile(f,compiler->world,compiler->size,&done,0)||done!=compiler->size)ExitProcess(4);CloseHandle(f);
 Console("Native monolithic world compiled");compiler->~Compiler();ExitProcess(0);
}
int main(){MonolithHostEntry();return 0;}
