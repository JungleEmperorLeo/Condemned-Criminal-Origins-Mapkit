#pragma once
#include "SoundDatabaseApi.h"
// Project database -> owned NDB1 manifest for the Nav generator (formerly the
// body of nav-sdk-native/database/export-nav-database.cpp). Loads the project's
// GameDatabase binary (and the ToolsRoot msvcr71/msvcp71 it needs) through its
// public IDatabaseMgr; never loads WorldPacker.dll.
namespace NavDatabaseManifest {
struct Record {unsigned int index,mask,nav,length;char* name;};
struct Paths {char project[MAX_PATH],resources[MAX_PATH],binary[MAX_PATH],database[MAX_PATH];};
class Builder {
 PrefabScene::Arena owned;
 static bool Exact(const char* a,const char* b){if(!a||!b)return false;while(*a&&*a==*b){++a;++b;}return *a==*b;}
 bool Fail(const char* message,unsigned int code){error=message;errorCode=code;return false;}
 bool CopyName(IDatabaseMgr* manager,HRECORD record,Record& out,unsigned int limit){const char* name=manager->GetRecordName(record);if(!name)return Fail("Database record has no name",11);unsigned int n=lstrlenA(name);if(!n||n>limit)return Fail("Database record name exceeds supported bounds",11);for(unsigned int i=0;i<n;i++)if((unsigned char)name[i]<32||(unsigned char)name[i]>126)return Fail("Database record name requires a non-ASCII lookup profile",11);out.length=n;out.name=(char*)owned.Allocate(n+1);if(!out.name)return Fail("Out of memory copying database names",12);lstrcpyA(out.name,name);return true;}
public:
 unsigned char* data;unsigned int size;Paths paths;const char* error;unsigned int errorCode;
 Builder():data(0),size(0),error(0),errorCode(0){}
 bool Build(const char* projectInput){
  owned.Clear();data=0;size=0;error=0;errorCode=0;PrefabScene::Arena working;PrefabScene::Project project;
  if(!project.LoadPath(projectInput))return Fail(project.error?project.error:"Invalid explicit project",2);char base[MAX_PATH],relative[MAX_PATH],game[MAX_PATH],tools[MAX_PATH],dependency[MAX_PATH];lstrcpyA(base,project.file);PrefabScene::Parent(base);
  DWORD gameSize=GetPrivateProfileStringA("General","Game","",game,MAX_PATH,project.file);if(!gameSize||gameSize>=MAX_PATH-1||!(Exact(game,"FEAR")||Exact(game,"DARK")))return Fail("Only explicit General/Game=FEAR default database convention is supported",2);
  DWORD n=GetPrivateProfileStringA("Project Binaries","GameDatabase","",relative,MAX_PATH,project.file);if(!n||n>=MAX_PATH-1||!PrefabScene::Join(base,relative,paths.binary))return Fail("Invalid project database binary/tools/resource path",2);
  n=GetPrivateProfileStringA("Project Directories","ToolsRoot","",relative,MAX_PATH,project.file);if(!n||n>=MAX_PATH-1||!PrefabScene::Join(base,relative,tools)||!PrefabScene::Join(project.resources,"Database\\FEAR.Gamdb00p",paths.database))return Fail("Invalid project database binary/tools/resource path",2);
  // CONDEMNED FORK: the retail game database is Database\Dark.Gamdb00p (no FEAR.Gamdb00p).
  {HANDLE probe=CreateFileA(paths.database,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);
   if(probe==INVALID_HANDLE_VALUE){char alt[MAX_PATH];if(PrefabScene::Join(project.resources,"Database\\Dark.Gamdb00p",alt))lstrcpyA(paths.database,alt);}else CloseHandle(probe);}
  lstrcpyA(paths.project,project.file);lstrcpyA(paths.resources,project.resources);
  if(PrefabScene::Join(tools,"msvcr71.dll",dependency))LoadLibraryA(dependency);if(PrefabScene::Join(tools,"msvcp71.dll",dependency))LoadLibraryA(dependency);
  HMODULE module=LoadLibraryA(paths.binary);if(!module)return Fail("Could not load project GameDatabase binary",3);typedef IDatabaseMgr* (*Factory)();Factory factory=(Factory)GetProcAddress(module,"GetIDatabaseMgr");IDatabaseMgr* manager=factory?factory():0;if(!manager)return Fail("GameDatabase has no public IDatabaseMgr factory",3);
  HANDLE input=CreateFileA(paths.database,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);if(input==INVALID_HANDLE_VALUE)return Fail("Could not open project default FEAR database",4);DWORD high=0;unsigned int bytes=GetFileSize(input,&high);
  if(high||!bytes||bytes>64*1024*1024){CloseHandle(input);return Fail("Database input size exceeds supported bounds",4);}unsigned char* content=(unsigned char*)working.Allocate(bytes);DWORD done=0;bool read=content&&ReadFile(input,content,bytes,&done,0)&&done==bytes;CloseHandle(input);if(!read)return Fail("Could not read project database",4);
  MemoryStream stream(content,bytes);HDATABASE handle=manager->OpenNewDatabase(paths.database,stream);if(!handle||stream.HasErrorOccurred())return Fail("Public database reader rejected project database",5);
  bool collected=Collect(manager,handle);manager->ReleaseDatabase(handle);if(!collected)return false;
  // Only owned scalar metadata/name copies are accessed after ReleaseDatabase.
  unsigned int total=16;for(unsigned int i=0;i<navCount;i++)total+=12+nav[i].length;for(unsigned int i=0;i<characterCount;i++)total+=16+characters[i].length;
  data=(unsigned char*)owned.Allocate(total);if(!data)return Fail("Out of memory writing database manifest",12);unsigned char* at=data;
  Word(at,0x3142444e);Word(at,1);Word(at,navCount);Word(at,characterCount);for(unsigned int i=0;i<navCount;i++){const Record& row=nav[i];Word(at,row.index);Word(at,row.mask);Word(at,row.length);memcpy(at,row.name,row.length);at+=row.length;}
  for(unsigned int i=0;i<characterCount;i++){const Record& row=characters[i];Word(at,row.index);Word(at,row.mask);Word(at,row.nav);Word(at,row.length);memcpy(at,row.name,row.length);at+=row.length;}
  size=total;return (unsigned int)(at-data)==total||Fail("Database manifest framing mismatch",6);
 }
private:
 Record* nav;Record* characters;unsigned int navCount,characterCount;
 static void Word(unsigned char*& at,unsigned int v){memcpy(at,&v,4);at+=4;}
 bool Collect(IDatabaseMgr* manager,HDATABASE handle){
  HCATEGORY navCategory=manager->GetCategory(handle,"AI/NavMeshTypes"),characterCategory=manager->GetCategory(handle,"AI/Attributes");if(!navCategory||!characterCategory)return Fail("Missing AI/NavMeshTypes or AI/Attributes category",10);
  navCount=manager->GetNumRecords(navCategory);characterCount=manager->GetNumRecords(characterCategory);if(!navCount||navCount>4096||characterCount>32)return Fail("Database categories exceed supported nav/32-bit character profile",10);
  nav=(Record*)owned.Allocate(navCount*sizeof(Record));characters=characterCount?(Record*)owned.Allocate(characterCount*sizeof(Record)):0;if(!nav||(characterCount&&!characters))return Fail("Out of memory collecting database",12);
  for(unsigned int i=0;i<navCount;i++){HRECORD record=manager->GetRecordByIndex(navCategory,i);if(!record||manager->GetRecordIndex(record)!=i)return Fail("Invalid indexed navigation type record",11);Record& row=nav[i];row.index=i;row.mask=0;row.nav=0xffffffffu;if(!CopyName(manager,record,row,29))return false;}
  for(unsigned int i=0;i<characterCount;i++){HRECORD record=manager->GetRecordByIndex(characterCategory,i);if(!record||manager->GetRecordIndex(record)!=i)return Fail("Invalid indexed AI attributes record",11);Record& row=characters[i];row.index=i;row.mask=1u<<i;row.nav=0xffffffffu;if(!CopyName(manager,record,row,4095))return false;
   HATTRIBUTE attribute=manager->GetAttribute(record,"NavMeshType");if(!attribute)continue;if(manager->GetAttributeType(attribute)!=eAttributeType_RecordLink||manager->GetNumValues(attribute)>1)return Fail("Unsupported NavMeshType record-link attribute",11);if(!manager->GetNumValues(attribute))continue;
   HRECORD link=manager->GetRecordLink(attribute,0,0);if(!link)continue;if(manager->GetRecordParent(link)!=navCategory)return Fail("NavMeshType link points outside AI/NavMeshTypes",11);row.nav=manager->GetRecordIndex(link);if(row.nav>=navCount)return Fail("NavMeshType link index out of range",11);nav[row.nav].mask|=row.mask;}
  return true;
 }
};
}
