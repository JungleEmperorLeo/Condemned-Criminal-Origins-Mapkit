#pragma once
namespace SourceMappingFiniteRepair {
// Original mapping loader repairs both directions before prefab transforms.
// Nonfinite origins are deliberately not synthesized or silently repaired.
inline void Prepare(WorldText::Scene& scene){
 for(unsigned int g=0;g<scene.geometryCount;g++)for(unsigned int p=0;p<scene.geometry[g].polygons;p++){
  WorldText::Polygon& face=scene.geometry[g].faces[p];
  for(unsigned int i=0;i<face.mappingCount;i++){
   WorldText::Mapping& m=face.mappings[i];
   if(!WorldText::FiniteVector(m.p)||!WorldText::FiniteVector(m.q)){
    m.p.x=1;m.p.y=0;m.p.z=0;m.q.x=0;m.q.y=0;m.q.z=1;
   }
   m.valid=WorldText::FiniteVector(m.origin)&&WorldText::FiniteVector(m.p)&&WorldText::FiniteVector(m.q);
  }
 }
}
}
