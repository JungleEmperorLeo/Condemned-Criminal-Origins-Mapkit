#pragma once
namespace RenderPointRemap {
// Matches original RemoveUnusedPoints' identity-map sentinel, including repeats
// when the first assigned output index equals the old index. Order is caller-owned.
inline bool Build(unsigned int pointCount,const unsigned int* references,unsigned int referenceCount,
 unsigned int* rename,unsigned int* remapped,unsigned int* copiedSource,unsigned int capacity,unsigned int& outputCount){
 outputCount=0;
 if(!pointCount||!references||!rename||!remapped||!copiedSource||capacity<referenceCount)return false;
 for(unsigned int i=0;i<referenceCount;i++)if(references[i]>=pointCount)return false;
 for(unsigned int i=0;i<pointCount;i++)rename[i]=i;
 for(unsigned int i=0;i<referenceCount;i++){
  unsigned int old=references[i];
  if(rename[old]==old){copiedSource[outputCount]=old;rename[old]=outputCount++;}
 }
 for(unsigned int i=0;i<referenceCount;i++)remapped[i]=rename[references[i]];
 return true;
}
}
