#pragma once
#include "RenderSectionRules.h"
#include "RenderVertexWeld.h"
#ifdef FEAR_RENDER_INDEX_REORDER
#include "RenderIndexReorder.h"
#endif
namespace RenderIndexedSections {
// One caller-supplied render node. Complete working triangles arrive in node
// order; section keys are source material text plus explicit render category.
// Shader cache IDs and final shader declarations belong to later preparation.
struct Triangle {RenderAttributes::Vertex vertices[3];const char* material;unsigned int category;};
class Builder {
 PrefabScene::Arena memory;
 RenderVertexWeld::Mesh weld;
 bool Fail(const char* reason){memory.Clear();sections=0;triangleSections=0;vertices=0;indices=0;sectionCount=triangleCount=vertexCount=0;error=reason;return false;}
 Builder(const Builder&);Builder& operator=(const Builder&);
public:
 RenderSectionRules::Record* sections;
 unsigned int* triangleSections;
 const RenderAttributes::Vertex* vertices;
 const unsigned int* indices;
 unsigned int sectionCount,triangleCount,vertexCount;const char* error;
 Builder():sections(0),triangleSections(0),vertices(0),indices(0),sectionCount(0),triangleCount(0),vertexCount(0),error(0){}
 bool Build(const Triangle* input,unsigned int size,bool allowExceptionalUV=false){
  Fail(0);if(size>RenderIndexedLimits::MaximumTriangles||(!input&&size))return Fail("Unsupported node triangle count");if(!size)return true;
  sections=(RenderSectionRules::Record*)memory.Allocate(size*40);
  triangleSections=(unsigned int*)memory.Allocate(size*4);
  RenderVertexWeld::Input* corners=(RenderVertexWeld::Input*)memory.Allocate(size*3*88);
  unsigned int* first=(unsigned int*)memory.Allocate(size*4);
  unsigned int* remap=(unsigned int*)memory.Allocate(size*4);
  if(!sections||!triangleSections||!corners||!first||!remap)return Fail("Node mesh allocation failed");
  for(unsigned int i=0;i<size;i++){
   const Triangle& t=input[i];unsigned int length=0;
   if(!t.material||!t.material[0]||t.category>3)return Fail("Invalid node material/category");
   while(t.material[length]&&length<4096)length++;if(length==4096)return Fail("Material name too long");
   for(unsigned int c=0;c<3;c++)if(!RenderVertexWeld::Valid(t.vertices[c],allowExceptionalUV))return Fail("Invalid node working vertex");
   RenderSectionRules::Record query={};const char* name=t.material;memcpy(query.bytes,&name,4);memcpy(query.bytes+20,&t.category,4);
   unsigned int found=RenderSectionRules::Find(sections,sectionCount,query);
   if(found==0xffffffff){char* stored=(char*)memory.Allocate(length+1);if(!stored)return Fail("Material allocation failed");memcpy(stored,name,length+1);name=stored;memcpy(query.bytes,&name,4);}
   if(!RenderSectionRules::Count(sections,sectionCount,size,query,1,triangleSections[i]))return Fail("Section count failed");
  }
  for(unsigned int i=0;i<sectionCount;i++)memcpy(sections[i].bytes+28,&i,4);
  if(!RenderSectionRules::Build(sections,sectionCount))return Fail("Section sort failed");
  unsigned int at=0;
  for(unsigned int s=0;s<sectionCount;s++){
   unsigned int old;memcpy(&old,sections[s].bytes+28,4);remap[old]=s;first[s]=at;
   for(unsigned int i=0;i<size;i++)if(triangleSections[i]==old){
    for(unsigned int c=0;c<3;c++){corners[at].reset=at==first[s]?1u:0u;corners[at++].vertex=input[i].vertices[c];}
   }
  }
  int origin[3]={};if(at!=size*3||!weld.Build(corners,at,origin,allowExceptionalUV))return Fail("Node indexed insertion failed");
  unsigned int end=0;
  for(unsigned int s=0;s<sectionCount;s++){
   unsigned int count;memcpy(&count,sections[s].bytes+24,4);
   unsigned int start=end;
   for(unsigned int c=first[s];c<first[s]+count*3;c++){unsigned int v=weld.indices[c];if(v<start)return Fail("Cross-section vertex reuse");if(v>=end)end=v+1;}
   unsigned int n=end-start;memcpy(sections[s].bytes+28,&s,4);memcpy(sections[s].bytes+32,&start,4);memcpy(sections[s].bytes+36,&n,4);
  }
  for(unsigned int i=0;i<size;i++)triangleSections[i]=remap[triangleSections[i]];
  if(end!=weld.count)return Fail("Invalid section vertex ranges");
  vertices=weld.vertices;indices=weld.indices;vertexCount=weld.count;triangleCount=size;return true;
 }
#ifdef FEAR_RENDER_INDEX_REORDER
 bool Reorder(){
  if(!triangleCount)return true;
  if(!indices||!sectionCount)return false;
  unsigned int* ordered=(unsigned int*)memory.Allocate(triangleCount*12);if(!ordered)return Fail("Cache index allocation failed");
  unsigned int input=0,output=0;
  for(unsigned int s=0;s<sectionCount;s++){
   unsigned int n;memcpy(&n,sections[s].bytes+24,4);NvCompat::Vector<RenderIndexReorder::Triangle> result;
   if(!RenderIndexReorder::Section((const RenderIndexReorder::Triangle*)(indices+input*3),n,result))return Fail("Unsupported cache index range");
   input+=n;n=result.size();if(n)memcpy(ordered+output*3,result.data(),n*12);output+=n;memcpy(sections[s].bytes+24,&n,4);
  }
  if(input!=triangleCount)return Fail("Invalid section cache partition");indices=ordered;triangleCount=output;return true;
 }
#endif
};
}
