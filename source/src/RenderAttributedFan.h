#pragma once
#include "RenderFanTriangulation.h"
#include "RenderClipVertex.h"
namespace RenderAttributedFan {
using Vertex=RenderClipVertex::Vertex;
struct Contour {const Vertex* vertices;unsigned int count,owner;};
struct Triangle {Vertex vertices[3];unsigned int owner,edgeFlags;};
class Builder {
 PrefabScene::Arena memory;
 bool Fail(const char* message){memory.Clear();triangles=0;count=filtered=candidates=0;error=message;return false;}
 Builder(const Builder&);Builder& operator=(const Builder&);

public:
 Triangle* triangles;unsigned int count,filtered,candidates;const char* error;
 Builder():triangles(0),count(0),filtered(0),candidates(0),error(0){}
 bool Build(const Contour* input,unsigned int contours){
  Fail(0);if(contours>200000||(contours&&!input))return Fail("Invalid fan contour input");
  for(unsigned int i=0;i<contours;i++){
   const Contour& c=input[i];if(c.count>65535||(c.count&&!c.vertices))return Fail("Invalid fan vertices");
   for(unsigned int j=0;j<c.count;j++)if(!WorldText::FiniteVector(c.vertices[j].point))return Fail("Nonfinite fan vertex");
   unsigned int fans=c.count>2?c.count-2:0;if(fans>1000000-candidates)return Fail("Fan triangle limit");candidates+=fans;
  }
  triangles=(Triangle*)memory.Allocate((candidates?candidates:1)*sizeof(Triangle));if(!triangles)return Fail("Fan triangle allocation failed");
  for(unsigned int i=0;i<contours;i++){
   const Contour& c=input[i];for(unsigned int fan=2;fan<c.count;fan++){
    const Vertex& a=c.vertices[0];const Vertex& b=c.vertices[fan-1];const Vertex& d=c.vertices[fan];if(!RenderFanTriangulation::Builder::Viable(a.point,b.point,d.point)){filtered++;continue;}
    Triangle& t=triangles[count++];t.vertices[0]=a;t.vertices[1]=b;t.vertices[2]=d;t.owner=c.owner;
    t.edgeFlags=2|(fan==2?1:0)|(fan==c.count-1?4:0);
   }
  }return true;
 }
};
// Intermediate endpoint attributes, owner and edge flags. Source attribute
// creation, UV expansion and T-junction fixes remain unfinished.
}

