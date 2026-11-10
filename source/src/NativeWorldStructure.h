#pragma once
#include "NativePreparedRenderScene.h"
#include "SourceWorldTree.h"
#include "SourceBlockerPolygons.h"
#include "ParticleBlockers.h"
// Source structural streams that already have independent acceptance evidence.
// Physical BSP/WorldModels are still supplied by a future source compiler;
// a successful Build here is deliberately not whole-World00p success.
namespace NativeWorldStructure {
enum Stage {Idle,WorldTree,Blockers,Particles,Complete};
struct Settings {unsigned int version;float worldNodeSize;Settings():version(1),worldNodeSize(1000){}};
struct Observer {bool(*cancel)(void*);void(*progress)(void*,Stage);void* context;Observer():cancel(0),progress(0),context(0){}};
struct Diagnostic {Stage stage;char message[512];};
class Builder {
 SourceWorldTree::Builder tree;SourceBlockerPolygons::Block blockers;ParticleBlockers::Block particles;
 bool ready,active;Observer observer;Diagnostic diagnostic;
 Builder(const Builder&);Builder& operator=(const Builder&);
 void Release(){particles.Clear();blockers.Clear();tree.Clear();ready=false;}
 bool Fail(Stage stage,const char* message){diagnostic.stage=stage;lstrcpynA(diagnostic.message,message?message:"Source structural compilation failed",512);return false;}
 bool Boundary(Stage stage){if(observer.cancel&&observer.cancel(observer.context))return Fail(stage,"Source structural compilation cancelled");if(observer.progress)observer.progress(observer.context,stage);return !(observer.cancel&&observer.cancel(observer.context))||Fail(stage,"Source structural compilation cancelled");}
public:
 Builder():ready(false),active(false){diagnostic.stage=Idle;diagnostic.message[0]=0;}
 void Clear(){if(active)return;Release();diagnostic.stage=Idle;diagnostic.message[0]=0;}
 bool Build(const NativePreparedRenderScene::Context& scene,const Settings& settings=Settings(),const Observer& events=Observer()){
  if(active)return Fail(Idle,"Reentrant source structural compilation is unsupported");Clear();active=true;observer=events;bool ok=false;
  if(!scene.Ready()||settings.version!=1)Fail(Idle,"Invalid prepared scene or source structural settings");
  else if(!Boundary(WorldTree)){}
  else if(!tree.Build(scene.Model(),scene.Partition(),scene.Offset(),settings.worldNodeSize))Fail(WorldTree,tree.error);
  else if(!Boundary(Blockers)){}
  else if(!blockers.Build(scene.Model(),scene.Partition(),scene.Offset()))Fail(Blockers,blockers.error);
  else if(!Boundary(Particles)){}
  else if(!particles.Build(scene.Model(),scene.Partition(),scene.Offset()))Fail(Particles,particles.error);
  else if(Boundary(Complete)){ready=true;diagnostic.stage=Idle;diagnostic.message[0]=0;ok=true;}
  active=false;if(!ok)Release();return ok;
 }
 bool Ready()const{return ready;}const Diagnostic& Error()const{return diagnostic;}
 const SourceWorldTree::Builder& Tree()const{return tree;}
 const SourceBlockerPolygons::Block& OrdinaryBlockers()const{return blockers;}
 const ParticleBlockers::Block& ParticleBlockersData()const{return particles;}
};
}
