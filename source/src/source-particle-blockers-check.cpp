#define FEAR_PRIMARY_SURFACE_PREPARATION 1
#include "WorldPacker.cpp"
#include "WorldCenter.h"
#include "GameClassMetadata.h"
#include "SourceExpandedRenderContours.h"
#include "ParticleBlockers.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
extern "C" __declspec(dllimport) unsigned int WINAPI SetErrorMode(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
#pragma comment(linker,"/alternatename:__imp__SetErrorMode@4=__imp__SetErrorMode")
struct Context {GameClassMetadata::Catalog catalog;SourceExpandedRenderContours::Consumed consumed;};
static WorldModels::ClassKind Resolve(const PrefabScene::Node& node,void* context){return GameClassMetadata::Catalog::Resolve(node,&((Context*)context)->catalog);}
static bool Ignore(const PrefabScene::Node& node,void* context){return ((Context*)context)->consumed.Ignore(node);}
extern "C" void HarnessEntry(){
 SetErrorMode(3);char input[MAX_PATH],projectPath[MAX_PATH],output[MAX_PATH];
 if(!GetEnvironmentVariableA("FEAR_READER_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_READER_PROJECT",projectPath,MAX_PATH)||!GetEnvironmentVariableA("FEAR_PARTICLE_OUTPUT",output,MAX_PATH))ExitProcess(1);
 PrefabScene::Project project;if(!project.LoadPath(projectPath))ExitProcess(2);PrefabScene::Arena stageStorage;
 void* expansionStorage=stageStorage.Allocate(sizeof(PrefabScene::Expansion));if(!expansionStorage)ExitProcess(8);PrefabScene::Expansion* expansion=new(expansionStorage)PrefabScene::Expansion(true);
 {PrefabScene::Source source;if(!source.LoadExplicit(input,true,true)||!expansion->Build(input,source.scene,project,true,false))ExitProcess(3);}
 Context context;if(!context.catalog.LoadProject(input)||!context.consumed.Build(expansion->Scene()))ExitProcess(4);
 WorldModels::Partition partition;if(!partition.Build(expansion->Scene(),Resolve,&context,Ignore))ExitProcess(5);
 WorldText::Vec3 offset;if(!WorldCenter::Compute(expansion->Scene(),offset))ExitProcess(6);
 ParticleBlockers::Block block;if(!block.Build(expansion->Scene(),partition,offset))ExitProcess(20+block.code);
 HANDLE file=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);DWORD done;
 if(file==INVALID_HANDLE_VALUE||!WriteFile(file,block.data,block.size,&done,0)||done!=block.size)ExitProcess(7);CloseHandle(file);ExitProcess(0);
}
