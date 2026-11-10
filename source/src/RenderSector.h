#pragma once
// Experimental convex-sector probe, verified on SampleWorld only.
// Caller supplies a normal-directed probe distance; exact FEAR epsilon is unresolved.
namespace RenderSector {
inline bool Contains(const WorldText::Geometry& sector,const WorldText::Vec3& point){
 if(!sector.points||!sector.polygons)return false;
 double center[3]={};for(unsigned int i=0;i<sector.points;i++){center[0]+=sector.vertices[i].x;center[1]+=sector.vertices[i].y;center[2]+=sector.vertices[i].z;}
 for(unsigned int k=0;k<3;k++)center[k]/=sector.points;
 for(unsigned int i=0;i<sector.polygons;i++){
  const WorldText::Polygon& f=sector.faces[i];if(!f.validPlane||!f.count)return false;
  const WorldText::Vec3& base=sector.vertices[f.indices[0]];
  double inside=(center[0]-base.x)*f.normal.x+(center[1]-base.y)*f.normal.y+(center[2]-base.z)*f.normal.z;
  double distance=((double)point.x-base.x)*f.normal.x+((double)point.y-base.y)*f.normal.y+((double)point.z-base.z)*f.normal.z;
  if(inside<0)distance=-distance;if(distance < -1e-6)return false;
 }
 return true;
}
inline bool Assigned(const WorldText::Scene& scene,const WorldText::Geometry& g,const WorldText::Polygon& face,float probeDistance){
 if(!face.count||!face.validPlane||probeDistance<=0)return false;double p[3]={};
 for(unsigned int i=0;i<face.count;i++){const WorldText::Vec3& v=g.vertices[face.indices[i]];p[0]+=v.x;p[1]+=v.y;p[2]+=v.z;}
 WorldText::Vec3 probe={(float)(p[0]/face.count+face.normal.x*probeDistance),(float)(p[1]/face.count+face.normal.y*probeDistance),(float)(p[2]/face.count+face.normal.z*probeDistance)};
 for(unsigned int i=0;i<scene.nodeCount;i++){const WorldText::SceneNode& node=scene.nodes[i];if(node.kind==1&&FearBrushFlags::Equal(node.type,"Sector")&&Contains(scene.geometry[node.geometry],probe))return true;}
 return false;
}
}
