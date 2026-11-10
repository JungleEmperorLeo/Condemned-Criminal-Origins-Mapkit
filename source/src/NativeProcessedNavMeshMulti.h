#pragma once
#include "NativeProcessedNavMesh.h"
// Independent finite acceptance for a 4x4 source cell lattice. Multiple roots
// currently require equal NavMeshType strings and empty character restrictions.
// It verifies the processed partition against the source union, rather than
// requiring the exporter to preserve one polygon per authored brush.
namespace NativeProcessedNavMeshMulti {
namespace detail {
using NativeProcessedNavMesh::detail::Word;
using NativeProcessedNavMesh::detail::Float;
using NativeProcessedNavMesh::detail::F32;
using NativeProcessedNavMesh::detail::VectorEqual;
using NativeProcessedNavMesh::detail::FiniteVector;
using NativeProcessedNavMesh::detail::Span;
enum {MaximumPolys=128,MaximumEdges=512,MaximumRefs=2048,MaximumVertices=64,MaximumTree=341,MaximumTreeRefs=32768,ClipCapacity=256};
struct Point {double x,z;};
struct Cell {unsigned int x,z,root,component;};
struct Poly {Point* points;unsigned int count,component;WorldText::Vec3 low,high;double area;};
struct Edge {unsigned int a,b,key0,key1,refs;};
inline double Abs(double x){return x<0?-x:x;}
inline double Cross(const Point& a,const Point& b,const Point& c){return (b.x-a.x)*(c.z-a.z)-(b.z-a.z)*(c.x-a.x);}
inline double Area(const Point* p,unsigned int n){double area=0;for(unsigned int i=1;i+1<n;i++)area+=Cross(p[0],p[i],p[i+1]);return area*.5;}
inline bool Same(const Point& a,const Point& b){return a.x==b.x&&a.z==b.z;}
class Reader {
 const unsigned char* data;unsigned int size,at;
public:
 Reader(const void* p,unsigned int n):data((const unsigned char*)p),size(n),at(0){}
 bool ReadWord(unsigned int& v){if(at>size||4u>size-at)return false;v=Word(data+at);at+=4;return true;}
 bool Array(Span& span,unsigned int stride,unsigned int maximum){unsigned int count;if(!ReadWord(count)||count>maximum||!stride||at>size||count>(size-at)/stride)return false;span.data=data+at;span.count=count;span.stride=stride;at+=count*stride;return true;}
 bool End()const{return at==size;}
};
// All geometry here is normalized to the source's small integer 0..4 lattice.
// Clockwise convex clipping avoids world-coordinate cancellation. Scratch is
// caller-owned arena storage, so no >4KB stack frame or CRT stack helper.
inline bool Intersection(const Point* subject,unsigned int count,const Point* clip,unsigned int clips,Point* first,Point* second,double& area){
 if(count>ClipCapacity)return false;for(unsigned int i=0;i<count;i++)first[i]=subject[i];Point *in=first,*out=second;
 for(unsigned int edge=0;edge<clips&&count;edge++){
  unsigned int wrote=0;Point previous=in[count-1];double prior=Cross(clip[edge],clip[(edge+1)%clips],previous);
  for(unsigned int i=0;i<count;i++){
   Point current=in[i];double side=Cross(clip[edge],clip[(edge+1)%clips],current);
   if((prior<=0)!=(side<=0)){double denominator=prior-side;if(denominator==0||wrote==ClipCapacity)return false;double t=prior/denominator;out[wrote].x=previous.x+(current.x-previous.x)*t;out[wrote++].z=previous.z+(current.z-previous.z)*t;}
   if(side<=0){if(wrote==ClipCapacity)return false;out[wrote++]=current;}previous=current;prior=side;
  }count=wrote;Point* swap=in;in=out;out=swap;
 }area=count>=3?Abs(Area(in,count)):0;return true;
}
inline bool VertexKey(const unsigned char* p,const WorldText::Vec3* source,unsigned int vertices,const WorldText::Vec3& low,double width,double depth,unsigned int& key){
 if(!FiniteVector(p))return false;bool found=false;for(unsigned int i=0;i<vertices;i++)if(VectorEqual(p,source[i])){found=true;break;}if(!found)return false;
  double x=(double(Float(p))-low.x)/width,z=(double(Float(p+8))-low.z)/depth;if(x<0||x>4||z<0||z>4)return false;int ix=(int)x,iz=(int)z;if(x!=ix||z!=iz)return false;key=(unsigned int)(ix+iz*5);return true;
}
inline Point FromKey(unsigned int key){Point p={double(key%5),double(key/5)};return p;}
}
inline bool ValidateRootSet(const NativeRawNavMesh::Builder& raw,const NativeProcessedNavMesh::Input& input,const char*& error,unsigned int rootCount){
 using namespace detail;error=0;
#define NAV_MULTI_REQUIRE(condition,message) do{if(!(condition)){error=message;return false;}}while(0)
 NAV_MULTI_REQUIRE(rootCount>=1&&rootCount<=4&&raw.data&&raw.size>=192&&raw.size<=16u*1024u*1024u&&input.sourceRaw&&input.sourceRawBytes==raw.size&&input.processed&&input.bytes>=76&&input.bytes<=16u*1024u*1024u,"Multi navigation requires bounded roots, source witness and processed bytes");
 for(unsigned int i=0;i<raw.size;i++)NAV_MULTI_REQUIRE(((const unsigned char*)input.sourceRaw)[i]==raw.data[i],"Multi navigation raw witness differs from current source scene");
 unsigned int cells=Word(raw.data+32);NAV_MULTI_REQUIRE(Word(raw.data)==6&&Word(raw.data+28)==rootCount&&cells>=2&&cells<=16&&cells>=rootCount&&raw.size>=70u+60u*cells,"Multi navigation source root/brush count is outside the lattice profile");
 unsigned int tail=36+60*cells;for(unsigned int i=0;i<4;i++)NAV_MULTI_REQUIRE(Word(raw.data+tail+i*4)==0,"Multi navigation auxiliary source families remain unsupported");
 NAV_MULTI_REQUIRE(Word(raw.data+tail+16)==rootCount,"Multi navigation source restriction count differs from source roots");
 unsigned int restrictionAt=tail+20,firstNameAt=0,firstNameBytes=0;
 for(unsigned int r=0;r<rootCount;r++){
  NAV_MULTI_REQUIRE(restrictionAt<=raw.size&&raw.size-restrictionAt>=14&&Word(raw.data+restrictionAt)==r+1,"Multi navigation source restriction identity/order is invalid");
  unsigned int nameBytes=(unsigned int)raw.data[restrictionAt+4]|((unsigned int)raw.data[restrictionAt+5]<<8);restrictionAt+=6;
  NAV_MULTI_REQUIRE(nameBytes>=1&&nameBytes<=63&&nameBytes<=raw.size-restrictionAt&&raw.size-restrictionAt-nameBytes>=8,"Multi navigation source type string exceeds its bounded row");
  for(unsigned int i=0;i<nameBytes;i++)NAV_MULTI_REQUIRE(raw.data[restrictionAt+i]>=32&&raw.data[restrictionAt+i]<=126,"Multi navigation source type string is not printable ASCII");
  if(!r){firstNameAt=restrictionAt;firstNameBytes=nameBytes;}else{NAV_MULTI_REQUIRE(nameBytes==firstNameBytes,"Multi navigation roots require identical source type strings");for(unsigned int i=0;i<nameBytes;i++)NAV_MULTI_REQUIRE(raw.data[restrictionAt+i]==raw.data[firstNameAt+i],"Multi navigation roots require identical source type strings");}
  restrictionAt+=nameBytes;NAV_MULTI_REQUIRE(Word(raw.data+restrictionAt)<=1&&!Word(raw.data+restrictionAt+4),"Multi registered navigation requires canonical mask flags and empty character restrictions");restrictionAt+=8;
 }
 NAV_MULTI_REQUIRE(restrictionAt==raw.size,"Multi navigation source has trailing restriction data");
 PrefabScene::Arena memory;WorldText::Vec3* source=(WorldText::Vec3*)memory.Allocate(cells*4*12);Cell* sourceCells=(Cell*)memory.Allocate(cells*sizeof(Cell));NAV_MULTI_REQUIRE(source&&sourceCells,"Multi navigation source geometry allocation failed");
 double cellWidth=0,cellDepth=0;unsigned int occupied=0,previousRoot=1;float y=0;
 for(unsigned int c=0;c<cells;c++){
  const unsigned char* row=raw.data+36+c*60;unsigned int parent=Word(row);NAV_MULTI_REQUIRE(parent>=1&&parent<=rootCount&&(c?(parent==previousRoot||parent==previousRoot+1):parent==1)&&Word(row+4)==0xffffffffu&&Word(row+8)==4,"Multi navigation source row is not a grouped unlinked root quad");previousRoot=parent;WorldText::Vec3 low={},high={};
  for(unsigned int i=0;i<4;i++){WorldText::Vec3& p=source[c*4+i];memcpy(&p,row+12+i*12,12);NAV_MULTI_REQUIRE(WorldText::FiniteVector(p)&&p.x>=-1048576.f&&p.x<=1048576.f&&p.y>=-1048576.f&&p.y<=1048576.f&&p.z>=-1048576.f&&p.z<=1048576.f&&p.x==(float)(int)p.x&&p.y==(float)(int)p.y&&p.z==(float)(int)p.z,"Multi navigation source vertices exceed finite integral coordinates");if(!c&&!i)y=p.y;NAV_MULTI_REQUIRE(p.y==y,"Multi navigation source cells have different heights");if(!i)low=high=p;else{if(p.x<low.x)low.x=p.x;if(p.z<low.z)low.z=p.z;if(p.x>high.x)high.x=p.x;if(p.z>high.z)high.z=p.z;}}
  double width=double(high.x)-low.x,depth=double(high.z)-low.z;NAV_MULTI_REQUIRE(width>=64&&depth>=64&&width<=4*depth&&depth<=4*width,"Multi navigation source cell sides/aspect exceed profile");if(!c){cellWidth=width;cellDepth=depth;}NAV_MULTI_REQUIRE(width==cellWidth&&depth==cellDepth,"Multi navigation source cells are not uniform");double gx=(double(low.x)-raw.boundsMin.x)/width,gz=(double(low.z)-raw.boundsMin.z)/depth;NAV_MULTI_REQUIRE(gx>=0&&gx<=3&&gz>=0&&gz<=3&&gx==(int)gx&&gz==(int)gz,"Multi navigation source cell is off the 4x4 lattice");sourceCells[c].x=(unsigned int)(int)gx;sourceCells[c].z=(unsigned int)(int)gz;sourceCells[c].root=parent;sourceCells[c].component=0xffffffffu;unsigned int bit=1u<<(sourceCells[c].x+sourceCells[c].z*4);NAV_MULTI_REQUIRE(!(occupied&bit),"Multi navigation source cells overlap");occupied|=bit;
 }
 NAV_MULTI_REQUIRE(previousRoot==rootCount,"Multi navigation source has an empty or skipped root");
 // Parent identity may propagate across equal-mask roots during merge/split,
 // but it cannot jump between disconnected source islands in this profile.
 unsigned int* sourceRootMask=(unsigned int*)memory.Allocate(cells*4),*sourceQueue=(unsigned int*)memory.Allocate(cells*4);NAV_MULTI_REQUIRE(sourceRootMask&&sourceQueue,"Multi navigation source-component allocation failed");unsigned int sourceComponents=0;
 for(unsigned int i=0;i<cells;i++)if(sourceCells[i].component==0xffffffffu){unsigned int head=0,end=1;sourceQueue[0]=i;sourceCells[i].component=sourceComponents;while(head<end){unsigned int current=sourceQueue[head++];sourceRootMask[sourceComponents]|=1u<<(sourceCells[current].root-1);for(unsigned int j=0;j<cells;j++)if(sourceCells[j].component==0xffffffffu){int dx=(int)sourceCells[current].x-(int)sourceCells[j].x,dz=(int)sourceCells[current].z-(int)sourceCells[j].z;if((dx==0&&(dz==1||dz==-1))||(dz==0&&(dx==1||dx==-1))){NAV_MULTI_REQUIRE(end<cells,"Multi navigation source-component traversal exceeds cell count");sourceCells[j].component=sourceComponents;sourceQueue[end++]=j;}}}sourceComponents++;}
 Reader read(input.processed,input.bytes);unsigned int v;NAV_MULTI_REQUIRE(read.ReadWord(v)&&v==1&&read.ReadWord(v)&&v==6&&read.ReadWord(v)&&v==0,"Multi navigation requires processed v6 unfixed framing");
 Span edges,edgeRefs,regionRefs,normals,polys,regionPolys,regions,components,neighbors,links,boundary,nodes,names,tree,treePolys;
 NAV_MULTI_REQUIRE(read.Array(edges,72,MaximumEdges)&&read.Array(edgeRefs,4,MaximumRefs)&&read.Array(regionRefs,4,0)&&read.Array(normals,12,1)&&read.Array(polys,72,MaximumPolys)&&read.Array(regionPolys,4,0)&&read.Array(regions,28,0)&&read.Array(components,16,16)&&read.Array(neighbors,4,rootCount>1?MaximumRefs:0)&&read.Array(links,12,0)&&read.Array(boundary,12,0)&&read.Array(nodes,8,0)&&read.Array(names,1,4)&&read.Array(tree,52,MaximumTree)&&read.Array(treePolys,4,MaximumTreeRefs)&&read.End(),"Multi navigation counted streams exceed finite profile or framing");
 NAV_MULTI_REQUIRE(edges.count>=3&&polys.count&&components.count&&tree.count&&names.count==4&&normals.count==1,"Multi navigation is missing required arrays");for(unsigned int i=0;i<4;i++)NAV_MULTI_REQUIRE(!names.data[i],"Multi navigation node-name padding is not zero");WorldText::Vec3 up={0,1,0};NAV_MULTI_REQUIRE(VectorEqual(normals.data,up),"Multi navigation normal is not upward");
 Edge* edge=(Edge*)memory.Allocate(edges.count*sizeof(Edge));Poly* poly=(Poly*)memory.Allocate(polys.count*sizeof(Poly));unsigned int* border=(unsigned int*)memory.Allocate(25*4);Point* clipA=(Point*)memory.Allocate(ClipCapacity*sizeof(Point));Point* clipB=(Point*)memory.Allocate(ClipCapacity*sizeof(Point));NAV_MULTI_REQUIRE(edge&&poly&&border&&clipA&&clipB,"Multi navigation validation allocation failed");
 for(unsigned int i=0;i<edges.count;i++){
  const unsigned char* row=edges.Row(i);Edge& e=edge[i];e.a=Word(row+52);e.b=Word(row+56);NAV_MULTI_REQUIRE(Word(row)==i&&Word(row+4)<=1&&e.a<polys.count&&(e.b==0xffffffffu||e.b<polys.count)&&e.a!=e.b&&Word(row+20)<=1&&Word(row+36)<=1,"Multi navigation edge IDs/type/booleans are invalid");NAV_MULTI_REQUIRE((Word(row+4)==0)==(e.b==0xffffffffu),"Multi navigation border/shared edge type disagrees with incidence");NAV_MULTI_REQUIRE(VertexKey(row+8,source,cells*4,raw.boundsMin,cellWidth,cellDepth,e.key0)&&VertexKey(row+24,source,cells*4,raw.boundsMin,cellWidth,cellDepth,e.key1)&&e.key0!=e.key1&&FiniteVector(row+40)&&FiniteVector(row+60),"Multi navigation edge has non-source or nonfinite vertices");
  WorldText::Vec3 midpoint={F32(double(F32(double(Float(row+8))+Float(row+24)))*.5),y,F32(double(F32(double(Float(row+16))+Float(row+32)))*.5)};NAV_MULTI_REQUIRE(VectorEqual(row+40,midpoint)&&Float(row+64)==0,"Multi navigation edge midpoint or normal height is invalid");double dx=double(Float(row+24))-Float(row+8),dz=double(Float(row+32))-Float(row+16),nx=Float(row+60),nz=Float(row+68),squared=nx*nx+nz*nz;NAV_MULTI_REQUIRE(Abs(squared-1)<.00001&&Abs(nx*dx+nz*dz)<.00001*(Abs(dx)+Abs(dz)),"Multi navigation edge normal is not unit and perpendicular");
  if(e.b==0xffffffffu)border[e.key0]=border[e.key1]=1;
  for(unsigned int j=0;j<i;j++){const Edge& other=edge[j];NAV_MULTI_REQUIRE(!((e.key0==other.key0&&e.key1==other.key1)||(e.key0==other.key1&&e.key1==other.key0)),"Multi navigation has duplicate geometric edges");Point a=FromKey(e.key0),b=FromKey(e.key1),p=FromKey(other.key0),q=FromKey(other.key1);if(Cross(a,b,p)==0&&Cross(a,b,q)==0){double length=(b.x-a.x)*(b.x-a.x)+(b.z-a.z)*(b.z-a.z),p0=(p.x-a.x)*(b.x-a.x)+(p.z-a.z)*(b.z-a.z),p1=(q.x-a.x)*(b.x-a.x)+(q.z-a.z)*(b.z-a.z);NAV_MULTI_REQUIRE(!((p0>0&&p0<length)||(p1>0&&p1<length)),"Multi navigation output edges have a T junction");double otherLength=(q.x-p.x)*(q.x-p.x)+(q.z-p.z)*(q.z-p.z),a0=(a.x-p.x)*(q.x-p.x)+(a.z-p.z)*(q.z-p.z),a1=(b.x-p.x)*(q.x-p.x)+(b.z-p.z)*(q.z-p.z);NAV_MULTI_REQUIRE(!((a0>0&&a0<otherLength)||(a1>0&&a1<otherLength)),"Multi navigation output edges overlap collinearly");}}
 }
 for(unsigned int i=0;i<edges.count;i++)NAV_MULTI_REQUIRE(Word(edges.Row(i)+20)==border[edge[i].key0]&&Word(edges.Row(i)+36)==border[edge[i].key1],"Multi navigation border-vertex flags disagree with border edges");
 unsigned int nextRef=0;double totalArea=0;
 for(unsigned int i=0;i<polys.count;i++){
  const unsigned char* row=polys.Row(i);Poly& p=poly[i];p.count=Word(row+56);p.component=Word(row+8);NAV_MULTI_REQUIRE(Word(row)==i&&!Word(row+4)&&p.component<components.count&&Word(row+12)==0xffffffffu&&Word(row+16)>=1&&Word(row+16)<=rootCount&&p.count>=3&&p.count<=MaximumVertices&&Word(row+60)==nextRef&&p.count<=edgeRefs.count-nextRef&&!Word(row+64)&&!Word(row+68)&&FiniteVector(row+20)&&FiniteVector(row+32)&&FiniteVector(row+44),"Multi navigation polygon IDs/ranges/geometry are invalid");p.points=(Point*)memory.Allocate(p.count*sizeof(Point));NAV_MULTI_REQUIRE(p.points,"Multi navigation polygon allocation failed");WorldText::Vec3 center={};
  for(unsigned int j=0;j<p.count;j++){
   unsigned int current=Word(edgeRefs.Row(nextRef+j)),prior=Word(edgeRefs.Row(nextRef+(j+p.count-1)%p.count));NAV_MULTI_REQUIRE(current<edges.count&&prior<edges.count&&current!=prior,"Multi navigation polygon edge reference is invalid");Edge& e=edge[current];NAV_MULTI_REQUIRE(e.a==i||e.b==i,"Multi navigation polygon uses a nonincident edge");e.refs++;unsigned int key=0xffffffffu,matches=0;const Edge& previous=edge[prior];if(e.key0==previous.key0||e.key0==previous.key1){key=e.key0;matches++;}if(e.key1==previous.key0||e.key1==previous.key1){key=e.key1;matches++;}NAV_MULTI_REQUIRE(matches==1,"Multi navigation edge references do not form a simple cycle");p.points[j]=FromKey(key);for(unsigned int k=0;k<j;k++)NAV_MULTI_REQUIRE(!Same(p.points[j],p.points[k]),"Multi navigation polygon repeats a vertex");WorldText::Vec3 point={F32(double(raw.boundsMin.x)+p.points[j].x*cellWidth),y,F32(double(raw.boundsMin.z)+p.points[j].z*cellDepth)};if(!j)p.low=p.high=point;else{if(point.x<p.low.x)p.low.x=point.x;if(point.z<p.low.z)p.low.z=point.z;if(point.x>p.high.x)p.high.x=point.x;if(point.z>p.high.z)p.high.z=point.z;}center.x=F32(double(center.x)+point.x);center.y=F32(double(center.y)+point.y);center.z=F32(double(center.z)+point.z);
  }
  nextRef+=p.count;p.area=-Area(p.points,p.count);NAV_MULTI_REQUIRE(p.area>0,"Multi navigation polygon winding/area is invalid");for(unsigned int j=0;j<p.count;j++)for(unsigned int k=0;k<p.count;k++)NAV_MULTI_REQUIRE(Cross(p.points[j],p.points[(j+1)%p.count],p.points[k])<=0,"Multi navigation polygon is not convex");center.x=F32(double(center.x)/p.count);center.y=F32(double(center.y)/p.count);center.z=F32(double(center.z)/p.count);NAV_MULTI_REQUIRE(VectorEqual(row+20,center)&&VectorEqual(row+32,p.low)&&VectorEqual(row+44,p.high),"Multi navigation polygon center/bounds differ from edge cycle");
  double covered=0;unsigned int sourceComponent=0xffffffffu;for(unsigned int c=0;c<cells;c++){double x=sourceCells[c].x,z=sourceCells[c].z;Point rectangle[4]={{x,z},{x,z+1},{x+1,z+1},{x+1,z}};double area;NAV_MULTI_REQUIRE(Intersection(p.points,p.count,rectangle,4,clipA,clipB,area),"Multi navigation source-cell clipping failed");covered+=area;if(area>1e-10){if(sourceComponent==0xffffffffu)sourceComponent=sourceCells[c].component;NAV_MULTI_REQUIRE(sourceComponent==sourceCells[c].component,"Multi navigation polygon bridges disconnected source islands");}}NAV_MULTI_REQUIRE(Abs(covered-p.area)<1e-10,"Multi navigation polygon lies outside the source union");NAV_MULTI_REQUIRE(sourceComponent<sourceComponents&&(sourceRootMask[sourceComponent]&(1u<<(Word(row+16)-1))),"Multi navigation polygon parent belongs to a different source island");
  for(unsigned int j=0;j<i;j++){double overlap;NAV_MULTI_REQUIRE(Intersection(p.points,p.count,poly[j].points,poly[j].count,clipA,clipB,overlap)&&overlap<1e-10,"Multi navigation polygons overlap in their interiors");}totalArea+=p.area;
 }
 NAV_MULTI_REQUIRE(nextRef==edgeRefs.count&&Abs(totalArea-cells)<1e-10,"Multi navigation output leaves uncovered source area or unused edge references");
 for(unsigned int i=0;i<edges.count;i++){
  const unsigned char* row=edges.Row(i);NAV_MULTI_REQUIRE(edge[i].refs==(edge[i].b==0xffffffffu?1u:2u),"Multi navigation edge reference multiplicity is invalid");double towardX=double(Float(polys.Row(edge[i].a)+20))-Float(row+40),towardZ=double(Float(polys.Row(edge[i].a)+28))-Float(row+48);NAV_MULTI_REQUIRE(towardX*Float(row+60)+towardZ*Float(row+68)>0,"Multi navigation edge normal points away from polygon A");
 }
 unsigned int* visited=(unsigned int*)memory.Allocate(polys.count*4),*usedComponents=(unsigned int*)memory.Allocate(components.count*4),*queue=(unsigned int*)memory.Allocate((MaximumTree>polys.count?MaximumTree:polys.count)*4);NAV_MULTI_REQUIRE(visited&&usedComponents&&queue,"Multi navigation component validation allocation failed");unsigned int graphComponents=0;
 // Movement components can stop at a restriction boundary even when there is
 // a shared geometric edge. Require each individual component to be connected.
 for(unsigned int i=0;i<polys.count;i++)if(!visited[i]){unsigned int component=poly[i].component;NAV_MULTI_REQUIRE(!usedComponents[component],"Multi navigation component ID groups disconnected polygons");usedComponents[component]=1;graphComponents++;unsigned int head=0,end=1;queue[0]=i;visited[i]=1;while(head<end){unsigned int current=queue[head++];for(unsigned int j=0;j<edges.count;j++)if(edge[j].b!=0xffffffffu&&(edge[j].a==current||edge[j].b==current)){unsigned int neighbor=edge[j].a==current?edge[j].b:edge[j].a;if(poly[neighbor].component==component&&!visited[neighbor]){NAV_MULTI_REQUIRE(end<polys.count,"Multi navigation component traversal exceeds polygon count");visited[neighbor]=1;queue[end++]=neighbor;}}}}
 NAV_MULTI_REQUIRE(graphComponents==components.count,"Multi navigation has unused component rows");
 unsigned int* expectedNeighbors=(unsigned int*)memory.Allocate(components.count*components.count*4),*actualNeighbors=(unsigned int*)memory.Allocate(components.count*components.count*4),*sensorySeen=(unsigned int*)memory.Allocate(components.count*4);NAV_MULTI_REQUIRE(expectedNeighbors&&actualNeighbors&&sensorySeen,"Multi navigation component-neighbor allocation failed");
 unsigned int nextNeighbor=0;
 for(unsigned int i=0;i<components.count;i++){
  const unsigned char* row=components.Row(i);unsigned int count=Word(row+8);NAV_MULTI_REQUIRE(Word(row)==i&&Word(row+4)<components.count&&Word(row+12)==nextNeighbor&&count<=neighbors.count-nextNeighbor,"Multi navigation component ID/sensory/neighbor range is invalid");
  for(unsigned int j=0;j<count;j++){unsigned int target=Word(neighbors.Row(nextNeighbor+j));NAV_MULTI_REQUIRE(target<components.count&&target!=i,"Multi navigation component neighbor is invalid or self-referential");actualNeighbors[i*components.count+target]++;}nextNeighbor+=count;
  // SDK stores each neighboring POLYGON once, then exports its component ID.
  // Several different neighboring polygons may therefore repeat that ID.
  for(unsigned int p=0;p<polys.count;p++)if(poly[p].component!=i){bool adjacent=false;for(unsigned int j=0;j<edges.count;j++)if(edge[j].b!=0xffffffffu&&((edge[j].a==p&&poly[edge[j].b].component==i)||(edge[j].b==p&&poly[edge[j].a].component==i))){adjacent=true;break;}if(adjacent)expectedNeighbors[i*components.count+poly[p].component]++;}
 }
 NAV_MULTI_REQUIRE(nextNeighbor==neighbors.count,"Multi navigation has unused component-neighbor references");for(unsigned int i=0;i<components.count*components.count;i++)NAV_MULTI_REQUIRE(actualNeighbors[i]==expectedNeighbors[i],"Multi navigation component-neighbor multiplicity differs from adjacent polygons");
 unsigned int sensoryComponents=0;
 for(unsigned int i=0;i<components.count;i++)if(!sensorySeen[i]){unsigned int head=0,end=1;queue[0]=i;sensorySeen[i]=1;while(head<end){unsigned int current=queue[head++];NAV_MULTI_REQUIRE(Word(components.Row(current)+4)==sensoryComponents,"Multi navigation sensory ID disagrees with component connectivity");for(unsigned int j=0;j<components.count;j++)if(expectedNeighbors[current*components.count+j]&&!sensorySeen[j]){NAV_MULTI_REQUIRE(end<components.count,"Multi navigation sensory traversal exceeds component count");sensorySeen[j]=1;queue[end++]=j;}}sensoryComponents++;}
 NAV_MULTI_REQUIRE(sourceComponents==sensoryComponents,"Multi navigation sensory topology disagrees with source-cell connectivity");
 unsigned int* parents=(unsigned int*)memory.Allocate(tree.count*4),*levels=(unsigned int*)memory.Allocate(tree.count*4);NAV_MULTI_REQUIRE(parents&&levels,"Multi navigation quadtree validation allocation failed");unsigned int nextPoly=0;
 for(unsigned int i=0;i<tree.count;i++){
  const unsigned char* row=tree.Row(i);unsigned int count=Word(row+28);NAV_MULTI_REQUIRE(Word(row)==i&&FiniteVector(row+4)&&FiniteVector(row+16)&&Float(row+4)<=Float(row+16)&&Float(row+8)<=Float(row+20)&&Float(row+12)<=Float(row+24)&&Word(row+32)==nextPoly&&count<=treePolys.count-nextPoly,"Multi navigation quadtree row/list is invalid");nextPoly+=count;unsigned int children=0;for(unsigned int j=0;j<4;j++){unsigned int child=Word(row+36+j*4);if(child==0xffffffffu)continue;NAV_MULTI_REQUIRE(child<tree.count&&child!=i&&++parents[child]==1,"Multi navigation quadtree has invalid/shared children");children++;}NAV_MULTI_REQUIRE(children==0||children==4,"Multi navigation quadtree requires four children per internal row");
 }NAV_MULTI_REQUIRE(!parents[0]&&nextPoly==treePolys.count&&VectorEqual(tree.data+4,raw.boundsMin)&&VectorEqual(tree.data+16,raw.boundsMax),"Multi navigation quadtree root differs from source bounds");for(unsigned int i=1;i<tree.count;i++)NAV_MULTI_REQUIRE(parents[i]==1,"Multi navigation quadtree has an orphan row");
 unsigned int head=0,end=1;queue[0]=0;while(head<end){unsigned int id=queue[head++],level=levels[id];const unsigned char* row=tree.Row(id);NAV_MULTI_REQUIRE(level<=4,"Multi navigation quadtree exceeds SDK depth");bool leaf=Word(row+36)==0xffffffffu;if(leaf){unsigned int count=Word(row+28),start=Word(row+32),expected=0;for(unsigned int p=0;p<polys.count;p++){bool intersects=poly[p].low.x<=Float(row+16)&&poly[p].high.x>=Float(row+4)&&poly[p].low.z<=Float(row+24)&&poly[p].high.z>=Float(row+12);unsigned int occurrences=0;for(unsigned int j=0;j<count;j++){unsigned int ref=Word(treePolys.Row(start+j));NAV_MULTI_REQUIRE(ref<polys.count,"Multi navigation quadtree polygon reference is out of range");if(ref==p)occurrences++;}NAV_MULTI_REQUIRE(occurrences==(intersects&&level==4?1u:0u),"Multi navigation quadtree leaf misses/duplicates a polygon AABB reference");if(intersects)expected++;}NAV_MULTI_REQUIRE(level==4||!expected,"Multi navigation nonempty quadtree leaf stops before SDK depth");}else{
   NAV_MULTI_REQUIRE(level<4&&!Word(row+28),"Multi navigation internal quadtree row has leaf data");WorldText::Vec3 low={Float(row+4),Float(row+8),Float(row+12)},high={Float(row+16),Float(row+20),Float(row+24)};float midX=F32(double(F32(double(low.x)+high.x))*.5),midZ=F32(double(F32(double(low.z)+high.z))*.5);for(unsigned int j=0;j<4;j++){unsigned int child=Word(row+36+j*4);NAV_MULTI_REQUIRE(child<tree.count&&end<tree.count,"Multi navigation quadtree traversal is cyclic/incomplete");WorldText::Vec3 lo=low,hi=high;if(j<2)lo.z=midZ;else hi.z=midZ;if(j&1)lo.x=midX;else hi.x=midX;NAV_MULTI_REQUIRE(VectorEqual(tree.Row(child)+4,lo)&&VectorEqual(tree.Row(child)+16,hi),"Multi navigation quadtree subdivision bounds are invalid");levels[child]=level+1;queue[end++]=child;}
  }}NAV_MULTI_REQUIRE(end==tree.count,"Multi navigation quadtree has disconnected rows");
#undef NAV_MULTI_REQUIRE
 return true;
}
inline bool Validate(const NativeRawNavMesh::Builder& raw,const NativeProcessedNavMesh::Input& input,const char*& error){return ValidateRootSet(raw,input,error,1);}
}
