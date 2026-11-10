#pragma once
// Winpacker log in the original WorldPacker form (VA 0x10053580..0x10053960):
// every line is indented by two spaces per open scope (capped at 255), then a
// "Error: "/"Warning: "/""/"Debug: " prefix, then the formatted text, sent with
// IPackerOutput::LogMessage. A scope logs its name, optionally activates it as
// the subtask, and on close logs "Finished %s: %.2fs" (or "%dm %.2fs" from 60 s)
// at the inner indent. Formatting uses msvcrt.dll _vsnprintf (the old CRT float
// formatting of the original's static CRT), loaded at first use.
#include "IPackerOutput.h"
extern "C" {
__declspec(dllimport) DWORD WINAPI GetTickCount();
__declspec(dllimport) HMODULE WINAPI LoadLibraryA(const char*);
__declspec(dllimport) void* WINAPI GetProcAddress(HMODULE,const char*);
__declspec(dllimport) char* WINAPI lstrcatA(char*,const char*);
}
#pragma comment(linker,"/alternatename:__imp__lstrcatA@8=__imp__lstrcatA")
#pragma comment(linker,"/alternatename:__imp__GetTickCount@0=__imp__GetTickCount")
#pragma comment(linker,"/alternatename:__imp__LoadLibraryA@4=__imp__LoadLibraryA")
#pragma comment(linker,"/alternatename:__imp__GetProcAddress@8=__imp__GetProcAddress")
namespace PackerLog {
typedef char* va;
typedef int (__cdecl *Formatter)(char*,unsigned int,const char*,va);
struct State {IPackerOutput* output;int indent;Formatter format;State():output(0),indent(0),format(0){}};
inline State& Current(){static State state;return state;}
inline void Begin(IPackerOutput* output){State& s=Current();s.output=output;s.indent=0;if(!s.format){HMODULE m=LoadLibraryA("msvcrt.dll");if(m)s.format=(Formatter)GetProcAddress(m,"_vsnprintf");}}
inline void End(){Current().output=0;Current().indent=0;}
inline void Indent(int delta){int v=Current().indent+delta;Current().indent=v<0?0:v;}
inline void Emit(EMsgType type,const char* prefix,const char* format,va args){
 State& s=Current();if(!s.output)return;char text[512],line[1100];text[0]=0;
 if(s.format){s.format(text,sizeof(text),format,args);text[sizeof(text)-1]=0;}else lstrcpynA(text,format,sizeof(text));
 int spaces=s.indent<255?s.indent:255;unsigned int n=0;for(int i=0;i<2*spaces&&n<512;i++)line[n++]=' ';line[n]=0;
 lstrcpynA(line+n,prefix,(int)(sizeof(line)-n));n=lstrlenA(line);lstrcpynA(line+n,text,(int)(sizeof(line)-n));
 s.output->LogMessage(type,line);
}
inline void Normal(const char* format,...){Emit(MSG_NORMAL,"",format,(va)(&format+1));}
inline void Warning(const char* format,...){Emit(MSG_WARNING,"Warning: ",format,(va)(&format+1));}
inline void Error(const char* format,...){Emit(MSG_ERROR,"Error: ",format,(va)(&format+1));}
inline void Debug(const char* format,...){Emit(MSG_RAW_LEVEL3,"Debug: ",format,(va)(&format+1));}
inline void Task(const char* name){IPackerOutput* o=Current().output;if(o){o->CreateTask(name);o->ActivateTask(name);}}
inline void SubTask(const char* name){IPackerOutput* o=Current().output;if(o)o->ActivateSubTask(name);}
inline void Progress(float percent){IPackerOutput* o=Current().output;if(o)o->UpdateProgress(percent);}
inline void CheckFile(const char* path){IPackerOutput* o=Current().output;if(o)o->CheckOutputFile(path);}
// "Finished %s: %.2fs", or "%dm %.2fs" from 60 s (0x10053810: ticks / 1000.0).
inline void Finished(const char* name,unsigned long milliseconds){
 double seconds=(double)(int)milliseconds/1000.0;
 if(seconds<60.0)Normal("Finished %s: %.2fs",name,seconds);
 else{int minutes=(int)(seconds/60.0);Normal("Finished %s: %dm %.2fs",name,minutes,seconds-minutes*60.0);}
}
// Original timed scope (0x10053780 open, 0x10053810 close).
class Scope {
 char name[256];DWORD start;bool open;
public:
 explicit Scope(const char* text,bool subtask=true):start(GetTickCount()),open(true){lstrcpynA(name,text,sizeof(name));Normal("%s",name);if(subtask)SubTask(name);Indent(1);}
 void Close(){if(!open)return;open=false;Finished(name,GetTickCount()-start);Indent(-1);}
 // Close reporting the duration of work that ran before the scope was opened.
 void CloseAfter(unsigned long milliseconds){if(!open)return;open=false;Finished(name,milliseconds);Indent(-1);}
 ~Scope(){Close();}
};
// A scope whose work already ran (scene preparation is one call): same lines,
// with the duration measured for the corresponding stage.
inline void Completed(const char* name,unsigned long milliseconds,bool subtask=true){Normal("%s",name);if(subtask)SubTask(name);Indent(1);Finished(name,milliseconds);Indent(-1);}
}
