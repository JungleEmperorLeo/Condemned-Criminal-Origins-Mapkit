#include "WorldPacker.cpp"
#include "OfficeClassProfile.h"
#include "WorldModelSharing.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
extern "C" void HarnessEntry(){
 char input[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_READER_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_SHARING_OUTPUT",output,MAX_PATH))ExitProcess(1);
 PrefabScene::Source source;if(!source.Load(input))ExitProcess(2);void* expansionStorage=VirtualAlloc(0,sizeof(PrefabScene::Expansion),MEM_COMMIT|MEM_RESERVE,4);if(!expansionStorage)ExitProcess(3);PrefabScene::Expansion* expanded=new(expansionStorage) PrefabScene::Expansion;if(!expanded->Build(input,source.scene))ExitProcess(3);
 WorldModels::Partition partition;if(!partition.Build(expanded->Scene(),ResolveOffice,0))ExitProcess(4);WorldModelSharing::Identity identity;if(!identity.Build(expanded->Scene(),partition))ExitProcess(5);
 PrefabScene::Arena memory;unsigned int bytes=12+(partition.groupCount-1)*264;unsigned char* data=(unsigned char*)memory.Allocate(bytes);if(!data)ExitProcess(6);((unsigned int*)data)[0]=0x31495357;((unsigned int*)data)[1]=partition.groupCount-1;((unsigned int*)data)[2]=identity.uniqueCount-1;
 for(unsigned int i=1;i<partition.groupCount;i++){unsigned char* row=data+12+(i-1)*264;*(unsigned int*)row=partition.groups[i].ownerIndex;*(unsigned int*)(row+4)=identity.serializedIndex[i];for(unsigned int k=0;k<256;k++)row[8+k]=0;lstrcpyA((char*)(row+8),partition.groups[i].owner->name);}
 HANDLE f=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);DWORD written;if(f==INVALID_HANDLE_VALUE||!WriteFile(f,data,bytes,&written,0)||written!=bytes)ExitProcess(7);CloseHandle(f);ExitProcess(0);
}
