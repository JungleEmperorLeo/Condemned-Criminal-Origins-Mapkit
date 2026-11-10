#define FEAR_PRIMARY_SURFACE_PREPARATION 1
#include "WorldPacker.cpp"
#include "SourceObjectInventory.h"
#include "SourceRuntimeObjectOrder.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
static bool Write(HANDLE file,const void* data,unsigned int bytes){DWORD done=0;return !bytes||(WriteFile(file,data,bytes,&done,0)&&done==bytes);}
extern "C" void HarnessEntry(){
 char source[MAX_PATH],project[MAX_PATH],output[MAX_PATH];DWORD a=GetEnvironmentVariableA("FEAR_READER_INPUT",source,MAX_PATH),b=GetEnvironmentVariableA("FEAR_READER_PROJECT",project,MAX_PATH),c=GetEnvironmentVariableA("FEAR_SOURCE_OBJECT_INVENTORY",output,MAX_PATH);if(!a||a>=MAX_PATH||!b||b>=MAX_PATH||!c||c>=MAX_PATH)ExitProcess(1);
 PrefabScene::Arena memory;void* place=memory.Allocate(sizeof(NativePreparedRenderScene::Context));if(!place)ExitProcess(2);NativePreparedRenderScene::Context* scene=new(place)NativePreparedRenderScene::Context;if(!scene->Build(source,project)){LogLine(scene->Diagnostic().message);ExitProcess(2);}
 SourceObjectInventory::Builder inventory;SourceRuntimeObjectOrder::Builder order;if(!inventory.Build(*scene)||!order.Build(*scene)){LogLine(inventory.error?inventory.error:order.error);ExitProcess(3);}WorldText::Vec3 offset=scene->Offset();scene->Clear();scene->~Context();
 const unsigned int header[6]={0x31494f53,2,inventory.count,inventory.candidates,inventory.noRuntime,inventory.consumedCandidates};
 HANDLE file=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(file==INVALID_HANDLE_VALUE)ExitProcess(4);bool ok=Write(file,header,sizeof(header))&&Write(file,&offset,12);
 for(unsigned int i=0;ok&&i<inventory.count;i++){const SourceObjectInventory::Row& row=inventory.rows[i];const unsigned int fields[10]={row.sourceNode,row.classIndex,row.classFlags,row.authoredProperties,row.consumedCategory,row.candidateIndex,row.sourceParent,row.sourceId,row.importedFromPrefab,row.sourceKind};ok=Write(file,fields,40)&&Write(file,row.name,256)&&Write(file,row.className,64);}
 CloseHandle(file);if(!ok)ExitProcess(4);
 // A separate owned stream exercises the actual source ordering API, rather
 // than sorting diagnostic rows in the comparison process.
 char orderPath[MAX_PATH];DWORD length=GetEnvironmentVariableA("FEAR_SOURCE_RUNTIME_OBJECT_ORDER",orderPath,MAX_PATH);if(length>=MAX_PATH)ExitProcess(1);if(length){HANDLE ordered=CreateFileA(orderPath,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(ordered==INVALID_HANDLE_VALUE)ExitProcess(4);const unsigned int h[7]={0x314f4f53,1,order.count,order.sourceObjects,order.excludedNoRuntime,order.authoredCount,order.importedCount};ok=Write(ordered,h,28);for(unsigned int i=0;ok&&i<order.count;i++){const SourceRuntimeObjectOrder::Row& row=order.rows[i];const unsigned int fields[3]={row.sourceNode,row.candidateIndex,row.authoredProperties};ok=Write(ordered,fields,12)&&Write(ordered,row.name,256)&&Write(ordered,row.className,64);}CloseHandle(ordered);if(!ok)ExitProcess(4);}
 ExitProcess(0);
}
