#pragma once
namespace InaccurateGeometry {
// Original SnapFloat RVA 0x58E40, called by region cleanup with 0.01f.
inline float Snap(float value,float threshold=0.01f){
 if(!WorldText::Finite(value)||value<=-2147483520.0f||value>=2147483520.0f)return value;
 int candidate=(int)(double(value)+(value<0?-double(threshold):double(threshold)));
 float integer=(float)candidate;
 float residual=(float)(double(value)-integer);if(residual<0)residual=-residual;
 return residual<threshold?integer:value;
}
// The region cleanup caller bypasses SnapFloat for numerically exact integers
// (VA100591D1..100591F1,10059249..10059269). Preserve their original bits,
// including authored -0. Noninteger negative values still use SnapFloat and
// may correctly become +0; this is not a sign-copy operation.
inline float RegionCoordinate(float value){
 if(WorldText::Finite(value)&&value>-2147483520.0f&&value<2147483520.0f&&value==(float)(int)value)return value;
 return Snap(value);
}
inline bool WeldClose(const WorldText::Vec3& a,const WorldText::Vec3& b){
 // Original VA10056D54..10056D98: three binary32 differences, then
 // (dx*dx+dy*dy)+dz*dz at 53 bits, <= the stored square of float0.1.
 // Volatile stores preserve these boundaries across source-worker builds.
 if(!WorldText::FiniteVector(a)||!WorldText::FiniteVector(b))return false;
 volatile float dx=(float)(double(b.x)-double(a.x));
 volatile float dy=(float)(double(b.y)-double(a.y));
 volatile float dz=(float)(double(b.z)-double(a.z));
 volatile double xy=double(dx)*double(dx)+double(dy)*double(dy);
 volatile double squared=double(xy)+double(dz)*double(dz);
 union {unsigned int bits;float value;} threshold;threshold.bits=0x3c23d70bu;
 return squared<=double(threshold.value);
}
inline void Weld(WorldText::Geometry& geometry){
 // Ascending complete point pool, retaining the earlier point and its color.
 // Recheck a removed slot. This intentionally does not compute transitive
 // clusters and does not weld separate brushes or transformed instances.
 for(unsigned int i=0;i<geometry.points;i++){
  unsigned int j=i+1;
  while(j<geometry.points){
   if(!WeldClose(geometry.vertices[i],geometry.vertices[j])){j++;continue;}
   for(unsigned int f=0;f<geometry.polygons;f++){
    WorldText::Polygon& polygon=geometry.faces[f];
    for(unsigned int k=0;k<polygon.count;k++){
     unsigned int& index=polygon.indices[k];
     if(index==j)index=i;else if(index>j)index--;
    }
   }
   for(unsigned int k=j+1;k<geometry.points;k++){
    geometry.vertices[k-1]=geometry.vertices[k];
    if(geometry.colors)geometry.colors[k-1]=geometry.colors[k];
   }
   geometry.points--;
  }
 }
}
inline void CleanCycles(WorldText::Geometry& geometry){
 unsigned int kept=0,used=0;
 for(unsigned int f=0;f<geometry.polygons;f++){
  WorldText::Polygon polygon=geometry.faces[f];unsigned int k=0;
  // VA100586F0: remove the current member of an adjacent equal pair,
  // including the closing pair. Nonadjacent repeats/collinear corners remain.
  while(k<polygon.count){
   unsigned int next=k+1==polygon.count?0:k+1;
   if(polygon.indices[k]!=polygon.indices[next]){k++;continue;}
   for(unsigned int n=k+1;n<polygon.count;n++)polygon.indices[n-1]=polygon.indices[n];
   polygon.count--;
  }
  if(polygon.count<3)continue;
  // Compact the shared index buffer too: Expansion copies its prefix and
  // reconstructs each face's slice from the surviving counts.
  for(unsigned int n=0;n<polygon.count;n++)geometry.indices[used+n]=polygon.indices[n];
  polygon.indices=geometry.indices+used;used+=polygon.count;
  geometry.faces[kept++]=polygon;
 }
 geometry.polygons=kept;geometry.references=used;
}
inline void Prepare(WorldText::Scene& scene){
 // Per loaded source file, before prefab transforms: pool weld, cyclic
 // index cleanup, inaccurate-coordinate snap, then plane refresh.
 for(unsigned int g=0;g<scene.geometryCount;g++){
  WorldText::Geometry& geometry=scene.geometry[g];Weld(geometry);CleanCycles(geometry);
  for(unsigned int v=0;v<geometry.points;v++){
   WorldText::Vec3& p=geometry.vertices[v];p.x=RegionCoordinate(p.x);p.y=RegionCoordinate(p.y);p.z=RegionCoordinate(p.z);
  }
  for(unsigned int f=0;f<geometry.polygons;f++){
   WorldText::Polygon& polygon=geometry.faces[f];
   polygon.validPlane=WorldText::ComputePlane(geometry.vertices,polygon.indices,polygon.count,polygon.normal,polygon.distance);
  }
 }
 scene.activePoints=scene.activePolygons=0;
 for(unsigned int n=0;n<scene.nodeCount;n++)if(scene.nodes[n].kind==1){
  const WorldText::Geometry& geometry=scene.geometry[scene.nodes[n].geometry];
  scene.activePoints+=geometry.points;scene.activePolygons+=geometry.polygons;
 }
}
}
