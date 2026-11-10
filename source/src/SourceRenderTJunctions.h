#pragma once
#include "SourceRenderPositionCleanup.h"
#include "RenderExceptionalUV.h"
namespace SourceRenderTJunctions {
// FEAR FixTJunctions 0x48960, with the source triangle identity carried through
// subdivision. The caller supplies one whole working mesh before sector routing.
struct Triangle {unsigned int origin,edgeFlags;RenderAttributes::Vertex vertices[3];};
static_assert(sizeof(Triangle)==260,"source T-junction packet triangle");
// Bound source storage in bytes rather than a small-map triangle count.
// This also keeps size*3 candidate entries and every source byte product safe.
static const unsigned int MaximumInputTriangles=64u*1024u*1024u/sizeof(Triangle);
static const unsigned int MaximumOutputTriangles=1000000;
static_assert(3*sizeof(SourceRenderPositionCleanup::Entry)<=sizeof(Triangle),"T-junction candidate storage fits source byte bound");
struct Stats {unsigned int input,output,snapped,flipped,filtered,splits;};
inline WorldText::Vec3 Sub(const WorldText::Vec3& a,const WorldText::Vec3& b){WorldText::Vec3 v={(float)(double(a.x)-b.x),(float)(double(a.y)-b.y),(float)(double(a.z)-b.z)};return v;}
inline double WideDot(const WorldText::Vec3& a,const WorldText::Vec3& b){return (double(a.x)*b.x+double(a.y)*b.y)+double(a.z)*b.z;}
inline WorldText::Vec3 Cross(const WorldText::Vec3& a,const WorldText::Vec3& b){WorldText::Vec3 v={(float)(double(a.y)*b.z-double(a.z)*b.y),(float)(double(a.z)*b.x-double(a.x)*b.z),(float)(double(a.x)*b.y-double(a.y)*b.x)};return v;}
inline bool Near(const WorldText::Vec3& a,const WorldText::Vec3& b){WorldText::Vec3 d=Sub(a,b);return WideDot(d,d)<=double(0.1f)*0.1f;}
inline bool Unit(WorldText::Vec3& d){float length=(float)RenderAttributes::Root(WideDot(d,d));if(!length)return false;float inv=(float)(1.0/double(length));d.x=(float)(double(d.x)*inv);d.y=(float)(double(d.y)*inv);d.z=(float)(double(d.z)*inv);return true;}
inline unsigned int ColorRound(float value){double rounded=double(value)+0.5;unsigned short before,temporary;int result;__asm {fnstcw before}temporary=(unsigned short)((before&0xf3ff)|0x0c00);__asm {
 fldcw temporary
 fld rounded
 fistp result
 fldcw before
 }return (unsigned int)result;}
inline RenderAttributes::Vertex Lerp(const RenderAttributes::Vertex& a,const RenderAttributes::Vertex& b,float t){
 RenderAttributes::Vertex out;const float* first=(const float*)&a;const float* last=(const float*)&b;float* result=(float*)&out;
 for(unsigned int i=0;i<21;i++){if(i==11)continue;result[i]=(float)((double(last[i])-first[i])*t+first[i]);}
 out.color=0;for(unsigned int shift=0;shift<32;shift+=8){float value=(float)((double((b.color>>shift)&255)-((a.color>>shift)&255))*t+((a.color>>shift)&255));unsigned int c=ColorRound(value);if(c>255)c=255;out.color|=c<<shift;}return out;
}
inline bool SharedReverse(const Triangle& current,unsigned int edge,const Triangle& other){const WorldText::Vec3& a=current.vertices[edge].position;const WorldText::Vec3& b=current.vertices[(edge+1)%3].position;for(unsigned int i=0;i<3;i++)if(Near(other.vertices[i].position,b)&&Near(other.vertices[(i+1)%3].position,a))return true;return false;}
inline bool OnLine(const WorldText::Vec3& p,const WorldText::Vec3& start,const WorldText::Vec3& direction){WorldText::Vec3 d=Sub(p,start);float t=(float)((double(d.y)*direction.y+double(d.z)*direction.z)+double(d.x)*direction.x);WorldText::Vec3 projected={(float)(double(direction.x)*t),(float)(double(direction.y)*t),(float)(double(direction.z)*t)};WorldText::Vec3 r=Sub(d,projected);return WideDot(r,r)<=double(0.010000000707805157f);}
inline bool CollinearEdge(const Triangle& current,const Triangle& other){for(unsigned int edge=0;edge<3;edge++){const WorldText::Vec3& start=current.vertices[edge].position;WorldText::Vec3 direction=Sub(current.vertices[(edge+1)%3].position,start);if(!Unit(direction))continue;for(unsigned int i=0;i<3;i++)if(OnLine(other.vertices[i].position,start,direction)&&OnLine(other.vertices[(i+1)%3].position,start,direction))return true;}return false;}
inline bool Split(const Triangle& current,const WorldText::Vec3& point,unsigned int blocked,Triangle& left,Triangle& right){
 for(unsigned int i=0;i<3;i++)if(Near(current.vertices[i].position,point))return false;
 for(unsigned int edge=0;edge<3;edge++){
  if(!(current.edgeFlags&(1u<<edge))||(blocked&(1u<<edge)))continue;unsigned int end=(edge+1)%3,other=3-edge-end;
  const RenderAttributes::Vertex& a=current.vertices[edge];const RenderAttributes::Vertex& b=current.vertices[end];WorldText::Vec3 direction=Sub(b.position,a.position);
  float squared=(float)WideDot(direction,direction);if(squared<0.001f)continue;float length=(float)RenderAttributes::Root(squared);float inv=(float)(1.0/double(length));direction.x=(float)(double(direction.x)*inv);direction.y=(float)(double(direction.y)*inv);direction.z=(float)(double(direction.z)*inv);
  WorldText::Vec3 delta=Sub(point,a.position);float distance=(float)((double(delta.z)*direction.z+double(delta.y)*direction.y)+double(delta.x)*direction.x);
  if(distance<0.1f||double(distance)>double(length)-0.1f)continue;
  WorldText::Vec3 scaled={(float)(double(direction.x)*distance),(float)(double(direction.y)*distance),(float)(double(direction.z)*distance)};
  WorldText::Vec3 projected={(float)(double(a.position.x)+scaled.x),(float)(double(a.position.y)+scaled.y),(float)(double(a.position.z)+scaled.z)};WorldText::Vec3 residual=Sub(point,projected);float lineSquared=(float)WideDot(residual,residual);if(lineSquared>0.01f)continue;
  float t=(float)(double(distance)/length);left=right=current;left.vertices[end]=Lerp(a,b,t);left.vertices[end].position=point;right.vertices[edge]=left.vertices[end];left.edgeFlags&=~(1u<<end);right.edgeFlags&=~(1u<<other);return true;
 }return false;
}
class Builder {
 typedef SourceRenderPositionCleanup::Range Range;typedef SourceRenderPositionCleanup::Entry Entry;
 struct Node {Node* children;Entry* entries;unsigned int count;};
 struct Link {Triangle triangle;Link* next;};
 PrefabScene::Arena memory;Node root;Range bounds;Triangle* prepared;unsigned int inputCount;Entry* candidates;unsigned int candidateCount;Link* first;Link* last;
 bool Fail(const char* why){memory.Clear();vertices=0;count=0;prepared=0;first=last=0;error=why;return false;}
 bool Add(Node& node,const Range& range,const Entry& entry,unsigned int depth){
  if(depth>=64)return false;if(node.count==256){node.children=(Node*)memory.Allocate(8*sizeof(Node));if(!node.children)return false;memset(node.children,0,8*sizeof(Node));for(unsigned int i=0;i<256;i++){unsigned int c=0;for(;c<8;c++){Range sub=SourceRenderPositionCleanup::Child(range,c);if(SourceRenderPositionCleanup::Contains(sub,node.entries[i].point)){if(!Add(node.children[c],sub,node.entries[i],depth+1))return false;break;}}if(c==8)return false;}node.entries=0;node.count=0;}
  if(node.children){for(unsigned int c=0;c<8;c++){Range sub=SourceRenderPositionCleanup::Child(range,c);if(SourceRenderPositionCleanup::Contains(sub,entry.point))return Add(node.children[c],sub,entry,depth+1);}return false;}
  if(!node.entries){node.entries=(Entry*)memory.Allocate(256*sizeof(Entry));if(!node.entries)return false;}node.entries[node.count++]=entry;return true;
 }
 bool Nearest(const Node& node,const Range& range,const WorldText::Vec3& point,Range& query,float& limit,WorldText::Vec3& result)const{
  bool found=false;if(node.children){for(unsigned int c=0;c<8;c++){Range sub=SourceRenderPositionCleanup::Child(range,c);if(SourceRenderPositionCleanup::Overlaps(sub,query))found=Nearest(node.children[c],sub,point,query,limit,result)||found;}}
  else if(node.entries){for(unsigned int i=0;i<node.count;i++){float d=SourceRenderPositionCleanup::DistanceSquared(point,node.entries[i].point);if(d<limit){limit=d;result=node.entries[i].point;found=true;}}if(found)query=SourceRenderPositionCleanup::Around(point,(float)RenderAttributes::Root(limit));}return found;
 }
 bool Query(const Node& node,const Range& range,const Range& query){if(node.children){for(unsigned int c=0;c<8;c++){Range sub=SourceRenderPositionCleanup::Child(range,c);if(SourceRenderPositionCleanup::Overlaps(sub,query)&&!Query(node.children[c],sub,query))return false;}}
  else if(node.entries){for(unsigned int i=0;i<node.count;i++)if(SourceRenderPositionCleanup::Contains(query,node.entries[i].point)){if(candidateCount>=inputCount*3)return false;candidates[candidateCount++]=node.entries[i];}}return true;
 }
 bool Push(Link*& stack,const Triangle& triangle){Link* node=(Link*)memory.Allocate(sizeof(Link));if(!node)return false;node->triangle=triangle;node->next=stack;stack=node;return true;}
 bool Emit(const Triangle& triangle){if(count>=MaximumOutputTriangles)return false;Link* node=(Link*)memory.Allocate(sizeof(Link));if(!node)return false;node->triangle=triangle;node->next=0;if(last)last->next=node;else first=node;last=node;++count;return true;}
public:
 Triangle* vertices;unsigned int count;Stats stats;const char* error;
 Builder():prepared(0),inputCount(0),candidates(0),candidateCount(0),first(0),last(0),vertices(0),count(0),error(0){memset(&stats,0,sizeof(stats));memset(&root,0,sizeof(root));}
 bool Build(const Triangle* input,unsigned int size,bool allowExceptionalUV=false){
  Fail(0);memset(&stats,0,sizeof(stats));memset(&root,0,sizeof(root));if(!input||!size||size>MaximumInputTriangles)return Fail("Invalid T-junction triangle count");inputCount=size;stats.input=size;
  prepared=(Triangle*)memory.Allocate(size*sizeof(Triangle));candidates=(Entry*)memory.Allocate(size*3*sizeof(Entry));if(!prepared||!candidates)return Fail("T-junction source allocation failed");memcpy(prepared,input,size*sizeof(Triangle));bounds.minimum=bounds.maximum=input[0].vertices[0].position;
  for(unsigned int i=0;i<size;i++){if(input[i].edgeFlags>7)return Fail("Invalid T-junction exterior flags");for(unsigned int j=0;j<3;j++){const RenderAttributes::Vertex& v=input[i].vertices[j];const float* fields=(const float*)&v;for(unsigned int f=0;f<21;f++)if(f!=11&&!(f>=3&&f<=10?RenderExceptionalUV::Allowed(fields[f],allowExceptionalUV):WorldText::Finite(fields[f])))return Fail("Non-finite T-junction vertex");const WorldText::Vec3& p=v.position;if(p.x>10000000||p.x< -10000000||p.y>10000000||p.y< -10000000||p.z>10000000||p.z< -10000000)return Fail("T-junction source bounds overflow");if(p.x<bounds.minimum.x)bounds.minimum.x=p.x;if(p.y<bounds.minimum.y)bounds.minimum.y=p.y;if(p.z<bounds.minimum.z)bounds.minimum.z=p.z;if(p.x>bounds.maximum.x)bounds.maximum.x=p.x;if(p.y>bounds.maximum.y)bounds.maximum.y=p.y;if(p.z>bounds.maximum.z)bounds.maximum.z=p.z;}}
  for(unsigned int i=0;i<size;i++){
   Triangle& triangle=prepared[i];WorldText::Vec3 oldFacing=Cross(Sub(triangle.vertices[0].position,triangle.vertices[1].position),Sub(triangle.vertices[2].position,triangle.vertices[1].position));bool snapped=false;
   for(unsigned int j=0;j<3;j++){WorldText::Vec3& p=triangle.vertices[j].position;float limit=(float)(double(0.1f)*0.1f);Range query=SourceRenderPositionCleanup::Around(p,0.1f);WorldText::Vec3 closest={};if(Nearest(root,bounds,p,query,limit,closest)){p=closest;snapped=true;++stats.snapped;}Entry entry={p,i};if(!Add(root,bounds,entry,0))return Fail("T-junction octree insertion failed");}
   if(snapped){WorldText::Vec3 facing=Cross(Sub(triangle.vertices[0].position,triangle.vertices[1].position),Sub(triangle.vertices[2].position,triangle.vertices[1].position));if((double(facing.z)*oldFacing.z+double(facing.y)*oldFacing.y)+double(facing.x)*oldFacing.x<0){RenderAttributes::Vertex saved=triangle.vertices[1];triangle.vertices[1]=triangle.vertices[2];triangle.vertices[2]=saved;unsigned int flags=triangle.edgeFlags;triangle.edgeFlags=(flags&1)|((flags&2)<<1)|((flags&4)>>1);++stats.flipped;}}
  }
  for(unsigned int i=0;i<size;i++){
   const Triangle& inputTriangle=prepared[i];bool degenerate=false;for(unsigned int edge=0;edge<3;edge++){WorldText::Vec3 d=Sub(inputTriangle.vertices[edge].position,inputTriangle.vertices[(edge+1)%3].position);if(WideDot(d,d)<=double(0.010000000707805157f))degenerate=true;}if(degenerate){++stats.filtered;continue;}
   Link* stack=0;if(!Push(stack,inputTriangle))return Fail("T-junction stack allocation failed");unsigned int attempts=0;
   while(stack){if(++attempts>1000000)return Fail("T-junction subdivision limit reached");Triangle current=stack->triangle;stack=stack->next;Range query={current.vertices[0].position,current.vertices[0].position};for(unsigned int j=1;j<3;j++){const WorldText::Vec3& p=current.vertices[j].position;if(p.x<query.minimum.x)query.minimum.x=p.x;if(p.y<query.minimum.y)query.minimum.y=p.y;if(p.z<query.minimum.z)query.minimum.z=p.z;if(p.x>query.maximum.x)query.maximum.x=p.x;if(p.y>query.maximum.y)query.maximum.y=p.y;if(p.z>query.maximum.z)query.maximum.z=p.z;}query.minimum.x=(float)(double(query.minimum.x)-0.1f);query.minimum.y=(float)(double(query.minimum.y)-0.1f);query.minimum.z=(float)(double(query.minimum.z)-0.1f);query.maximum.x=(float)(double(query.maximum.x)+0.1f);query.maximum.y=(float)(double(query.maximum.y)+0.1f);query.maximum.z=(float)(double(query.maximum.z)+0.1f);
    candidateCount=0;if(!Query(root,bounds,query))return Fail("T-junction candidate query failed");unsigned int blocked=0;for(unsigned int c=0;c<candidateCount;c++){unsigned int other=candidates[c].triangle;if(other==i)continue;for(unsigned int edge=0;edge<3;edge++)if(SharedReverse(current,edge,prepared[other]))blocked|=1u<<edge;}
    bool split=false;for(unsigned int c=0;c<candidateCount;c++){unsigned int other=candidates[c].triangle;if(other==i||!CollinearEdge(current,prepared[other]))continue;Triangle left,right;if(Split(current,candidates[c].point,blocked,left,right)){if(!Push(stack,left)||!Push(stack,right))return Fail("T-junction stack allocation failed");++stats.splits;split=true;break;}}
    if(!split&&!Emit(current))return Fail("T-junction output allocation failed");
   }
  }
  vertices=(Triangle*)memory.Allocate(count*sizeof(Triangle));if(count&&!vertices)return Fail("T-junction result allocation failed");unsigned int ordinal=0;for(Link* node=first;node;node=node->next)vertices[ordinal++]=node->triangle;stats.output=count;return true;
 }
};
}
