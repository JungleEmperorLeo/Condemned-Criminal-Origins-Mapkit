#pragma once
#include "ResourceOrder.h"
namespace StreamingResources {
struct Seed {unsigned int region;const char* path;};
class Sets {
 struct Item {unsigned int region;const char* path;StreamingBlock::Asset asset;};
 PrefabScene::Arena memory;Item* items;unsigned int count,capacity;ResourceOrder::List* orders;
 char Fold(char c){return c>='A'&&c<='Z'?c+('a'-'A'):c;}
 bool Equal(const char* a,const char* b){while(*a&&Fold(*a)==Fold(*b)){a++;b++;}return Fold(*a)==Fold(*b);}
 bool Fail(const char* e){global=0;regions=0;globalCount=0;error=e;return false;}
 bool Insert(unsigned int region,const char* path){
  if(!path||!path[0])return false;unsigned int n=0,split=0;while(path[n]&&n<=4096){if(path[n]=='\\')split=n+1;n++;}if(n>4096||split==n)return false;
  
  
  for(unsigned int i=0;i<count;i++)if(items[i].region==region&&Equal(items[i].path,path))return true;
  if(count==capacity)return false;char* owned=(char*)memory.Allocate(n+1);char* folder=(char*)memory.Allocate(split?split:1);if(!owned||!folder)return false;
  for(unsigned int i=0;i<=n;i++)owned[i]=path[i];if(split){for(unsigned int i=0;i<split-1;i++)folder[i]=path[i];folder[split-1]=0;}else folder[0]=0;
  bool inserted;if(!orders[region].Insert(owned,count,&inserted))return false;if(!inserted)return true;
  items[count].region=region;items[count].path=owned;items[count].asset.folder=folder;items[count].asset.file=owned+split;count++;return true;
 }
 const char* Artifact(const char* stem,const char* region,const char* suffix){
  unsigned int a=0,b=0,c=0;while(stem[a]&&a<=4096)a++;if(region)while(region[b]&&b<=4096)b++;while(suffix[c])c++;if(a+b+c+(region?1:0)>4096)return 0;
  char* p=(char*)memory.Allocate(a+b+c+2);if(!p)return 0;unsigned int i=0;for(unsigned int j=0;j<a;j++)p[i++]=stem[j];if(region){p[i++]='_';for(unsigned int j=0;j<b;j++)p[i++]=region[j];}for(unsigned int j=0;j<c;j++)p[i++]=suffix[j];p[i]=0;return p;
 }
 bool IsGlobal(const char* path){for(unsigned int i=0;i<count;i++)if(items[i].region==0&&Equal(items[i].path,path))return true;return false;}
 bool Streamable(const char* path){const char* extension=0;for(const char* p=path;*p;p++)if(*p=='.')extension=p+1;return extension&&(Equal(extension,"dds")||Equal(extension,"wav")||Equal(extension,"xsb")||Equal(extension,"xwb")||Equal(extension,"Mesh00p"));}
public:
 StreamingBlock::Asset* global;unsigned int globalCount;StreamingTables::Region* regions;const char* error;
 Sets():global(0),globalCount(0),regions(0),error(0){}
 bool Build(const char* worldStem,const char* const* names,const unsigned int* budgets,unsigned int regionCount,const Seed* seeds,unsigned int seedCount,const AssetList::List& list,const Seed* objectSeeds=0,unsigned int objectSeedCount=0){
  memory.Clear();global=0;globalCount=0;regions=0;error=0;count=0;
  if(!worldStem||!worldStem[0]||regionCount>10000||(!names&&regionCount)||(!budgets&&regionCount)||seedCount>100000||(!seeds&&seedCount)||list.count>100000)return Fail("Invalid resource input");
  if(objectSeedCount>100000||(!objectSeeds&&objectSeedCount))return Fail("Invalid object seeds");
  capacity=seedCount+objectSeedCount+list.count+regionCount*3+1;if(capacity>100000)return Fail("Too many resources");capacity*=2;items=(Item*)memory.Allocate((capacity+1)*sizeof(Item));regions=(StreamingTables::Region*)memory.Allocate((regionCount+1)*sizeof(StreamingTables::Region));if(!items||!regions)return Fail("Allocation failed");
  orders=(ResourceOrder::List*)memory.Allocate((regionCount+1)*sizeof(ResourceOrder::List));if(!orders)return Fail("Allocation failed");for(unsigned int i=0;i<=regionCount;i++)orders[i].Init(&memory);
  for(unsigned int i=0;i<seedCount;i++)if(seeds[i].region>regionCount||!Insert(seeds[i].region,seeds[i].path))return Fail("Invalid seed");
  if(!Insert(0,Artifact(worldStem,0,".xsb")))return Fail("Invalid world artifact");
  for(unsigned int i=0;i<regionCount;i++){
   if(!names[i]||!names[i][0])return Fail("Invalid region name");
   const char* suffixes[]={".Mesh00p",".xsb",".xwb"};for(unsigned int j=0;j<3;j++)if(!Insert(i+1,Artifact(worldStem,names[i],suffixes[j])))return Fail("Invalid region artifact");
  }
  // Asset00 entries precede object resources: untouched SDK WTF_Entry (340 listed,
  // 4148 total) is exact only with this position; before source seeds or after
  // object seeds the linear-hash chains order differently.
  for(unsigned int i=0;i<list.count;i++)if(list.entries[i].region>regionCount||!Insert(list.entries[i].region,list.entries[i].path))return Fail("Invalid asset entry");
  for(unsigned int i=0;i<objectSeedCount;i++)if(objectSeeds[i].region>regionCount||!Insert(objectSeeds[i].region,objectSeeds[i].path))return Fail("Invalid object seed");
  // Shared resources are promoted by regional hash iteration (original RVA 0x37E50),
  // not source insertion order. This affects Global ordering after bucket splits.
  // RVA 0x35F00/0x35FB0: nonstreamable types always become Global;
  // streamable types become Global only when present in EVERY region.
  unsigned int beforePromotion=count;for(unsigned int r=1;r<=regionCount;r++)for(ResourceOrder::Node* p=orders[r].head;p;p=p->next){const Item& item=items[p->value];if(IsGlobal(item.path))continue;bool promote=!Streamable(item.path);if(!promote){promote=true;for(unsigned int other=1;other<=regionCount&&promote;other++){bool present=false;for(unsigned int j=0;j<beforePromotion;j++)if(items[j].region==other&&Equal(items[j].path,item.path)){present=true;break;}if(!present)promote=false;}}if(promote&&!Insert(0,item.path))return Fail("Promotion failed");}
  // Original linear-hash iteration; global resources suppress regional copies.
  for(unsigned int r=0;r<=regionCount;r++){
   unsigned int selected=0;for(unsigned int j=0;j<count;j++)if(items[j].region==r&&(r==0||!IsGlobal(items[j].path)))selected++;StreamingBlock::Asset* assets=(StreamingBlock::Asset*)memory.Allocate((selected+1)*sizeof(StreamingBlock::Asset));if(!assets)return Fail("Allocation failed");unsigned int n=0;
   for(ResourceOrder::Node* p=orders[r].head;p;p=p->next){Item& x=items[p->value];if(r==0||!IsGlobal(x.path))assets[n++]=x.asset;}
   if(r==0){global=assets;globalCount=n;}else{regions[r-1].name=names[r-1];regions[r-1].budget=budgets[r-1];regions[r-1].references=0;regions[r-1].referenceCount=0;regions[r-1].assets=assets;regions[r-1].assetCount=n;}
  }
  return true;
 }
};
// Resource order uses the recovered original linear-hash splitting algorithm.
// ASCII case folding matches controlled path tests. Slash equivalence, non-ASCII
// names and general automatic-artifact conditions have not been recovered.
}







