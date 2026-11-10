#pragma once
#ifndef FEAR_PRIMARY_SURFACE_PREPARATION
#error NativePreparedRenderScene requires FEAR_PRIMARY_SURFACE_PREPARATION before WorldPacker.cpp
#endif
#if !defined(FEAR_LTA_DECIMAL_COMPAT) || !defined(FEAR_GEOMETRY_DOUBLE_EXPERIMENT) || !defined(FEAR_PLANE_NEWELL_DOUBLE_EXPERIMENT)
#error NativePreparedRenderScene requires the recovered arithmetic profile to be included before source headers
#endif
#include "SourceRenderAllTrees.h"
#include "GameClassMetadata.h"
// One owned and immutable source scene per native compilation. No FEAR_*
// environment setting is consulted by this frontend or recursively by prefabs.
// Class metadata is copied from the explicitly selected project's GameServer;
// neither original WorldPacker nor game objects are instantiated.
namespace NativePreparedRenderScene {
enum Stage {NoStage=0,SettingsStage=1,ProjectStage=2,SourceStage=3,
 ExpansionStage=4,ClassMetadataStage=5,ConsumedStage=6,PartitionStage=7,
 CenterStage=8,VisibilityStage=9,SkyStage=10,TreesStage=11};
enum Code {Ok=0,InvalidArgument=1,UnsupportedProfile=2,PreparationFailed=3};
struct Settings {
 unsigned int version;bool centerWorld,smoothNormals,sharePrototypes;
 bool ignoreHidden,ignoreFrozen; // packer options IgnoreHidden / IgnoreFrozen
 bool objectsOnly; // ObjectsOnly packs stop before World Streaming (no streaming node deletion)
 Settings():version(1),centerWorld(true),smoothNormals(true),sharePrototypes(true),ignoreHidden(false),ignoreFrozen(false),objectsOnly(false){}
};
struct Failure {
 Stage stage;Code code;unsigned int detailCode,sourceNode,group;
 char message[512],sourcePath[MAX_PATH],detail[256];
};
class Context {
 // Declaration/destruction order matters: every borrowed identity below is
 // destroyed before the expansion that owns its model and geometry.
 PrefabScene::Expansion expansion;
 GameClassMetadata::Catalog catalog;
 SourceExpandedRenderContours::Consumed consumed;
 WorldModels::Partition partition;
 VisibilityCompiler::Builder visibility;
 SourceSkyPortals::Builder sky;
 SourceRenderAllTrees::Builder trees;
 PrefabScene::Project project;
 Settings profile;WorldText::Vec3 offset;bool ready,visibilityFatal;char sourcePath[MAX_PATH];
public:
 // Phase boundaries for the packer log only (the hook measures time): 0 start,
 // 1 project+GameServer classes, 2 source load, 3 prefab expansion,
 // 4 consumed/partition/centering, 5 visibility+sky portals, 6 render trees.
 void (*phaseHook)(void*,unsigned int);void* phaseContext;
private:
 Failure failure;
 void Phase(unsigned int n){if(phaseHook)phaseHook(phaseContext,n);}
 Context(const Context&);Context& operator=(const Context&);
 static WorldModels::ClassKind Resolve(const PrefabScene::Node& node,void* context){return GameClassMetadata::Catalog::Resolve(node,&((Context*)context)->catalog);}
  static bool OwnerClass(const char* name,void* context){const GameClassMetadata::Class* definition=((Context*)context)->catalog.Find(name);return definition&&(definition->flags&GameClassMetadata::WorldModelFlag)!=0;}
 static bool Ignore(const PrefabScene::Node& node,void* context){return ((Context*)context)->consumed.Ignore(node);}
 void ResetFailure(){failure.stage=NoStage;failure.code=Ok;failure.detailCode=0;failure.sourceNode=failure.group=0xffffffffu;failure.message[0]=failure.sourcePath[0]=failure.detail[0]=0;}
 bool Fail(Stage stage,Code code,const char* message,unsigned int detailCode=0,const char* detail=0){
  // Error strings can point into a builder that Clear invalidates. Copy first.
  Failure saved;saved.stage=stage;saved.code=code;saved.detailCode=detailCode;saved.sourceNode=saved.group=0xffffffffu;
  lstrcpynA(saved.message,message?message:"Source scene preparation failed",512);lstrcpynA(saved.sourcePath,sourcePath,MAX_PATH);lstrcpynA(saved.detail,detail?detail:"",256);
  Clear();failure=saved;return false;
 }
public:
 Context():expansion(true),ready(false),visibilityFatal(false),phaseHook(0),phaseContext(0){sourcePath[0]=0;offset.x=offset.y=offset.z=0;ResetFailure();}
 ~Context(){Clear();}
 // Clear/reuse invalidate every view; successful Build retains no borrowed
 // source text or module pointer. A failed Build leaves no partial scene ready.
 void Clear(){
  ready=false;visibilityFatal=false;trees.Clear();sky.~Builder();new(&sky)SourceSkyPortals::Builder();visibility.~Builder();new(&visibility)VisibilityCompiler::Builder();
  partition.Clear();consumed.~Consumed();new(&consumed)SourceExpandedRenderContours::Consumed();catalog.Clear();
  expansion.~Expansion();new(&expansion)PrefabScene::Expansion(true);project=PrefabScene::Project();profile=Settings();offset.x=offset.y=offset.z=0;sourcePath[0]=0;ResetFailure();
 }
 bool Ready()const{return ready;}
 // Build stopped only by intersecting visibility sectors (the model is kept).
 bool VisibilityFatal()const{return visibilityFatal;}
 const Failure& Diagnostic()const{return failure;}
 const PrefabScene::Model& Model()const{return expansion.Scene();}
 const GameClassMetadata::Catalog& Catalog()const{return catalog;}
 const SourceExpandedRenderContours::Consumed& Consumed()const{return consumed;}
 const WorldModels::Partition& Partition()const{return partition;}
 const VisibilityCompiler::Builder& Visibility()const{return visibility;}
 const SourceSkyPortals::Builder& Sky()const{return sky;}
 const SourceRenderAllTrees::Builder& Trees()const{return trees;}
 const WorldText::Vec3& Offset()const{return offset;}
 const PrefabScene::Project& Project()const{return project;}
 const Settings& Profile()const{return profile;}
 const char* SourcePath()const{return sourcePath;}
 __declspec(noinline) bool Build(const char* input,const char* projectPath,const Settings& settings=Settings()){
  Clear();profile=settings;Phase(0);
  if(!input||!input[0]||!projectPath||!projectPath[0])return Fail(SettingsStage,InvalidArgument,"Explicit source and project paths are required");
  if(!PrefabScene::Full(input,sourcePath))return Fail(SourceStage,InvalidArgument,"Invalid explicit source path");
  if(settings.version!=1||!settings.smoothNormals||!settings.sharePrototypes)return Fail(SettingsStage,UnsupportedProfile,"Native render requires recovered smooth/shared profile version 1");
  if(!project.LoadPath(projectPath))return Fail(ProjectStage,PreparationFailed,project.error);
  char binary[MAX_PATH],base[MAX_PATH],modulePath[MAX_PATH];
  DWORD length=GetPrivateProfileStringA("Project Binaries","GameServer","",binary,MAX_PATH,project.file);
  if(!length||length>=MAX_PATH-1)return Fail(ClassMetadataStage,PreparationFailed,"Missing or oversized project GameServer path");
  lstrcpyA(base,project.file);PrefabScene::Parent(base);
  if(!PrefabScene::Join(base,binary,modulePath))return Fail(ClassMetadataStage,PreparationFailed,"Project GameServer path exceeds MAX_PATH");
  if(!catalog.LoadModule(modulePath))return Fail(ClassMetadataStage,PreparationFailed,catalog.error,0,modulePath);
  lstrcpyA(catalog.projectPath,project.file);Phase(1);
   {
    PrefabScene::Source source;if(!source.LoadExplicit(sourcePath,true,true))return Fail(SourceStage,PreparationFailed,source.scene.doc.error,source.scene.doc.errorOffset);
    Phase(2);
    expansion.SetIgnore(settings.ignoreHidden,settings.ignoreFrozen);
    if(!expansion.Build(sourcePath,source.scene,project,true,false,OwnerClass,this))return Fail(ExpansionStage,PreparationFailed,expansion.Error(),0,expansion.ErrorPath());
    expansion.SetUncentered(!settings.centerWorld);
   }
  Phase(3);
  const PrefabScene::Model& model=expansion.Scene();
  if(!consumed.Build(model))return Fail(ConsumedStage,PreparationFailed,consumed.error);
  if(!partition.Build(model,Resolve,this,Ignore))return Fail(PartitionStage,PreparationFailed,partition.error);
  if(!WorldCenter::Compute(model,offset))return Fail(CenterStage,PreparationFailed,"World centering failed");
  Phase(4);
  // Intersecting sectors stop the original only at its Visibility stage: the
  // model, partition and notes stay available for the earlier stages and the log.
  if(!visibility.Build(model,&offset)){if(visibility.intersecting){visibilityFatal=true;failure.stage=VisibilityStage;failure.code=PreparationFailed;lstrcpyA(failure.message,"Intersecting visibility sectors");return false;}return Fail(VisibilityStage,PreparationFailed,"Visibility compiler failed",visibility.errorCode,visibility.errorSector);}
  if(!sky.Build(model,&visibility))return Fail(SkyStage,PreparationFailed,sky.error);
  Phase(5);
  if(!trees.Build(model,partition,visibility,&sky,true))return Fail(TreesStage,PreparationFailed,trees.error);
  Phase(6);ready=true;return true;
 }
};
}
