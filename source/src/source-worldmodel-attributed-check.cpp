#define FEAR_PRIMARY_SURFACE_PREPARATION 1
#include "WorldPacker.cpp"
#include "SourceWorldModelRenderContours.h"
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
static bool Number(HANDLE file,unsigned int value){char text[16];*Decimal(text,value)=0;return Text(file,text);}
static bool String(HANDLE file,const char* text){
 if(!Text(file,"\""))return false;for(const unsigned char* p=(const unsigned char*)text;*p;p++){
  char one[2]={(char)*p,0};if(*p=='"'){if(!Text(file,"\\\""))return false;}else if(*p=='\\'){if(!Text(file,"\\\\"))return false;}
  else if(*p<32||*p>=127){const char hex[]="0123456789abcdef";char escaped[7]={'\\','u','0','0',hex[*p>>4],hex[*p&15],0};if(!Text(file,escaped))return false;}else if(!Text(file,one))return false;
 }return Text(file,"\"");
}
static bool Field(HANDLE file,const char* name,unsigned int value){return Text(file,",\"")&&Text(file,name)&&Text(file,"\":")&&Number(file,value);}
static unsigned int Hash(unsigned int hash,const void* input,unsigned int bytes){const unsigned char* data=(const unsigned char*)input;for(unsigned int i=0;i<bytes;i++)hash=(hash^data[i])*16777619u;return hash;}
static unsigned int SourceFingerprint(const PrefabScene::Model& model){
 unsigned int hash=2166136261u;for(const PrefabScene::Node* n=model.first;n;n=n->next){hash=Hash(hash,n,sizeof(PrefabScene::Node));if(n->kind!=1)continue;
  const WorldText::Geometry& g=n->geometry;hash=Hash(hash,g.vertices,g.points*12);hash=Hash(hash,g.faces,g.polygons*sizeof(WorldText::Polygon));if(g.colors)hash=Hash(hash,g.colors,g.points*sizeof(WorldText::PointColor));
  for(unsigned int p=0;p<g.polygons;p++){hash=Hash(hash,g.faces[p].indices,g.faces[p].count*4);hash=Hash(hash,g.faces[p].mappings,g.faces[p].mappingCount*sizeof(WorldText::Mapping));}
 }return hash;
}
static bool Stats(HANDLE file,const SourceWorldModelRenderContours::Builder& out,const GameClassMetadata::Catalog& catalog){
 if(file==INVALID_HANDLE_VALUE)return true;
 if(!Text(file,"{\"format\":\"source-worldmodel-contours-v1\",\"ownerModelsOnly\":true,\"mainWorldOnly\":false,\"fullRender\":false,\"ownerContoursComplete\":true,\"ownerFrameProfile\":\"centered-world-owner-inverse\",\"sourceModelUnmodified\":true,\"treeSharingApplied\":false,\"sharedMainOwnerPlanePool\":true,\"ownerBasisAlreadyTransformed\":true,\"ownerNormalsPreparedInCenteredWorld\":true,\"allSourceGroupsPreparedBeforeSharing\":true,\"localNode\":0")||!Text(file,out.sharedPrototypePlaneNormals?",\"sharedPrototypePlaneNormals\":true":",\"sharedPrototypePlaneNormals\":false")||!Field(file,"classCount",catalog.count)||!Text(file,",\"classModule\":")||!String(file,catalog.modulePath))return false;
 if(!Field(file,"sourceNodes",out.sourceNodes)||!Field(file,"sourceBrushes",out.sourceBrushes)||!Field(file,"sourcePolygons",out.sourcePolygons)||!Field(file,"mainBrushes",out.mainBrushes)||!Field(file,"mainPolygons",out.mainPolygons)||!Field(file,"ownerCount",out.ownerCount)||!Field(file,"nonemptyOwners",out.nonemptyOwners)||!Field(file,"ownerBrushes",out.ownerBrushes)||!Field(file,"ownerPolygons",out.ownerPolygons)||!Field(file,"candidateBrushes",out.candidateBrushes)||!Field(file,"candidatePolygons",out.candidatePolygons)||!Field(file,"visibleBrushes",out.visibleBrushes)||!Field(file,"tagReferences",out.tagReferences)||!Field(file,"initialPoints",out.initialPoints)||!Field(file,"remappedPoints",out.remappedPoints)||!Field(file,"sourceRecords",out.recordCount)||!Field(file,"contours",out.contourCount)||!Field(file,"outputVertices",out.outputVertices)||!Field(file,"consumedBrushes",out.consumedBrushes)||!Field(file,"streamingBrushes",out.streamingBrushes)||!Field(file,"navigationBrushes",out.navigationBrushes))return false;
 if(!Text(file,",\"offsetBits\":["))return false;for(unsigned int a=0;a<3;a++){union{float f;unsigned int u;}bits;bits.f=(&out.offset.x)[a];if((a&&!Text(file,","))||!Number(file,bits.u))return false;}
 if(!Text(file,"],\"owners\":["))return false;for(unsigned int i=0;i<out.ownerCount;i++){
  const SourceWorldModelRenderContours::Owner& o=out.owners[i];if((i&&!Text(file,","))||!Text(file,"{\"name\":")||!String(file,o.name)||!Text(file,",\"class\":")||!String(file,o.className)||!Field(file,"group",o.group)||!Field(file,"sourceNode",o.sourceNode)||!Field(file,"brushes",o.brushes)||!Field(file,"polygons",o.polygons)||!Field(file,"candidateBrushes",o.candidateBrushes)||!Field(file,"candidatePolygons",o.candidatePolygons)||!Field(file,"visibleBrushes",o.visibleBrushes)||!Field(file,"tagReferences",o.tagReferences)||!Field(file,"initialPoints",o.initialPoints)||!Field(file,"remappedPoints",o.remappedPoints)||!Field(file,"firstRecord",o.firstRecord)||!Field(file,"recordCount",o.recordCount)||!Field(file,"firstContour",o.firstContour)||!Field(file,"contourCount",o.contourCount)||!Field(file,"outputVertices",o.outputVertices)||!Text(file,"}"))return false;
 }return Text(file,"]}\r\n");
}
extern "C" void HarnessEntry(){
 SetErrorMode(3);char input[MAX_PATH];DWORD size=GetEnvironmentVariableA("FEAR_READER_INPUT",input,MAX_PATH);if(!size||size>=MAX_PATH)ExitProcess(1);
 char shared[8];bool shareNormals=false;size=GetEnvironmentVariableA("FEAR_SOURCE_OWNER_SHARED",shared,8);if(size>=8)ExitProcess(1);if(size){if(FearBrushFlags::Equal(shared,"TRUE")||FearBrushFlags::Equal(shared,"T"))shareNormals=true;else if(!FearBrushFlags::Equal(shared,"FALSE")&&!FearBrushFlags::Equal(shared,"F"))ExitProcess(1);}
 char mode[4]={};if(GetEnvironmentVariableA("FEAR_SOURCE_NORMAL_MODE",mode,4)&&mode[0]!='8')ExitProcess(8);if(GetEnvironmentVariableA("FEAR_SOURCE_RECOMPUTE_NORMALS",mode,4)&&mode[0]!='3')ExitProcess(8);bool smooth=true;if(GetEnvironmentVariableA("FEAR_SOURCE_SMOOTH_NORMALS",mode,4))smooth=mode[0]!='0';
 PrefabScene::Source source;if(!source.Load(input,true)){LogLine(source.scene.doc.error?source.scene.doc.error:"WorldModel source load failed");ExitProcess(2);}
 PrefabScene::Arena memory;void* stageStorage=memory.Allocate(sizeof(PrefabScene::Expansion));if(!stageStorage)ExitProcess(3);PrefabScene::Expansion* expanded=new(stageStorage)PrefabScene::Expansion(true);if(!expanded->Build(input,source.scene)){LogLine("WorldModel expanded hierarchy failed");ExitProcess(3);}
 const PrefabScene::Model& model=expanded->Scene();GameClassMetadata::Catalog catalog;if(!catalog.LoadProject(input)){LogLine(catalog.error?catalog.error:"WorldModel class metadata load failed");ExitProcess(4);}
 SourceExpandedRenderContours::Consumed consumed;if(!consumed.Build(model)){LogLine(consumed.error);ExitProcess(5);}
 WorldModels::Partition partition;PartitionContext context={&catalog,&consumed};if(!partition.Build(model,Resolve,&context,Ignore)){LogLine(partition.error);ExitProcess(5);}
 unsigned int fingerprint=SourceFingerprint(model);stageStorage=memory.Allocate(sizeof(SourceWorldModelRenderContours::Builder));if(!stageStorage)ExitProcess(7);SourceWorldModelRenderContours::Builder* out=new(stageStorage)SourceWorldModelRenderContours::Builder;if(!out->Build(model,partition,smooth,RenderExceptionalUV::Enabled(),shareNormals)){LogLine(out->error?out->error:"WorldModel source contours failed");ExitProcess(7);}
 if(SourceFingerprint(model)!=fingerprint){LogLine("WorldModel contour builder modified source geometry");ExitProcess(7);}
 const char* keys[]={"FEAR_SOURCE_CLIP_OUTPUT","FEAR_SOURCE_CLIP_METADATA","FEAR_SOURCE_CLIP_BASES","FEAR_SOURCE_FRAME_SURFACES","FEAR_SOURCE_SECTION_SURFACES","FEAR_SOURCE_WM_STATS","FEAR_SOURCE_WM_PROVENANCE","FEAR_SOURCE_OFFSET_OUTPUT"};
 char (*paths)[MAX_PATH]=(char(*)[MAX_PATH])memory.Allocate(8*MAX_PATH);HANDLE files[8];if(!paths)ExitProcess(9);
 for(unsigned int i=0;i<8;i++){files[i]=INVALID_HANDLE_VALUE;DWORD length=GetEnvironmentVariableA(keys[i],paths[i],MAX_PATH);if(length>=MAX_PATH||(i<2&&!length))ExitProcess(9);if(!length)paths[i][0]=0;}
 // All geometry, properties, classes and source-preservation guards precede
 // opening any output. Geometry is local; offset.bin is full-scene diagnostics.
 for(unsigned int i=0;i<8;i++)if(paths[i][0]){files[i]=CreateFileA(paths[i],0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(files[i]==INVALID_HANDLE_VALUE)ExitProcess(9);}
 bool ok=true;for(unsigned int i=0;i<out->recordCount&&ok;i++){
  const SourceWorldModelRenderContours::Record& r=out->records[i];unsigned int header[2]={i,r.contourCount},provenance[5]={i,r.sourceNode,r.sourcePolygon,r.fragment,r.group};ok=Write(files[1],header,8)&&Write(files[6],provenance,20);
  for(unsigned int c=0;c<r.contourCount&&ok;c++){
   const SourceWorldModelRenderContours::Contour& contour=out->contours[r.firstContour+c];unsigned int pair[2]={contour.count,contour.localNode},frame[2]={contour.brushGroup,contour.tangentMethod};
   ok=Write(files[1],pair,8)&&Write(files[0],contour.vertices,contour.count*44)&&Write(files[2],contour.bases,sizeof(contour.bases))&&Write(files[3],&contour.plane,12)&&Write(files[3],frame,8)&&Write(files[4],&contour.surfaceFlags,4)&&Write(files[4],contour.material,260);
  }
 }
 if(ok)ok=Stats(files[5],*out,catalog)&&Write(files[7],&out->offset,12);for(unsigned int i=0;i<8;i++)if(files[i]!=INVALID_HANDLE_VALUE)CloseHandle(files[i]);ExitProcess(ok?0:10);
}
