#pragma once
namespace RenderTriangleRouting {
// Stable routing of already prepared records. The caller derives geometry and
// sector ownership; this module never dereferences original runtime pointers.
static const unsigned int Unassigned=0xffffffff;
template<class Triangle> class Builder {
 PrefabScene::Arena memory;
 bool Fail(const char* message){memory.Clear();nodes=0;nodeCount=assigned=skipped=0;error=message;return false;}
 Builder(const Builder&);Builder& operator=(const Builder&);
public:
 struct Input {const Triangle* triangle;unsigned int owner;};
 struct Node {const Triangle* triangles;unsigned int count;};
 Node* nodes;unsigned int nodeCount,assigned,skipped;const char* error;
 Builder():nodes(0),nodeCount(0),assigned(0),skipped(0),error(0){}
 bool Build(const Input* input,unsigned int count,unsigned int sectors){
  Fail(0);if(!sectors||sectors>65536||count>1000000||(count&&!input))return Fail("Invalid triangle routing counts");
  nodes=(Node*)memory.Allocate(sectors*sizeof(Node));if(!nodes)return Fail("Triangle node allocation failed");
  for(unsigned int i=0;i<sectors;i++){nodes[i].triangles=0;nodes[i].count=0;}
  for(unsigned int i=0;i<count;i++){
   if(input[i].owner==Unassigned){skipped++;continue;}
   if(input[i].owner>=sectors||!input[i].triangle)return Fail("Invalid triangle owner or record");
   nodes[input[i].owner].count++;assigned++;
  }
  if(assigned>64*1024*1024/sizeof(Triangle))return Fail("Triangle routing storage limit");
  Triangle* records=(Triangle*)memory.Allocate((assigned?assigned:1)*sizeof(Triangle));if(!records)return Fail("Triangle routing allocation failed");
  unsigned int at=0;for(unsigned int i=0;i<sectors;i++){if(nodes[i].count)nodes[i].triangles=records+at;at+=nodes[i].count;nodes[i].count=0;}
  for(unsigned int i=0;i<count;i++)if(input[i].owner!=Unassigned){Node& node=nodes[input[i].owner];Triangle* output=(Triangle*)node.triangles;output[node.count++]=*input[i].triangle;}
  nodeCount=sectors;return true;
 }
};
}
