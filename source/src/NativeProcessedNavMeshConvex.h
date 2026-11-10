#pragma once
#include "NativeProcessedNavMesh.h"
// Independent source-bound acceptance for one retained strictly convex horizontal
// polygon or the finite rational-plane slope profile. No SDK call, original
// packer, or retained byte table is used here.
namespace NativeProcessedNavMeshConvex {
namespace detail {
using NativeProcessedNavMesh::detail::Word;
using NativeProcessedNavMesh::detail::Float;
using NativeProcessedNavMesh::detail::F32;
using NativeProcessedNavMesh::detail::VectorEqual;
using NativeProcessedNavMesh::detail::FiniteVector;
using NativeProcessedNavMesh::detail::Span;
using NativeProcessedNavMesh::detail::Reader;
inline double Abs(double value){return value<0?-value:value;}
inline double Inner(const WorldText::Vec3& a,const WorldText::Vec3& b,const WorldText::Vec3& p){return (double(b.z)-a.z)*(double(p.x)-a.x)-(double(b.x)-a.x)*(double(p.z)-a.z);}
inline bool SourceGate(const WorldText::Vec3* points,unsigned int count,WorldText::Vec3& low,WorldText::Vec3& high){
 if(count<3||count>8)return false;low=high=points[0];double area2=0;
 for(unsigned int i=0;i<count;i++){
  const WorldText::Vec3 &a=points[i],&b=points[(i+1)%count],&c=points[(i+2)%count];
  if(!WorldText::FiniteVector(a)||a.x<-1048576.f||a.x>1048576.f||a.y<-1048576.f||a.y>1048576.f||a.z<-1048576.f||a.z>1048576.f||a.x!=(float)(int)a.x||a.y!=(float)(int)a.y||a.z!=(float)(int)a.z||a.y!=points[0].y)return false;
  if(a.x<low.x)low.x=a.x;if(a.x>high.x)high.x=a.x;if(a.z<low.z)low.z=a.z;if(a.z>high.z)high.z=a.z;
  double ax=double(b.x)-a.x,az=double(b.z)-a.z,bx=double(c.x)-b.x,bz=double(c.z)-b.z,aa=ax*ax+az*az,bb=bx*bx+bz*bz,turn=az*bx-ax*bz;
  if(aa<4096||bb<4096||turn<=0||turn*turn<.0025*aa*bb)return false;
  // SDK simplification compares prev->cur with prev->next, rather than the
  // two adjacent edge directions. Keep twice its 0.01 direction-difference
  // margin. This equivalent double predicate needs no CRT sqrt function.
  double cx=double(c.x)-a.x,cz=double(c.z)-a.z,cc=cx*cx+cz*cz,dot=ax*cx+az*cz;
  if(cc<4096||(dot>0&&dot*dot>.99960004*aa*cc))return false;
  for(unsigned int j=0;j<count;j++){
   if(j!=i){double dx=double(points[j].x)-a.x,dz=double(points[j].z)-a.z;if(dx*dx+dz*dz<4096)return false;}
   if(j!=i&&j!=(i+1)%count&&Inner(a,b,points[j])<=0)return false;
  }
  area2+=double(a.z)*b.x-double(a.x)*b.z;
 }
 double width=double(high.x)-low.x,depth=double(high.z)-low.z;
 return width>=64&&depth>=64&&width<=4*depth&&depth<=4*width&&area2>=8192;
}
}
inline bool Validate(const NativeRawNavMesh::Builder& raw,const NativeProcessedNavMesh::Input& input,const char*& error){
 using namespace detail;error=0;
#define NAV_CONVEX_REQUIRE(condition,message) do{if(!(condition)){error=message;return false;}}while(0)
 NAV_CONVEX_REQUIRE(raw.data&&raw.size>=48&&raw.size<=16u*1024u*1024u&&input.sourceRaw&&input.sourceRawBytes==raw.size&&input.processed&&input.bytes>=76&&input.bytes<=16u*1024u*1024u,"Convex navigation requires bounded source witness and processed bytes");
 for(unsigned int i=0;i<raw.size;i++)NAV_CONVEX_REQUIRE(((const unsigned char*)input.sourceRaw)[i]==raw.data[i],"Convex navigation raw witness differs from the current source scene");
 unsigned int count=Word(raw.data+44);NAV_CONVEX_REQUIRE(Word(raw.data)==6&&Word(raw.data+28)==1&&Word(raw.data+32)==1&&Word(raw.data+36)==1&&Word(raw.data+40)==0xffffffffu&&count>=3&&count<=8,"Convex navigation requires one unlinked source root/polygon with three to eight corners");
 unsigned int tail=48+12*count;NAV_CONVEX_REQUIRE(raw.size>=tail+35,"Convex navigation source restriction framing is truncated");
 for(unsigned int i=0;i<4;i++)NAV_CONVEX_REQUIRE(!Word(raw.data+tail+i*4),"Convex navigation auxiliary source families remain unsupported");
 NAV_CONVEX_REQUIRE(Word(raw.data+tail+16)==1&&Word(raw.data+tail+20)==1,"Convex navigation restriction count or identity is invalid");
 unsigned int nameBytes=(unsigned int)raw.data[tail+24]|((unsigned int)raw.data[tail+25]<<8);
 NAV_CONVEX_REQUIRE(nameBytes>=1&&nameBytes<=63&&raw.size==tail+34+nameBytes,"Convex navigation source type string or final cursor is invalid");
 for(unsigned int i=0;i<nameBytes;i++)NAV_CONVEX_REQUIRE(raw.data[tail+26+i]>=32&&raw.data[tail+26+i]<=126,"Convex navigation source type string is not printable ASCII");
 NAV_CONVEX_REQUIRE(Word(raw.data+tail+26+nameBytes)<=1&&!Word(raw.data+tail+30+nameBytes),"Convex navigation requires canonical mask flags and empty character restrictions");
 WorldText::Vec3 points[8],low,high;for(unsigned int i=0;i<count;i++)memcpy(points+i,raw.data+48+i*12,12);
 NativeNavSlopeGeometry::Plane plane={};const bool slope=raw.slopePolygonProfile;
 NAV_CONVEX_REQUIRE(slope?NativeNavSlopeGeometry::Check(points,count,plane,low,high):SourceGate(points,count,low,high),"Convex navigation source geometry exceeds its strict finite polygon profile");
 NAV_CONVEX_REQUIRE(Word(raw.data+8)==0xff7fffffu&&Word(raw.data+20)==0x7f7fffffu&&Float(raw.data+4)==low.x&&Float(raw.data+12)==low.z&&Float(raw.data+16)==high.x&&Float(raw.data+24)==high.z&&VectorEqual(raw.data+4,raw.boundsMin)&&VectorEqual(raw.data+16,raw.boundsMax),"Convex navigation source bounds differ from the source polygon");
 Reader read(input.processed,input.bytes);unsigned int word;NAV_CONVEX_REQUIRE(read.ReadWord(word)&&word==1&&read.ReadWord(word)&&word==6&&read.ReadWord(word)&&word==0,"Convex navigation requires processed v6 unfixed framing");
 Span edges,edgeRefs,regionRefs,normals,polys,regionPolys,regions,components,neighbors,links,boundary,nodes,names,tree,treePolys;
 NAV_CONVEX_REQUIRE(read.Array(edges,72,count)&&read.Array(edgeRefs,4,count)&&read.Array(regionRefs,4,0)&&read.Array(normals,12,1)&&read.Array(polys,72,1)&&read.Array(regionPolys,4,0)&&read.Array(regions,28,0)&&read.Array(components,16,1)&&read.Array(neighbors,4,0)&&read.Array(links,12,0)&&read.Array(boundary,12,0)&&read.Array(nodes,8,0)&&read.Array(names,1,4)&&read.Array(tree,52,341)&&read.Array(treePolys,4,256)&&read.End(),"Convex navigation counted streams differ from the complete one-polygon profile");
 for(unsigned int i=0;i<4;i++)NAV_CONVEX_REQUIRE(!names.data[i],"Convex navigation empty node-name padding is not zero");
 const unsigned char *poly=polys.data,*component=components.data;
 NAV_CONVEX_REQUIRE(!Word(poly)&&!Word(poly+4)&&!Word(poly+8)&&Word(poly+12)==0xffffffffu&&Word(poly+16)==1&&Word(poly+56)==count&&!Word(poly+60)&&!Word(poly+64)&&!Word(poly+68)&&!Word(component)&&!Word(component+4)&&!Word(component+8)&&!Word(component+12),"Convex navigation polygon/component identities or ranges are invalid");
 WorldText::Vec3 center={};for(unsigned int i=0;i<count;i++){center.x=F32(double(center.x)+points[i].x);center.y=F32(double(center.y)+points[i].y);center.z=F32(double(center.z)+points[i].z);}center.x=F32(double(center.x)/count);center.y=F32(double(center.y)/count);center.z=F32(double(center.z)/count);
 NAV_CONVEX_REQUIRE(VectorEqual(poly+20,center)&&VectorEqual(poly+32,low)&&VectorEqual(poly+44,high),"Convex navigation polygon center/bounds differ from the source cycle");
 NAV_CONVEX_REQUIRE(FiniteVector(normals.data)&&(slope?NativeNavSlopeGeometry::Direction(Float(normals.data),Float(normals.data+4),Float(normals.data+8),-double(plane.p),double(plane.q),-double(plane.r)):(Float(normals.data)==0&&Abs(double(Float(normals.data+4))-1)<.00001&&Float(normals.data+8)==0)),"Convex navigation polygon normal differs from the source plane");
 unsigned int segments=0,edgeIds[8]={};
 for(unsigned int i=0;i<count;i++){
  const unsigned char* row=edges.Row(i);NAV_CONVEX_REQUIRE(Word(row)==i&&!Word(row+4)&&Word(row+20)==1&&Word(row+36)==1&&!Word(row+52)&&Word(row+56)==0xffffffffu&&FiniteVector(row+8)&&FiniteVector(row+24)&&FiniteVector(row+40)&&FiniteVector(row+60),"Convex navigation border edge identity/incidence/geometry is invalid");
  unsigned int segment=0xffffffffu;for(unsigned int j=0;j<count;j++){const WorldText::Vec3 &a=points[j],&b=points[(j+1)%count];if((VectorEqual(row+8,a)&&VectorEqual(row+24,b))||(VectorEqual(row+8,b)&&VectorEqual(row+24,a)))segment=j;}
  NAV_CONVEX_REQUIRE(segment<count&&!(segments&(1u<<segment)),"Convex navigation output has missing, duplicate or non-source edges");segments|=1u<<segment;edgeIds[segment]=i;
  const WorldText::Vec3 &a=points[segment],&b=points[(segment+1)%count];WorldText::Vec3 midpoint={F32(double(F32(double(a.x)+b.x))*.5),F32(double(F32(double(a.y)+b.y))*.5),F32(double(F32(double(a.z)+b.z))*.5)};
  double dx=double(b.x)-a.x,dz=double(b.z)-a.z,nx=Float(row+60),nz=Float(row+68),length2=dx*dx+dz*dz,dot=nx*dx+nz*dz;
  double dy=double(b.y)-a.y,hx=double(plane.q)*dz+double(plane.r)*dy,hz=-double(plane.p)*dy-double(plane.q)*dx;
  NAV_CONVEX_REQUIRE(VectorEqual(row+40,midpoint)&&Float(row+64)==0&&(slope?NativeNavSlopeGeometry::Direction(nx,0,nz,hx,0,hz):(Abs(nx*nx+nz*nz-1)<.00001&&dot*dot<.0000000001*length2&&nx*(double(center.x)-midpoint.x)+nz*(double(center.z)-midpoint.z)>0)),"Convex navigation edge midpoint/normal differs from the 3D source-edge construction");
 }
 unsigned int start=count;for(unsigned int i=0;i<count;i++)if(Word(edgeRefs.data)==edgeIds[i])start=i;NAV_CONVEX_REQUIRE(start<count,"Convex navigation edge cycle starts outside source edges");for(unsigned int i=0;i<count;i++)NAV_CONVEX_REQUIRE(Word(edgeRefs.Row(i))==edgeIds[(start+i)%count],"Convex navigation edge references do not preserve the source cycle");
 PrefabScene::Arena scratch;unsigned int* parents=(unsigned int*)scratch.Allocate(341*4),*queue=(unsigned int*)scratch.Allocate(341*4),*levels=(unsigned int*)scratch.Allocate(341*4);NAV_CONVEX_REQUIRE(parents&&queue&&levels,"Convex navigation quadtree validator allocation failed");unsigned int nextPoly=0;
 for(unsigned int i=0;i<341;i++){
  const unsigned char* row=tree.Row(i);unsigned int refs=Word(row+28);NAV_CONVEX_REQUIRE(Word(row)==i&&FiniteVector(row+4)&&FiniteVector(row+16)&&Float(row+4)<=Float(row+16)&&Float(row+8)<=Float(row+20)&&Float(row+12)<=Float(row+24)&&Word(row+32)==nextPoly&&refs<=256-nextPoly,"Convex navigation quadtree row/bounds/list is invalid");nextPoly+=refs;
  unsigned int children=0;for(unsigned int j=0;j<4;j++){unsigned int child=Word(row+36+j*4);if(child==0xffffffffu)continue;NAV_CONVEX_REQUIRE(child<341&&child!=i&&++parents[child]==1,"Convex navigation quadtree has an invalid/shared child");children++;}NAV_CONVEX_REQUIRE(children==0||children==4,"Convex navigation quadtree internal row requires four children");
 }
 NAV_CONVEX_REQUIRE(!parents[0]&&nextPoly==256&&VectorEqual(tree.data+4,raw.boundsMin)&&VectorEqual(tree.data+16,raw.boundsMax),"Convex navigation quadtree root/list differs from source bounds");for(unsigned int i=1;i<341;i++)NAV_CONVEX_REQUIRE(parents[i]==1,"Convex navigation quadtree has an orphan row");
 unsigned int head=0,end=1;while(head<end){unsigned int id=queue[head++],level=levels[id];const unsigned char* row=tree.Row(id);NAV_CONVEX_REQUIRE(level<=4,"Convex navigation quadtree exceeds SDK depth");if(level==4){NAV_CONVEX_REQUIRE(Word(row+28)==1,"Convex navigation quadtree leaf must reference the single polygon");for(unsigned int j=0;j<4;j++)NAV_CONVEX_REQUIRE(Word(row+36+j*4)==0xffffffffu,"Convex navigation quadtree leaf has children");}else{
   NAV_CONVEX_REQUIRE(!Word(row+28),"Convex navigation quadtree internal row has polygon references");WorldText::Vec3 lo={Float(row+4),Float(row+8),Float(row+12)},hi={Float(row+16),Float(row+20),Float(row+24)};float midX=F32(double(F32(double(lo.x)+hi.x))*.5),midZ=F32(double(F32(double(lo.z)+hi.z))*.5);
   for(unsigned int j=0;j<4;j++){unsigned int child=Word(row+36+j*4);NAV_CONVEX_REQUIRE(child<341&&end<341,"Convex navigation quadtree is cyclic or incomplete");WorldText::Vec3 a=lo,b=hi;if(j<2)a.z=midZ;else b.z=midZ;if(j&1)a.x=midX;else b.x=midX;NAV_CONVEX_REQUIRE(VectorEqual(tree.Row(child)+4,a)&&VectorEqual(tree.Row(child)+16,b),"Convex navigation quadtree subdivision bounds are invalid");levels[child]=level+1;queue[end++]=child;}
  }}
 NAV_CONVEX_REQUIRE(end==341,"Convex navigation quadtree has disconnected rows");for(unsigned int i=0;i<256;i++)NAV_CONVEX_REQUIRE(!Word(treePolys.Row(i)),"Convex navigation quadtree references a non-source polygon");
#undef NAV_CONVEX_REQUIRE
 return true;
}
}
