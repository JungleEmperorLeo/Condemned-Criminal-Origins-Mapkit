#pragma once
#include "SourceNormalArithmetic.h"
namespace SourceRenderNormals {
inline double Root(double value){double result;__asm {fld value}
 __asm {fsqrt}
 __asm {fstp result}
 return result;
}
// Preserve wide Newell components through normalization; narrow only the unit normal.
inline bool Face(const WorldText::Geometry& geometry,const WorldText::Polygon& polygon,WorldText::Vec3& output,const WorldText::Vec3* coordinateOffset=0){
 if(!geometry.vertices||!polygon.indices||polygon.count<3)return false;
 if(coordinateOffset&&!WorldText::FiniteVector(*coordinateOffset))return false;
 for(unsigned int i=0;i<polygon.count;i++)if(polygon.indices[i]>=geometry.points||!WorldText::FiniteVector(geometry.vertices[polygon.indices[i]]))return false;
 double x=0,y=0,z=0;for(unsigned int i=0,j=polygon.count-1;i<polygon.count;j=i++){
  WorldText::Vec3 a=geometry.vertices[polygon.indices[j]],b=geometry.vertices[polygon.indices[i]];
  // CenterWorldAroundOrigin moves brush points with float stores before updating
  // their planes. Translation is not numerically neutral on curved geometry.
  if(coordinateOffset){a.x-=coordinateOffset->x;a.y-=coordinateOffset->y;a.z-=coordinateOffset->z;b.x-=coordinateOffset->x;b.y-=coordinateOffset->y;b.z-=coordinateOffset->z;}
  x+=(double(a.y)-b.y)*(double(a.z)+b.z);y+=(double(a.z)-b.z)*(double(a.x)+b.x);z+=(double(a.x)-b.x)*(double(a.y)+b.y);
 }
 // Zero-area face (collinear points): the original plane update (0x10055050)
 // tests the three Newell sums against 0 and stores the up vector.
 if(x==0&&y==0&&z==0){WorldText::Vec3 up={0,1,0};output=up;return true;}
 double length=Root((x*x+y*y)+z*z);if(length<=0)return false;
 WorldText::Vec3 normal={(float)(x/length),(float)(y/length),(float)(z/length)};
 if(!WorldText::FiniteVector(normal))return false;output=normal;return true;
}
// Observed FEAR crease tolerance; caller supplies the shader/brush-selected cosine.
inline bool Threshold(float cosine,float& threshold){if(!WorldText::Finite(cosine)||cosine< -1||cosine>1)return false;threshold=(float)(double(cosine)-.001);return true;}
#ifdef FEAR_SMOOTH_TRACE
}
void FearSmoothTrace(const WorldText::Geometry&,const WorldText::Vec3*,unsigned int,unsigned int,float,const WorldText::Vec3&);
namespace SourceRenderNormals {
#endif
class Builder {
 PrefabScene::Arena memory;const WorldText::Geometry* source;
public:
 WorldText::Vec3* faces;
 Builder():source(0),faces(0){}
 bool Build(const WorldText::Geometry& geometry,const WorldText::Vec3* coordinateOffset=0){
  memory.Clear();source=0;faces=0;if(!geometry.faces||!geometry.polygons||geometry.polygons>1000000)return false;
  WorldText::Vec3* normals=(WorldText::Vec3*)memory.Allocate(geometry.polygons*12);if(!normals)return false;
  for(unsigned int i=0;i<geometry.polygons;i++)if(!Face(geometry,geometry.faces[i],normals[i],coordinateOffset))return false;
  faces=normals;source=&geometry;return true;
 }
 bool Matches(const WorldText::Geometry& geometry)const{return source==&geometry&&faces!=0;}
 bool Corner(unsigned int polygon,unsigned int point,float threshold,WorldText::Vec3& output)const{
  if(!source||polygon>=source->polygons||point>=source->points||!WorldText::Finite(threshold))return false;
  WorldText::Vec3 sum={};unsigned int count=0;
  for(unsigned int p=0;p<source->polygons;p++){
   const WorldText::Polygon& face=source->faces[p];unsigned int j=0;for(;j<face.count;j++)if(face.indices[j]==point)break;if(j==face.count)continue;
   if(SourceNormalArithmetic::DotWide(faces[polygon],faces[p])<threshold)continue;
   sum.x=(float)(double(sum.x)+faces[p].x);sum.y=(float)(double(sum.y)+faces[p].y);sum.z=(float)(double(sum.z)+faces[p].z);count++;
  }
  if(!count||!SourceNormalArithmetic::Normalize(sum))return false;output=sum;
#ifdef FEAR_SMOOTH_TRACE
  FearSmoothTrace(*source,faces,polygon,point,threshold,output);
#endif
  return true;
 }
};
}
