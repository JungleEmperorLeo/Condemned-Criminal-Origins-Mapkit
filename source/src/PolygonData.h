#pragma once
// Included within WorldText after Document. Geometry owns all allocations.
inline float Sqrt(float value){float result;__asm fld value
 __asm fsqrt
 __asm fstp result
 return result;}
inline bool Finite(float f){union {float f;unsigned int u;} n;n.f=f;return (n.u&0x7f800000u)!=0x7f800000u;}
inline float Dot(const Vec3& a,const Vec3& b){return a.x*b.x+a.y*b.y+a.z*b.z;}
inline bool FiniteVector(const Vec3& v){return Finite(v.x)&&Finite(v.y)&&Finite(v.z);}
struct Mapping {unsigned int channel;Vec3 origin,p,q;bool valid;};
struct Polygon {unsigned int form,count,*indices,mappingCount;Mapping* mappings;char material[260],occlusion[260];Vec3 normal;float distance;bool validPlane;};
struct PointColor {unsigned char r,g,b,a;};
struct Geometry {unsigned int form,points,polygons,references;Vec3* vertices;Polygon* faces;unsigned int* indices;PointColor* colors;};
inline bool ComputePlane(const Vec3* vertices,const unsigned int* indices,unsigned int count,Vec3& normal,float& distance){
 normal.x=normal.y=normal.z=0;distance=0;
#ifdef FEAR_PLANE_NEWELL_DOUBLE_EXPERIMENT
 double nx=0,ny=0,nz=0;
#endif
 for(unsigned int i=0,previous=count-1;i<count;previous=i++){
  const Vec3& a=vertices[indices[previous]];const Vec3& b=vertices[indices[i]];
#ifdef FEAR_PLANE_NEWELL_DOUBLE_EXPERIMENT
  nx+=(double(a.y)-b.y)*(double(a.z)+b.z);
  ny+=(double(a.z)-b.z)*(double(a.x)+b.x);
  nz+=(double(a.x)-b.x)*(double(a.y)+b.y);
#else
  normal.x+=(a.y-b.y)*(a.z+b.z);normal.y+=(a.z-b.z)*(a.x+b.x);normal.z+=(a.x-b.x)*(a.y+b.y);
#endif
 }
#ifdef FEAR_PLANE_NEWELL_DOUBLE_EXPERIMENT
 normal.x=(float)nx;normal.y=(float)ny;normal.z=(float)nz;
#endif
 float squared=Dot(normal,normal);
 if(!Finite(squared)||squared<=0){normal.x=normal.y=normal.z=0;return false;}
 float length=Sqrt(squared);normal.x/=length;normal.y/=length;normal.z/=length;distance=Dot(normal,vertices[indices[0]]);
 return Finite(distance);
}
inline bool ReadPath(Document& doc,unsigned int polygon,const char* key,char* result){
 unsigned int form=doc.Form(polygon,key);if(!form){result[0]=0;return true;}unsigned int value=doc.Value(form);
 if(!value||doc.tokens[value].list||!doc.tokens[value].quoted||doc.Next(value))return doc.Fail("Expected quoted polygon resource path",doc.tokens[form].offset);
 if(doc.tokens[value].length>=260)return doc.Fail("Polygon resource path exceeds MAX_PATH",doc.tokens[value].offset);doc.Copy(value,result,260);return true;
}
// Polygon load (0x10057E83): leading slashes are skipped and the extension is set
// to Mat00 (0x10051050): text after the last '.' following the last slash is
// replaced, otherwise ".Mat00" is appended (Enterprise_bsp_export "Default").
inline void MaterialExtension(char* path){
 const char* start=path;while(*start=='\\'||*start=='/')start++;if(!*start){path[0]=0;return;}
 char buffer[260];lstrcpynA(buffer,start,260);int n=lstrlenA(buffer),dot=-1;
 for(int i=n-1;i>=0;i--){if(buffer[i]=='.'){dot=i+1;break;}if(buffer[i]=='/'||buffer[i]=='\\')break;}
 const char* extension="Mat00";
 if(dot<0){if(n<259){buffer[n++]='.';buffer[n]=0;}for(const char* e=extension;*e&&n<259;)buffer[n++]=*e++;buffer[n]=0;}
 else{int at=dot;for(const char* e=extension;*e&&at<259;)buffer[at++]=*e++;buffer[at]=0;}
 lstrcpyA(path,buffer);
}
inline bool ReadPolygons(Document& doc,Geometry& g,unsigned int list){
 g.faces=(Polygon*)doc.geometryMemory.Allocate((g.polygons+1)*sizeof(Polygon));
 g.indices=(unsigned int*)doc.geometryMemory.Allocate((g.references+1)*sizeof(unsigned int));
 if(!g.faces||!g.indices)return doc.Fail("Could not allocate polygon model",doc.tokens[g.form].offset);
 unsigned int pi=0,used=0;
 for(unsigned int form=doc.First(list);form;form=doc.Next(form),pi++){
  Polygon& p=g.faces[pi];p.form=form;p.indices=g.indices+used;
  for(unsigned int ref=doc.Value(doc.Form(form,"f"));ref;ref=doc.Next(ref)){if(!doc.UInt(ref,g.indices[used++]))return false;p.count++;}
  if(!ReadPath(doc,form,"material",p.material)||!ReadPath(doc,form,"occlusion",p.occlusion))return false;
  MaterialExtension(p.material);
  p.validPlane=ComputePlane(g.vertices,p.indices,p.count,p.normal,p.distance);
  unsigned int mappingsForm=doc.Form(form,"mappings"),mappings=doc.Container(mappingsForm);
  if(mappingsForm&&!mappings)return doc.Fail("Invalid polygon mappings list",doc.tokens[mappingsForm].offset);
  if(!mappings)continue;p.mappingCount=doc.Elements(mappings);
  if(p.mappingCount>32)return doc.Fail("Polygon has more than 32 mapping channels",doc.tokens[mappings].offset);
  p.mappings=(Mapping*)doc.geometryMemory.Allocate((p.mappingCount+1)*sizeof(Mapping));
  if(!p.mappings)return doc.Fail("Could not allocate texture mappings",doc.tokens[mappings].offset);
  unsigned int mi=0;
  for(unsigned int item=doc.First(mappings);item;item=doc.Next(item),mi++){
   if(!doc.tokens[item].list)return doc.Fail("Expected mapping record",doc.tokens[item].offset);
   Mapping& m=p.mappings[mi];if(!doc.UInt(doc.First(item),m.channel))return false;
   for(unsigned int previous=0;previous<mi;previous++)if(p.mappings[previous].channel==m.channel)return doc.Fail("Duplicate texture mapping channel",doc.tokens[item].offset);
   unsigned int info=doc.Form(item,"textureinfo");if(!info||doc.Elements(info)!=4)return doc.Fail("Texture mapping requires origin, P and Q",doc.tokens[item].offset);
   unsigned int o=doc.Value(info),a=doc.Next(o),b=doc.Next(a);
   if(!doc.tokens[o].list||!doc.tokens[a].list||!doc.tokens[b].list)return doc.Fail("Texture mapping vectors must be lists",doc.tokens[info].offset);
   // Real alma_Tank_ANIM001 contains FFC00000 texture vectors. Retain
   // explicit IEEE payloads, never permit them in geometry coordinates.
   if(!doc.Vector(o,m.origin,true)||!doc.Vector(a,m.p,true)||!doc.Vector(b,m.q,true))return false;
#ifdef FEAR_INPUT_Q_ZERO_EXPERIMENT
   for(unsigned int component=0;component<3;component++)if((&m.q.x)[component]==0)(&m.q.x)[component]=0;
#endif
#ifdef FEAR_EXPLICIT_MAPPING_ZERO_EXPERIMENT
   for(unsigned int component=0;component<3;component++){
    if((&m.origin.x)[component]==0)(&m.origin.x)[component]=0;
    if((&m.p.x)[component]==0)(&m.p.x)[component]=0;
    if((&m.q.x)[component]==0)(&m.q.x)[component]=0;
   }
#endif
   m.valid=FiniteVector(m.origin)&&FiniteVector(m.p)&&FiniteVector(m.q);
  }
 }return true;
}
