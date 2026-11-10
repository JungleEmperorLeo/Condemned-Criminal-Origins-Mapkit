#pragma once
#include "win_min.h"

// Structural storage only. Producers supply source-generated typed bytes and
// consumers explicitly encode an ordinal for their own runtime property type.
namespace NativeBlindDataRegistry {
enum {Version=1,MaximumBytes=16u*1024u*1024u,MaximumRecords=100000,MaximumBindings=100000,MaximumPropertyBytes=255};
enum ErrorCode {NoError=0,InvalidSettings,InvalidInput,InvalidType,InvalidBinding,DuplicateBinding,ResourceLimit,InputAlias,AllocationFailure};
struct RecordInput {unsigned int typeId;const void* data;unsigned int bytes;};
struct BindingInput {unsigned int sourceNode;const char* property;unsigned int ordinal,typeId;};
struct Record {unsigned int typeId;const void* data;unsigned int bytes,start;};
struct Binding {unsigned int sourceNode;const char* property;unsigned int ordinal,typeId;};
struct Settings {
 unsigned int version,maximumBytes,maximumRecords,maximumBindings;
 Settings():version(Version),maximumBytes(MaximumBytes),maximumRecords(MaximumRecords),maximumBindings(MaximumBindings){}
};
namespace detail {
inline void Copy(void* target,const void* source,unsigned int bytes){unsigned char* t=(unsigned char*)target;const unsigned char* s=(const unsigned char*)source;for(unsigned int i=0;i<bytes;i++)t[i]=s[i];}
inline void Put(unsigned char* p,unsigned int value){p[0]=(unsigned char)value;p[1]=(unsigned char)(value>>8);p[2]=(unsigned char)(value>>16);p[3]=(unsigned char)(value>>24);}
inline unsigned int Get(const unsigned char* p){return (unsigned int)p[0]|((unsigned int)p[1]<<8)|((unsigned int)p[2]<<16)|((unsigned int)p[3]<<24);}
inline bool Overlap(const void* a,unsigned int an,const void* b,unsigned int bn){if(!a||!an||!b||!bn)return false;const unsigned long long x=(unsigned long long)a,y=(unsigned long long)b;return x<y+bn&&y<x+an;}
inline bool Equal(const char* a,const char* b){if(!a||!b)return false;for(unsigned int i=0;i<=MaximumPropertyBytes;i++){if(a[i]!=b[i])return false;if(!a[i])return true;}return false;}
inline unsigned int Length(const char* name){if(!name)return 0;for(unsigned int i=0;i<=MaximumPropertyBytes;i++)if(!name[i])return i;return MaximumPropertyBytes+1;}
inline unsigned int Hash(unsigned int node,const char* property){unsigned int h=2166136261u;for(unsigned int shift=0;shift<32;shift+=8)h=(h^((node>>shift)&255))*16777619u;for(unsigned int i=0;property[i];i++)h=(h^(unsigned char)property[i])*16777619u;return h;}
inline void* Allocate(unsigned int bytes){return bytes?VirtualAlloc(0,bytes,MEM_COMMIT|MEM_RESERVE,4):0;}
inline void Release(void* p){if(p)VirtualFree(p,0,MEM_RELEASE);}
}

class Registry {
 unsigned char* packet;Record* recordData;Binding* bindingData;char* names;unsigned int* lookup;
 unsigned int packetBytes,recordCount,bindingCount,namesBytes,lookupCount,ownedBytes;
 Registry(const Registry&);Registry& operator=(const Registry&);
 bool Aliases(const void* p,unsigned int n)const{
  return detail::Overlap(p,n,packet,packetBytes)||detail::Overlap(p,n,recordData,recordCount*sizeof(Record))||detail::Overlap(p,n,bindingData,bindingCount*sizeof(Binding))||detail::Overlap(p,n,names,namesBytes)||detail::Overlap(p,n,lookup,lookupCount*sizeof(unsigned int));
 }
 bool Fail(ErrorCode code,const char* message,unsigned int index=0xffffffffu){Clear();errorCode=code;error=message;failedIndex=index;return false;}
public:
 ErrorCode errorCode;const char* error;unsigned int failedIndex;
 Registry():packet(0),recordData(0),bindingData(0),names(0),lookup(0),packetBytes(0),recordCount(0),bindingCount(0),namesBytes(0),lookupCount(0),ownedBytes(0),errorCode(NoError),error(0),failedIndex(0xffffffffu){}
 ~Registry(){Clear();}
 void Clear(){detail::Release(packet);detail::Release(recordData);detail::Release(bindingData);detail::Release(names);detail::Release(lookup);packet=0;recordData=0;bindingData=0;names=0;lookup=0;packetBytes=recordCount=bindingCount=namesBytes=lookupCount=ownedBytes=0;errorCode=NoError;error=0;failedIndex=0xffffffffu;}
 const unsigned char* Data()const{return packet;}
 unsigned int Size()const{return packetBytes;}
 const Record* Records()const{return recordData;}
 unsigned int Count()const{return recordCount;}
 const Binding* Bindings()const{return bindingData;}
 unsigned int BindingCount()const{return bindingCount;}
 unsigned int OwnedBytes()const{return ownedBytes;}
 const Binding* FindBinding(unsigned int sourceNode,const char* property)const{
  const unsigned int length=detail::Length(property);if(error||!lookupCount||!length||length>MaximumPropertyBytes)return 0;
  unsigned int slot=detail::Hash(sourceNode,property)&(lookupCount-1);
  for(unsigned int tried=0;tried<lookupCount;tried++,slot=(slot+1)&(lookupCount-1)){const unsigned int entry=lookup[slot];if(!entry)return 0;const Binding& binding=bindingData[entry-1];if(binding.sourceNode==sourceNode&&detail::Equal(binding.property,property))return &binding;}return 0;
 }
 bool Build(const RecordInput* records,unsigned int count,const BindingInput* bindings=0,unsigned int bindingsCount=0,const Settings& settings=Settings()){
  // Inspect aliases while the previous successful storage is still live.
  if(settings.version!=Version||settings.maximumBytes<8||settings.maximumBytes>MaximumBytes||!settings.maximumRecords||settings.maximumRecords>MaximumRecords||!settings.maximumBindings||settings.maximumBindings>MaximumBindings)return Fail(InvalidSettings,"Invalid BlindData registry limits");
  if(count>settings.maximumRecords||bindingsCount>settings.maximumBindings)return Fail(ResourceLimit,"BlindData record or binding count limit");
  if((count&&!records)||(bindingsCount&&!bindings))return Fail(InvalidInput,"Missing BlindData input array");
  if(Aliases(records,count*sizeof(RecordInput))||Aliases(bindings,bindingsCount*sizeof(BindingInput)))return Fail(InputAlias,"BlindData input array aliases owned registry storage");
  unsigned int payload=0,stringBytes=0;
  for(unsigned int i=0;i<count;i++){
   const RecordInput& record=records[i];if(!record.typeId)return Fail(InvalidType,"BlindData type ID must be nonzero",i);
   if(record.bytes>settings.maximumBytes-payload)return Fail(ResourceLimit,"BlindData payload byte limit",i);
   if(record.bytes&&!record.data)return Fail(InvalidInput,"Missing BlindData record payload",i);
   if(Aliases(record.data,record.bytes))return Fail(InputAlias,"BlindData payload aliases owned registry storage",i);payload+=record.bytes;
   // Original writer 0x10051820..0x10051929: every record, including
   // the last, occupies ALIGN4(bytes). Table sizes remain unpadded.
   const unsigned int padding=(0u-record.bytes)&3u;
   if(padding>settings.maximumBytes-payload)return Fail(ResourceLimit,"BlindData alignment byte limit",i);payload+=padding;
  }
  for(unsigned int i=0;i<bindingsCount;i++){
   const BindingInput& binding=bindings[i];if(Aliases(binding.property,MaximumPropertyBytes+1))return Fail(InputAlias,"BlindData property aliases owned registry storage",i);
   const unsigned int length=detail::Length(binding.property);
   if(binding.sourceNode==0xffffffffu||!length||length>MaximumPropertyBytes||binding.ordinal>=count||!binding.typeId||binding.typeId!=records[binding.ordinal].typeId)return Fail(InvalidBinding,"Invalid BlindData source/property/ordinal/type binding",i);
   if(length+1>settings.maximumBytes-stringBytes)return Fail(ResourceLimit,"BlindData property storage limit",i);stringBytes+=length+1;
  }
  const unsigned int tableBytes=count*12u;
  if(tableBytes>settings.maximumBytes-8||payload>settings.maximumBytes-8-tableBytes)return Fail(ResourceLimit,"BlindData serialized byte limit");
  const unsigned int serialized=8+payload+tableBytes;
  unsigned int slots=0;if(bindingsCount){slots=2;while(slots<bindingsCount*2u)slots*=2u;}
  const unsigned int metadata=count*sizeof(Record)+bindingsCount*sizeof(Binding)+slots*sizeof(unsigned int);
  if(metadata>settings.maximumBytes-serialized||stringBytes>settings.maximumBytes-serialized-metadata)return Fail(ResourceLimit,"BlindData total owned storage limit");
  Clear();packetBytes=serialized;recordCount=count;bindingCount=bindingsCount;namesBytes=stringBytes;lookupCount=slots;ownedBytes=serialized+metadata+stringBytes;
  packet=(unsigned char*)detail::Allocate(packetBytes);recordData=(Record*)detail::Allocate(recordCount*sizeof(Record));bindingData=(Binding*)detail::Allocate(bindingCount*sizeof(Binding));names=(char*)detail::Allocate(namesBytes);lookup=(unsigned int*)detail::Allocate(lookupCount*sizeof(unsigned int));
  if(!packet||(recordCount&&!recordData)||(bindingCount&&(!bindingData||!names||!lookup)))return Fail(AllocationFailure,"BlindData owned storage allocation failed");
  detail::Put(packet,count);detail::Put(packet+4,payload);unsigned int at=0;
  for(unsigned int i=0;i<count;i++){
   const RecordInput& input=records[i];Record& record=recordData[i];record.typeId=input.typeId;record.bytes=input.bytes;record.start=at;record.data=packet+8+at;detail::Copy(packet+8+at,input.data,input.bytes);
   const unsigned int padding=(0u-input.bytes)&3u;for(unsigned int j=0;j<padding;j++)packet[8+at+input.bytes+j]=0;
   unsigned char* row=packet+8+payload+i*12u;detail::Put(row,input.bytes);detail::Put(row+4,input.typeId);detail::Put(row+8,at);at+=input.bytes+padding;
  }
  at=0;
  for(unsigned int i=0;i<bindingsCount;i++){
   const BindingInput& input=bindings[i];Binding& binding=bindingData[i];binding.sourceNode=input.sourceNode;binding.ordinal=input.ordinal;binding.typeId=input.typeId;binding.property=names+at;
   const unsigned int length=detail::Length(input.property);detail::Copy(names+at,input.property,length+1);at+=length+1;
   unsigned int slot=detail::Hash(binding.sourceNode,binding.property)&(lookupCount-1);
   while(lookup[slot]){const Binding& previous=bindingData[lookup[slot]-1];if(previous.sourceNode==binding.sourceNode&&detail::Equal(previous.property,binding.property))return Fail(DuplicateBinding,"Duplicate BlindData source/property binding",i);slot=(slot+1)&(lookupCount-1);}lookup[slot]=i+1;
  }
  return true;
 }
 bool Write(void* target,unsigned int capacity)const{if(error||!packet||!target||capacity<packetBytes||Aliases(target,packetBytes))return false;detail::Copy(target,packet,packetBytes);return true;}
};
}
