#include "win_min.h"
#include "IPackerImpl.h"
#include "IPackerOutput.h"
#include "WinPackerNativeOptions.h"
#include "WinPackerNativeProgress.h"
extern "C" {
struct NativeSecurity {DWORD bytes;void* descriptor;BOOL inherit;};
struct NativeStartup {DWORD bytes;char* reserved;char* desktop;char* title;DWORD x,y,width,height,xchars,ychars,fill,flags;unsigned short show,reservedBytes;unsigned char* reservedData;HANDLE input,output,error;};
struct NativeProcess {HANDLE process,thread;DWORD processId,threadId;};
struct NativeJobLimits {long long processTime,jobTime;DWORD flags;unsigned int minimumWorking,maximumWorking,processes,affinity,priority,scheduling;unsigned long long io[6];unsigned int processMemory,jobMemory,peakProcess,peakJob;};
__declspec(dllimport) BOOL WINAPI CreatePipe(HANDLE*,HANDLE*,NativeSecurity*,DWORD);
__declspec(dllimport) BOOL WINAPI SetHandleInformation(HANDLE,DWORD,DWORD);
__declspec(dllimport) BOOL WINAPI CreateProcessA(const char*,char*,void*,void*,BOOL,DWORD,void*,const char*,NativeStartup*,NativeProcess*);
__declspec(dllimport) BOOL WINAPI PeekNamedPipe(HANDLE,void*,DWORD,DWORD*,DWORD*,DWORD*);
__declspec(dllimport) DWORD WINAPI WaitForSingleObject(HANDLE,DWORD);
__declspec(dllimport) BOOL WINAPI GetExitCodeProcess(HANDLE,DWORD*);
__declspec(dllimport) BOOL WINAPI TerminateProcess(HANDLE,unsigned int);
__declspec(dllimport) DWORD WINAPI GetTickCount();
__declspec(dllimport) void WINAPI Sleep(DWORD);
__declspec(dllimport) HANDLE WINAPI CreateJobObjectA(void*,const char*);
__declspec(dllimport) BOOL WINAPI SetInformationJobObject(HANDLE,unsigned int,void*,DWORD);
__declspec(dllimport) BOOL WINAPI AssignProcessToJobObject(HANDLE,HANDLE);
__declspec(dllimport) DWORD WINAPI ResumeThread(HANDLE);
}
static_assert(sizeof(NativeStartup)==68,"x86 STARTUPINFO ABI");
static_assert(sizeof(NativeProcess)==16,"x86 PROCESS_INFORMATION ABI");
static_assert(sizeof(NativeJobLimits)==112,"x86 JOBOBJECT_EXTENDED_LIMIT_INFORMATION ABI");
static HMODULE nativeModule;
namespace NativeWinPackerDll {
struct Command {char* text;Command():text((char*)VirtualAlloc(0,4096,MEM_COMMIT|MEM_RESERVE,4)){}~Command(){if(text)VirtualFree(text,0,MEM_RELEASE);}};
struct OwnedSettings {
 typedef WinPackerNativeOptions::Settings Settings;Settings* value;
 OwnedSettings():value((Settings*)VirtualAlloc(0,sizeof(Settings),MEM_COMMIT|MEM_RESERVE,4)){if(value)new(value)Settings;}
 ~OwnedSettings(){if(value){value->~Settings();VirtualFree(value,0,MEM_RELEASE);}}
};
static bool Append(char* target,unsigned int capacity,const char* text){unsigned int n=lstrlenA(target),m=lstrlenA(text);if(n>=capacity||m>=capacity-n)return false;for(unsigned int i=0;i<=m;i++)target[n+i]=text[i];return true;}
static bool Path(const char* base,const char* relative,char* target){char path[2*MAX_PATH];if(!relative||!relative[0])return false;if(relative[0]=='\\'||relative[0]=='/'||relative[1]==':'){if(lstrlenA(relative)>=MAX_PATH)return false;lstrcpyA(path,relative);}else{lstrcpyA(path,base);if(!Append(path,sizeof(path),"\\")||!Append(path,sizeof(path),relative))return false;}DWORD n=GetFullPathNameA(path,MAX_PATH,target,0);return n&&n<MAX_PATH;}
static void Parent(char* path){char* p=path,*last=0;for(;*p;p++)if(*p=='\\'||*p=='/')last=p;if(last)*last=0;else path[0]=0;}
static bool ModuleFolder(char* folder){DWORD n=GetModuleFileNameA(nativeModule,folder,MAX_PATH);if(!n||n>=MAX_PATH)return false;Parent(folder);return folder[0]!=0;}
static void Log(IPackerOutput* output,EMsgType level,const char* text){char folder[MAX_PATH],path[MAX_PATH];if(ModuleFolder(folder)&&Path(folder,"native-rebuild.log",path)){HANDLE f=CreateFileA(path,FILE_APPEND_DATA,FILE_SHARE_READ,0,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,0);if(f!=INVALID_HANDLE_VALUE){DWORD done;WriteFile(f,text,lstrlenA(text),&done,0);WriteFile(f,"\r\n",2,&done,0);CloseHandle(f);}}if(output)output->LogMessage(level,text);}
static bool Quote(char* command,const char* value){if(!value||!value[0])return false;unsigned int length=0;for(;value[length];length++)if(value[length]=='"'||value[length]=='\r'||value[length]=='\n')return false;if(!Append(command,4096,command[0]?" \"":"\"")||!Append(command,4096,value))return false;for(unsigned int n=length;n&&value[n-1]=='\\';n--)if(!Append(command,4096,"\\"))return false;return Append(command,4096,"\"");}
static void Line(IPackerOutput* output,char* text,WinPackerNativeProgress::Progress& progress){unsigned int n=lstrlenA(text);if(n&&text[n-1]=='\r')text[n-1]=0;if(!text[0])return;if(progress.OnLine(output,text))Log(output,MSG_NORMAL,text+6);else Log(output,MSG_NORMAL,text);}
static bool Run(char* command,const char* working,IPackerOutput* output,WinPackerNativeProgress::Progress& progress){
 HANDLE readPipe=0,writePipe=0;NativeSecurity security={sizeof(NativeSecurity),0,TRUE};if(!CreatePipe(&readPipe,&writePipe,&security,0)||!SetHandleInformation(readPipe,1,0)){if(readPipe)CloseHandle(readPipe);if(writePipe)CloseHandle(writePipe);Log(output,MSG_ERROR,"Could not create isolated compiler output channel");return false;}
 NativeStartup startup={};startup.bytes=sizeof(startup);startup.flags=0x100|1;startup.show=0;startup.output=startup.error=writePipe;NativeProcess child={};
 HANDLE job=CreateJobObjectA(0,0);NativeJobLimits limits={};limits.flags=0x2000;if(!job||!SetInformationJobObject(job,9,&limits,sizeof(limits))){if(job)CloseHandle(job);CloseHandle(readPipe);CloseHandle(writePipe);Log(output,MSG_ERROR,"Could not initialize isolated compiler job");return false;}
 BOOL launched=CreateProcessA(0,command,0,0,TRUE,0x08000000|4,0,working,&startup,&child);CloseHandle(writePipe);if(!launched){CloseHandle(job);CloseHandle(readPipe);Log(output,MSG_ERROR,"Could not start native compiler runtime; check native-packer.ini");return false;}
 if(!AssignProcessToJobObject(job,child.process)||ResumeThread(child.thread)==0xffffffffu){TerminateProcess(child.process,125);CloseHandle(child.thread);CloseHandle(child.process);CloseHandle(job);CloseHandle(readPipe);Log(output,MSG_ERROR,"Could not isolate native compiler process tree");return false;}
  char line[1024];unsigned int used=0;DWORD started=GetTickCount(),exitCode=259;bool timeout=false;for(;;){DWORD available=0;if(!PeekNamedPipe(readPipe,0,0,0,&available,0))available=0;if(available){char buffer[512];DWORD received=0;if(ReadFile(readPipe,buffer,available>sizeof(buffer)?sizeof(buffer):available,&received,0))for(DWORD i=0;i<received;i++){if(buffer[i]=='\n'){line[used]=0;Line(output,line,progress);used=0;}else if(used<sizeof(line)-1)line[used++]=buffer[i];}}
  else if(WaitForSingleObject(child.process,0)==0)break;
  if(GetTickCount()-started>15u*60u*1000u){timeout=true;TerminateProcess(child.process,124);break;}Sleep(20);
  }if(used){line[used]=0;Line(output,line,progress);}GetExitCodeProcess(child.process,&exitCode);CloseHandle(readPipe);CloseHandle(child.thread);CloseHandle(child.process);CloseHandle(job);if(timeout){Log(output,MSG_ERROR,"Native compiler timed out; isolated workers were stopped");return false;}if(exitCode){Log(output,MSG_ERROR,"Native compilation did not complete; see the stage messages");return false;}return true;
}
}
class NativeWinPacker:public IPackerImpl {
 bool running;
public:
 NativeWinPacker():running(false){}
 bool GetPackerName(char* name,int bytes){if(!name||bytes<1)return false;lstrcpynA(name,"FEAR Native Rebuild - integration test",bytes);return true;}
 bool RequestUserOptions(IPackerUI* ui){return WinPackerNativeOptions::Register(ui);}
 bool PropertyChanged(CPackerProperty*,CPackerPropList*,IPackerUI*){return true;}
 bool Process(const char* filename,CPackerPropList* properties,IPackerOutput* output){
  using namespace NativeWinPackerDll;if(running){Log(output,MSG_ERROR,"A native compilation is already active");return false;}OwnedSettings ownedSettings;if(!ownedSettings.value){Log(output,MSG_ERROR,"Could not allocate native compiler settings");return false;}WinPackerNativeOptions::Settings& options=*ownedSettings.value;if(!WinPackerNativeOptions::Read(properties,options)){Log(output,MSG_ERROR,options.error);return false;}if(!filename||!filename[0]){Log(output,MSG_ERROR,"Select a source World00a or World00c");return false;}
  char folder[MAX_PATH],base[MAX_PATH],ini[MAX_PATH],python[MAX_PATH],driver[MAX_PATH],project[MAX_PATH],resources[MAX_PATH],destination[MAX_PATH],source[MAX_PATH];if(!ModuleFolder(folder))return false;lstrcpyA(base,folder);Parent(base);
  if(!Path(folder,"native-packer.ini",ini)||!Path(base,filename,source)||!Path(base,options.project,project)||!Path(base,options.output,destination)){Log(output,MSG_ERROR,"A native input/output path exceeds the test profile");return false;}
  DWORD pythonLength=GetPrivateProfileStringA("Runtime","Python","",python,MAX_PATH,ini),driverLength=GetPrivateProfileStringA("Runtime","Driver","",driver,MAX_PATH,ini);if(!pythonLength||pythonLength>=MAX_PATH-1||!driverLength||driverLength>=MAX_PATH-1){Log(output,MSG_ERROR,"Missing or oversized native-packer.ini runtime paths");return false;}
  Command ownedCommand;char* command=ownedCommand.text;if(!command||!Quote(command,python)||!Quote(command,driver)||!Append(command,4096," --source")||!Quote(command,source)||!Append(command,4096," --project")||!Quote(command,project)||!Append(command,4096," --output")||!Quote(command,destination)||!Append(command,4096," --sections-only")||!Quote(command,options.sectionsOnly?"true":"false")){Log(output,MSG_ERROR,"Native compiler command is outside the supported path profile");return false;}
  if(options.resources[0]){if(!Path(base,options.resources,resources)||!Append(command,4096," --resources")||!Quote(command,resources)){Log(output,MSG_ERROR,"Invalid explicit resource root");return false;}}
  if(!Append(command,4096," --split-meshes")||!Quote(command,options.splitMeshes?"true":"false")||!Append(command,4096," --post-profile")||!Quote(command,options.postProfile)){Log(output,MSG_ERROR,"Native compiler command exceeds supported limits");return false;}
  if(options.worldStem[0]&&(!Append(command,4096," --world-stem")||!Quote(command,options.worldStem))){Log(output,MSG_ERROR,"Invalid explicit Game-relative world path");return false;}
  if(!options.centerWorld){Log(output,MSG_ERROR,"Native compilation requires CenterWorld=true");return false;}
  running=true;WinPackerNativeProgress::Progress progress;progress.Begin(output);Log(output,MSG_NORMAL,"Native rebuild: source workers only; original WorldPacker is not used");Log(output,MSG_NORMAL,options.sectionsOnly?"Mode: compile section files and manifest (no game World00p requested)":"Mode: assemble source-derived World00p in the supported test profile");Log(output,MSG_NORMAL,options.splitMeshes?"SplitMeshes: enabled":"SplitMeshes: disabled");bool good=Run(command,base,output,progress);progress.Finish(output,good);if(good){Log(output,MSG_NORMAL,options.sectionsOnly?"Native section test completed. Output folder:":"Native World00p compilation completed. Output folder:");Log(output,MSG_NORMAL,destination);}running=false;return good;
 }
};
__declspec(align(8)) static char nativeStorage[sizeof(NativeWinPacker)];
extern "C" __declspec(dllexport) IPackerImpl* AssociatePacker(const char* filename){if(!filename||!filename[0])return 0;return new(nativeStorage)NativeWinPacker;}
BOOL WINAPI DllMain(HINSTANCE module,DWORD reason,LPVOID){if(reason==DLL_PROCESS_ATTACH)nativeModule=module;return TRUE;}
void operator delete(void*,unsigned int){}
extern "C" int __cdecl _purecall(){return 0;}
extern "C" {int _fltused=0;}
extern "C" void* __cdecl memset(void* target,int value,unsigned int bytes){unsigned char* p=(unsigned char*)target;while(bytes--)*p++=(unsigned char)value;return target;}
