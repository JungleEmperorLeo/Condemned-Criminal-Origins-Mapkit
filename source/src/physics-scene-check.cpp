#include "WorldPacker.cpp"
#include "RenderSceneProfile.h"
#include "PhysicsSceneMesh.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
extern "C" void HarnessEntry(){
 WorldText::Vec3 offset={};char value[64];const char* names[3]={"FEAR_PHYSICS_OFFSET_X","FEAR_PHYSICS_OFFSET_Y","FEAR_PHYSICS_OFFSET_Z"};float* coordinates=&offset.x;
 for(unsigned int i=0;i<3;i++){DWORD length=GetEnvironmentVariableA(names[i],value,64);if(!length)continue;if(length>=64)ExitProcess(7);WorldText::Token tokens[2]={};tokens[1].length=length;WorldText::Document doc;doc.source=value;doc.tokens=tokens;bool valid=doc.Number(1,coordinates[i]);doc.tokens=0;if(!valid)ExitProcess(7);}
 char input[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_READER_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_PHYSICS_SCENE_MESH",output,MAX_PATH))ExitProcess(1);PrefabScene::Source source;if(!source.Load(input))ExitProcess(2);void* sceneStorage=VirtualAlloc(0,sizeof(PrefabScene::Expansion),MEM_COMMIT|MEM_RESERVE,4);if(!sceneStorage)ExitProcess(3);PrefabScene::Expansion* expanded=new(sceneStorage) PrefabScene::Expansion;if(!expanded->Build(input,source.scene))ExitProcess(3);WorldModels::Partition partition;if(!partition.Build(expanded->Scene(),Resolve,0))ExitProcess(4);PhysicsSceneMesh::MainWorld mesh;if(!mesh.Build(partition,offset))ExitProcess(5);HANDLE f=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);DWORD done;if(f==INVALID_HANDLE_VALUE||!WriteFile(f,mesh.prefix.data,mesh.prefix.size,&done,0)||done!=mesh.prefix.size)ExitProcess(6);CloseHandle(f);ExitProcess(0);
}

