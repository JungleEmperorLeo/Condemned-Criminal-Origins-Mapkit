#pragma once
#include "PhysicalBspSerialization.h"
namespace WorldModelsSerialization {
static const unsigned int MaximumModels=4096u,MaximumNames=100000u,MaximumNormals=100000u,MaximumNameBytes=16u*1024u*1024u,MaximumBytes=128u*1024u*1024u;
struct Name {const char* text;unsigned int model;};
class Block {
 PrefabScene::Arena memory;unsigned int cursor;
 Block(const Block&);Block& operator=(const Block&);
 bool Fail(const char* reason){Clear();error=reason;return false;}
 bool Put(const void* source,unsigned int bytes){if(bytes>size-cursor||(!source&&bytes))return false;if(bytes)memcpy(data+cursor,source,bytes);cursor+=bytes;return true;}
 static bool Add(unsigned int value,unsigned int& sum){if(value>0x7fffffffu-sum)return false;sum+=value;return true;}
public:
 unsigned char* data;unsigned int size,counts[8];const char* error;
 Block():cursor(0),data(0),size(0),error(0){memset(counts,0,sizeof(counts));}
 void Clear(){memory.Clear();data=0;size=cursor=0;error=0;memset(counts,0,sizeof(counts));}
 bool Build(const Name* names,unsigned int nameCount,const WorldText::Vec3* normals,unsigned int normalCount,const PhysicalBspSerialization::Block* const* models,unsigned int modelCount){
  Clear();if(!names||!nameCount||nameCount>MaximumNames||!normals||!normalCount||normalCount>MaximumNormals||!models||!modelCount||modelCount>MaximumModels)return Fail("Invalid WorldModels tables");
  for(unsigned int i=0;i<normalCount;i++)if(!WorldText::FiniteVector(normals[i])||(normals[i].x==0&&normals[i].y==0&&normals[i].z==0))return Fail("Invalid WorldModels normal");
  PrefabScene::Arena temporary;unsigned int* offsets=(unsigned int*)temporary.Allocate(nameCount*4);if(!offsets)return Fail("WorldModels names allocation failed");unsigned int strings=0,payload=0;
  for(unsigned int i=0;i<nameCount;i++){
   const Name& name=names[i];if(!name.text||!name.text[0]||name.model>=modelCount)return Fail("Invalid WorldModels name reference");unsigned int length=0;while(length<256&&name.text[length])length++;if(length==256||length+1>MaximumNameBytes-strings)return Fail("WorldModels name table bound");offsets[i]=strings;strings+=length+1;
  }
  strings=(strings+3u)&~3u;counts[0]=nameCount;counts[1]=strings;counts[2]=normalCount;counts[3]=modelCount;
  for(unsigned int i=0;i<modelCount;i++){
   if(!models[i]||!models[i]->data||models[i]->size<48||models[i]->error||!Add(models[i]->size,payload)||!Add(models[i]->header.nodes,counts[4])||!Add(models[i]->header.polygons,counts[5])||!Add(models[i]->header.references,counts[6])||!Add(models[i]->header.points,counts[7]))return Fail("Invalid WorldModels BSP block or aggregate count");
   const PhysicalBspSerialization::Header& header=models[i]->header;
   unsigned int at=48+header.polygons;for(unsigned int p=0;p<header.polygons;p++){
    unsigned int normal=0;memcpy(&normal,models[i]->data+at+4,4);if(normal>=normalCount)return Fail("WorldModels global normal reference out of range");at+=12u+models[i]->data[48+p]*4u;
   }
  }
  unsigned int needed=32u+strings+nameCount*8u+normalCount*12u;if(payload>MaximumBytes||needed>MaximumBytes-payload)return Fail("WorldModels serialized byte bound");size=needed+payload;data=(unsigned char*)memory.Allocate(size);if(!data)return Fail("WorldModels output allocation failed");
  for(unsigned int i=0;i<8;i++){unsigned int encoded=counts[i]^399u;if(!Put(&encoded,4))return Fail("WorldModels count write overflow");}
  unsigned int written=0;for(unsigned int i=0;i<nameCount;i++){unsigned int length=lstrlenA(names[i].text)+1;if(!Put(names[i].text,length))return Fail("WorldModels name write overflow");written+=length;}
  unsigned char zero=0;while(written++<strings)if(!Put(&zero,1))return Fail("WorldModels string padding overflow");
  for(unsigned int i=0;i<nameCount;i++)if(!Put(offsets+i,4)||!Put(&names[i].model,4))return Fail("WorldModels name index write overflow");
  if(!Put(normals,normalCount*12u))return Fail("WorldModels normal write overflow");for(unsigned int i=0;i<modelCount;i++)if(!Put(models[i]->data,models[i]->size))return Fail("WorldModels BSP write overflow");
  return cursor==size||Fail("WorldModels final write size mismatch");
 }
};
}
