#pragma once
#include "RenderBlock.h"
#include "RenderTrees.h"
namespace CompleteRender {
struct Surface {const unsigned char* vertices;const unsigned short* indices;unsigned int count,triangles,stride,definition,material,tree,branchType;};
struct Element {unsigned short stream,offset;unsigned char type,method,usage,index;};
struct Profile {const char* const* materials;unsigned int materialCount;const void* const* declarations;const unsigned int* declarationBytes;unsigned int declarationCount;};
static const Element declarations0[]={{0,0,2,0,0,0},{0,12,2,0,3,0},{255,0,17,0,0,0}};
static const Element declarations1[]={{0,0,2,0,0,0},{0,12,4,0,10,0},{0,16,1,0,5,0},{255,0,17,0,0,0}};
static const Element declarations2[]={{0,0,2,0,0,0},{0,12,2,0,3,0},{0,24,1,0,5,0},{0,32,2,0,6,0},{0,44,2,0,7,0},{255,0,17,0,0,0}};
static const void* const declarations[]={declarations0,declarations1,declarations2};
static const unsigned int declarationBytes[]={sizeof(declarations0),sizeof(declarations1),sizeof(declarations2)};
static const char* const materials[]={"engine\\shadowvolume.Mat00","Prefabs\\Office\\Cactus1.Mat00","Materials\\Invisible.Mat00","Materials\\Grid.Mat00","Prefabs\\Office\\Cactus.Mat00"};
class Block {
 unsigned int cursor;
 bool Put(const void* input,unsigned int bytes){if(bytes>size-cursor)return false;const unsigned char* p=(const unsigned char*)input;for(unsigned int i=0;i<bytes;i++)data[cursor++]=p[i];return true;}
 bool Zero(unsigned int bytes){if(bytes>size-cursor)return false;while(bytes--)data[cursor++]=0;return true;}
 bool Fail(const char* msg){memory.Clear();data=0;size=cursor=0;error=msg;return false;}
public:
 PrefabScene::Arena memory;unsigned char* data;unsigned int size;const char* error;
 Block():cursor(0),data(0),size(0),error(0){}
 bool Build(const Surface* surfaces,unsigned int count,unsigned int trees){
  const Profile sample={materials,5,declarations,declarationBytes,3};return Build(surfaces,count,trees,sample);
 }
 bool Build(const Surface* surfaces,unsigned int count,unsigned int trees,const Profile& profile){
  return Build(surfaces,count,trees,profile,0);
 }
 bool Build(const Surface* surfaces,unsigned int count,unsigned int trees,const Profile& profile,const RenderTrees::Layout* layout){
  memory.Clear();data=0;size=cursor=0;error=0;if(!surfaces||!count||count>1024||!trees||trees>4096)return Fail("Unsupported render profile count");
  if(!profile.materials||!profile.materialCount||profile.materialCount>4096||!profile.declarations||!profile.declarationBytes||!profile.declarationCount||profile.declarationCount>256)return Fail("Invalid render profile tables");
  unsigned int names=0,definitionSize=0;
  for(unsigned int i=0;i<profile.materialCount;i++){const char* name=profile.materials[i];if(!name||!name[0])return Fail("Invalid render material name");unsigned int length=0;while(name[length]&&length<=4096)length++;if(length>4096)return Fail("Render material name too long");names+=2+length;}
  for(unsigned int i=0;i<profile.declarationCount;i++){unsigned int bytes=profile.declarationBytes[i];if(!profile.declarations[i]||bytes<16||bytes>256||bytes%8)return Fail("Invalid render declaration");definitionSize+=4+bytes;}
  unsigned int* record=(unsigned int*)memory.Allocate(count*36);unsigned int* padding=(unsigned int*)memory.Allocate(count*4);WorldText::Vec3* bounds=(WorldText::Vec3*)memory.Allocate(count*24);if(!record||!padding||!bounds)return Fail("Render metadata allocation failed");
  unsigned int vb=0,ib=0;RenderBlock::Block validator;
  for(unsigned int i=0;i<count;i++){
   const Surface& s=surfaces[i];if(s.tree>=trees||s.branchType>3||s.definition>=profile.declarationCount||s.material>=profile.materialCount)return Fail("Unsupported surface profile");
   if(!validator.BuildSingle(s.vertices,s.count,s.stride,s.indices,s.triangles,profile.declarations[s.definition],profile.declarationBytes[s.definition],profile.materials[s.material]))return Fail("Invalid render surface");
   padding[i]=(s.stride-vb%s.stride)%s.stride;if(s.count*s.stride+padding[i]>16*1024*1024-vb||s.triangles*6>16*1024*1024-ib)return Fail("Render buffers exceed limit");vb+=padding[i];
   unsigned int r[9]={vb/s.stride,s.count,s.stride,ib/2,vb/s.stride,s.triangles,s.material,0,s.definition};for(unsigned int k=0;k<9;k++)record[i*9+k]=r[k];vb+=s.count*s.stride;ib+=s.triangles*6;
   WorldText::Vec3& low=bounds[i*2];WorldText::Vec3& high=bounds[i*2+1];
   for(unsigned int j=0;j<s.count;j++){WorldText::Vec3 p;unsigned char* b=(unsigned char*)&p;for(unsigned int k=0;k<12;k++)b[k]=s.vertices[j*s.stride+k];if(!j)low=high=p;if(p.x<low.x)low.x=p.x;if(p.y<low.y)low.y=p.y;if(p.z<low.z)low.z=p.z;if(p.x>high.x)high.x=p.x;if(p.y>high.y)high.y=p.y;if(p.z>high.z)high.z=p.z;}
  }
  RenderTrees::Block topology;unsigned int nodeCount=trees,treeBytes=trees*12+count*36;
  if(layout){if(layout->count!=trees||!topology.Build(*layout,count)||topology.branches!=count)return Fail("Invalid complete render topology");
   unsigned char* seen=(unsigned char*)memory.Allocate(count);if(!seen)return Fail("Render reference allocation failed");memset(seen,0,count);
   for(unsigned int t=0;t<trees;t++)for(unsigned int n=0;n<layout->trees[t].count;n++)for(unsigned int b=0;b<layout->trees[t].nodes[n].count;b++){
    const RenderTrees::Branch& branch=layout->trees[t].nodes[n].branches[b];if(branch.meshIndex!=0)return Fail("Complete render supports only mesh index zero");if(seen[branch.surface]||surfaces[branch.surface].tree!=t||surfaces[branch.surface].branchType!=branch.type)return Fail("Render topology surface mismatch");seen[branch.surface]=1;
   }nodeCount=topology.nodes;treeBytes=topology.size;
  }
  size=60+vb+ib+4+definitionSize+4+count*36+names+treeBytes;
  if(size>32*1024*1024)return Fail("Render block exceeds limit");data=(unsigned char*)memory.Allocate(size);if(!data)return Fail("Render block allocation failed");
  unsigned int header[15]={trees,nodeCount,count,1,profile.materialCount,1,count,topology.skyPortals,topology.skyPortalVertices,0,0,count,profile.materialCount,vb,ib};if(!Put(header,60))return Fail("Render header overflow");
  for(unsigned int i=0;i<count;i++)if(!Zero(padding[i])||!Put(surfaces[i].vertices,surfaces[i].count*surfaces[i].stride))return Fail("Render vertex overflow");
  for(unsigned int i=0;i<count;i++)if(!Put(surfaces[i].indices,surfaces[i].triangles*6))return Fail("Render index overflow");
  unsigned int defs=profile.declarationCount;if(!Put(&defs,4))return Fail("Render declaration overflow");for(unsigned int i=0;i<defs;i++)if(!Put(&profile.declarationBytes[i],4)||!Put(profile.declarations[i],profile.declarationBytes[i]))return Fail("Render declaration overflow");
  if(!Put(&count,4)||!Put(record,count*36))return Fail("Render record overflow");for(unsigned int i=0;i<profile.materialCount;i++){unsigned short length=(unsigned short)lstrlenA(profile.materials[i]);if(!Put(&length,2)||!Put(profile.materials[i],length))return Fail("Render material overflow");}
  if(layout){if(!Put(topology.data,topology.size))return Fail("Render topology overflow");}
  else for(unsigned int tree=0;tree<trees;tree++){unsigned int branches=0;for(unsigned int i=0;i<count;i++)if(surfaces[i].tree==tree)branches++;unsigned int node[3]={1,branches,0};if(!Put(node,12))return Fail("Render tree overflow");for(unsigned int i=0;i<count;i++)if(surfaces[i].tree==tree){unsigned int branch[3]={surfaces[i].branchType,0,i};if(!Put(&bounds[i*2],24)||!Put(branch,12))return Fail("Render branch overflow");}}
  if(cursor!=size)return Fail("Render size mismatch");return true;
 }
};
// Caller-supplied metadata and optional multi-node topology with skyPortal polygons.
}
