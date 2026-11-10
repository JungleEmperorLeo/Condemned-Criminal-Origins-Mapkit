#define FEAR_PRIMARY_SURFACE_PREPARATION 1
#include "WorldPacker.cpp"
#include "WorldCenter.h"
#include "GameClassMetadata.h"
#include "SourceExpandedRenderContours.h"
#include "SourceBlockerPolygons.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
#pragma comment(linker,"/alternatename:__imp__LoadLibraryExA@12=__imp__LoadLibraryExA")
#pragma comment(linker,"/alternatename:__imp__FreeLibrary@4=__imp__FreeLibrary")
#pragma comment(linker,"/alternatename:__imp__VirtualQuery@12=__imp__VirtualQuery")
struct Context {GameClassMetadata::Catalog catalog;SourceExpandedRenderContours::Consumed consumed;};
struct State {PrefabScene::Project project;PrefabScene::Expansion expansion;Context context;WorldModels::Partition partition;SourceBlockerPolygons::Block block;State():expansion(true){}};
static WorldModels::ClassKind Resolve(const PrefabScene::Node& node,void* context){return GameClassMetadata::Catalog::Resolve(node,&((Context*)context)->catalog);}
static bool Ignore(const PrefabScene::Node& node,void* context){return ((Context*)context)->consumed.Ignore(node);}
extern "C" void HarnessEntry(){
 char input[MAX_PATH],projectPath[MAX_PATH],output[MAX_PATH];
 if(!GetEnvironmentVariableA("FEAR_READER_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_READER_PROJECT",projectPath,MAX_PATH)||!GetEnvironmentVariableA("FEAR_BLOCKER_OUTPUT",output,MAX_PATH))ExitProcess(1);
 PrefabScene::Arena harnessMemory;void* stateMemory=harnessMemory.Allocate(sizeof(State));if(!stateMemory)ExitProcess(8);State& state=*new(stateMemory)State;
 if(!state.project.LoadPath(projectPath))ExitProcess(2);
 void* sourceMemory=harnessMemory.Allocate(sizeof(PrefabScene::Source));if(!sourceMemory)ExitProcess(8);PrefabScene::Source* source=new(sourceMemory)PrefabScene::Source;
 if(!source->LoadExplicit(input,true,true)||!state.expansion.Build(input,source->scene,state.project,true,false))ExitProcess(3);source->~Source();
 if(!state.context.catalog.LoadProject(input)||!state.context.consumed.Build(state.expansion.Scene()))ExitProcess(4);
 if(!state.partition.Build(state.expansion.Scene(),Resolve,&state.context,Ignore))ExitProcess(5);
 WorldText::Vec3 offset;if(!WorldCenter::Compute(state.expansion.Scene(),offset))ExitProcess(6);
 SourceBlockerPolygons::Block& block=state.block;if(!block.Build(state.expansion.Scene(),state.partition,offset))ExitProcess(20+block.code);
 HANDLE file=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);DWORD done;
 if(file==INVALID_HANDLE_VALUE||!WriteFile(file,block.data,block.size,&done,0)||done!=block.size)ExitProcess(7);CloseHandle(file);ExitProcess(0);
}
