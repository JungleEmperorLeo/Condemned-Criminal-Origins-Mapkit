#pragma once
// Explicit node order and bounds supplied by caller; no spatial partition algorithm here.
namespace RenderTrees {
// meshIndex is the global mesh-list index; surface is local to that mesh.
// Global tree topology follows all mesh entries once. Surface indices are local
// to the referenced mesh; the multi-mesh overload validates that mesh's bound.
struct Branch {WorldText::Vec3 low,high;unsigned int type,meshIndex,surface;};
inline void EmptyBounds(Branch& branch){unsigned int positive=0x7f7fffffu,negative=0xff7fffffu;for(unsigned int a=0;a<3;a++){memcpy((&branch.low.x)+a,&positive,4);memcpy((&branch.high.x)+a,&negative,4);}}
inline bool IsEmptyBounds(const Branch& branch){for(unsigned int a=0;a<3;a++){unsigned int low,high;memcpy(&low,(&branch.low.x)+a,4);memcpy(&high,(&branch.high.x)+a,4);if(low!=0x7f7fffffu||high!=0xff7fffffu)return false;}return true;}
struct SkyPortal {const WorldText::Vec3* vertices;unsigned int count;};
struct Node {const Branch* branches;unsigned int count;const SkyPortal* skyPortals;unsigned int skyPortalCount;};
struct Tree {const Node* nodes;unsigned int count;};
struct Layout {const Tree* trees;unsigned int count;};
class Block {
 unsigned int cursor;
 bool Put(const void* input,unsigned int bytes){if(bytes>size-cursor)return false;memcpy(data+cursor,input,bytes);cursor+=bytes;return true;}
 bool Fail(const char* why){memory.Clear();data=0;size=cursor=nodes=branches=skyPortals=skyPortalVertices=0;error=why;return false;}
public:
 PrefabScene::Arena memory;unsigned char* data;unsigned int size,nodes,branches,skyPortals,skyPortalVertices;const char* error;
 Block():cursor(0),data(0),size(0),nodes(0),branches(0),skyPortals(0),skyPortalVertices(0),error(0){}
 bool Build(const Layout& layout,unsigned int surfaces){
  return Build(layout,&surfaces,1);
 }
 bool Build(const Layout& layout,const unsigned int* meshSurfaces,unsigned int meshCount){
  memory.Clear();data=0;size=cursor=nodes=branches=skyPortals=skyPortalVertices=0;error=0;
  if(!layout.trees||!layout.count||layout.count>4096||!meshSurfaces||!meshCount||meshCount>4096)return Fail("Invalid render tree counts");
  for(unsigned int m=0;m<meshCount;m++)if(meshSurfaces[m]>100000)return Fail("Invalid render mesh surface bound");
  for(unsigned int t=0;t<layout.count;t++){const Tree& tree=layout.trees[t];if((tree.count&&!tree.nodes)||tree.count>65536||tree.count>65536-nodes)return Fail("Invalid render node counts");nodes+=tree.count;
   for(unsigned int n=0;n<tree.count;n++){const Node& node=tree.nodes[n];if(node.count>100000-branches||(node.count&&!node.branches))return Fail("Invalid render branches");branches+=node.count;
    for(unsigned int b=0;b<node.count;b++){const Branch& branch=node.branches[b];bool reversed=branch.low.x>branch.high.x||branch.low.y>branch.high.y||branch.low.z>branch.high.z;if(branch.type>3||branch.meshIndex>=meshCount||branch.surface>=meshSurfaces[branch.meshIndex]||!WorldText::FiniteVector(branch.low)||!WorldText::FiniteVector(branch.high)||(reversed&&!IsEmptyBounds(branch)))return Fail("Invalid render branch bounds or reference");}
    if(node.skyPortalCount>100000-skyPortals||(node.skyPortalCount&&!node.skyPortals))return Fail("Invalid render skyPortal count");skyPortals+=node.skyPortalCount;
    for(unsigned int o=0;o<node.skyPortalCount;o++){const SkyPortal& skyPortal=node.skyPortals[o];if(!skyPortal.vertices||skyPortal.count<3||skyPortal.count>255||skyPortal.count>1000000-skyPortalVertices)return Fail("Invalid render skyPortal vertices");skyPortalVertices+=skyPortal.count;
     for(unsigned int v=0;v<skyPortal.count;v++)if(!WorldText::FiniteVector(skyPortal.vertices[v]))return Fail("Invalid render skyPortal position");
    }
   }
  }
  size=layout.count*4+nodes*8+branches*36+skyPortals+skyPortalVertices*12;data=(unsigned char*)memory.Allocate(size);if(!data)return Fail("Render tree allocation failed");
  for(unsigned int t=0;t<layout.count;t++){const Tree& tree=layout.trees[t];if(!Put(&tree.count,4))return Fail("Render tree overflow");
   for(unsigned int n=0;n<tree.count;n++){const Node& node=tree.nodes[n];unsigned int header[2]={node.count,node.skyPortalCount};if(!Put(header,8))return Fail("Render node overflow");
    for(unsigned int b=0;b<node.count;b++){const Branch& branch=node.branches[b];unsigned int values[3]={branch.type,branch.meshIndex,branch.surface};if(!Put(&branch.low,12)||!Put(&branch.high,12)||!Put(values,12))return Fail("Render branch overflow");}
    for(unsigned int o=0;o<node.skyPortalCount;o++){const SkyPortal& skyPortal=node.skyPortals[o];unsigned char vertices=(unsigned char)skyPortal.count;if(!Put(&vertices,1)||!Put(skyPortal.vertices,skyPortal.count*12))return Fail("Render skyPortal overflow");}
   }
  }
  if(cursor!=size)return Fail("Render tree size mismatch");return true;
 }
};
// Polygon count is one byte; positions retain caller-provided float32 bits/order.
// SourceRenderNodes supplies main-world sector scaffolding and source SkyPortal
// records; source surface branches and other world-model trees remain caller tasks.
}
