#include "WorldPacker.cpp"
#include "ShaderDependencyPaths.h"
#include "ShaderDeclarationClosure.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
extern "C" __declspec(dllimport) HMODULE WINAPI LoadLibraryA(const char*);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
#pragma comment(linker,"/alternatename:__imp__LoadLibraryA@4=__imp__LoadLibraryA")
struct DependencyInput {unsigned int nullEffect,parentPresent,parentStringFail,parentLoadFail,techniques;unsigned int annotationPresent[8],annotationStringFail[8],annotationLoadFail[8];char root[264];};
struct Event {unsigned int kind,index,value;};
static DependencyInput* current;static Event events[192];static unsigned int eventCount,pathCount,lastHandle;static char paths[64][264],names[64][64];
static void EventAdd(unsigned int kind,unsigned int index,unsigned int value){if(eventCount>=192)ExitProcess(20);events[eventCount].kind=kind;events[eventCount].index=index;events[eventCount++].value=value;}
static unsigned int Decimal(const char* p){unsigned int n=0;while(*p>='0'&&*p<='9')n=n*10+(unsigned int)(*p++-'0');return n;}
static bool Equal(const char* a,const char* b){while(*a&&*a==*b){a++;b++;}return *a==*b;}
static void Number(char* out,unsigned int value){char reverse[16];unsigned int n=0;do{reverse[n++]=(char)('0'+value%10);value/=10;}while(value);for(unsigned int i=0;i<n;i++)out[i]=reverse[n-1-i];out[n]=0;}
static const char* WINAPI Find(void*,const char* parent,const char* name){
 if(parent)ExitProcess(21);for(unsigned int i=0;i<14;i++)if(name[i]!="ParentMaterial"[i])ExitProcess(21);unsigned int index=Decimal(name+14);bool present=index<32&&((current->parentPresent>>index)&1)!=0;EventAdd(0,index,present?1:0);return present?(const char*)(index+1):0;
}
static const char* WINAPI Technique(void*,unsigned int index){bool present=index<current->techniques;EventAdd(1,index,present?1:0);return present?(const char*)(0x100+index):0;}
static const char* WINAPI Annotation(void*,const char* handle,const char* name){
 unsigned int index=(unsigned int)handle-0x100,kind=0;if(Equal(name,"Fallback"))kind=0;else if(Equal(name,"High"))kind=1;else if(Equal(name,"Medium"))kind=2;else if(Equal(name,"Low"))kind=3;else ExitProcess(22);
 if(index>=8)ExitProcess(23);bool present=(current->annotationPresent[index]>>kind)&1;EventAdd(2,index,(kind+1)|(present?256:0));return present?(const char*)(0x200+index*4+kind):0;
}
static unsigned int Ordinal(unsigned int handle){return handle<0x100?handle-1:32+handle-0x200;}
static bool Failure(unsigned int handle,bool string){if(handle<0x100)return (((string?current->parentStringFail:current->parentLoadFail)>>(handle-1))&1)!=0;unsigned int id=handle-0x200;return ((((string?current->annotationStringFail[id/4]:current->annotationLoadFail[id/4])>>(id%4)))&1)!=0;}
static long WINAPI String(void*,const char* parameter,const char** output){lastHandle=(unsigned int)parameter;bool fail=Failure(lastHandle,true);EventAdd(3,Ordinal(lastHandle),fail?0:1);if(fail)return (long)0x80004005;*output=(current->nullEffect&2)?0:((current->nullEffect&4)?"":names[Ordinal(lastHandle)]);return 0;}
static bool Load(const char* path){if(pathCount>=64)ExitProcess(24);lstrcpyA(paths[pathCount++],path);bool fail=Failure(lastHandle,false);EventAdd(4,Ordinal(lastHandle),fail?0:1);return !fail;}
#ifndef FEAR_SHADER_DEPENDENCY_ORACLE
static bool Visit(void*,const ShaderDeclarationClosure::Dependency& dependency){char directory[264],resolved[264];if(!ShaderDependencyPaths::Directory(current->root,directory,264)||!ShaderDependencyPaths::Resolve(directory,dependency.path,resolved,264))ExitProcess(25);return Load(resolved);}
#endif
#ifdef FEAR_SHADER_DEPENDENCY_ORACLE
// Copies retain original control flow. Only dependency cache/merge calls use
// local mock callbacks; no instruction in the loaded module is modified.
static unsigned int manager[5],loaded[15];
static void* __cdecl Manager(){return manager;}
static void* __fastcall Lookup(void*,void*,const char* path){return Load(path)?loaded:0;}
static bool __fastcall Merge(void*,void*,void*){return true;}
static void Patch(unsigned char* copy,unsigned int offset,void* target){if(copy[offset]!=0xe8)ExitProcess(26);*(int*)(copy+offset+1)=(int)((unsigned char*)target-copy-offset-5);}
#endif
extern "C" void HarnessEntry(){
 char source[MAX_PATH],destination[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_DEPENDENCY_INPUT",source,MAX_PATH)||!GetEnvironmentVariableA("FEAR_DEPENDENCY_OUTPUT",destination,MAX_PATH))ExitProcess(1);
 HANDLE in=CreateFileA(source,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);unsigned int count=0;DWORD done=0;if(in==INVALID_HANDLE_VALUE||!ReadFile(in,&count,4,&done,0)||done!=4||!count||count>10000)ExitProcess(2);
 PrefabScene::Arena memory;DependencyInput* inputs=(DependencyInput*)memory.Allocate(count*sizeof(DependencyInput));if(!inputs||!ReadFile(in,inputs,count*sizeof(DependencyInput),&done,0)||done!=count*sizeof(DependencyInput))ExitProcess(3);CloseHandle(in);
 void* methods[52]={};methods[9]=(void*)Find;methods[12]=(void*)Technique;methods[19]=(void*)Annotation;methods[51]=(void*)String;void** effect=methods;
 for(unsigned int i=0;i<64;i++){lstrcpyA(names[i],i<32?"../parent":"./lod");Number(names[i]+ShaderDependencyPaths::Length(names[i]),i);lstrcpyA(names[i]+ShaderDependencyPaths::Length(names[i]),".fx");}
#ifdef FEAR_SHADER_DEPENDENCY_ORACLE
 LoadLibraryA("E:/fearsdk/Dev/Tools/msvcr71.dll");LoadLibraryA("E:/fearsdk/Dev/Tools/msvcp71.dll");LoadLibraryA("E:/fearsdk/Dev/Tools/MFC71.dll");unsigned char* dll=(unsigned char*)LoadLibraryA("E:/fearsdk/Dev/Tools/Packers/WorldPacker.dll");if(!dll||dll[0x7c530]!=0x8b||dll[0x7c5e0]!=0x81)ExitProcess(4);
 unsigned char* load=(unsigned char*)VirtualAlloc(0,0x100,MEM_COMMIT|MEM_RESERVE,0x40);unsigned char* traversal=(unsigned char*)VirtualAlloc(0,0x200,MEM_COMMIT|MEM_RESERVE,0x40);if(!load||!traversal)ExitProcess(5);
 memcpy(load,dll+0x7c530,0xa7);memcpy(traversal,dll+0x7c5e0,0x140);
 Patch(load,0x58,dll+0x7ca40);Patch(load,0x65,(void*)Manager);Patch(load,0x6c,(void*)Lookup);Patch(load,0x7a,(void*)Merge);Patch(load,0x86,(void*)Merge);
 Patch(traversal,0x31,dll+0x7c9e0);Patch(traversal,0x45,dll+0x16b0);Patch(traversal,0x69,load);Patch(traversal,0x84,dll+0x16b0);Patch(traversal,0xef,load);
 typedef bool (__thiscall* Traverse)(void*,void*);
#endif
 HANDLE out=CreateFileA(destination,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(out==INVALID_HANDLE_VALUE||!WriteFile(out,&count,4,&done,0)||done!=4)ExitProcess(6);
 for(unsigned int i=0;i<count;i++){
  current=&inputs[i];current->root[263]=0;if(current->techniques>8)ExitProcess(7);eventCount=pathCount=lastHandle=0;memset(events,0,sizeof(events));memset(paths,0,sizeof(paths));bool success=false;
#ifdef FEAR_SHADER_DEPENDENCY_ORACLE
  unsigned int shader[15]={};shader[0]=(unsigned int)current->root;success=((Traverse)traversal)(shader,(current->nullEffect&1)?0:&effect);
#else
  success=ShaderDeclarationClosure::Enumerate((current->nullEffect&1)?0:&effect,Visit,0);
#endif
  unsigned int header[3]={success?1u:0u,eventCount,pathCount};if(!WriteFile(out,header,12,&done,0)||done!=12||!WriteFile(out,events,eventCount*sizeof(Event),&done,0)||done!=eventCount*sizeof(Event)||!WriteFile(out,paths,pathCount*264,&done,0)||done!=pathCount*264)ExitProcess(8);
 }
 CloseHandle(out);ExitProcess(0);
}
