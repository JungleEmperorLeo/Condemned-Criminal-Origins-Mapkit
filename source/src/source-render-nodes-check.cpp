#include "WorldPacker.cpp"
#include "SourceRenderNodes.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
static bool Put(HANDLE f,const void* data,unsigned int bytes){DWORD done;return WriteFile(f,data,bytes,&done,0)&&done==bytes;}
static bool Guards(const VisibilityCompiler::Builder& visibility,SourceSkyPortals::Builder& sky){
 SourceRenderNodes::Builder test;StreamingMetadata::External duplicate[2]={visibility.streamingSectors[0],visibility.streamingSectors[0]};WorldText::Vec3 offset=visibility.offset;
 if(test.Build(duplicate,2,offset)||test.nodes||test.layout.trees||test.tree.count||!test.error)return false;
 if(!test.Build(visibility,&sky))return false;
 offset.x+=1;if(test.Build(visibility.streamingSectors,visibility.sectorCount,offset,&sky)||test.nodes||test.layout.count||!test.error)return false;
 if(sky.count){unsigned int saved=sky.polygons[0].renderNode;sky.polygons[0].renderNode=visibility.sectorCount;bool rejected=!test.Build(visibility,&sky)&&!test.nodes&&!test.layout.trees&&test.error;sky.polygons[0].renderNode=saved;if(!rejected)return false;}
 return test.Build(visibility,&sky);
}
extern "C" void HarnessEntry(){
 char input[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_READER_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_RENDER_NODES_OUTPUT",output,MAX_PATH))ExitProcess(1);
 PrefabScene::Arena memory;void* sceneStorage=memory.Allocate(sizeof(PrefabScene::Expansion));if(!sceneStorage)ExitProcess(2);PrefabScene::Expansion* expansion=new(sceneStorage) PrefabScene::Expansion;
 {PrefabScene::Source source;if(!source.Load(input)||!expansion->Build(input,source.scene))ExitProcess(3);}
 // Visibility builders are large: allocate outside the small no-CRT stack.
 void* visibilityStorage=memory.Allocate(sizeof(VisibilityCompiler::Builder));if(!visibilityStorage)ExitProcess(2);VisibilityCompiler::Builder* visibility=new(visibilityStorage) VisibilityCompiler::Builder;
 if(!visibility->Build(expansion->Scene())){LogLine("Source render sector preparation failed");ExitProcess(4);}
 SourceSkyPortals::Builder sky;if(!sky.Build(expansion->Scene())){LogLine(sky.error);ExitProcess(4);}
 SourceRenderNodes::Builder builder;if(!builder.Build(*visibility,&sky)){LogLine(builder.error);ExitProcess(4);}if(!Guards(*visibility,sky))ExitProcess(7);
 RenderTrees::Block block;if(!block.Build(builder.layout,1)){LogLine(block.error);ExitProcess(4);}
 HANDLE f=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(f==INVALID_HANDLE_VALUE)ExitProcess(5);
 unsigned int header[4]={0x314e5253,builder.tree.count,builder.skyPortalCount,block.size};if(!Put(f,header,16)||!Put(f,&visibility->offset,12))ExitProcess(6);
 for(unsigned int i=0;i<builder.tree.count;i++){const SourceRenderNodes::Sector& s=builder.sectors[i];unsigned int ids[2]={s.constructionIndex,i};if(!Put(f,ids,8)||!Put(f,&s.low,12)||!Put(f,&s.high,12))ExitProcess(6);}
 if(!Put(f,block.data,block.size))ExitProcess(6);CloseHandle(f);visibility->~Builder();expansion->~Expansion();ExitProcess(0);
}
