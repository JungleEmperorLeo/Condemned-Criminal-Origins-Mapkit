#define FEAR_PRIMARY_SURFACE_PREPARATION 1
#include "WorldPacker.cpp"
#include "SourceExpandedRenderContours.h"
#include "GameClassMetadata.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
extern "C" __declspec(dllimport) unsigned int WINAPI SetErrorMode(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
#pragma comment(linker,"/alternatename:__imp__SetErrorMode@4=__imp__SetErrorMode")
struct PartitionContext {GameClassMetadata::Catalog* catalog;SourceExpandedRenderContours::Consumed* consumed;};
static WorldModels::ClassKind Resolve(const PrefabScene::Node& node,void* context){return GameClassMetadata::Catalog::Resolve(node,((PartitionContext*)context)->catalog);}
static bool Ignore(const PrefabScene::Node& node,void* context){return ((PartitionContext*)context)->consumed->Ignore(node);}
static bool Write(HANDLE file,const void* bytes,unsigned int count){if(file==INVALID_HANDLE_VALUE)return true;DWORD done;return WriteFile(file,bytes,count,&done,0)&&done==count;}
static bool Text(HANDLE file,const char* text){return Write(file,text,lstrlenA(text));}
static bool Number(HANDLE file,unsigned int number){char text[16];*Decimal(text,number)=0;return Text(file,text);}
static bool String(HANDLE file,const char* text){
 if(!Text(file,"\""))return false;for(const unsigned char* p=(const unsigned char*)text;*p;p++){
  char one[2]={(char)*p,0};if(*p=='"'){if(!Text(file,"\\\""))return false;}else if(*p=='\\'){if(!Text(file,"\\\\"))return false;}
  else if(*p<32||*p>=127){const char hex[]="0123456789abcdef";char escaped[7]={'\\','u','0','0',hex[*p>>4],hex[*p&15],0};if(!Text(file,escaped))return false;}else if(!Text(file,one))return false;
 }return Text(file,"\"");
}
static bool Field(HANDLE file,const char* name,unsigned int value){return Text(file,",\"")&&Text(file,name)&&Text(file,"\":")&&Number(file,value);}
// Optional failure diagnostics are separate from successful render artifacts.
// The source builder still fails closed; no unknown brush is dropped or mapped.
static void Failure(const PrefabScene::Model& model,const WorldModels::Partition& partition,const SourceExpandedRenderContours::Consumed& consumed,const SourceExpandedRenderContours::Builder& out){
 char path[MAX_PATH];DWORD length=GetEnvironmentVariableA("FEAR_SOURCE_EXPANDED_ERROR_OUTPUT",path,MAX_PATH);if(!length||length>=MAX_PATH)return;
 HANDLE file=CreateFileA(path,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(file==INVALID_HANDLE_VALUE)return;
 bool ok=Text(file,"{\"error\":")&&String(file,out.error?out.error:"Expanded contours failed")&&Field(file,"errorNode",out.errorNode)&&Field(file,"sourceNodes",model.count)&&Text(file,",\"unknownBrushes\":[");
 unsigned int seen=0;for(const PrefabScene::Node* n=model.first;n&&ok;n=n->next)if(n->kind==1&&!n->knownType){
  unsigned int group=0xffffffffu;for(unsigned int i=0;i<partition.assignmentCount;i++)if(partition.assignments[i].brush==n){group=partition.assignments[i].group;break;}
  ok=(!seen++||Text(file,","))&&Text(file,"{\"node\":")&&Number(file,n->index)&&Field(file,"sourceNodeId",n->sourceId)&&Field(file,"parent",n->parent)&&Field(file,"group",group)&&Field(file,"surfaceFlags",n->surfaceFlags)&&Field(file,"points",n->geometry.points)&&Field(file,"polygons",n->geometry.polygons)&&Text(file,",\"consumed\":")&&Text(file,consumed.Ignore(*n)?"true":"false")&&Text(file,",\"name\":")&&String(file,n->name)&&Text(file,",\"type\":")&&String(file,n->type)&&Text(file,",\"source\":")&&String(file,n->source)&&Text(file,"}");
 }
 if(ok)Text(file,"]}\r\n");CloseHandle(file);
}
static bool Stats(HANDLE file,const SourceExpandedRenderContours::Builder& out,const GameClassMetadata::Catalog& catalog){
 if(file==INVALID_HANDLE_VALUE)return true;
 if(!Text(file,"{\"format\":\"expanded-main-world-contours-v1\",\"mainWorldOnly\":true,\"fullRender\":false")||!Field(file,"classCount",catalog.count)||!Text(file,",\"classModule\":")||!String(file,catalog.modulePath))return false;
 if(!Field(file,"sourceNodes",out.sourceNodes)||!Field(file,"sourceBrushes",out.sourceBrushes)||!Field(file,"sourcePolygons",out.sourcePolygons)||!Field(file,"mainBrushes",out.mainBrushes)||!Field(file,"mainPolygons",out.mainPolygons)||!Field(file,"candidateBrushes",out.candidateBrushes)||!Field(file,"candidatePolygons",out.candidatePolygons)||!Field(file,"visibleBrushes",out.visibleBrushes)||!Field(file,"tagReferences",out.tagReferences)||!Field(file,"initialPoints",out.initialPoints)||!Field(file,"remappedPoints",out.remappedPoints)||!Field(file,"sourceRecords",out.recordCount)||!Field(file,"contours",out.contourCount)||!Field(file,"outputVertices",out.outputVertices)||!Field(file,"assignedContours",out.assignedContours)||!Field(file,"unassignedContours",out.unassignedContours)||!Field(file,"smallFragments",out.smallFragments)||!Field(file,"consumedBrushes",out.consumedBrushes)||!Field(file,"streamingBrushes",out.streamingBrushes)||!Field(file,"navigationBrushes",out.navigationBrushes)||!Field(file,"deferredOwners",out.deferredOwners)||!Field(file,"deferredBrushes",out.deferredBrushes)||!Field(file,"deferredPolygons",out.deferredPolygons))return false;
 if(!Text(file,",\"offsetBits\":["))return false;for(unsigned int a=0;a<3;a++){union{float f;unsigned int u;}bits;bits.f=(&out.offset.x)[a];if((a&&!Text(file,","))||!Number(file,bits.u))return false;}
 if(!Text(file,"],\"pendingOwners\":["))return false;
 for(unsigned int i=0;i<out.deferredOwners;i++){const SourceExpandedRenderContours::DeferredOwner& owner=out.deferred[i];if((i&&!Text(file,","))||!Text(file,"{\"name\":")||!String(file,owner.name)||!Text(file,",\"class\":")||!String(file,owner.className)||!Field(file,"group",owner.group)||!Field(file,"sourceNode",owner.sourceNode)||!Field(file,"brushes",owner.brushes)||!Field(file,"polygons",owner.polygons)||!Text(file,"}"))return false;}
 return Text(file,"]}\r\n");
}
extern "C" void HarnessEntry(){
 SetErrorMode(3);char input[MAX_PATH];DWORD inputSize=GetEnvironmentVariableA("FEAR_READER_INPUT",input,MAX_PATH);if(!inputSize||inputSize>=MAX_PATH)ExitProcess(1);
 char mode[4]={};if(GetEnvironmentVariableA("FEAR_SOURCE_NORMAL_MODE",mode,4)&&mode[0]!='8')ExitProcess(8);if(GetEnvironmentVariableA("FEAR_SOURCE_RECOMPUTE_NORMALS",mode,4)&&mode[0]!='3')ExitProcess(8);
 bool smooth=true;if(GetEnvironmentVariableA("FEAR_SOURCE_SMOOTH_NORMALS",mode,4))smooth=mode[0]!='0';
 PrefabScene::Source source;if(!source.Load(input,true)){LogLine(source.scene.doc.error?source.scene.doc.error:"Expanded source load failed");ExitProcess(2);}
 PrefabScene::Arena memory;void* stageStorage=memory.Allocate(sizeof(PrefabScene::Expansion));if(!stageStorage)ExitProcess(3);PrefabScene::Expansion* expanded=new(stageStorage)PrefabScene::Expansion(true);if(!expanded->Build(input,source.scene)){LogLine("Expanded source hierarchy failed");ExitProcess(3);}
 const PrefabScene::Model& model=expanded->Scene();WorldText::Vec3 offset;if(!WorldCenter::Compute(model,offset))ExitProcess(3);
 GameClassMetadata::Catalog catalog;if(!catalog.LoadProject(input)){LogLine(catalog.error?catalog.error:"Game class metadata load failed");ExitProcess(4);}
 SourceExpandedRenderContours::Consumed consumed;if(!consumed.Build(model)){LogLine(consumed.error);ExitProcess(5);}
 WorldModels::Partition partition;PartitionContext context={&catalog,&consumed};if(!partition.Build(model,Resolve,&context,Ignore)){LogLine(partition.error);ExitProcess(5);}
 stageStorage=memory.Allocate(sizeof(VisibilityCompiler::Builder));if(!stageStorage)ExitProcess(6);VisibilityCompiler::Builder* visibility=new(stageStorage)VisibilityCompiler::Builder;if(!visibility->Build(model,&offset))ExitProcess(6);
 stageStorage=memory.Allocate(sizeof(SourceExpandedRenderContours::Builder));if(!stageStorage)ExitProcess(7);SourceExpandedRenderContours::Builder* out=new(stageStorage)SourceExpandedRenderContours::Builder;if(!out->Build(model,partition,*visibility,smooth)){LogLine(out->error?out->error:"Expanded contours failed");Failure(model,partition,consumed,*out);ExitProcess(7);}
 // Validate every output path only after complete source preparation succeeds.
 const char* keys[]={"FEAR_SOURCE_CLIP_OUTPUT","FEAR_SOURCE_CLIP_METADATA","FEAR_SOURCE_CLIP_BASES","FEAR_SOURCE_FRAME_SURFACES","FEAR_SOURCE_SECTION_SURFACES","FEAR_SOURCE_EXPANDED_STATS","FEAR_SOURCE_EXPANDED_PROVENANCE","FEAR_SOURCE_OFFSET_OUTPUT"};
 char (*paths)[MAX_PATH]=(char(*)[MAX_PATH])memory.Allocate(sizeof(char)*8*MAX_PATH);HANDLE files[8];if(!paths)ExitProcess(9);
 for(unsigned int i=0;i<8;i++){files[i]=INVALID_HANDLE_VALUE;DWORD length=GetEnvironmentVariableA(keys[i],paths[i],MAX_PATH);if(length>=MAX_PATH||(i<2&&!length))ExitProcess(9);if(!length)paths[i][0]=0;}
 for(unsigned int i=0;i<8;i++)if(paths[i][0]){files[i]=CreateFileA(paths[i],0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(files[i]==INVALID_HANDLE_VALUE)ExitProcess(9);}
 bool ok=true;for(unsigned int i=0;i<out->recordCount&&ok;i++){
  const SourceExpandedRenderContours::Record& record=out->records[i];unsigned int header[2]={i,record.contourCount},provenance[4]={i,record.sourceNode,record.sourcePolygon,record.fragment};ok=Write(files[1],header,8)&&Write(files[6],provenance,16);
  for(unsigned int c=0;c<record.contourCount&&ok;c++){
   const SourceExpandedRenderContours::Contour& contour=out->contours[record.firstContour+c];unsigned int pair[2]={contour.count,contour.owner},frame[2]={contour.brushGroup,contour.tangentMethod};
   ok=Write(files[1],pair,8)&&Write(files[0],contour.vertices,contour.count*44)&&Write(files[2],contour.bases,sizeof(contour.bases))&&Write(files[3],&contour.plane,12)&&Write(files[3],frame,8)&&Write(files[4],&contour.surfaceFlags,4)&&Write(files[4],contour.material,260);
  }
 }
 if(ok)ok=Stats(files[5],*out,catalog)&&Write(files[7],&out->offset,12);for(unsigned int i=0;i<8;i++)if(files[i]!=INVALID_HANDLE_VALUE)CloseHandle(files[i]);ExitProcess(ok?0:10);
}
