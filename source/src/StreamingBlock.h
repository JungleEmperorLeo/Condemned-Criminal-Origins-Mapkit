#pragma once
namespace StreamingBlock {
enum HeaderField {
 StringPoolBytes=0,GlobalRegionMemoryLimitBytes=1,StreamingMemoryLimitBytes=2,
 RegionCount=3,TotalAssetReferences=4,ConvexRegionCount=5,ConvexPolygonCount=6,
 ConvexVertexCount=7,RegionTreeNodeCount=8,SelectedConvexVolumeIndex=9,RegionAggregate=10
};
struct Asset {const char* folder;const char* file;};
class Block {
 bool Fail(const char* text){memory.Clear();data=0;size=0;error=text;return false;}
public:
 PrefabScene::Arena memory;unsigned char* data;unsigned int size;const char* error;
 Block():data(0),size(0),error(0){}
 bool BuildGlobal(const Asset* assets,unsigned int count){
  memory.Clear();data=0;size=0;error=0;
  if(count>100000||(!assets&&count))return Fail("Invalid asset list");
  // First occurrence, exact case-sensitive strings; each entry aligned to four.
  const char** strings=(const char**)memory.Allocate((count*2+1)*sizeof(char*));
  unsigned int* offsets=(unsigned int*)memory.Allocate((count*2+1)*4);
  unsigned int* references=(unsigned int*)memory.Allocate((count*2+1)*4);
  if(!strings||!offsets||!references)return Fail("Streaming allocation failed");
  unsigned int unique=0,pool=0;
  for(unsigned int i=0;i<count*2;i++){
   const char* value=(i&1)?assets[i/2].file:assets[i/2].folder;
   if(!value||(i&1&&!value[0]))return Fail("Invalid resource string");
   unsigned int length=0;while(value[length]&&length<=4096)length++;
   if(length>4096)return Fail("Resource string too long");
   unsigned int found=0;
   for(;found<unique;found++){unsigned int j=0;while(value[j]&&value[j]==strings[found][j])j++;if(value[j]==strings[found][j])break;}
   if(found==unique){unsigned int bytes=(length+4)&~3u;if(pool>16*1024*1024-bytes)return Fail("String pool too large");strings[unique]=value;offsets[unique]=pool;pool+=bytes;unique++;}
   references[i]=offsets[found];
  }
  size=48+pool+count*8;if(size>16*1024*1024)return Fail("Streaming block too large");
  data=(unsigned char*)memory.Allocate(size);if(!data)return Fail("Streaming allocation failed");
  for(unsigned int i=0;i<size;i++)data[i]=0;
  // Empty-region mode: original embedded global-region constructor initializes
  // its byte budget to zero. Manager+0xA4 is StreamingInfo.StreamingLimit in
  // bytes; zero here is the verified no-StreamingInfo case, not a format-wide assertion.
  unsigned int* header=(unsigned int*)data;header[StringPoolBytes]=pool;header[TotalAssetReferences]=count;header[SelectedConvexVolumeIndex]=0xffffffff;
  for(unsigned int i=0;i<unique;i++){unsigned int j=0;do{data[44+offsets[i]+j]=(unsigned char)strings[i][j];}while(strings[i][j++]);}
  *(unsigned int*)(data+44+pool)=count;
  unsigned int* indices=(unsigned int*)(data+48+pool);for(unsigned int i=0;i<count*2;i++)indices[i]=references[i];
  return true;
 }
};
// Global references only. Nonempty region geometry/tree and resource discovery pending.
}

