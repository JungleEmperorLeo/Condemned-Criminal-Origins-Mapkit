#define FEAR_NATIVE_NV_RUNTIME 1
#define FEAR_RENDER_INDEX_REORDER 1
#define FEAR_PRIMARY_SURFACE_PREPARATION 1
#include "WorldPacker.cpp"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
#include "RenderNvRuntime.h"
#include "NativeRenderCompiler.h"
namespace CompilerGuards {
struct State {
 NativeMaterialLayouts::Cache* cache;NativeRenderCompiler::Builder* compiler;
 const NativePreparedRenderScene::Context* scene;NativeRenderCompiler::Settings settings;
 NativeMaterialLayouts::Layout temporary;char shader[300];RenderVertexPacking::Element declaration[64];
 unsigned int calls;bool fail,unsupported,invalidState,emptyFailed,emptyReady,nullResult,tooMany,oversized,reentry,clearWhileRunning,cancelOnResolve;
 bool cancellation,reentryRejected;unsigned int cancelStage;
 State():cache(0),compiler(0),scene(0),calls(0),fail(false),unsupported(false),invalidState(false),emptyFailed(false),emptyReady(false),nullResult(false),tooMany(false),oversized(false),reentry(false),clearWhileRunning(false),cancelOnResolve(false),cancellation(false),reentryRejected(false),cancelStage(0xffffffffu){}
};
static bool Resolve(void* context,const char* material,const NativeMaterialLayouts::Layout** output){
 State& state=*(State*)context;++state.calls;if(state.fail)return false;if(state.nullResult){*output=0;return true;}
 const NativeMaterialLayouts::Layout* from=0;if(!state.cache->Resolve(material,&from))return false;
 state.temporary=*from;lstrcpyA(state.shader,from->shader);memcpy(state.declaration,from->declaration,from->count*8);
 state.temporary.shader=state.shader;state.temporary.declaration=state.declaration;
 if(state.unsupported)state.temporary.state=NativeMaterialLayouts::Unsupported;
 if(state.invalidState)state.temporary.state=(NativeMaterialLayouts::State)99;
 if(state.emptyFailed||state.emptyReady){state.temporary.state=state.emptyFailed?NativeMaterialLayouts::Failed:NativeMaterialLayouts::Ready;state.shader[0]=0;state.temporary.count=10;memcpy(state.declaration,RenderVertexPacking::Working,80);}
 if(state.tooMany)state.temporary.count=65;
 if(state.oversized){memset(state.shader,'x',264);state.shader[264]=0;}
 if(state.reentry){NativeRenderCompiler::Resolver nested;nested.resolve=Resolve;nested.context=&state;state.reentryRejected=!state.compiler->Build(*state.scene,state.settings,nested)&&state.compiler->Error().code==NativeRenderCompiler::Reentrant;}
 if(state.clearWhileRunning)state.compiler->Clear();
 if(state.cancelOnResolve)state.cancellation=true;
 *output=&state.temporary;return true;
}
static bool Cancel(void* context){return ((State*)context)->cancellation;}
static void Progress(void* context,NativeRenderCompiler::Stage stage,unsigned int,unsigned int){State& state=*(State*)context;if((unsigned int)stage==state.cancelStage)state.cancellation=true;}
static bool Equal(const unsigned char* a,unsigned int an,const unsigned char* b,unsigned int bn){if(an!=bn)return false;for(unsigned int i=0;i<an;i++)if(a[i]!=b[i])return false;return true;}
static bool Empty(const NativeRenderCompiler::Builder& compiler){return !compiler.Ready()&&!compiler.Result().data&&!compiler.Result().size&&!compiler.Result().meshes&&!compiler.Statistics().trees&&!compiler.Statistics().nodes&&!compiler.Statistics().surfaces&&!compiler.NodeReports();}
static bool Write(HANDLE file,const void* data,unsigned int bytes){DWORD done=0;return WriteFile(file,data,bytes,&done,0)&&done==bytes;}
static void FailureFile(const char* path,const NativeRenderCompiler::Builder& compiler){HANDLE file=CreateFileA(path,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(file==INVALID_HANDLE_VALUE)return;const NativeRenderCompiler::Diagnostic& error=compiler.Error();unsigned int length=lstrlenA(error.message),header[4]={0x3146434e,(unsigned int)error.code,(unsigned int)error.stage,length};Write(file,header,16);if(length)Write(file,error.message,length);CloseHandle(file);}
}
extern "C" void CompilerGuardsEntry(){
 using namespace CompilerGuards;
 char source[260],project[260],root[260],output[260];
 if(!GetEnvironmentVariableA("FEAR_COMPILER_GUARDS_SOURCE",source,260)||!GetEnvironmentVariableA("FEAR_COMPILER_GUARDS_PROJECT",project,260)||!GetEnvironmentVariableA("FEAR_COMPILER_GUARDS_ROOT",root,260)||!GetEnvironmentVariableA("FEAR_COMPILER_GUARDS_OUTPUT",output,260))ExitProcess(1);
 PrefabScene::Arena memory;
 void* contextMemory=memory.Allocate(sizeof(NativePreparedRenderScene::Context));void* compilerMemory=memory.Allocate(sizeof(NativeRenderCompiler::Builder));void* cacheMemory=memory.Allocate(sizeof(NativeMaterialLayouts::Cache));
 if(!contextMemory||!compilerMemory||!cacheMemory)ExitProcess(2);
 NativePreparedRenderScene::Context* scene=new(contextMemory)NativePreparedRenderScene::Context();NativeRenderCompiler::Builder* compiler=new(compilerMemory)NativeRenderCompiler::Builder();NativeMaterialLayouts::Cache* cache=new(cacheMemory)NativeMaterialLayouts::Cache();
 State state;state.compiler=compiler;state.cache=cache;state.scene=scene;state.settings.worldStem=source;state.settings.projectRoot=root;
 NativeRenderCompiler::Resolver resolver;resolver.resolve=Resolve;resolver.context=&state;
 NativeRenderCompiler::Observer observer;observer.cancel=Cancel;observer.progress=Progress;observer.context=&state;
 unsigned int checks[20]={},count=0;
 checks[count++]=!compiler->Build(*scene,state.settings,resolver)&&Empty(*compiler)&&compiler->Error().code==NativeRenderCompiler::InvalidArgument;
 if(!scene->Build(source,project)||!cache->Initialize(root))ExitProcess(3);
 NativeRenderCompiler::Resolver persistent;persistent.resolve=NativeMaterialLayouts::Cache::Callback;persistent.context=cache;
 if(!compiler->Build(*scene,state.settings,persistent,observer)){FailureFile(output,*compiler);ExitProcess(4);}
 unsigned int referenceBytes=compiler->Result().size;unsigned char* reference=(unsigned char*)memory.Allocate(referenceBytes);if(!reference)ExitProcess(2);memcpy(reference,compiler->Result().data,referenceBytes);
 checks[count++]=compiler->Build(*scene,state.settings,resolver,observer)&&compiler->Ready()&&state.calls>=2&&compiler->Error().code==NativeRenderCompiler::Ok&&Equal(reference,referenceBytes,compiler->Result().data,compiler->Result().size);
 state.fail=true;checks[count++]=!compiler->Build(*scene,state.settings,resolver,observer)&&Empty(*compiler)&&compiler->Error().code==NativeRenderCompiler::MaterialFailure;state.fail=false;
 checks[count++]=compiler->Build(*scene,state.settings,resolver,observer)&&Equal(reference,referenceBytes,compiler->Result().data,compiler->Result().size);
 state.reentry=true;state.clearWhileRunning=true;state.reentryRejected=false;checks[count++]=compiler->Build(*scene,state.settings,resolver,observer)&&state.reentryRejected&&compiler->Ready()&&compiler->Error().code==NativeRenderCompiler::Ok&&Equal(reference,referenceBytes,compiler->Result().data,compiler->Result().size);state.reentry=state.clearWhileRunning=false;
 state.cancellation=true;checks[count++]=!compiler->Build(*scene,state.settings,resolver,observer)&&Empty(*compiler)&&compiler->Error().code==NativeRenderCompiler::Cancelled;state.cancellation=false;
 state.cancelStage=NativeRenderCompiler::MaterialLayouts;checks[count++]=!compiler->Build(*scene,state.settings,resolver,observer)&&Empty(*compiler)&&compiler->Error().code==NativeRenderCompiler::Cancelled&&compiler->Error().stage==NativeRenderCompiler::MaterialLayouts;state.cancellation=false;state.cancelStage=0xffffffffu;
 state.cancelOnResolve=true;checks[count++]=!compiler->Build(*scene,state.settings,resolver,observer)&&Empty(*compiler)&&compiler->Error().code==NativeRenderCompiler::Cancelled;state.cancellation=false;state.cancelOnResolve=false;
 state.unsupported=true;checks[count++]=!compiler->Build(*scene,state.settings,resolver,observer)&&Empty(*compiler)&&compiler->Error().code==NativeRenderCompiler::MaterialFailure;state.unsupported=false;
 state.invalidState=true;checks[count++]=!compiler->Build(*scene,state.settings,resolver,observer)&&Empty(*compiler)&&compiler->Error().code==NativeRenderCompiler::MaterialFailure;state.invalidState=false;
 state.emptyFailed=true;checks[count++]=compiler->Build(*scene,state.settings,resolver,observer)&&compiler->Ready()&&compiler->Statistics().shaders==1;state.emptyFailed=false;
 state.emptyReady=true;checks[count++]=!compiler->Build(*scene,state.settings,resolver,observer)&&Empty(*compiler)&&compiler->Error().code==NativeRenderCompiler::MaterialFailure;state.emptyReady=false;
 state.nullResult=true;checks[count++]=!compiler->Build(*scene,state.settings,resolver,observer)&&Empty(*compiler)&&compiler->Error().code==NativeRenderCompiler::MaterialFailure;state.nullResult=false;
 state.tooMany=true;checks[count++]=!compiler->Build(*scene,state.settings,resolver,observer)&&Empty(*compiler)&&compiler->Error().code==NativeRenderCompiler::MaterialFailure;state.tooMany=false;
 state.oversized=true;checks[count++]=!compiler->Build(*scene,state.settings,resolver,observer)&&Empty(*compiler)&&compiler->Error().code==NativeRenderCompiler::MaterialFailure;state.oversized=false;
 state.settings.version=2;checks[count++]=!compiler->Build(*scene,state.settings,resolver,observer)&&Empty(*compiler)&&compiler->Error().code==NativeRenderCompiler::InvalidArgument;state.settings.version=1;
 state.settings.ownedBudget=1024u*1024u;checks[count++]=!compiler->Build(*scene,state.settings,resolver,observer)&&Empty(*compiler)&&compiler->Error().code==NativeRenderCompiler::Allocation;state.settings.ownedBudget=128u*1024u*1024u;
 checks[count++]=compiler->Build(*scene,state.settings,resolver,observer)&&Equal(reference,referenceBytes,compiler->Result().data,compiler->Result().size);
 compiler->Clear();checks[count++]=Empty(*compiler)&&compiler->Error().code==NativeRenderCompiler::Ok;
 HANDLE file=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(file==INVALID_HANDLE_VALUE)ExitProcess(5);
 unsigned int header[4]={0x3147434e,count,referenceBytes,state.calls};bool good=Write(file,header,16)&&Write(file,checks,count*4)&&Write(file,reference,referenceBytes);CloseHandle(file);
 bool passed=good;for(unsigned int i=0;i<count;i++)if(!checks[i])passed=false;
 compiler->~Builder();scene->~Context();cache->~Cache();ExitProcess(passed?0:6);
}
