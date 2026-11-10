#define FEAR_PRIMARY_SURFACE_PREPARATION 1
#include "PrimarySurfacePreparationConfig.h"
// Raw navigation has no texture fields. Keep the production geometry arithmetic
// and cleanup, while allowing authored raw-only fixtures without materials/UVs.
#undef FEAR_MISSING_TEXTURE_MAPPINGS_EXPERIMENT
#include "WorldPacker.cpp"
#include "NativePreparedNavSource.h"
#include "NativeRawNavMesh.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
static bool Put(HANDLE file,const void* data,unsigned int n){DWORD done=0;return WriteFile(file,data,n,&done,0)&&done==n;}
static void Fail(const char* output,const char* message){char path[MAX_PATH];unsigned int length=lstrlenA(output);if(length<MAX_PATH-11){lstrcpyA(path,output);lstrcpyA(path+length,".error.txt");HANDLE f=CreateFileA(path,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(f!=INVALID_HANDLE_VALUE){if(message)Put(f,message,lstrlenA(message));CloseHandle(f);}}ExitProcess(2);}
extern "C" void HarnessEntry(){char source[MAX_PATH],project[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_READER_INPUT",source,MAX_PATH)||!GetEnvironmentVariableA("FEAR_READER_PROJECT",project,MAX_PATH)||!GetEnvironmentVariableA("FEAR_RAW_NAVMESH_OUTPUT",output,MAX_PATH))ExitProcess(1);NativeRawNavMesh::Builder builder;WorldText::Vec3 offset={};
 // Use the same source preparation/centering as Objects/BlindData publication.
 // The former diagnostic loader skipped source SnapFloat preparation, so raw
 // from fractional hulls could not be accepted by the actual common worker.
 {PrefabScene::Arena memory;void* place=memory.Allocate(sizeof(NativePreparedNavSource::Context));if(!place)Fail(output,"Navigation source context allocation failed");NativePreparedNavSource::Context* scene=new(place)NativePreparedNavSource::Context;if(!scene->Build(source,project))Fail(output,scene->Error());offset=scene->Offset();if(!builder.Build(scene->Model(),offset))Fail(output,builder.error);scene->~Context();}
 unsigned int nameBytes=lstrlenA(builder.generatedLong.property);unsigned int header[10]={0x31564e52,1,builder.size,builder.sourceNode,builder.sourceId,builder.brushNode,builder.brushSourceId,builder.generatedLong.sourceNode,(unsigned int)builder.generatedLong.value,nameBytes};HANDLE file=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(file==INVALID_HANDLE_VALUE||!Put(file,header,sizeof(header))||!Put(file,&offset,12)||!Put(file,builder.generatedLong.property,nameBytes)||!Put(file,builder.data,builder.size))ExitProcess(3);CloseHandle(file);ExitProcess(0);
}
