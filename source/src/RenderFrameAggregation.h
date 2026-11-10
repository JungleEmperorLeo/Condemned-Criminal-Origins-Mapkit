#pragma once
#include "RenderFrameSort.h"
#include "RenderFrameProjection.h"
#ifdef FEAR_FRAME_TRACE
void FearFrameTrace(const RenderFrameSort::Record*,unsigned int,unsigned int,const WorldText::Vec3&,const WorldText::Vec3&,const WorldText::Vec3&);
#endif
namespace RenderFrameAggregation {
inline unsigned int UInt(const RenderFrameSort::Record& r,unsigned int o){unsigned int v;memcpy(&v,r.bytes+o,4);return v;}
inline WorldText::Vec3 Vector(const RenderFrameSort::Record& r,unsigned int o){WorldText::Vec3 v;memcpy(&v,r.bytes+o,12);return v;}
inline double Squared(const WorldText::Vec3& v){return (double(v.x)*v.x+double(v.z)*v.z)+double(v.y)*v.y;}
// Original RVA 0x41690 chooses the least absolute normal component, including
// ties, and normalizes its perpendicular before the final frame normalization.
inline bool Perpendicular(const WorldText::Vec3& normal,WorldText::Vec3& result){
 float x=normal.x<0?-normal.x:normal.x,y=normal.y<0?-normal.y:normal.y,z=normal.z<0?-normal.z:normal.z;
 if(x<=y&&x<=z){result.x=0;result.y=-normal.z;result.z=normal.y;}
 else if(y<=x&&y<=z){result.x=-normal.z;result.y=0;result.z=normal.x;}
 else {result.x=-normal.y;result.y=normal.x;result.z=0;}
 return RenderFrameProjection::Normalize(result);
}
inline bool Near(const RenderFrameSort::Record& a,const RenderFrameSort::Record& b,unsigned int offset,unsigned int count,float threshold){double sum=0;for(unsigned int i=0;i<count;i++){float x,y;memcpy(&x,a.bytes+offset+i*4,4);memcpy(&y,b.bytes+offset+i*4,4);float d=(float)(double(x)-y);sum+=double(d)*d;}return sum<double(threshold);}
inline bool Compatible(const RenderFrameSort::Record& a,const RenderFrameSort::Record& b){unsigned int mode=UInt(a,48);return UInt(a,56)==UInt(b,56)&&mode==UInt(b,48)&&a.bytes[52]==b.bytes[52]&&Near(a,b,0,3,0.010000000707805157f)&&(mode==1||Near(a,b,60,2,0.0001f))&&(mode==2||Near(a,b,12,3,0.0001f));}
inline void Add(WorldText::Vec3& a,const WorldText::Vec3& b){a.x=(float)(double(a.x)+b.x);a.y=(float)(double(a.y)+b.y);a.z=(float)(double(a.z)+b.z);}
inline WorldText::Vec3 Cross(const WorldText::Vec3& a,const WorldText::Vec3& b,float sign){WorldText::Vec3 v={(float)(double(a.y)*b.z-double(a.z)*b.y),(float)(double(a.z)*b.x-double(a.x)*b.z),(float)(double(a.x)*b.y-double(a.y)*b.x)};v.x=(float)(double(v.x)*sign);v.y=(float)(double(v.y)*sign);v.z=(float)(double(v.z)*sign);return v;}
struct Frame {unsigned int vertex;WorldText::Vec3 normal,tangent,binormal;};
inline bool Build(const RenderFrameSort::Record* records,unsigned int count,Frame* frames){if(!records||!frames)return false;for(unsigned int i=0;i<count;i++){
 const RenderFrameSort::Record& current=records[i];unsigned int mode=UInt(current,48);if(mode>2)return false;
 WorldText::Vec3 normal=Vector(current,12),p=Vector(current,24),q=Vector(current,36);float x=RenderFrameSort::X(current),lower=(float)(double(x)-0.1f),upper=(float)(double(x)+0.1f);
 for(unsigned int j=i;j;){--j;if(RenderFrameSort::X(records[j])<lower)break;if(!Compatible(current,records[j]))continue;if(mode==2)Add(normal,Vector(records[j],12));Add(p,Vector(records[j],24));Add(q,Vector(records[j],36));}
 // Original RVA 0x482A0..0x482AD includes equality at the float-stored upper bound.
 for(unsigned int j=i+1;j<count;j++){if(RenderFrameSort::X(records[j])>upper)break;if(!Compatible(current,records[j]))continue;if(mode==2)Add(normal,Vector(records[j],12));Add(p,Vector(records[j],24));Add(q,Vector(records[j],36));}
 if(mode==2&&!RenderFrameProjection::Normalize(normal))return false;float sign=current.bytes[52]?-1.f:1.f;
 // Original tests ordered >, so NaN projections also enter the fallback.
 if(!(Squared(p)>double(0.0001f))){if(!(Squared(q)>double(0.0001f))){if(!Perpendicular(normal,p))return false;}else p=Cross(q,normal,sign);}
 q=Cross(normal,p,sign);if(!RenderFrameProjection::Normalize(p)||!RenderFrameProjection::Normalize(q))return false;
 frames[i].vertex=UInt(current,68);frames[i].normal=normal;frames[i].tangent=p;frames[i].binormal=q;
#ifdef FEAR_FRAME_TRACE
 FearFrameTrace(records,count,i,normal,p,q);
#endif
 }return true;}
}
