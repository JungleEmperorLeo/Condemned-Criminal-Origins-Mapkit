#pragma once
namespace PhysicsMeshPrefix {
// The original fans every source contour. Zero-area faces (up-vector plane) and
// contours with points on their own sides are kept; MOPP skips degenerate triangles.
inline unsigned int FanIssues(bool production){return production?(unsigned int)(PolygonCheck::NonPlanar|PolygonCheck::Concave|PolygonCheck::Degenerate|PolygonCheck::SelfIntersection|PolygonCheck::RepeatedPoint):(unsigned int)PolygonCheck::Concave;}
class Block {
 unsigned int cursor;
 bool Put(const void* value,unsigned int bytes){if(bytes>size-cursor)return false;const unsigned char* p=(const unsigned char*)value;for(unsigned int i=0;i<bytes;i++)data[cursor++]=p[i];return true;}
public:
 PrefabScene::Arena memory;unsigned char* data;unsigned int size;
 Block():cursor(0),data(0),size(0){}
 bool BuildStatic(const WorldText::Geometry& g,float scale,float mass,float density,bool allowNonPlanarFan=false){
  memory.Clear();data=0;size=cursor=0;
  if(!g.points||g.points>1000000||!g.polygons||g.polygons>100000||!WorldText::Finite(scale)||scale<=0||!WorldText::Finite(mass)||!WorldText::Finite(density))return false;
  unsigned int indices=0;for(unsigned int i=0;i<g.polygons;i++){if((PolygonCheck::Check(g,g.faces[i]).issues&~FanIssues(allowNonPlanarFan))||g.faces[i].count>4096)return false;indices+=(g.faces[i].count-2)*3;}
  const bool wide=g.points>65535; /* CONDEMNED FORK: 32-bit triangle indices above 65535 vertices */ size=4+32+72+g.points*12+indices*(wide?4:2);data=(unsigned char*)memory.Allocate(size);if(!data){size=0;return false;}
  unsigned int count=1,type=1;float transform[7]={0,0,0,0,0,0,1};float values[16]={};values[0]=mass;values[1]=density;values[15]=1;
  if(!Put(&count,4)||!Put(transform,28)||!Put(&type,4)||!Put(values,64)||!Put(&g.points,4)||!Put(&indices,4))return false;
  for(unsigned int i=0;i<g.points;i++){WorldText::Vec3 v={g.vertices[i].x*scale,g.vertices[i].y*scale,g.vertices[i].z*scale};if(!WorldText::FiniteVector(v)){data=0;size=0;return false;}if(!Put(&v,12))return false;}
  for(unsigned int i=0;i<g.polygons;i++){const WorldText::Polygon& p=g.faces[i];for(unsigned int k=1;k+1<p.count;k++){if(wide){unsigned int triangle[3]={p.indices[0],p.indices[k],p.indices[k+1]};if(!Put(triangle,12))return false;}else{unsigned short triangle[3]={(unsigned short)p.indices[0],(unsigned short)p.indices[k],(unsigned short)p.indices[k+1]};if(!Put(triangle,6))return false;}}}
  return cursor==size;
 }
};
// Static single mesh, zero mass properties and identity transform, fan triangulation.
// Stops BEFORE required MOPP CodeInfo/bytecode. Not a complete physics shape.
}
