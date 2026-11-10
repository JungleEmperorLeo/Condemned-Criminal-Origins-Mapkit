#pragma once
// Experimental load-time defaults, before prefab transforms. Explicit invalid
// mappings are retained; only absent additional channels are synthesized.
namespace SourceMissingTextureMappings {
inline bool Prepare(WorldText::Scene& scene){
 const WorldText::Vec3 normals[6]={{0,1,0},{0,-1,0},{1,0,0},{-1,0,0},{0,0,1},{0,0,-1}};
 const WorldText::Vec3 rights[6]={{1,0,0},{-1,0,0},{0,0,1},{0,0,-1},{-1,0,0},{1,0,0}};
 for(unsigned int gi=0;gi<scene.geometryCount;gi++)for(unsigned int pi=0;pi<scene.geometry[gi].polygons;pi++){
  WorldText::Polygon& face=scene.geometry[gi].faces[pi];unsigned int seen=0;
  for(unsigned int i=0;i<face.mappingCount;i++)if(face.mappings[i].channel<4)seen|=1u<<face.mappings[i].channel;
  if((seen&14)==14)continue;if(!(seen&1)||!face.validPlane)return false;
  unsigned int chosen=0;float score=WorldText::Dot(face.normal,normals[0]);
  for(unsigned int i=1;i<6;i++){float next=WorldText::Dot(face.normal,normals[i]);if(next>score){score=next;chosen=i;}}
  unsigned int missing=0;for(unsigned int ch=1;ch<4;ch++)if(!(seen&(1u<<ch)))missing++;
  WorldText::Mapping* maps=(WorldText::Mapping*)scene.doc.geometryMemory.Allocate((face.mappingCount+missing)*sizeof(WorldText::Mapping));if(!maps)return false;
  unsigned int count=face.mappingCount;for(unsigned int i=0;i<count;i++)maps[i]=face.mappings[i];
  const WorldText::Vec3& p=rights[chosen];const WorldText::Vec3& n=normals[chosen];
  for(unsigned int ch=1;ch<4;ch++)if(!(seen&(1u<<ch))){WorldText::Mapping& m=maps[count++];m.channel=ch;m.valid=true;m.origin.x=m.origin.y=m.origin.z=0;
   m.p.x=p.x*0.01f;m.p.y=p.y*0.01f;m.p.z=p.z*0.01f;
   m.q.x=(n.y*p.z-n.z*p.y)*0.01f;m.q.y=(n.z*p.x-n.x*p.z)*0.01f;m.q.z=(n.x*p.y-n.y*p.x)*0.01f;
  }face.mappings=maps;face.mappingCount=count;
 }return true;
}
}
