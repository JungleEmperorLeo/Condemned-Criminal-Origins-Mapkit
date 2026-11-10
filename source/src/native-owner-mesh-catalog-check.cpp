#define FEAR_PRIMARY_SURFACE_PREPARATION 1
#include "WorldPacker.cpp"
#include "NativePhysicsOwnerMeshTransport.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
static bool Write(const char* path,const void* data,unsigned int size){HANDLE f=CreateFileA(path,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);DWORD done=0;if(f==INVALID_HANDLE_VALUE)return false;bool ok=WriteFile(f,data,size,&done,0)&&done==size;CloseHandle(f);return ok;}
extern "C" void HarnessEntry(){namespace T=NativePhysicsOwnerMeshTransport;
 char source[MAX_PATH],project[MAX_PATH],output[MAX_PATH],owners[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_READER_INPUT",source,MAX_PATH)||!GetEnvironmentVariableA("FEAR_READER_PROJECT",project,MAX_PATH)||!GetEnvironmentVariableA("FEAR_OWNER_MESH_CATALOG",output,MAX_PATH)||!GetEnvironmentVariableA("FEAR_OWNER_MESH_IDENTITIES",owners,MAX_PATH))ExitProcess(1);
 PrefabScene::Arena memory;void* place=memory.Allocate(sizeof(NativePreparedRenderScene::Context));if(!place)ExitProcess(2);NativePreparedRenderScene::Context* scene=new(place)NativePreparedRenderScene::Context;if(!scene->Build(source,project)){LogLine(scene->Diagnostic().message);ExitProcess(2);}
 T::Catalog catalog;if(!catalog.Build(*scene)){LogLine(catalog.error);ExitProcess(3);}
 // Independent enumeration emits all retained owners, so skipped modes and
 // catalog model IDs can be checked outside the catalog implementation.
 WorldModelSharing::Identity sharing;if(!sharing.Build(scene->Model(),scene->Partition()))ExitProcess(4);unsigned int bytes=4+(scene->Partition().groupCount+1)*272;unsigned char* ids=(unsigned char*)memory.Allocate(bytes);if(!ids)ExitProcess(4);unsigned int n=0,at=4,modelIndex=1;
 for(unsigned int g=1;g<scene->Partition().groupCount;g++){
  if(!SourcePhysicalBspContours::RetainedGroup(scene->Model(),scene->Partition(),g)||sharing.representative[g]!=g)continue;const PrefabScene::Node& owner=*scene->Partition().groups[g].owner;unsigned int fields[4]={g,modelIndex++,(unsigned int)NativePhysicsProperties::Automatic(owner),owner.index};memcpy(ids+at,fields,16);at+=16;lstrcpynA((char*)ids+at,owner.name,256);at+=256;n++;
 }
 memcpy(ids,&n,4);
 if(catalog.count){T::Catalog limited;T::Settings limits;limits.maximumBytes=12;if(limited.Build(*scene,limits)||limited.errorCode!=T::ResourceLimit||limited.data||limited.first||limited.count||limited.size)ExitProcess(6);
  if(catalog.count>1){limits=T::Settings();limits.maximumRecords=1;if(limited.Build(*scene,limits)||limited.errorCode!=T::ResourceLimit||limited.data||limited.first||limited.count||limited.size)ExitProcess(6);}}
 scene->Clear();scene->~Context();if(!Write(owners,ids,at))ExitProcess(4);
 // Catalog storage survives source scene and every temporary prefix builder.
 unsigned int position=12,recordCount=0,meshBytes=0;for(const T::Record* r=catalog.first;r;r=r->next){if(recordCount==catalog.count||position>catalog.size||12>catalog.size-position||r->meshBytes>catalog.size-position-12||!T::detail::Equal(r->mesh,catalog.data+position+12,r->meshBytes))ExitProcess(5);position+=12+r->meshBytes;meshBytes+=r->meshBytes;recordCount++;}if(position!=catalog.size||recordCount!=catalog.count||meshBytes!=catalog.totalBytes)ExitProcess(5);
 unsigned char* copy=(unsigned char*)memory.Allocate(catalog.size);if(!copy||!catalog.WriteCatalog(copy,catalog.size)||!T::detail::Equal(copy,catalog.data,catalog.size))ExitProcess(5);
 if(catalog.WriteCatalog(copy,catalog.size-1)||catalog.WriteCatalog(catalog.data,catalog.size))ExitProcess(5);
 const unsigned int savedSize=catalog.size;catalog.Clear();if(!Write(output,copy,savedSize))ExitProcess(5);
 ExitProcess(0);
}
