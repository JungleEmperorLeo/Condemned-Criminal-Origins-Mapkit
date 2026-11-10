#define FEAR_SOURCE_GEOMETRY_REPAIR_DEFAULT 1
#include "WorldPacker.cpp"
#include "WorldModels.h"
#include "GameClassMetadata.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
extern "C" __declspec(dllimport) unsigned int WINAPI SetErrorMode(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
#pragma comment(linker,"/alternatename:__imp__SetErrorMode@4=__imp__SetErrorMode")
static bool Path(const char* name,char* result){unsigned int n=GetEnvironmentVariableA(name,result,MAX_PATH);return n&&n<MAX_PATH;}
static bool Write(HANDLE file,const void* data,unsigned int bytes){DWORD done;return !bytes||(WriteFile(file,data,bytes,&done,0)&&done==bytes);}
static void Error(const char* message,unsigned int code){LogLine(message?message:"Unknown failure");ExitProcess(code);}
static void PureChecks(){
 GameClassMetadata::SDKClassDef a={},b={};a.name="Base";a.flags=16;b.name="Derived";b.parent=&a;
 GameClassMetadata::SDKClassDef* defs[2]={&a,&b};GameClassMetadata::Catalog catalog;PrefabScene::Node node={};
 if(!catalog.CopyDefinitions(defs,2,1)||catalog.count!=2||catalog.classes[1].parentIndex!=0)ExitProcess(101);
 lstrcpyA(node.className,"Derived");if(catalog.Resolve(node,&catalog)!=WorldModels::Ordinary)ExitProcess(102);
 lstrcpyA(node.className,"Base");if(catalog.Resolve(node,&catalog)!=WorldModels::WorldModel)ExitProcess(103);
 lstrcpyA(node.className,"base");if(catalog.Resolve(node,&catalog)!=WorldModels::Unknown)ExitProcess(104);
 if(catalog.CopyDefinitions(defs,2,2)||!catalog.error||catalog.count)ExitProcess(105);
 b.name="Base";if(catalog.CopyDefinitions(defs,2,1)||catalog.count)ExitProcess(106);b.name="Derived";
 if(catalog.CopyDefinitions(defs,4097,1)||catalog.count)ExitProcess(107);
 b.parent=&b;if(catalog.CopyDefinitions(defs,2,1)||catalog.count)ExitProcess(108);b.parent=&a;
 a.name="";if(catalog.CopyDefinitions(defs,2,1)||catalog.count)ExitProcess(109);a.name="Base";
 defs[1]=0;if(catalog.CopyDefinitions(defs,2,1)||catalog.count)ExitProcess(110);defs[1]=&b;
 GameClassMetadata::SDKClassDef missing={};b.parent=&missing;if(catalog.CopyDefinitions(defs,2,1)||catalog.count)ExitProcess(111);
 ExitProcess(0);
}
extern "C" void HarnessEntry(){
 SetErrorMode(3);char option[4];if(GetEnvironmentVariableA("FEAR_GAME_CLASSES_SELFTEST",option,4)&&option[0]=='1')PureChecks();
 char sourcePath[MAX_PATH],output[MAX_PATH];if(!Path("FEAR_READER_INPUT",sourcePath)||!Path("FEAR_GAME_CLASSES_OUTPUT",output))ExitProcess(1);
 GameClassMetadata::Catalog catalog;if(!catalog.LoadProject(sourcePath))Error(catalog.error,2);
 PrefabScene::Arena memory;void* stageStorage=memory.Allocate(sizeof(PrefabScene::Expansion));if(!stageStorage)ExitProcess(3);
 PrefabScene::Expansion* expansion=new(stageStorage) PrefabScene::Expansion(true);
 {PrefabScene::Source source;if(!source.Load(sourcePath,true)||!expansion->Build(sourcePath,source.scene))Error(source.scene.doc.error,4);}
 const PrefabScene::Model& model=expansion->Scene();WorldModels::Partition partition;
 if(!partition.Build(model,GameClassMetadata::Catalog::Resolve,&catalog)){
  for(const PrefabScene::Node* n=model.first;n;n=n->next)if(n->kind==2&&!catalog.Find(n->className))LogLine(n->className);
  Error(partition.error,5);
 }
 HANDLE file=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);
 unsigned int header[7]={0x314c4347,catalog.count,partition.groupCount,partition.assignmentCount,model.count,(unsigned int)lstrlenA(catalog.projectPath),(unsigned int)lstrlenA(catalog.modulePath)};
 if(file==INVALID_HANDLE_VALUE||!Write(file,header,sizeof(header))||!Write(file,catalog.projectPath,header[5])||!Write(file,catalog.modulePath,header[6]))ExitProcess(6);
 for(unsigned int i=0;i<catalog.count;i++){
  const GameClassMetadata::Class& c=catalog.classes[i];unsigned int record[5]={c.flags,c.parentIndex,(unsigned int)lstrlenA(c.name),(unsigned int)lstrlenA(c.parentName),(unsigned int)lstrlenA(c.description)};
  if(!Write(file,record,sizeof(record))||!Write(file,c.name,record[2])||!Write(file,c.parentName,record[3])||!Write(file,c.description,record[4]))ExitProcess(6);
 }
 for(unsigned int i=0;i<partition.groupCount;i++){
  const WorldModels::Group& g=partition.groups[i];unsigned int record[5]={g.ownerIndex,g.brushes,g.polygons,g.owner?(unsigned int)lstrlenA(g.owner->name):0,g.owner?(unsigned int)lstrlenA(g.owner->className):0};
  if(!Write(file,record,sizeof(record))||!Write(file,g.owner?g.owner->name:0,record[3])||!Write(file,g.owner?g.owner->className:0,record[4]))ExitProcess(6);
 }
 for(unsigned int i=0;i<partition.assignmentCount;i++){unsigned int record[2]={partition.assignments[i].brush->index,partition.assignments[i].group};if(!Write(file,record,sizeof(record)))ExitProcess(6);}
 CloseHandle(file);partition.Clear();expansion->~Expansion();ExitProcess(0);
}
