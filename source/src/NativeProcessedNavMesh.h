#pragma once
#include "NativeBlindDataRegistry.h"
#include "NativeRawNavMesh.h"
// Bounded source-bound acceptance for the INITIAL one-quad SDK output only.
// This is deliberately narrower than the independent general Python decoder.
// No exporter call or donor payload lookup occurs here.
namespace NativeProcessedNavMesh {
static const unsigned int TypeId=0x83f47c31u;
struct Input {
 const void* processed;unsigned int bytes;const void* sourceRaw;unsigned int sourceRawBytes;
 Input():processed(0),bytes(0),sourceRaw(0),sourceRawBytes(0){}
};
namespace detail {
inline unsigned int Word(const unsigned char* p){return NativeBlindDataRegistry::detail::Get(p);}
inline float Float(const unsigned char* p){float f;memcpy(&f,p,4);return f;}
inline float F32(double value){volatile float f=(float)value;return f;}
inline bool VectorEqual(const unsigned char* p,const WorldText::Vec3& v){return Float(p)==v.x&&Float(p+4)==v.y&&Float(p+8)==v.z;}
inline bool FiniteVector(const unsigned char* p){return WorldText::Finite(Float(p))&&WorldText::Finite(Float(p+4))&&WorldText::Finite(Float(p+8));}
struct Span {const unsigned char* data;unsigned int count,stride;const unsigned char* Row(unsigned int i)const{return data+i*stride;}};
class Reader {
 const unsigned char* data;unsigned int size,at;
public:
 Reader(const void* p,unsigned int n):data((const unsigned char*)p),size(n),at(0){}
 bool ReadWord(unsigned int& v){if(at>size||4u>size-at)return false;v=Word(data+at);at+=4;return true;}
 bool Array(Span& span,unsigned int stride,unsigned int expected){unsigned int count;if(!ReadWord(count)||count!=expected||!stride||at>size||count>(size-at)/stride)return false;span.data=data+at;span.count=count;span.stride=stride;at+=count*stride;return true;}
 bool End()const{return at==size;}
};
}
inline bool Validate(const NativeRawNavMesh::Builder& raw,const Input& input,const char*& error){
 using namespace detail;error=0;
 if(!raw.data||!raw.size||!input.processed||input.bytes<76||input.bytes>16u*1024u*1024u||!input.sourceRaw||input.sourceRawBytes!=raw.size){error="Navigation input requires source raw witness and a bounded processed payload";return false;}
 const unsigned char* witness=(const unsigned char*)input.sourceRaw;for(unsigned int i=0;i<raw.size;i++)if(witness[i]!=raw.data[i]){error="Navigation payload raw witness differs from the current source scene";return false;}
 // Active CharacterType restrictions remain disabled until the difference
 // between authored suffixes0..15 and runtime suffixes1..16 is resolved.
 unsigned int navTypeBytes=(unsigned int)raw.data[120]|((unsigned int)raw.data[121]<<8);
 if(raw.size<130u+navTypeBytes||Word(raw.data+126u+navTypeBytes)!=0){error="Registered navigation requires all authored CharacterType restrictions to be None";return false;}
 WorldText::Vec3 points[4];for(unsigned int i=0;i<4;i++)memcpy(points+i,raw.data+48+i*12,12);
 float width=F32(double(raw.boundsMax.x)-raw.boundsMin.x),depth=F32(double(raw.boundsMax.z)-raw.boundsMin.z);
 for(unsigned int i=0;i<4;i++)if(points[i].x>1048576.f||points[i].x<-1048576.f||points[i].y>1048576.f||points[i].y<-1048576.f||points[i].z>1048576.f||points[i].z<-1048576.f){error="Navigation source exceeds the initial SDK coordinate range";return false;}
 if(width<64||depth<64||double(width)>16.0*depth||double(depth)>16.0*width){error="Navigation source exceeds the initial SDK side/aspect profile";return false;}
 Reader read(input.processed,input.bytes);unsigned int word;
 if(!read.ReadWord(word)||word!=1||!read.ReadWord(word)||word!=6||!read.ReadWord(word)||word!=0){error="Navigation requires processed v6 unfixed disk framing";return false;}
 Span edges,edgeRefs,regionRefs,normals,polys,regionPolys,regions,components,neighbors,links,boundary,nodes,names,tree,treePolys;
 if(!read.Array(edges,72,4)||!read.Array(edgeRefs,4,4)||!read.Array(regionRefs,4,0)||!read.Array(normals,12,1)||!read.Array(polys,72,1)||!read.Array(regionPolys,4,0)||!read.Array(regions,28,0)||!read.Array(components,16,1)||!read.Array(neighbors,4,0)||!read.Array(links,12,0)||!read.Array(boundary,12,0)||!read.Array(nodes,8,0)||!read.Array(names,1,4)||!read.Array(tree,52,341)||!read.Array(treePolys,4,256)||!read.End()){error="Navigation output differs from complete single-quad SDK stream framing";return false;}
 for(unsigned int i=0;i<4;i++)if(names.data[i]){error="Navigation empty node-name padding must be zero";return false;}
 const unsigned char* poly=polys.data;const unsigned char* component=components.data;
 if(Word(poly)||Word(poly+4)||Word(poly+8)||Word(poly+12)!=0xffffffffu||Word(poly+16)!=1||Word(poly+56)!=4||Word(poly+60)||Word(poly+64)||Word(poly+68)||Word(component)||Word(component+4)||Word(component+8)||Word(component+12)){error="Navigation polygon/component IDs or list ranges differ from the source profile";return false;}
 WorldText::Vec3 center={};for(unsigned int i=0;i<4;i++){center.x=F32(double(center.x)+points[i].x);center.y=F32(double(center.y)+points[i].y);center.z=F32(double(center.z)+points[i].z);}center.x=F32(double(center.x)/4);center.y=F32(double(center.y)/4);center.z=F32(double(center.z)/4);
 WorldText::Vec3 lo=raw.boundsMin,hi=raw.boundsMax;lo.y=hi.y=points[0].y;WorldText::Vec3 normal={0,1,0};
 if(!VectorEqual(poly+20,center)||!VectorEqual(poly+32,lo)||!VectorEqual(poly+44,hi)||!VectorEqual(normals.data,normal)){error="Navigation polygon center/bounds/normal differ from the raw source quad";return false;}
 unsigned int segments=0,edgeIds[4]={};
 for(unsigned int i=0;i<4;i++){
  const unsigned char* e=edges.Row(i);if(Word(e)!=i||Word(e+4)||Word(e+20)!=1||Word(e+36)!=1||Word(e+52)||Word(e+56)!=0xffffffffu||!FiniteVector(e+8)||!FiniteVector(e+24)||!FiniteVector(e+40)||!FiniteVector(e+60)){error="Navigation border edge fields differ from the source profile";return false;}
  unsigned int segment=0xffffffffu;for(unsigned int j=0;j<4;j++){const WorldText::Vec3 &a=points[j],&b=points[(j+1)&3];if((VectorEqual(e+8,a)&&VectorEqual(e+24,b))||(VectorEqual(e+8,b)&&VectorEqual(e+24,a)))segment=j;}
  if(segment==0xffffffffu||(segments&(1u<<segment))){error="Navigation output has missing/duplicate/non-source quad edges";return false;}segments|=1u<<segment;edgeIds[segment]=i;const WorldText::Vec3 &a=points[segment],&b=points[(segment+1)&3];WorldText::Vec3 midpoint={F32(double(F32(double(a.x)+b.x))*.5),F32(double(F32(double(a.y)+b.y))*.5),F32(double(F32(double(a.z)+b.z))*.5)};WorldText::Vec3 edgeNormal={};if(a.x==b.x)edgeNormal.x=center.x>a.x?1.f:-1.f;else edgeNormal.z=center.z>a.z?1.f:-1.f;
  if(!VectorEqual(e+40,midpoint)||!VectorEqual(e+60,edgeNormal)){error="Navigation edge midpoint/normal differs from source quad";return false;}
 }
 for(unsigned int i=0;i<4;i++)if(Word(edgeRefs.Row(i))!=edgeIds[i]){error="Navigation polygon edge order differs from source winding";return false;}
 PrefabScene::Arena scratch;unsigned int* parents=(unsigned int*)scratch.Allocate(341*4),*queue=(unsigned int*)scratch.Allocate(341*4),*levels=(unsigned int*)scratch.Allocate(341*4);if(!parents||!queue||!levels){error="Navigation quadtree validator allocation failed";return false;}unsigned int nextPoly=0;
 for(unsigned int i=0;i<341;i++){
  const unsigned char* q=tree.Row(i);if(Word(q)!=i||!FiniteVector(q+4)||!FiniteVector(q+16)||Float(q+4)>Float(q+16)||Float(q+8)>Float(q+20)||Float(q+12)>Float(q+24)||Word(q+32)!=nextPoly||Word(q+28)>256-nextPoly){error="Navigation quadtree row/bounds/poly list is invalid";return false;}nextPoly+=Word(q+28);
  unsigned int children=0;for(unsigned int j=0;j<4;j++){unsigned int child=Word(q+36+j*4);if(child==0xffffffffu)continue;if(child>=341||child==i||++parents[child]!=1){error="Navigation quadtree child is outside a unique tree";return false;}children++;}if(children!=0&&children!=4){error="Navigation quadtree internal row requires four children";return false;}
 }
 if(parents[0]||nextPoly!=256||!VectorEqual(tree.data+4,raw.boundsMin)||!VectorEqual(tree.data+16,raw.boundsMax)){error="Navigation quadtree root/list differs from source bounds";return false;}for(unsigned int i=1;i<341;i++)if(parents[i]!=1){error="Navigation quadtree has an orphan row";return false;}
 unsigned int head=0,tail=1;while(head<tail){unsigned int id=queue[head++],level=levels[id];const unsigned char* q=tree.Row(id);if(level>4){error="Navigation quadtree exceeds the fixed SDK depth";return false;}if(level==4){if(Word(q+28)!=1){error="Navigation quadtree leaf must reference the one source polygon";return false;}for(unsigned int j=0;j<4;j++)if(Word(q+36+j*4)!=0xffffffffu){error="Navigation quadtree leaf has children";return false;}}else{
   if(Word(q+28)){error="Navigation quadtree internal row has unexpected polygon references";return false;}WorldText::Vec3 low={Float(q+4),Float(q+8),Float(q+12)},high={Float(q+16),Float(q+20),Float(q+24)};float midX=F32(double(F32(double(low.x)+high.x))*.5),midZ=F32(double(F32(double(low.z)+high.z))*.5);
   for(unsigned int j=0;j<4;j++){unsigned int child=Word(q+36+j*4);if(child>=341||tail>=341){error="Navigation quadtree is cyclic or incomplete";return false;}WorldText::Vec3 childLow=low,childHigh=high;if(j<2)childLow.z=midZ;else childHigh.z=midZ;if(j&1)childLow.x=midX;else childHigh.x=midX;if(!VectorEqual(tree.Row(child)+4,childLow)||!VectorEqual(tree.Row(child)+16,childHigh)){error="Navigation quadtree child bounds differ from SDK subdivision";return false;}levels[child]=level+1;queue[tail++]=child;}
  }}
 if(tail!=341){error="Navigation quadtree is disconnected";return false;}for(unsigned int i=0;i<256;i++)if(Word(treePolys.Row(i))){error="Navigation quadtree references a non-source polygon";return false;}return true;
}
}
