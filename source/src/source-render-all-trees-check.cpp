#define FEAR_PRIMARY_SURFACE_PREPARATION 1
#include "WorldPacker.cpp"
#include "SourceRenderAllTrees.h"
#include "GameClassMetadata.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
extern "C" __declspec(dllimport) unsigned int WINAPI SetErrorMode(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
#pragma comment(linker,"/alternatename:__imp__SetErrorMode@4=__imp__SetErrorMode")
struct Context {GameClassMetadata::Catalog* catalog;SourceExpandedRenderContours::Consumed* consumed;};
static WorldModels::ClassKind Resolve(const PrefabScene::Node& node,void* context){return GameClassMetadata::Catalog::Resolve(node,((Context*)context)->catalog);}
static bool Ignore(const PrefabScene::Node& node,void* context){return ((Context*)context)->consumed->Ignore(node);}
static bool Put(HANDLE file,const void* input,unsigned int size){if(file==INVALID_HANDLE_VALUE)return true;DWORD done;return WriteFile(file,input,size,&done,0)&&done==size;}
static bool Text(HANDLE file,const char* text){return Put(file,text,lstrlenA(text));}
static bool Number(HANDLE file,unsigned int value){char text[16];*Decimal(text,value)=0;return Text(file,text);}
static bool Field(HANDLE file,const char* name,unsigned int value){return Text(file,",\"")&&Text(file,name)&&Text(file,"\":")&&Number(file,value);}
static bool String(HANDLE file,const char* text){if(!Text(file,"\""))return false;for(const unsigned char* p=(const unsigned char*)text;*p;p++){char one[2]={(char)*p,0};if(*p=='"'){if(!Text(file,"\\\""))return false;}else if(*p=='\\'){if(!Text(file,"\\\\"))return false;}else if(*p<32||*p>=127){const char hex[]="0123456789abcdef";char escaped[7]={'\\','u','0','0',hex[*p>>4],hex[*p&15],0};if(!Text(file,escaped))return false;}else if(!Text(file,one))return false;}return Text(file,"\"");}
static void FailureDiagnostic(const char* message,const char* stage=0,unsigned int code=0,const char* owner=0,const PrefabScene::Model* model=0,const VisibilityCompiler::Builder* visibility=0){
 char path[MAX_PATH];DWORD length=GetEnvironmentVariableA("FEAR_ALL_TREES_ERROR_OUTPUT",path,MAX_PATH);if(!length||length>=MAX_PATH)return;HANDLE file=CreateFileA(path,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(file==INVALID_HANDLE_VALUE)return;
 // Preserve the established plain error protocol for all-tree builder guards.
 if(!stage){Text(file,message?message:"No error message");Text(file,"\r\n");CloseHandle(file);return;}
 Text(file,"{\"stage\":");String(file,stage);Text(file,",\"message\":");String(file,message?message:"No error message");Field(file,"code",code);Text(file,",\"sourceOwner\":");String(file,owner?owner:"");
 if(model){unsigned int nodes=0,sectors=0,sectorPolygons=0,portals=0,portalPolygons=0,extrusions=0;for(const PrefabScene::Node* n=model->first;n;n=n->next){nodes++;if(n->kind!=1)continue;if(FearBrushFlags::Equal(n->type,"Sector")){sectors++;sectorPolygons+=n->geometry.polygons;}if(FearBrushFlags::Equal(n->type,"SkyPortal")){portals++;portalPolygons+=n->geometry.polygons;}if(FearBrushFlags::Equal(n->type,"ExtrudeSector")||FearBrushFlags::Equal(n->type,"SemiExtrudeSector"))extrusions++;}
  Field(file,"expandedNodes",nodes);Field(file,"sourceSectors",sectors);Field(file,"sectorPolygons",sectorPolygons);Field(file,"sourceSkyPortals",portals);Field(file,"skyPortalPolygons",portalPolygons);Field(file,"sourceExtrusions",extrusions);
  Text(file,",\"extrusionSources\":[");unsigned int emitted=0;for(const PrefabScene::Node* n=model->first;n;n=n->next)if(n->kind==1&&(FearBrushFlags::Equal(n->type,"ExtrudeSector")||FearBrushFlags::Equal(n->type,"SemiExtrudeSector"))){if(emitted++)Text(file,",");Text(file,"{\"name\":");String(file,n->name);Text(file,",\"type\":");String(file,n->type);Field(file,"sourceIndex",n->index);Field(file,"polygons",n->geometry.polygons);Text(file,"}");}Text(file,"]");
 }
 if(visibility){Field(file,"visibilityErrorCode",visibility->errorCode);Field(file,"preparedSectors",visibility->sectorCount);Field(file,"acceptedExtrusions",visibility->acceptedExtrusions);Field(file,"skippedExtrusions",visibility->skippedExtrusions);Field(file,"visibilityBytes",visibility->size);}
 Text(file,"}\r\n");CloseHandle(file);
}
static bool Empty(const SourceRenderAllTrees::Builder& builder){return !builder.sourceModel&&!builder.sourceVisibility&&!builder.layout.trees&&!builder.layout.count&&!builder.mainNodeCount&&!builder.totalNodeCount&&!builder.groupCount&&!builder.aliasCount&&!builder.groupTree&&!builder.groupNode&&!builder.groupRepresentative&&builder.error;}
static bool Guards(const PrefabScene::Model& model,WorldModels::Partition& partition,const VisibilityCompiler::Builder& visibility,const SourceSkyPortals::Builder& sky,unsigned int& passed,bool share){
 SourceRenderAllTrees::Builder guard;passed=0;unsigned int saved=partition.groups[0].ownerIndex;partition.groups[0].ownerIndex=0;bool ok=!guard.Build(model,partition,visibility,&sky,share)&&Empty(guard);partition.groups[0].ownerIndex=saved;if(!ok)return false;passed++;
 if(partition.assignmentCount){saved=partition.assignments[0].group;partition.assignments[0].group=partition.groupCount;ok=!guard.Build(model,partition,visibility,&sky,share)&&Empty(guard);partition.assignments[0].group=saved;if(!ok)return false;passed++;}
 for(unsigned int a=0;a<partition.assignmentCount;a++)if(partition.assignments[a].group){PrefabScene::Node* brush=(PrefabScene::Node*)partition.assignments[a].brush;char type[64];lstrcpyA(type,brush->type);lstrcpyA(brush->type,"Sector");ok=!guard.Build(model,partition,visibility,&sky,share)&&Empty(guard);lstrcpyA(brush->type,type);if(!ok)return false;passed++;lstrcpyA(brush->type,"SkyPortal");ok=!guard.Build(model,partition,visibility,&sky,share)&&Empty(guard);lstrcpyA(brush->type,type);if(!ok)return false;passed++;break;}
 if(share)for(unsigned int g=1;g<partition.groupCount;g++)if(partition.groups[g].brushes&&guard.Build(model,partition,visibility,&sky,share)&&guard.groupRepresentative[g]!=g){unsigned int original=partition.groups[g].polygons;partition.groups[g].polygons++;ok=!guard.Build(model,partition,visibility,&sky,share)&&Empty(guard);partition.groups[g].polygons=original;if(!ok)return false;passed++;break;}
 return guard.Build(model,partition,visibility,&sky,share);
}
static bool Stats(HANDLE file,const SourceRenderAllTrees::Builder& builder,const WorldModels::Partition& partition,unsigned int guards){
 if(file==INVALID_HANDLE_VALUE)return true;
 if(!Text(file,"{\"format\":\"source-all-tree-scaffold-v1\",\"sourceOwnedOnly\":true,\"originalDLLLoaded\":false,\"fullRenderGeometry\":false")||!Field(file,"trees",builder.layout.count)||!Field(file,"mainTree",builder.mainTree)||!Field(file,"mainNodes",builder.mainNodeCount)||!Field(file,"totalNodes",builder.totalNodeCount)||!Field(file,"groups",builder.groupCount)||!Field(file,"aliases",builder.aliasCount)||!Field(file,"inputGuardChecks",guards)||!Text(file,",\"groupMaps\":["))return false;
 for(unsigned int g=0;g<builder.groupCount;g++){const WorldModels::Group& group=partition.groups[g];if((g&&!Text(file,","))||!Text(file,"{\"group\":")||!Number(file,g)||!Field(file,"sourceOwner",group.ownerIndex)||!Field(file,"brushes",group.brushes)||!Field(file,"polygons",group.polygons)||!Field(file,"representative",builder.groupRepresentative[g])||!Field(file,"tree",builder.groupTree[g])||!Field(file,"flattenedNode",builder.groupNode[g])||!Text(file,",\"name\":")||!String(file,group.owner?group.owner->name:"MainWorld")||!Text(file,"}"))return false;}
 return Text(file,"]}\r\n");
}
static bool Scaffold(HANDLE file,const SourceRenderAllTrees::Builder& builder,bool mainOnly){
 if(file==INVALID_HANDLE_VALUE)return true;unsigned int count=mainOnly?1:builder.layout.count;unsigned int header[2]={0x31544653,count};if(!Put(file,header,8))return false;
 for(unsigned int t=0;t<count;t++){const RenderTrees::Tree& tree=builder.layout.trees[t];if(!Put(file,&tree.count,4))return false;for(unsigned int n=0;n<tree.count;n++){const RenderTrees::Node& node=tree.nodes[n];if(node.count||node.branches||!Put(file,&node.skyPortalCount,4))return false;for(unsigned int p=0;p<node.skyPortalCount;p++){const RenderTrees::SkyPortal& portal=node.skyPortals[p];if(!Put(file,&portal.count,4)||!Put(file,portal.vertices,portal.count*12))return false;}}}return true;
}
extern "C" void HarnessEntry(){
 SetErrorMode(3);char input[MAX_PATH];DWORD length=GetEnvironmentVariableA("FEAR_READER_INPUT",input,MAX_PATH);if(!length||length>=MAX_PATH)ExitProcess(1);
 PrefabScene::Arena memory;PrefabScene::Source source;if(!source.Load(input,true))ExitProcess(2);
 void* stageStorage=memory.Allocate(sizeof(PrefabScene::Expansion));if(!stageStorage)ExitProcess(3);PrefabScene::Expansion* expansion=new(stageStorage)PrefabScene::Expansion(true);if(!expansion->Build(input,source.scene))ExitProcess(3);
 const PrefabScene::Model& model=expansion->Scene();GameClassMetadata::Catalog catalog;if(!catalog.LoadProject(input)){LogLine(catalog.error);ExitProcess(4);}
 SourceExpandedRenderContours::Consumed consumed;if(!consumed.Build(model)){LogLine(consumed.error);ExitProcess(5);}Context context={&catalog,&consumed};WorldModels::Partition partition;if(!partition.Build(model,Resolve,&context,Ignore)){LogLine(partition.error);ExitProcess(5);}
 stageStorage=memory.Allocate(sizeof(VisibilityCompiler::Builder));if(!stageStorage){FailureDiagnostic("Visibility builder allocation failed","VisibilityAllocation",6,0,&model);ExitProcess(6);}VisibilityCompiler::Builder* visibility=new(stageStorage)VisibilityCompiler::Builder;WorldText::Vec3 offset;if(!WorldCenter::Compute(model,offset)){FailureDiagnostic("World centering failed","WorldCenter",0,0,&model);ExitProcess(6);}if(!visibility->Build(model,&offset)){FailureDiagnostic("Visibility compiler failed","VisibilityCompiler",visibility->errorCode,visibility->errorSector,&model);ExitProcess(6);}
 SourceSkyPortals::Builder sky;if(!sky.Build(model,visibility)){LogLine(sky.error);FailureDiagnostic(sky.error,"SourceSkyPortals",0,0,&model,visibility);ExitProcess(6);}
 char shared[8];bool share=false;length=GetEnvironmentVariableA("FEAR_ALL_TREES_SHARED",shared,8);if(length>=8)ExitProcess(1);if(length){if(FearBrushFlags::Equal(shared,"TRUE")||FearBrushFlags::Equal(shared,"T"))share=true;else if(!FearBrushFlags::Equal(shared,"FALSE")&&!FearBrushFlags::Equal(shared,"F"))ExitProcess(1);}
 bool mainOnly=false;length=GetEnvironmentVariableA("FEAR_ALL_TREES_MAIN_ONLY",shared,8);if(length>=8)ExitProcess(1);if(length){if(FearBrushFlags::Equal(shared,"TRUE")||FearBrushFlags::Equal(shared,"T"))mainOnly=true;else if(!FearBrushFlags::Equal(shared,"FALSE")&&!FearBrushFlags::Equal(shared,"F"))ExitProcess(1);}
 SourceRenderAllTrees::Builder builder;if(!builder.Build(model,partition,*visibility,&sky,share)){LogLine(builder.error);FailureDiagnostic(builder.error);ExitProcess(7);}unsigned int checks=0;char guard[4];if(GetEnvironmentVariableA("FEAR_ALL_TREES_GUARDS",guard,4)&&guard[0]=='1'&&!Guards(model,partition,*visibility,sky,checks,share))ExitProcess(8);
 RenderTrees::Block block;if(!block.Build(builder.layout,0)){LogLine(block.error);ExitProcess(7);}
 const char* keys[3]={"FEAR_ALL_TREES_OUTPUT","FEAR_ALL_TREES_SCAFFOLD_OUTPUT","FEAR_ALL_TREES_STATS"};char paths[3][MAX_PATH];HANDLE files[3];bool any=false;
 for(unsigned int f=0;f<3;f++){files[f]=INVALID_HANDLE_VALUE;length=GetEnvironmentVariableA(keys[f],paths[f],MAX_PATH);if(length>=MAX_PATH)ExitProcess(9);if(!length)paths[f][0]=0;else any=true;}if(!any)ExitProcess(9);
 for(unsigned int f=0;f<3;f++)if(paths[f][0]){files[f]=CreateFileA(paths[f],0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(files[f]==INVALID_HANDLE_VALUE)ExitProcess(9);}
 unsigned int header[6]={0x31544153,builder.layout.count,builder.mainNodeCount,builder.totalNodeCount,builder.groupCount,block.size};bool ok=Put(files[0],header,24)&&Put(files[0],block.data,block.size)&&Scaffold(files[1],builder,mainOnly)&&Stats(files[2],builder,partition,checks);for(unsigned int f=0;f<3;f++)if(files[f]!=INVALID_HANDLE_VALUE)CloseHandle(files[f]);ExitProcess(ok?0:10);
}
