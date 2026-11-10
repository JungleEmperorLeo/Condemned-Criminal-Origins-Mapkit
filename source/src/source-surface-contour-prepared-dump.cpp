#define FEAR_TEXTURE_ORIGIN_DOUBLE_EXPERIMENT 1
#define FEAR_TEXTURE_TRANSFORM_DOUBLE_EXPERIMENT 1
#include "WorldPacker.cpp"
#include "WorldCenter.h"
#include "SourceTextureBasis.h"
#include "TextureOriginMinimize.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
static bool Put(HANDLE f,const void* p,unsigned int n){DWORD done;return WriteFile(f,p,n,&done,0)&&done==n;}
extern "C" void HarnessEntry(){
 char input[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_READER_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_BASIS_OUTPUT",output,MAX_PATH))ExitProcess(1);
 PrefabScene::Arena memory;void* expansionStorage=memory.Allocate(sizeof(PrefabScene::Expansion));if(!expansionStorage)ExitProcess(2);PrefabScene::Expansion* expansion=new(expansionStorage) PrefabScene::Expansion(true);
 {PrefabScene::Source source;if(!source.Load(input,true)||!expansion->Build(input,source.scene))ExitProcess(3);}
 WorldText::Vec3 offset;if(!WorldCenter::Compute(expansion->Scene(),offset))ExitProcess(4);
 HANDLE f=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(f==INVALID_HANDLE_VALUE||!Put(f,&offset,12))ExitProcess(5);
 for(const PrefabScene::Node* node=expansion->Scene().first;node;node=node->next)if(node->kind==1){for(unsigned int i=0;i<node->geometry.polygons;i++){
 RenderTextureCoordinates::Basis bases[4];const WorldText::Polygon& face=node->geometry.faces[i];if(!SourceTextureBasis::Build(face,offset,bases))continue;WorldText::Vec3 points[128];if(face.count>128)ExitProcess(7);for(unsigned int j=0;j<face.count;j++){points[j]=node->geometry.vertices[face.indices[j]];points[j].x-=offset.x;points[j].y-=offset.y;points[j].z-=offset.z;}if(!TextureOriginMinimize::Build(points,face.count,bases))ExitProcess(8);unsigned int ids[3]={node->index,i,node->surfaceFlags};const WorldText::Polygon& polygon=node->geometry.faces[i];if(!Put(f,ids,12)||!Put(f,bases,144)||!Put(f,&polygon.count,4))ExitProcess(6);for(unsigned int j=0;j<polygon.count;j++){WorldText::Vec3 point=node->geometry.vertices[polygon.indices[j]];point.x-=offset.x;point.y-=offset.y;point.z-=offset.z;if(!Put(f,&point,12))ExitProcess(6);}
 }}CloseHandle(f);ExitProcess(0);
}



