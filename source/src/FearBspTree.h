#pragma once
#include "FearBspSplitPolicy.h"
// Prepared-contour BSP. Source extraction/CSG and disk polygon indexing are
// separate stages; callers must not pass unprepared brushes as a closed world.
namespace FearBspTree {
struct Input {FearBspSplitPolicy::Polygon polygon;unsigned int source;};
struct Fragment {Fragment* next;FearBspSplitPolicy::Polygon polygon;unsigned int source,id;};
struct Node {Node* next;Fragment* polygon;int negative,positive;unsigned int index,depth;};
struct List {Fragment* first;Fragment* last;unsigned int count;List():first(0),last(0),count(0){}void Append(Fragment* f){f->next=0;if(last)last->next=f;else first=f;last=f;count++;}};
__declspec(noinline) inline float Lerp(float a,float b,float fraction){float result;__asm {
 fld b
 fsub a
 fmul fraction
 fadd a
 fstp result
}return result;}
__declspec(noinline) inline float Fraction(float a,float b){float result;__asm {
 fld b
 fsub a
 fdivr a
 fchs
 fstp result
}return result;}
__declspec(noinline) inline void Bounds(FearBspSplitPolicy::Polygon& p){
 WorldText::Vec3 lo=p.points[0],hi=lo;
 for(unsigned int i=1;i<p.count;i++)for(unsigned int k=0;k<3;k++){float v=(&p.points[i].x)[k];if(v<(&lo.x)[k])(&lo.x)[k]=v;if(v>(&hi.x)[k])(&hi.x)[k]=v;}
 for(unsigned int k=0;k<3;k++){volatile float sum=(&lo.x)[k]+(&hi.x)[k];(&p.center.x)[k]=sum*0.5f;}
 float maxSquared=0;
 for(unsigned int i=0;i<p.count;i++){
  volatile float deltaX=p.points[i].x-p.center.x,deltaY=p.points[i].y-p.center.y,deltaZ=p.points[i].z-p.center.z;float squared;
  __asm {
  fld deltaX
  fmul deltaX
  fld deltaY
  fmul deltaY
  faddp st(1),st
  fld deltaZ
  fmul deltaZ
  faddp st(1),st
  fstp squared
  }
  if(squared>maxSquared)maxSquared=squared;
 }
 float radius;__asm {
 fld maxSquared
 fsqrt
 fstp radius
 }p.radius=radius;
}
class Builder {
 unsigned int depthLimit,fragmentLimit,pointLimit;FearBspSplitPolicy::Polygon* selection;Fragment** selectedPointers;
 struct CutScratch {WorldText::Vec3 sides[2][257];float distance[255];int side[255];};CutScratch* scratch;
 bool Fail(const char* text){error=text;return false;}
 Fragment* Own(const FearBspSplitPolicy::Polygon& polygon,unsigned int source,const WorldText::Vec3* pts,unsigned int count){
  if(fragmentCount>=fragmentLimit||count>pointLimit-pointCount){Fail("Prepared BSP capacity exceeded");return 0;}
  Fragment* f=(Fragment*)memory.Allocate(sizeof(Fragment));WorldText::Vec3* points=(WorldText::Vec3*)memory.Allocate(count*12);if(!f||!points){Fail("Prepared BSP allocation failed");return 0;}
  memcpy(points,pts,count*12);f->polygon=polygon;f->polygon.points=points;f->polygon.count=count;f->source=source;f->id=fragmentCount++;pointCount+=count;Bounds(f->polygon);
  if(!FearBspSplitPolicy::Validate(f->polygon)){Fail("Prepared BSP fragment is invalid");return 0;}return f;
 }
 bool Cut(const FearBspSplitPolicy::Polygon& plane,Fragment* f,Fragment*& negative,Fragment*& positive){
  unsigned int n=f->polygon.count;if(n>255)return Fail("Prepared BSP split contour exceeds 255 corners");
  WorldText::Vec3 (*sides)[257]=scratch->sides;float* distance=scratch->distance;int* side=scratch->side;unsigned int counts[2]={},crossings=0;
  for(unsigned int i=0;i<n;i++){distance[i]=FearBspSplitPolicy::Distance(plane,f->polygon.points[i]);side[i]=distance[i]>0.01f?1:(distance[i]<-0.01f?0:2);}
  for(unsigned int i=0;i<n;i++){
   const unsigned int j=(i+1)%n;const WorldText::Vec3& a=f->polygon.points[i];const WorldText::Vec3& b=f->polygon.points[j];
   if(side[i]==2){sides[0][counts[0]++]=a;sides[1][counts[1]++]=a;}else sides[side[i]][counts[side[i]]++]=a;
   if(side[i]==2||side[j]==2||side[i]==side[j])continue;
   // Original VA 0x10008A70: the third crossing logs "Bad poly found! > 2 edge
   // splits!" and leaves the loop; both partial fragments are kept and the false
   // result is ignored by the caller (SDK Asylum).
   if(++crossings>2){if(badCount<64)badPoints[badCount++]=a;break;}float fraction=Fraction(distance[i],distance[j]);if(!WorldText::Finite(fraction)||fraction<0||fraction>1)return Fail("Prepared BSP intersection fraction is invalid");
   WorldText::Vec3 point;
   for(unsigned int k=0;k<3;k++){const float normal=(&plane.normal.x)[k];(&point.x)[k]=normal==1.0f?plane.distance:(normal==-1.0f?-plane.distance:Lerp((&a.x)[k],(&b.x)[k],fraction));}
   sides[0][counts[0]++]=point;sides[1][counts[1]++]=point;
  }
  if(counts[0]<3||counts[1]<3)return Fail("Prepared BSP split produced invalid fragments");
  negative=Own(f->polygon,f->source,sides[0],counts[0]);positive=Own(f->polygon,f->source,sides[1],counts[1]);return negative&&positive;
 }
 bool Recurse(List& list,int& index,unsigned int depth){
  if(!list.count)return Fail("Prepared BSP cannot recurse an empty list");if(depth>depthLimit)return Fail("Prepared BSP depth limit exceeded");
  unsigned int count=0;for(Fragment* f=list.first;f;f=f->next){if(count>=fragmentLimit)return Fail("Prepared BSP working list exceeded limit");selection[count]=f->polygon;selectedPointers[count++]=f;}
  FearBspSplitPolicy::Selector selected={};if(!FearBspSplitPolicy::Select(selection,count,depth,selected))return Fail("Prepared BSP selection failed");Fragment* splitter=selectedPointers[selected.index];
  Node* node=(Node*)memory.Allocate(sizeof(Node));if(!node)return Fail("Prepared BSP allocation failed");node->polygon=splitter;node->index=nodeCount++;
  // Packer progress (0x10009880): every 1000th node reports nodes / (input polygons +
  // both halves of every split so far).
  if(!(nodeCount%1000)&&progressCount<progressCapacity)progressTotals[progressCount++]=progressBase+2*splits;node->depth=depth;node->negative=-2;node->positive=-1;node->next=0;
  if(last)last->next=node;else first=node;last=node;index=(int)node->index;if(depth>maxDepth)maxDepth=depth;
  // Processing is SKY first, NORMAL second. Appends retain each list order.
  List negative,positive;
  for(unsigned int type=0;type<2;type++)for(unsigned int item=0;item<count;item++){Fragment* f=selectedPointers[item];
   if(f!=splitter&&FearBspSplitPolicy::Type(f->polygon.flags)==type){
    FearBspSplitPolicy::Side side=FearBspSplitPolicy::FullSide(splitter->polygon,f->polygon);
    if(side==FearBspSplitPolicy::Negative)negative.Append(f);else if(side==FearBspSplitPolicy::Positive)positive.Append(f);
    else {Fragment* a=0;Fragment* b=0;if(!Cut(splitter->polygon,f,a,b))return false;negative.Append(a);positive.Append(b);splits++;}
   }
  }
  if(negative.count&&!Recurse(negative,node->negative,depth+1))return false;
  if(positive.count&&!Recurse(positive,node->positive,depth+1))return false;return true;
 }
public:
 PrefabScene::Arena memory;Node* first;Node* last;int root;const char* error;unsigned int nodeCount,fragmentCount,pointCount,maxDepth,splits,inputCount;
 unsigned int* progressTotals;unsigned int progressCount,progressCapacity,progressBase; // packer log progress only (base: every input polygon)
 // Packer log: the vertex at each third crossing ("Bad poly found! > 2 edge splits!", 0x10008E72).
 WorldText::Vec3 badPoints[64];unsigned int badCount;
 Builder():first(0),last(0),root(-2),error(0),nodeCount(0),fragmentCount(0),pointCount(0),maxDepth(0),splits(0),inputCount(0),progressTotals(0),progressCount(0),progressCapacity(0),progressBase(0),badCount(0),selection(0),selectedPointers(0),scratch(0){}
 void Clear(){memory.Clear();first=last=0;root=-2;error=0;nodeCount=fragmentCount=pointCount=maxDepth=splits=inputCount=progressCount=progressCapacity=progressBase=badCount=0;progressTotals=0;selection=0;selectedPointers=0;scratch=0;}
 bool Build(const Input* input,unsigned int count,unsigned int maxDepthAllowed=128,unsigned int maxFragmentsAllowed=100000,unsigned int maxPointsAllowed=1000000){
  Clear();depthLimit=maxDepthAllowed;fragmentLimit=maxFragmentsAllowed;pointLimit=maxPointsAllowed;
   if((!input&&count)||!count||count>100000||!depthLimit||depthLimit>4096||fragmentLimit<count||fragmentLimit>1000000||pointLimit>10000000)return Fail("Invalid prepared BSP input or limits");
  selection=(FearBspSplitPolicy::Polygon*)memory.Allocate(fragmentLimit*sizeof(FearBspSplitPolicy::Polygon));selectedPointers=(Fragment**)memory.Allocate(fragmentLimit*4);scratch=(CutScratch*)memory.Allocate(sizeof(CutScratch));progressCapacity=fragmentLimit/1000+2;progressBase=count;progressTotals=(unsigned int*)memory.Allocate(progressCapacity*4);if(!selection||!selectedPointers||!scratch||!progressTotals)return Fail("Prepared BSP allocation failed");
  List list;for(unsigned int i=0;i<count;i++){
   if(!FearBspSplitPolicy::Validate(input[i].polygon)||input[i].polygon.count>255)return Fail("Invalid prepared BSP input polygon");
   if(FearBspSplitPolicy::Type(input[i].polygon.flags)==2)continue;
   Fragment* f=Own(input[i].polygon,input[i].source,input[i].polygon.points,input[i].polygon.count);if(!f)return false;list.Append(f);inputCount++;
  }
  if(!list.count)return Fail("Prepared BSP has no eligible polygons");int built=-2;
  if(!Recurse(list,built,1)){first=last=0;nodeCount=0;return false;}root=built;return true;
 }
};
}
