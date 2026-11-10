#include "WorldPacker.cpp"
#include "ShaderEffectInclude.h"
#include "ShaderRenderDeclaration.h"
#include "ShaderLoaderGate.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
typedef long (WINAPI *EffectCreate)(const char*,const void*,void*,unsigned int,void**,void**);
static void* Compile(EffectCreate create,PrefabScene::Arena& memory,const char* root,const char* shader,long& result,ShaderEffectInclude::Resolver*& include){
 void* resolverStorage=memory.Allocate(sizeof(ShaderEffectInclude::Resolver));if(!resolverStorage)ExitProcess(3);include=new(resolverStorage)ShaderEffectInclude::Resolver(root);void* effect=0,*errors=0;result=create(shader,0,include,0,&effect,&errors);
 if(errors){char errorPath[MAX_PATH];if(GetEnvironmentVariableA("FEAR_GATE_ERROR",errorPath,MAX_PATH)){typedef void* (WINAPI *Pointer)(void*);typedef unsigned int (WINAPI *Size)(void*);void* message=((Pointer)(*(void***)errors)[3])(errors);unsigned int size=((Size)(*(void***)errors)[4])(errors);HANDLE file=CreateFileA(errorPath,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);DWORD done;if(file!=INVALID_HANDLE_VALUE){WriteFile(file,message,size,&done,0);CloseHandle(file);}}typedef unsigned long (WINAPI *Release)(void*);((Release)(*(void***)errors)[2])(errors);}return effect;
}
extern "C" void HarnessEntry(){
 char root[MAX_PATH],shader[MAX_PATH],parent[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_GATE_ROOT",root,MAX_PATH)||!GetEnvironmentVariableA("FEAR_GATE_SHADER",shader,MAX_PATH)||!GetEnvironmentVariableA("FEAR_GATE_OUTPUT",output,MAX_PATH))ExitProcess(1);bool hasParent=GetEnvironmentVariableA("FEAR_GATE_PARENT",parent,MAX_PATH)!=0;
 HMODULE dx=LoadLibraryA("d3dx9_27.dll");EffectCreate create=dx?(EffectCreate)GetProcAddress(dx,"D3DXCreateEffectCompilerFromFileA"):0;if(!create)ExitProcess(2);PrefabScene::Arena memory;long result=0,parentResult=0;ShaderEffectInclude::Resolver* include=0;ShaderEffectInclude::Resolver* parentInclude=0;void* effect=Compile(create,memory,root,shader,result,include);void* parentEffect=hasParent?Compile(create,memory,root,parent,parentResult,parentInclude):0;if(result<0||!effect||hasParent&&(parentResult<0||!parentEffect))ExitProcess(4);
 unsigned int success=0,unsupported=0,count=0,counterCount=0;ShaderLoaderGate::Parameter* parameters=0;unsigned int* counters=0;
#ifdef FEAR_GATE_ORACLE
 LoadLibraryA("E:/fearsdk/Dev/Tools/msvcr71.dll");LoadLibraryA("E:/fearsdk/Dev/Tools/msvcp71.dll");LoadLibraryA("E:/fearsdk/Dev/Tools/MFC71.dll");unsigned char* original=(unsigned char*)LoadLibraryA("E:/fearsdk/Dev/Tools/Packers/WorldPacker.dll");if(!original)ExitProcess(5);
 const unsigned char signature[4]={0x83,0xec,0x38,0x53};for(unsigned int i=0;i<4;i++)if(original[0x7be80+i]!=signature[i])ExitProcess(6);
 unsigned int object[15]={},parentObject[15]={};typedef void* (__thiscall *Construct)(void*);typedef void (__thiscall *Clear)(void*);typedef bool (__thiscall *Reflect)(void*,void*);typedef bool (__thiscall *Finish)(void*);
 ((Construct)(original+0x7bd70))(object);((Construct)(original+0x7bd70))(parentObject);success=((Reflect)(original+0x7be80))(object,effect)?1:0;if(hasParent&&success)success=((Reflect)(original+0x7be80))(parentObject,parentEffect)&&((Reflect)(original+0x7bfa0))(object,parentObject)?1:0;
 count=(object[3]-object[2])/4;parameters=(ShaderLoaderGate::Parameter*)memory.Allocate(count*sizeof(ShaderLoaderGate::Parameter));if(count&&!parameters)ExitProcess(7);for(unsigned int i=0;i<count;i++){unsigned int* parameter=((unsigned int**)object[2])[i];typedef unsigned int (__thiscall *Kind)(void*);unsigned int kind=((Kind)(*(void***)parameter)[3])(parameter);ShaderLoaderGate::Parameter selected={(const char*)parameter[1],kind,kind==1?parameter[4]:0};parameters[i]=selected;if(selected.kind==1&&selected.mapping>=65536)unsupported=1;}
 if(success&&!unsupported)success=((Finish)(original+0x7b6e0))(object)?1:0;counterCount=object[11];counters=(unsigned int*)object[12];
#else
 void* tableStorage=memory.Allocate(sizeof(ShaderLoaderGate::Table));void* parentStorage=memory.Allocate(sizeof(ShaderLoaderGate::Table));if(!tableStorage||!parentStorage)ExitProcess(7);ShaderLoaderGate::Table* table=new(tableStorage)ShaderLoaderGate::Table();ShaderLoaderGate::Table* parentTable=new(parentStorage)ShaderLoaderGate::Table();success=table->Reflect(effect)?1:0;if(hasParent&&success)success=parentTable->Reflect(parentEffect)&&table->Merge(*parentTable)?1:0;if(success){bool finished=table->Finish();if(!finished&&!table->unsupported)success=0;}unsupported=table->unsupported?1:0;count=table->count;parameters=table->parameters;counterCount=table->counterCount;counters=table->counters;
#endif
 HANDLE file=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);DWORD done;unsigned int header[6]={0x31474c53,(unsigned int)result,success,unsupported,count,counterCount};if(file==INVALID_HANDLE_VALUE||!WriteFile(file,header,24,&done,0)||done!=24)ExitProcess(8);
 for(unsigned int i=0;i<count;i++){unsigned int n=lstrlenA(parameters[i].name),record[3]={parameters[i].kind,parameters[i].mapping,n};if(!WriteFile(file,record,12,&done,0)||done!=12||!WriteFile(file,parameters[i].name,n,&done,0)||done!=n)ExitProcess(8);}if(counterCount&&(!WriteFile(file,counters,counterCount*4,&done,0)||done!=counterCount*4))ExitProcess(8);CloseHandle(file);
#ifdef FEAR_GATE_ORACLE
 ((Clear)(original+0x7bdd0))(parentObject);((Clear)(original+0x7bdd0))(object);
#else
 parentTable->~Table();table->~Table();
#endif
 typedef unsigned long (WINAPI *Release)(void*);if(parentEffect)((Release)(*(void***)parentEffect)[2])(parentEffect);((Release)(*(void***)effect)[2])(effect);if(parentInclude)parentInclude->~Resolver();include->~Resolver();ExitProcess(0);
}
