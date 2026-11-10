#include "PrimarySurfacePreparationConfig.h"
#include "win_min.h"
#include "IPackerImpl.h"
#include "IPackerOutput.h"
#include "../../worldpacker-common/BrushFlags.h"

static HMODULE moduleHandle;
static char* Decimal(char* p,unsigned int value){char tmp[16];unsigned int n=0;do{tmp[n++]=(char)('0'+value%10);value/=10;}while(value);while(n)*p++=tmp[--n];return p;}
static void Append(char*& p,const char* s){while(*s)*p++=*s++;}
static void LogLine(const char* message){
 char path[MAX_PATH];GetModuleFileNameA(moduleHandle,path,MAX_PATH);
 char* leaf=path;for(char* p=path;*p;p++)if(*p=='\\')leaf=p+1;
 lstrcpyA(leaf,"rebuild.log");
 HANDLE h=CreateFileA(path,FILE_APPEND_DATA,FILE_SHARE_READ,0,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,0);
 if(h==INVALID_HANDLE_VALUE)return;DWORD done;WriteFile(h,message,lstrlenA(message),&done,0);WriteFile(h,"\r\n",2,&done,0);CloseHandle(h);
}
static void Report(IPackerOutput* output,EMsgType level,const char* message){LogLine(message);if(output)output->LogMessage(level,message);}
#include "WorldText.h"
#include "SceneReport.h"
#include "LtcDecode.h"
#include "PolygonCheck.h"
#include "PrefabScene.h"
#include "VisibilityCompiler.h"
extern "C" void* __cdecl memcpy(void*,const void*,unsigned int);
#ifdef FEAR_PRIMARY_SURFACE_PREPARATION
#include "SourcePreparedSurfaceOutput.h"
#endif
#ifndef FEAR_LIBRARY_ONLY
class WorldPacker:public IPackerImpl{
public:
 bool GetPackerName(char* buffer,int size){if(!buffer||size<1)return false;lstrcpynA(buffer,"FEAR WorldPacker Rebuild (partial compiler)",size);return true;}
 bool RequestUserOptions(IPackerUI*){return true;}
 bool PropertyChanged(CPackerProperty*,CPackerPropList*,IPackerUI*){return true;}
 bool Process(const char* filename,CPackerPropList*,IPackerOutput* output){
  if(output){output->CreateTask("Reading source world");output->ActivateTask("Reading source world");}
  if(!filename){Report(output,MSG_ERROR,"No input path supplied");return false;}
  HANDLE file=CreateFileA(filename,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);
  if(file==INVALID_HANDLE_VALUE){Report(output,MSG_ERROR,"Could not open source world");return false;}
  DWORD size=GetFileSize(file,0);
  if(size==0xFFFFFFFF || size>64*1024*1024){CloseHandle(file);Report(output,MSG_ERROR,"Source world is too large or unreadable (64 MiB limit)");return false;}
  char* data=(char*)VirtualAlloc(0,size+1,MEM_COMMIT|MEM_RESERVE,4);
  if(!data){CloseHandle(file);Report(output,MSG_ERROR,"Could not allocate memory for source world");return false;}
  DWORD read=0;BOOL ok=ReadFile(file,data,size,&read,0);CloseHandle(file);
  if(!ok || read!=size){VirtualFree(data,0,MEM_RELEASE);Report(output,MSG_ERROR,"Could not read complete source world");return false;}
  data[size]=0;
  const char* extension=filename;for(const char* q=filename;*q;q++)if(*q=='.')extension=q;
  bool compressed=FearBrushFlags::Equal(extension,".World00c")||FearBrushFlags::Equal(extension,".ltc")||(size>=4&&!data[0]&&!data[1]&&!data[2]&&!data[3]);
  LtcDecode decoder;WorldText::Scene scene;bool loaded=false,decoderError=false;
  char reportOption[4]={};bool diagnosticReports=!(GetEnvironmentVariableA("FEAR_PROCESS_REPORTS",reportOption,4)&&reportOption[0]=='0');
  if(compressed&&!decoder.Decode(data,size)){decoderError=true;scene.doc.source=data;scene.doc.bytes=size;scene.doc.Fail(decoder.error,decoder.errorOffset);}
  else loaded=scene.Load(compressed?decoder.data:data,compressed?decoder.size:size);
  char reportPath[MAX_PATH];GetModuleFileNameA(moduleHandle,reportPath,MAX_PATH);char* leaf=reportPath;for(char* q=reportPath;*q;q++)if(*q=='\\')leaf=q+1;lstrcpyA(leaf,"scene.json");
  SceneReport report;if(diagnosticReports)report.Save(reportPath,loaded,scene,compressed,size,compressed?decoder.size:size,decoderError,compressed&&decoder.shortTerminator);
  if(diagnosticReports&&compressed&&loaded){char decodedPath[MAX_PATH];GetModuleFileNameA(moduleHandle,decodedPath,MAX_PATH);char* decodedLeaf=decodedPath;for(char* q=decodedPath;*q;q++)if(*q=='\\')decodedLeaf=q+1;lstrcpyA(decodedLeaf,"decoded.World00a");HANDLE saved=CreateFileA(decodedPath,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(saved!=INVALID_HANDLE_VALUE){DWORD written;WriteFile(saved,decoder.data,decoder.size,&written,0);CloseHandle(saved);}}
  char message[256];char* p=message;
  if(!loaded){Append(p,decoderError?"Compressed source error at byte ":"World text error at byte ");p=Decimal(p,scene.doc.errorOffset);Append(p,": ");Append(p,scene.doc.error?scene.doc.error:"unknown error");*p=0;Report(output,MSG_ERROR,message);VirtualFree(data,0,MEM_RELEASE);return false;}
  Append(p,"World00a scene: ");p=Decimal(p,scene.brushCount);Append(p," active brushes, ");p=Decimal(p,scene.objectCount);Append(p," objects, ");p=Decimal(p,scene.prefabCount);Append(p," prefab references; ");p=Decimal(p,scene.activePoints);Append(p," points, ");p=Decimal(p,scene.activePolygons);Append(p," polygons");*p=0;Report(output,MSG_NORMAL,message);
  lstrcpyA(leaf,"expanded-scene.json");
#ifdef FEAR_PRIMARY_SURFACE_PREPARATION
  PrefabScene::SourceMappingFiniteRepair::Prepare(scene);
  if(!PrefabScene::SourceMissingTextureMappings::Prepare(scene)){Report(output,MSG_ERROR,"Could not prepare missing texture mappings");VirtualFree(data,0,MEM_RELEASE);return false;}
#endif
  PrefabScene::InaccurateGeometry::Prepare(scene);
  void* expansionMemory=VirtualAlloc(0,sizeof(PrefabScene::Expansion),MEM_COMMIT|MEM_RESERVE,4);
  if(expansionMemory){
   PrefabScene::Expansion* expansion=new(expansionMemory) PrefabScene::Expansion(true);
   bool built=expansion->Build(filename,scene),saved=diagnosticReports?expansion->SaveBuilt(reportPath,built):built;
   if(built){const PrefabScene::Model& model=expansion->Scene();p=message;Append(p,"Owned scene prepared: ");p=Decimal(p,model.count);Append(p," nodes, ");p=Decimal(p,model.cleanContours);Append(p," clean contours, ");p=Decimal(p,model.problemContours);Append(p," contours need processing");*p=0;Report(output,MSG_NORMAL,message);}
   if(diagnosticReports)Report(output,saved?MSG_NORMAL:MSG_WARNING,saved?"Owned prefab scene saved; see expanded-scene.json":"Scene preparation or report write failed; see expanded-scene.json");
   if(built){
#ifdef FEAR_PRIMARY_SURFACE_PREPARATION
    char surfacePath[MAX_PATH];DWORD selectedSurface=GetEnvironmentVariableA("FEAR_PROCESS_SURFACE_OUTPUT",surfacePath,MAX_PATH);
    if(!selectedSurface){lstrcpyA(leaf,"prepared-surfaces.bin");lstrcpyA(surfacePath,reportPath);}
    SourcePreparedSurfaceOutput::Stats surfaceStats={};
    bool surfacesWritten=selectedSurface<MAX_PATH&&SourcePreparedSurfaceOutput::Save(expansion->Scene(),surfacePath,surfaceStats);
    if(surfacesWritten){p=message;Append(p,"Source surfaces prepared: ");p=Decimal(p,surfaceStats.surfaces);Append(p," surfaces, ");p=Decimal(p,surfaceStats.fragments);Append(p," fragments, ");p=Decimal(p,surfaceStats.rejected);Append(p," rejected surfaces");*p=0;Report(output,surfaceStats.rejected?MSG_WARNING:MSG_NORMAL,message);}
    else Report(output,MSG_ERROR,"Could not save prepared source surfaces");
#endif
    if(output){output->CreateTask("Creating visibility");output->ActivateTask("Creating visibility");}
    void* compilerMemory=VirtualAlloc(0,sizeof(VisibilityCompiler::Builder),MEM_COMMIT|MEM_RESERVE,4);
    if(compilerMemory){
     VisibilityCompiler::Builder* compiler=new(compilerMemory) VisibilityCompiler::Builder;
     if(compiler->Build(expansion->Scene())){
      char visibilityPath[MAX_PATH];DWORD selected=GetEnvironmentVariableA("FEAR_PROCESS_VISIBILITY_OUTPUT",visibilityPath,MAX_PATH);
      if(!selected){lstrcpyA(leaf,"visibility.bin");lstrcpyA(visibilityPath,reportPath);}
      HANDLE visibilityFile=selected>=MAX_PATH?INVALID_HANDLE_VALUE:CreateFileA(visibilityPath,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);DWORD done=0;
      bool written=visibilityFile!=INVALID_HANDLE_VALUE&&WriteFile(visibilityFile,compiler->data,compiler->size,&done,0)&&done==compiler->size;if(visibilityFile!=INVALID_HANDLE_VALUE)CloseHandle(visibilityFile);
      if(written){p=message;Append(p,"Visibility section compiled: ");p=Decimal(p,compiler->sectorCount);Append(p," sectors, ");p=Decimal(p,compiler->size);Append(p," bytes");*p=0;Report(output,MSG_NORMAL,message);
       lstrcpyA(leaf,"visibility.offset.bin");HANDLE offsetFile=CreateFileA(reportPath,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(offsetFile!=INVALID_HANDLE_VALUE){WriteFile(offsetFile,&compiler->offset,12,&done,0);CloseHandle(offsetFile);}
      }else Report(output,MSG_ERROR,"Could not save compiled visibility section");
     }else{p=message;Append(p,"Visibility compilation failed at stage ");p=Decimal(p,compiler->errorCode);*p=0;Report(output,MSG_ERROR,message);}
     compiler->~Builder();VirtualFree(compilerMemory,0,MEM_RELEASE);
    }else Report(output,MSG_ERROR,"Could not allocate visibility compiler");
   }
   expansion->~Expansion();VirtualFree(expansionMemory,0,MEM_RELEASE);
  }
  else Report(output,MSG_ERROR,"Could not allocate prefab expansion state");
  VirtualFree(data,0,MEM_RELEASE);
  Report(output,MSG_WARNING,"Partial compilation only; complete World00p assembly is not implemented yet");
  return false;
 }
};
__declspec(align(8)) static char storage[sizeof(WorldPacker)];
extern "C" __declspec(dllexport) IPackerImpl* AssociatePacker(const char* filename){LogLine("AssociatePacker");if(!filename)return 0;return new(storage) WorldPacker;}
BOOL WINAPI DllMain(HINSTANCE h,DWORD reason,LPVOID){if(reason==DLL_PROCESS_ATTACH)moduleHandle=h;return TRUE;}
#endif
#ifndef FEAR_NATIVE_NV_RUNTIME
void operator delete(void*,unsigned int){}
#endif
extern "C" int __cdecl _purecall(){return 0;}
#pragma comment(linker,"/alternatename:__imp__GetModuleFileNameA@12=__imp__GetModuleFileNameA")
#pragma comment(linker,"/alternatename:__imp__lstrcpyA@8=__imp__lstrcpyA")
#pragma comment(linker,"/alternatename:__imp__lstrcpynA@12=__imp__lstrcpynA")
#pragma comment(linker,"/alternatename:__imp__lstrlenA@4=__imp__lstrlenA")
#pragma comment(linker,"/alternatename:__imp__CreateFileA@28=__imp__CreateFileA")
#pragma comment(linker,"/alternatename:__imp__WriteFile@20=__imp__WriteFile")
#pragma comment(linker,"/alternatename:__imp__ReadFile@20=__imp__ReadFile")
#pragma comment(linker,"/alternatename:__imp__GetFileSize@8=__imp__GetFileSize")
#pragma comment(linker,"/alternatename:__imp__VirtualAlloc@16=__imp__VirtualAlloc")
#pragma comment(linker,"/alternatename:__imp__VirtualFree@12=__imp__VirtualFree")
#pragma comment(linker,"/alternatename:__imp__CloseHandle@4=__imp__CloseHandle")
extern "C" void* __cdecl memset(void* dest,int value,unsigned int bytes){unsigned char* p=(unsigned char*)dest;while(bytes--)*p++=(unsigned char)value;return dest;}




extern "C" { int _fltused=0; }

extern "C" void* __cdecl memcpy(void* dest,const void* src,unsigned int bytes){unsigned char* d=(unsigned char*)dest;const unsigned char* s=(const unsigned char*)src;while(bytes--)*d++=*s++;return dest;}
