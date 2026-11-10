#pragma once
#include "ShaderRenderDeclaration.h"
#include "ShaderDependencyPaths.h"
#include "ShaderEffectInclude.h"
#include "ShaderLoaderGate.h"
// Shader declarations reached by the original dependency walk (RVA 0x7C5E0).
// This closes declaration/include dependencies and instance metadata affecting
// load eligibility. Actual default values/runtime parameter objects remain
// separate from this declaration compiler. Cache identity preserves
// shader filename text and compares it case insensitively, as RVA 0x7AA70 does.
namespace ShaderDeclarationClosure {
enum Kind {Parent=0,Fallback=1,High=2,Medium=3,Low=4};
struct Dependency {unsigned int kind,index;const char* path;};
typedef bool (*Visitor)(void*,const Dependency&);
inline void ParentName(unsigned int index,char* out){
 const char* prefix="ParentMaterial";unsigned int n=0;while(prefix[n]){out[n]=prefix[n];n++;}
 char digits[10];unsigned int used=0;do{digits[used++]=(char)('0'+index%10);index/=10;}while(index);
 while(used)out[n++]=digits[--used];out[n]=0;
}
inline bool Enumerate(void* effect,Visitor visitor,void* context,bool* boundExceeded=0){
 if(boundExceeded)*boundExceeded=false;
 if(!effect)return true;if(!visitor)return false;
 typedef const char* (WINAPI *Find)(void*,const char*,const char*);
 typedef const char* (WINAPI *Technique)(void*,unsigned int);
 typedef const char* (WINAPI *Annotation)(void*,const char*,const char*);
 typedef long (WINAPI *String)(void*,const char*,const char**);
 void** methods=*(void***)effect;
 for(unsigned int i=0;;i++){
  char name[25];ParentName(i,name);const char* handle=((Find)methods[9])(effect,0,name);if(!handle)break;if(i>=256){if(boundExceeded)*boundExceeded=true;return false;}
  const char* path=0;if(((String)methods[51])(effect,handle,&path)<0||!path)return false;
  Dependency dependency={Parent,i,path};if(!visitor(context,dependency))return false;
 }
 static const char* names[]={"Fallback","High","Medium","Low"};
 for(unsigned int i=0;;i++){
  const char* technique=((Technique)methods[12])(effect,i);if(!technique)break;if(i>=256){if(boundExceeded)*boundExceeded=true;return false;}
  for(unsigned int j=0;j<4;j++){
   const char* handle=((Annotation)methods[19])(effect,technique,names[j]);if(!handle)continue;
   const char* path=0;if(((String)methods[51])(effect,handle,&path)<0||!path)return false;
   Dependency dependency={j+1,i,path};if(!visitor(context,dependency))return false;
  }
 }
 return true;
}
enum State {Unresolved=0,Failed=1,Ready=2,Unsupported=3,Loading=4};
struct Record {
 char key[264];State state;const char* error;ShaderRenderDeclaration::Reader primary;ShaderEffectInclude::Resolver* include;void* includeMemory;ShaderLoaderGate::Table instances;
 unsigned int count,warnings,includeOpens,includeMaximumDepth;ShaderRenderDeclaration::Element elements[64];
 Record():state(Unresolved),error(0),include(0),includeMemory(0),count(0),warnings(0),includeOpens(0),includeMaximumDepth(0){key[0]=0;}
 void ClearCompiler(){primary.Clear();if(include){include->~Resolver();include=0;}}
 ~Record(){ClearCompiler();}
};
struct Edge {unsigned int owner,child,kind,index;};
class Cache {
 PrefabScene::Arena memory;char root[264];Record* records[256];
 struct Walk {Cache* cache;unsigned int owner;char directory[264];};
 Cache(const Cache&);Cache& operator=(const Cache&);
 static bool Equal(const char* a,const char* b){while(*a&&ShaderRenderDeclaration::Lower((unsigned char)*a)==ShaderRenderDeclaration::Lower((unsigned char)*b)){a++;b++;}return *a==*b;}
 static bool Visit(void* opaque,const Dependency& dependency){
  Walk& walk=*(Walk*)opaque;Cache& cache=*walk.cache;Record& owner=*cache.records[walk.owner];char childKey[264];
  if(!ShaderDependencyPaths::Resolve(walk.directory,dependency.path,childKey,264)){owner.state=Unsupported;owner.error="Dependency path exceeds bound";return false;}
  if(cache.edgeCount>=1024){owner.state=Unsupported;owner.error="Dependency edge bound";return false;}
  unsigned int edgeIndex=cache.edgeCount++;Edge edge={walk.owner,0xffffffffu,dependency.kind,dependency.index};cache.edges[edgeIndex]=edge;
  unsigned int child=0;bool resolved=cache.Resolve(childKey,&child);cache.edges[edgeIndex].child=child;
  if(!resolved){owner.state=Unsupported;owner.error=cache.error;return false;}
  const Record& parent=*cache.records[child];if(parent.state!=Ready){owner.state=Failed;owner.error="Dependent shader failed";return false;}
  if(!owner.instances.Merge(parent.instances)){owner.state=Unsupported;owner.error=owner.instances.error;return false;}
  if(!ShaderRenderDeclaration::Merge(owner.elements,&owner.count,64,parent.elements,parent.count)){owner.state=Unsupported;owner.error="Merged declaration exceeds bound";return false;}
  return true;
 }
public:
 unsigned int count,edgeCount;Edge edges[1024];const char* error;
 Cache():count(0),edgeCount(0),error(0){root[0]=0;for(unsigned int i=0;i<256;i++)records[i]=0;}
 ~Cache(){Clear();}
 void Clear(){for(unsigned int i=0;i<count;i++)records[i]->~Record();memory.Clear();count=edgeCount=0;error=0;root[0]=0;}
 bool Initialize(const char* resourceRoot){
  Clear();if(!resourceRoot||!PrefabScene::Full(resourceRoot,root)){error="Invalid resource root";return false;}
  unsigned int length=ShaderDependencyPaths::Length(root);if(!length||length+2>=264){error="Resource root exceeds bound";return false;}
  if(root[length-1]!='\\'&&root[length-1]!='/'){root[length++]='\\';root[length]=0;}return true;
 }
 const Record* Get(unsigned int identity)const{return identity<count?records[identity]:0;}
 bool Resolve(const char* key,unsigned int* identity){
  error=0;if(!identity||!key||!root[0]){error="Invalid shader key/cache";return false;}*identity=0xffffffffu;
  unsigned int length=ShaderDependencyPaths::Length(key);if(length>=264){error="Shader key exceeds bound";return false;}
  for(unsigned int i=0;i<length;i++)if((unsigned char)key[i]>=128){error="Non-ASCII shader cache identity unsupported";return false;}
  unsigned int slot=0;for(;slot<count;slot++)if(Equal(records[slot]->key,key))break;
  if(slot==count){if(count>=256){error="Shader cache bound";return false;}void* recordStorage=memory.Allocate(sizeof(Record));if(!recordStorage){error="Shader record allocation failed";return false;}records[count++]=new(recordStorage)Record();lstrcpyA(records[slot]->key,key);}
  *identity=slot;Record& record=*records[slot];
  if(record.state==Ready)return true;
  if(record.state==Loading){error="Cyclic shader dependencies are unsupported";return false;}
  // Failed loads are not successful cache entries. Retrying them is permitted,
  // so changes to a previously missing shader can be observed by this cache.
  record.ClearCompiler();record.instances.Clear();record.state=Loading;record.error=0;record.count=record.warnings=0;record.includeOpens=record.includeMaximumDepth=0;
  if(!record.includeMemory)record.includeMemory=memory.Allocate(sizeof(ShaderEffectInclude::Resolver));if(!record.includeMemory){record.state=Unsupported;record.error=error="Include resolver allocation failed";return false;}
  record.include=new(record.includeMemory)ShaderEffectInclude::Resolver(root);
  bool primary=record.primary.ReadPrimary(record.key,record.include);record.includeOpens=record.include->opens;record.includeMaximumDepth=record.include->maximumDepth;const char* includeError=record.include->error;
  if(!primary){record.state=record.include->unsupported||record.primary.unsupported?Unsupported:Failed;record.error=includeError?includeError:record.primary.error;if(record.state==Unsupported){error=record.error;return false;}return true;}
  if(!record.instances.Reflect(record.primary.effect)){record.state=Unsupported;record.error=error=record.instances.error;return false;}
  record.count=record.primary.count;record.warnings=record.primary.warnings;for(unsigned int i=0;i<record.count;i++)record.elements[i]=record.primary.elements[i];
  Walk walk={this,slot,{0}};if(!ShaderDependencyPaths::Directory(record.key,walk.directory,264)){record.state=Unsupported;record.error=error="Shader directory exceeds bound";return false;}
  bool boundExceeded=false;bool walked=Enumerate(record.primary.effect,Visit,&walk,&boundExceeded);
  if(boundExceeded){record.state=Unsupported;record.error="Dependency reflection bound";}
  if(!walked){if(record.state==Loading){record.state=Failed;record.error="Invalid shader dependency metadata";}if(record.state==Unsupported){error=record.error;return false;}return true;}
  if(!record.instances.Finish()){record.state=Unsupported;record.error=error=record.instances.error;return false;}
  record.state=Ready;return true;
 }
};
}
