#pragma once
#include "NativePreparedRenderScene.h"
#include "NativePhysicsOwnerMeshPrefix.h"
#include "NativePhysicsProperties.h"
#include "WorldModelSharing.h"
// Owned source-prefix catalog and bounded association framing. Prefix and MOPP
// contents are validated by their actual compiler/backend consumers, not here.
namespace NativePhysicsOwnerMeshTransport {
enum {CatalogMagic=0x31434d50u,AssociationMagic=0x31414d50u,Version=1,MaximumBytes=16u*1024u*1024u,MaximumRecords=4095};
enum ErrorCode {NoError=0,InvalidInput,InvalidSettings,InvalidScene,InvalidHeader,ResourceLimit,TruncatedPacket,TrailingBytes,InvalidIdentity,DuplicateIdentity,AllocationFailure,SourcePrefixFailure,InternalInvariant};
struct Association {unsigned int group,modelIndex;const void* mesh;unsigned int meshBytes;const void* code;unsigned int codeBytes;};
struct Record {Record* next;unsigned int group,modelIndex;const void* mesh;unsigned int meshBytes;};
struct Settings {
 unsigned int version,maximumBytes,maximumRecords;NativePhysicsOwnerMeshPrefix::Settings prefix;
 Settings():version(1),maximumBytes(MaximumBytes),maximumRecords(MaximumRecords){}
};
namespace detail {
inline bool Equal(const void* a,const void* b,unsigned int n){const unsigned char* x=(const unsigned char*)a,*y=(const unsigned char*)b;for(unsigned int i=0;i<n;i++)if(x[i]!=y[i])return false;return true;}
inline void Copy(void* target,const void* source,unsigned int n){unsigned char* to=(unsigned char*)target;const unsigned char* from=(const unsigned char*)source;for(unsigned int i=0;i<n;i++)to[i]=from[i];}
inline unsigned int Word(const unsigned char* p){return (unsigned int)p[0]|((unsigned int)p[1]<<8)|((unsigned int)p[2]<<16)|((unsigned int)p[3]<<24);}
inline void PutWord(unsigned char* p,unsigned int v){p[0]=(unsigned char)v;p[1]=(unsigned char)(v>>8);p[2]=(unsigned char)(v>>16);p[3]=(unsigned char)(v>>24);}
inline bool Overlap(const void* a,unsigned int an,const void* b,unsigned int bn){if(!a||!an||!b||!bn)return false;unsigned long long x=(unsigned int)a,y=(unsigned int)b;return x<y+bn&&y<x+an;}
}
class Catalog {
 PrefabScene::Arena memory;
 Catalog(const Catalog&);Catalog& operator=(const Catalog&);
 bool Fail(ErrorCode code,const char* message,unsigned int group=0xffffffffu){char saved[512];lstrcpynA(saved,message?message:"Owner Mesh catalog failed",512);Clear();lstrcpyA(diagnostic,saved);error=diagnostic;errorCode=code;failedGroup=group;return false;}
 bool Serialize(){
  data=(unsigned char*)memory.Allocate(size);if(!data)return Fail(AllocationFailure,"Owner Mesh catalog packet allocation failed");
  detail::PutWord(data,CatalogMagic);detail::PutWord(data+4,Version);detail::PutWord(data+8,count);unsigned int at=12,seen=0;
  for(const Record* r=first;r;r=r->next){if(seen==count||at>size||12>size-at||r->meshBytes>size-at-12)return Fail(InternalInvariant,"Owner Mesh catalog serialization mismatch");detail::PutWord(data+at,r->group);detail::PutWord(data+at+4,r->modelIndex);detail::PutWord(data+at+8,r->meshBytes);at+=12;detail::Copy(data+at,r->mesh,r->meshBytes);at+=r->meshBytes;seen++;}
  if(at!=size||seen!=count)return Fail(InternalInvariant,"Owner Mesh catalog final framing mismatch");return true;
 }
public:
 Record* first;Record* last;unsigned char* data;unsigned int count,size,totalBytes,retainedModelCount,failedGroup;ErrorCode errorCode;const char* error;char diagnostic[512];
 Catalog():first(0),last(0),data(0),count(0),size(0),totalBytes(0),retainedModelCount(0),failedGroup(0xffffffffu),errorCode(NoError),error(0){diagnostic[0]=0;}
 void Clear(){memory.Clear();first=last=0;data=0;count=size=totalBytes=retainedModelCount=0;failedGroup=0xffffffffu;errorCode=NoError;error=0;diagnostic[0]=0;}
 bool Build(const NativePreparedRenderScene::Context& scene,const Settings& settings=Settings()){
  Clear();if(settings.version!=Version||settings.maximumBytes<12||settings.maximumBytes>MaximumBytes||!settings.maximumRecords||settings.maximumRecords>MaximumRecords)return Fail(InvalidSettings,"Invalid owner Mesh catalog limits");
  if(!scene.Ready())return Fail(InvalidScene,"Owner Mesh catalog requires a prepared source scene");
  const PrefabScene::Model& model=scene.Model();const WorldModels::Partition& partition=scene.Partition();WorldModelSharing::Identity sharing;if(!sharing.Build(model,partition))return Fail(InvalidScene,"Owner Mesh catalog sharing failed");
  size=12;unsigned int modelIndex=1;
  for(unsigned int g=1;g<partition.groupCount;g++){
   if(!SourcePhysicalBspContours::RetainedGroup(model,partition,g)||sharing.representative[g]!=g)continue;
   if(retainedModelCount==MaximumRecords)return Fail(ResourceLimit,"Owner Mesh catalog retained owner limit",g);retainedModelCount++;const unsigned int currentIndex=modelIndex++;
   const PrefabScene::Node& owner=*partition.groups[g].owner;if(NativePhysicsProperties::Automatic(owner)!=NativePhysicsProperties::Mesh)continue;
   if(count==settings.maximumRecords)return Fail(ResourceLimit,"Owner Mesh catalog record limit",g);
   float mass=0,density=0;if(!NativePhysicsProperties::MassDensity(owner,true,mass,density))return Fail(SourcePrefixFailure,"Owner Mesh catalog mass/density preparation failed",g);
   NativePhysicsOwnerMeshPrefix::Builder prefix;
   if(!prefix.Build(model,partition,g,scene.Offset(),mass,density,settings.prefix))return Fail(SourcePrefixFailure,prefix.error,g);
   if(!prefix.data||!prefix.size)return Fail(InternalInvariant,"Owner Mesh catalog prefix is empty",g);
   if(size>settings.maximumBytes||12>settings.maximumBytes-size||prefix.size>settings.maximumBytes-size-12)return Fail(ResourceLimit,"Owner Mesh catalog byte limit",g);
   Record* record=(Record*)memory.Allocate(sizeof(Record));void* mesh=memory.Allocate(prefix.size);if(!record||!mesh)return Fail(AllocationFailure,"Owner Mesh catalog owned prefix allocation failed",g);detail::Copy(mesh,prefix.data,prefix.size);
   record->next=0;record->group=g;record->modelIndex=currentIndex;record->mesh=mesh;record->meshBytes=prefix.size;if(last)last->next=record;else first=record;last=record;count++;totalBytes+=prefix.size;size+=12+prefix.size;
  }
  return Serialize();
 }
 bool WriteCatalog(void* target,unsigned int capacity)const{if(!data||error||!target||capacity<size||detail::Overlap(target,size,data,size))return false;detail::Copy(target,data,size);return true;}
};
class Parser {
 PrefabScene::Arena memory;
 Parser(const Parser&);Parser& operator=(const Parser&);
 bool Fail(ErrorCode code,const char* text){Clear();errorCode=code;error=text;return false;}
public:
 unsigned char* data;unsigned int size,count;Association* records;ErrorCode errorCode;const char* error;
 Parser():data(0),size(0),count(0),records(0),errorCode(NoError),error(0){}
 void Clear(){memory.Clear();data=0;size=count=0;records=0;errorCode=NoError;error=0;}
 bool Build(const void* bytes,unsigned int byteCount){
  const bool aliases=detail::Overlap(bytes,byteCount,data,size)||detail::Overlap(bytes,byteCount,records,count*sizeof(Association));Clear();
  if(aliases||!bytes)return Fail(InvalidInput,"Association input is missing or aliases parser storage");
  if(byteCount>MaximumBytes)return Fail(ResourceLimit,"Association packet byte limit");if(byteCount<12)return Fail(TruncatedPacket,"Association packet header is truncated");
  const unsigned char* source=(const unsigned char*)bytes;if(detail::Word(source)!=AssociationMagic||detail::Word(source+4)!=Version)return Fail(InvalidHeader,"Association packet magic/version mismatch");
  const unsigned int wanted=detail::Word(source+8);if(wanted>MaximumRecords)return Fail(ResourceLimit,"Association record limit");if(wanted>(byteCount-12)/18)return Fail(TruncatedPacket,"Association record framing is truncated");
  data=(unsigned char*)memory.Allocate(byteCount);if(!data)return Fail(AllocationFailure,"Association owned packet allocation failed");detail::Copy(data,source,byteCount);size=byteCount;
  if(wanted){records=(Association*)memory.Allocate(wanted*sizeof(Association));if(!records)return Fail(AllocationFailure,"Association record allocation failed");}
  unsigned int at=12;
  for(unsigned int i=0;i<wanted;i++){
   if(at>size||16>size-at)return Fail(TruncatedPacket,"Association record header is truncated");
   const unsigned int group=detail::Word(data+at),index=detail::Word(data+at+4),meshBytes=detail::Word(data+at+8),codeBytes=detail::Word(data+at+12);at+=16;
   if(!group||!index||index>MaximumRecords)return Fail(InvalidIdentity,"Association group must be nonzero and model index must be 1..4095");
   if(!meshBytes||!codeBytes)return Fail(InvalidInput,"Association mesh/code payload must be nonempty");
   if(meshBytes>size-at||codeBytes>size-at-meshBytes)return Fail(TruncatedPacket,"Association payload is truncated");
   for(unsigned int j=0;j<i;j++)if(records[j].group==group||records[j].modelIndex==index)return Fail(DuplicateIdentity,"Duplicate association group or model index");
   Association& r=records[i];r.group=group;r.modelIndex=index;r.meshBytes=meshBytes;r.codeBytes=codeBytes;r.mesh=data+at;at+=meshBytes;r.code=data+at;at+=codeBytes;count++;
  }
  if(at!=size)return Fail(TrailingBytes,"Association packet has trailing bytes");return true;
 }
};
typedef Parser AssociationParser;
}
