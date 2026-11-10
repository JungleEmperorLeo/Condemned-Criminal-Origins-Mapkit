#pragma once
// SDK singleplayer post pass subset for non-mission tutorial controls.
// Multiplayer, mission weapons and weapon SurfaceFX state are separate stages.
class TutorialPostResources {
 PrefabScene::Arena memory;MaterialReader* reader;const char** files;unsigned int count;
 bool Add(const char* assetPath){if(!assetPath||!assetPath[0])return true;if(count>=100000)return false;unsigned int n=lstrlenA(assetPath);if(n>4096)return false;char* copy=(char*)memory.Allocate(n+1);if(!copy)return false;lstrcpyA(copy,assetPath);files[count++]=copy;return true;}
 bool Record(HRECORD record,bool recurse=true){if(!record)return true;const char** paths=0;unsigned int n=0;if(!reader->sounds.ResolveRecord(record,paths,n,recurse,DatabaseEffectResources,reader))return false;for(unsigned int i=0;i<n;i++)if(!Add(paths[i]))return false;return true;}
 static bool Listed(const char** list,unsigned int n,const char* wanted){while(*wanted=='\\'||*wanted=='/')wanted++;for(unsigned int i=0;i<n;i++){const char* item=list[i];while(*item=='\\'||*item=='/')item++;if(FearBrushFlags::Equal(item,wanted))return true;}return false;}
 bool Named(const char* category,const char* name,bool recurse=true){return Record(reader->sounds.Record(category,name),recurse);}
 bool Effect(const char* name){if(!name||!name[0])return true;const char** paths=0;unsigned int n=0;bool found=false;if(!DatabaseEffectResources(reader,name,paths,n,found))return false;for(unsigned int i=0;i<n;i++)if(!Add(paths[i]))return false;return true;}
public:
 bool Build(MaterialReader& source,const PrefabScene::Model& model,const StreamingResources::Seed* sourceSeeds,unsigned int sourceCount,const char**& paths,unsigned int& total,unsigned int worldSeedCount=0){
  memory.Clear();reader=&source;count=0;paths=0;total=0;files=(const char**)memory.Allocate(100000*sizeof(char*));if(!files||!reader->sounds.Initialize(reader->project))return false;
  for(const PrefabScene::Node* node=model.first;node;node=node->next)if(node->kind==2&&FearBrushFlags::Equal(node->className,"WeaponItem"))return false;
  bool worldProperties=false;
  for(const PrefabScene::Node* node=model.first;node;node=node->next)if(node->kind==2&&FearBrushFlags::Equal(node->className,"WorldProperties")){
   worldProperties=true;for(const PrefabScene::Property* p=node->properties;p;p=p->next)if(FearBrushFlags::Equal(p->name,"WorldType")&&FearBrushFlags::Equal(p->data,"MultiPlayer"))return false;
  }
  if(!worldProperties)return false;
  LogLine("Tutorial post: SlowMo");
  if(!Named("SlowMo","Default"))return false;
  if(!Effect(reader->sounds.String(reader->sounds.Record("Client/Shared","Shared"),"MultiplayerDeathClientFX")))return false;
  LogLine("Tutorial post: dialogue");
  HCATEGORY dialogue=reader->sounds.Category("Interface/Dialogue");if(!dialogue)return false;unsigned int dialogues=reader->sounds.RecordCount(dialogue);if(dialogues>4096)return false;
  for(unsigned int i=0;i<dialogues;i++)if(!Add(reader->sounds.String(reader->sounds.RecordAt(dialogue,i),"IconTexture")))return false;
  LogLine("Tutorial post: AI sounds");
  const char* ai[]={"AI_desk_slide","AI_divethru","AI_jump","AI_land","AI_slidecover","Signal"};
  for(unsigned int i=0;i<6;i++)if(!Named("Sound/Sound",ai[i],false))return false;
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
  for(unsigned int i=0;i<surfaceCount;i++){HRECORD surface=reader->sounds.RecordAt(surfaces,i);int id=reader->sounds.Integer(surface,"Id",-1);if(id>=0&&id<256&&used[id]&&!Record(surface)){LogLine(reader->sounds.error);return false;}}
  LogLine("Tutorial post: complete");
  paths=files;total=count;return true;
 }
};
