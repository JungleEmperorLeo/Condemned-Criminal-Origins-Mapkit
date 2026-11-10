// Monolithic Winpacker WorldPacker.dll: compiles the source world in process
// (NativeWorldCompiler.h). No Python, no worker EXEs, no external binaries: the MOPP
// is compiled by the recovered original builder (NativeMoppCompiler.h). Failures and
// exceptions are reported in the log, not fatal.
#define FEAR_PRIMARY_SURFACE_PREPARATION 1
#define FEAR_NATIVE_NV_RUNTIME 1
#define FEAR_RENDER_INDEX_REORDER 1
#define FEAR_SOURCE_GEOMETRY_REPAIR_DEFAULT 1
#define FEAR_LIBRARY_ONLY 1
#include "WorldPacker.cpp"
extern "C" {
__declspec(dllimport) void WINAPI ExitProcess(unsigned int);
__declspec(dllimport) HANDLE WINAPI GetStdHandle(unsigned int);
__declspec(dllimport) unsigned int WINAPI GetCurrentProcessId();
__declspec(dllimport) int WINAPI MoveFileExA(const char*,const char*,unsigned int);
__declspec(dllimport) int WINAPI DeleteFileA(const char*);
__declspec(dllimport) int WINAPI MultiByteToWideChar(unsigned int,unsigned int,const char*,int,unsigned short*,int);
__declspec(dllimport) int WINAPI CompareStringOrdinal(const unsigned short*,int,const unsigned short*,int,BOOL);
__declspec(dllimport) BOOL WINAPI CreateDirectoryA(const char*,void*);
__declspec(dllimport) DWORD WINAPI GetFullPathNameA(const char*,DWORD,char*,char**);
}
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
#pragma comment(linker,"/alternatename:__imp__GetStdHandle@4=__imp__GetStdHandle")
#pragma comment(linker,"/alternatename:__imp__GetCurrentProcessId@0=__imp__GetCurrentProcessId")
#pragma comment(linker,"/alternatename:__imp__MoveFileExA@12=__imp__MoveFileExA")
#pragma comment(linker,"/alternatename:__imp__DeleteFileA@4=__imp__DeleteFileA")
#pragma comment(linker,"/alternatename:__imp__MultiByteToWideChar@24=__imp__MultiByteToWideChar")
#pragma comment(linker,"/alternatename:__imp__CompareStringOrdinal@20=__imp__CompareStringOrdinal")
#pragma comment(linker,"/alternatename:__imp__LoadLibraryExA@12=__imp__LoadLibraryExA")
#pragma comment(linker,"/alternatename:__imp__FreeLibrary@4=__imp__FreeLibrary")
#pragma comment(linker,"/alternatename:__imp__VirtualQuery@12=__imp__VirtualQuery")
#pragma comment(linker,"/alternatename:__imp__CreateDirectoryA@8=__imp__CreateDirectoryA")
#pragma comment(linker,"/alternatename:__imp__GetFullPathNameA@16=__imp__GetFullPathNameA")
#include "WinPackerOriginalOptions.h"
#include "NativeWorldCompiler.h"
#include "CondemnedContainer.h"
namespace MonolithDll {
static bool Append(char* target,unsigned int capacity,const char* text){unsigned int n=lstrlenA(target),m=lstrlenA(text);if(n>=capacity||m>=capacity-n)return false;for(unsigned int i=0;i<=m;i++)target[n+i]=text[i];return true;}
static bool Path(const char* base,const char* relative,char* target){char path[2*MAX_PATH];if(!relative||!relative[0])return false;if(relative[0]=='\\'||relative[0]=='/'||relative[1]==':'){if(lstrlenA(relative)>=MAX_PATH)return false;lstrcpyA(path,relative);}else{lstrcpyA(path,base);if(!Append(path,sizeof(path),"\\")||!Append(path,sizeof(path),relative))return false;}DWORD n=GetFullPathNameA(path,MAX_PATH,target,0);return n&&n<MAX_PATH;}
static void Parent(char* path){char* last=0;for(char* p=path;*p;p++)if(*p=='\\'||*p=='/')last=p;if(last)*last=0;else path[0]=0;}
static bool ModuleFolder(char* folder){DWORD n=GetModuleFileNameA(moduleHandle,folder,MAX_PATH);if(!n||n>=MAX_PATH)return false;Parent(folder);return folder[0]!=0;}
static char Lower(char c){return c>='A'&&c<='Z'?(char)(c+32):c;}
// Game-relative stem: the source below the project's ResourceRoot without its extension.
static bool Stem(const char* source,const char* resources,char* stem){
 unsigned int n=lstrlenA(resources);while(n&&(resources[n-1]=='\\'||resources[n-1]=='/'))n--;for(unsigned int i=0;i<n;i++)if(Lower(source[i])!=Lower(resources[i]))return false;if(source[n]!='\\'&&source[n]!='/')return false;
 lstrcpynA(stem,source+n+1,MAX_PATH);unsigned int dot=0,length=lstrlenA(stem);for(unsigned int i=0;i<length;i++){if(stem[i]=='.')dot=i;else if(stem[i]=='\\'||stem[i]=='/')dot=0;}if(dot)stem[dot]=0;return stem[0]!=0;
}
static void Log(IPackerOutput* output,EMsgType level,const char* text){Report(output,level,text);}
}
// Structured guard without destructible locals (C2712): out-of-memory from
// operator new, access violations in any stage or in SDK code end only
// this compilation. Arena memory of an aborted run may leak.
extern "C" unsigned long __cdecl _exception_code(void);
static int Filter(unsigned long code,unsigned long& fault){fault=code;return 1;}
static bool Guarded(NativeWorldCompiler::Compiler& compiler,const NativeWorldCompiler::Request& request,unsigned long& fault){
 __try{return compiler.Compile(request);}
 __except(Filter(_exception_code(),fault)){return false;}
}
struct SystemTime {unsigned short year,month,weekday,day,hour,minute,second,milliseconds;};
extern "C" __declspec(dllimport) void WINAPI GetLocalTime(SystemTime*);
#pragma comment(linker,"/alternatename:__imp__GetLocalTime@4=__imp__GetLocalTime")
namespace MonolithDll {
// 0x10054AB0: from the source folder upward, the first *.proj00 (FindFirstFile order).
static bool LocateProject(const char* source,char* project){
 char folder[MAX_PATH];lstrcpynA(folder,source,MAX_PATH);Parent(folder);
 while(folder[0]){char pattern[MAX_PATH];lstrcpyA(pattern,folder);if(!Append(pattern,MAX_PATH,"\\*.proj00"))return false;FindDataA found;HANDLE h=FindFirstFileA(pattern,&found);
  if(h!=INVALID_HANDLE_VALUE){FindClose(h);lstrcpyA(project,folder);return Append(project,MAX_PATH,"\\")&&Append(project,MAX_PATH,found.name);}
  unsigned int n=lstrlenA(folder);Parent(folder);if((unsigned int)lstrlenA(folder)==n)break;}
 return false;
}
static void TwoDigits(char* at,unsigned int v){at[0]=(char)('0'+v/10%10);at[1]=(char)('0'+v%10);}
}
// The original packer surface: name "World Packer", the original property set
// and PropertyChanged rules (WinPackerOriginalOptions.h), the project located
// above the source and the World00p written beside it, and the original log
// (PackerLog.h) for every stage of NativeWorldCompiler.
class MonolithWinPacker:public IPackerImpl {
 bool running;
public:
 MonolithWinPacker():running(false){}
 bool GetPackerName(char* name,int bytes){if(!name||bytes<1)return false;lstrcpynA(name,"World Packer",bytes);return true;}
 bool RequestUserOptions(IPackerUI* ui){return WinPackerOriginalOptions::Register(ui);}
 bool PropertyChanged(CPackerProperty*,CPackerPropList* properties,IPackerUI*){WinPackerOriginalOptions::Changed(properties);return true;}
 bool Process(const char* filename,CPackerPropList* properties,IPackerOutput* output){
  using namespace MonolithDll;if(running){Report(output,MSG_ERROR,"A compilation is already active");return false;}
  PackerLog::Begin(output);PackerLog::Task("Initializing");
  const WinPackerOriginalOptions::Settings options=WinPackerOriginalOptions::Read(properties);
  char source[MAX_PATH],project[MAX_PATH],stem[MAX_PATH],resources[MAX_PATH];
  if(!filename||!filename[0]||!PrefabScene::Full(filename,source)||!LocateProject(source,project)){PackerLog::Error("Unable to find a project locator file for the resource %s. Packing failed",filename?filename:"");PackerLog::End();return false;}
  const DWORD started=GetTickCount();
  SystemTime now;GetLocalTime(&now);char date[16],time[16];
  TwoDigits(date,now.month);date[2]='/';TwoDigits(date+3,now.day);date[5]='/';TwoDigits(date+6,now.year%100);date[8]=0;
  TwoDigits(time,now.hour);time[2]=':';TwoDigits(time+3,now.minute);time[5]=':';TwoDigits(time+6,now.second);time[8]=0;
  PackerLog::Normal("Processing %s",filename);PackerLog::Normal("");PackerLog::Normal("Date: %s, Time: %s",date,time);
  {PrefabScene::Project located;if(!located.LoadPath(project)||!Stem(source,located.resources,stem)){PackerLog::Error("The resource %s is outside the project ResourceRoot. Packing failed",filename);PackerLog::End();return false;}lstrcpynA(resources,located.resources,MAX_PATH);}
  char destination[MAX_PATH],world[MAX_PATH],objectFile[MAX_PATH];lstrcpyA(destination,source);Parent(destination);
  {const char* name=source;for(const char* p=source;*p;p++)if(*p=='\\'||*p=='/')name=p+1;char leaf[MAX_PATH];lstrcpynA(leaf,name,MAX_PATH);unsigned int dot=0;for(unsigned int i=0;leaf[i];i++)if(leaf[i]=='.')dot=i;if(dot)leaf[dot]=0;
   char objectLeaf[MAX_PATH];lstrcpyA(objectLeaf,leaf);
   if(!Append(objectLeaf,MAX_PATH,".Objects00p")||!PrefabScene::Join(destination,objectLeaf,objectFile)||!Append(leaf,MAX_PATH,".World00p")||!PrefabScene::Join(destination,leaf,world)||lstrlenA(world)+8>=MAX_PATH||lstrlenA(objectFile)+8>=MAX_PATH){PackerLog::Error("Output path exceeds MAX_PATH");PackerLog::End();return false;}}
  running=true;PrefabScene::Arena storage;
  NativeWorldCompiler::Request request;request.source=source;request.project=project;request.stem=stem;request.postProfile="condemned";request.splitMeshes=options.splitMeshes;
  request.outputWorld=world;request.outputFolder=resources;request.startTicks=started;request.outputObjects=objectFile;
  request.centerWorld=options.centerWorld;request.ignoreHidden=options.ignoreHidden;request.ignoreFrozen=options.ignoreFrozen;request.optimizeVertices=options.optimizeVertices;request.rawNavMesh=options.rawNavMesh;/* CONDEMNED FORK: objects live in WorldServer00p and blind data in World00p, so ObjectsOnly/CreateObjectFile rebuild the whole world (below) instead of patching a FEAR single file */ request.objectsOnly=false;request.createObjectFile=false;if(options.objectsOnly||options.createObjectFile)PackerLog::Warning("Condemned layout: ObjectsOnly/CreateObjectFile rebuild the whole world%s",options.createObjectFile?"; only WorldServer00p (objects) is written":"");
  void* place=storage.Allocate(sizeof(NativeWorldCompiler::Compiler));NativeWorldCompiler::Compiler* compiler=place?new(place)NativeWorldCompiler::Compiler:0;
  unsigned long fault=0;bool good=compiler&&Guarded(*compiler,request,fault);
  if(fault)PackerLog::Error(fault==OutOfMemoryException?"Compilation ran out of memory (exception 0x%08x)":"Compilation stopped by an exception (0x%08x)",fault);
  else if(!good&&!(compiler&&compiler->reported))PackerLog::Error("%s",compiler?compiler->error:"Could not allocate the compiler");
  // compile_world publish_result: Game-relative meshes (below ResourceRoot, as
  // the original writes them) before the world that references them; each file
  // is replaced atomically (the set is not transactional).
  for(unsigned int m=0;good&&m<compiler->meshCount;m++){char target[MAX_PATH],temporary[MAX_PATH];const char* name=compiler->meshes[m].name;
   if(lstrlenA(resources)+1+lstrlenA(name)+8>=MAX_PATH){PackerLog::Error("External mesh output path exceeds MAX_PATH");good=false;break;}
   lstrcpyA(target,resources);if(target[0]&&target[lstrlenA(target)-1]!='\\'&&target[lstrlenA(target)-1]!='/')Append(target,MAX_PATH,"\\");Append(target,MAX_PATH,name);for(char* p=target;*p;p++)if(*p=='/')*p='\\';
   for(char* p=target+lstrlenA(resources)+1;*p;p++)if(*p=='\\'){*p=0;CreateDirectoryA(target,0);*p='\\';}
   lstrcpyA(temporary,target);Append(temporary,MAX_PATH,".tmp");HANDLE f=CreateFileA(temporary,0x40000000,0,0,2,FILE_ATTRIBUTE_NORMAL,0);DWORD done=0;
   good=f!=INVALID_HANDLE_VALUE&&WriteFile(f,compiler->meshes[m].data,compiler->meshes[m].size,&done,0)&&done==compiler->meshes[m].size;if(f!=INVALID_HANDLE_VALUE)CloseHandle(f);
   if(!good||!MoveFileExA(temporary,target,1u|8u)){DeleteFileA(temporary);PackerLog::Error("Could not write %s",target);good=false;}}
  const char* target=good&&compiler->objectFile?objectFile:world;
  // CONDEMNED FORK: the compiled whole world is split into the Condemned v113 trio
  // (World00p = WLDP, WorldClient00p = WLDC, WorldServer00p = WLDS). Object files keep the FEAR form.
  CondemnedContainer::Output trio={0,0,0,0,0,0};bool split=false;char clientPath[MAX_PATH],serverPath[MAX_PATH];
  if(good&&!compiler->objectFile){
   if(!CondemnedContainer::Split(compiler->world,compiler->size,storage,trio)){PackerLog::Error("Could not convert the compiled world to the Condemned v113 layout");good=false;}
   else{lstrcpyA(clientPath,world);lstrcpyA(serverPath,world);unsigned n=lstrlenA(world);if(n>9){clientPath[n-9]=0;serverPath[n-9]=0;}Append(clientPath,MAX_PATH,".WorldClient00p");Append(serverPath,MAX_PATH,".WorldServer00p");split=true;}}
  struct Out{const char* path;const unsigned char* data;unsigned size;};
  Out outs[3]={{target,split?trio.world:compiler?compiler->world:0,split?trio.worldSize:compiler?compiler->size:0},{clientPath,trio.client,trio.clientSize},{serverPath,trio.server,trio.serverSize}};
  for(unsigned k=(options.createObjectFile&&split)?2u:0u;good&&k<(split?3u:1u);k++){const char* path=outs[k].path;char temporary[MAX_PATH];lstrcpyA(temporary,path);Append(temporary,MAX_PATH,".tmp");HANDLE f=CreateFileA(temporary,0x40000000,0,0,2,FILE_ATTRIBUTE_NORMAL,0);DWORD done=0;good=f!=INVALID_HANDLE_VALUE&&WriteFile(f,outs[k].data,outs[k].size,&done,0)&&done==outs[k].size;if(f!=INVALID_HANDLE_VALUE)CloseHandle(f);
   if(!good||!MoveFileExA(temporary,path,1u|8u)){DeleteFileA(temporary);PackerLog::Error("Could not write %s",path);good=false;}}
  if(compiler)compiler->~Compiler();PackerLog::End();running=false;return good;
 }
};
// Winpacker releases the packer with delete: allocate it with the matching
// operator new (RenderNvRuntime.h, process heap), not in static storage.
extern "C" __declspec(dllexport) IPackerImpl* AssociatePacker(const char* filename){if(!filename||!filename[0])return 0;return new MonolithWinPacker;}
// No CRT startup object is linked (the static UCRT part is unavailable): the
// C++ dynamic initializers of all objects (.CRT$XCU, including the Nav SDK) run
// between these markers; the dynamic CRT DLLs initialize themselves. Static
// destructors are not registered (atexit below) and do not run at unload.
typedef void (__cdecl *Initializer)(void);
#pragma section(".CRT$XCAA",long,read)
#pragma section(".CRT$XCZZ",long,read)
__declspec(allocate(".CRT$XCAA")) static Initializer initializersBegin=0;
__declspec(allocate(".CRT$XCZZ")) static Initializer initializersEnd=0;
extern "C" int __cdecl atexit(void (__cdecl*)(void)){return 0;}
extern "C" BOOL WINAPI DllMain(HINSTANCE module,DWORD reason,LPVOID){
 if(reason==DLL_PROCESS_ATTACH){moduleHandle=module;for(const Initializer* p=&initializersBegin+1;p<&initializersEnd;p++)if(*p)(*p)();}
 return TRUE;
}
