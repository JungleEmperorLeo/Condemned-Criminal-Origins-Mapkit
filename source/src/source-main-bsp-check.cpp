#define FEAR_PRIMARY_SURFACE_PREPARATION 1
#include "WorldPacker.cpp"
#include "NativePreparedRenderScene.h"
#include "SourcePhysicalBspContours.h"
#include "SourcePhysicalBspPointTable.h"
#include "SourcePhysicalBspMain.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
static bool Write(HANDLE file,const void* data,unsigned int size){DWORD done=0;return WriteFile(file,data,size,&done,0)&&done==size;}
extern "C" void HarnessEntry(){
 char source[MAX_PATH],project[MAX_PATH],output[MAX_PATH];
 if(!GetEnvironmentVariableA("FEAR_READER_INPUT",source,MAX_PATH)||!GetEnvironmentVariableA("FEAR_READER_PROJECT",project,MAX_PATH)||!GetEnvironmentVariableA("FEAR_MAIN_BSP_OUTPUT",output,MAX_PATH))ExitProcess(1);
 PrefabScene::Arena memory;void* sceneStorage=memory.Allocate(sizeof(NativePreparedRenderScene::Context));if(!sceneStorage)ExitProcess(2);NativePreparedRenderScene::Context* scene=new(sceneStorage)NativePreparedRenderScene::Context;
 if(!scene->Build(source,project)){LogLine(scene->Diagnostic().message);ExitProcess(2);}
 SourcePhysicalBspContours::Main main;
 if(!main.Build(scene->Model(),scene->Partition(),scene->Offset())){
  LogLine(main.error);char diagnosticPath[MAX_PATH];if(lstrlenA(output)+10<MAX_PATH){lstrcpyA(diagnosticPath,output);lstrcpyA(diagnosticPath+lstrlenA(output),".error.txt");HANDLE diagnosticFile=CreateFileA(diagnosticPath,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(diagnosticFile!=INVALID_HANDLE_VALUE){if(main.error)Write(diagnosticFile,main.error,lstrlenA(main.error));CloseHandle(diagnosticFile);}}
  ExitProcess(3);
 }
 const FearBspTree::Builder& tree=main.Tree();
 SourcePhysicalBspPointTable::Table points;if(!points.Build(main))ExitProcess(5);
 SourcePhysicalBspNormals::Table normals;if(!normals.Build(scene->Model(),scene->Partition(),scene->Offset()))ExitProcess(6);
 SourcePhysicalBspMain::Builder disk;if(!disk.Build(scene->Model(),scene->Partition(),scene->Offset(),scene->Project())){LogLine(disk.error);char diagnosticPath[MAX_PATH];if(lstrlenA(output)+10<MAX_PATH){lstrcpyA(diagnosticPath,output);lstrcpyA(diagnosticPath+lstrlenA(output),".error.txt");HANDLE df=CreateFileA(diagnosticPath,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(df!=INVALID_HANDLE_VALUE){Write(df,disk.error,lstrlenA(disk.error));CloseHandle(df);}}ExitProcess(7);}
#ifdef FEAR_MAIN_BSP_GUARDS
 unsigned int guards=0;unsigned int size=disk.size;unsigned char* saved=(unsigned char*)memory.Allocate(size);if(!saved)ExitProcess(20);memcpy(saved,disk.data,size);
 PrefabScene::Project emptyProject;if(disk.Build(scene->Model(),scene->Partition(),scene->Offset(),emptyProject)||disk.data||disk.size||disk.normalTable||!disk.error)ExitProcess(21);guards++;
 if(!disk.Build(scene->Model(),scene->Partition(),scene->Offset(),scene->Project())||disk.size!=size)ExitProcess(22);for(unsigned int i=0;i<size;i++)if(saved[i]!=disk.data[i])ExitProcess(22);guards++;
 SourcePhysicalBspMetadata::Resolver resolver;WorldText::Polygon test={};SourcePhysicalBspMetadata::Value value={};if(!resolver.Initialize(scene->Project()))ExitProcess(23);
 lstrcpyA(test.material,"__missing_bsp_material_guard__.Mat00");if(resolver.Resolve(test,value)||!resolver.error)ExitProcess(23);guards++;
 lstrcpyA(test.material,"..\\__outside_bsp_material_guard__.Mat00");if(resolver.Resolve(test,value)||!resolver.error)ExitProcess(24);guards++;
 test.material[0]=0;lstrcpyA(test.occlusion,"__unknown_bsp_occlusion_guard__");if(!resolver.Resolve(test,value)||value.occlusion!=255||value.material!=0)ExitProcess(25);guards++;
 test.occlusion[0]=0;if(!resolver.Resolve(test,value)||value.occlusion!=255)ExitProcess(26);guards++;
 SoundDatabaseResources::Reader lookup;if(!lookup.Initialize(scene->Project()))ExitProcess(30);HCATEGORY category=lookup.Category("Sound/Occlusion");unsigned int occlusions=lookup.RecordCount(category);
 if(!occlusions||occlusions>255)ExitProcess(31);
 for(unsigned int i=0;i<occlusions;i++){const char* name=lookup.RecordName(lookup.RecordAt(category,i));if(!name||lstrlenA(name)>=260)ExitProcess(32);lstrcpyA(test.occlusion,name);if(!resolver.Resolve(test,value)||value.occlusion!=i)ExitProcess(32);for(unsigned int k=0;test.occlusion[k];k++){char c=test.occlusion[k];if(c>='A'&&c<='Z')test.occlusion[k]=(char)(c+32);else if(c>='a'&&c<='z')test.occlusion[k]=(char)(c-32);}if(!resolver.Resolve(test,value)||value.occlusion!=i)ExitProcess(33);}guards++;
 scene->Clear();if(disk.size!=size||!disk.data||!disk.normalTable)ExitProcess(27);for(unsigned int i=0;i<size;i++)if(saved[i]!=disk.data[i])ExitProcess(27);guards++;
 disk.Clear();if(disk.data||disk.normalTable||disk.size||disk.error)ExitProcess(28);guards++;
 HANDLE guardFile=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(guardFile==INVALID_HANDLE_VALUE||!Write(guardFile,&guards,4))ExitProcess(29);CloseHandle(guardFile);scene->~Context();ExitProcess(0);
#endif
 // This diagnostic owns a source contour table and postprocessed node links.
 // It is not a physical BSP disk block or a whole-world result.
 unsigned int header[9]={0x3153424du,2,main.count,tree.nodeCount,(unsigned int)tree.root,tree.inputCount,tree.splits,tree.maxDepth,main.references};
 HANDLE file=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);
 if(file==INVALID_HANDLE_VALUE||!Write(file,header,sizeof(header)))ExitProcess(4);
 for(unsigned int i=0;i<main.count;i++){
  const SourcePhysicalBspContours::Contour& contour=main.contours[i];unsigned int fields[5]={contour.polygon.flags,contour.sourceNode,contour.sourcePolygon,contour.sourceFragment,contour.polygon.count};
  if(!Write(file,fields,20)||!Write(file,&contour.polygon.normal,12)||!Write(file,&contour.polygon.distance,4)||!Write(file,contour.polygon.points,contour.polygon.count*12))ExitProcess(4);
 }
 for(const FearBspTree::Node* node=tree.first;node;node=node->next){int fields[3]={(int)node->polygon->source,node->negative,node->positive};if(!Write(file,fields,12))ExitProcess(4);}
 unsigned int pointHeader[5]={0x31545042u,points.pointCount,points.referenceCount,points.initialPoints,points.internalPoints};
 if(!Write(file,pointHeader,20)||!Write(file,points.points,points.pointCount*12)||!Write(file,points.references,points.referenceCount*4))ExitProcess(4);
 unsigned int normalHeader[2]={0x314e5342u,normals.count};if(!Write(file,normalHeader,8)||!Write(file,normals.normals,normals.count*12))ExitProcess(4);
 unsigned int diskHeader[3]={0x31445342u,disk.size,disk.materials};if(!Write(file,diskHeader,12)||!Write(file,disk.data,disk.size))ExitProcess(4);
 CloseHandle(file);main.Clear();scene->~Context();ExitProcess(0);
}
