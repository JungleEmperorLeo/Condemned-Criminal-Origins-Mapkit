#pragma once
namespace WorldPrefix {
struct Header {unsigned int version,render,visibility,objects,blind;WorldText::Vec3 low,high,offset;};
struct TreeRoot {WorldText::Vec3 low,high;unsigned int count,reserved;};
static_assert(sizeof(Header)==56&&sizeof(TreeRoot)==32,"World prefix layout");
class Block {
public:
 PrefabScene::Arena memory;unsigned char* data;unsigned int size;const char* error;bool relocated;
 Block():data(0),size(0),error(0),relocated(false){}
 bool BuildLeaf(const WorldText::Geometry& geometry,const WorldModelsBlock::Block& models,const WorldText::Vec3& offset,bool emptyBlockers){
  memory.Clear();data=0;size=0;error=0;relocated=false;
  if(!geometry.points||!geometry.vertices||!models.data||!models.size||!WorldText::FiniteVector(offset)){error="Invalid prefix inputs";return false;}
  Header header={};header.version=113;header.low=header.high=geometry.vertices[0];header.offset=offset;
  for(unsigned int i=0;i<geometry.points;i++){const WorldText::Vec3& v=geometry.vertices[i];if(!WorldText::FiniteVector(v)){error="Non-finite prefix bounds";return false;}
   if(v.x<header.low.x)header.low.x=v.x;if(v.y<header.low.y)header.low.y=v.y;if(v.z<header.low.z)header.low.z=v.z;
   if(v.x>header.high.x)header.high.x=v.x;if(v.y>header.high.y)header.high.y=v.y;if(v.z>header.high.z)header.high.z=v.z;
  }
  // Only one unsplit quadtree root. Header offsets remain relocation slots.
  TreeRoot tree={};tree.low=header.low;tree.high=header.high;tree.count=1;
  size=56+32+1+models.size+(emptyBlockers?8:0);data=(unsigned char*)memory.Allocate(size);if(!data){size=0;error="Prefix allocation failed";return false;}
  const unsigned char* h=(const unsigned char*)&header;for(unsigned int i=0;i<56;i++)data[i]=h[i];
  const unsigned char* t=(const unsigned char*)&tree;for(unsigned int i=0;i<32;i++)data[56+i]=t[i];
  data[88]=0;for(unsigned int i=0;i<models.size;i++)data[89+i]=models.data[i];
  if(emptyBlockers)for(unsigned int i=0;i<8;i++)data[89+models.size+i]=0;
  return true;
 }
 bool SetOffsets(unsigned int render,unsigned int visibility,unsigned int objects,unsigned int blind,unsigned int fullFileSize){
  if(!data||render<size||visibility<render||objects<visibility||blind<objects||blind>=fullFileSize){error="Invalid prefix relocation offsets";return false;}
  Header* h=(Header*)data;h->render=render;h->visibility=visibility;h->objects=objects;h->blind=blind;relocated=true;error=0;return true;
 }
};
// Prefix is not a loadable world. Later sections and final relocation are required.
}
