#pragma once
#include "MaterialResources.h"
#include "TextureAnimationResources.h"
#include "StreamingConvex.h"
#include "ResourceAreaIntersection.h"
#include "StalkNodes.h"
namespace SourceMaterialSeeds {
typedef bool (*ReadMaterial)(void*,const char*,const unsigned char*&,unsigned int&);
typedef bool (*ShaderReady)(void*,const char*);
typedef bool (*TextureReady)(void*,const char*);
// Resolver results live until its next call; false signals unavailable/unsupported data.
typedef bool (*SoundResources)(void*,const char*,const char**&,unsigned int&,bool&,unsigned int);
inline unsigned int SoundProperties(const char* className,const char** names){
 if(FearBrushFlags::Equal(className,"PolyGrid")){names[0]="Material";return 1;}
 if(FearBrushFlags::Equal(className,"CAI")){names[0]="ModelTemplate";return 1;}
 if(FearBrushFlags::Equal(className,"Prop")){names[0]="Model";return 1;}
 if(FearBrushFlags::Equal(className,"Dialogue")){static const char* dialogue[]={"Dialogue1","Dialogue2","Dialogue3","Dialogue4","Dialogue5","Dialogue6","Dialogue7","Dialogue8","Dialogue9","Dialogue10","Dialogue11","Dialogue12","Dialogue13","Dialogue14","Dialogue15","Dialogue16","Dialogue17","Dialogue18","Dialogue19"};for(unsigned int i=0;i<19;i++)names[i]=dialogue[i];return 19;}
 if(FearBrushFlags::Equal(className,"WorldProperties")){names[0]="Name";return 1;}
 if(FearBrushFlags::Equal(className,"WorldModel")){names[0]="CollisionProperty";names[1]="ShatterType";names[2]="DestroyedFX";return 3;}
 if(FearBrushFlags::Equal(className,"WeaponItem")){names[0]="WeaponType";return 1;}
 if(FearBrushFlags::Equal(className,"GearItem")){names[0]="GearType";return 1;}
 if(FearBrushFlags::Equal(className,"RefillStation")){names[0]="RefillStation";return 1;}
 // SDK CTFFlagBase/ControlPoint::GetPrefetchResourceList: GetRecordResources of
 // GameModes/CTF/FlagBase (FlagBase) and GameModes/CP/Types (Type) records.
 if(FearBrushFlags::Equal(className,"CTFFlagBase")){names[0]="FlagBase";return 1;}
 if(FearBrushFlags::Equal(className,"ControlPoint")){names[0]="Type";return 1;}
 // SDK Turret::GetPrefetchResourceList (TurretType record of Arsenal/Turrets).
 if(FearBrushFlags::Equal(className,"Turret")){names[0]="TurretType";return 1;}
 if(FearBrushFlags::Equal(className,"Ladder")){names[0]="SurfaceFlagsOverride";return 1;}
 if(FearBrushFlags::Equal(className,"SpecialFX")){names[0]="FxName";return 1;}
 if(FearBrushFlags::Equal(className,"SoundButeFX")){names[0]="SoundBute";return 1;}
 if(FearBrushFlags::Equal(className,"SoundFX")){names[0]="Sound";return 1;}
 if(FearBrushFlags::Equal(className,"KeyFramer")){names[0]="ActiveSound";return 1;}
 bool door=FearBrushFlags::Equal(className,"Door")||FearBrushFlags::Equal(className,"SlidingDoor")||FearBrushFlags::Equal(className,"RotatingDoor");
 bool active=FearBrushFlags::Equal(className,"ActiveWorldModel")||FearBrushFlags::Equal(className,"SlidingWorldModel")||FearBrushFlags::Equal(className,"SpinningWorldModel")||FearBrushFlags::Equal(className,"Switch")||FearBrushFlags::Equal(className,"SlidingSwitch")||FearBrushFlags::Equal(className,"RotatingSwitch");
 if(!door&&!active)return 0;
 names[0]=door?"OpeningSound":"PowerOnSound";names[1]=door?"OpenSound":"OnSound";names[2]=door?"ClosingSound":"PowerOffSound";names[3]=door?"ClosedSound":"OffSound";names[4]="LockedSound";return 5;
}
inline bool MaterialCandidate(unsigned int flags){return (flags&0x20)!=0&&((flags&1)==0||(flags&0x10)!=0);}
// Optional diagnostic requests retain object context without affecting seed ordering.
class RequestTrace {
 HANDLE file;
 void Field(const char* value){if(file==INVALID_HANDLE_VALUE)return;DWORD done;WriteFile(file,value,lstrlenA(value)+1,&done,0);}
public:
 RequestTrace():file(INVALID_HANDLE_VALUE){char target[MAX_PATH];if(GetEnvironmentVariableA("FEAR_TRACE_REQUEST_OUTPUT",target,MAX_PATH))file=CreateFileA(target,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);}
 ~RequestTrace(){if(file!=INVALID_HANDLE_VALUE)CloseHandle(file);}
 void Write(const char* object,const char* className,const char* property,const char* resource){Field(object);Field(className);Field(property);Field(resource);}
};
class Builder {
 struct Item {const char* path;Item* next;unsigned int region;};
 PrefabScene::Arena memory;Item* first;Item* last;unsigned int total;
 bool Equal(const char* a,const char* b){while(*a&&*b){char x=*a++,y=*b++;if(x>='A'&&x<='Z')x+='a'-'A';if(y>='A'&&y<='Z')y+='a'-'A';if(x!=y)return false;}return *a==*b;}
 bool Add(const char* path,unsigned int region=0){
  if(!path)return true;while(*path=='\\'||*path=='/')path++;if(!path[0])return true;unsigned int n=lstrlenA(path);if(n>4096)return false;
  for(Item* i=first;i;i=i->next)if(i->region==region&&Equal(i->path,path))return true;if(total>=100000)return false;
  Item* item=(Item*)memory.Allocate(sizeof(Item));char* copy=(char*)memory.Allocate(n+1);if(!item||!copy)return false;lstrcpyA(copy,path);item->path=copy;item->next=0;item->region=region;if(last)last->next=item;else first=item;last=item;total++;return true;
 }
 bool AddAsset(const char* path,unsigned int region,ReadMaterial read,void* context,bool expandTextures,ShaderReady shaderReady,TextureReady textureReady){
  if(!Add(path,region))return false;if(!expandTextures||!path||!path[0])return true;
  unsigned int length=lstrlenA(path);
  if(length>=6&&Equal(path+length-6,".Mat00")){
   const unsigned char* bytes=0;unsigned int size=0;if(!read)return false;if(!read(context,path,bytes,size)){suppressedMaterials++;return true;}
   MaterialResources::Material parsed;if(!parsed.Read(bytes,size)){LogLine(path);LogLine(parsed.error);return false;}
   if(shaderReady&&!shaderReady(context,parsed.shader)){suppressedMaterials++;return true;}
   for(unsigned int i=0;i<parsed.textureCount;i++)if(!textureReady||textureReady(context,parsed.textureNames[i]))if(!AddAsset(parsed.textures[i],region,read,context,expandTextures,shaderReady,textureReady))return false;
  }else if(length>=8&&Equal(path+length-8,".TxAnm00")){
   const unsigned char* bytes=0;unsigned int size=0;if(!read)return false;if(!read(context,path,bytes,size)){suppressedMaterials++;return true;}
   TextureAnimationResources::Animation animation;if(!animation.Read(bytes,size))return false;
   for(unsigned int i=0;i<animation.count;i++)if(!Add(animation.frames[i],region))return false;
  }return true;
 }
 bool Fail(const char* message){seeds=0;count=0;error=message;LogLine(message);return false;}
public:
 StreamingResources::Seed* seeds;unsigned int count,materialCount,suppressedMaterials,objectSeedStart;const char* error;
 Builder():first(0),last(0),total(0),seeds(0),count(0),materialCount(0),suppressedMaterials(0),objectSeedStart(0),error(0){}
 // CONDEMNED FORK: like AppendGlobal, with a streaming region index per path (0 = global, r+1 = region r)
 bool AppendRegional(const char** paths,const unsigned int* regions,unsigned int n,ReadMaterial read,void* context,bool textures,ShaderReady shaderReady,TextureReady textureReady){
  if(n>100000)return Fail("Invalid regional resources");for(unsigned int i=0;i<n;i++)if(!AddAsset(paths[i],regions[i],read,context,textures,shaderReady,textureReady))return Fail("Invalid regional resource");
  count=0;seeds=(StreamingResources::Seed*)memory.Allocate((total+1)*sizeof(StreamingResources::Seed));if(!seeds)return Fail("Allocation failed");for(Item* i=first;i;i=i->next){seeds[count].region=i->region;seeds[count++].path=i->path;}return true;
 }
 bool AppendGlobal(const char** paths,unsigned int n,ReadMaterial read,void* context,bool textures,ShaderReady shaderReady,TextureReady textureReady){
  if(n>100000)return Fail("Invalid post resources");for(unsigned int i=0;i<n;i++)if(!AddAsset(paths[i],0,read,context,textures,shaderReady,textureReady))return Fail("Invalid post resource");
  count=0;seeds=(StreamingResources::Seed*)memory.Allocate((total+1)*sizeof(StreamingResources::Seed));if(!seeds)return Fail("Allocation failed");for(Item* i=first;i;i=i->next){seeds[count].region=i->region;seeds[count++].path=i->path;}return true;
 }
 // Brushes deleted by ConvertStalkNodes before geometry import (StalkNodes.h);
 // they contribute neither materials nor region surface types.
 static unsigned char* StalkBrushes(const PrefabScene::Model& model,PrefabScene::Arena& memory){
  unsigned char* marks=(unsigned char*)memory.Allocate(model.count+1);if(!marks)return 0;memset(marks,0,model.count+1);
  for(const PrefabScene::Node* n=model.first;n;n=n->next)if(const PrefabScene::Node* c=StalkNodes::ConvertedChild(*n))if(c->index<model.count)marks[c->index]=1;return marks;
 }
 // Region membership of brushes (original 0x1001F600 -> 0x1003A300): the
 // StreamingRegion's subtree is walked, not entered at StreamingLink,
 // StreamingRegion, AINavMesh or AINodeStalk objects; a brush whose Type is
 // Normal, RenderOnly or NonSolid adds its polygon materials to that region
 // (AddResource 0x10039360, which also registers their surface types there).
 // Result per node index: 0 = not regional, r+1 = r-th StreamingRegion in
 // source order (the StreamingConvex/region table numbering).
 static unsigned int* RegionalBrushes(const PrefabScene::Model& model,PrefabScene::Arena& memory){
  unsigned int* region=(unsigned int*)memory.Allocate((model.count+1)*sizeof(unsigned int));const PrefabScene::Node** nodes=(const PrefabScene::Node**)memory.Allocate((model.count+1)*sizeof(void*));if(!region||!nodes)return 0;
  for(unsigned int i=0;i<=model.count;i++){region[i]=0;nodes[i]=0;}
  for(const PrefabScene::Node* n=model.first;n;n=n->next){if(n->index>=model.count)return 0;nodes[n->index]=n;}
  // Region numbers follow the original's region table: an ignored or removed
  // StreamingRegion (StreamingStructure.h) has no number and keeps no resources.
  unsigned int* table=(unsigned int*)memory.Allocate((model.count+1)*sizeof(unsigned int));if(!table)return 0;for(unsigned int i=0;i<=model.count;i++)table[i]=0;
  if(model.count){WorldText::Vec3 zero={0,0,0};StreamingStructure::Builder structure;if(!structure.Build(model,zero))return 0;
   if(!structure.fatal)for(unsigned int r=0;r<structure.regionCount;r++){unsigned int k=structure.TableIndex(r);if(k!=StreamingStructure::None)table[structure.regions[r].node->index]=k+1;}}
  for(const PrefabScene::Node* n=model.first;n;n=n->next){
   if(n->kind==2&&FearBrushFlags::Equal(n->className,"StreamingRegion"))continue;
   if(n->kind!=1||!(FearBrushFlags::Equal(n->type,"Normal")||FearBrushFlags::Equal(n->type,"RenderOnly")||FearBrushFlags::Equal(n->type,"NonSolid")))continue;
   const PrefabScene::Node* ancestor=n;unsigned int depth=0;
   while(ancestor->parent!=0xffffffffu){if(ancestor->parent>=model.count||!nodes[ancestor->parent]||depth++>128)return 0;ancestor=nodes[ancestor->parent];if(ancestor->kind!=2)continue;
    if(FearBrushFlags::Equal(ancestor->className,"StreamingRegion")){region[n->index]=table[ancestor->index];break;}
    if(FearBrushFlags::Equal(ancestor->className,"StreamingLink")||FearBrushFlags::Equal(ancestor->className,"AINavMesh")||FearBrushFlags::Equal(ancestor->className,"AINodeStalk"))break;
   }
  }
  // Region object marks were only counters; only brushes carry membership.
  for(const PrefabScene::Node* n=model.first;n;n=n->next)if(n->kind!=1)region[n->index]=0;
  return region;
 }
 // World geometry streams with the visibility sector that holds it: a Sector
 // brush lying inside a region volume belongs to that region. A polygon wholly
 // inside such a sector (0.5 render ownership epsilon per plane) is regional,
 // any other polygon is global. ThreeRegions: boxes inside the room sectors are
 // regional; ThreeRegionsExpanded: floor/wall/ceiling brushes bounding the
 // sectors stay global.
 struct RegionSectors {
  struct Sector {unsigned int region;ExtrusionVolume::Plane* planes;unsigned int count;};
  Sector* sectors;unsigned int count;
  RegionSectors():sectors(0),count(0){}
  static bool Inside(const ExtrusionVolume::Plane* planes,unsigned int planeCount,const WorldText::Vec3& p,double ox,double oy,double oz){
   for(unsigned int f=0;f<planeCount;f++){const ExtrusionVolume::Plane& q=planes[f];if((double)q.x*((double)p.x-ox)+(double)q.y*((double)p.y-oy)+(double)q.z*((double)p.z-oz)-q.d< -0.5)return false;}return true;
  }
  bool Build(const PrefabScene::Model& model,const StreamingConvex::Block& regions,PrefabScene::Arena& memory){
   sectors=0;count=0;if(!regions.regionCount)return true;
   sectors=(Sector*)memory.Allocate((model.brushes+1)*sizeof(Sector));if(!sectors)return false;
   for(const PrefabScene::Node* n=model.first;n;n=n->next){if(n->kind!=1||!FearBrushFlags::Equal(n->type,"Sector"))continue;const WorldText::Geometry& g=n->geometry;if(g.polygons<4||g.polygons>4096)return false;
    Sector& s=sectors[count];s.region=0;s.count=g.polygons;s.planes=(ExtrusionVolume::Plane*)memory.Allocate(g.polygons*sizeof(ExtrusionVolume::Plane));if(!s.planes)return false;
    for(unsigned int i=0;i<g.polygons;i++){const WorldText::Polygon& face=g.faces[i];if(face.count<3||face.count>128)return false;WorldText::Vec3 points[128];for(unsigned int j=0;j<face.count;j++){if(face.indices[j]>=g.points)return false;points[j]=g.vertices[face.indices[j]];}if(!ExtrusionVolume::ComputeInternalPlane(points,face.count,s.planes[i]))return false;}
    for(unsigned int v=0;v<regions.volumeCount&&!s.region;v++){const StreamingConvex::Volume& source=regions.volumes[v];if(source.owner>=regions.regionCount||source.linked>=0)continue;
     ExtrusionVolume::Plane* planes=(ExtrusionVolume::Plane*)memory.Allocate(source.count*sizeof(ExtrusionVolume::Plane));if(!planes)return false;for(unsigned int f=0;f<source.count;f++)planes[f]=source.faces[f].plane;
     bool within=true;for(unsigned int j=0;j<g.points&&within;j++)within=Inside(planes,source.count,g.vertices[j],regions.offset.x,regions.offset.y,regions.offset.z);if(within)s.region=source.owner+1;}
    if(s.region)count++;
   }
   return true;
  }
  bool PolygonRegion(const WorldText::Geometry& g,const WorldText::Polygon& face,unsigned int& region)const{
   region=0;for(unsigned int s=0;s<count&&!region;s++){bool within=face.count!=0;for(unsigned int i=0;i<face.count&&within;i++){if(face.indices[i]>=g.points)return false;within=Inside(sectors[s].planes,sectors[s].count,g.vertices[face.indices[i]],0,0,0);}if(within)region=sectors[s].region;}
   return true;
  }
 };
 // Collects global material candidates from source polygons. Texture expansion
 // is opt-in: shader load/compile success must be established by the caller.
 bool Build(const PrefabScene::Model& model,ReadMaterial read,void* context,bool expandTextures,ShaderReady shaderReady=0,TextureReady textureReady=0,SoundResources soundResources=0,SoundResources effectResources=0){
  RequestTrace requests;memory.Clear();first=last=0;total=count=materialCount=suppressedMaterials=objectSeedStart=0;seeds=0;error=0;
  // AINavMesh consumes its child polygons during navigation preprocessing;
  // those editor materials never become render/streaming material seeds.
  unsigned char* navigation=(unsigned char*)memory.Allocate(model.count+1);if(!navigation)return Fail("Allocation failed");
  for(const PrefabScene::Node* n=model.first;n;n=n->next){
   if(n->index>=model.count||(n->parent!=0xffffffffu&&n->parent>=n->index))return Fail("Invalid material ancestry");
   navigation[n->index]=(n->kind==2&&FearBrushFlags::Equal(n->className,"AINavMesh"))||(n->parent!=0xffffffffu&&navigation[n->parent]);
  }
  // ConvertStalkNodes deletes a converted node's child brush before geometry import.
  for(const PrefabScene::Node* n=model.first;n;n=n->next)if(const PrefabScene::Node* c=StalkNodes::ConvertedChild(*n))if(c->index<model.count)navigation[c->index]=1;
  // Brush materials are gathered over the compiler's post-ShadowOverride tree.
  WorldModels::CompileOrder compileOrder;if(!compileOrder.Build(model))return Fail("Material traversal order failed");
  unsigned int* regional=RegionalBrushes(model,memory);if(!regional)return Fail("Invalid streaming region ancestry");
  StreamingConvex::Block materialRegions;if(!materialRegions.Build(model))return Fail(materialRegions.error);RegionSectors regionSectors;if(!regionSectors.Build(model,materialRegions,memory))return Fail("Invalid region sector");
  for(unsigned int k=0;k<compileOrder.count;k++){const PrefabScene::Node* n=compileOrder.nodes[compileOrder.order[k]];
   // A StreamingRegion subtree brush, or a polygon wholly inside a region sector,
   // adds its material to that region; other polygons add it globally.
   if(n->kind==1&&!navigation[n->index]&&MaterialCandidate(n->surfaceFlags))for(unsigned int i=0;i<n->geometry.polygons;i++){unsigned int region=regional[n->index];if(!region&&!regionSectors.PolygonRegion(n->geometry,n->geometry.faces[i],region))return Fail("Invalid material polygon");if(!Add(n->geometry.faces[i].material,region))return Fail("Invalid material candidate");}
  }
  materialCount=total;Item* material=first;first=last=0;total=0;
  for(unsigned int i=0;i<materialCount;i++,material=material->next){
   const unsigned int materialRegion=material->region;if(!Add(material->path,materialRegion))return Fail("Invalid material candidate");
   if(!expandTextures)continue;const unsigned char* bytes=0;unsigned int size=0;
   if(!read)return Fail("Could not load source material");
   // As in AddAsset, an unreadable/absent Mat00 keeps only its own path (SDK
   // SpillKill: the original lists the missing Wood_SL001.Mat00 itself).
   if(!read(context,material->path,bytes,size)){suppressedMaterials++;continue;}
   MaterialResources::Material parsed;if(!parsed.Read(bytes,size)){LogLine(material->path);return Fail(parsed.error);}
   if(shaderReady&&!shaderReady(context,parsed.shader)){suppressedMaterials++;continue;}
   for(unsigned int j=0;j<parsed.textureCount;j++)if(!textureReady||textureReady(context,parsed.textureNames[j])){
    const char* path=parsed.textures[j];if(!Add(path,materialRegion))return Fail("Invalid material texture dependency");unsigned int length=lstrlenA(path);
    if(length>=8&&Equal(path+length-8,".TxAnm00")){
     const unsigned char* animationBytes=0;unsigned int animationSize=0;if(!read(context,path,animationBytes,animationSize))return Fail("Could not load texture animation");
     TextureAnimationResources::Animation animation;if(!animation.Read(animationBytes,animationSize))return Fail(animation.error);
     for(unsigned int k=0;k<animation.count;k++)if(!Add(animation.frames[k],materialRegion))return Fail("Invalid texture animation dependency");
    }
   }
  }
  objectSeedStart=total;StreamingConvex::Block spatialGeometry;bool spatialBuilt=false;
  // Original gatherer invokes callbacks in its name hash-map iteration order.
  // Area containers establish ancestry but are not inserted in that map.
  // Key and ShadowOverride are compiler inputs consumed before resource callbacks.
  // Live original table observation confirms their absence, retaining navigation nodes.
  ResourceOrder::List objectOrder;objectOrder.Init(&memory);
  const PrefabScene::Node** objectNodes=(const PrefabScene::Node**)memory.Allocate((model.count+1)*sizeof(void*));if(!objectNodes)return Fail("Allocation failed");
  for(const PrefabScene::Node* node=model.first;node;node=node->next){if(node->index>=model.count)return Fail("Invalid object index");objectNodes[node->index]=node;
   if(node->kind==2&&!FearBrushFlags::Equal(node->className,"ObjectResourceArea")&&!FearBrushFlags::Equal(node->className,"Key")&&!FearBrushFlags::Equal(node->className,"ShadowOverride")){bool inserted;if(!objectOrder.Insert(node->name,node->index,&inserted)){LogLine(node->name);return Fail("Invalid resource object name");}}
  }
  char callbackCount[16];unsigned int traceCount=objectOrder.count,traceDigits=0;do{callbackCount[traceDigits++]=(char)('0'+traceCount%10);traceCount/=10;}while(traceCount);for(unsigned int i=0;i<traceDigits/2;i++){char c=callbackCount[i];callbackCount[i]=callbackCount[traceDigits-1-i];callbackCount[traceDigits-1-i]=c;}callbackCount[traceDigits]=0;requests.Write("@metadata","ObjectOrder","UniqueCount",callbackCount);
  // Supported object callbacks: SoundFX.Sound and KeyFramer.ActiveSound,
  // Explicit WAV paths, global/nearest area routing and recovered spatial AABB
  // insertion. Optional resolver supplies compiled sound database record resources.
  for(ResourceOrder::Node* callback=objectOrder.head;callback;callback=callback->next){
   const PrefabScene::Node* node=objectNodes[callback->value];
   requests.Write(node->name,node->className,"@callback","");
   const char* propertyNames[19];unsigned int propertyCount=SoundProperties(node->className,propertyNames);if(!propertyCount)continue;
   const PrefabScene::Node* resourceArea=0;bool contained=false;unsigned int depth=0;for(const PrefabScene::Node* ancestor=node;ancestor->parent!=0xffffffff;){if(depth++>=128)return Fail("Invalid sound object ancestry");const PrefabScene::Node* found=0;for(const PrefabScene::Node* n=model.first;n;n=n->next)if(n->index==ancestor->parent){found=n;break;}if(!found)return Fail("Invalid sound object ancestry");ancestor=found;
    if(FearBrushFlags::Equal(ancestor->className,"ObjectResourceArea")){
     contained=true;bool global=false;for(const PrefabScene::Property* p=ancestor->properties;p;p=p->next)if(FearBrushFlags::Equal(p->name,"GlobalResourceOverride"))global=FearBrushFlags::Equal(p->data,"1");
     if(!global)resourceArea=ancestor;break;
    }
   }
   if(FearBrushFlags::Equal(node->className,"WorldProperties")||FearBrushFlags::Equal(node->className,"Dialogue"))resourceArea=0;
   bool polyGrid=FearBrushFlags::Equal(node->className,"PolyGrid");if(polyGrid)resourceArea=node;
   if(contained&&FearBrushFlags::Equal(node->className,"WorldModel"))continue;
   for(unsigned int propertyIndex=0;propertyIndex<propertyCount;propertyIndex++){
    const PrefabScene::Property* p=node->properties;while(p&&!FearBrushFlags::Equal(p->name,propertyNames[propertyIndex]))p=p->next;if(!p)continue;
    const char* path=p->data;if(!path||lstrlenA(path)>4096)return Fail("Invalid sound property");
    const char* propertyPath=path;
    if(polyGrid){unsigned int length=lstrlenA(path);if(length>MAX_PATH)length=MAX_PATH;char* truncated=(char*)memory.Allocate(length+1);if(!truncated)return Fail("Allocation failed");for(unsigned int i=0;i<length;i++)truncated[i]=path[i];truncated[length]=0;propertyPath=truncated;}
    if(FearBrushFlags::Equal(node->className,"CAI")){
     static const char* extra[]={"ChildModel_1","ChildModel_2","ChildModel_3","ChildModel_4","ChildModel_5","ChildModel_6","ChildModel_7","ChildModel_8","Weapon0","Weapon1","Weapon2","Weapon3"};
     unsigned int bytes=lstrlenA(path)+1;const char* values[12];for(unsigned int i=0;i<12;i++){values[i]="";for(const PrefabScene::Property* prop=node->properties;prop;prop=prop->next)if(FearBrushFlags::Equal(prop->name,extra[i])){values[i]=prop->data;break;}unsigned int length=lstrlenA(values[i]);if(length>4096)return Fail("Invalid AI resource property");bytes+=length+1;}
     char* packed=(char*)memory.Allocate(bytes);if(!packed)return Fail("Allocation failed");lstrcpyA(packed,path);unsigned int at=lstrlenA(path)+1;for(unsigned int i=0;i<12;i++){lstrcpyA(packed+at,values[i]);at+=lstrlenA(values[i])+1;}propertyPath=packed;
    }
    if(FearBrushFlags::Equal(node->className,"WeaponItem")){if(!path[0])continue;const char* ammo="";for(const PrefabScene::Property* a=node->properties;a;a=a->next)if(FearBrushFlags::Equal(a->name,"AmmoName"))ammo=a->data;
     unsigned int weaponLength=lstrlenA(path),ammoLength=lstrlenA(ammo);if(ammoLength>4096)return Fail("Invalid ammo property");char* pair=(char*)memory.Allocate(weaponLength+ammoLength+2);if(!pair)return Fail("Allocation failed");lstrcpyA(pair,path);pair[weaponLength]='\n';lstrcpyA(pair+weaponLength+1,ammo);propertyPath=pair;}
    const char** resolved=0;unsigned int resolvedCount=0;bool found=false;
    bool worldModel=FearBrushFlags::Equal(node->className,"WorldModel");
    SoundResources resolver=(FearBrushFlags::Equal(node->className,"SpecialFX")||(worldModel&&propertyIndex==2))?effectResources:soundResources;
    if(polyGrid){resolved=&propertyPath;resolvedCount=1;found=true;}
    else if(resolver&&propertyPath[0]&&!resolver(context,propertyPath,resolved,resolvedCount,found,FearBrushFlags::Equal(node->className,"Prop")?12:FearBrushFlags::Equal(node->className,"CAI")?11:FearBrushFlags::Equal(node->className,"Dialogue")?10:FearBrushFlags::Equal(node->className,"WorldProperties")?9:worldModel?6+propertyIndex:FearBrushFlags::Equal(node->className,"WeaponItem")?5:FearBrushFlags::Equal(node->className,"GearItem")?3:FearBrushFlags::Equal(node->className,"RefillStation")?4:FearBrushFlags::Equal(node->className,"CTFFlagBase")?13:FearBrushFlags::Equal(node->className,"ControlPoint")?14:FearBrushFlags::Equal(node->className,"Turret")?15:FearBrushFlags::Equal(node->className,"Ladder")?2:FearBrushFlags::Equal(node->className,"SoundButeFX")?1:0))return Fail("Could not resolve sound database dependencies");
    if(found&&(resolvedCount>100000||(!resolved&&resolvedCount)))return Fail("Invalid sound database results");
    if(!found&&(FearBrushFlags::Equal(node->className,"WorldProperties")||worldModel||FearBrushFlags::Equal(node->className,"SoundButeFX")||FearBrushFlags::Equal(node->className,"SpecialFX")||FearBrushFlags::Equal(node->className,"Ladder")||FearBrushFlags::Equal(node->className,"GearItem")||FearBrushFlags::Equal(node->className,"RefillStation")||FearBrushFlags::Equal(node->className,"WeaponItem")||FearBrushFlags::Equal(node->className,"CTFFlagBase")||FearBrushFlags::Equal(node->className,"ControlPoint")||FearBrushFlags::Equal(node->className,"Turret"))){/* CONDEMNED FORK: profile "none" has no sound/FX database; class resources are not gathered */continue;}
    if(!found&&!resolver)continue; // CONDEMNED FORK: no game resource database (profile none)
    if(!found){unsigned int length=lstrlenA(propertyPath);if(length&&!(length>=4&&Equal(propertyPath+length-4,".wav")))return Fail("Sound database dependencies not implemented");resolved=&propertyPath;resolvedCount=1;}
    for(unsigned int soundIndex=0;soundIndex<resolvedCount;soundIndex++){
    path=resolved[soundIndex];if(path)requests.Write(node->name,node->className,propertyNames[propertyIndex],path);if(!path||lstrlenA(path)>4096)return Fail("Invalid resolved sound path");unsigned int n=lstrlenA(path);if(resourceArea){
     WorldText::Vec3 dims={0,0,0};if(polyGrid)dims.x=dims.y=dims.z=1;const char* regions="";for(const PrefabScene::Property* a=resourceArea->properties;a;a=a->next){
      if(FearBrushFlags::Equal(a->name,"StreamingRegions"))regions=a->data;
      if(FearBrushFlags::Equal(a->name,"Dims")){WorldText::Document doc;if(!doc.Parse(a->data,lstrlenA(a->data)))return Fail("Invalid resource area dimensions");unsigned int form=doc.First(0),inner=doc.Next(doc.First(form));if(!inner||!doc.Vector(inner,dims))return Fail("Invalid resource area dimensions");}
     }
     if(polyGrid||((double)dims.x*dims.x+(double)dims.y*dims.y)+(double)dims.z*dims.z>(double)0.001f){
      // The original encloses the rotated area in an AABB, then subtracts
      // the same world offset as streaming convex geometry.
      if(!spatialBuilt){if(!spatialGeometry.Build(model))return Fail(spatialGeometry.error);spatialBuilt=true;}
      WorldText::Vec3 low,high;const PrefabScene::Matrix& rotation=resourceArea->orientation;
      for(unsigned int a=0;a<3;a++){double extent=0;for(unsigned int b=0;b<3;b++){float coefficient=rotation.m[a][b];if(coefficient<0)coefficient=-coefficient;extent+=(double)coefficient*(&dims.x)[b];}float half=(float)extent;(&low.x)[a]=(float)((double)(&resourceArea->position.x)[a]-half)-(&spatialGeometry.offset.x)[a];(&high.x)[a]=(float)((double)(&resourceArea->position.x)[a]+half)-(&spatialGeometry.offset.x)[a];}
      if(!WorldText::FiniteVector(low)||!WorldText::FiniteVector(high))return Fail("Invalid spatial area bounds");
      bool any=false;
      if(n)for(unsigned int r=0;r<spatialGeometry.regionCount;r++){
       bool matchedRegion=false;
       for(unsigned int v=0;v<spatialGeometry.volumeCount&&!matchedRegion;v++)if(spatialGeometry.volumes[v].owner==r){
        const StreamingConvex::Volume& source=spatialGeometry.volumes[v];ConvexIntersection::Volume volume;volume.count=source.count;volume.low=*(const WorldText::Vec3*)spatialGeometry.bounds[v].low;volume.high=*(const WorldText::Vec3*)spatialGeometry.bounds[v].high;
        ConvexIntersection::Face* faces=(ConvexIntersection::Face*)memory.Allocate(source.count*sizeof(ConvexIntersection::Face));if(!faces)return Fail("Allocation failed");volume.faces=faces;
        for(unsigned int f=0;f<source.count;f++){faces[f].points=source.faces[f].points;faces[f].count=source.faces[f].count;faces[f].plane=source.faces[f].plane;}
        if(!ResourceAreaIntersection::Test(volume,low,high,matchedRegion))return Fail("Invalid spatial area intersection");
       }
       if(matchedRegion){any=true;if(!AddAsset(path,r+1,read,context,expandTextures,shaderReady,textureReady))return Fail("Invalid regional sound resource");}
      }
      if(n&&!any&&!AddAsset(path,0,read,context,expandTextures,shaderReady,textureReady))return Fail("Invalid global sound fallback");continue;
     }
     bool matched=false;const char* cursor=regions;while(*cursor){while(*cursor==' '||*cursor=='\t'||*cursor==',')cursor++;const char* start=cursor;while(*cursor&&*cursor!=',')cursor++;unsigned int length=(unsigned int)(cursor-start);while(length&&(start[length-1]==' '||start[length-1]=='\t'))length--;if(length>255)return Fail("Region name too long");char name[256];for(unsigned int k=0;k<length;k++)name[k]=start[k];name[length]=0;
      for(const PrefabScene::Node* region=model.first;region;region=region->next)if(region->kind==2&&FearBrushFlags::Equal(region->className,"StreamingRegion")&&Equal(region->name,name))matched=true;
     }
     // Original DefaultPrefetch passes object name instead of resource path in
     // named-region mode; observed packer puts this resource in Global.
     if(n&&matched&&!Add(node->name,0))return Fail("Invalid compatible sound object resource");
    }else if(!AddAsset(path,0,read,context,expandTextures,shaderReady,textureReady))return Fail("Invalid sound resource");
    }
   }
  }
  seeds=(StreamingResources::Seed*)memory.Allocate((total+1)*sizeof(StreamingResources::Seed));if(!seeds)return Fail("Allocation failed");
  for(Item* i=first;i;i=i->next){seeds[count].region=i->region;seeds[count++].path=i->path;}return true;
 }
};
}





