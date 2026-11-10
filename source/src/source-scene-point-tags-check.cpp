#define FEAR_PRIMARY_SURFACE_PREPARATION 1
#include "WorldPacker.cpp"
#include "SourceRenderPointTags.h"
#include "WorldCenter.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
extern "C" void HarnessEntry(){
 char input[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_READER_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_SCENE_TAG_OUTPUT",output,MAX_PATH))ExitProcess(1);
 PrefabScene::Source source;if(!source.Load(input,true))ExitProcess(2);
 PrefabScene::Arena expansionMemory;void* expansionStorage=expansionMemory.Allocate(sizeof(PrefabScene::Expansion));if(!expansionStorage)ExitProcess(2);PrefabScene::Expansion* expanded=new(expansionStorage) PrefabScene::Expansion(true);if(!expanded->Build(input,source.scene))ExitProcess(2);WorldText::Vec3 offset={};if(!WorldCenter::Compute(expanded->Scene(),offset))ExitProcess(2);
 unsigned int count=0,brushes=0;
 for(const PrefabScene::Node* node=expanded->Scene().first;node;node=node->next){if(!SourceRenderPointTags::MainWorldCandidate(*node))continue;brushes++;const WorldText::Geometry& g=node->geometry;for(unsigned int p=0;p<g.polygons;p++)for(unsigned int f=0;f<SourceSurfacePreparation::FragmentCount(g.faces[p].count);f++){SourceSurfacePreparation::Fragment fragment;if(!SourceSurfacePreparation::FragmentAt(g.faces[p],f,fragment)||count>1000000-fragment.count)ExitProcess(3);count+=fragment.count;}}
 PrefabScene::Arena memory;WorldText::Vec3* points=(WorldText::Vec3*)memory.Allocate(count*12);unsigned int* rename=(unsigned int*)memory.Allocate(count*4);unsigned int* tags=(unsigned int*)memory.Allocate(count*4);unsigned int* copied=(unsigned int*)memory.Allocate(count*4);if(!points||!rename||!tags||!copied)ExitProcess(3);
 unsigned int at=0;unsigned int* visible=(unsigned int*)memory.Allocate(count*4);if(!visible)ExitProcess(3);unsigned int emitted=0;for(const PrefabScene::Node* node=expanded->Scene().first;node;node=node->next){if(!SourceRenderPointTags::MainWorldCandidate(*node))continue;const WorldText::Geometry& g=node->geometry;for(unsigned int p=0;p<g.polygons;p++)for(unsigned int f=0;f<SourceSurfacePreparation::FragmentCount(g.faces[p].count);f++){SourceSurfacePreparation::Fragment fragment;if(!SourceSurfacePreparation::FragmentAt(g.faces[p],f,fragment))ExitProcess(3);for(unsigned int i=0;i<fragment.count;i++){if(fragment.indices[i]>=g.points)ExitProcess(3);WorldText::Vec3 point=g.vertices[fragment.indices[i]];point.x-=offset.x;point.y-=offset.y;point.z-=offset.z;visible[at]=SourceRenderPointTags::RenderInputVisible(node->surfaceFlags);if(visible[at])emitted++;points[at++]=point;}}}
 SourceRenderPointTags::Indexer indexer;unsigned int generated=0;if(!indexer.Build(points,count)||!RenderPointRemap::Build(indexer.pointCount,indexer.indices,count,rename,tags,copied,count,generated))ExitProcess(4);
 HANDLE file=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);DWORD done;unsigned int header[4]={brushes,emitted,indexer.pointCount,generated};if(file==INVALID_HANDLE_VALUE||!WriteFile(file,header,16,&done,0)||done!=16)ExitProcess(5);
 for(unsigned int i=0;i<count;i++){if(!visible[i])continue;if(!WriteFile(file,tags+i,4,&done,0)||done!=4||!WriteFile(file,points+i,12,&done,0)||done!=12)ExitProcess(5);}CloseHandle(file);ExitProcess(0);
}


