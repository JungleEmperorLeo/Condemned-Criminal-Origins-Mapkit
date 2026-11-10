#pragma once
// FEAR's world quadtree callback (RVA 0x14E20) has different clipping
// arithmetic and corner thresholds from its physical BSP splitter.
namespace FearWorldTreeIntersection {
struct Box {WorldText::Vec3 low,high,center;float minimumDimension;};
// Caller-owned scratch avoids a 6312-byte stack frame in minimal-runtime
// production programs. One workspace may be reused between sequential calls;
// concurrent/reentrant calls require separate instances.
struct Workspace {WorldText::Vec3 buffers[2][263];};
inline bool Inside(const WorldText::Vec3& p,const Box& b){return p.x>=b.low.x&&p.y>=b.low.y&&p.z>=b.low.z&&p.x<=b.high.x&&p.y<=b.high.y&&p.z<=b.high.z;}
inline float Distance(const WorldText::Vec3& p,unsigned int plane,const Box& box){
 if(plane==0)return p.x-box.low.x;if(plane==1)return box.high.x-p.x;if(plane==2)return p.z-box.low.z;return box.high.z-p.z;
}
__declspec(noinline) inline void SetBounds(const WorldText::Vec3& low,const WorldText::Vec3& high,Box& box){
 box.low=low;box.high=high;
 for(unsigned int k=0;k<3;k++){volatile float span=(&high.x)[k]-(&low.x)[k];volatile float half=span*0.5f;(&box.center.x)[k]=half+(&low.x)[k];}
 volatile float width=high.x-low.x,length=high.z-low.z;box.minimumDimension=width<length?width:length;
}
__declspec(noinline) inline float Interpolate(float a,float b,float distance,float reciprocal){
 volatile float delta=b-a;volatile float scaled=delta*distance;volatile float product=scaled*reciprocal;return product+a;
}
inline bool Intersects(const WorldText::Vec3* points,unsigned int count,const Box& box,Workspace& workspace){
 if(!points||count<3||count>255)return false;
 for(unsigned int plane=0;plane<4;plane++){bool positive=false;for(unsigned int i=0;i<count;i++)if(Distance(points[i],plane,box)>0){positive=true;break;}if(!positive)return false;}
 for(unsigned int i=0;i<count;i++)if(Inside(points[i],box))return true;
 WorldText::Vec3 (*buffers)[263]=workspace.buffers;memcpy(buffers[0],points,count*12);unsigned int from=0;
 for(unsigned int plane=0;plane<4;plane++){
  unsigned int used=0;
  for(unsigned int i=0;i<count;i++){
   const unsigned int j=(i+1)%count;const WorldText::Vec3& a=buffers[from][i];const WorldText::Vec3& b=buffers[from][j];const float da=Distance(a,plane,box),db=Distance(b,plane,box);const bool sa=da>0,sb=db>0;
   if(sa){if(used>=263)return false;buffers[1-from][used++]=a;}
   if(sa!=sb){if(used>=263)return false;volatile float reciprocal=(float)(1.0/((double)da-db));WorldText::Vec3 p;
    for(unsigned int k=0;k<3;k++)(&p.x)[k]=Interpolate((&a.x)[k],(&b.x)[k],da,reciprocal);buffers[1-from][used++]=p;
   }
  }
  if(used<=3)return false;count=used;from=1-from;
 }
 return true;
}
// Compatibility convenience API. Use the explicit workspace overload when
// building without CRT stack-probe support; this wrapper retains stack scratch.
inline bool Intersects(const WorldText::Vec3* points,unsigned int count,const Box& box){Workspace workspace;return Intersects(points,count,box,workspace);}
}
