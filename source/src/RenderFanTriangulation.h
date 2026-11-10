#pragma once
namespace RenderFanTriangulation {
struct Contour {const WorldText::Vec3* vertices;unsigned int count,owner;};
struct Triangle {WorldText::Vec3 points[3];unsigned int owner,edgeFlags;};
class Builder {
 PrefabScene::Arena memory;
 bool Fail(const char* message){memory.Clear();triangles=0;count=filtered=candidates=0;error=message;return false;}
 Builder(const Builder&);Builder& operator=(const Builder&);
public:
 static bool Viable(const WorldText::Vec3& a,const WorldText::Vec3& b,const WorldText::Vec3& c){
  // Original VA 1004C6AA stores float bits 0x3C23D70B. It tests squared
  // lengths of both fan edges, then squared altitude to the longer edge.
  const double epsilon=0.010000000707805157;
  double ux=double(b.x)-a.x,uy=double(b.y)-a.y,uz=double(b.z)-a.z;
  double vx=double(c.x)-a.x,vy=double(c.y)-a.y,vz=double(c.z)-a.z;
  double uu=ux*ux+uy*uy+uz*uz,vv=vx*vx+vy*vy+vz*vz;if(uu<epsilon||vv<epsilon)return false;
  double x=uy*vz-uz*vy,y=uz*vx-ux*vz,z=ux*vy-uy*vx;
  return (x*x+y*y+z*z)/(uu>vv?uu:vv)>=epsilon;
 }
public:
 Triangle* triangles;unsigned int count,filtered,candidates;const char* error;
 Builder():triangles(0),count(0),filtered(0),candidates(0),error(0){}
 bool Build(const Contour* input,unsigned int contours){
  Fail(0);if(contours>200000||(contours&&!input))return Fail("Invalid fan contour input");
  for(unsigned int i=0;i<contours;i++){
   const Contour& c=input[i];if(c.count>65535||(c.count&&!c.vertices))return Fail("Invalid fan vertices");
   for(unsigned int j=0;j<c.count;j++)if(!WorldText::FiniteVector(c.vertices[j]))return Fail("Nonfinite fan vertex");
   unsigned int fans=c.count>2?c.count-2:0;if(fans>1000000-candidates)return Fail("Fan triangle limit");candidates+=fans;
  }
  triangles=(Triangle*)memory.Allocate((candidates?candidates:1)*sizeof(Triangle));if(!triangles)return Fail("Fan triangle allocation failed");
  for(unsigned int i=0;i<contours;i++){
   const Contour& c=input[i];for(unsigned int fan=2;fan<c.count;fan++){
    const WorldText::Vec3& a=c.vertices[0];const WorldText::Vec3& b=c.vertices[fan-1];const WorldText::Vec3& d=c.vertices[fan];if(!Viable(a,b,d)){filtered++;continue;}
    Triangle& t=triangles[count++];t.points[0]=a;t.points[1]=b;t.points[2]=d;t.owner=c.owner;
    t.edgeFlags=2|(fan==2?1:0)|(fan==c.count-1?4:0);
   }
  }return true;
 }
};
// Geometry/owner/edge flags only. Attributes, source clipping, ownership
// derivation and subsequent T-junction fixes are separate unfinished stages.
}
