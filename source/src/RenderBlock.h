#pragma once
namespace RenderBlock {
struct SurfaceInput {const void* vertices;unsigned int count;const unsigned short* indices;unsigned int triangles;const char* material;unsigned int tree;};
inline bool SameMaterial(const char* a,const char* b){while(*a&&*a==*b){a++;b++;}return *a==*b;}

class Block {
 unsigned int cursor;
 bool Put(const void* p,unsigned int n){if(n>size-cursor)return false;const unsigned char* s=(const unsigned char*)p;for(unsigned int i=0;i<n;i++)data[cursor++]=s[i];return true;}
 bool Fail(const char* message){memory.Clear();data=0;size=cursor=0;error=message;return false;}
public:
 PrefabScene::Arena memory;unsigned char* data;unsigned int size;const char* error;
 Block():cursor(0),data(0),size(0),error(0){}
 bool BuildMultiple(const SurfaceInput* surfaces,unsigned int surfaceCount,unsigned int treeCount,unsigned int stride,const void* declaration,unsigned int declarationBytes){
  memory.Clear();data=0;size=cursor=0;error=0;
  if(!surfaces||!surfaceCount||surfaceCount>1024||!treeCount||treeCount>surfaceCount)return Fail("Unsupported surface/tree count");
  unsigned int* materials=(unsigned int*)memory.Allocate(surfaceCount*4);unsigned int* names=(unsigned int*)memory.Allocate(surfaceCount*4);
  unsigned int* lengths=(unsigned int*)memory.Allocate(surfaceCount*4);unsigned int* branches=(unsigned int*)memory.Allocate(treeCount*4);
  WorldText::Vec3* low=(WorldText::Vec3*)memory.Allocate(surfaceCount*12);WorldText::Vec3* high=(WorldText::Vec3*)memory.Allocate(surfaceCount*12);
  if(!materials||!names||!lengths||!branches||!low||!high)return Fail("Render metadata allocation failed");
  for(unsigned int t=0;t<treeCount;t++)branches[t]=0;
  unsigned int vertexBytes=0,indexBytes=0,materialCount=0,namesBytes=0;Block validator;
  for(unsigned int i=0;i<surfaceCount;i++){
   const SurfaceInput& in=surfaces[i];if(in.tree>=treeCount)return Fail("Surface tree out of range");
   if(!validator.BuildSingle(in.vertices,in.count,stride,in.indices,in.triangles,declaration,declarationBytes,in.material))return Fail("Invalid render surface input");
   unsigned int vb=in.count*stride,ib=in.triangles*6;if(vb>32*1024*1024-vertexBytes||ib>32*1024*1024-indexBytes)return Fail("Render buffers too large");vertexBytes+=vb;indexBytes+=ib;
   unsigned int material=0;for(;material<materialCount;material++)if(SameMaterial(in.material,surfaces[names[material]].material))break;
   if(material==materialCount){unsigned int len=0;while(in.material[len])len++;names[material]=i;lengths[material]=len;namesBytes+=len+2;materialCount++;}materials[i]=material;
   for(unsigned int v=0;v<in.count;v++){const unsigned char* raw=(const unsigned char*)in.vertices+v*stride;WorldText::Vec3 p;unsigned char* dst=(unsigned char*)&p;for(unsigned int k=0;k<12;k++)dst[k]=raw[k];
    if(!v)low[i]=high[i]=p;
    WorldText::Vec3& a=low[i];WorldText::Vec3& b=high[i];if(p.x<a.x)a.x=p.x;if(p.y<a.y)a.y=p.y;if(p.z<a.z)a.z=p.z;if(p.x>b.x)b.x=p.x;if(p.y>b.y)b.y=p.y;if(p.z>b.z)b.z=p.z;
   }branches[in.tree]++;
  }
  for(unsigned int t=0;t<treeCount;t++)if(!branches[t])return Fail("Empty render tree unsupported");
  size=60+vertexBytes+indexBytes+8+declarationBytes+4+surfaceCount*36+namesBytes+treeCount*12+surfaceCount*36;
  if(size>32*1024*1024)return Fail("Render block too large");data=(unsigned char*)memory.Allocate(size);if(!data)return Fail("Render allocation failed");
  unsigned int header[15]={treeCount,treeCount,surfaceCount,1,materialCount,1,surfaceCount,0,0,0,0,surfaceCount,materialCount,vertexBytes,indexBytes};
  if(!Put(header,60))return Fail("Render header overflow");
  for(unsigned int i=0;i<surfaceCount;i++)if(!Put(surfaces[i].vertices,surfaces[i].count*stride))return Fail("Render vertices overflow");
  for(unsigned int i=0;i<surfaceCount;i++)if(!Put(surfaces[i].indices,surfaces[i].triangles*6))return Fail("Render indices overflow");
  unsigned int defs[2]={1,declarationBytes};if(!Put(defs,8)||!Put(declaration,declarationBytes)||!Put(&surfaceCount,4))return Fail("Render declarations overflow");
  unsigned int vbase=0,ibase=0;for(unsigned int i=0;i<surfaceCount;i++){const SurfaceInput& in=surfaces[i];unsigned int surface[9]={vbase,in.count,stride,ibase,vbase,in.triangles,materials[i],0,0};if(!Put(surface,36))return Fail("Render surfaces overflow");vbase+=in.count;ibase+=in.triangles*3;}
  for(unsigned int m=0;m<materialCount;m++){unsigned short len=(unsigned short)lengths[m];if(!Put(&len,2)||!Put(surfaces[names[m]].material,len))return Fail("Render materials overflow");}
  for(unsigned int t=0;t<treeCount;t++){unsigned int node[3]={1,branches[t],0};if(!Put(node,12))return Fail("Render trees overflow");for(unsigned int i=0;i<surfaceCount;i++)if(surfaces[i].tree==t){unsigned int branch[3]={0,0,i};if(!Put(&low[i],12)||!Put(&high[i],12)||!Put(branch,12))return Fail("Render branches overflow");}}
  if(cursor!=size)return Fail("Render size mismatch");return true;
 }
 bool BuildSingle(const void* vertices,unsigned int count,unsigned int stride,const unsigned short* indices,unsigned int triangles,const void* declaration,unsigned int declarationBytes,const char* material){
  memory.Clear();data=0;size=cursor=0;error=0;
  if(!vertices||!indices||!declaration||!material||!material[0]||!count||count>=65536||stride<12||stride>256||!triangles||triangles>1000000||declarationBytes<16||declarationBytes>256||declarationBytes%8)return Fail("Unsupported render input");
  unsigned int nameLength=0;while(material[nameLength]&&nameLength<=4096)nameLength++;if(nameLength>4096)return Fail("Material name too long");
  const unsigned char* decl=(const unsigned char*)declaration;
  for(unsigned int i=0;i<declarationBytes;i+=8){
   unsigned int stream=decl[i]|(decl[i+1]<<8),offset=decl[i+2]|(decl[i+3]<<8),type=decl[i+4];
   if(i+8==declarationBytes){if(stream!=255||type!=17)return Fail("Missing declaration terminator");continue;}
   unsigned int width=type==1?8:type==2?12:type==3?16:type==4?4:0;
   if(stream||!width||offset+width>stride)return Fail("Unsupported vertex element");
  }
  if(decl[2]||decl[3]||decl[4]!=2||decl[6])return Fail("Position must be first float3");
  WorldText::Vec3 low={},high={};
  for(unsigned int i=0;i<count;i++){
   const unsigned char* raw=(const unsigned char*)vertices+i*stride;WorldText::Vec3 v;unsigned char* b=(unsigned char*)&v;for(unsigned int j=0;j<12;j++)b[j]=raw[j];
   if(!WorldText::FiniteVector(v))return Fail("Invalid render position");if(!i)low=high=v;
   if(v.x<low.x)low.x=v.x;if(v.y<low.y)low.y=v.y;if(v.z<low.z)low.z=v.z;
   if(v.x>high.x)high.x=v.x;if(v.y>high.y)high.y=v.y;if(v.z>high.z)high.z=v.z;
  }
  for(unsigned int i=0;i<triangles*3;i++)if(indices[i]>=count)return Fail("Render index out of range");
  unsigned int vertexBytes=count*stride,indexBytes=triangles*6;
  size=60+vertexBytes+indexBytes+8+declarationBytes+40+2+nameLength+48;
  if(size>32*1024*1024)return Fail("Render block too large");data=(unsigned char*)memory.Allocate(size);if(!data)return Fail("Render allocation failed");
  unsigned int header[15]={1,1,1,1,1,1,1,0,0,0,0,1,1,vertexBytes,indexBytes};
  unsigned int defs[2]={1,declarationBytes},surface[10]={1,0,count,stride,0,0,triangles,0,0,0};unsigned short nameBytes=(unsigned short)nameLength;
  unsigned int tree[3]={1,1,0},branch[3]={0,0,0};
  if(!Put(header,60)||!Put(vertices,vertexBytes)||!Put(indices,indexBytes)||!Put(defs,8)||!Put(declaration,declarationBytes)||!Put(surface,40)||!Put(&nameBytes,2)||!Put(material,nameLength)||!Put(tree,12)||!Put(&low,12)||!Put(&high,12)||!Put(branch,12)||cursor!=size)return Fail("Render write overflow");
  return true;
 }
};
// Shared vertex declaration/stride, visible surfaces, one node per explicit tree; no shadows or occluders.
// Attribute generation and FEAR surface partitioning are not implemented here.
}
