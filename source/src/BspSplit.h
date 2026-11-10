#pragma once
// First BSP primitive. Does not mutate the owned source scene or construct a tree.
namespace BspSplit {
enum Side {Coplanar=0,Front=1,Back=2,Spanning=3,Invalid=4};
inline double Sqrt(double value){double result;__asm fld value
 __asm fsqrt
 __asm fstp result
 return result;}
struct Plane {
 double x,y,z,distance,epsilon;bool valid;
 Plane():x(0),y(0),z(0),distance(0),epsilon(0),valid(false){}
 bool Set(const WorldText::Vec3& normal,float d,float tolerance){
  valid=false;if(!WorldText::FiniteVector(normal)||!WorldText::Finite(d)||!WorldText::Finite(tolerance)||tolerance<0)return false;
  double length=Sqrt((double)normal.x*normal.x+(double)normal.y*normal.y+(double)normal.z*normal.z);if(length<=0)return false;
  x=normal.x/length;y=normal.y/length;z=normal.z/length;distance=d/length;epsilon=tolerance;valid=true;return true;
 }
 double Distance(const WorldText::Vec3& p)const{return x*p.x+y*p.y+z*p.z-distance;}
 int Classify(const WorldText::Vec3& p)const{double d=Distance(p);return d>epsilon?1:d< -epsilon?-1:0;}
};
struct Fragment {
 WorldText::Vec3* vertices;unsigned int count,capacity;
 WorldText::Polygon polygon;unsigned int surfaceFlags,ownerIndex,sourcePolygonIndex;
};
struct Intersection {unsigned int firstIndex,secondIndex;double fraction;WorldText::Vec3 point;};
struct Result {
 Side side,routingSide;bool success;const char* error;Fragment front,back,coplanar;
 Intersection* intersections;unsigned int intersectionCount;
};
inline bool Same(const WorldText::Vec3& a,const WorldText::Vec3& b){return a.x==b.x&&a.y==b.y&&a.z==b.z;}
inline bool Append(Fragment& fragment,const WorldText::Vec3& point){if(fragment.count&&Same(fragment.vertices[fragment.count-1],point))return true;if(fragment.count==fragment.capacity)return false;fragment.vertices[fragment.count++]=point;return true;}
inline bool Allocate(Fragment& f,PrefabScene::Arena& arena,unsigned int capacity){f.vertices=(WorldText::Vec3*)arena.Allocate(capacity*sizeof(WorldText::Vec3));f.capacity=capacity;return f.vertices!=0;}
inline bool Finish(Fragment& f,PrefabScene::Arena& arena,const PrefabScene::Node& owner,unsigned int polygonIndex){
 if(f.count>1&&Same(f.vertices[0],f.vertices[f.count-1]))f.count--;
 if(f.count<3)return false;
 const WorldText::Polygon& source=owner.geometry.faces[polygonIndex];WorldText::Polygon& p=f.polygon;
 p.count=f.count;p.indices=(unsigned int*)arena.Allocate(f.count*sizeof(unsigned int));if(!p.indices)return false;for(unsigned int i=0;i<f.count;i++)p.indices[i]=i;
 p.validPlane=WorldText::ComputePlane(f.vertices,p.indices,p.count,p.normal,p.distance);if(!p.validPlane||WorldText::Dot(p.normal,source.normal)<=0)return false;
 lstrcpyA(p.material,source.material);lstrcpyA(p.occlusion,source.occlusion);p.mappingCount=source.mappingCount;
 if(p.mappingCount){p.mappings=(WorldText::Mapping*)arena.Allocate(p.mappingCount*sizeof(WorldText::Mapping));if(!p.mappings)return false;for(unsigned int i=0;i<p.mappingCount;i++)p.mappings[i]=source.mappings[i];}
 f.surfaceFlags=owner.surfaceFlags;f.ownerIndex=owner.index;f.sourcePolygonIndex=polygonIndex;return true;
}
inline Result Failure(Result& result,const char* error){result.success=false;result.error=error;result.front=Fragment();result.back=Fragment();result.coplanar=Fragment();result.intersections=0;result.intersectionCount=0;return result;}
__declspec(noinline) inline Result Split(const PrefabScene::Node& owner,unsigned int polygonIndex,const Plane& plane,PrefabScene::Arena& arena){
 Result result={};result.side=result.routingSide=Invalid;
 if(!plane.valid)return Failure(result,"Invalid split plane");
 if(owner.kind!=1||polygonIndex>=owner.geometry.polygons)return Failure(result,"Invalid source polygon");
 const WorldText::Geometry& g=owner.geometry;const WorldText::Polygon& p=g.faces[polygonIndex];PolygonCheck::Result checked=PolygonCheck::Check(g,p);
 if(checked.issues)return Failure(result,"Split requires a valid convex contour");
 bool hasFront=false,hasBack=false;
 for(unsigned int i=0;i<p.count;i++){int side=plane.Classify(g.vertices[p.indices[i]]);if(side>0)hasFront=true;if(side<0)hasBack=true;}
 result.side=hasFront?(hasBack?Spanning:Front):(hasBack?Back:Coplanar);
 // Keep geometric coplanarity explicit, but expose Jupiter's tie routing.
 result.routingSide=result.side==Coplanar?((plane.x*p.normal.x+plane.y*p.normal.y+plane.z*p.normal.z)>=0?Front:Back):result.side;
 if(result.side!=Spanning){
  Fragment& f=result.side==Front?result.front:result.side==Back?result.back:result.coplanar;
  if(!Allocate(f,arena,p.count))return Failure(result,"Could not allocate polygon copy");
  for(unsigned int i=0;i<p.count;i++)if(!Append(f,g.vertices[p.indices[i]]))return Failure(result,"Polygon copy overflow");
  if(!Finish(f,arena,owner,polygonIndex))return Failure(result,"Could not finish polygon copy");result.success=true;return result;
 }
 if(!Allocate(result.front,arena,2*p.count+2)||!Allocate(result.back,arena,2*p.count+2))return Failure(result,"Could not allocate split fragments");
 result.intersections=(Intersection*)arena.Allocate(p.count*sizeof(Intersection));if(!result.intersections)return Failure(result,"Could not allocate split intersections");
 for(unsigned int i=0;i<p.count;i++){
  unsigned int next=(i+1)%p.count;const WorldText::Vec3& a=g.vertices[p.indices[i]];const WorldText::Vec3& b=g.vertices[p.indices[next]];int sa=plane.Classify(a),sb=plane.Classify(b);
  if(sa>=0&&!Append(result.front,a))return Failure(result,"Front fragment overflow");if(sa<=0&&!Append(result.back,a))return Failure(result,"Back fragment overflow");
  if(sa*sb<0){
   double da=plane.Distance(a),db=plane.Distance(b),fraction=da/(da-db);
   WorldText::Vec3 hit={(float)((double)a.x+((double)b.x-a.x)*fraction),(float)((double)a.y+((double)b.y-a.y)*fraction),(float)((double)a.z+((double)b.z-a.z)*fraction)};
   // Match the legacy axis-plane fix for pure axis cuts; avoid a rounded
   // intersection drifting off the plane after repeated BSP operations.
   if((plane.x==1||plane.x== -1)&&plane.y==0&&plane.z==0)hit.x=(float)(plane.x*plane.distance);
   if((plane.y==1||plane.y== -1)&&plane.x==0&&plane.z==0)hit.y=(float)(plane.y*plane.distance);
   if((plane.z==1||plane.z== -1)&&plane.x==0&&plane.y==0)hit.z=(float)(plane.z*plane.distance);
   if(!WorldText::FiniteVector(hit))return Failure(result,"Non-finite split intersection");
   if(result.intersectionCount==2)return Failure(result,"Convex split produced more than two crossings");
   Intersection& record=result.intersections[result.intersectionCount++];record.firstIndex=p.indices[i];record.secondIndex=p.indices[next];record.fraction=fraction;record.point=hit;
   // One calculated point is shared bit-for-bit by both sides of the cut.
   if(!Append(result.front,hit)||!Append(result.back,hit))return Failure(result,"Split intersection overflow");
  }
 }
 if(!Finish(result.front,arena,owner,polygonIndex)||!Finish(result.back,arena,owner,polygonIndex))return Failure(result,"Split collapsed or could not allocate metadata");
 result.success=true;return result;
}
}
