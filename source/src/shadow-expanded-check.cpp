#include "WorldPacker.cpp"
#include "RenderSceneProfile.h"
#include "ShadowScene.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
static void Save(HANDLE f,const void* data,DWORD bytes){DWORD done;if(!WriteFile(f,data,bytes,&done,0)||done!=bytes)ExitProcess(8);}
extern "C" void HarnessEntry(){
 char input[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_READER_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_SHADOW_EXPANDED",output,MAX_PATH))ExitProcess(1);
 PrefabScene::Source source;if(!source.Load(input))ExitProcess(2);void* sceneStorage=VirtualAlloc(0,sizeof(PrefabScene::Expansion),MEM_COMMIT|MEM_RESERVE,4);if(!sceneStorage)ExitProcess(3);PrefabScene::Expansion* expanded=new(sceneStorage) PrefabScene::Expansion;if(!expanded->Build(input,source.scene))ExitProcess(3);
 const PrefabScene::Model& model=expanded->Scene();WorldModels::Partition partition;if(!partition.Build(model,Resolve,0))ExitProcess(4);
 ShadowScene::Groups groups;if(!groups.Build(model,partition,Assigned))ExitProcess(5);
 HANDLE f=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(f==INVALID_HANDLE_VALUE)ExitProcess(6);unsigned int header[3]={0x31534853,groups.count,partition.groupCount};Save(f,header,12);
 for(unsigned int i=0;i<groups.count;i++){const ShadowScene::Surface& s=groups.surfaces[i];unsigned int h[4]={s.tree,s.lod,s.count,s.triangles};Save(f,h,16);Save(f,s.vertices,s.count*24);Save(f,s.indices,s.triangles*6);}CloseHandle(f);ExitProcess(0);
}
