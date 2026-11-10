#define FEAR_PRIMARY_SURFACE_PREPARATION 1
#include "WorldPacker.cpp"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
extern "C" void HarnessEntry(){
 char input[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_READER_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_COLOR_OUTPUT",output,MAX_PATH))ExitProcess(1);
 PrefabScene::Arena memory;void* expansionStorage=memory.Allocate(sizeof(PrefabScene::Expansion));if(!expansionStorage)ExitProcess(2);PrefabScene::Expansion* expansion=new(expansionStorage) PrefabScene::Expansion(true);
 {PrefabScene::Source source;if(!source.Load(input,true)||!expansion->Build(input,source.scene))ExitProcess(3);}
 HANDLE file=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(file==INVALID_HANDLE_VALUE)ExitProcess(4);
 for(const PrefabScene::Node* n=expansion->Scene().first;n;n=n->next)if(n->kind==1){
  for(unsigned int i=0;i<n->geometry.points;i++){
   unsigned int ids[2]={n->index,i};DWORD done;
   if(!n->geometry.colors||!WriteFile(file,ids,8,&done,0)||done!=8||!WriteFile(file,n->geometry.colors+i,4,&done,0)||done!=4)ExitProcess(5);
  }
 }CloseHandle(file);ExitProcess(0);
}
