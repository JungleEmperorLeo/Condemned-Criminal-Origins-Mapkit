#pragma once
#include "MaterialResources.h"
#include "SoundDatabaseResources.h"
namespace SourcePhysicalBspMetadata {
struct Value {unsigned short material;unsigned char occlusion;};
class Resolver {
 struct Record {Record* next;char name[260];unsigned short surface;bool present;};
 // Packer log only: material file presence, kept apart from the resolution cache.
 struct Presence {Presence* next;char name[260];bool present;};PrefabScene::Arena presenceMemory;Presence* presence;
 PrefabScene::Arena memory;Record* first;unsigned int materialCount;
 PrefabScene::Project project;char resourceRoot[260];SoundDatabaseResources::Reader database;bool ready,databaseReady;
 Resolver(const Resolver&);Resolver& operator=(const Resolver&);
 bool Fail(const char* reason){error=reason;return false;}
 bool Material(const char* input,unsigned short& output){
  output=0;if(!input||!input[0])return true;while(*input=='/'||*input=='\\')input++;
  if(!input[0]||lstrlenA(input)>259)return Fail("Invalid BSP material name");
  for(unsigned int i=0;input[i];i++)if((unsigned char)input[i]<32||(unsigned char)input[i]>=128||input[i]==':')return Fail("Unsupported BSP material path");
  for(Record* r=first;r;r=r->next)if(FearBrushFlags::Equal(r->name,input)){output=r->surface;return true;}
  if(materialCount==16384)return Fail("BSP material cache limit exceeded");char path[260];if(!PrefabScene::Join(resourceRoot,input,path))return Fail("BSP material path exceeds bound");
  for(unsigned int i=0;resourceRoot[i];i++){char a=resourceRoot[i],b=path[i];if(a>='A'&&a<='Z')a=(char)(a+32);if(b>='A'&&b<='Z')b=(char)(b+32);if(!b||a!=b)return Fail("BSP material path leaves resource root");}
  HANDLE file=CreateFileA(path,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);
  if(file==INVALID_HANDLE_VALUE){
   // A material file absent from the resources contributes surface flags 0,
   // like an empty name (SDK SpillKill references a missing Wood_SL001.Mat00).
   Record* missing=(Record*)memory.Allocate(sizeof(Record));if(!missing)return Fail("BSP material cache allocation failed");
   lstrcpyA(missing->name,input);missing->surface=0;missing->present=false;missing->next=first;first=missing;materialCount++;return true;
  }
  DWORD high=0,bytes=GetFileSize(file,&high),done=0;
  if(high||bytes==0xffffffffu||bytes>16u*1024u*1024u){CloseHandle(file);return Fail("Invalid BSP material resource size");}
  PrefabScene::Arena temporary;unsigned char* inputBytes=(unsigned char*)temporary.Allocate(bytes?bytes:1);
  bool read=inputBytes&&ReadFile(file,inputBytes,bytes,&done,0)&&done==bytes;CloseHandle(file);if(!read)return Fail("BSP material resource read failed");
  MaterialResources::Material material;if(!material.Read(inputBytes,bytes))return Fail(material.error);
  Record* record=(Record*)memory.Allocate(sizeof(Record));if(!record)return Fail("BSP material cache allocation failed");
  lstrcpyA(record->name,input);record->surface=(unsigned short)material.surfaceFlags;record->present=true;record->next=first;first=record;materialCount++;output=record->surface;return true;
 }
 bool Occlusion(const char* name,unsigned char& output){
  output=255;if(!name||!name[0])return true;
  if(!databaseReady){if(!database.Initialize(project))return Fail(database.error);databaseReady=true;}
  HCATEGORY category=database.Category("Sound/Occlusion");if(!category)return Fail("Missing sound occlusion category");unsigned int count=database.RecordCount(category);
  if(count>256)return Fail("Sound occlusion editor table exceeds 256 records");
  // The compiler compares pcName and returns the FIRST record ordinal.
  // The database ID attribute does not participate; unknown names stay255.
  for(unsigned int i=0;i<count;i++){const char* recordName=database.RecordName(database.RecordAt(category,i));if(recordName&&FearBrushFlags::Equal(recordName,name)){output=(unsigned char)i;break;}}return true;
 }
public:
 const char* error;
 Resolver():presence(0),first(0),materialCount(0),ready(false),databaseReady(false),error(0){resourceRoot[0]=0;}
  bool Initialize(const PrefabScene::Project& input){if(ready)return Fail("BSP metadata resolver already initialized");if(input.error||!input.file[0]||!input.resources[0]||!PrefabScene::Full(input.resources,resourceRoot))return Fail("BSP metadata requires an explicit valid project");unsigned int n=lstrlenA(resourceRoot);if(!n||n+2>=260)return Fail("BSP resource root exceeds bound");if(resourceRoot[n-1]!='\\'&&resourceRoot[n-1]!='/'){resourceRoot[n++]='\\';resourceRoot[n]=0;}project=input;ready=true;error=0;return true;}
 // Packer log "Imported %d texture flags" (0x1001D6B0/0x1001D4B0): every polygon
 // with a texture name gets a table entry, whether or not its Mat00 loads.
 bool Present(const char* material,bool& present){present=false;if(!material)return true;while(*material=='/'||*material=='\\')material++;present=material[0]!=0;return true;}
 bool Resolve(const WorldText::Polygon& polygon,Value& output){error=0;if(!ready)return Fail("BSP metadata resolver not initialized");Value value={};if(!Material(polygon.material,value.material)||!Occlusion(polygon.occlusion,value.occlusion))return false;output=value;return true;}
 unsigned int Materials()const{return materialCount;}
};
}
