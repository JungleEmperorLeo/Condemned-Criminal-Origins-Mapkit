#pragma once
#include "RenderClipVertex.h"
#include "SourceRenderColor.h"
#include "SourceSurfacePreparation.h"
#include "SourceRenderNormals.h"
namespace SourceAttributedContour {
// Explicit flat-normal/white-RGB profile; shader and smoothing selection is external.
inline bool BuildFlatWhiteRGB(const WorldText::Geometry& geometry,const unsigned int* indices,unsigned int count,const WorldText::Vec3& offset,const WorldText::Vec3& normal,RenderClipVertex::Vertex* output,unsigned int capacity,const unsigned int* sourceVertexTags=0){
 if(!indices||!output||count<3||count>capacity||!geometry.vertices||!WorldText::FiniteVector(offset)||!WorldText::FiniteVector(normal)||(normal.x==0&&normal.y==0&&normal.z==0))return false;
 for(unsigned int i=0;i<count;i++)if(indices[i]>=geometry.points||!WorldText::FiniteVector(geometry.vertices[indices[i]]))return false;
 for(unsigned int i=0;i<count;i++){
  RenderClipVertex::Vertex v={};v.flags=sourceVertexTags?sourceVertexTags[indices[i]]:0;v.point=geometry.vertices[indices[i]];
  v.point.x-=offset.x;v.point.y-=offset.y;v.point.z-=offset.z;
  WorldText::PointColor white={255,255,255,255};
  SourceRenderColor::AuthoredWhiteRGB(geometry.colors?geometry.colors[indices[i]]:white,v.attributes);
  v.attributes[4]=normal.x;v.attributes[5]=normal.y;v.attributes[6]=normal.z;output[i]=v;
 }
 return true;
}
inline bool BuildPreparedFlatWhiteRGB(const SourceSurfacePreparation::Surface& surface,unsigned int ordinal,const WorldText::Vec3& offset,const WorldText::Vec3& normal,RenderClipVertex::Vertex* output,unsigned int capacity,const unsigned int* sourceVertexTags=0){
 if(!surface.owner||surface.polygonIndex>=surface.owner->geometry.polygons)return false;
 const WorldText::Geometry& geometry=surface.owner->geometry;
 SourceSurfacePreparation::Fragment fragment;
 if(!SourceSurfacePreparation::FragmentAt(geometry.faces[surface.polygonIndex],ordinal,fragment))return false;
 return BuildFlatWhiteRGB(geometry,fragment.indices,fragment.count,offset,normal,output,capacity,sourceVertexTags);
}
inline bool BuildPreparedSmoothWhiteRGB(const SourceSurfacePreparation::Surface& surface,unsigned int ordinal,const WorldText::Vec3& offset,const SourceRenderNormals::Builder& normals,float threshold,RenderClipVertex::Vertex* output,unsigned int capacity,const unsigned int* sourceVertexTags=0){
 if(!surface.owner||!normals.Matches(surface.owner->geometry)||surface.polygonIndex>=surface.owner->geometry.polygons)return false;
 const WorldText::Geometry& geometry=surface.owner->geometry;SourceSurfacePreparation::Fragment fragment;
 if(!SourceSurfacePreparation::FragmentAt(geometry.faces[surface.polygonIndex],ordinal,fragment)||!BuildFlatWhiteRGB(geometry,fragment.indices,fragment.count,offset,normals.faces[surface.polygonIndex],output,capacity,sourceVertexTags))return false;
 for(unsigned int i=0;i<fragment.count;i++){WorldText::Vec3 normal;if(!normals.Corner(surface.polygonIndex,fragment.indices[i],threshold,normal))return false;output[i].attributes[4]=normal.x;output[i].attributes[5]=normal.y;output[i].attributes[6]=normal.z;}
 return true;
}
}
