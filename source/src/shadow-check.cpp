#include "WorldPacker.cpp"
#include "ShadowMesh.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
static void Save(HANDLE f,const void* data,DWORD bytes){DWORD done;if(!WriteFile(f,data,bytes,&done,0)||done!=bytes)ExitProcess(7);}
extern "C" void HarnessEntry(){
 char input[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_SHADOW_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_SHADOW_OUTPUT",output,MAX_PATH))ExitProcess(1);
 HANDLE f=CreateFileA(input,0x80000000,FILE_SHARE_READ,0,3,FILE_ATTRIBUTE_NORMAL,0);if(f==INVALID_HANDLE_VALUE)ExitProcess(2);DWORD done;unsigned int header[2];WorldText::Vec3 offset;
 if(!ReadFile(f,header,8,&done,0)||done!=8||header[0]!=0x31574853||!header[1]||header[1]>6000)ExitProcess(3);
 if(!ReadFile(f,&offset,12,&done,0)||done!=12)ExitProcess(3);PrefabScene::Arena arena;ShadowMesh::Triangle* triangles=(ShadowMesh::Triangle*)arena.Allocate(header[1]*sizeof(ShadowMesh::Triangle));
 if(!triangles||!ReadFile(f,triangles,header[1]*sizeof(ShadowMesh::Triangle),&done,0)||done!=header[1]*sizeof(ShadowMesh::Triangle))ExitProcess(4);CloseHandle(f);
 ShadowMesh::Mesh mesh;if(!mesh.Build(triangles,header[1],offset))ExitProcess(5);f=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(f==INVALID_HANDLE_VALUE)ExitProcess(6);
 unsigned int counts[2]={mesh.count,mesh.triangles};Save(f,counts,8);Save(f,mesh.vertices,mesh.count*24);Save(f,mesh.indices,mesh.triangles*6);CloseHandle(f);ExitProcess(0);
}
