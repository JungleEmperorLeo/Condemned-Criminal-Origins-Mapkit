#pragma once
#include "MaterialRenderLayout.h"
// Compile-local source resource adapter. Materials and copied declarations live
// until Clear/Initialize/destruction; no effects or temporary Material pointers
// escape the cache. Shader identity remains filename text, not declaration bytes.
namespace NativeMaterialLayouts {
static const unsigned int MaximumLayouts=16384;
static const unsigned int BucketCount=32768;
enum State {Ready=0,Failed=1,Unsupported=2};
enum Code {Ok=0,InvalidArgument=1,InvalidPath=2,MaterialUnavailable=3,
 MaterialMalformed=4,ShaderUnsupported=5,Capacity=6,Allocation=7,
 Cancelled=8,Reentrant=9};
typedef bool (*Cancel)(void*); // true requests cancellation at a boundary.
struct Layout {
 const char* material;const char* shader;
 const RenderVertexPacking::Element* declaration;
 unsigned int count,warnings,shaderIdentity,surfaceFlags;long compilerResult;
 State state;
};
struct Diagnostic {Code code;char material[260],resource[260],message[512];};
class Cache {
 struct Record {Layout layout;char name[260],shader[264];RenderVertexPacking::Element elements[64];};
 PrefabScene::Arena memory;ShaderDeclarationClosure::Cache shaders;
 Record** records;unsigned int* buckets;char root[260];unsigned int count,allocated,budget;
 Cancel cancel;void* cancelContext;bool initialized,active;
 Diagnostic diagnostic;
 Cache(const Cache&);Cache& operator=(const Cache&);
 static unsigned char Lower(unsigned char c){return c>='A'&&c<='Z'?(unsigned char)(c+32):c;}
 static bool Equal(const char* a,const char* b){while(*a&&Lower((unsigned char)*a)==Lower((unsigned char)*b)){a++;b++;}return Lower((unsigned char)*a)==Lower((unsigned char)*b);}
 static unsigned int Hash(const char* name){unsigned int h=2166136261u;while(*name){h=(h^Lower((unsigned char)*name++))*16777619u;}return h;}
 static bool Contains(const char* text,const char* token){if(!text)return false;for(;*text;text++){const char* p=text;const char* q=token;while(*q&&*p==*q){p++;q++;}if(!*q)return true;}return false;}
 void ResetDiagnostic(){diagnostic.code=Ok;diagnostic.material[0]=diagnostic.resource[0]=diagnostic.message[0]=0;}
 bool Fail(Code code,const char* message,const char* material=0,const char* resource=0){diagnostic.code=code;lstrcpynA(diagnostic.message,message?message:"Material layout failure",512);lstrcpynA(diagnostic.material,material?material:"",260);lstrcpynA(diagnostic.resource,resource?resource:"",260);return false;}
 bool Boundary(const char* material=0){return !(cancel&&cancel(cancelContext))||Fail(Cancelled,"Material layout compilation cancelled",material);}
 void* Allocate(unsigned int bytes){if(bytes>budget-allocated)return 0;void* p=memory.Allocate(bytes);if(p)allocated+=bytes;return p;}
 static bool Canonical(const char* input,char* output){
  if(!input||!output)return false;while(*input=='/'||*input=='\\')input++;
  unsigned int n=0;for(;input[n];n++){unsigned char c=(unsigned char)input[n];if(n>=259||c>=128||c<32||c==':')return false;output[n]=(char)c;}
  output[n]=0;return n!=0;
 }
 bool Resource(const char* name,char* path){
  if(!PrefabScene::Join(root,name,path))return false;
  unsigned int n=lstrlenA(root);for(unsigned int i=0;i<n;i++)if(!path[i]||Lower((unsigned char)root[i])!=Lower((unsigned char)path[i]))return false;
  // root always includes a trailing separator; sibling prefixes cannot match.
  return path[n]!=0;
 }
 struct ResolveScope {bool& active;ResolveScope(bool& value):active(value){active=true;}~ResolveScope(){active=false;}};
public:
 Cache():records(0),buckets(0),count(0),allocated(0),budget(0),cancel(0),cancelContext(0),initialized(false),active(false){root[0]=0;ResetDiagnostic();}
 ~Cache(){Clear();}
 void Clear(){shaders.Clear();memory.Clear();records=0;buckets=0;root[0]=0;count=allocated=budget=0;cancel=0;cancelContext=0;initialized=active=false;ResetDiagnostic();}
 // budget is this adapter's copied material/layout storage. Shader frontend
 // allocations remain subject to the existing shader cache/include bounds.
 bool Initialize(const char* resourceRoot,Cancel cancellation=0,void* context=0,unsigned int ownedBudget=32u*1024u*1024u){
  Clear();cancel=cancellation;cancelContext=context;budget=ownedBudget;
  if(!resourceRoot||!PrefabScene::Full(resourceRoot,root))return Fail(InvalidPath,"Invalid material resource root");
  unsigned int length=lstrlenA(root);if(!length||length+2>=260)return Fail(InvalidPath,"Material resource root exceeds bound");
  if(root[length-1]!='\\'&&root[length-1]!='/'){root[length++]='\\';root[length]=0;}
  if(!Boundary())return false;
  records=(Record**)Allocate(MaximumLayouts*sizeof(Record*));buckets=(unsigned int*)Allocate(BucketCount*4);
  if(!records||!buckets)return Fail(Allocation,"Material cache allocation failed");
  for(unsigned int i=0;i<BucketCount;i++)buckets[i]=0xffffffffu;
  if(!shaders.Initialize(root))return Fail(InvalidPath,shaders.error);
  initialized=true;return true;
 }
 const Diagnostic& Error()const{return diagnostic;}
 unsigned int Count()const{return count;}
 unsigned int ShaderCount()const{return shaders.count;}
 unsigned int ShaderEdges()const{return shaders.edgeCount;}
 unsigned int OwnedBytes()const{return memory.Capacity();}
 const Layout* Get(unsigned int index)const{return index<count?&records[index]->layout:0;}
 bool Resolve(const char* virtualMaterialName,const Layout** output){
  if(output)*output=0;
  if(active)return Fail(Reentrant,"Reentrant material layout resolution is unsupported",virtualMaterialName);
  ResetDiagnostic();if(!initialized||!output)return Fail(InvalidArgument,"Uninitialized material layout cache or invalid output",virtualMaterialName);
  ResolveScope scope(active);if(!Boundary(virtualMaterialName))return false;
  char name[260],path[260];if(!Canonical(virtualMaterialName,name)||!Resource(name,path))return Fail(InvalidPath,"Invalid material resource path",virtualMaterialName);
  unsigned int bucket=Hash(name)&(BucketCount-1);while(buckets[bucket]!=0xffffffffu){Record* record=records[buckets[bucket]];if(Equal(record->name,name)){*output=&record->layout;return true;}bucket=(bucket+1)&(BucketCount-1);}
  if(count>=MaximumLayouts)return Fail(Capacity,"Material layout count exceeds bound",name,path);
  HANDLE file=CreateFileA(path,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);
  if(file==INVALID_HANDLE_VALUE){
   // An absent Mat00 loads no shader: the packed writer's NULL Shader* fallback
   // uses the Working declaration (SDK SpillKill Wood_SL001.Mat00, stride 84).
   Record* record=(Record*)Allocate(sizeof(Record));if(!record)return Fail(Allocation,"Copied material layout allocation failed",name,path);
   lstrcpyA(record->name,name);record->shader[0]=0;for(unsigned int i=0;i<10;i++)record->elements[i]=RenderVertexPacking::Working[i];
   Layout& layout=record->layout;layout.material=record->name;layout.shader=record->shader;layout.declaration=record->elements;layout.count=10;layout.warnings=0;
   layout.shaderIdentity=0xffffffffu;layout.surfaceFlags=0;layout.compilerResult=0;layout.state=Failed;
   records[count]=record;buckets[bucket]=count++;*output=&layout;return true;
  }
  DWORD high=0,size=GetFileSize(file,&high),done=0;if(high||size==0xffffffffu||size>16u*1024u*1024u){CloseHandle(file);return Fail(MaterialMalformed,"Unsupported material resource size",name,path);}
  PrefabScene::Arena temporary;unsigned char* bytes=(unsigned char*)temporary.Allocate(size?size:1);
  if(!bytes){CloseHandle(file);return Fail(Allocation,"Material input allocation failed",name,path);}
  bool read=!size||ReadFile(file,bytes,size,&done,0)&&done==size;CloseHandle(file);
  if(!read)return Fail(MaterialUnavailable,"Material resource read failed",name,path);
  if(!Boundary(name))return false;
  MaterialResources::Material material;if(!material.Read(bytes,size))return Fail(Contains(material.error,"allocation")?Allocation:MaterialMalformed,material.error,name,path);
  MaterialRenderLayout::Layout resolved;if(!MaterialRenderLayout::Resolve(shaders,material,&resolved))return Fail(Contains(shaders.error,"allocation")?Allocation:ShaderUnsupported,shaders.error?shaders.error:"Unsupported shader dependency closure",name,path);
  if(resolved.compilerResult==(long)0x8007000eu)return Fail(Allocation,"Shader frontend ran out of memory",name,path);
  if(!resolved.declaration||!resolved.count||resolved.count>64)return Fail(ShaderUnsupported,"Invalid resolved shader declaration",name,path);
  if(!Boundary(name))return false;
  Record* record=(Record*)Allocate(sizeof(Record));if(!record)return Fail(Allocation,"Copied material layout allocation failed",name,path);
  lstrcpyA(record->name,name);lstrcpyA(record->shader,material.shader);for(unsigned int i=0;i<resolved.count;i++)record->elements[i]=resolved.declaration[i];
  Layout& layout=record->layout;layout.material=record->name;layout.shader=record->shader;layout.declaration=record->elements;layout.count=resolved.count;layout.warnings=resolved.warnings;layout.shaderIdentity=resolved.identity;layout.surfaceFlags=material.surfaceFlags;layout.compilerResult=resolved.compilerResult;layout.state=resolved.shader.state==RenderShadowSelection::Failed?Failed:Ready;
  records[count]=record;buckets[bucket]=count++;*output=&layout;return true;
 }
 static bool Callback(void* context,const char* material,const Layout** output){return context&&((Cache*)context)->Resolve(material,output);}
};
}
