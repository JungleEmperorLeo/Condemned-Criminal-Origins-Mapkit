#pragma once
// Original SaveLayout 0x62440: bounds, total node count, literal U32 zero,
// then one LSB-first subdivision bit per node in child0..3 preorder.
// Input is explicit prepared topology; source subdivision policy is separate.
namespace WorldTreeSerialization {
static const unsigned int MaximumNodes=1000000u;
struct Node {int children[4];};
class Block {
 PrefabScene::Arena memory;const Node* input;unsigned int inputCount,visited;unsigned char* states;unsigned int* order;
 Block(const Block&);Block& operator=(const Block&);
 bool Fail(const char* message){Clear();error=message;return false;}
 __declspec(noinline) bool Visit(unsigned int index,unsigned int depth){
  if(index>=inputCount||depth>128||states[index])return Fail("Invalid world tree index, cycle or depth");states[index]=1;order[visited++]=index;
  bool split=input[index].children[0]>=0;for(unsigned int c=0;c<4;c++){
   int child=input[index].children[c];if(split){if(child<0||!Visit((unsigned int)child,depth+1))return false;}else if(child!=-1)return Fail("World tree subdivision must have exactly four children");
  }states[index]=2;return true;
 }
public:
 unsigned char* data;unsigned int size,nodeCount;const char* error;
 Block():input(0),inputCount(0),visited(0),states(0),order(0),data(0),size(0),nodeCount(0),error(0){}
 void Clear(){memory.Clear();input=0;inputCount=visited=0;states=0;order=0;data=0;size=nodeCount=0;error=0;}
 bool Build(const WorldText::Vec3& low,const WorldText::Vec3& high,const Node* nodes,unsigned int count,unsigned int root=0){
  Clear();if(!WorldText::FiniteVector(low)||!WorldText::FiniteVector(high)||low.x>high.x||low.y>high.y||low.z>high.z||!nodes||!count||count>MaximumNodes||root>=count)return Fail("Invalid prepared world tree inputs");
  input=nodes;inputCount=count;states=(unsigned char*)memory.Allocate(count);order=(unsigned int*)memory.Allocate(count*4);if(!states||!order)return Fail("World tree topology allocation failed");memset(states,0,count);
  if(!Visit(root,0))return false;if(visited!=count)return Fail("Unreachable world tree nodes");
  nodeCount=count;size=32u+(count+7u)/8u;data=(unsigned char*)memory.Allocate(size);if(!data)return Fail("World tree output allocation failed");memset(data,0,size);memcpy(data,&low,12);memcpy(data+12,&high,12);memcpy(data+24,&count,4);
  for(unsigned int i=0;i<count;i++)if(nodes[order[i]].children[0]>=0)data[32+(i>>3)]|=(unsigned char)(1u<<(i&7));return true;
 }
};
}
