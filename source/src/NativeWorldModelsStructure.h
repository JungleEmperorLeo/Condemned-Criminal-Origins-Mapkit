#pragma once
#include "NativeWorldStructure.h"
#include "SourceWorldModels.h"
// One prepared scene now produces the physical models and all three structural
// streams. Physics/streaming/render/runtime objects are separate later stages.
namespace NativeWorldModelsStructure {
enum Stage {Idle,StructuralStreams,PhysicalModels,Complete};
struct Settings {unsigned int version;NativeWorldStructure::Settings structural;SourceWorldModels::Settings physical;Settings():version(1){}};
struct Observer {bool(*cancel)(void*);void(*progress)(void*,Stage);void* context;Observer():cancel(0),progress(0),context(0){}};
struct Diagnostic {Stage stage;unsigned int group;char message[512];};
class Builder {
 NativeWorldStructure::Builder structure;SourceWorldModels::Builder models;
 bool ready,active;Observer observer;Diagnostic diagnostic;
 Builder(const Builder&);Builder& operator=(const Builder&);
 void Release(){models.Clear();structure.Clear();ready=false;}
 bool Fail(Stage stage,const char* message,unsigned int group=0xffffffffu){diagnostic.stage=stage;diagnostic.group=group;lstrcpynA(diagnostic.message,message?message:"Source WorldModels structural compilation failed",512);return false;}
 bool Boundary(Stage stage){if(observer.cancel&&observer.cancel(observer.context))return Fail(stage,"Source WorldModels structural compilation cancelled");if(observer.progress)observer.progress(observer.context,stage);return !(observer.cancel&&observer.cancel(observer.context))||Fail(stage,"Source WorldModels structural compilation cancelled");}
public:
 Builder():ready(false),active(false){diagnostic.stage=Idle;diagnostic.group=0xffffffffu;diagnostic.message[0]=0;}
 void Clear(){if(active)return;Release();diagnostic.stage=Idle;diagnostic.group=0xffffffffu;diagnostic.message[0]=0;}
 bool Build(const NativePreparedRenderScene::Context& scene,const Settings& settings=Settings(),const Observer& events=Observer()){
  if(active)return Fail(Idle,"Reentrant source WorldModels structural compilation is unsupported");Clear();active=true;observer=events;bool ok=false;
  if(!scene.Ready()||settings.version!=1)Fail(Idle,"Invalid prepared scene or source WorldModels structural settings");
  else if(!Boundary(StructuralStreams)){}
  else if(!structure.Build(scene,settings.structural))Fail(StructuralStreams,structure.Error().message);
  else if(!Boundary(PhysicalModels)){}
  else if(!models.Build(scene.Model(),scene.Partition(),scene.Offset(),scene.Project(),settings.physical))Fail(PhysicalModels,models.error,models.failedGroup);
  else if(Boundary(Complete)){ready=true;diagnostic.stage=Idle;diagnostic.group=0xffffffffu;diagnostic.message[0]=0;ok=true;}
  active=false;if(!ok)Release();return ok;
 }
 bool Ready()const{return ready;}const Diagnostic& Error()const{return diagnostic;}
 const NativeWorldStructure::Builder& Structural()const{return structure;}
 const SourceWorldModels::Builder& Models()const{return models;}
};
}
