#pragma once
namespace BspBlock {
struct SurfaceMetadata {unsigned char occlusion;unsigned short material;};
struct Header {unsigned int flags,points,polygons,references,nodes;WorldText::Vec3 half,center;int root;};
static_assert(sizeof(Header)==48,"Physical BSP header size");
class Block {
 unsigned int cursor;
 bool Put(const void* source,unsigned int bytes){if(bytes>size-cursor)return false;const unsigned char* p=(const unsigned char*)source;for(unsigned int i=0;i<bytes;i++)data[cursor++]=p[i];return true;}
 bool Fail(const char* message){error=message;data=0;size=cursor=0;return false;}
public:
 PrefabScene::Arena memory;unsigned char* data;unsigned int size;const char* error;
 Block():cursor(0),data(0),size(0),error(0){}
 bool Build(const PrefabScene::Node& brush,const BspTree::Tree& tree,const WorldText::Vec3* normals,unsigned int normalCount,const SurfaceMetadata* metadata,unsigned int metadataCount,unsigned int worldFlags,int positiveLeaf,int negativeLeaf){
  memory.Clear();data=0;size=cursor=0;error=0;const WorldText::Geometry& g=brush.geometry;
  if(brush.kind!=1||!g.points||!g.polygons||g.points>1000000||g.polygons>100000||metadataCount!=g.polygons||!metadata||!normals||!normalCount)return Fail("Unsupported BSP block input");
  if(tree.splits||tree.inputPolygons!=g.polygons||tree.branches!=g.polygons)return Fail("BSP block requires an unsplit single-brush chain");
  BspNodeTable::Record* nodes=(BspNodeTable::Record*)memory.Allocate(g.polygons*sizeof(BspNodeTable::Record));unsigned int nodeCount=0;
  if(!nodes||!BspNodeTable::ConvexChain(tree.root,nodes,g.polygons,positiveLeaf,negativeLeaf,nodeCount)||nodeCount!=g.polygons)return Fail("Unsupported BSP node chain");
  for(const BspTree::Branch* b=tree.root;b;b=b->back)if(b->coplanar->fragment.ownerIndex!=brush.index)return Fail("BSP owner mismatch");
  unsigned int refs=0;for(unsigned int i=0;i<g.polygons;i++){
   if(g.faces[i].count<3||g.faces[i].count>255||PolygonCheck::Check(g,g.faces[i]).issues)return Fail("Invalid saved contour");refs+=g.faces[i].count;
   if(nodes[i].polygon!=(int)i)return Fail("Control block requires source-order polygon IDs");
  }
  size=48+g.polygons+12*g.polygons+4*refs+12*nodeCount+12*g.points;
  data=(unsigned char*)memory.Allocate(size);if(!data)return Fail("Could not allocate BSP block");
  WorldText::Vec3 low=g.vertices[0],high=low;
  for(unsigned int i=0;i<g.points;i++){const WorldText::Vec3& v=g.vertices[i];if(!WorldText::FiniteVector(v))return Fail("Non-finite BSP point");
   if(v.x<low.x)low.x=v.x;if(v.y<low.y)low.y=v.y;if(v.z<low.z)low.z=v.z;
   if(v.x>high.x)high.x=v.x;if(v.y>high.y)high.y=v.y;if(v.z>high.z)high.z=v.z;
  }
  Header h={};h.flags=worldFlags;h.points=g.points;h.polygons=g.polygons;h.references=refs;h.nodes=nodeCount;h.root=0;
  h.half.x=(high.x-low.x)*.5f;h.half.y=(high.y-low.y)*.5f;h.half.z=(high.z-low.z)*.5f;
  h.center.x=(high.x+low.x)*.5f;h.center.y=(high.y+low.y)*.5f;h.center.z=(high.z+low.z)*.5f;
  if(!WorldText::FiniteVector(h.half)||!WorldText::FiniteVector(h.center))return Fail("Non-finite BSP bounds");
  if(!Put(&h,sizeof(h)))return Fail("BSP buffer overflow");
  for(unsigned int i=0;i<g.polygons;i++){unsigned char count=(unsigned char)g.faces[i].count;if(!Put(&count,1))return Fail("BSP buffer overflow");}
  for(unsigned int i=0;i<g.polygons;i++){
   const WorldText::Polygon& p=g.faces[i];unsigned int plane=0;while(plane<normalCount&&!BspSplit::Same(normals[plane],p.normal))plane++;
   if(plane==normalCount)return Fail("Normal missing from caller plane table");
   unsigned char flags=(unsigned char)(brush.surfaceFlags&7);
   if(!Put(&flags,1)||!Put(&metadata[i].occlusion,1)||!Put(&metadata[i].material,2)||!Put(&plane,4)||!Put(&p.distance,4)||!Put(p.indices,p.count*4))return Fail("BSP buffer overflow");
  }
  if(!Put(nodes,nodeCount*12)||!Put(g.vertices,g.points*12)||cursor!=size)return Fail("BSP buffer overflow");return true;
 }
};
// Restricted diagnostic block writer. Caller supplies global plane-table order,
// resolved surface metadata, world flags and leaf labels. No resource resolver.
}
