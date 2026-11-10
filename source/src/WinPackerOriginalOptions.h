#pragma once
// The original WorldPacker property set (RequestUserOptions VA 0x10022960,
// PropertyChanged 0x10022730, Process 0x100227D0), recorded with
// monolith-research/packer-log-recorder: groups Common/Advanced, two interface
// rows (type 4, style 1 title / 4 separator), the Separator reference into
// Advanced, eight booleans with the original names, defaults and help text.
#include "WinPackerNativeOptions.h"
class CPackerInterfaceProperty:public CPackerProperty {
 unsigned int style;
public:
 CPackerInterfaceProperty(const char* n,unsigned int s,const char* h):CPackerProperty(n,h),style(s){}
 unsigned int GetType()const{return 4;}unsigned int GetStyle()const{return style;}
 bool SaveValue(char*,unsigned int)const{return true;}
 bool LoadValue(char**,unsigned int){return true;}
};
static_assert(sizeof(CPackerInterfaceProperty)==20,"Interface property ABI");
namespace WinPackerOriginalOptions {
struct Settings {bool objectsOnly,ignoreHidden,ignoreFrozen,optimizeVertices,rawNavMesh,centerWorld,splitMeshes,createObjectFile;};
inline bool Register(IPackerUI* ui){
 if(!ui)return false;const char* common="Common";const char* advanced="Advanced";
 CPackerInterfaceProperty title("Process Settings",1,0),separator("Separator",4,0),advancedTitle("Advanced",1,0);
 CPackerBoolProperty objectsOnly("ObjectsOnly",false,"Determines if only objects should be processed");
 CPackerBoolProperty ignoreHidden("IgnoreHidden",false,"If all hidden nodes should be ignored while processing");
 CPackerBoolProperty ignoreFrozen("IgnoreFrozen",false,"If all frozen nodes should be ignored while processing");
 CPackerBoolProperty optimize("OptimizeVertices",true,"Determines if the vertices of the level should be optimized. This speeds up runtime, but takes longer to process");
 CPackerBoolProperty raw("RawNavMesh",false,"Determines if the NavMesh should not be processed and packed as raw data");
 CPackerBoolProperty center("CenterWorld",true,"This will center the world around the origin in order to reduce numerical accuracy errors");
 CPackerBoolProperty split("SplitMeshes",false,"Specifies whether or not the meshes in this level should be broken into separate resources for streaming");
 CPackerBoolProperty objectFile("CreateObjectFile",false,"Specifies whether or not the level should be packed or if just an external object file should be created");
 return ui->CreateProperty(title,common)&&ui->CreateProperty(separator,common)&&ui->CreateProperty(objectsOnly,common)&&ui->CreateProperty(ignoreHidden,common)&&
  ui->CreateProperty(ignoreFrozen,common)&&ui->CreateProperty(optimize,common)&&ui->CreateProperty(advancedTitle,advanced)&&ui->CreateReference("Separator",advanced)&&
  ui->CreateProperty(raw,advanced)&&ui->CreateProperty(center,advanced)&&ui->CreateProperty(split,advanced)&&ui->CreateProperty(objectFile,advanced);
}
// 0x10053C20: first property whose name matches (_stricmp).
inline CPackerProperty* Find(const CPackerPropList* list,const char* name){
 if(!list||!list->properties)return 0;for(unsigned int i=0;i<list->count;i++){CPackerProperty* p=list->properties[i];if(p&&WinPackerNativeOptions::Equal(p->GetName(),name))return p;}return 0;
}
// 0x10053CB0: a missing or non-boolean property yields the caller's default.
inline bool Bool(const CPackerPropList* list,const char* name,bool fallback){
 const CPackerProperty* p=Find(list,name);if(!p||p->GetType()!=0)return fallback;return *((const unsigned char*)p+16)!=0;
}
inline void Enable(CPackerPropList* list,const char* name,bool enabled){CPackerProperty* p=Find(list,name);if(p)p->SetEnabled(enabled);}
// 0x10022730: ObjectsOnly is available unless CreateObjectFile is set;
// OptimizeVertices and SplitMeshes only when neither is set.
inline void Changed(CPackerPropList* list){
 bool objectsOnly=Bool(list,"ObjectsOnly",false),objectFile=Bool(list,"CreateObjectFile",false);
 Enable(list,"ObjectsOnly",!objectFile);Enable(list,"OptimizeVertices",!objectsOnly&&!objectFile);Enable(list,"SplitMeshes",!objectsOnly&&!objectFile);
}
// 0x100227D0: CreateObjectFile implies ObjectsOnly.
inline Settings Read(const CPackerPropList* list){
 Settings s;s.createObjectFile=Bool(list,"CreateObjectFile",false);s.splitMeshes=Bool(list,"SplitMeshes",false);s.objectsOnly=Bool(list,"ObjectsOnly",false);
 s.ignoreHidden=Bool(list,"IgnoreHidden",false);s.ignoreFrozen=Bool(list,"IgnoreFrozen",false);s.optimizeVertices=Bool(list,"OptimizeVertices",true);
 s.rawNavMesh=Bool(list,"RawNavMesh",false);s.centerWorld=Bool(list,"CenterWorld",true);if(s.createObjectFile)s.objectsOnly=true;return s;
}
}
