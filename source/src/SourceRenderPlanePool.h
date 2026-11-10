#pragma once
#include "SourceRenderPlaneKey.h"
namespace SourceRenderPlanePool {
// The original surface preparation interns normals by three signed quantized
// components, retaining the first normal for a key. Distance is not part of
// the key and remains a separate surface value. Input is the prepared plane.
class Pool {
 struct Entry {SourceRenderPlaneKey::Key key;WorldText::Vec3 normal;unsigned int index;bool used;};
 PrefabScene::Arena memory;Entry* entries;unsigned int capacity,count,maximum;
public:
 Pool():entries(0),capacity(0),count(0),maximum(0){}
 bool Initialize(unsigned int expected){
  memory.Clear();entries=0;capacity=count=maximum=0;if(!expected||expected>1000000)return false;
  unsigned int size=2;while(size<expected*2)size*=2;
  entries=(Entry*)memory.Allocate(size*sizeof(Entry));if(!entries)return false;capacity=size;maximum=expected;return true;
 }
 bool Get(const WorldText::Vec3& input,WorldText::Vec3& output,unsigned int& index){
  if(!entries)return false;SourceRenderPlaneKey::Key key;if(!SourceRenderPlaneKey::Build(input,key))return false;
  unsigned int hash=(unsigned int)key.component[0]*73856093u^(unsigned int)key.component[1]*19349663u^(unsigned int)key.component[2]*83492791u;
  for(unsigned int probe=0;probe<capacity;probe++){
   Entry& entry=entries[(hash+probe)&(capacity-1)];
   if(!entry.used){if(count==maximum)return false;entry.key=key;entry.normal=input;entry.index=count;entry.used=true;count++;output=input;index=entry.index;return true;}
   if(SourceRenderPlaneKey::Equal(entry.key,key)){output=entry.normal;index=entry.index;return true;}
  }
  return false;
 }
 bool Get(const WorldText::Vec3& input,WorldText::Vec3& output){unsigned int index=0;return Get(input,output,index);}
 unsigned int Count()const{return count;}
 // The 18 axis/diagonal seeds the original inserts before surface normals
 // (the diagonal is the unnormalized 0x3fb504f3 of the original table).
 bool Seed(){
  float diagonal;unsigned int bits=0x3fb504f3u;memcpy(&diagonal,&bits,4);
  const WorldText::Vec3 seeds[18]={{0,0,1},{0,0,-1},{0,1,0},{0,-1,0},{1,0,0},{-1,0,0},{0,diagonal,diagonal},{0,diagonal,-diagonal},{0,-diagonal,-diagonal},{0,-diagonal,diagonal},{diagonal,0,diagonal},{diagonal,0,-diagonal},{-diagonal,0,-diagonal},{-diagonal,0,diagonal},{diagonal,diagonal,0},{diagonal,-diagonal,0},{-diagonal,-diagonal,0},{-diagonal,diagonal,0}};
  WorldText::Vec3 unused;unsigned int index=0;for(unsigned int i=0;i<18;i++)if(!Get(seeds[i],unused,index)||index!=i)return false;return true;
 }
 bool Ordered(WorldText::Vec3* output,unsigned int outputCapacity)const{
  if(!output||outputCapacity<count)return false;for(unsigned int i=0;i<capacity;i++)if(entries[i].used)output[entries[i].index]=entries[i].normal;return true;
 }
 // Diagnostic alias contours can reuse a representative's normal, but a
 // skipped prototype must not introduce a new global normal-table entry.
 bool LookupOrOriginal(const WorldText::Vec3& input,WorldText::Vec3& output)const{
  if(!entries)return false;SourceRenderPlaneKey::Key key;if(!SourceRenderPlaneKey::Build(input,key))return false;
  unsigned int hash=(unsigned int)key.component[0]*73856093u^(unsigned int)key.component[1]*19349663u^(unsigned int)key.component[2]*83492791u;
  for(unsigned int probe=0;probe<capacity;probe++){
   const Entry& entry=entries[(hash+probe)&(capacity-1)];
   if(!entry.used){output=input;return true;}
   if(SourceRenderPlaneKey::Equal(entry.key,key)){output=entry.normal;return true;}
  }
  return false;
 }
};
}
