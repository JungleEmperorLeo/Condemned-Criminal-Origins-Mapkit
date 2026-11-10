#pragma once
#include "RenderConnectedSectionOrder.h"
#include "RenderVisibilityPortalGraph.h"
namespace RenderGlobalSectionOrder {
// Apply the recovered main-world material-local portal order to caller-owned
// flattened sections. Other trees retain the original UINT_MAX default. The caller
// identifies the main tree explicitly; no object/map-name heuristic is used.
class Builder {
 PrefabScene::Arena memory;
 Builder(const Builder&);Builder& operator=(const Builder&);
public:
 const char* error;unsigned int assigned,materials;
 Builder():error(0),assigned(0),materials(0){}
 template<class Input> bool Apply(const Input* input,unsigned int count,unsigned int firstNode,const RenderVisibilityPortalGraph::Graph& graph,unsigned int* keys){
  memory.Clear();error=0;assigned=materials=0;
  // An empty main tree (no visibility sectors) has no portal order: all keep UINT_MAX.
  if(input&&keys&&count&&count<=4096&&!graph.error&&!graph.count){for(unsigned int i=0;i<count;i++)keys[i]=0xffffffffu;return true;}
  if(!input||!keys||!count||count>4096||graph.error||!graph.count||!graph.nodes||!graph.neighbors||firstNode>65536||graph.count>65536-firstNode){error="Invalid global section order inputs";return false;}
  RenderConnectedSectionOrder::Node* nodes=(RenderConnectedSectionOrder::Node*)memory.Allocate(graph.count*sizeof(RenderConnectedSectionOrder::Node));
  RenderConnectedSectionOrder::Section* sections=(RenderConnectedSectionOrder::Section*)memory.Allocate(count*sizeof(RenderConnectedSectionOrder::Section));
  unsigned int* source=(unsigned int*)memory.Allocate(count*4);unsigned int* order=(unsigned int*)memory.Allocate(count*4);
  if(!nodes||!sections||!source||!order){error="Global section order allocation failed";return false;}
  for(unsigned int i=0;i<count;i++)keys[i]=0xffffffffu;
  unsigned int used=0;
  for(unsigned int n=0;n<graph.count;n++){
   RenderConnectedSectionOrder::Node& node=nodes[n];node.sectionStart=used;node.sectionCount=0;node.neighborStart=graph.nodes[n].neighborStart;node.neighborCount=graph.nodes[n].neighborCount;
   for(unsigned int i=0;i<count;i++)if(input[i].node==firstNode+n){
    const char* material=input[i].range.material;if(!material||!material[0]){error="Missing connected section material";return false;}
    unsigned int length=0;while(material[length]){if(length>=4096||(unsigned char)material[length]>=128){error="Connected material name exceeds supported ASCII bound";return false;}length++;}
    sections[used].material=material;source[used]=i;order[used++]=0xffffffffu;node.sectionCount++;
   }
  }
  RenderConnectedSectionOrder::Builder connected;
  for(unsigned int i=0;i<used;i++){
   bool existing=false;for(unsigned int j=0;j<i;j++)if(RenderConnectedSectionOrder::Equal(sections[i].material,sections[j].material)){existing=true;break;}if(existing)continue;
   if(!connected.Assign(nodes,graph.count,sections,used,graph.neighbors,graph.edgeCount,sections[i].material,order)){error=connected.error;return false;}assigned+=connected.assigned;materials++;
  }
  if(assigned!=used){error="Incomplete material-local section order";return false;}
  for(unsigned int i=0;i<used;i++){if(order[i]==0xffffffffu){error="Unassigned connected section";return false;}keys[source[i]]=order[i];}return true;
 }
};
struct MainPortalGraph {const RenderVisibilityPortalGraph::Graph* graph;unsigned int tree;};
}
