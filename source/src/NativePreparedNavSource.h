#pragma once
#include "GameClassMetadata.h"
#include "WorldCenter.h"
// The source/metadata/expansion phase shared in behavior with the full prepared
// scene, without requiring rendering/visibility from a raw-only Nav fixture.
namespace NativePreparedNavSource {
class Context {
 PrefabScene::Expansion expansion;GameClassMetadata::Catalog catalog;PrefabScene::Project project;
 WorldText::Vec3 offset;char error[512];bool used;
 Context(const Context&);Context& operator=(const Context&);
 static bool Owner(const char* name,void* context){const GameClassMetadata::Class* c=((Context*)context)->catalog.Find(name);return c&&(c->flags&GameClassMetadata::WorldModelFlag)!=0;}
 bool Fail(const char* text){lstrcpynA(error,text?text:"Navigation source preparation failed",512);return false;}
public:
 Context():expansion(true),used(false){offset.x=offset.y=offset.z=0;error[0]=0;}
 const PrefabScene::Model& Model()const{return expansion.Scene();}
 const WorldText::Vec3& Offset()const{return offset;}
 const char* Error()const{return error;}
 bool Build(const char* input,const char* projectPath){
  if(used)return Fail("Navigation source context is single use");used=true;
  char source[MAX_PATH],binary[MAX_PATH],base[MAX_PATH],module[MAX_PATH];
  if(!input||!projectPath||!PrefabScene::Full(input,source)||!project.LoadPath(projectPath))return Fail("Explicit navigation source/project path is invalid");
  DWORD length=GetPrivateProfileStringA("Project Binaries","GameServer","",binary,MAX_PATH,project.file);
  if(!length||length>=MAX_PATH-1)return Fail("Missing or oversized project GameServer path");
  lstrcpyA(base,project.file);PrefabScene::Parent(base);
  if(!PrefabScene::Join(base,binary,module)||!catalog.LoadModule(module))return Fail("Navigation class metadata load failed");
  lstrcpyA(catalog.projectPath,project.file);
  {PrefabScene::Source loaded;if(!loaded.LoadExplicit(source,true,true))return Fail(loaded.scene.doc.error);
   if(!expansion.Build(source,loaded.scene,project,true,false,Owner,this))return Fail(expansion.Error());}
  if(!WorldCenter::Compute(expansion.Scene(),offset))return Fail("Navigation world centering failed");
  return true;
 }
};
}
