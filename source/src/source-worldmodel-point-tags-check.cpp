#define FEAR_PRIMARY_SURFACE_PREPARATION 1
#include "WorldPacker.cpp"
#include "SourceRenderPointTags.h"
#include "WorldCenter.h"
#include "SourceWorldModelTransform.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
static WorldModels::ClassKind ResolveTagClass(const PrefabScene::Node& n,void* context){
 const char* classes=(const char*)context;char name[128];unsigned int used=0;
 for(const char* p=classes;;p++){if(*p==';'||!*p){name[used]=0;if(FearBrushFlags::Equal(name,n.className))return WorldModels::WorldModel;used=0;if(!*p)break;}else if(used<127)name[used++]=*p;else return WorldModels::Unknown;}
 return WorldModels::Unknown;
}
extern "C" void HarnessEntry(){
 char input[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_READER_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_SCENE_TAG_OUTPUT",output,MAX_PATH))ExitProcess(1);
 PrefabScene::Source source;if(!source.Load(input,true))ExitProcess(2);
 PrefabScene::Arena expansionMemory;void* expansionStorage=expansionMemory.Allocate(sizeof(PrefabScene::Expansion));if(!expansionStorage)ExitProcess(2);PrefabScene::Expansion* expanded=new(expansionStorage) PrefabScene::Expansion(true);if(!expanded->Build(input,source.scene))ExitProcess(2);WorldText::Vec3 offset={};if(!WorldCenter::Compute(expanded->Scene(),offset))ExitProcess(2);
 char classes[2048];if(!GetEnvironmentVariableA("FEAR_TAG_WORLD_CLASSES",classes,2048))ExitProcess(6);
 WorldModels::Partition partition;if(!partition.Build(expanded->Scene(),ResolveTagClass,classes))ExitProcess(6);
 PrefabScene::Arena ownerMemory;unsigned int* nodeGroups=(unsigned int*)ownerMemory.Allocate(expanded->Scene().count*4);if(!nodeGroups)ExitProcess(3);
 for(unsigned int i=0;i<expanded->Scene().count;i++)nodeGroups[i]=0xffffffffu;
 for(unsigned int i=0;i<partition.assignmentCount;i++)nodeGroups[partition.assignments[i].brush->index]=partition.assignments[i].group;
 HANDLE file=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);DWORD done;unsigned int header[4]={};if(file==INVALID_HANDLE_VALUE||!WriteFile(file,header,16,&done,0)||done!=16)ExitProcess(5);
 for(unsigned int group=0;group<partition.groupCount;group++){char qpath[MAX_PATH];if(partition.groups[group].owner&&GetEnvironmentVariableA("FEAR_TAG_QUAT_OUTPUT",qpath,MAX_PATH)){float q[4];if(!SourceWorldModelTransform::Quaternion(partition.groups[group].owner->orientation,q))ExitProcess(7);HANDLE qfile=CreateFileA(qpath,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(qfile==INVALID_HANDLE_VALUE||!WriteFile(qfile,q,16,&done,0)||done!=16)ExitProcess(7);CloseHandle(qfile);}
 unsigned int count=0,brushes=0;
 for(const PrefabScene::Node* node=expanded->Scene().first;node;node=node->next){if(nodeGroups[node->index]!=group||!SourceRenderPointTags::MainWorldCandidate(*node))continue;brushes++;const WorldText::Geometry& g=node->geometry;for(unsigned int p=0;p<g.polygons;p++)for(unsigned int f=0;f<SourceSurfacePreparation::FragmentCount(g.faces[p].count);f++){SourceSurfacePreparation::Fragment fragment;if(!SourceSurfacePreparation::FragmentAt(g.faces[p],f,fragment)||count>1000000-fragment.count)ExitProcess(3);count+=fragment.count;}}
 PrefabScene::Arena memory;WorldText::Vec3* points=(WorldText::Vec3*)memory.Allocate(count*12);unsigned int* rename=(unsigned int*)memory.Allocate(count*4);unsigned int* tags=(unsigned int*)memory.Allocate(count*4);unsigned int* copied=(unsigned int*)memory.Allocate(count*4);if(!points||!rename||!tags||!copied)ExitProcess(3);
 unsigned int at=0;unsigned int* visible=(unsigned int*)memory.Allocate(count*4);if(!visible)ExitProcess(3);unsigned int emitted=0;for(const PrefabScene::Node* node=expanded->Scene().first;node;node=node->next){if(nodeGroups[node->index]!=group||!SourceRenderPointTags::MainWorldCandidate(*node))continue;const WorldText::Geometry& g=node->geometry;for(unsigned int p=0;p<g.polygons;p++)for(unsigned int f=0;f<SourceSurfacePreparation::FragmentCount(g.faces[p].count);f++){SourceSurfacePreparation::Fragment fragment;if(!SourceSurfacePreparation::FragmentAt(g.faces[p],f,fragment))ExitProcess(3);for(unsigned int i=0;i<fragment.count;i++){if(fragment.indices[i]>=g.points)ExitProcess(3);WorldText::Vec3 point=g.vertices[fragment.indices[i]];point.x-=offset.x;point.y-=offset.y;point.z-=offset.z;
 if(partition.groups[group].owner){WorldText::Vec3 local;if(!SourceWorldModelTransform::Local(*partition.groups[group].owner,g.vertices[fragment.indices[i]],local))ExitProcess(3);point=local;}visible[at]=SourceRenderPointTags::RenderInputVisible(node->surfaceFlags);if(visible[at])emitted++;points[at++]=point;}}}
 SourceRenderPointTags::Indexer indexer;unsigned int generated=0;if(!indexer.Build(points,count)||!RenderPointRemap::Build(indexer.pointCount,indexer.indices,count,rename,tags,copied,count,generated))ExitProcess(4);
 header[0]+=brushes;header[1]+=emitted;header[2]+=indexer.pointCount;header[3]+=generated;
 for(unsigned int i=0;i<count;i++){if(!visible[i])continue;if(!WriteFile(file,tags+i,4,&done,0)||done!=4||!WriteFile(file,points+i,12,&done,0)||done!=12)ExitProcess(5);}
 }
 CloseHandle(file);file=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,3,FILE_ATTRIBUTE_NORMAL,0);if(file==INVALID_HANDLE_VALUE)ExitProcess(5);if(!WriteFile(file,header,16,&done,0)||done!=16)ExitProcess(5);CloseHandle(file);ExitProcess(0);
}

