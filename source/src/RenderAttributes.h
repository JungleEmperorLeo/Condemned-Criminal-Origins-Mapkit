#pragma once
#include "SourceRenderColor.h"
namespace RenderAttributes {
struct Vertex {WorldText::Vec3 position;float uv[8];unsigned int color;WorldText::Vec3 normal,tangent,binormal;};
static_assert(sizeof(Vertex)==84,"render vertex stride");
inline double Root(double x){double y;__asm {
 fld x
 fsqrt
 fstp y
 }return y;}
inline bool Normalize(WorldText::Vec3& v){double length=Root((double)v.x*v.x+(double)v.y*v.y+(double)v.z*v.z);if(length<1e-12)return false;v.x=(float)(v.x/length);v.y=(float)(v.y/length);v.z=(float)(v.z/length);return true;}
inline WorldText::Vec3 Cross(const WorldText::Vec3& a,const WorldText::Vec3& b){WorldText::Vec3 v={a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x};return v;}
inline void Add(WorldText::Vec3& a,const WorldText::Vec3& b){a.x+=b.x;a.y+=b.y;a.z+=b.z;}
inline bool Project(WorldText::Vec3& v,const WorldText::Vec3& normal){if(!Normalize(v))return false;float d=WorldText::Dot(v,normal);v.x-=d*normal.x;v.y-=d*normal.y;v.z-=d*normal.z;return Normalize(v);}
inline bool CornerBasis(const WorldText::Geometry& g,const WorldText::Polygon& face,unsigned int point,float cosine,WorldText::Vec3& normal,WorldText::Vec3& tangent,WorldText::Vec3& bitangent,bool triangleWeighted=false){
 normal=WorldText::Vec3();tangent=WorldText::Vec3();bitangent=WorldText::Vec3();
 for(unsigned int i=0;i<g.polygons;i++){const WorldText::Polygon& other=g.faces[i];if(WorldText::Dot(face.normal,other.normal)<cosine)continue;unsigned int j=0;for(;j<other.count;j++)if(other.indices[j]==point)break;if(j==other.count)continue;
  const WorldText::Mapping* map=0;for(unsigned int m=0;m<other.mappingCount;m++)if(other.mappings[m].channel==0)map=&other.mappings[m];if(!map||!map->valid)return false;
  WorldText::Vec3 p=map->p,q=map->q;if(!Project(p,other.normal)||!Project(q,other.normal))return false;Add(normal,other.normal);unsigned int weight=triangleWeighted?(j==0?other.count-2:(j==1||j+1==other.count?1:2)):1;for(unsigned int w=0;w<weight;w++)Add(tangent,p);Add(bitangent,q);
 }
 const WorldText::Mapping* current=0;for(unsigned int m=0;m<face.mappingCount;m++)if(face.mappings[m].channel==0)current=&face.mappings[m];if(!current)return false;
 float sign=WorldText::Dot(Cross(face.normal,current->p),current->q);if(sign==0)return false;bitangent.x=sign<0?-1.f:1.f;bitangent.y=bitangent.z=0;
 return Normalize(normal)&&Normalize(tangent);
}
class Mesh {
 bool Fail(const char* message){memory.Clear();vertices=0;indices=0;count=triangles=0;error=message;return false;}
public:
 PrefabScene::Arena memory;Vertex* vertices;unsigned short* indices;unsigned int count,triangles;const char* error;
 Mesh():vertices(0),indices(0),count(0),triangles(0),error(0){}
 bool Build(const WorldText::Geometry& g,unsigned int color,unsigned int usedUVChannels=4,float uvWeldTolerance=0,float creaseAngle=-1,unsigned int tangentMethod=0,bool authoredWhiteRGB=false){
  memory.Clear();vertices=0;indices=0;count=triangles=0;error=0;
  if(tangentMethod!=0&&tangentMethod!=2&&tangentMethod!=4&&tangentMethod!=8)return Fail("Unknown tangent selector");
  if(tangentMethod==8)creaseAngle=-1;
  if((tangentMethod==2||tangentMethod==4)&&creaseAngle<0)return Fail("Tangent mode requires crease angle");
  if(!WorldText::Finite(creaseAngle)||creaseAngle< -1||creaseAngle>180||!WorldText::Finite(uvWeldTolerance)||uvWeldTolerance<0||uvWeldTolerance>0.01f||!usedUVChannels||usedUVChannels>4||!g.polygons||g.references>65535)return Fail("Unsupported render geometry size");
  float cosine=creaseAngle>=0?PrefabScene::Matrix::Cos(creaseAngle*0.017453292519943295f):0;if(cosine<0)cosine=0;
  unsigned int total=0;for(unsigned int p=0;p<g.polygons;p++){if(PolygonCheck::Check(g,g.faces[p]).issues)return Fail("Invalid render contour");total+=g.faces[p].count-2;}
  vertices=(Vertex*)memory.Allocate(g.references*sizeof(Vertex));indices=(unsigned short*)memory.Allocate(total*6);
  if(!vertices||!indices)return Fail("Render attribute allocation failed");
   WorldText::Vec3* weldNormals=(tangentMethod==2||tangentMethod==4)?(WorldText::Vec3*)memory.Allocate(g.references*12):0;
   if((tangentMethod==2||tangentMethod==4)&&!weldNormals)return Fail("Render normal-key allocation failed");
  for(unsigned int p=0;p<g.polygons;p++){
   const WorldText::Polygon& face=g.faces[p];const WorldText::Mapping* maps[4]={};
   for(unsigned int m=0;m<face.mappingCount;m++)if(face.mappings[m].channel<4)maps[face.mappings[m].channel]=&face.mappings[m];
   for(unsigned int m=0;m<4;m++)if(!maps[m]||!maps[m]->valid)return Fail("Four valid UV mappings required");
   WorldText::Vec3 tangent=maps[0]->p,bitangent=maps[0]->q;if(!Normalize(bitangent)||!Normalize(tangent))return Fail("Zero tangent");
   if(tangentMethod==8&&(!Project(tangent,face.normal)||!Project(bitangent,face.normal)))return Fail("Degenerate Kaldera plane basis");
   unsigned short* corners=(unsigned short*)memory.Allocate(face.count*2);if(!corners)return Fail("Render corner allocation failed");
   WorldText::Vec3* basis=creaseAngle>=0?(WorldText::Vec3*)memory.Allocate(face.count*36):0;if(creaseAngle>=0&&!basis)return Fail("Render corner basis allocation failed");
   for(unsigned int j=0;j<face.count;j++){
    if(basis&&!CornerBasis(g,face,face.indices[j],cosine,basis[j*3],basis[j*3+1],basis[j*3+2],tangentMethod==4))return Fail("Degenerate crease basis");
    if(basis&&tangentMethod==2)basis[j*3+1]=tangent;
    Vertex v={};v.position=g.vertices[face.indices[j]];v.color=color;
    if(authoredWhiteRGB&&!SourceRenderColor::PackWhiteRGB(g.colors?(float)g.colors[face.indices[j]].a:255.f,v.color))return Fail("Invalid authored alpha");
    for(unsigned int m=0;m<usedUVChannels;m++){WorldText::Vec3 delta={v.position.x-maps[m]->origin.x,v.position.y-maps[m]->origin.y,v.position.z-maps[m]->origin.z};v.uv[m*2]=WorldText::Dot(delta,maps[m]->p);v.uv[m*2+1]=WorldText::Dot(delta,maps[m]->q);}
    unsigned int found=0;for(;found<count;found++){
     if(weldNormals&&(weldNormals[found].x!=basis[j*3].x||weldNormals[found].y!=basis[j*3].y||weldNormals[found].z!=basis[j*3].z))continue;const unsigned char* a=(const unsigned char*)&v;const unsigned char* b=(const unsigned char*)&vertices[found];unsigned int k=0;while(k<48&&a[k]==b[k])k++;if(k==48)break;if(uvWeldTolerance>0&&v.position.x==vertices[found].position.x&&v.position.y==vertices[found].position.y&&v.position.z==vertices[found].position.z&&v.color==vertices[found].color){unsigned int m=0;for(;m<usedUVChannels*2;m++){float delta=v.uv[m]-vertices[found].uv[m];if(delta>uvWeldTolerance||delta< -uvWeldTolerance)break;}if(m==usedUVChannels*2)break;}}
    if(found==count){if(weldNormals)weldNormals[count]=basis[j*3];vertices[count++]=v;}corners[j]=(unsigned short)found;
   }
   for(unsigned int j=1;j+1<face.count;j++){
    unsigned short ids[3]={corners[0],corners[j],corners[j+1]};
    for(unsigned int k=0;k<3;k++){indices[triangles*3+k]=ids[k];Vertex& v=vertices[ids[k]];unsigned int corner=k==0?0:k==1?j:j+1;Add(v.normal,basis?basis[corner*3]:face.normal);Add(v.tangent,basis?basis[corner*3+1]:tangent);Add(v.binormal,basis?basis[corner*3+2]:bitangent);}triangles++;
   }
  }
  for(unsigned int i=0;i<count;i++){Vertex& v=vertices[i];if(!Normalize(v.normal)||!Normalize(v.tangent))return Fail("Degenerate smoothed basis");if(creaseAngle>=0){float sign=v.binormal.x;v.binormal=Cross(v.normal,v.tangent);if(sign<0){v.binormal.x=-v.binormal.x;v.binormal.y=-v.binormal.y;v.binormal.z=-v.binormal.z;}if(!Normalize(v.binormal))return Fail("Degenerate projected binormal");continue;}WorldText::Vec3 handedness=v.binormal;v.binormal=Cross(v.normal,v.tangent);if(WorldText::Dot(v.binormal,handedness)<0){v.binormal.x=-v.binormal.x;v.binormal.y=-v.binormal.y;v.binormal.z=-v.binormal.z;}if(!Normalize(v.binormal))return Fail("Parallel smoothed basis");}
  return true;
 }
};
// First-occurrence position/4UV/color dedup + triangle-incidence basis; control verified only.
}
