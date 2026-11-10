#pragma once
// Optional published GameServer ObjectDLLSetup adapter. No game objects,
// plugins, resource gatherers, DEdit hooks, or original packer are invoked.
// Include after PrefabScene and WorldModels. The pure catalog owns every string.
extern "C" {
__declspec(dllimport) HMODULE WINAPI LoadLibraryExA(const char*,HANDLE,DWORD);
__declspec(dllimport) void* WINAPI GetProcAddress(HMODULE,const char*);
__declspec(dllimport) BOOL WINAPI FreeLibrary(HMODULE);
__declspec(dllimport) unsigned int WINAPI VirtualQuery(const void*,void*,unsigned int);
}
#pragma comment(linker,"/alternatename:__imp__LoadLibraryExA@12=__imp__LoadLibraryExA")
#pragma comment(linker,"/alternatename:__imp__GetProcAddress@8=__imp__GetProcAddress")
#pragma comment(linker,"/alternatename:__imp__FreeLibrary@4=__imp__FreeLibrary")
#pragma comment(linker,"/alternatename:__imp__VirtualQuery@12=__imp__VirtualQuery")
namespace GameClassMetadata {
enum {WorldModelFlag=0x10,ServerObjectVersion=1,MaximumClasses=4096};
struct SDKClassDef {
 const char* name;const char* description;SDKClassDef* parent;unsigned int flags;
 void* callbacks[4];unsigned int propertyCount;void* properties;long objectSize;void* internal[2];
};
typedef char ClassDefSizeMustBe52[(sizeof(SDKClassDef)==52)?1:-1];
struct Class {const char* name;const char* description;const char* parentName;unsigned int flags,parentIndex;};
inline bool Exact(const char* a,const char* b){if(!a||!b)return false;while(*a&&*a==*b){a++;b++;}return *a==*b;}
// Validate committed readable spans before copying public module data. Module
// unloading occurs only after all parent relationships and strings are copied.
inline bool Readable(const void* pointer,unsigned int bytes){
 if(!pointer)return !bytes;unsigned int start=(unsigned int)pointer;if(bytes>0xffffffffu-start)return false;
 unsigned int end=start+bytes;while(start<end){unsigned int m[7];if(VirtualQuery((void*)start,m,sizeof(m))!=sizeof(m))return false;
  unsigned int next=m[0]+m[3];if(m[4]!=0x1000||!m[5]||(m[5]&0x101)||next<=start)return false;start=next<end?next:end;
 }return true;
}
inline bool StringLength(const char* value,unsigned int limit,unsigned int& length){
 length=0;if(!value)return false;while(length<=limit){if(!Readable(value+length,1))return false;if(!value[length])return true;length++;}return false;
}
class Catalog {
 PrefabScene::Arena memory;
 const char* Copy(const char* input,unsigned int limit,bool allowNull=false){
  if(!input&&allowNull)input="";unsigned int length;if(!StringLength(input,limit,length))return 0;
  char* result=(char*)memory.Allocate(length+1);if(!result)return 0;for(unsigned int i=0;i<=length;i++)result[i]=input[i];return result;
 }
 bool Fail(const char* message){Clear();error=message;return false;}
public:
 Class* classes;unsigned int count;const char* error;char projectPath[MAX_PATH],modulePath[MAX_PATH];
 Catalog():classes(0),count(0),error(0){projectPath[0]=modulePath[0]=0;}
 void Clear(){memory.Clear();classes=0;count=0;error=0;}
 const Class* Find(const char* name)const{for(unsigned int i=0;i<count;i++)if(Exact(classes[i].name,name))return classes+i;return 0;}
 static WorldModels::ClassKind Resolve(const PrefabScene::Node& node,void* context){
  const Catalog* catalog=(const Catalog*)context;const Class* definition=catalog?catalog->Find(node.className):0;
  // A class the GameServer does not define is kept as an ordinary object (the
  // original packs it with its authored properties).
  if(!catalog)return WorldModels::Unknown;if(!definition)return WorldModels::Ordinary;return (definition->flags&WorldModelFlag)?WorldModels::WorldModel:WorldModels::Ordinary;
 }
 // Copies only SDK metadata. Classification uses direct flags, never inferred
 // object names or inherited parent flags. Lookup matches old cb_FindClass.
 bool CopyDefinitions(SDKClassDef* const* definitions,int number,int version){
  Clear();if(version!=ServerObjectVersion)return Fail("Game class ABI version mismatch");
  if(number<1||number>MaximumClasses||!Readable(definitions,number*4))return Fail("Invalid game class definition count or array");
  classes=(Class*)memory.Allocate(number*sizeof(Class));if(!classes)return Fail("Could not allocate class catalog");count=number;
  for(unsigned int i=0;i<count;i++){
   const SDKClassDef* d=definitions[i];if(!Readable(d,sizeof(SDKClassDef)))return Fail("Invalid game class definition pointer");
   Class& c=classes[i];c.name=Copy(d->name,255);c.description=Copy(d->description,16383,true);c.flags=d->flags;c.parentIndex=0xffffffffu;c.parentName="";
   if(!c.name||!c.name[0]||!c.description)return Fail("Invalid or oversized game class name or description");
   for(unsigned int j=0;j<i;j++)if(Exact(classes[j].name,c.name))return Fail("Duplicate game class name");
  }
  for(unsigned int i=0;i<count;i++)if(definitions[i]->parent){
   unsigned int parent=0;while(parent<count&&definitions[parent]!=definitions[i]->parent)parent++;
   if(parent==count)return Fail("Game class parent is not present in class catalog");classes[i].parentIndex=parent;classes[i].parentName=classes[parent].name;
  }
  for(unsigned int i=0;i<count;i++){unsigned int parent=i,steps=0;while(parent!=0xffffffffu){if(steps++>=count)return Fail("Cyclic game class parent metadata");parent=classes[parent].parentIndex;}}
  return true;
 }
 bool LoadModule(const char* path){
  Clear();if(!PrefabScene::Full(path,modulePath))return Fail("Invalid GameServer module path");
  HMODULE module=LoadLibraryExA(modulePath,0,8);if(!module)return Fail("Could not load project GameServer module or dependency");
  typedef SDKClassDef** (__cdecl *Setup)(int*,void*,int*);Setup setup=(Setup)GetProcAddress(module,"ObjectDLLSetup");
  if(!setup){FreeLibrary(module);return Fail("Project GameServer has no ObjectDLLSetup export");}
  int number=0,version=0;SDKClassDef** definitions=setup(&number,0,&version);
  bool copied=CopyDefinitions(definitions,number,version);FreeLibrary(module);return copied;
 }
 bool LoadProject(const char* sourcePath){
  Clear();projectPath[0]=modulePath[0]=0;PrefabScene::Project project;if(!project.Locate(sourcePath))return Fail(project.error);
  lstrcpyA(projectPath,project.file);char binary[MAX_PATH],base[MAX_PATH],resolved[MAX_PATH];
  DWORD length=GetPrivateProfileStringA("Project Binaries","GameServer","",binary,MAX_PATH,projectPath);
  if(!length||length>=MAX_PATH-1)return Fail("Missing or oversized project GameServer path");
  lstrcpyA(base,projectPath);PrefabScene::Parent(base);if(!PrefabScene::Join(base,binary,resolved))return Fail("Project GameServer path exceeds MAX_PATH");
  return LoadModule(resolved);
 }
};
}
