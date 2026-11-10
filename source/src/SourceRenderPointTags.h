#pragma once
#include "SourceRenderNormals.h"
#include "SourceSurfacePreparation.h"
#include "RenderPointRemap.h"
namespace SourceRenderPointTags {
// Verified on isolated main-world controls; world-model partition must precede this selection.
inline bool MainWorldCandidate(const PrefabScene::Node& n){return n.kind==1&&n.knownType&&!FearBrushFlags::Equal(n.type,"Sector")&&!FearBrushFlags::Equal(n.type,"ExtrudeSector")&&!FearBrushFlags::Equal(n.type,"SemiExtrudeSector");}
inline bool RenderInputVisible(unsigned int flags){return !(flags&2)&&(flags&0x60)!=0;}
inline float Magnitude(const WorldText::Vec3& p){float squared=(float)((double(p.x)*p.x+double(p.y)*p.y)+double(p.z)*p.z);return (float)SourceRenderNormals::Root(squared);}
struct Entry {float magnitude;unsigned int reference;};
struct Stack {Entry entry;unsigned int head;};
class Indexer {
 PrefabScene::Arena memory;
 static void Push(Entry* heap,unsigned int hole,const Entry& value){
  while(hole){unsigned int parent=(hole-1)/2;if(!(heap[parent].magnitude>value.magnitude))break;heap[hole]=heap[parent];hole=parent;}heap[hole]=value;
 }
 void Flush(const Stack& s,const WorldText::Vec3* input,const unsigned int* next){
  for(unsigned int i=s.head;i!=0xffffffffu;i=next[i])indices[i]=pointCount;
  points[pointCount++]=input[s.entry.reference];
 }
public:
 unsigned int* indices;WorldText::Vec3* points;unsigned int pointCount;
 Indexer():indices(0),points(0),pointCount(0){}
 bool Build(const WorldText::Vec3* input,unsigned int count){
  memory.Clear();indices=0;points=0;pointCount=0;if(!input||!count||count>1000000)return false;
  Entry* heap=(Entry*)memory.Allocate(count*sizeof(Entry));Stack* stacks=(Stack*)memory.Allocate(count*sizeof(Stack));unsigned int* next=(unsigned int*)memory.Allocate(count*4);
  indices=(unsigned int*)memory.Allocate(count*4);points=(WorldText::Vec3*)memory.Allocate(count*12);if(!heap||!stacks||!next||!indices||!points)return false;
  for(unsigned int i=0;i<count;i++){if(!WorldText::FiniteVector(input[i]))return false;Entry e={Magnitude(input[i]),i};if(!WorldText::Finite(e.magnitude))return false;Push(heap,i,e);next[i]=0xffffffffu;}
  unsigned int heapCount=count,stackCount=0;
  while(heapCount){Entry current=heap[0];bool attached=false;
   for(unsigned int i=0;i<stackCount;){
    float difference=(float)(double(current.magnitude)-stacks[i].entry.magnitude);
    if(difference>.0001f){Flush(stacks[i],input,next);for(unsigned int j=i+1;j<stackCount;j++)stacks[j-1]=stacks[j];stackCount--;continue;}
    const WorldText::Vec3& a=input[current.reference];const WorldText::Vec3& b=input[stacks[i].entry.reference];WorldText::Vec3 delta={(float)(double(a.x)-b.x),(float)(double(a.y)-b.y),(float)(double(a.z)-b.z)};
    if(Magnitude(delta)<.0001f){next[current.reference]=stacks[i].head;stacks[i].head=current.reference;attached=true;break;}i++;
   }
   if(!attached){stacks[stackCount].entry=current;stacks[stackCount++].head=current.reference;}
   Entry value=heap[--heapCount];if(heapCount){unsigned int hole=0,right=2;
    while(right<heapCount){unsigned int child=heap[right].magnitude>heap[right-1].magnitude?right-1:right;heap[hole]=heap[child];hole=child;right=hole*2+2;}
    if(right==heapCount){heap[hole]=heap[right-1];hole=right-1;}Push(heap,hole,value);
   }
  }
  for(unsigned int i=0;i<stackCount;i++)Flush(stacks[i],input,next);return true;
 }
};
// Source polygon/fragment order is explicit here; full expanded-world ordering is caller work.
class Builder {
 PrefabScene::Arena memory;Indexer indexer;
public:
 unsigned int* tags;unsigned int initialPoints,remappedPoints;
 Builder():tags(0),initialPoints(0),remappedPoints(0){}
 bool Build(const WorldText::Geometry& geometry){
  memory.Clear();tags=0;initialPoints=remappedPoints=0;if(!geometry.vertices||!geometry.faces||!geometry.points)return false;
  unsigned int references=0;
  for(unsigned int i=0;i<geometry.polygons;i++)for(unsigned int j=0;j<SourceSurfacePreparation::FragmentCount(geometry.faces[i].count);j++){SourceSurfacePreparation::Fragment f;if(!SourceSurfacePreparation::FragmentAt(geometry.faces[i],j,f)||references>1000000-f.count)return false;references+=f.count;}
  WorldText::Vec3* positions=(WorldText::Vec3*)memory.Allocate(references*12);unsigned int* source=(unsigned int*)memory.Allocate(references*4);unsigned int* rename=(unsigned int*)memory.Allocate(references*4);unsigned int* result=(unsigned int*)memory.Allocate(references*4);unsigned int* copied=(unsigned int*)memory.Allocate(references*4);tags=(unsigned int*)memory.Allocate(geometry.points*4);
  if(!positions||!source||!rename||!result||!copied||!tags)return false;
  unsigned int at=0;for(unsigned int i=0;i<geometry.polygons;i++)for(unsigned int j=0;j<SourceSurfacePreparation::FragmentCount(geometry.faces[i].count);j++){SourceSurfacePreparation::Fragment f;if(!SourceSurfacePreparation::FragmentAt(geometry.faces[i],j,f))return false;for(unsigned int k=0;k<f.count;k++){if(f.indices[k]>=geometry.points)return false;source[at]=f.indices[k];positions[at++]=geometry.vertices[f.indices[k]];}}
  if(!indexer.Build(positions,references)||!RenderPointRemap::Build(indexer.pointCount,indexer.indices,references,rename,result,copied,references,remappedPoints))return false;
  initialPoints=indexer.pointCount;for(unsigned int i=0;i<geometry.points;i++)tags[i]=0xffffffffu;
  for(unsigned int i=0;i<references;i++){unsigned int& tag=tags[source[i]];if(tag!=0xffffffffu&&tag!=result[i])return false;tag=result[i];}return true;
 }
};
}
