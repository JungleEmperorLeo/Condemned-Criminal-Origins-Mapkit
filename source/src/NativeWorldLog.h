#pragma once
// Statistics of the original WorldPacker log, computed from the produced
// sections (each recovered from the original's code; see PACKER_LOG.md).
#include "PackerLog.h"
namespace NativeWorldLog {
inline unsigned int U32(const unsigned char* p){unsigned int v;memcpy(&v,p,4);return v;}
// WorldModels section header (0x10051300): eight counts XOR 399 - names,
// string bytes, normals, BSP blocks, nodes, polygons, vertex refs, points.
struct WorldModelsHeader {unsigned int names,strings,normals,models,nodes,polygons,references,points;};
inline bool ReadWorldModels(const unsigned char* data,unsigned int size,WorldModelsHeader& h){
 if(!data||size<32)return false;unsigned int* out=&h.names;for(unsigned int i=0;i<8;i++)out[i]=U32(data+4*i)^399u;return true;
}
// Main physics BSP (first block, flags 1): node table for the "BSP tree %0.2f%%
// balanced." value (0x100086D0) and the output totals (0x10028080).
struct MainBsp {unsigned int polygons,points,depth;double balance;bool valid;};
class Bsp {
 const unsigned char* nodes;unsigned int count;unsigned int* sizes;
 static bool Leaf(int child){return child<0;}
 int Child(unsigned int node,unsigned int which)const{return (int)U32(nodes+12*node+4+4*which);}
 // 0x100084C0: nodes of the subtree (float accumulation; integers are exact).
 float Size(int node)const{return Leaf(node)?0.0f:(float)sizes[node];}
 // 0x100086D0 with its float stores; x87 runs at 53-bit precision (CW 0x027F).
 double Balance(int node)const{
  if(Leaf(node))return 1.0;
  int front=Child((unsigned int)node,0),back=Child((unsigned int)node,1);
  volatile float a=Leaf(front)?0.0f:(float)((double)Size(Child((unsigned int)front,1))+(double)Size(Child((unsigned int)front,0))+1.0);
  volatile float b=Leaf(back)?0.0f:(float)((double)Size(Child((unsigned int)back,1))+(double)Size(Child((unsigned int)back,0))+1.0);
  volatile float s=(float)((double)b+(double)a);if(!((double)s>1.0))return 1.0;
  double difference=(double)b-(double)a;volatile float t=(float)(difference<0?-difference:difference);
  volatile float local=(float)(1.0-(double)t/(((double)s+1.0)*2.0));
  volatile float f=(float)Balance(front);volatile float k=(float)Balance(back);
  return ((double)k+(double)f)*0.25+(double)local*0.5;
 }
public:
 Bsp():nodes(0),count(0),sizes(0){}
 bool Read(const unsigned char* data,unsigned int size,PrefabScene::Arena& memory,MainBsp& out){
  out.valid=false;WorldModelsHeader h;if(!ReadWorldModels(data,size,h)||!h.models)return false;
  unsigned int at=32+h.strings;if(h.names>size/8||h.normals>size/12)return false;at+=h.names*8+h.normals*12;if(at+48>size)return false;
  const unsigned char* block=data+at;unsigned int flags=U32(block),points=U32(block+4),polygons=U32(block+8),nodeCount=U32(block+16);int root=(int)U32(block+44);
  if(flags!=1||polygons>size||nodeCount>size/12)return false;
  unsigned int cursor=48+polygons;for(unsigned int p=0;p<polygons;p++){if(at+48+p>=size)return false;cursor+=12+4u*block[48+p];}
  if(at+cursor+nodeCount*12>size)return false;nodes=block+cursor;count=nodeCount;
  sizes=(unsigned int*)memory.Allocate((count+1)*4);unsigned int* order=(unsigned int*)memory.Allocate((count+1)*4);unsigned int* stack=(unsigned int*)memory.Allocate((count+1)*4);if(!sizes||!order||!stack)return false;
  out.polygons=polygons;out.points=points;out.depth=0;out.balance=1.0;
  if(root>=0){
   if((unsigned int)root>=count)return false;unsigned int used=0,n=0;stack[used++]=(unsigned int)root;
   while(used){unsigned int node=stack[--used];if(n>=count)return false;order[n++]=node;for(unsigned int c=0;c<2;c++){int child=Child(node,c);if(child>=0){if((unsigned int)child>=count||used>=count)return false;stack[used++]=(unsigned int)child;}}}
   unsigned int* depth=stack;for(unsigned int i=n;i-->0;){unsigned int node=order[i];unsigned int s=1,d=0;for(unsigned int c=0;c<2;c++){int child=Child(node,c);if(child>=0){s+=sizes[child];if(depth[child]>d)d=depth[child];}}sizes[node]=s;depth[node]=d+1;}
   out.depth=depth[root];
   // Recursion depth equals the tree depth (hundreds on the largest SDK maps).
   out.balance=Balance(root);
  }
  out.valid=true;return true;
 }
};
// Section size line (0x10050F50): above 1 MB "%.1f MB", above 1 KB "%.1f KB",
// otherwise "%d B"; the byte count is converted to float before dividing.
inline void Size(const char* name,unsigned int bytes){
 volatile float value=(float)bytes;
 if(bytes>0x100000u)PackerLog::Normal("%s : %.1f MB",name,(double)value/(double)1048576.0f);
 else if(bytes>0x400u)PackerLog::Normal("%s : %.1f KB",name,(double)value/(double)1024.0f);
 else PackerLog::Normal("%s : %d B",name,bytes);
}
// Render archive tree records (0x10050AB7): bytes of the main tree and of all
// other trees, measured as written after the mesh table.
inline bool RenderTreeBytes(const unsigned char* data,unsigned int size,const RenderGlobalMeshes::Mesh* meshes,unsigned int meshCount,unsigned int& main,unsigned int& models){
 main=models=0;if(!data||size<40)return false;unsigned int trees=U32(data),mainTree=U32(data+36);unsigned int at=40;
 for(unsigned int m=0;m<meshCount;m++){const RenderPackedMesh::Block* g=meshes[m].geometry;if(!g)return false;at+=4;if(!m)at+=g->size;else at+=4+2+lstrlenA(meshes[m].name)+4+(g->size-g->namesOffset);if(at>size)return false;}
 for(unsigned int t=0;t<trees;t++){
  unsigned int begin=at;if(size-at<4)return false;unsigned int nodes=U32(data+at);at+=4;
  for(unsigned int n=0;n<nodes;n++){if(size-at<8)return false;unsigned int branches=U32(data+at),sky=U32(data+at+4);at+=8;if(branches>(size-at)/36)return false;at+=branches*36;
   for(unsigned int s=0;s<sky;s++){if(at>=size)return false;unsigned int c=data[at];at+=1;if(c*12>size-at)return false;at+=c*12;}}
  if(t==mainTree)main+=at-begin;else models+=at-begin;
 }
 return at==size;
}
}
