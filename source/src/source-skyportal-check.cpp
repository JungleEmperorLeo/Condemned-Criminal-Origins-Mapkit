#include "WorldPacker.cpp"
#include "SourceSkyPortals.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
static bool Put(HANDLE f,const void* data,unsigned int bytes){DWORD done;return WriteFile(f,data,bytes,&done,0)&&done==bytes;}
extern "C" void HarnessEntry(){
 char input[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_READER_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_SKYPORTAL_OUTPUT",output,MAX_PATH))ExitProcess(1);
 PrefabScene::Arena memory;void* sceneStorage=memory.Allocate(sizeof(PrefabScene::Expansion));if(!sceneStorage)ExitProcess(2);PrefabScene::Expansion* expansion=new(sceneStorage) PrefabScene::Expansion;
 {PrefabScene::Source source;if(!source.Load(input)||!expansion->Build(input,source.scene))ExitProcess(3);}
 SourceSkyPortals::Builder builder;if(!builder.Build(expansion->Scene())){LogLine(builder.error);ExitProcess(4);}
 HANDLE f=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(f==INVALID_HANDLE_VALUE)ExitProcess(5);unsigned int header[4]={0x32504b53,builder.sourceCount,builder.sectorCount,builder.count};if(!Put(f,header,16)||!Put(f,&builder.offset,12))ExitProcess(6);
 for(unsigned int i=0;i<builder.count;i++){const SourceSkyPortals::Polygon& p=builder.polygons[i];unsigned int row[5]={p.sectorModelIndex,p.portalModelIndex,p.sourcePolygon,p.renderNode,p.portal.count};if(!Put(f,row,20)||!Put(f,p.portal.vertices,p.portal.count*12))ExitProcess(6);}CloseHandle(f);expansion->~Expansion();ExitProcess(0);
}
