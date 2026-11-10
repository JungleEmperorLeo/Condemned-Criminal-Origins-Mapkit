#pragma once
#include "RenderAttributes.h"
#include "RenderFrameLimits.h"
namespace SourceRenderPositionCleanup {
// The position-only prepass of original FixTJunctions (0x48960), not the
// subsequent triangle flip, degeneration filter or T-junction subdivision.
// Nearest-vector wrapper 0x53330, search 0x52E10, insert 0x52C70.
struct Range {WorldText::Vec3 minimum,maximum;};
struct Entry {WorldText::Vec3 point;unsigned int triangle;};
struct Node {Node* children;Entry* entries;unsigned int count;};
static_assert(sizeof(Entry)==16,"position pool entry");
inline float Center(float a,float b){volatile float sum=(float)((double)a+b);return (float)((double)sum*0.5);}
inline Range Child(const Range& r,unsigned int i){
 WorldText::Vec3 c={Center(r.minimum.x,r.maximum.x),Center(r.minimum.y,r.maximum.y),Center(r.minimum.z,r.maximum.z)};
 Range out=r;if(i&1)out.minimum.x=c.x;else out.maximum.x=c.x;if(i&4)out.minimum.y=c.y;else out.maximum.y=c.y;if(i&2)out.minimum.z=c.z;else out.maximum.z=c.z;return out;
}
inline bool Contains(const Range& r,const WorldText::Vec3& p){return r.minimum.x<=p.x&&r.minimum.y<=p.y&&r.minimum.z<=p.z&&r.maximum.x>=p.x&&r.maximum.y>=p.y&&r.maximum.z>=p.z;}
inline bool Overlaps(const Range& a,const Range& b){return a.minimum.x<=b.maximum.x&&a.minimum.y<=b.maximum.y&&a.minimum.z<=b.maximum.z&&a.maximum.x>=b.minimum.x&&a.maximum.y>=b.minimum.y&&a.maximum.z>=b.minimum.z;}
inline Range Around(const WorldText::Vec3& p,float radius){Range r={{(float)((double)p.x-radius),(float)((double)p.y-radius),(float)((double)p.z-radius)},{(float)((double)p.x+radius),(float)((double)p.y+radius),(float)((double)p.z+radius)}};return r;}
inline float DistanceSquared(const WorldText::Vec3& a,const WorldText::Vec3& b){
 volatile float x=(float)((double)a.x-b.x),y=(float)((double)a.y-b.y),z=(float)((double)a.z-b.z);
 return (float)(((double)x*x+(double)y*y)+(double)z*z);
}
class Pool {
 PrefabScene::Arena memory;Node root;Range bounds;
 bool Add(Node& node,const Range& range,const Entry& entry,unsigned int depth){
  if(depth>=64)return false;
  if(node.count==256){
   node.children=(Node*)memory.Allocate(8*sizeof(Node));if(!node.children)return false;memset(node.children,0,8*sizeof(Node));
   for(unsigned int p=0;p<256;p++){unsigned int c=0;for(;c<8;c++){Range sub=Child(range,c);if(Contains(sub,node.entries[p].point)){if(!Add(node.children[c],sub,node.entries[p],depth+1))return false;break;}}if(c==8)return false;}
   node.entries=0;node.count=0;
  }
  if(node.children){for(unsigned int c=0;c<8;c++){Range sub=Child(range,c);if(Contains(sub,entry.point))return Add(node.children[c],sub,entry,depth+1);}return false;}
  if(!node.entries){node.entries=(Entry*)memory.Allocate(256*sizeof(Entry));if(!node.entries)return false;}
  node.entries[node.count++]=entry;return true;
 }
 bool Nearest(const Node& node,const Range& range,const WorldText::Vec3& p,Range& query,float& limit,WorldText::Vec3& result)const{
  bool found=false;
  if(node.children){for(unsigned int c=0;c<8;c++){Range sub=Child(range,c);if(Overlaps(sub,query))found=Nearest(node.children[c],sub,p,query,limit,result)||found;}}
  else if(node.entries){
   for(unsigned int i=0;i<node.count;i++){float d=DistanceSquared(p,node.entries[i].point);if(d<limit){limit=d;result=node.entries[i].point;found=true;}}
   if(found){float radius=(float)RenderAttributes::Root(limit);query=Around(p,radius);}
  }return found;
 }
public:
 Pool(){memset(&root,0,sizeof(root));memset(&bounds,0,sizeof(bounds));}
 bool Initialize(const WorldText::Vec3* points,unsigned int count){
  memory.Clear();memset(&root,0,sizeof(root));if(!points||!count||count>RenderFrameLimits::MaximumEndpoints)return false;
  bounds.minimum=bounds.maximum=points[0];
  for(unsigned int i=0;i<count;i++){const WorldText::Vec3& p=points[i];if(!WorldText::Finite(p.x)||!WorldText::Finite(p.y)||!WorldText::Finite(p.z)||p.x>10000000||p.x< -10000000||p.y>10000000||p.y< -10000000||p.z>10000000||p.z< -10000000)return false;
   if(p.x<bounds.minimum.x)bounds.minimum.x=p.x;if(p.y<bounds.minimum.y)bounds.minimum.y=p.y;if(p.z<bounds.minimum.z)bounds.minimum.z=p.z;
   if(p.x>bounds.maximum.x)bounds.maximum.x=p.x;if(p.y>bounds.maximum.y)bounds.maximum.y=p.y;if(p.z>bounds.maximum.z)bounds.maximum.z=p.z;
  }return true;
 }
 bool Snap(WorldText::Vec3& point,unsigned int triangle,bool& matched){
  const float epsilon=0.1f;float limit=(float)((double)epsilon*epsilon);Range query=Around(point,epsilon);WorldText::Vec3 result={};matched=Nearest(root,bounds,point,query,limit,result);if(matched)point=result;
  Entry entry={point,triangle};return Add(root,bounds,entry,0);
 }
};
}
