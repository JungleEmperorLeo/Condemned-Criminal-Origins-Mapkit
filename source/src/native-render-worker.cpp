#define FEAR_PRIMARY_SURFACE_PREPARATION 1
#define FEAR_NATIVE_NV_RUNTIME 1
#define FEAR_RENDER_INDEX_REORDER 1
#include "WorldPacker.cpp"
extern "C" {
__declspec(dllimport) void WINAPI ExitProcess(unsigned int);
__declspec(dllimport) unsigned int WINAPI SetErrorMode(unsigned int);
__declspec(dllimport) char* WINAPI GetCommandLineA();
__declspec(dllimport) HANDLE WINAPI GetStdHandle(unsigned int);
__declspec(dllimport) unsigned int WINAPI GetCurrentProcessId();
__declspec(dllimport) int WINAPI MoveFileExA(const char*,const char*,unsigned int);
__declspec(dllimport) int WINAPI DeleteFileA(const char*);
__declspec(dllimport) int WINAPI MultiByteToWideChar(unsigned int,unsigned int,const char*,int,unsigned short*,int);
__declspec(dllimport) int WINAPI CompareStringOrdinal(const unsigned short*,int,const unsigned short*,int,BOOL);
}
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
#pragma comment(linker,"/alternatename:__imp__SetErrorMode@4=__imp__SetErrorMode")
#pragma comment(linker,"/alternatename:__imp__GetCommandLineA@0=__imp__GetCommandLineA")
#pragma comment(linker,"/alternatename:__imp__GetStdHandle@4=__imp__GetStdHandle")
#pragma comment(linker,"/alternatename:__imp__GetCurrentProcessId@0=__imp__GetCurrentProcessId")
#pragma comment(linker,"/alternatename:__imp__MoveFileExA@12=__imp__MoveFileExA")
#pragma comment(linker,"/alternatename:__imp__DeleteFileA@4=__imp__DeleteFileA")
#pragma comment(linker,"/alternatename:__imp__MultiByteToWideChar@24=__imp__MultiByteToWideChar")
#pragma comment(linker,"/alternatename:__imp__CompareStringOrdinal@20=__imp__CompareStringOrdinal")
#pragma comment(linker,"/alternatename:__imp__LoadLibraryExA@12=__imp__LoadLibraryExA")
#pragma comment(linker,"/alternatename:__imp__FreeLibrary@4=__imp__FreeLibrary")
#pragma comment(linker,"/alternatename:__imp__VirtualQuery@12=__imp__VirtualQuery")
#include "RenderNvRuntime.h"
#include "NativeRenderCompiler.h"
namespace NativeRenderWorker {
struct Arguments {char source[260],project[260],resources[260],archive[260],report[260],stem[260];bool split;Arguments():split(false){source[0]=project[0]=resources[0]=archive[0]=report[0]=stem[0]=0;}};
// Literal quotes in path names are not supported by this bounded ANSI adapter.
// The compiler library receives all settings explicitly, without FEAR_* input.
static bool Token(const char*& at,char out[260]){
 while(*at==' '||*at=='\t')++at;if(!*at)return false;bool quoted=false;unsigned int n=0;
 while(*at&&(quoted||(*at!=' '&&*at!='\t'))){if(*at=='"'){quoted=!quoted;++at;continue;}if(n==259)return false;out[n++]=*at++;}out[n]=0;return !quoted&&n!=0;
}
static bool Parse(Arguments& args){
 const char* command=GetCommandLineA();char key[260],value[260];if(!Token(command,key))return false;unsigned int seen=0;
 while(*command){while(*command==' '||*command=='\t')++command;if(!*command)break;if(!Token(command,key)||!Token(command,value))return false;char* output=0;unsigned int flag=0;
  if(!RenderSectionRules::TextCompare(key,"--source")){output=args.source;flag=1;}
  else if(!RenderSectionRules::TextCompare(key,"--project")){output=args.project;flag=2;}
  else if(!RenderSectionRules::TextCompare(key,"--resources")){output=args.resources;flag=4;}
  else if(!RenderSectionRules::TextCompare(key,"--archive")){output=args.archive;flag=8;}
  else if(!RenderSectionRules::TextCompare(key,"--report")){output=args.report;flag=16;}
  else if(!RenderSectionRules::TextCompare(key,"--world-stem")){output=args.stem;flag=32;}
  else if(!RenderSectionRules::TextCompare(key,"--split-meshes")){flag=64;if(!RenderSectionRules::TextCompare(value,"true")||!RenderSectionRules::TextCompare(value,"1"))args.split=true;else if(RenderSectionRules::TextCompare(value,"false")&&RenderSectionRules::TextCompare(value,"0"))return false;}
  else return false;
  if(seen&flag)return false;seen|=flag;if(output)lstrcpyA(output,value);
 }if((seen&63)!=63)return false;
 char path[260];char* paths[6]={args.source,args.project,args.resources,args.archive,args.report,args.stem};
 for(unsigned int i=0;i<6;i++){if(!PrefabScene::Full(paths[i],path))return false;lstrcpyA(paths[i],path);}return true;
}
// Filesystem aliases follow Windows Unicode case rules. Render/material text
// identity keeps its recovered ASCII policy and is not reused for file safety.
static bool WidePath(const char* path,unsigned short out[260],int& length){
 int count=path?MultiByteToWideChar(0,0,path,-1,out,260):0;length=count?count-1:0;return count>1;
}
__declspec(noinline) static bool SamePath(const char* a,const char* b){
 unsigned short left[260],right[260];int nl=0,nr=0;
 // A failed conversion rejects publication instead of permitting an alias.
 if(!WidePath(a,left,nl)||!WidePath(b,right,nr))return true;
 return CompareStringOrdinal(left,nl,right,nr,TRUE)==2;
}
__declspec(noinline) static bool Under(const char* path,const char* root){
 unsigned short value[260],folder[260];int n=0,r=0;
 if(!WidePath(path,value,n)||!WidePath(root,folder,r))return true;
 if(n<r||CompareStringOrdinal(value,r,folder,r,TRUE)!=2)return false;
 return n==r||folder[r-1]=='\\'||folder[r-1]=='/'||value[r]=='\\'||value[r]=='/';
}
static bool OutputPaths(const Arguments& args,const char* prefabRoot){
 if(SamePath(args.archive,args.report))return false;
 const char* outputs[2]={args.archive,args.report};char alternate[260];lstrcpyA(alternate,args.source);unsigned int length=lstrlenA(alternate);
 if(length>=9&&!RenderSectionRules::TextCompare(alternate+length-9,".World00a"))alternate[length-1]='c';else if(length>=9&&!RenderSectionRules::TextCompare(alternate+length-9,".World00c"))alternate[length-1]='a';
 for(unsigned int i=0;i<2;i++)if(SamePath(outputs[i],args.source)||SamePath(outputs[i],alternate)||SamePath(outputs[i],args.project)||Under(outputs[i],args.resources)||(prefabRoot&&prefabRoot[0]&&Under(outputs[i],prefabRoot)))return false;
 return true;
}
static bool Put(HANDLE file,const void* data,unsigned int size){DWORD done=0;return !size||(WriteFile(file,data,size,&done,0)&&done==size);}
static void Console(const char* text){Put(GetStdHandle((unsigned int)-11),text,lstrlenA(text));Put(GetStdHandle((unsigned int)-11),"\r\n",2);}
static void Failure(const char* stage,const char* reason){char message[768];char* at=message;Append(at,stage);Append(at,": ");Append(at,reason?reason:"failure");*at=0;Console(message);}
static bool Temporary(const char* path,char out[300]){unsigned int n=lstrlenA(path);if(!n||n>259)return false;lstrcpyA(out,path);char* at=out+n;Append(at,".tmp.");at=Decimal(at,GetCurrentProcessId());*at=0;return true;}
class Json {
 HANDLE file;bool good;
public:
 explicit Json(HANDLE target):file(target),good(true){}
 void Raw(const char* value){if(good)good=Put(file,value,lstrlenA(value));}
 void Number(unsigned int n){char buffer[16];*Decimal(buffer,n)=0;Raw(buffer);}
 void String(const char* value){unsigned short text[512];int length=MultiByteToWideChar(0,0,value,-1,text,512);if(!length){good=false;return;}Raw("\"");for(int i=0;good&&i<length-1;i++){unsigned int c=text[i];char buffer[8];unsigned int n=0;if(c=='"'||c=='\\'){buffer[n++]='\\';buffer[n++]=(char)c;}else if(c<32||c>=128){const char* hex="0123456789abcdef";buffer[n++]='\\';buffer[n++]='u';buffer[n++]=hex[(c>>12)&15];buffer[n++]=hex[(c>>8)&15];buffer[n++]=hex[(c>>4)&15];buffer[n++]=hex[c&15];}else buffer[n++]=(char)c;good=Put(file,buffer,n);}Raw("\"");}
 void Field(const char* key,unsigned int value,bool comma=true){String(key);Raw(":");Number(value);if(comma)Raw(",");}bool Good()const{return good;}
};
static bool Report(HANDLE file,const Arguments& args,const NativeRenderCompiler::Builder& compiler){
 const NativeRenderCompiler::Stats& s=compiler.Statistics();const RenderSourceGlobalMeshes::Block& result=compiler.Result();Json json(file);
 json.Raw("{\n\"format\":\"native-source-render-worker-v1\",\"sourceOnly\":true,\"originalDLLLoaded\":false,\"originalProcessUsed\":false,\"referenceRead\":false,\"fullWorld00pCompiler\":false,\"source\":");json.String(args.source);json.Raw(",\"project\":");json.String(args.project);json.Raw(",\"resources\":");json.String(args.resources);json.Raw(",\"splitMeshes\":");json.Raw(args.split?"true,":"false,");
 json.Field("trees",s.trees);json.Field("nodes",s.nodes);json.Field("surfaces",s.surfaces);json.Field("mainFanTriangles",s.mainFanTriangles);json.Field("ownerFanTriangles",s.ownerFanTriangles);json.Field("filteredMainFan",s.filteredMainFan);json.Field("filteredOwnerFan",s.filteredOwnerFan);json.Field("prototypeAliases",s.aliases);json.Field("representatives",s.representatives);json.Field("materials",s.materials);json.Field("shaders",s.shaders);json.Field("workingVertices",s.workingVertices);json.Field("indexedTriangles",s.indexedTriangles);json.Field("ownedBytes",s.ownedBytes);json.Field("renderBytes",result.size);json.Field("externalMeshFiles",result.meshCount-1);json.Field("skyPortals",result.skyPortalCount);
 json.Raw("\"nodeReports\":[");const NativeRenderCompiler::NodeStats* reports=compiler.NodeReports();
 for(unsigned int n=0;n<s.nodes;n++){if(n)json.Raw(",");json.Raw("{");const NativeRenderCompiler::NodeStats& r=reports[n];json.Field("node",r.node);json.Field("group",r.group);json.Field("sourceTriangles",r.sourceTriangles);json.Field("fixedTriangles",r.fixedTriangles);json.Field("indexedTriangles",r.indexedTriangles);json.Field("vertices",r.vertices);json.Field("surfaces",r.surfaces);json.Field("shadowTriangles",r.shadowTriangles);json.Field("oneSided",r.oneSided);json.Field("twoSided",r.twoSided);json.Field("boundaryEdges",r.boundaryEdges);json.Raw("\"junctionStats\":[");const unsigned int* fields=(const unsigned int*)&r.junction;for(unsigned int i=0;i<6;i++){if(i)json.Raw(",");json.Number(fields[i]);}json.Raw("]}");}
 json.Raw("]\n}\n");return json.Good();
}
static bool Archive(HANDLE file,const RenderSourceGlobalMeshes::Block& block){
 unsigned int header[3]={0x31475257,block.size,block.meshCount};if(!Put(file,header,12)||!Put(file,block.data,block.size))return false;
 for(unsigned int m=0;m<block.meshCount;m++){const RenderGlobalMeshes::Mesh& mesh=block.meshes[m];unsigned int length=mesh.name?lstrlenA(mesh.name):0,record[3]={m,length,mesh.externalBytes};if(!Put(file,record,12)||!Put(file,mesh.name,length)||!Put(file,mesh.external,mesh.externalBytes))return false;}return true;
}
static bool Publish(const Arguments& args,const NativeRenderCompiler::Builder& compiler){
 char archiveTemp[300],reportTemp[300];if(!Temporary(args.archive,archiveTemp)||!Temporary(args.report,reportTemp)||!RenderSectionRules::TextCompare(args.archive,args.report))return false;
 HANDLE archive=CreateFileA(archiveTemp,0x40000000,0,0,1,FILE_ATTRIBUTE_NORMAL,0);if(archive==INVALID_HANDLE_VALUE)return false;
 bool good=Archive(archive,compiler.Result());CloseHandle(archive);if(!good){DeleteFileA(archiveTemp);return false;}
 HANDLE report=CreateFileA(reportTemp,0x40000000,0,0,1,FILE_ATTRIBUTE_NORMAL,0);if(report==INVALID_HANDLE_VALUE){DeleteFileA(archiveTemp);return false;}
 good=Report(report,args,compiler);CloseHandle(report);if(!good){DeleteFileA(archiveTemp);DeleteFileA(reportTemp);return false;}
 // Each file is replaced only after its complete contents have been written.
 // The JSON report is supplementary, rather than another game artifact.
 if(!MoveFileExA(archiveTemp,args.archive,1u|8u)){DeleteFileA(archiveTemp);DeleteFileA(reportTemp);return false;}
 if(!MoveFileExA(reportTemp,args.report,1u|8u)){DeleteFileA(reportTemp);Console("warning: render archive completed; supplementary report could not be replaced");}return true;
}
}
extern "C" void NativeRenderWorkerEntry(){
 SetErrorMode(3);NativeRenderWorker::Arguments args;if(!NativeRenderWorker::Parse(args)){NativeRenderWorker::Console("Usage: native-render-worker --source PATH --project PATH --resources ROOT --archive PATH --report PATH --world-stem PATH [--split-meshes true|false]");ExitProcess(1);}
 if(!NativeRenderWorker::OutputPaths(args,0)){NativeRenderWorker::Console("output: archive/report must be distinct from source/project and outside input resource folders");ExitProcess(1);}
 PrefabScene::Arena workerStorage;void* p=workerStorage.Allocate(sizeof(NativePreparedRenderScene::Context));if(!p)ExitProcess(20);NativePreparedRenderScene::Context* scene=new(p)NativePreparedRenderScene::Context;
 if(!scene->Build(args.source,args.project,NativePreparedRenderScene::Settings())){NativeRenderWorker::Failure("scene",scene->Diagnostic().message);ExitProcess(2);}
 if(!NativeRenderWorker::OutputPaths(args,scene->Project().resources)){NativeRenderWorker::Console("output: archive/report must be outside the prefab resource folder");ExitProcess(1);}
 p=workerStorage.Allocate(sizeof(NativeMaterialLayouts::Cache));if(!p)ExitProcess(20);NativeMaterialLayouts::Cache* materials=new(p)NativeMaterialLayouts::Cache;
 if(!materials->Initialize(args.resources)){NativeRenderWorker::Failure("materials",materials->Error().message);ExitProcess(3);}
 NativeRenderCompiler::Resolver resolver;resolver.resolve=NativeMaterialLayouts::Cache::Callback;resolver.context=materials;NativeRenderCompiler::Settings settings;settings.worldStem=args.stem;settings.projectRoot=args.resources;settings.splitMeshes=args.split;
 p=workerStorage.Allocate(sizeof(NativeRenderCompiler::Builder));if(!p)ExitProcess(20);NativeRenderCompiler::Builder* compiler=new(p)NativeRenderCompiler::Builder;
 if(!compiler->Build(*scene,settings,resolver)){NativeRenderWorker::Failure("render",compiler->Error().message);ExitProcess(4);}
 if(!NativeRenderWorker::Publish(args,*compiler)){NativeRenderWorker::Console("publish: could not write or replace the completed render archive/report");ExitProcess(5);}
 NativeRenderWorker::Console("Native source render completed");compiler->~Builder();scene->~Context();materials->~Cache();workerStorage.Clear();ExitProcess(0);
}
