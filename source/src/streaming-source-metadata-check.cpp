#define FEAR_SOURCE_GEOMETRY_REPAIR_DEFAULT 1
#include "WorldPacker.cpp"
#include "VisibilityCompiler.h"
#include "StreamingSourceMetadata.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
extern "C" void HarnessEntry(){
 char input[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_READER_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_METADATA_OUTPUT",output,MAX_PATH))ExitProcess(1);
 PrefabScene::Source source;if(!source.Load(input,true))ExitProcess(2);
 void* expansionMemory=VirtualAlloc(0,sizeof(PrefabScene::Expansion),MEM_COMMIT|MEM_RESERVE,4);if(!expansionMemory)ExitProcess(3);PrefabScene::Expansion* scene=new(expansionMemory) PrefabScene::Expansion(true);if(!scene->Build(input,source.scene))ExitProcess(4);
 void* vb=VirtualAlloc(0,sizeof(VisibilityCompiler::Builder),MEM_COMMIT|MEM_RESERVE,4);if(!vb)ExitProcess(3);VisibilityCompiler::Builder* visibility=new(vb) VisibilityCompiler::Builder;if(!visibility->Build(scene->Scene()))ExitProcess(5);
 StreamingConvex::Block geometry;if(!geometry.Build(scene->Scene()))ExitProcess(6);StreamingSourceMetadata::Builder bridge;
 if(!bridge.Build(geometry,visibility->streamingSectors,visibility->sectorCount,visibility->offset))ExitProcess(7);
 HANDLE f=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);DWORD done;
 unsigned int header[]={geometry.regionCount,visibility->sectorCount,bridge.assignment.unassigned,bridge.assignment.ambiguous};
 if(f==INVALID_HANDLE_VALUE||!WriteFile(f,header,16,&done,0)||done!=16)ExitProcess(8);
 for(unsigned int i=0;i<geometry.regionCount;i++){const StreamingMetadata::References& refs=bridge.assignment.regions[i];if(!WriteFile(f,&refs.count,4,&done,0)||done!=4||(refs.count&&(!WriteFile(f,refs.indices,refs.count*4,&done,0)||done!=refs.count*4)))ExitProcess(8);}
 CloseHandle(f);
 WorldText::Vec3 wrong=visibility->offset;wrong.x+=256;if(bridge.Build(geometry,visibility->streamingSectors,visibility->sectorCount,wrong)||bridge.assignment.regions||bridge.assignment.owners)ExitProcess(9);
 StreamingMetadata::External* invalid=(StreamingMetadata::External*)VirtualAlloc(0,(visibility->sectorCount+1)*sizeof(StreamingMetadata::External),MEM_COMMIT|MEM_RESERVE,4);if(!invalid)ExitProcess(9);
 memcpy(invalid,visibility->streamingSectors,visibility->sectorCount*sizeof(StreamingMetadata::External));invalid[0].index=visibility->sectorCount;
 if(bridge.Build(geometry,invalid,visibility->sectorCount,visibility->offset)||bridge.assignment.regions||bridge.assignment.owners||!bridge.error)ExitProcess(9);
 if(visibility->sectorCount>1){memcpy(invalid,visibility->streamingSectors,visibility->sectorCount*sizeof(StreamingMetadata::External));invalid[1].index=invalid[0].index;if(bridge.Build(geometry,invalid,visibility->sectorCount,visibility->offset)||bridge.assignment.regions||bridge.assignment.owners||!bridge.error)ExitProcess(9);}
 if(!bridge.Build(geometry,visibility->streamingSectors,visibility->sectorCount,visibility->offset)||bridge.error)ExitProcess(9);VirtualFree(invalid,0,MEM_RELEASE);
 ExitProcess(0);
}

