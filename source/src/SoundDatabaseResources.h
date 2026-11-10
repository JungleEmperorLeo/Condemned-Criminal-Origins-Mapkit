#pragma once
#include "SoundDatabaseApi.h"
namespace SoundDatabaseResources {
typedef bool (*EffectResources)(void*,const char*,const char**&,unsigned int&,bool&);
// External public SDK database adapter. This does not decode Gamdb00p independently.
class Reader {
 PrefabScene::Arena data,results;IDatabaseMgr* manager;HDATABASE database;HCATEGORY category;
 Reader(const Reader&);Reader& operator=(const Reader&);
public:
 const char* error;
 Reader():manager(0),database(0),category(0),error(0){}
 ~Reader(){if(database&&manager)manager->ReleaseDatabase(database);}
 bool Initialize(const PrefabScene::Project& project){
  if(database)return true;error=0;char dllPath[MAX_PATH],databasePath[MAX_PATH];
  if(!project.Resolve("Project Binaries","GameDatabase",dllPath)){error="Missing GameDatabase binary";return false;}
  // CRT prerequisites are supplied by the installed SDK Tools directory.
  char tools[MAX_PATH],dependency[MAX_PATH];if(project.Resolve("Project Directories","ToolsRoot",tools)){
   if(PrefabScene::Join(tools,"msvcr71.dll",dependency))LoadLibraryA(dependency);
   if(PrefabScene::Join(tools,"msvcp71.dll",dependency))LoadLibraryA(dependency);
  }
  void* dll=LoadLibraryA(dllPath);if(!dll){error="Could not load GameDatabase";return false;}
  typedef IDatabaseMgr* (*Factory)();Factory factory=(Factory)GetProcAddress(dll,"GetIDatabaseMgr");if(!factory){error="Missing database interface";return false;}manager=factory();if(!manager){error="Missing database manager";return false;}
  if(!PrefabScene::Join(project.resources,"Database\\FEAR.Gamdb00p",databasePath)){error="Database path too long";return false;}
  HANDLE f=CreateFileA(databasePath,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);if(f==INVALID_HANDLE_VALUE){error="Could not open compiled database";return false;}
  unsigned int size=GetFileSize(f,0);unsigned char* bytes=size&&size<=64*1024*1024?(unsigned char*)data.Allocate(size):0;DWORD done;
  bool ok=bytes&&ReadFile(f,bytes,size,&done,0)&&done==size;CloseHandle(f);if(!ok){error="Invalid compiled database input";return false;}
  MemoryStream stream(bytes,size);database=manager->OpenNewDatabase(databasePath,stream);
  if(!database||stream.HasErrorOccurred()){if(database)manager->ReleaseDatabase(database);database=0;error="Database load failed";return false;}
  category=manager->GetCategory(database,"Sound/Sound");if(!category){manager->ReleaseDatabase(database);database=0;error="Missing sound category";return false;}return true;
 }
 HCATEGORY Category(const char* path){return database?manager->GetCategory(database,path):0;}
 unsigned int RecordCount(HCATEGORY c){return c?manager->GetNumRecords(c):0;}
 HRECORD RecordAt(HCATEGORY c,unsigned int i){return c?manager->GetRecordByIndex(c,i):0;}
 const char* RecordName(HRECORD record){return record?manager->GetRecordName(record):0;}
 const char* String(HRECORD r,const char* attr,unsigned int i=0){if(!r)return "";HATTRIBUTE a=manager->GetAttribute(r,attr);return a?manager->GetString(a,i,""):"";}
 int Integer(HRECORD r,const char* attr,int fallback=0){if(!r)return fallback;HATTRIBUTE a=manager->GetAttribute(r,attr);return a?manager->GetInt32(a,0,fallback):fallback;}
 unsigned int ValueCount(HRECORD record,const char* attribute){if(!record)return 0;HATTRIBUTE a=manager->GetAttribute(record,attribute);return a?manager->GetNumValues(a):0;}
 bool HasRecord(const char* categoryPath,const char* name){HCATEGORY selected=database?manager->GetCategory(database,categoryPath):0;return selected&&manager->GetRecord(selected,name);}
 bool Resolve(const char* name,const char**& paths,unsigned int& count,bool& found,bool recurse=false,const char* categoryPath="Sound/Sound",EffectResources effects=0,void* effectContext=0){
  results.Clear();paths=0;count=0;found=false;error=0;if(!database||!category){error="Database not initialized";return false;}
  HCATEGORY selected=manager->GetCategory(database,categoryPath);if(!selected){error="Missing resource category";return false;}HRECORD record=manager->GetRecord(selected,name);if(!record)return true;found=true;
  return ResolveRecord(record,paths,count,recurse,effects,effectContext);
 }
 HRECORD Record(const char* categoryPath,const char* name){HCATEGORY selected=database?manager->GetCategory(database,categoryPath):0;return selected?manager->GetRecord(selected,name):0;}
 HRECORD Link(HRECORD record,const char* attribute,unsigned int index=0){if(!record)return 0;HATTRIBUTE a=manager->GetAttribute(record,attribute);return a?manager->GetRecordLink(a,index,0):0;}
 bool PlayerRecords(PrefabScene::Arena& playerStorage,HRECORD*& records,unsigned int& count){
  count=0;records=0;if(!database)return false;HRECORD player=Record("Character/Models","Player");if(!player)return true;
  HRECORD global=Record("Arsenal/Global","Global");if(!global)return false;HATTRIBUTE defaults=manager->GetAttribute(global,"AdditionalDefaultWeapons");unsigned int n=defaults?manager->GetNumValues(defaults):0;if(n>4096)return false;
  records=(HRECORD*)playerStorage.Allocate((1+2*n)*sizeof(HRECORD));if(!records)return false;records[count++]=player;
  // Packer editor context has no server restriction list. Default weapon data,
  // and Player ammo data are used; multiplayer overrides require separate proof.
  for(unsigned int i=0;i<n;i++){HRECORD weapon=manager->GetRecordLink(defaults,i,0);if(!weapon)continue;HRECORD weaponData=Link(weapon,"Default"),ammo=Link(weaponData,"AmmoName");if(!Link(ammo,"Player"))continue;records[count++]=weapon;records[count++]=ammo;}
  return true;
 }
 bool ResolveRecord(HRECORD record,const char**& paths,unsigned int& count,bool recurse,EffectResources effects=0,void* effectContext=0){
  results.Clear();paths=0;count=0;if(!database||!record)return false;
  // SDK PrefetchUtilities uses a LIFO work list, pending-link deduplication,
  // and a per-call history. Preserve attribute order and reverse queued traversal.
  const unsigned int limit=100000;
  HRECORD* pending=(HRECORD*)results.Allocate(limit*sizeof(HRECORD));HRECORD* history=(HRECORD*)results.Allocate(limit*sizeof(HRECORD));
  paths=(const char**)results.Allocate(limit*sizeof(char*));if(!pending||!history||!paths)return false;
  unsigned int pendingCount=1,historyCount=0;pending[0]=record;
  while(pendingCount){record=pending[--pendingCount];bool seen=false;for(unsigned int h=0;h<historyCount;h++)if(history[h]==record){seen=true;break;}if(seen)continue;
   if(historyCount==limit)return false;history[historyCount++]=record;
   unsigned int attributes=manager->GetNumAttributes(record);if(attributes>4096)return false;
   for(unsigned int a=0;a<attributes;a++){HATTRIBUTE attr=manager->GetAttributeByIndex(record,a);EAttributeType type=manager->GetAttributeType(attr);EAttributeUsage usage=manager->GetAttributeUsage(attr);unsigned int n=manager->GetNumValues(attr);if(n>limit)return false;
    if(type==eAttributeType_String&&usage==eAttributeUsage_ClientFX)for(unsigned int i=0;i<n;i++){const char* fx=manager->GetString(attr,i,"");if(!fx[0])continue;if(!effects){error="Database ClientFX dependencies unsupported";return false;}const char** files=0;unsigned int fileCount=0;bool fxFound=false;if(!effects(effectContext,fx,files,fileCount,fxFound)||fileCount>limit-count||(!files&&fileCount)){error="Database ClientFX lookup failed";return false;}for(unsigned int j=0;j<fileCount;j++){unsigned int length=lstrlenA(files[j]);if(length>4096)return false;char* copy=(char*)results.Allocate(length+1);if(!copy)return false;lstrcpyA(copy,files[j]);paths[count++]=copy;}}
    if(type==eAttributeType_String&&usage==eAttributeUsage_Filename)for(unsigned int i=0;i<n;i++){if(count==limit)return false;const char* value=manager->GetString(attr,i,"");unsigned int length=lstrlenA(value);if(length>4096)return false;char* copy=(char*)results.Allocate(length+1);if(!copy)return false;lstrcpyA(copy,value);paths[count++]=copy;}
    if(recurse&&type==eAttributeType_RecordLink)for(unsigned int i=0;i<n;i++){HRECORD link=manager->GetRecordLink(attr,i,0);if(!link||link==record)continue;bool queued=false;for(unsigned int j=0;j<pendingCount;j++)if(pending[j]==link){queued=true;break;}if(!queued){if(pendingCount==limit)return false;pending[pendingCount++]=link;}}
   }
  }return true;
 }
};
}
