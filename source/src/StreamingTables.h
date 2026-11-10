#pragma once
namespace StreamingTables {
// References address visibility sectors, not the streaming convex-volume array.
struct Region {const char* name;unsigned int budget;const unsigned int* references;unsigned int referenceCount;const StreamingBlock::Asset* assets;unsigned int assetCount;};
class Tables {
 PrefabScene::Arena memory;const char** strings;unsigned int* offsets;unsigned int unique,pool;
 bool Fail(const char* e){data=0;size=0;error=e;return false;}
 bool Add(const char* value,unsigned int* offset){
  if(!value)return false;unsigned int length=0;while(value[length]&&length<=4096)length++;if(length>4096)return false;
  unsigned int found=0;for(;found<unique;found++){unsigned int j=0;while(value[j]&&value[j]==strings[found][j])j++;if(value[j]==strings[found][j])break;}
  if(found==unique){unsigned int bytes=(length+4)&~3u;if(pool>16*1024*1024-bytes)return false;strings[unique]=value;offsets[unique]=pool;pool+=bytes;unique++;}*offset=offsets[found];return true;
 }
 bool Assets(const StreamingBlock::Asset* assets,unsigned int count,unsigned int* out){
  if(count&&!assets)return false;for(unsigned int i=0;i<count;i++)if(!assets[i].file||!assets[i].file[0]||!Add(assets[i].folder,out+i*2)||!Add(assets[i].file,out+i*2+1))return false;return true;
 }
public:
 unsigned char* data;unsigned int size,poolBytes,totalAssetReferences,aggregate;const char* error;
 Tables():data(0),size(0),poolBytes(0),totalAssetReferences(0),aggregate(0),error(0){}
 bool Build(const StreamingBlock::Asset* global,unsigned int globalCount,const Region* regions,unsigned int regionCount,unsigned int visibilitySectorCount){
  memory.Clear();data=0;size=poolBytes=aggregate=0;totalAssetReferences=globalCount;error=0;unique=pool=0;
  if(globalCount>100000||regionCount>100000||(!regions&&regionCount))return Fail("Invalid tables");
  for(unsigned int i=0;i<regionCount;i++){const Region& r=regions[i];if(!r.name||!r.name[0]||r.assetCount>100000-totalAssetReferences||r.referenceCount>100000-aggregate||(!r.references&&r.referenceCount))return Fail("Invalid region");totalAssetReferences+=r.assetCount;aggregate+=r.referenceCount;for(unsigned int j=0;j<r.referenceCount;j++)if(r.references[j]>=visibilitySectorCount)return Fail("Invalid region reference");}
  unsigned int capacity=totalAssetReferences*2+regionCount+1;
  strings=(const char**)memory.Allocate(capacity*sizeof(char*));offsets=(unsigned int*)memory.Allocate(capacity*4);
  unsigned int tableBytes=4+totalAssetReferences*8+regionCount*16+aggregate*4;
  unsigned int* tables=(unsigned int*)memory.Allocate(tableBytes);if(!strings||!offsets||!tables)return Fail("Allocation failed");
  // Original pool is populated by named regions first, then the global region.
  unsigned int cursor=1+globalCount*2;tables[0]=globalCount;
  for(unsigned int i=0;i<regionCount;i++){const Region& r=regions[i];if(!Add(r.name,tables+cursor))return Fail("Invalid name");tables[cursor+1]=r.budget;tables[cursor+2]=r.referenceCount;cursor+=3;for(unsigned int j=0;j<r.referenceCount;j++)tables[cursor++]=r.references[j];tables[cursor++]=r.assetCount;if(!Assets(r.assets,r.assetCount,tables+cursor))return Fail("Invalid region assets");cursor+=r.assetCount*2;}
  if(!Assets(global,globalCount,tables+1))return Fail("Invalid global assets");if(cursor*4!=tableBytes||pool>16*1024*1024-tableBytes)return Fail("Tables too large");
  size=pool+tableBytes;poolBytes=pool;data=(unsigned char*)memory.Allocate(size);if(!data)return Fail("Allocation failed");for(unsigned int i=0;i<size;i++)data[i]=0;
  for(unsigned int i=0;i<unique;i++){unsigned int j=0;do{data[offsets[i]+j]=(unsigned char)strings[i][j];}while(strings[i][j++]);}
  for(unsigned int i=0;i<tableBytes;i++)data[pool+i]=((unsigned char*)tables)[i];return true;
 }
};
}
