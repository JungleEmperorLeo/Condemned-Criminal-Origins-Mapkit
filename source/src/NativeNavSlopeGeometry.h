#pragma once
// Deliberately finite source admission, not a claimed original slope limit.
// No SDK, compiled reference, allocation, CRT sqrt or map-specific data.
namespace NativeNavSlopeGeometry {
struct Plane { int p,q,r; };
inline double Abs(double value){return value<0?-value:value;}
template<class Point> bool Check(const Point* points,unsigned int count,Plane& plane,Point& low,Point& high){
 if(!points||count<3||count>8)return false;low=high=points[0];double area2=0;
 for(unsigned int i=0;i<count;i++){
  const Point &a=points[i],&b=points[(i+1)%count],&c=points[(i+2)%count];
  // The range test precedes every float->int conversion and rejects NaN/Inf.
  if(!(a.x>=-262144.f&&a.x<=262144.f&&a.y>=-262144.f&&a.y<=262144.f&&a.z>=-262144.f&&a.z<=262144.f)||a.x!=(float)(int)a.x||a.y!=(float)(int)a.y||a.z!=(float)(int)a.z)return false;
  if(a.x<low.x)low.x=a.x;if(a.y<low.y)low.y=a.y;if(a.z<low.z)low.z=a.z;if(a.x>high.x)high.x=a.x;if(a.y>high.y)high.y=a.y;if(a.z>high.z)high.z=a.z;
  double dx=double(b.x)-a.x,dy=double(b.y)-a.y,dz=double(b.z)-a.z,nx=double(c.x)-b.x,nz=double(c.z)-b.z,aa=dx*dx+dz*dz,bb=nx*nx+nz*nz,turn=dz*nx-dx*nz;
  if(aa<4096||bb<4096||turn<=0||turn*turn<.0025*aa*bb)return false;
  double cx=double(c.x)-a.x,cy=double(c.y)-a.y,cz=double(c.z)-a.z,cc=cx*cx+cz*cz,dot=dx*cx+dz*cz;
  if(cc<4096||(dot>0&&dot*dot>.99960004*aa*cc))return false;
  // The actual SDK simplification uses full 3D directions as well.
  double aa3=aa+dy*dy,cc3=cc+cy*cy,dot3=dot+dy*cy;
  if(dot3>0&&dot3*dot3>.99960004*aa3*cc3)return false;
  for(unsigned int j=0;j<count;j++)if(j!=i){double x=double(points[j].x)-a.x,z=double(points[j].z)-a.z;if(x*x+z*z<4096)return false;if(j!=(i+1)%count&&dz*x-dx*z<=0)return false;}
  area2+=double(a.z)*b.x-double(a.x)*b.z;
 }
 double width=double(high.x)-low.x,depth=double(high.z)-low.z;
 if(low.y==high.y||width<64||depth<64||width>16384||depth>16384||width>4*depth||depth>4*width||area2<8192)return false;
 // Search the small dyadic coefficient domain instead of rounding a normal or
 // using an overflowing 3D determinant. Products here are exact in double.
 for(int q=1;q<=8;q*=2)for(int p=-q;p<=q;p++)for(int r=-q;r<=q;r++){
  if((!p&&!r)||Abs(double(p))+Abs(double(r))>q)continue;bool same=true;
  for(unsigned int i=1;i<count;i++)if(q*(double(points[i].y)-points[0].y)!=p*(double(points[i].x)-points[0].x)+r*(double(points[i].z)-points[0].z)){same=false;break;}
  if(same){plane.p=p;plane.q=q;plane.r=r;return true;}
 }return false;
}
// Exact membership of further coplanar faces in an admitted plane through anchor.
template<class Point> bool OnPlane(const Point* points,unsigned int count,const Plane& plane,const Point& anchor){
 for(unsigned int i=0;i<count;i++)if(plane.q*(double(points[i].y)-anchor.y)!=plane.p*(double(points[i].x)-anchor.x)+plane.r*(double(points[i].z)-anchor.z))return false;return true;
}
inline bool Same(const Plane& a,const Plane& b){return a.p==b.p&&a.q==b.q&&a.r==b.r;}
// Exact face plane q*y=p*x+r*z+k; a horizontal face is (0,1,0,y). Slopes must
// pass Check; the horizontal convex gate is applied separately by each caller.
struct FacePlane { int p,q,r; double k; };
template<class Point> bool Face(const Point* points,unsigned int count,FacePlane& face){
 if(!points||count<3||count>8)return false;bool flat=true;for(unsigned int i=1;i<count;i++)if(points[i].y!=points[0].y)flat=false;
 if(flat){if(!(points[0].y>=-262144.f&&points[0].y<=262144.f)||points[0].y!=(float)(int)points[0].y)return false;face.p=face.r=0;face.q=1;face.k=points[0].y;return true;}
 Plane plane={};Point low,high;if(!Check(points,count,plane,low,high))return false;
 face.p=plane.p;face.q=plane.q;face.r=plane.r;face.k=plane.q*double(points[0].y)-plane.p*double(points[0].x)-plane.r*double(points[0].z);return true;
}
inline bool SameFace(const FacePlane& a,const FacePlane& b){return a.p==b.p&&a.q==b.q&&a.r==b.r&&a.k==b.k;}
inline bool SameDirection(const FacePlane& a,const FacePlane& b){return a.p==b.p&&a.q==b.q&&a.r==b.r;}
template<class Point> bool OnFace(const Point& point,const FacePlane& face){return face.q*double(point.y)==face.p*double(point.x)+face.r*double(point.z)+face.k;}
// Faces on different planes may touch in XZ only at one shared corner or along
// one complete edge of both faces; no T-junction, partial edge or vertical step.
template<class Point> bool CrossPlaneContact(const Point* a,unsigned int na,const Point* b,unsigned int nb){
 for(unsigned int pass=0;pass<2;pass++){const Point* s=pass?b:a;const Point* o=pass?a:b;unsigned int ns=pass?nb:na,no=pass?na:nb;
  for(unsigned int i=0;i<ns;i++)for(unsigned int j=0;j<no;j++){const Point &e0=o[j],&e1=o[(j+1)%no];double dx=double(e1.x)-e0.x,dz=double(e1.z)-e0.z,x=double(s[i].x)-e0.x,z=double(s[i].z)-e0.z,t=dx*x+dz*z;
   if(dz*x-dx*z==0&&t>0&&t<dx*dx+dz*dz)return false;}}
 unsigned int common=0,ia[2]={},ib[2]={};
 for(unsigned int i=0;i<na;i++)for(unsigned int j=0;j<nb;j++)if(a[i].x==b[j].x&&a[i].z==b[j].z){if(a[i].y!=b[j].y||common==2)return false;ia[common]=i;ib[common]=j;common++;}
 if(common<2)return true;
 return (ia[1]==(ia[0]+1)%na||ia[0]==(ia[1]+1)%na)&&(ib[1]==(ib[0]+1)%nb||ib[0]==(ib[1]+1)%nb);
}
// Compare a finite packed unit normal with an exact nonzero source direction.
inline bool Direction(double x,double y,double z,double a,double b,double c){
 double length=a*a+b*b+c*c,unit=x*x+y*y+z*z,cx=y*c-z*b,cy=z*a-x*c,cz=x*b-y*a;
 return length>0&&Abs(unit-1)<.000004&&x*a+y*b+z*c>0&&(cx*cx+cy*cy+cz*cz)<.000000000004*length;
}
}
