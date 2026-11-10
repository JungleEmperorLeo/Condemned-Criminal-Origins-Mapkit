#define FEAR_PRIMARY_SURFACE_PREPARATION 1
#include "WorldPacker.cpp"
#include "NativePreparedRenderScene.h"
#include "SourcePhysicalBspContours.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
static bool Write(HANDLE file,const void* data,unsigned int size){DWORD done=0;return WriteFile(file,data,size,&done,0)&&done==size;}
extern "C" void HarnessEntry(){
 char source[MAX_PATH],project[MAX_PATH],output[MAX_PATH];
 if(!GetEnvironmentVariableA("FEAR_READER_INPUT",source,MAX_PATH)||!GetEnvironmentVariableA("FEAR_READER_PROJECT",project,MAX_PATH)||!GetEnvironmentVariableA("FEAR_SOURCE_BSP_PROVENANCE_OUTPUT",output,MAX_PATH))ExitProcess(1);
 PrefabScene::Arena memory;void* sceneStorage=memory.Allocate(sizeof(NativePreparedRenderScene::Context));if(!sceneStorage)ExitProcess(2);NativePreparedRenderScene::Context* scene=new(sceneStorage)NativePreparedRenderScene::Context;
 if(!scene->Build(source,project)){LogLine(scene->Diagnostic().message);ExitProcess(2);}
 SourceBlockerPlanePool::Prepared planes;if(!planes.Build(scene->Model(),scene->Partition(),scene->Offset())){LogLine(planes.error);ExitProcess(3);}
 SourcePhysicalBspContours::Main contours;
 if(!contours.BuildGroup(scene->Model(),scene->Partition(),0,planes,256,500000,4000000)){LogLine(contours.error);ExitProcess(3);}
 const PrefabScene::Model& model=scene->Model();const PrefabScene::Node** nodes=(const PrefabScene::Node**)memory.Allocate((model.count+1)*sizeof(void*));if(!nodes)ExitProcess(3);
 unsigned int at=0;for(const PrefabScene::Node* node=model.first;node;node=node->next){if(node->index!=at||at>=model.count)ExitProcess(3);nodes[at++]=node;}if(at!=model.count)ExitProcess(3);
 // Source-only diagnostic: no compiled BSP, capture or original module is read.
 unsigned int header[8]={0x31505342u,1,contours.count,contours.references,model.count,model.brushes,model.points,model.polygons};
 HANDLE file=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(file==INVALID_HANDLE_VALUE||!Write(file,header,32)||!Write(file,&scene->Offset(),12))ExitProcess(4);
 for(unsigned int i=0;i<contours.count;i++){
  const SourcePhysicalBspContours::Contour& contour=contours.contours[i];if(contour.sourceNode>=model.count)ExitProcess(4);const PrefabScene::Node& node=*nodes[contour.sourceNode];if(contour.sourcePolygon>=node.geometry.polygons)ExitProcess(4);
  const unsigned int nameBytes=lstrlenA(node.name)+1,pathBytes=lstrlenA(node.source)+1;
  if(!contour.polygon.flags&&!contour.sourcePolygon&&!contour.sourceFragment){LogLine("FLAG0 SOURCE BRUSH");LogLine(node.name);LogLine(node.type);}
  unsigned int fields[10]={contour.polygon.flags,contour.sourceNode,contour.sourcePolygon,contour.sourceFragment,contour.polygon.count,node.geometry.faces[contour.sourcePolygon].count,node.sourceId,node.importedFromPrefab?1u:0u,nameBytes,pathBytes};
  if(!Write(file,fields,40)||!Write(file,&contour.polygon.normal,12)||!Write(file,&contour.polygon.distance,4)||!Write(file,node.name,nameBytes)||!Write(file,node.source,pathBytes)||!Write(file,contour.polygon.points,contour.polygon.count*12))ExitProcess(4);
 }
 CloseHandle(file);contours.Clear();planes.~Prepared();scene->~Context();ExitProcess(0);
}
