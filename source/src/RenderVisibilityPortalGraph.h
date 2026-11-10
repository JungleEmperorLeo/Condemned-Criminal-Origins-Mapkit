#pragma once
// Ordered adjacency from the independently serialized VisibilityCompiler data.
// The original connected section pass uses portal references in this order.
namespace RenderVisibilityPortalGraph {
struct Node {unsigned int neighborStart,neighborCount;};
class Graph {
 PrefabScene::Arena memory;const unsigned char* input;unsigned int length,cursor;
 Graph(const Graph&);Graph& operator=(const Graph&);
 bool Fail(const char* message){memory.Clear();count=edgeCount=0;nodes=0;neighbors=0;error=message;return false;}
 bool Take(unsigned int n,const unsigned char** p){if(n>length-cursor)return false;*p=input+cursor;cursor+=n;return true;}
 bool Skip(unsigned int n){const unsigned char* p;return Take(n,&p);}
 bool U16(unsigned int* n){const unsigned char* p;if(!Take(2,&p))return false;*n=p[0]|((unsigned int)p[1]<<8);return true;}
 bool U32(unsigned int* n){const unsigned char* p;if(!Take(4,&p))return false;*n=*(const unsigned int*)p;return true;}
public:
 unsigned int count,edgeCount;Node* nodes;unsigned int* neighbors;const char* error;
 Graph():input(0),length(0),cursor(0),count(0),edgeCount(0),nodes(0),neighbors(0),error(0){}
 bool Build(const unsigned char* data,unsigned int bytes){
  Fail(0);input=data;length=bytes;cursor=0;if(!data||bytes<32||bytes>16*1024*1024)return Fail("Invalid source visibility bytes");
  unsigned int h[8];for(unsigned int i=0;i<8;i++)if(!U32(&h[i]))return Fail("Truncated visibility header");
  if(h[0]>4096||h[1]>100000||h[7]!=h[1]*2)return Fail("Visibility graph count bound or reference count mismatch");count=h[0];
  unsigned int vertices=0;for(unsigned int i=0;i<h[1];i++){unsigned int n;if(!U16(&n)||n<3||n>128||!Skip(n*12+32))return Fail("Invalid portal contour prefix");vertices+=n;}if(vertices!=h[4])return Fail("Visibility portal vertex count mismatch");
  unsigned int* owners=(unsigned int*)memory.Allocate((h[1]+1)*8);unsigned int* ownerCount=(unsigned int*)memory.Allocate((h[1]+1)*4);nodes=(Node*)memory.Allocate((count+1)*sizeof(Node));neighbors=(unsigned int*)memory.Allocate((h[7]+1)*4);if(!owners||!ownerCount||!nodes||!neighbors)return Fail("Visibility graph allocation failed");
  for(unsigned int p=0;p<h[1];p++)ownerCount[p]=0;unsigned int outlines=0;
  for(unsigned int n=0;n<count;n++){
   unsigned int nameLength,planes,refs,id;if(!U16(&nameLength)||nameLength>4095||!Skip(nameLength+24)||!U32(&planes)||planes>128||!Skip(planes*16)||!U32(&refs)||refs>h[7]-edgeCount)return Fail("Invalid source visibility sector record");outlines+=planes;
   nodes[n].neighborStart=edgeCount;nodes[n].neighborCount=refs;
   for(unsigned int i=0;i<refs;i++){unsigned int p;if(!U32(&p)||p>=h[1]||ownerCount[p]>=2)return Fail("Invalid source sector portal reference");owners[p*2+ownerCount[p]++]=n;neighbors[edgeCount++]=p;}
   if(!U32(&id)||id!=n)return Fail("Visibility save ID order mismatch");
  }
  if(edgeCount!=h[7]||outlines!=h[6])return Fail("Visibility graph aggregate mismatch");
  for(unsigned int p=0;p<h[1];p++)if(ownerCount[p]!=2||owners[p*2]==owners[p*2+1])return Fail("Portal must connect two different source sectors");
  for(unsigned int n=0;n<count;n++)for(unsigned int i=0;i<nodes[n].neighborCount;i++){unsigned int& ref=neighbors[nodes[n].neighborStart+i];ref=owners[ref*2]==n?owners[ref*2+1]:owners[ref*2];}
  return true;
 }
};
}
