#pragma once
namespace WorldModelsBlock {
class Block {
 unsigned int cursor;
 bool Put(const void* p,unsigned int bytes){if(bytes>size-cursor)return false;const unsigned char* source=(const unsigned char*)p;for(unsigned int i=0;i<bytes;i++)data[cursor++]=source[i];return true;}
 bool Fail(const char* text){error=text;data=0;size=cursor=0;return false;}
public:
 PrefabScene::Arena memory;unsigned char* data;unsigned int size,normalCount;const char* error;
 Block():cursor(0),data(0),size(0),normalCount(0),error(0){}
 bool BuildSingle(const PrefabScene::Node& brush,const BspTree::Tree& tree,const BspBlock::SurfaceMetadata* metadata,unsigned int metadataCount,const char* name,unsigned int flags,int positiveLeaf,int negativeLeaf){
  memory.Clear();data=0;size=cursor=normalCount=0;error=0;
  if(!name||!name[0]||lstrlenA(name)>255||brush.geometry.polygons>100000)return Fail("Invalid single WorldModels input");
  WorldText::Vec3* normals=(WorldText::Vec3*)memory.Allocate((brush.geometry.polygons+1)*sizeof(WorldText::Vec3));if(!normals)return Fail("Could not allocate normal table");
  // Own first-occurrence order, not FEAR's unrecovered global collection order.
  for(unsigned int i=0;i<brush.geometry.polygons;i++){
   const WorldText::Vec3& normal=brush.geometry.faces[i].normal;if(!WorldText::FiniteVector(normal))return Fail("Non-finite normal table entry");
   unsigned int found=0;while(found<normalCount&&!BspSplit::Same(normals[found],normal))found++;
   if(found==normalCount)normals[normalCount++]=normal;
  }
  BspBlock::Block bsp;if(!bsp.Build(brush,tree,normals,normalCount,metadata,metadataCount,flags,positiveLeaf,negativeLeaf))return Fail(bsp.error);
  unsigned int nameBytes=(lstrlenA(name)+1+3)&~3u;
  size=32+nameBytes+8+normalCount*12+bsp.size;data=(unsigned char*)memory.Allocate(size);if(!data)return Fail("Could not allocate WorldModels block");
  unsigned int refs=0;for(unsigned int i=0;i<brush.geometry.polygons;i++)refs+=brush.geometry.faces[i].count;
  unsigned int counts[8]={1,nameBytes,normalCount,1,tree.branches,brush.geometry.polygons,refs,brush.geometry.points};
  for(unsigned int i=0;i<8;i++){unsigned int encoded=counts[i]^399;if(!Put(&encoded,4))return Fail("WorldModels buffer overflow");}
  if(!Put(name,lstrlenA(name)+1))return Fail("WorldModels buffer overflow");
  unsigned char zero=0;for(unsigned int i=lstrlenA(name)+1;i<nameBytes;i++)if(!Put(&zero,1))return Fail("WorldModels buffer overflow");
  unsigned int nameIndex[2]={0,0};if(!Put(nameIndex,8)||!Put(normals,normalCount*12)||!Put(bsp.data,bsp.size)||cursor!=size)return Fail("WorldModels buffer overflow");
  return true;
 }
};
// WorldModels only: no file header, world quadtree, blockers or later sections.
// Restricted to the single unsplit brush supported by BspBlock.
}
