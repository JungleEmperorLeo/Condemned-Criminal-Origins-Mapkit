#pragma once
#include "ShaderInstanceParameter.h"
// Remaining instance-parameter load-success metadata, original 0x7BE80,
// parent clone/dedup 0x7BFA0 and texture mapping table 0x7B6E0. This table is
// sufficient for load eligibility; actual material parameter defaults are not
// serialized here. The complete source loader still needs declaration/include
// and ordered dependency closure success in addition to this gate.
namespace ShaderLoaderGate {
struct Parameter {const char* name;unsigned int kind,mapping;};
class Table {
 PrefabScene::Arena memory;
 Table(const Table&);Table& operator=(const Table&);
 bool Reject(const char* message){unsupported=true;error=message;return false;}
 bool Append(const Parameter& source){
  if(count>=1024)return Reject("Instance parameter count exceeds bound");
  if(!source.name)return Reject("Invalid instance parameter name");unsigned int size=0;while(source.name[size]){if(size>=260)return Reject("Instance parameter name exceeds bound");size++;}
  char* name=(char*)memory.Allocate(size+1);if(!name)return Reject("Instance parameter storage allocation failed");lstrcpyA(name,source.name);Parameter copied={name,source.kind,source.mapping};parameters[count++]=copied;return true;
 }
 static bool Equal(const char* a,const char* b){while(*a&&*a==*b){a++;b++;}return *a==*b;}
public:
 unsigned int count,textureCount,counterCount;Parameter parameters[1024];unsigned int* counters;bool unsupported;const char* error;
 Table():count(0),textureCount(0),counterCount(0),counters(0),unsupported(false),error(0){}
 void Clear(){memory.Clear();count=textureCount=counterCount=0;counters=0;unsupported=false;error=0;}
 bool Reflect(void* effect){
  Clear();if(!effect)return Reject("Missing instance reflection effect");
  typedef const char* (WINAPI *Child)(void*,const char*,unsigned int);typedef long (WINAPI *Describe)(void*,const char*,ShaderInstanceParameter::Descriptor*);
  typedef const char* (WINAPI *Annotation)(void*,const char*,const char*);typedef long (WINAPI *Boolean)(void*,const char*,int*);typedef long (WINAPI *Integer)(void*,const char*,int*);
  void** methods=*(void***)effect;
  for(unsigned int i=0;;i++){
   const char* handle=((Child)methods[8])(effect,0,i);if(!handle)return true;if(i>=16384)return Reject("Reflected parameter enumeration exceeds bound");
   ShaderInstanceParameter::Descriptor description={};if(((Describe)methods[4])(effect,handle,&description)<0)continue;unsigned int kind=ShaderInstanceParameter::Classify(description);if(!kind)continue;
   const char* annotation=((Annotation)methods[19])(effect,handle,"InstanceParam");int enabled=0;if(!annotation||((Boolean)methods[23])(effect,annotation,&enabled)<0||!enabled)continue;
   Parameter parameter={(const char*)description.name,kind,0};if(kind==ShaderInstanceParameter::Texture){annotation=((Annotation)methods[19])(effect,handle,"Mapping");int mapping=0;if(annotation&&((Integer)methods[27])(effect,annotation,&mapping)>=0)parameter.mapping=(unsigned int)mapping;}
   if(!Append(parameter))return false;
  }
 }
 bool Merge(const Table& parent){
  if(unsupported||parent.unsupported)return Reject("Unsupported inherited instance parameters");
  for(unsigned int p=0;p<parent.count;p++){bool existing=false;for(unsigned int c=0;c<count;c++)if(parameters[c].kind==parent.parameters[p].kind&&Equal(parameters[c].name,parent.parameters[p].name)){existing=true;break;}if(!existing&&!Append(parent.parameters[p]))return false;}return true;
 }
 bool Finish(){
  if(unsupported)return false;unsigned int maximum=0;textureCount=0;
  for(unsigned int i=0;i<count;i++)if(parameters[i].kind==ShaderInstanceParameter::Texture){unsigned int mapping=parameters[i].mapping;if(mapping>=65536)return Reject("Texture Mapping exceeds safe counter bound");if(mapping>maximum)maximum=mapping;textureCount++;}
  counterCount=maximum+1;counters=(unsigned int*)memory.Allocate(counterCount*4);if(!counters)return Reject("Texture mapping table allocation failed");for(unsigned int i=0;i<counterCount;i++)counters[i]=0;
  for(unsigned int i=0;i<count;i++)if(parameters[i].kind==ShaderInstanceParameter::Texture)counters[parameters[i].mapping]++;return true;
 }
};
}
