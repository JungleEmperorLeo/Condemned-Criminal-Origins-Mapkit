#define FEAR_PRIMARY_SURFACE_PREPARATION 1
#include "WorldPacker.cpp"
#include "NativePreparedRenderScene.h"
#include "SourceWorldTree.h"
#include "StreamingConvex.h"
#include "StreamingSettings.h"
#include "NativeRawNavMesh.h"
extern "C" {
__declspec(dllimport) void WINAPI ExitProcess(unsigned int);
__declspec(dllimport) char* WINAPI GetCommandLineA();
__declspec(dllimport) unsigned int WINAPI SetErrorMode(unsigned int);
}
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
#pragma comment(linker,"/alternatename:__imp__GetCommandLineA@0=__imp__GetCommandLineA")
#pragma comment(linker,"/alternatename:__imp__SetErrorMode@4=__imp__SetErrorMode")
namespace WinPackerSections {
static bool Token(const char*& p,char* output){while(*p==' '||*p=='\t')p++;if(!*p)return false;unsigned int n=0;bool quoted=false;while(*p&&(quoted||(*p!=' '&&*p!='\t'))){if(*p=='"'){quoted=!quoted;p++;continue;}if(n==259)return false;output[n++]=*p++;}output[n]=0;return n&&!quoted;}
static bool Put(HANDLE f,const void* p,unsigned int n){DWORD done=0;return !n||(WriteFile(f,p,n,&done,0)&&done==n);}
static bool String(HANDLE f,const char* p){return Put(f,p,lstrlenA(p));}
static void JsonString(HANDLE f,const char* text){String(f,"\"");for(const unsigned char* p=(const unsigned char*)text;*p;p++){char part[7];if(*p=='"'||*p=='\\'){part[0]='\\';part[1]=(char)*p;Put(f,part,2);}else if(*p<32||*p>=128){const char* h="0123456789abcdef";part[0]='\\';part[1]='u';part[2]=part[3]='0';part[4]=h[*p>>4];part[5]=h[*p&15];Put(f,part,6);}else Put(f,p,1);}String(f,"\"");}
static HANDLE Open(const char* folder,const char* name){char path[MAX_PATH];return PrefabScene::Join(folder,name,path)?CreateFileA(path,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0):INVALID_HANDLE_VALUE;}
static bool Save(const char* folder,const char* name,const void* p,unsigned int n){HANDLE f=Open(folder,name);if(f==INVALID_HANDLE_VALUE)return false;bool ok=Put(f,p,n);CloseHandle(f);return ok;}
// The original reads the authored asset list beside the source world
// (Worlds\Release\WTF_Entry.World00c -> WTF_Entry.Asset00); absent is only a warning.
static bool AssetListText(const char* source,PrefabScene::Arena& memory,const unsigned char*& text,unsigned int& size){
 text=0;size=0;char path[MAX_PATH];if(!PrefabScene::Full(source,path))return false;unsigned int dot=0,length=lstrlenA(path);
 for(unsigned int i=0;i<length;i++){if(path[i]=='.')dot=i;else if(path[i]=='\\'||path[i]=='/')dot=0;}if(!dot)dot=length;if(dot+9>=MAX_PATH)return false;lstrcpyA(path+dot,".Asset00");
 HANDLE f=CreateFileA(path,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);if(f==INVALID_HANDLE_VALUE)return true;
 DWORD high=0,low=GetFileSize(f,&high),done=0;unsigned char* bytes=!high&&low<=8*1024*1024?(unsigned char*)memory.Allocate(low+1):0;
 bool ok=bytes&&(!low||(ReadFile(f,bytes,low,&done,0)&&done==low));CloseHandle(f);if(!ok)return false;text=bytes;size=low;return true;
}
static const char* Unsupported(const PrefabScene::Node& n,bool navReady,const char* navError){
 if(n.kind!=2)return 0;
 if(GameClassMetadata::Exact(n.className,"AINavMesh"))return navReady?0:(navError?navError:"Navigation requires the implemented single source quad profile");
 if(GameClassMetadata::Exact(n.className,"ScatterFX"))return "Native BlindData producer is not integrated for this source class";
  return 0;
}
static bool Capabilities(const char* folder,const NativePreparedRenderScene::Context& scene,unsigned int regionCount){
 HANDLE f=Open(folder,"capabilities.json");if(f==INVALID_HANDLE_VALUE)return false;unsigned int issues=0,objects=0,navRoots=0;for(const PrefabScene::Node* n=scene.Model().first;n;n=n->next)if(n->kind==2&&GameClassMetadata::Exact(n->className,"AINavMesh"))navRoots++;
 NativeRawNavMesh::Builder nav;bool navReady=navRoots&&nav.Build(scene.Model(),scene.Offset());
 for(const PrefabScene::Node* n=scene.Model().first;n;n=n->next){if(n->kind==2)objects++;if(Unsupported(*n,navReady,nav.error))issues++;}
 String(f,"{\"sourceOnly\":true,\"supportedWholeWorld\":");String(f,issues?"false":"true");String(f,",\"centerWorld\":true,\"splitMeshes\":false,\"streamingRegions\":");char number[16];*Decimal(number,regionCount)=0;String(f,number);String(f,",\"sourceObjects\":");*Decimal(number,objects)=0;String(f,number);String(f,",\"navMeshSourceRoots\":");*Decimal(number,navRoots)=0;String(f,number);String(f,",\"profile\":\"initial source-derived inline DLL integration; no broader compatibility claim\",\"unsupported\":[");bool comma=false;
 for(const PrefabScene::Node* n=scene.Model().first;n;n=n->next){const char* reason=Unsupported(*n,navReady,nav.error);if(!reason)continue;if(comma)String(f,",");comma=true;String(f,"{\"className\":");JsonString(f,n->className);String(f,",\"sourceNode\":");*Decimal(number,n->index)=0;String(f,number);String(f,",\"reason\":");JsonString(f,reason);String(f,"}");}
 bool ok=String(f,"]}\n");CloseHandle(f);return ok;
}
}
extern "C" void HarnessEntry(){using namespace WinPackerSections;SetErrorMode(3);char source[MAX_PATH]={},project[MAX_PATH]={},folder[MAX_PATH]={},stem[MAX_PATH]={},key[MAX_PATH],value[MAX_PATH];const char* command=GetCommandLineA();if(!Token(command,key))ExitProcess(1);unsigned int seen=0;
 while(*command){while(*command==' '||*command=='\t')command++;if(!*command)break;if(!Token(command,key)||!Token(command,value))ExitProcess(1);char* target=0;unsigned int flag=0;if(FearBrushFlags::Equal(key,"--source")){target=source;flag=1;}else if(FearBrushFlags::Equal(key,"--project")){target=project;flag=2;}else if(FearBrushFlags::Equal(key,"--output")){target=folder;flag=4;}else if(FearBrushFlags::Equal(key,"--world-stem")){target=stem;flag=8;}if(!target||(seen&flag))ExitProcess(1);seen|=flag;lstrcpyA(target,value);}
 if(seen!=15)ExitProcess(1);PrefabScene::Arena memory;void* place=memory.Allocate(sizeof(NativePreparedRenderScene::Context));if(!place)ExitProcess(2);NativePreparedRenderScene::Context* scene=new(place)NativePreparedRenderScene::Context;
 if(!scene->Build(source,project)){Save(folder,"error.txt",scene->Diagnostic().message,lstrlenA(scene->Diagnostic().message));ExitProcess(3);}
 SourceWorldTree::Builder tree;if(!tree.Build(scene->Model(),scene->Partition(),scene->Offset())){Save(folder,"error.txt",tree.error,lstrlenA(tree.error));ExitProcess(4);}float vectors[9];memcpy(vectors,&tree.low,12);memcpy(vectors+3,&tree.high,12);memcpy(vectors+6,&scene->Offset(),12);
 if(!Save(folder,"header-vectors.bin",vectors,36)||!Save(folder,"visibility.bin",scene->Visibility().data,scene->Visibility().size))ExitProcess(5);
 StreamingSettings::Settings streaming;if(!streaming.Read(scene->Model()))ExitProcess(6);HANDLE f=Open(folder,"streaming.input");if(f==INVALID_HANDLE_VALUE)ExitProcess(5);bool good=Put(f,stem,lstrlenA(stem)+1)&&Put(f,&streaming.count,4);for(unsigned int i=0;good&&i<streaming.count;i++)good=Put(f,streaming.names[i],lstrlenA(streaming.names[i])+1)&&Put(f,streaming.budgets+i,4);const unsigned char* assetText;unsigned int assetBytes,noSeeds=0;if(!AssetListText(source,memory,assetText,assetBytes)){CloseHandle(f);Save(folder,"error.txt","Asset list read failed",22);ExitProcess(7);}
 good=good&&Put(f,&noSeeds,4)&&Put(f,&assetBytes,4)&&Put(f,assetText,assetBytes);CloseHandle(f);
 if(!good||!Capabilities(folder,*scene,streaming.count))ExitProcess(5);scene->~Context();memory.Clear();ExitProcess(0);
}
