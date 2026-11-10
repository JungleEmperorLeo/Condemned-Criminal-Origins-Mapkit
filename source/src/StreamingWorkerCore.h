#pragma once
// Shared streaming/resource assembly (formerly the body of streaming-assembly-check.cpp).
// The worker EXE and the monolithic compiler call Run with an explicit Request.
#include "VisibilityCompiler.h"
#include "StreamingBlock.h"
#include "StreamingTables.h"
#include "AssetList.h"
#include "StreamingResources.h"
#include "StreamingAssembly.h"
#include "StreamingSettings.h"
#include "SourceMaterialSeeds.h"
#include "ShaderDependencyGate.h"
#include "SoundDatabaseResources.h"
#include "ClientFxResources.h"
#include "WeaponSurfaceResources.h"
#include "VoiceDatabaseResources.h"
namespace StreamingWorkerCore {
struct Request {const char* source;const unsigned char* input;unsigned int bytes;const PrefabScene::Model* model;bool sourceMaterials,soundDatabase,clientFx,visibility,automaticPost,tutorialPost,multiplayerPost,singleplayerPost,condemnedGather;char textureMode;
 Request():source(0),input(0),bytes(0),model(0),sourceMaterials(false),soundDatabase(false),clientFx(false),visibility(false),automaticPost(false),tutorialPost(false),multiplayerPost(false),singleplayerPost(false),condemnedGather(false),textureMode(0){}};
}
static char path[MAX_PATH];static const unsigned char* input;static unsigned int bytes,cursor,streamFailure;static bool requestClientFx;
static unsigned int U(){if(cursor>bytes||bytes-cursor<4){if(!streamFailure)streamFailure=10;return 0;}unsigned int v=*(unsigned int*)(input+cursor);cursor+=4;return v;}
static const char* S(){unsigned int start=cursor;while(cursor<bytes&&input[cursor])cursor++;if(cursor==bytes||cursor-start>4096){if(!streamFailure)streamFailure=11;return "";}cursor++;return (const char*)(input+start);}
struct MaterialReader {PrefabScene::Project project;PrefabScene::Arena memory,combined;ShaderDependencyGate::Compiler gate;SoundDatabaseResources::Reader sounds;ClientFxResources::Reader effects;WeaponSurfaceResources weaponSurfaces;VoiceDatabaseResources voices;};
static bool ReadSourceMaterial(void* context,const char* relative,const unsigned char*& data,unsigned int& size){
 MaterialReader& reader=*(MaterialReader*)context;reader.memory.Clear();while(*relative=='\\'||*relative=='/')relative++;char target[MAX_PATH];if(!PrefabScene::Join(reader.project.resources,relative,target))return false;
 HANDLE f=CreateFileA(target,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);if(f==INVALID_HANDLE_VALUE)return false;size=GetFileSize(f,0);DWORD done;bool ok=false;
 if(size&&size<=16*1024*1024){data=(const unsigned char*)reader.memory.Allocate(size);ok=data&&ReadFile(f,(void*)data,size,&done,0)&&done==size;}CloseHandle(f);return ok;
}
static bool SourceShaderReady(void* context,const char* shader){MaterialReader& reader=*(MaterialReader*)context;char target[MAX_PATH];return PrefabScene::Join(reader.project.resources,shader,target)&&reader.gate.Check(target);}
static bool SourceTextureReady(void* context,const char* name){return ((MaterialReader*)context)->gate.HasTexture(name);}
static bool DatabaseEffectResources(void* context,const char* name,const char**& paths,unsigned int& count,bool& found){MaterialReader& reader=*(MaterialReader*)context;return reader.effects.Initialize(reader.project)&&reader.effects.Resolve(name,paths,count,found);}
static bool SourceEffectResources(void* context,const char* name,const char**& paths,unsigned int& count,bool& found,unsigned int){return DatabaseEffectResources(context,name,paths,count,found);}
static bool WeaponResources(MaterialReader& reader,const char* pair,const char**& paths,unsigned int& count,bool& found,bool fxEnabled){
 reader.combined.Clear();paths=0;count=0;found=false;char* weapon=(char*)reader.combined.Allocate(4097);if(!weapon)return false;unsigned int n=0;while(pair[n]&&pair[n]!='\n'){if(n>=4096)return false;weapon[n]=pair[n];n++;}if(pair[n]!='\n')return false;weapon[n]=0;const char* ammo=pair+n+1;
 if(!reader.sounds.HasRecord("Arsenal/Weapons",weapon))return true;found=true;paths=(const char**)reader.combined.Allocate(100000*sizeof(char*));if(!paths)return false;
 const char* names[]={weapon,ammo,"WeaponItemRespawn","WeaponItemPickup"};const char* categories[]={"Arsenal/Weapons","Arsenal/Ammo","Sound/Sound","Sound/Sound"};
 for(unsigned int step=0;step<4;step++){if(!names[step][0])continue;const char** files=0;unsigned int total=0;bool exists=false;if(!reader.sounds.Resolve(names[step],files,total,exists,true,categories[step],fxEnabled?DatabaseEffectResources:0,&reader)||total>100000-count)return false;
  for(unsigned int i=0;i<total;i++){unsigned int length=lstrlenA(files[i]);if(length>4096)return false;char* copy=(char*)reader.combined.Allocate(length+1);if(!copy)return false;lstrcpyA(copy,files[i]);paths[count++]=copy;}
 }HRECORD ammoRecord=reader.sounds.Record("Arsenal/Ammo",ammo);if(reader.sounds.ValueCount(ammoRecord,"SurfaceFXType")&&!reader.weaponSurfaces.Add(reader.sounds.String(ammoRecord,"SurfaceFXType")))return false;return true;
}
static bool AppendResource(MaterialReader& reader,const char** paths,unsigned int& count,const char* file){
 if(!file||!file[0])return true;unsigned int length=lstrlenA(file);if(length>4096||count>=100000)return false;char* copy=(char*)reader.combined.Allocate(length+1);if(!copy)return false;lstrcpyA(copy,file);paths[count++]=copy;return true;
}
static bool AppendEffect(MaterialReader& reader,const char** paths,unsigned int& count,const char* fx,bool fxEnabled){
 if(!fx||!fx[0]||!fxEnabled)return true;const char** files=0;unsigned int total=0;bool exists=false;if(!DatabaseEffectResources(&reader,fx,files,total,exists))return false;
 for(unsigned int i=0;i<total;i++)if(!AppendResource(reader,paths,count,files[i]))return false;return true;
}
// SDK Turret::GetPrefetchResourceList (Turret.cpp 993): base model/material,
// LoopFX/DamageFX, each DamageState ClientFX, then the weapon's Default record.
static bool TurretResources(MaterialReader& reader,const char* type,const char**& paths,unsigned int& count,bool& found,bool fxEnabled){
 reader.combined.Clear();paths=0;count=0;found=false;if(!reader.sounds.Initialize(reader.project))return false;
 HRECORD turret=reader.sounds.Record("Arsenal/Turrets",type);if(!turret)return true;found=true;paths=(const char**)reader.combined.Allocate(100000*sizeof(char*));if(!paths)return false;
 if(!AppendResource(reader,paths,count,reader.sounds.String(turret,"BaseModel"))||!AppendResource(reader,paths,count,reader.sounds.String(turret,"BaseMaterial")))return false;
 if(!AppendEffect(reader,paths,count,reader.sounds.String(turret,"LoopFX"),fxEnabled)||!AppendEffect(reader,paths,count,reader.sounds.String(turret,"DamageFX"),fxEnabled))return false;
 unsigned int states=reader.sounds.ValueCount(turret,"DamageState");if(states>4096)return false;
 for(unsigned int i=0;i<states;i++){char name[64];lstrcpyA(name,"DamageState.");lstrcpyA(Decimal(name+12,i),".ClientFX");if(!AppendEffect(reader,paths,count,reader.sounds.String(turret,name),fxEnabled))return false;}
 HRECORD weapon=reader.sounds.Link(turret,"Weapon");HRECORD standard=weapon?reader.sounds.Link(weapon,"Default"):0;
 if(standard){const char** files=0;unsigned int total=0;if(!reader.sounds.ResolveRecord(standard,files,total,true,fxEnabled?DatabaseEffectResources:0,&reader))return false;for(unsigned int i=0;i<total;i++)if(!AppendResource(reader,paths,count,files[i]))return false;}
 return true;
}
static bool PlayerResources(MaterialReader& reader,const char**& paths,unsigned int& count,bool& found,bool fxEnabled){
 reader.combined.Clear();count=0;found=true;HRECORD* records=0;unsigned int recordCount=0;if(!reader.sounds.PlayerRecords(reader.combined,records,recordCount))return false;
 paths=(const char**)reader.combined.Allocate(100000*sizeof(char*));if(!paths)return false;
 for(unsigned int r=0;r<recordCount;r++){const char** files=0;unsigned int n=0;if(!reader.sounds.ResolveRecord(records[r],files,n,true,fxEnabled?DatabaseEffectResources:0,&reader)||n>100000-count)return false;
  for(unsigned int i=0;i<n;i++){unsigned int length=lstrlenA(files[i]);if(length>4096)return false;char* copy=(char*)reader.combined.Allocate(length+1);if(!copy)return false;lstrcpyA(copy,files[i]);paths[count++]=copy;}
 }return true;
}
static bool DialogueResources(MaterialReader& reader,const char* text,const char**& paths,unsigned int& count,bool& found){
 reader.combined.Clear();count=0;found=true;paths=0;
 char value[MAX_PATH];unsigned int n=lstrlenA(text);if(n>=MAX_PATH)n=MAX_PATH-1;for(unsigned int i=0;i<n;i++)value[i]=text[i];value[n]=0;
 unsigned int firstSpace=0;while(value[firstSpace]&&value[firstSpace]!=' ')firstSpace++;if(!value[firstSpace])return true;
 bool radio=false;for(unsigned int i=0;i+5<=n;i++)if(value[i]=='r'&&value[i+1]=='a'&&value[i+2]=='d'&&value[i+3]=='i'&&value[i+4]=='o')radio=true;
 value[firstSpace]=0;if(!reader.voices.Initialize(reader.project))return false;
 const char* voice=reader.voices.Path(value);if(!reader.voices.Exists(value)||!voice||!voice[0])return true;
 unsigned int length=lstrlenA(voice);if(length>4096)return false;
 paths=(const char**)reader.combined.Allocate(2*sizeof(char*));char* copy=(char*)reader.combined.Allocate(length+1);if(!paths||!copy)return false;lstrcpyA(copy,voice);paths[count++]=copy;
 if(radio){unsigned int dot=0;while(voice[dot]&&voice[dot]!='.')dot++;char* variant=(char*)reader.combined.Allocate(length+2);if(!variant)return false;for(unsigned int i=0;i<dot;i++)variant[i]=voice[i];variant[dot]='R';lstrcpyA(variant+dot+1,voice+dot);paths[count++]=variant;}
 return true;
}
#include "AiPrefetchResources.h"
static bool PropResources(MaterialReader& reader,const char* name,const char**& paths,unsigned int& count,bool& found){
 reader.combined.Clear();paths=0;count=0;found=true;char value[256];unsigned int length=lstrlenA(name),n=length<256?length:255;for(unsigned int i=0;i<n;i++)value[i]=name[i];value[n]=0;if(!value[0])return true;
 HRECORD prop=reader.sounds.Record("Props",value);if(!prop)return true;unsigned int materials=reader.sounds.ValueCount(prop,"Material");if(materials>4096)return false;
 paths=(const char**)reader.combined.Allocate((materials+1)*sizeof(char*));if(!paths)return false;
 for(unsigned int i=0;i<=materials;i++){const char* asset=i?reader.sounds.String(prop,"Material",i-1):reader.sounds.String(prop,"ModelFile");if(!asset[0])continue;unsigned int size=lstrlenA(asset);if(size>4096)return false;char* copy=(char*)reader.combined.Allocate(size+1);if(!copy)return false;lstrcpyA(copy,asset);paths[count++]=copy;}
 return true;
}
static bool SourceSoundResources(void* context,const char* name,const char**& paths,unsigned int& count,bool& found,unsigned int mode){MaterialReader& reader=*(MaterialReader*)context;bool fxEnabled=requestClientFx;if(!reader.sounds.Initialize(reader.project))return false;if(mode==12)return PropResources(reader,name,paths,count,found);if(mode==15)return TurretResources(reader,name,paths,count,found,fxEnabled);if(mode==11){AiPrefetchResources ai;return ai.Build(reader,name,paths,count,found,fxEnabled);}if(mode==10)return DialogueResources(reader,name,paths,count,found);if(mode==9)return PlayerResources(reader,paths,count,found,fxEnabled);if(mode==5)return WeaponResources(reader,name,paths,count,found,fxEnabled);if(mode==13||mode==14){/* SDK GetPrefetchResourceList returns when GetRecordByName finds no record (or category). */if(!reader.sounds.Record(mode==13?"GameModes/CTF/FlagBase":"GameModes/CP/Types",name)){paths=0;count=0;found=false;return true;}}bool resolvedOk=reader.sounds.Resolve(name,paths,count,found,mode!=0,mode==6?"Collisions/CollisionProperty":mode==7?"Fx/ShatterType":mode==3?"Arsenal/Gear":mode==4?"Arsenal/RefillStations":mode==13?"GameModes/CTF/FlagBase":mode==14?"GameModes/CP/Types":mode==2?"Surfaces/Surfaces":"Sound/Sound",fxEnabled?DatabaseEffectResources:0,context);if(!resolvedOk&&reader.sounds.error)LogLine(reader.sounds.error);return resolvedOk;}
#include "TutorialPostResources.h"
#include "GamePostResources.h"
#include "CondemnedGatherer.h"
namespace StreamingWorkerCore {
// Returns the former worker exit code (0 = success); output owns a copy of the block.
inline unsigned int Run(const Request& request,PrefabScene::Arena& output,const unsigned char*& data,unsigned int& size,const char*& postProfile){
 data=0;size=0;postProfile=0;PrefabScene::Arena arena;
 if(!request.source||!request.input)return 1;input=request.input;bytes=request.bytes;cursor=0;streamFailure=0;requestClientFx=request.clientFx;if(bytes<12||bytes>16*1024*1024)return 3;
 const char* stem=S();unsigned int count=U();if(count>10000)return (12);const char** names=(const char**)arena.Allocate((count+1)*sizeof(char*));unsigned int* budgets=(unsigned int*)arena.Allocate((count+1)*4);if(!names||!budgets)return (13);for(unsigned int i=0;i<count;i++){names[i]=S();budgets[i]=U();}
 unsigned int seedCount=U();if(seedCount>100000)return (12);StreamingResources::Seed* seeds=(StreamingResources::Seed*)arena.Allocate((seedCount+1)*sizeof(StreamingResources::Seed));if(!seeds)return (13);for(unsigned int i=0;i<seedCount;i++){seeds[i].region=U();seeds[i].path=S();}
 if(streamFailure)return streamFailure;unsigned int textBytes=U();if(streamFailure)return streamFailure;if(textBytes>bytes-cursor||cursor+textBytes!=bytes)return (14);AssetList::List list;if(!list.Read(input+cursor,textBytes,names,count))return (20);
 StreamingResources::Sets sets;if(!sets.Build(stem,names,budgets,count,seeds,seedCount,list))return (21);char sourcePath[MAX_PATH];if(lstrlenA(request.source)>=MAX_PATH)return 23;lstrcpyA(sourcePath,request.source);const PrefabScene::Model* sharedModel=request.model;PrefabScene::Expansion* scene=0;if(!sharedModel){PrefabScene::Source source;if(!source.Load(sourcePath))return (24);void* assemblyStorage=VirtualAlloc(0,sizeof(PrefabScene::Expansion),MEM_COMMIT|MEM_RESERVE,4);if(!assemblyStorage)return (25);scene=new(assemblyStorage) PrefabScene::Expansion;if(!scene->Build(sourcePath,source.scene))return (26);sharedModel=&scene->Scene();}const PrefabScene::Model& model=*sharedModel;char autoMaterials[4]={};SourceMaterialSeeds::Builder sourceSeeds;MaterialReader materialReader;
 SourcePostProfile::Request postRequest;postRequest.automatic=request.automaticPost;postRequest.tutorial=request.tutorialPost;postRequest.multiplayer=request.multiplayerPost;postRequest.singleplayer=request.singleplayerPost;SourcePostProfile::Selection postSelection;if(!postSelection.Build(model,postRequest)){LogLine(postSelection.error);return (40);}
 (void)autoMaterials;bool gatherSourceMaterials=request.sourceMaterials;if(postSelection.mode!=SourcePostProfile::Disabled&&!gatherSourceMaterials){LogLine("Resource post-profile requires source resource gathering");return (40);}
 if(gatherSourceMaterials){
  char textureMode[4]={request.textureMode,0,0,0};bool textures=textureMode[0]=='1'||textureMode[0]=='2';
  if(textureMode[0]=='2'&&!materialReader.gate.Initialize())return (35);
  bool soundDatabase=request.soundDatabase;
  if(!materialReader.project.Locate(sourcePath)||!sourceSeeds.Build(model,ReadSourceMaterial,&materialReader,textures,textureMode[0]=='2'?SourceShaderReady:0,textureMode[0]=='2'?SourceTextureReady:0,soundDatabase?SourceSoundResources:0,request.clientFx?SourceEffectResources:0))return (33);
  if(request.condemnedGather){ // CONDEMNED FORK: resources reported by the project GameServer.dll through the gatherer interface
   char serverPath[MAX_PATH];if(!materialReader.project.Resolve("Project Binaries","GameServer",serverPath)){LogLine("Project has no GameServer binary");return (41);}
   const CondemnedGatherer::Gatherer::Entry* gatheredEntries=0;unsigned int gatheredCount=0;const char* gatherError=0;
   if(!CondemnedGatherer::Run(serverPath,materialReader.project.resources,model,arena,stem,names,count,gatheredEntries,gatheredCount,gatherError)){LogLine(gatherError);return (41);}
   const char** gatheredPaths=0;unsigned int* gatheredRegions=0;if(!CondemnedGatherer::Resolve(model,arena,names,count,gatheredEntries,gatheredCount,gatheredPaths,gatheredRegions)){LogLine("Could not map gathered resources to streaming regions");return (41);}
   if(!sourceSeeds.AppendRegional(gatheredPaths,gatheredRegions,gatheredCount,ReadSourceMaterial,&materialReader,textures,textureMode[0]=='2'?SourceShaderReady:0,textureMode[0]=='2'?SourceTextureReady:0))return (42);
  }
  if(postSelection.mode==SourcePostProfile::Tutorial){
   const char** postFiles=0;unsigned int postCount=0;TutorialPostResources post;if(!post.Build(materialReader,model,sourceSeeds.seeds,sourceSeeds.count,postFiles,postCount,sourceSeeds.objectSeedStart)||!sourceSeeds.AppendGlobal(postFiles,postCount,ReadSourceMaterial,&materialReader,textures,textureMode[0]=='2'?SourceShaderReady:0,textureMode[0]=='2'?SourceTextureReady:0))return (36);
  }
  if(postSelection.mode==SourcePostProfile::Multiplayer){
   const char** postFiles=0;unsigned int postCount=0;GamePostResources post;if(!post.Build(materialReader,model,sourceSeeds.seeds,sourceSeeds.count,postFiles,postCount,true,0,sourceSeeds.objectSeedStart)||!sourceSeeds.AppendGlobal(postFiles,postCount,ReadSourceMaterial,&materialReader,textures,textureMode[0]=='2'?SourceShaderReady:0,textureMode[0]=='2'?SourceTextureReady:0))return (37);
  }
  if(postSelection.mode==SourcePostProfile::Singleplayer){
   const char** postFiles=0;unsigned int postCount=0;GamePostResources post;if(!post.Build(materialReader,model,sourceSeeds.seeds,sourceSeeds.count,postFiles,postCount,false,stem,sourceSeeds.objectSeedStart)||!sourceSeeds.AppendGlobal(postFiles,postCount,ReadSourceMaterial,&materialReader,textures,textureMode[0]=='2'?SourceShaderReady:0,textureMode[0]=='2'?SourceTextureReady:0))return (38);
  }
  char seedTracePath[MAX_PATH];if(GetEnvironmentVariableA("FEAR_TRACE_SEED_OUTPUT",seedTracePath,MAX_PATH)){
   HANDLE trace=CreateFileA(seedTracePath,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);DWORD written;if(trace==INVALID_HANDLE_VALUE)return (39);
   bool traceOK=WriteFile(trace,&sourceSeeds.count,4,&written,0)&&written==4;
   for(unsigned int i=0;traceOK&&i<sourceSeeds.count;i++){const StreamingResources::Seed& seed=sourceSeeds.seeds[i];unsigned int length=lstrlenA(seed.path)+1;traceOK=WriteFile(trace,&seed.region,4,&written,0)&&written==4&&WriteFile(trace,seed.path,length,&written,0)&&written==length;}
   CloseHandle(trace);if(!traceOK)return (39);
  }
  if(!sets.Build(stem,names,budgets,count,sourceSeeds.seeds,sourceSeeds.objectSeedStart,list,sourceSeeds.seeds+sourceSeeds.objectSeedStart,sourceSeeds.count-sourceSeeds.objectSeedStart))return (34);
 }
StreamingSettings::Settings settings;if(!settings.Read(model)||settings.count!=count)return (28);StreamingConvex::Block selectionConvex;if(!selectionConvex.Build(model))return (29);int selected=settings.Select(selectionConvex);for(unsigned int i=0;i<count;i++)if(!FearBrushFlags::Equal(names[i],settings.names[i])||budgets[i]!=settings.budgets[i])return (30);StreamingAssembly::Block block;if(request.visibility){void* visibilityMemory=VirtualAlloc(0,sizeof(VisibilityCompiler::Builder),MEM_COMMIT|MEM_RESERVE,4);if(!visibilityMemory)return (25);VisibilityCompiler::Builder* visibility=new(visibilityMemory) VisibilityCompiler::Builder;if(!visibility->Build(model))return (31);if(!block.Build(model,sets,count,settings.globalBudget,settings.streamingBudget,selected,visibility->sectorCount,visibility->streamingSectors,&visibility->offset))return (32);}else if(!block.Build(model,sets,count,settings.globalBudget,settings.streamingBudget,selected))return (27);
 unsigned char* copy=(unsigned char*)output.Allocate(block.size?block.size:1);if(!copy)return 5;memcpy(copy,block.data,block.size);data=copy;size=block.size;
 postProfile=SourcePostProfile::Name(postSelection.mode);return 0;
}
}
