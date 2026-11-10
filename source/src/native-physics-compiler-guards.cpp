#define FEAR_PRIMARY_SURFACE_PREPARATION 1
#include "WorldPacker.cpp"
#include "NativePhysicsCompiler.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
static bool Equal(const void* a,const void* b,unsigned int n){const unsigned char* x=(const unsigned char*)a;const unsigned char* y=(const unsigned char*)b;for(unsigned int i=0;i<n;i++)if(x[i]!=y[i])return false;return true;}
static bool Empty(const NativePhysicsCompiler::Builder& r){return !r.data&&!r.size&&!r.shapeCount&&!r.mainType&&!r.nonMainCount&&!r.mainExplicitCount&&!r.meshVertices&&!r.meshTriangles;}
static bool Write(HANDLE f,const void* p,unsigned int n){DWORD done=0;return !n||(WriteFile(f,p,n,&done,0)&&done==n);}
static WorldModels::ClassKind Ordinary(const PrefabScene::Node&,void*){return WorldModels::Ordinary;}
extern "C" void HarnessEntry(){
 char source[MAX_PATH],project[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_READER_INPUT",source,MAX_PATH)||!GetEnvironmentVariableA("FEAR_READER_PROJECT",project,MAX_PATH)||!GetEnvironmentVariableA("FEAR_PHYSICS_COMPILER_GUARDS_OUTPUT",output,MAX_PATH))ExitProcess(1);
 PrefabScene::Arena memory;void* at=memory.Allocate(sizeof(NativePreparedRenderScene::Context));if(!at)ExitProcess(2);NativePreparedRenderScene::Context* scene=new(at)NativePreparedRenderScene::Context;
 NativePhysicsCompiler::Builder result;NativePhysicsCompiler::MoppInput ticket;unsigned int checks[64]={},count=0;
#define CHECK(label,expression) checks[count++]=(expression)?1u:0u
 CHECK("fresh output empty",Empty(result)&&!result.error);
 CHECK("unprepared scene clears output",!result.Build(*scene,ticket)&&Empty(result)&&result.error);
 if(!scene->Build(source,project))ExitProcess(2);
 NativePhysicsMesh::Builder mesh;NativePhysicsOwners::Builder owners;NativePhysicsMainExplicit::Builder helpers;if(!mesh.Build(*scene)||!owners.Build(*scene)||!helpers.Build(*scene))ExitProcess(3);
 // Structurally valid leaf-only bytecode is an assembly fixture, never collision evidence.
 unsigned char mopp[21]={};float info[4]={0,0,0,1};memcpy(mopp,info,16);unsigned int length=1;memcpy(mopp+16,&length,4);mopp[20]=0x30;
 ticket.mesh=mesh.data;ticket.meshBytes=mesh.size;ticket.code=mopp;ticket.codeBytes=21;
 CHECK("source scene with explicit associated mesh builds full section",result.Build(*scene,ticket));
 CHECK("six source owners keep MAIN mesh and five nonMAIN records",result.shapeCount==6&&result.mainType==1&&result.nonMainCount==5&&!result.mainExplicitCount);
 CHECK("complete source payload size accounts for MOPP and owner records",result.size==mesh.size+21+owners.size);
 const unsigned int baselineSize=result.size;unsigned char* baseline=(unsigned char*)memory.Allocate(baselineSize);if(!baseline)ExitProcess(2);memcpy(baseline,result.data,baselineSize);
 CHECK("prepared pieces reproduce scene compilation",result.BuildParts(mesh,owners,helpers,ticket)&&result.size==baselineSize&&Equal(result.data,baseline,baselineSize));
 NativePhysicsCompiler::Settings settings;settings.version=0;
 CHECK("unknown settings version clears output",!result.BuildParts(mesh,owners,helpers,ticket,settings)&&Empty(result));settings.version=1;settings.maximumBytes=0;
 CHECK("zero byte budget rejected",!result.BuildParts(mesh,owners,helpers,ticket,settings)&&Empty(result));settings.maximumBytes=16u*1024u*1024u+1;
 CHECK("oversized byte budget rejected",!result.BuildParts(mesh,owners,helpers,ticket,settings)&&Empty(result));settings.maximumBytes=baselineSize-1;
 CHECK("one byte below required size clears all counters",!result.BuildParts(mesh,owners,helpers,ticket,settings)&&Empty(result));settings.maximumBytes=baselineSize;
 CHECK("exact byte budget succeeds",result.BuildParts(mesh,owners,helpers,ticket,settings)&&result.size==baselineSize);settings.maximumBytes=16u*1024u*1024u;
 ticket.meshBytes--;
 CHECK("mesh transport size mismatch rejected",!result.BuildParts(mesh,owners,helpers,ticket)&&Empty(result));ticket.meshBytes++;
 unsigned char* changed=(unsigned char*)memory.Allocate(mesh.size);if(!changed)ExitProcess(2);memcpy(changed,mesh.data,mesh.size);changed[mesh.size-1]^=1;ticket.mesh=changed;
 CHECK("stale different associated mesh rejected",!result.BuildParts(mesh,owners,helpers,ticket)&&Empty(result));ticket.mesh=0;
 CHECK("absent mesh association rejected",!result.BuildParts(mesh,owners,helpers,ticket)&&Empty(result));ticket.mesh=mesh.data;ticket.code=0;
 CHECK("absent MOPP rejected",!result.BuildParts(mesh,owners,helpers,ticket)&&Empty(result));ticket.code=mopp;ticket.codeBytes=20;
 CHECK("truncated MOPP framing rejected",!result.BuildParts(mesh,owners,helpers,ticket)&&Empty(result));ticket.codeBytes=21;length=2;memcpy(mopp+16,&length,4);
 CHECK("MOPP length field mismatch rejected",!result.BuildParts(mesh,owners,helpers,ticket)&&Empty(result));length=1;memcpy(mopp+16,&length,4);unsigned int nanBits=0x7fc00000u;memcpy(mopp,&nanBits,4);
 CHECK("nonfinite MOPP origin rejected",!result.BuildParts(mesh,owners,helpers,ticket)&&Empty(result));memcpy(mopp,info,16);float scale=0;memcpy(mopp+12,&scale,4);
 CHECK("nonpositive MOPP scale rejected",!result.BuildParts(mesh,owners,helpers,ticket)&&Empty(result));memcpy(mopp,info,16);
 CHECK("reuse after all transport errors exact",result.BuildParts(mesh,owners,helpers,ticket)&&Equal(result.data,baseline,baselineSize)&&!result.error);
 const unsigned int firstModel=owners.first->modelIndex;owners.first->modelIndex=7;
 CHECK("owner model index mismatch clears full output",!result.BuildParts(mesh,owners,helpers,ticket)&&Empty(result));owners.first->modelIndex=firstModel;owners.unownedExplicitShapes=1;
 CHECK("MAIN helper membership mismatch clears output",!result.BuildParts(mesh,owners,helpers,ticket)&&Empty(result));owners.unownedExplicitShapes=0;
 PrefabScene::Model synthetic;WorldModels::Partition partition;const char* classes[3]={"PhysicsShapeOBB","PhysicsShapeSphere","PhysicsShapeCapsule"};
 for(unsigned int i=0;i<3;i++){PrefabScene::Node* node=synthetic.Add();if(!node)ExitProcess(2);node->kind=node->sourceKind=2;node->parent=0xffffffffu;lstrcpyA(node->className,classes[i]);lstrcpyA(node->source,"guard-source.World00a");}
 if(!partition.Build(synthetic,Ordinary,0)||!helpers.BuildPrepared(synthetic,partition,WorldText::Vec3{}))ExitProcess(3);owners.unownedExplicitShapes=3;
 CHECK("MAIN mesh plus all three source primitive kinds assemble",result.BuildParts(mesh,owners,helpers,ticket)&&result.mainType==6&&result.mainExplicitCount==3&&result.shapeCount==6);
 CHECK("compound MAIN has four children and transformed mesh first",*(unsigned int*)(result.data+32)==6&&*(unsigned int*)(result.data+36)==4&&*(unsigned int*)(result.data+40)==5&&*(unsigned int*)(result.data+72)==1);
 CHECK("compound overhead and helper bytes fully accounted",result.size==mesh.size+21+owners.size+helpers.size+40);
 const unsigned int compoundSize=result.size;unsigned char* compound=(unsigned char*)memory.Allocate(compoundSize);if(!compound)ExitProcess(2);memcpy(compound,result.data,compoundSize);
 unsigned int savedBytes=helpers.first->bytes;helpers.first->bytes=59;
 CHECK("invalid explicit child size rejected atomically",!result.BuildParts(mesh,owners,helpers,ticket)&&Empty(result));helpers.first->bytes=savedBytes;
 CHECK("compound reuse restores exact serialized stream",result.BuildParts(mesh,owners,helpers,ticket)&&Equal(result.data,compound,compoundSize));
 scene->Clear();scene->~Context();partition.Clear();synthetic.Clear();mesh.Clear();owners.Clear();helpers.Clear();
 CHECK("final bytes survive all source scenes and component destruction",result.size==compoundSize&&Equal(result.data,compound,compoundSize));
 result.Clear();CHECK("Clear resets every output counter and diagnostic",Empty(result)&&!result.error);result.Clear();CHECK("repeated Clear remains empty",Empty(result)&&!result.error);
#undef CHECK
 HANDLE file=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(file==INVALID_HANDLE_VALUE)ExitProcess(5);unsigned int header[3]={0x31475043u,count,compoundSize};bool ok=Write(file,header,12)&&Write(file,checks,count*4)&&Write(file,compound,compoundSize);CloseHandle(file);for(unsigned int i=0;i<count;i++)if(!checks[i])ok=false;ExitProcess(ok?0:6);
}
