#pragma once
#include "SourceTextureBasis.h"
#include "TextureOriginMinimize.h"
namespace SourceSurfacePreparation {
// Identity follows source provenance, never numeric equality of mappings.
// The owning expanded scene must outlive this record and its descendants.
struct Surface {const PrefabScene::Node* owner;unsigned int polygonIndex;RenderTextureCoordinates::Basis bases[4];};
struct Fragment {unsigned int count,indices[30];};
inline unsigned int FragmentCount(unsigned int vertices){return vertices<3?0:vertices<=30?1:2+(vertices-31)/28;}
inline bool FragmentAt(const WorldText::Polygon& polygon,unsigned int ordinal,Fragment& output){
 unsigned int fragments=FragmentCount(polygon.count);if(!polygon.indices||ordinal>=fragments)return false;
 Fragment prepared={};unsigned int start=ordinal?30+(ordinal-1)*28:0;
 if(ordinal){prepared.indices[prepared.count++]=polygon.indices[0];prepared.indices[prepared.count++]=polygon.indices[start-1];}
 unsigned int remaining=polygon.count-start,capacity=ordinal?28:30,n=remaining<capacity?remaining:capacity;
 for(unsigned int i=0;i<n;i++)prepared.indices[prepared.count++]=polygon.indices[start+i];
 output=prepared;return true;
}
inline bool Build(const PrefabScene::Node& owner,unsigned int polygonIndex,const WorldText::Vec3& offset,Surface& output){
 const WorldText::Geometry& geometry=owner.geometry;if(polygonIndex>=geometry.polygons)return false;
 const WorldText::Polygon& polygon=geometry.faces[polygonIndex];Fragment first;
 if(!geometry.vertices||!FragmentAt(polygon,0,first))return false;
 for(unsigned int i=0;i<polygon.count;i++)if(polygon.indices[i]>=geometry.points||!WorldText::FiniteVector(geometry.vertices[polygon.indices[i]]))return false;
 Surface prepared;prepared.owner=&owner;prepared.polygonIndex=polygonIndex;
 if(!SourceTextureBasis::Build(polygon,offset,prepared.bases))return false;
 WorldText::Vec3 points[30];for(unsigned int i=0;i<first.count;i++){
  points[i]=geometry.vertices[first.indices[i]];points[i].x-=offset.x;points[i].y-=offset.y;points[i].z-=offset.z;
 }
 if(!TextureOriginMinimize::Build(points,first.count,prepared.bases))return false;
 output=prepared;return true;
}
}
