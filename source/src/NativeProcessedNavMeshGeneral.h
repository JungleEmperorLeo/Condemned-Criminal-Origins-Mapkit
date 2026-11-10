#pragma once
#include "NativeProcessedNavMesh.h"
// Bounded structural acceptance of processed v6 for the general source route.
// The general route has no independent geometric proof: its raw follows the
// recovered original construction and its processing uses the SDK generator
// with the observed original arithmetic. This validator mirrors
// nav-sdk-native/processed_navmesh.py: framing, counted streams bounded by the
// remaining bytes, finite floats, index ranges, contiguous list partitions,
// edge incidence, an acyclic single-root quadtree, authored parent identities
// 1..roots and an exact raw witness of the current source scene.
namespace NativeProcessedNavMeshGeneral {
namespace detail {
struct Span {const unsigned char* data;unsigned int count,stride;const unsigned char* Row(unsigned int i)const{return data+i*stride;}};
inline unsigned int Word(const unsigned char* p){unsigned int v;memcpy(&v,p,4);return v;}
inline bool Finite(const unsigned char* p,unsigned int floats){for(unsigned int i=0;i<floats;i++){float v;memcpy(&v,p+4*i,4);if(!WorldText::Finite(v))return false;}return true;}
struct Reader {const unsigned char* data;unsigned int size,at;
 bool Take(unsigned int n,const unsigned char*& p){if(n>size-at)return false;p=data+at;at+=n;return true;}
 bool Array(Span& s,unsigned int stride){const unsigned char* p;if(!Take(4,p))return false;unsigned int count=Word(p);if(count>(size-at)/stride)return false;s.count=count;s.stride=stride;s.data=data+at;at+=count*stride;return true;}};
inline bool Partition(const Span& rows,unsigned int startAt,unsigned int countAt,unsigned int total){unsigned int next=0;for(unsigned int i=0;i<rows.count;i++){unsigned int start=Word(rows.Row(i)+startAt),count=Word(rows.Row(i)+countAt);if(start!=next||count>total-next)return false;next+=count;}return next==total;}
// Original compaction leaves detached export slots after deleting empty nodes.
// Accept only the exact cleared representation; references are checked below.
inline bool DeletedTreeSlot(const unsigned char* t){
 if(!Word(t)||Word(t+28))return false;
 for(unsigned int i=0;i<3;i++)if(Word(t+4+4*i)!=0x7f7fffffu||Word(t+16+4*i)!=0xff7fffffu)return false;
 for(unsigned int i=0;i<4;i++)if(Word(t+36+4*i)!=0xffffffffu)return false;
 return true;
}
}
inline bool Validate(const unsigned char* raw,unsigned int rawSize,unsigned int roots,const NativeProcessedNavMesh::Input& input,const char*& error){
 using namespace detail;error=0;
#define NAV_GENERAL_REQUIRE(c,m) do{if(!(c)){error=m;return false;}}while(0)
 NAV_GENERAL_REQUIRE(raw&&rawSize&&input.sourceRaw&&input.sourceRawBytes==rawSize&&input.processed&&input.bytes>=12&&input.bytes<=16u*1024u*1024u&&roots>=1&&roots<=65536,"General navigation requires bounded source witness and processed input");
 for(unsigned int i=0;i<rawSize;i++)NAV_GENERAL_REQUIRE(((const unsigned char*)input.sourceRaw)[i]==raw[i],"General navigation raw witness differs from the current source scene");
 Reader r={(const unsigned char*)input.processed,input.bytes,0};const unsigned char* p;
 NAV_GENERAL_REQUIRE(r.Take(12,p)&&Word(p)==1&&Word(p+4)==6&&Word(p+8)==0,"General navigation requires processed v6 unfixed framing");
 Span edges,edgeRefs,regionRefs,normals,polys,regionPolys,regions,components,neighbors,links,boundary,nodes,tree,treePolys;
 NAV_GENERAL_REQUIRE(r.Array(edges,72)&&r.Array(edgeRefs,4)&&r.Array(regionRefs,4)&&r.Array(normals,12)&&r.Array(polys,72)&&r.Array(regionPolys,4)&&r.Array(regions,28)&&r.Array(components,16)&&r.Array(neighbors,4)&&r.Array(links,12)&&r.Array(boundary,12)&&r.Array(nodes,8),"General navigation counted streams exceed the processed byte bound");
 NAV_GENERAL_REQUIRE(r.Take(4,p),"General navigation node-name pool is truncated");unsigned int nameBytes=Word(p);const unsigned char* names;NAV_GENERAL_REQUIRE(r.Take(nameBytes,names),"General navigation node-name pool exceeds the processed byte bound");
 NAV_GENERAL_REQUIRE(r.Array(tree,52)&&r.Array(treePolys,4)&&r.at==r.size,"General navigation quadtree streams or final cursor are invalid");
  NAV_GENERAL_REQUIRE(polys.count>=1&&edges.count>=3&&normals.count>=1&&components.count>=1,"General navigation requires nonempty polygon, edge, normal and component streams");
 const unsigned int invalid=0xffffffffu;
 for(unsigned int i=0;i<edges.count;i++){const unsigned char* e=edges.Row(i);NAV_GENERAL_REQUIRE(Word(e)==i&&Word(e+4)<=1&&Word(e+20)<=1&&Word(e+36)<=1&&Finite(e+8,3)&&Finite(e+24,3)&&Finite(e+40,3)&&Finite(e+60,3)&&Word(e+52)<polys.count&&(Word(e+56)==invalid||Word(e+56)<polys.count),"General navigation edge row is invalid");}
 for(unsigned int i=0;i<edgeRefs.count;i++)NAV_GENERAL_REQUIRE(Word(edgeRefs.Row(i))<edges.count,"General navigation edge reference is outside its array");
 for(unsigned int i=0;i<normals.count;i++)NAV_GENERAL_REQUIRE(Finite(normals.Row(i),3),"General navigation normal is nonfinite");
 for(unsigned int i=0;i<regions.count;i++){const unsigned char* g=regions.Row(i);NAV_GENERAL_REQUIRE(Word(g)!=invalid&&Finite(g+12,4),"General navigation region row is invalid");float radius;memcpy(&radius,g+24,4);NAV_GENERAL_REQUIRE(radius>=0,"General navigation region radius is negative");NAV_GENERAL_REQUIRE(Word(g+4)!=0,"General navigation region without polygons has an uninitialized original sphere");for(unsigned int j=0;j<i;j++)NAV_GENERAL_REQUIRE(Word(regions.Row(j))!=Word(g),"General navigation region identities repeat");}
 for(unsigned int i=0;i<links.count;i++){NAV_GENERAL_REQUIRE(Word(links.Row(i))!=invalid,"General navigation link identity is invalid");for(unsigned int j=0;j<i;j++)NAV_GENERAL_REQUIRE(Word(links.Row(j))!=Word(links.Row(i)),"General navigation link identities repeat");}
 for(unsigned int i=0;i<regionRefs.count;i++){unsigned int id=Word(regionRefs.Row(i));bool found=false;for(unsigned int j=0;j<regions.count&&!found;j++)found=Word(regions.Row(j))==id;NAV_GENERAL_REQUIRE(found,"General navigation region reference is absent");}
 for(unsigned int i=0;i<polys.count;i++){const unsigned char* q=polys.Row(i);unsigned int link=Word(q+12),parent=Word(q+16);bool linked=link==invalid;for(unsigned int j=0;j<links.count&&!linked;j++)linked=Word(links.Row(j))==link;
  float lo[3],hi[3];memcpy(lo,q+32,12);memcpy(hi,q+44,12);
   NAV_GENERAL_REQUIRE(Word(q)==i&&Word(q+4)<normals.count&&Word(q+8)<components.count&&linked&&parent>=1&&parent<=roots&&Finite(q+20,9)&&lo[0]<=hi[0]&&lo[1]<=hi[1]&&lo[2]<=hi[2]&&Word(q+56)>=3,"General navigation polygon row is invalid");}
 for(unsigned int i=0;i<regionPolys.count;i++)NAV_GENERAL_REQUIRE(Word(regionPolys.Row(i))<polys.count,"General navigation region polygon reference is outside its array");
 for(unsigned int i=0;i<treePolys.count;i++)NAV_GENERAL_REQUIRE(Word(treePolys.Row(i))<polys.count,"General navigation quadtree polygon reference is outside its array");
 NAV_GENERAL_REQUIRE(Partition(polys,60,56,edgeRefs.count)&&Partition(polys,68,64,regionRefs.count)&&Partition(regions,8,4,regionPolys.count)&&Partition(components,12,8,neighbors.count)&&Partition(links,8,4,boundary.count)&&Partition(tree,32,28,treePolys.count),"General navigation list partitions are not contiguous");
 for(unsigned int i=0;i<polys.count;i++){const unsigned char* q=polys.Row(i);for(unsigned int k=0;k<Word(q+56);k++){const unsigned char* e=edges.Row(Word(edgeRefs.Row(Word(q+60)+k)));NAV_GENERAL_REQUIRE(Word(e+52)==i||Word(e+56)==i,"General navigation polygon uses a nonincident edge");}}
 for(unsigned int i=0;i<components.count;i++){const unsigned char* c=components.Row(i);NAV_GENERAL_REQUIRE(Word(c)==i&&(Word(c+4)==invalid||Word(c+4)<components.count),"General navigation component row is invalid");}
 for(unsigned int i=0;i<neighbors.count;i++)NAV_GENERAL_REQUIRE(Word(neighbors.Row(i))<components.count,"General navigation component neighbor is outside its array");
 for(unsigned int i=0;i<boundary.count;i++)NAV_GENERAL_REQUIRE(Finite(boundary.Row(i),3),"General navigation link boundary vertex is nonfinite");
 unsigned int expected=0;for(unsigned int i=0;i<nodes.count;i++){const unsigned char* n=nodes.Row(i);NAV_GENERAL_REQUIRE(Word(n)==expected&&(Word(n+4)==invalid||Word(n+4)<nodes.count),"General navigation node row is invalid");unsigned int end=expected;while(end<nameBytes&&names[end])end++;NAV_GENERAL_REQUIRE(end<nameBytes,"General navigation node name is not terminated");expected=end+1;}
 unsigned int pad=4-expected%4;NAV_GENERAL_REQUIRE(nameBytes==expected+pad,"General navigation node-name padding is invalid");for(unsigned int i=expected;i<nameBytes;i++)NAV_GENERAL_REQUIRE(!names[i],"General navigation node-name padding is not zero");
 NAV_GENERAL_REQUIRE(tree.count>=1&&tree.count<=1000000,"General navigation quadtree is empty or unbounded");
 PrefabScene::Arena memory;unsigned int* parents=(unsigned int*)memory.Allocate(tree.count*4);unsigned int* queue=(unsigned int*)memory.Allocate(tree.count*4);NAV_GENERAL_REQUIRE(parents&&queue,"General navigation quadtree allocation failed");
 for(unsigned int i=0;i<tree.count;i++)parents[i]=0;
 unsigned int deletedSlots=0;
 for(unsigned int i=0;i<tree.count;i++){const unsigned char* t=tree.Row(i);float lo[3],hi[3];memcpy(lo,t+4,12);memcpy(hi,t+16,12);bool deleted=DeletedTreeSlot(t);NAV_GENERAL_REQUIRE(Word(t)==i&&Finite(t+4,6)&&(deleted||(lo[0]<=hi[0]&&lo[1]<=hi[1]&&lo[2]<=hi[2])),"General navigation quadtree row is invalid");if(deleted)deletedSlots++;for(unsigned int j=0;j<4;j++){unsigned int c=Word(t+36+4*j);if(c==invalid)continue;NAV_GENERAL_REQUIRE(c<tree.count&&++parents[c]==1,"General navigation quadtree child is invalid or shared");}}
 for(unsigned int i=1;i<tree.count;i++)NAV_GENERAL_REQUIRE(parents[i]==(DeletedTreeSlot(tree.Row(i))?0u:1u),"General navigation quadtree has an orphan live row or a referenced deleted slot");
 NAV_GENERAL_REQUIRE(!parents[0],"General navigation quadtree root has a parent");unsigned int head=0,end=1;queue[0]=0;
 while(head<end){const unsigned char* t=tree.Row(queue[head++]);for(unsigned int j=0;j<4;j++){unsigned int c=Word(t+36+4*j);if(c==invalid)continue;NAV_GENERAL_REQUIRE(end<tree.count,"General navigation quadtree traversal is cyclic");queue[end++]=c;}}
 NAV_GENERAL_REQUIRE(end==tree.count-deletedSlots,"General navigation quadtree has unreachable live rows");
#undef NAV_GENERAL_REQUIRE
 return true;
}
}
