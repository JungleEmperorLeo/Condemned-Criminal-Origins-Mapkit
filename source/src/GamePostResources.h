#pragma once
#include "VoiceDatabaseResources.h"
#include "MissionResourceLookup.h"
#include "SourcePostProfile.h"
// SDK multiplayer/singleplayer post passes; mission lookup uses supplied world path.
class GamePostResources {
 PrefabScene::Arena memory;MaterialReader* reader;const char** files;unsigned int count;VoiceDatabaseResources voices;
 bool Add(const char* assetPath){if(!assetPath||!assetPath[0])return true;if(count>=100000)return false;unsigned int n=lstrlenA(assetPath);if(n>4096)return false;char* copy=(char*)memory.Allocate(n+1);if(!copy)return false;lstrcpyA(copy,assetPath);files[count++]=copy;return true;}
 bool Record(HRECORD record,bool recurse=true){if(!record)return true;const char** paths=0;unsigned int n=0;if(!reader->sounds.ResolveRecord(record,paths,n,recurse,DatabaseEffectResources,reader))return false;for(unsigned int i=0;i<n;i++)if(!Add(paths[i]))return false;return true;}
 static bool Listed(const char** list,unsigned int n,const char* wanted){while(*wanted=='\\'||*wanted=='/')wanted++;for(unsigned int i=0;i<n;i++){const char* item=list[i];while(*item=='\\'||*item=='/')item++;if(FearBrushFlags::Equal(item,wanted))return true;}return false;}
 bool Named(const char* category,const char* name,bool recurse=true){return Record(reader->sounds.Record(category,name),recurse);}
 bool Effect(const char* name){if(!name||!name[0])return true;const char** paths=0;unsigned int n=0;bool found=false;if(!DatabaseEffectResources(reader,name,paths,n,found))return false;for(unsigned int i=0;i<n;i++)if(!Add(paths[i]))return false;return true;}
 bool Weapon(HRECORD link,bool includeAmmo){
  HRECORD weapon=reader->sounds.Link(link,"MultiPlayer");if(!weapon)weapon=reader->sounds.Link(link,"Default");if(!Record(weapon))return false;
  unsigned int n=reader->sounds.ValueCount(weapon,"AmmoName");if(n>4096)return false;
  for(unsigned int i=0;i<n;i++){HRECORD ammoLink=reader->sounds.Link(weapon,"AmmoName",i);HRECORD ammo=reader->sounds.Link(ammoLink,"MultiPlayer");if(!ammo)ammo=reader->sounds.Link(ammoLink,"Player");if(includeAmmo&&!Record(ammo))return false;if(reader->sounds.ValueCount(ammo,"SurfaceFXType")&&!reader->weaponSurfaces.Add(reader->sounds.String(ammo,"SurfaceFXType")))return false;}
  return true;
 }
 bool Multiplayer(){
  LogLine("MP post: models");
  HCATEGORY models=reader->sounds.Category("Character/Models/Global");HRECORD globalModels=reader->sounds.RecordAt(models,0);if(!globalModels)return false;
  unsigned int n=reader->sounds.ValueCount(globalModels,"DeathmatchModels");if(n>4096)return false;
  for(unsigned int i=0;i<n;i++)if(!Record(reader->sounds.Link(globalModels,"DeathmatchModels",i)))return false;
  LogLine("MP post: loadouts");HRECORD arsenal=reader->sounds.Record("Arsenal/Global","Global");if(!arsenal)return false;
  n=reader->sounds.ValueCount(arsenal,"LoadoutOrder");if(n>4096)return false;
  for(unsigned int i=0;i<n;i++){HRECORD loadout=reader->sounds.Link(arsenal,"LoadoutOrder",i);unsigned int weapons=reader->sounds.ValueCount(loadout,"Weapons");if(weapons>4096)return false;for(unsigned int w=0;w<weapons;w++)if(!Weapon(reader->sounds.Link(loadout,"Weapons",w),true))return false;}
  if(!Weapon(reader->sounds.Link(reader->sounds.Record("Server/Player","Player"),"DefaultWeapon"),false))return false;
  if(!Named("SlowMo","Multiplayer")||!Effect(reader->sounds.String(arsenal,"SlowMoRechargerFXName")))return false;
  LogLine("MP post: voices");if(!voices.Initialize(reader->project))return false;
  HCATEGORY broadcast=reader->sounds.Category("Sound/Broadcast/Broadcasts");if(!broadcast)return false;n=reader->sounds.RecordCount(broadcast);if(n>4096)return false;
  for(unsigned int i=0;i<n;i++){HRECORD record=reader->sounds.RecordAt(broadcast,i);unsigned int lines=reader->sounds.ValueCount(record,"Line");if(lines>4096)return false;for(unsigned int l=0;l<lines;l++)if(!Add(voices.Path(reader->sounds.String(record,"Line",l))))return false;}
  LogLine("MP post: markers");HCATEGORY markers=reader->sounds.Category("Interface/NavMarkerTypes");if(!markers)return false;n=reader->sounds.RecordCount(markers);if(n>4096)return false;
  for(unsigned int i=0;i<n;i++)if(!Record(reader->sounds.RecordAt(markers,i)))return false;
  if(!Effect(reader->sounds.String(reader->sounds.Record("Client/Shared","Shared"),"MultiplayerDeathClientFX")))return false;
  n=reader->sounds.ValueCount(arsenal,"DroppedGearItems");if(n>4096)return false;for(unsigned int i=0;i<n;i++)if(!Record(reader->sounds.Link(arsenal,"DroppedGearItems",i)))return false;
  return Add("guns\\Snd\\Impacts\\MultiDing.wav")&&Add("Interface\\Snd\\chat.wav")&&Add("Interface\\Snd\\RadioOn.wav")&&Add("Interface\\Snd\\RadioOff.wav");
 }
 bool Mission(const char* worldStem){
  MissionResourceLookup lookup;if(!lookup.Find(reader->sounds,worldStem)){LogLine(lookup.error);return false;}if(!lookup.mission)return true;
  unsigned int n=reader->sounds.ValueCount(lookup.mission,"DefaultWeapons");if(n>4096)return false;
  for(unsigned int i=0;i<n;i++){HRECORD weapon=reader->sounds.Link(reader->sounds.Link(lookup.mission,"DefaultWeapons",i),"Default");if(!weapon)continue;if(!Record(weapon))return false;
   unsigned int ammoCount=reader->sounds.ValueCount(weapon,"AmmoName");if(ammoCount>4096)return false;
   for(unsigned int j=0;j<ammoCount;j++){HRECORD ammoLink=reader->sounds.Link(weapon,"AmmoName",j),ammo=reader->sounds.Link(ammoLink,"MultiPlayer");if(!ammo)ammo=reader->sounds.Link(ammoLink,"Player");if(reader->sounds.ValueCount(ammo,"SurfaceFXType")&&!reader->weaponSurfaces.Add(reader->sounds.String(ammo,"SurfaceFXType")))return false;}
  }return true;
 }
public:
 bool Build(MaterialReader& source,const PrefabScene::Model& model,const StreamingResources::Seed* sourceSeeds,unsigned int sourceCount,const char**& paths,unsigned int& total,bool multiplayer,const char* worldStem=0,unsigned int worldSeedCount=0){
  memory.Clear();reader=&source;count=0;paths=0;total=0;files=(const char**)memory.Allocate(100000*sizeof(char*));if(!files||!reader->sounds.Initialize(reader->project))return false;

  SourcePostProfile::Request selectionRequest;selectionRequest.multiplayer=multiplayer;selectionRequest.singleplayer=!multiplayer;SourcePostProfile::Selection selection;if(!selection.Build(model,selectionRequest)){LogLine(selection.error);return false;}
  if(multiplayer){if(!Multiplayer())return false;}else{
  if(!Mission(worldStem))return false;
  LogLine("Tutorial post: SlowMo");
  if(!Named("SlowMo","Default"))return false;
  if(!Effect(reader->sounds.String(reader->sounds.Record("Client/Shared","Shared"),"MultiplayerDeathClientFX")))return false;
  LogLine("Tutorial post: dialogue");
  HCATEGORY dialogue=reader->sounds.Category("Interface/Dialogue");if(!dialogue)return false;unsigned int dialogues=reader->sounds.RecordCount(dialogue);if(dialogues>4096)return false;
  for(unsigned int i=0;i<dialogues;i++)if(!Add(reader->sounds.String(reader->sounds.RecordAt(dialogue,i),"IconTexture")))return false;
  LogLine("Tutorial post: AI sounds");
  const char* ai[]={"AI_desk_slide","AI_divethru","AI_jump","AI_land","AI_slidecover","Signal"};
  for(unsigned int i=0;i<6;i++)if(!Named("Sound/Sound",ai[i],false))return false;
  }
  if(!Add("Models\\Default.Model00p"))return false;
  const char* sounds[]={"punchimpact","feetland","feetjump","Jump3D","JumpLocal","EarRingingBuzz","melee_01","punch_01","kick_01","slidekick","Death","PlayerRespawn","bulletflyby","breath","HeadShot","Heartbeat","Kill","Team Kill","flickerloop","Ladder_slide"};
  for(unsigned int i=0;i<20;i++)if(!Named("Sound/Sound",sounds[i],false))return false;
  LogLine("Tutorial post: flashlight");
  if(!Named("Client/Flashlight","Player_Narrow")||!Add("guns\\Snd\\Proximity\\arm.wav")||!Add("guns\\Snd\\Proximity\\armed.wav")||!Add("guns\\Snd\\Proximity\\activate.wav")||!Add("Snd\\Events\\beep01.wav"))return false;
  // Collect surface IDs from source material metadata and brush overrides.
  // The final original polygon filtering/order is not claimed here.
  LogLine("Tutorial post: surfaces");
  bool* used=(bool*)memory.Allocate(256);if(!used)return false;for(unsigned int i=0;i<256;i++)used[i]=false;
  HCATEGORY surfaces=reader->sounds.Category("Surfaces/Surfaces");if(!surfaces)return false;unsigned int surfaceCount=reader->sounds.RecordCount(surfaces);if(surfaceCount>4096)return false;
  unsigned char* stalkBrushes=SourceMaterialSeeds::Builder::StalkBrushes(model,memory);if(!stalkBrushes)return false;
   unsigned int* regional=SourceMaterialSeeds::Builder::RegionalBrushes(model,memory);if(!regional)return false;StreamingConvex::Block streamingRegions;if(!streamingRegions.Build(model))return false;SourceMaterialSeeds::Builder::RegionSectors regionSectors;if(!regionSectors.Build(model,streamingRegions,memory))return false;
   unsigned int polygonTotal=0;for(const PrefabScene::Node* node=model.first;node;node=node->next)if(node->kind==1)polygonTotal+=node->geometry.polygons;
   const char** globalMaterials=(const char**)memory.Allocate((polygonTotal+1)*sizeof(char*));unsigned int globalMaterialCount=0;if(!globalMaterials)return false;
   for(const PrefabScene::Node* node=model.first;node;node=node->next)if(node->kind==1&&!stalkBrushes[node->index]&&SourceMaterialSeeds::MaterialCandidate(node->surfaceFlags)){
    // Surface types register globally only for polygons outside every region (subtree brush or wholly inside a region sector).
    if(regional[node->index])continue;
    unsigned char* global=(unsigned char*)memory.Allocate(node->geometry.polygons+1);if(!global)return false;bool anyGlobal=false;
    for(unsigned int i=0;i<node->geometry.polygons;i++){unsigned int region=0;if(!regionSectors.PolygonRegion(node->geometry,node->geometry.faces[i],region))return false;global[i]=region==0;if(!region)anyGlobal=true;}
    if(!anyGlobal)continue;
    for(unsigned int i=0;i<node->geometry.polygons;i++)if(global[i]&&node->geometry.faces[i].material[0]&&!Listed(globalMaterials,globalMaterialCount,node->geometry.faces[i].material))globalMaterials[globalMaterialCount++]=node->geometry.faces[i].material;
   bool overrideFound=false;for(const PrefabScene::Property* p=node->properties;p;p=p->next)if(FearBrushFlags::Equal(p->name,"SurfaceFlagsOverride")&&p->data[0]){HRECORD surface=reader->sounds.Record("Surfaces/Surfaces",p->data);if(surface){int id=reader->sounds.Integer(surface,"Id",-1);if(id>=0&&id<256){used[id]=true;overrideFound=true;}}}
   if(!overrideFound)for(unsigned int i=0;i<node->geometry.polygons;i++){if(!global[i]||!node->geometry.faces[i].material[0])continue;
    // An absent Mat00 has surface flags 0, as in the physical BSP metadata.
    const unsigned char* sourceBytes=0;unsigned int size=0;if(!ReadSourceMaterial(reader,node->geometry.faces[i].material,sourceBytes,size)){used[0]=true;continue;}MaterialResources::Material material;if(!material.Read(sourceBytes,size)){LogLine(material.error);return false;}used[material.surfaceFlags&255]=true;
   }
  }
  unsigned int beforeSurfaces=count;
  for(unsigned int i=0;i<sourceCount+beforeSurfaces;i++){
   const char* materialPath=i<sourceCount?sourceSeeds[i].path:files[i-sourceCount];unsigned int length=lstrlenA(materialPath);if(length<6||!FearBrushFlags::Equal(materialPath+length-6,".Mat00"))continue;
   // A world-geometry material registers its surface only through global polygons.
   if(i<worldSeedCount&&!Listed(globalMaterials,globalMaterialCount,materialPath))continue;
   const unsigned char* materialBytes=0;unsigned int materialSize=0;if(!ReadSourceMaterial(reader,materialPath,materialBytes,materialSize))continue;MaterialResources::Material material;if(!material.Read(materialBytes,materialSize))return false;used[material.surfaceFlags&255]=true;
  }
  // Original RVA 0x33C40 appends unique surface IDs as material resources arrive.
  unsigned int* ordered=(unsigned int*)memory.Allocate(256*sizeof(unsigned int));unsigned int orderedCount=0;if(!ordered)return false;
  for(unsigned int i=0;i<sourceCount+beforeSurfaces;i++){
   const char* orderedMaterialPath=i<sourceCount?sourceSeeds[i].path:files[i-sourceCount];unsigned int length=lstrlenA(orderedMaterialPath);if(length<6||!FearBrushFlags::Equal(orderedMaterialPath+length-6,".Mat00"))continue;
   if(i<worldSeedCount&&!Listed(globalMaterials,globalMaterialCount,orderedMaterialPath))continue;
   const unsigned char* orderedMaterialBytes=0;unsigned int size=0;if(!ReadSourceMaterial(reader,orderedMaterialPath,orderedMaterialBytes,size))continue;MaterialResources::Material material;if(!material.Read(orderedMaterialBytes,size))return false;
   unsigned int id=material.surfaceFlags&255;bool seen=false;for(unsigned int j=0;j<orderedCount;j++)if(ordered[j]==id)seen=true;if(!seen)ordered[orderedCount++]=id;
  }
  for(unsigned int i=0;i<surfaceCount;i++){HRECORD surface=reader->sounds.RecordAt(surfaces,i);int id=reader->sounds.Integer(surface,"Id",-1);if(id>=0&&id<256&&used[id]){bool seen=false;for(unsigned int j=0;j<orderedCount;j++)if(ordered[j]==(unsigned int)id)seen=true;if(!seen)ordered[orderedCount++]=id;}}
  for(unsigned int i=0;i<orderedCount;i++){HRECORD surface=0;for(unsigned int j=0;j<surfaceCount;j++){HRECORD r=reader->sounds.RecordAt(surfaces,j);if(reader->sounds.Integer(r,"Id",-1)==(int)ordered[i]){surface=r;break;}}if(!surface)continue;if(!Record(surface)){LogLine(reader->sounds.error);return false;}HRECORD weaponFX=reader->sounds.Link(surface,"WeaponFX");for(WeaponSurfaceResources::Entry* type=reader->weaponSurfaces.first;type;type=type->next)if(!Record(reader->sounds.Link(weaponFX,type->name)))return false;}

  LogLine("Tutorial post: complete");
  paths=files;total=count;return true;
 }
};
