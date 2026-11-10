// Compile the same implementation directly; serialize only after every input
// Document, Scene, source allocation and LTC decoder has been destroyed.
#include "WorldPacker.cpp"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
extern "C" void HarnessEntry(){
 char input[MAX_PATH],output[MAX_PATH];
 if(!GetEnvironmentVariableA("FEAR_READER_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_MODEL_REPORT",output,MAX_PATH))ExitProcess(3);
 void* memory=VirtualAlloc(0,sizeof(PrefabScene::Expansion),MEM_COMMIT|MEM_RESERVE,4);if(!memory)ExitProcess(4);
 PrefabScene::Expansion* expansion=new(memory) PrefabScene::Expansion;bool built=false;
 {
  PrefabScene::Source source;if(!source.Load(input)){HANDLE diagnostic=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);DWORD done;const char* message=source.scene.doc.error?source.scene.doc.error:"Unknown source error";WriteFile(diagnostic,message,lstrlenA(message),&done,0);CloseHandle(diagnostic);ExitProcess(5);}
  char rawReport[MAX_PATH];if(GetEnvironmentVariableA("FEAR_RAW_REPORT",rawReport,MAX_PATH)){HANDLE f=CreateFileA(rawReport,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);DWORD done;unsigned int stats[]={source.scene.doc.count,source.scene.nodeCount,source.scene.brushCount,source.scene.objectCount,source.scene.prefabCount,source.scene.activePoints,source.scene.activePolygons};if(f==INVALID_HANDLE_VALUE||!WriteFile(f,stats,sizeof(stats),&done,0)||done!=sizeof(stats))ExitProcess(10);CloseHandle(f);}
  built=expansion->Build(input,source.scene);
 }
 // No pointers into any source are permitted beyond this boundary.
 expansion->SaveBuilt(output,built);
 const PrefabScene::Model& model=expansion->Scene();unsigned int index=0;
 for(PrefabScene::Node* n=model.first;n;n=n->next,index++){
  if(n->index!=index||(n->parent!=0xffffffffu&&n->parent>=index))ExitProcess(6);
  if(n->kind==1)for(PrefabScene::Node* other=n->next;other;other=other->next)if(other->kind==1){
   if(n->geometry.vertices==other->geometry.vertices||n->geometry.faces==other->geometry.faces||n->geometry.indices==other->geometry.indices)ExitProcess(9);
  }
 }
 if(index!=model.count)ExitProcess(7);
 for(unsigned int i=0;i<model.polygons;i++)if(!model.prepared[i].owner||model.prepared[i].polygonIndex>=model.prepared[i].owner->geometry.polygons)ExitProcess(8);
 expansion->~Expansion();VirtualFree(memory,0,MEM_RELEASE);ExitProcess(0);
}
