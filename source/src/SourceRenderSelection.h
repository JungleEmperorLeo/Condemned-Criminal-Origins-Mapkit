#pragma once
#include "WorldText.h"
#include "../../worldpacker-common/BrushFlags.h"
namespace SourceRenderSelection {
// Main-world profile. StreamingRegion/StreamingLink child brushes describe
// metadata volumes and do not enter render point tags, frames or triangles.
// Other object-owned geometry is a separate world-model stage, still unsupported
// by this caller. Do not silently discard it as though it were region metadata.
enum Ownership { MainWorld, StreamingVolume, Unsupported };
inline Ownership Classify(const WorldText::Scene& scene,unsigned int index){
 if(index>=scene.nodeCount||scene.nodes[index].kind!=1)return Unsupported;
 bool streaming=false,region=false;unsigned int child=index,depth=0;
 for(unsigned int ancestor=scene.nodes[index].parent;ancestor!=0xffffffffu;ancestor=scene.nodes[ancestor].parent){
  if(ancestor>=child||ancestor>=scene.nodeCount||depth++>=128)return Unsupported;
  const WorldText::SceneNode& owner=scene.nodes[ancestor];
  if(owner.kind==2){
   if(!FearBrushFlags::Equal(owner.className,"StreamingRegion")&&!FearBrushFlags::Equal(owner.className,"StreamingLink"))return Unsupported;
   if(FearBrushFlags::Equal(owner.className,"StreamingRegion"))region=true;
   streaming=true;
  }
  child=ancestor;
 }
 if(streaming&&!region)return Unsupported; // Orphan links have no verified render owner.
 return streaming?StreamingVolume:MainWorld;
}
inline bool Candidate(const WorldText::Scene& scene,unsigned int index){
 if(Classify(scene,index)!=MainWorld)return false;
 const WorldText::SceneNode& node=scene.nodes[index];
 return node.knownType&&!FearBrushFlags::Equal(node.type,"Sector")&&!FearBrushFlags::Equal(node.type,"ExtrudeSector")&&!FearBrushFlags::Equal(node.type,"SemiExtrudeSector");
}
}
