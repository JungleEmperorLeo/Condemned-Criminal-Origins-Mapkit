#pragma once
#include "RenderPackedRanges.h"
// Inline geometry plus material strings, without mesh_kind/global topology.
// [0,namesOffset) is the shared 4E880 payload used by external Mesh00p;
// [namesOffset,size) is written only in the world mesh/material entry.
namespace RenderPackedMesh {
class Block {
 PrefabScene::Arena memory;unsigned int cursor;
 Block(const Block&);Block& operator=(const Block&);
 bool Fail(const char* message){memory.Clear();data=0;ranges=0;order=0;size=count=materials=definitions=definitionElements=vertexBytes=indexBytes=namesOffset=cursor=0;error=message;return false;}
 bool Put(const void* source,unsigned int bytes){if(bytes>size-cursor||(!source&&bytes))return false;if(bytes)memcpy(data+cursor,source,bytes);cursor+=bytes;return true;}
 bool U32(unsigned int value){return Put(&value,4);}
public:
 unsigned char* data;RenderPackedRanges::Range* ranges;unsigned int* order;
 unsigned int size,count,materials,definitions,definitionElements,vertexBytes,indexBytes,namesOffset;const char* error;
 Block():cursor(0),data(0),ranges(0),order(0),size(0),count(0),materials(0),definitions(0),definitionElements(0),vertexBytes(0),indexBytes(0),namesOffset(0),error(0){}
 bool Build(const RenderPackedRanges::Input* input,unsigned int sectionCount,bool sort){
  Fail(0);if(sectionCount>RenderPackedRanges::MaximumSections||(!input&&sectionCount))return Fail("Invalid mesh payload section count");
  if(!sectionCount){size=namesOffset=24;data=(unsigned char*)memory.Allocate(size);if(!data)return Fail("Empty mesh allocation failed");memset(data,0,size);return true;}
  RenderPackedRanges::Builder packed;if(!packed.Build(input,sectionCount,sort))return Fail(packed.error);
  const char** names=(const char**)memory.Allocate(packed.materialCount*4);ranges=(RenderPackedRanges::Range*)memory.Allocate(sectionCount*36);order=(unsigned int*)memory.Allocate(sectionCount*4);if(!names||!ranges||!order)return Fail("Mesh metadata allocation failed");memset(names,0,packed.materialCount*4);
  unsigned int nameBytes=0,definitionBytes=0;
  for(unsigned int i=0;i<sectionCount;i++){unsigned int material=packed.ranges[i].material;if(!names[material]){names[material]=input[packed.order[i]].material;nameBytes+=2+lstrlenA(names[material]);}order[i]=packed.order[i];ranges[i]=packed.ranges[i];}
  for(unsigned int i=0;i<packed.declarationCount;i++){definitionElements+=packed.declarationElements[i];definitionBytes+=4+packed.declarationElements[i]*8;}
  size=24+packed.vertexBytes+packed.indexCount*2+definitionBytes+sectionCount*36+nameBytes;
  if(size>32*1024*1024)return Fail("Mesh payload exceeds byte bound");data=(unsigned char*)memory.Allocate(size);if(!data)return Fail("Mesh payload allocation failed");
  unsigned int header[4]={sectionCount,packed.materialCount,packed.vertexBytes,packed.indexCount*2};
  if(!Put(header,16)||!Put(packed.vertices,packed.vertexBytes)||!Put(packed.indices,packed.indexCount*2)||!U32(packed.declarationCount))return Fail("Mesh payload buffer overflow");
  for(unsigned int i=0;i<packed.declarationCount;i++)if(!U32(packed.declarationElements[i]*8)||!Put(packed.declarations[i],packed.declarationElements[i]*8))return Fail("Mesh declaration overflow");
  if(!U32(sectionCount)||!Put(packed.ranges,sectionCount*36))return Fail("Mesh surface overflow");
  namesOffset=cursor;
  for(unsigned int i=0;i<packed.materialCount;i++){unsigned short length=(unsigned short)lstrlenA(names[i]);if(!Put(&length,2)||!Put(names[i],length))return Fail("Mesh name overflow");}
  if(cursor!=size)return Fail("Mesh payload size mismatch");count=sectionCount;materials=packed.materialCount;definitions=packed.declarationCount;vertexBytes=packed.vertexBytes;indexBytes=packed.indexCount*2;return true;
 }
};
}
