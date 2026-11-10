#pragma once
namespace StreamingTree {
struct Node {unsigned int axis;float split;int childA,childB;const unsigned int* convexIds;unsigned int convexCount;};
class Block {
 const Node* nodes;unsigned int count,convexLimit,totalBytes,next;unsigned int* order;unsigned int* index;unsigned char* state;
 bool Visit(unsigned int id,unsigned int depth){
  if(id>=count||depth>128||state[id])return false;state[id]=1;index[id]=next;order[next++]=id;const Node& n=nodes[id];
  unsigned int bits=*(const unsigned int*)&n.split;if(n.axis>3||(bits&0x7f800000)==0x7f800000||n.convexCount>100000||(!n.convexIds&&n.convexCount))return false;
  if(n.axis==3&&(n.childA!=-1||n.childB!=-1))return false;
  if(n.axis!=3&&n.childA==-1&&n.childB==-1)return false;
  if(totalBytes>16*1024*1024-20-n.convexCount*4)return false;totalBytes+=20+n.convexCount*4;
  for(unsigned int j=0;j<n.convexCount;j++)if(n.convexIds[j]>=convexLimit)return false;
  if(n.childA!=-1&&(n.childA<0||!Visit((unsigned int)n.childA,depth+1)))return false;
  if(n.childB!=-1&&(n.childB<0||!Visit((unsigned int)n.childB,depth+1)))return false;
  state[id]=2;return true;
 }
 bool Fail(){data=0;size=0;error="Invalid streaming tree";return false;}
public:
 PrefabScene::Arena memory;unsigned char* data;unsigned int size;const char* error;
 Block():nodes(0),count(0),convexLimit(0),totalBytes(0),next(0),order(0),index(0),state(0),data(0),size(0),error(0){}
 bool Build(const Node* input,unsigned int nodeCount,unsigned int root,unsigned int convexCount){
  memory.Clear();data=0;size=0;error=0;nodes=input;count=nodeCount;convexLimit=convexCount;totalBytes=0;next=0;
  if(!input||!count||count>100000)return Fail();
  order=(unsigned int*)memory.Allocate(count*4);index=(unsigned int*)memory.Allocate(count*4);state=(unsigned char*)memory.Allocate(count);if(!order||!index||!state)return Fail();for(unsigned int i=0;i<count;i++)state[i]=0;
  if(!Visit(root,0)||next!=count)return Fail();data=(unsigned char*)memory.Allocate(totalBytes);if(!data)return Fail();size=totalBytes;unsigned int* out=(unsigned int*)data;
  for(unsigned int i=0;i<count;i++){const Node& n=nodes[order[i]];*out++=n.convexCount;for(unsigned int j=0;j<n.convexCount;j++)*out++=n.convexIds[j];*out++=n.axis;*out++=*(const unsigned int*)&n.split;*out++=n.childA==-1?0xffffffff:index[n.childA];*out++=n.childB==-1?0xffffffff:index[n.childB];}
  return true;
 }
};
// Serializer. Source geometry and split selection are connected by StreamingTreeBuilder.h.
}

