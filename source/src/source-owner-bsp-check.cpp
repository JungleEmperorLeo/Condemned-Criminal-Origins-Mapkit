#define FEAR_PRIMARY_SURFACE_PREPARATION 1
#include "WorldPacker.cpp"
#include "NativePreparedRenderScene.h"
#include "SourcePhysicalBspOwners.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
static bool Write(HANDLE file,const void* data,unsigned int size){DWORD done=0;return WriteFile(file,data,size,&done,0)&&done==size;}
extern "C" void HarnessEntry(){
 char source[MAX_PATH],project[MAX_PATH],output[MAX_PATH];
 if(!GetEnvironmentVariableA("FEAR_READER_INPUT",source,MAX_PATH)||!GetEnvironmentVariableA("FEAR_READER_PROJECT",project,MAX_PATH)||!GetEnvironmentVariableA("FEAR_OWNER_BSP_OUTPUT",output,MAX_PATH))ExitProcess(1);
 PrefabScene::Arena memory;void* sceneStorage=memory.Allocate(sizeof(NativePreparedRenderScene::Context));if(!sceneStorage)ExitProcess(2);NativePreparedRenderScene::Context* scene=new(sceneStorage)NativePreparedRenderScene::Context;
 if(!scene->Build(source,project)){LogLine(scene->Diagnostic().message);ExitProcess(2);}
 SourcePhysicalBspOwners::Builder bsp;
 if(!bsp.Build(scene->Model(),scene->Partition(),scene->Offset(),scene->Project())){
  LogLine(bsp.error);char errorPath[MAX_PATH];lstrcpynA(errorPath,output,MAX_PATH-11);lstrcpyA(errorPath+lstrlenA(errorPath),".error.txt");HANDLE f=CreateFileA(errorPath,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(f!=INVALID_HANDLE_VALUE){Write(f,&bsp.failedGroup,4);if(bsp.error)Write(f,bsp.error,lstrlenA(bsp.error));CloseHandle(f);}ExitProcess(3);
 }
 // Release the scene before writing; every returned payload/name/normal is owned.
 scene->Clear();
 unsigned int h[8]={0x3153424fu,1,bsp.count,bsp.normalCount,bsp.aliases,bsp.omittedGroups,bsp.emptyCount,bsp.materials};
 HANDLE file=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(file==INVALID_HANDLE_VALUE||!Write(file,h,32)||!Write(file,bsp.normals,bsp.normalCount*12))ExitProcess(4);
 for(const SourcePhysicalBspOwners::Model* item=bsp.first;item;item=item->next){unsigned int nameBytes=lstrlenA(item->name)+1;unsigned int fields[8]={item->group,item->ownerNode,item->size,nameBytes,item->sourceContours,item->sourceReferences,item->splits,item->depth};if(!Write(file,fields,32)||!Write(file,item->name,nameBytes)||!Write(file,item->data,item->size))ExitProcess(4);}
 CloseHandle(file);scene->~Context();ExitProcess(0);
}
