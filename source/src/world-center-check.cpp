#include "WorldPacker.cpp"
#include "WorldCenter.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
static bool Read(HANDLE f,void* data,unsigned int size){DWORD done;return ReadFile(f,data,size,&done,0)&&done==size;}
extern "C" void HarnessEntry(){
 char input[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_WORLD_OFFSET_OUTPUT",output,MAX_PATH))ExitProcess(1);PrefabScene::Arena memory;unsigned char* data=0;unsigned int size=0;
 if(GetEnvironmentVariableA("FEAR_WORLD_CENTER_INPUT",input,MAX_PATH)){
  HANDLE f=CreateFileA(input,0x80000000,FILE_SHARE_READ,0,3,FILE_ATTRIBUTE_NORMAL,0);unsigned int header[2]={};if(f==INVALID_HANDLE_VALUE||!Read(f,header,8)||header[0]!=0x31435357||header[1]>8192)ExitProcess(2);size=4+header[1]*4;data=(unsigned char*)memory.Allocate(size);if(!data)ExitProcess(3);*(unsigned int*)data=header[1];
  for(unsigned int i=0;i<header[1];i++){float value=0,result=0;if(!Read(f,&value,4))ExitProcess(2);if(!WorldCenter::Snap(value,result))ExitProcess(4);((float*)(data+4))[i]=result;}
  unsigned char extra;DWORD done;if(!ReadFile(f,&extra,1,&done,0)||done)ExitProcess(2);CloseHandle(f);
 }else{
  if(!GetEnvironmentVariableA("FEAR_READER_INPUT",input,MAX_PATH))ExitProcess(1);PrefabScene::Source source;if(!source.Load(input))ExitProcess(2);void* expansionStorage=VirtualAlloc(0,sizeof(PrefabScene::Expansion),MEM_COMMIT|MEM_RESERVE,4);if(!expansionStorage)ExitProcess(3);PrefabScene::Expansion* expanded=new(expansionStorage) PrefabScene::Expansion;if(!expanded->Build(input,source.scene))ExitProcess(3);WorldText::Vec3 offset;if(!WorldCenter::Compute(expanded->Scene(),offset))ExitProcess(4);size=12;data=(unsigned char*)memory.Allocate(size);if(!data)ExitProcess(3);for(unsigned int i=0;i<12;i++)data[i]=((unsigned char*)&offset)[i];
 }
 HANDLE f=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);DWORD done;if(f==INVALID_HANDLE_VALUE||!WriteFile(f,data,size,&done,0)||done!=size)ExitProcess(5);CloseHandle(f);ExitProcess(0);
}
