#pragma once
#include "ShadowMesh.h"
#include "BrushRenderSettings.h"
namespace ShadowScene {
typedef bool (*SelectRootFace)(const PrefabScene::Model&,const WorldText::Geometry&,const WorldText::Polygon&);
struct Surface {unsigned int tree,lod,count,triangles;ShadowMesh::Vertex* vertices;unsigned short* indices;};
class Groups {
 PrefabScene::Arena memory;
 bool Fail(const char* msg){memory.Clear();surfaces=0;count=0;error=msg;return false;}
public:
 Surface* surfaces;unsigned int count;const char* error;
 Groups():surfaces(0),count(0),error(0){}
 bool Build(const PrefabScene::Model& model,const WorldModels::Partition& partition,SelectRootFace select){
  memory.Clear();surfaces=0;count=0;error=0;if(!select||!partition.groupCount||partition.groupCount>4096)return Fail("Unsupported shadow scene profile");
  for(unsigned int a=0;a<partition.assignmentCount;a++){
   const PrefabScene::Node& node=*partition.assignments[a].brush;if(!node.knownType)return Fail("Unknown brush type");
   for(const PrefabScene::Property* p=node.properties;p;p=p->next)if(FearBrushFlags::Equal(p->name,"ShadowLOD")&&(!p->string||!BrushRenderSettings::KnownLOD(p->data)))return Fail("Unknown shadow LOD");
   if(((node.surfaceFlags>>16)&3)==3)return Fail("Reserved shadow LOD bits");
  }
  surfaces=(Surface*)memory.Allocate(partition.groupCount*3*sizeof(Surface));ShadowMesh::Triangle* input=(ShadowMesh::Triangle*)memory.Allocate(6000*sizeof(ShadowMesh::Triangle));if(!surfaces||!input)return Fail("Shadow scene allocation failed");
  for(unsigned int tree=0;tree<partition.groupCount;tree++){
   const PrefabScene::Node* owner=partition.groups[tree].owner;WorldText::Vec3 offset={};
   if(owner){for(unsigned int i=0;i<3;i++)for(unsigned int j=0;j<3;j++)if(owner->orientation.m[i][j]!=(i==j?1.f:0.f))return Fail("Rotated shadow WorldModel unsupported");offset=owner->position;}
   for(unsigned int lod=0;lod<3;lod++){
    unsigned int used=0;
    for(unsigned int a=0;a<partition.assignmentCount;a++){
     const WorldModels::Assignment& assignment=partition.assignments[a];const PrefabScene::Node& node=*assignment.brush;if(assignment.group!=tree||!(node.surfaceFlags&0x40)||((node.surfaceFlags>>16)&3)!=lod)continue;
     const WorldText::Geometry& g=node.geometry;
     for(unsigned int p=0;p<g.polygons;p++){
      const WorldText::Polygon& face=g.faces[p];if(!owner&&!select(model,g,face))continue;
      if(PolygonCheck::Check(g,face).issues)return Fail("Invalid shadow contour");if(face.count-2>6000-used)return Fail("Shadow pass exceeds 6000 triangles");
      for(unsigned int j=1;j+1<face.count;j++){ShadowMesh::Triangle& t=input[used++];t.p[0]=g.vertices[face.indices[0]];t.p[1]=g.vertices[face.indices[j]];t.p[2]=g.vertices[face.indices[j+1]];t.normal=face.normal;}
     }
    }
    if(!used)continue;ShadowMesh::Mesh mesh;if(!mesh.Build(input,used,offset))return Fail(mesh.error);
    Surface& s=surfaces[count++];s.tree=tree;s.lod=lod;s.count=mesh.count;s.triangles=mesh.triangles;s.vertices=(ShadowMesh::Vertex*)memory.Allocate(mesh.count*24);s.indices=(unsigned short*)memory.Allocate(mesh.triangles*6);if(!s.vertices||!s.indices)return Fail("Shadow surface allocation failed");
    for(unsigned int i=0;i<mesh.count;i++)s.vertices[i]=mesh.vertices[i];for(unsigned int i=0;i<mesh.triangles*3;i++)s.indices[i]=mesh.indices[i];
   }
  }
  return true;
 }
};
}
