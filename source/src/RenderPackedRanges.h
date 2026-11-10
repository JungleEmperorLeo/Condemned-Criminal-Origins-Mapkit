#pragma once
#include "RenderVertexPacking.h"
#include "RenderPackedLimits.h"
namespace RenderPackedRanges {
// Metadata is arena allocated; this bound limits section bookkeeping only.
// Existing vertex/index/byte caps remain independent of the section count.
static const unsigned int MaximumSections=4096;
struct Input {
 const char* material;unsigned int shaderKey,orderKey;
 const RenderVertexPacking::Element* declaration;unsigned int elements;
 const RenderAttributes::Vertex* vertices;unsigned int vertexCount,workingVertexStart;
 const unsigned int* indices;unsigned int triangles;
};
struct Range {unsigned int vertexStart,vertexCount,stride,indexStart,baseVertex,triangles,material,reserved,declaration;};
static_assert(sizeof(Range)==36,"packed render range");
inline unsigned char Fold(unsigned char c){return c>='A'&&c<='Z'?(unsigned char)(c+32):c;}
inline int CompareNames(const char* a,const char* b){for(;;a++,b++){unsigned char x=Fold((unsigned char)*a),y=Fold((unsigned char)*b);if(x!=y)return x<y?-1:1;if(!x)return 0;}}
inline bool Name(const char* a){if(!a||!*a)return false;unsigned int i=0;for(;a[i]&&i<=4096;i++)if((unsigned char)a[i]>127)return false;return i<=4096;}
inline int Compare(const Input& a,const Input& b){if(a.shaderKey!=b.shaderKey)return a.shaderKey<b.shaderKey?-1:1;int material=CompareNames(a.material,b.material);if(material)return material;if(a.orderKey==b.orderKey)return 0;return a.orderKey<b.orderKey?-1:1;}
inline bool SameDeclaration(const RenderVertexPacking::Element* a,unsigned int an,const RenderVertexPacking::Element* b,unsigned int bn){if(an!=bn)return false;const unsigned char* x=(const unsigned char*)a;const unsigned char* y=(const unsigned char*)b;for(unsigned int i=0;i<an*8;i++)if(x[i]!=y[i])return false;return true;}
// Original pair introsort0x4E6F0: equal complete keys retain the exact native
// partition/insertion/heap permutation rather than a stable-sort substitute.
struct SortItem {const Input* input;unsigned int ordinal;};
inline bool Less(const SortItem& a,const SortItem& b){return Compare(*a.input,*b.input)<0;}
inline void Swap(SortItem& a,SortItem& b){SortItem t=a;a=b;b=t;}
inline void Median(SortItem* a,SortItem* b,SortItem* c){if(Less(*b,*a))Swap(*a,*b);if(Less(*c,*b)){Swap(*b,*c);if(Less(*b,*a))Swap(*a,*b);}}
inline void Pivot(SortItem* first,SortItem* mid,SortItem* last){unsigned int n=(unsigned int)(last-first);if(n>40){unsigned int step=(n+1)/8;Median(first,first+step,first+2*step);Median(mid-step,mid,mid+step);Median(last-2*step,last-step,last);Median(first+step,mid,last-step);}else Median(first,mid,last);}
inline void Partition(SortItem* first,SortItem* last,SortItem*& low,SortItem*& high){
 SortItem* mid=first+(last-first)/2;Pivot(first,mid,last-1);low=mid;high=mid+1;
 while(first<low&&!Less(*(low-1),*low)&&!Less(*low,*(low-1)))--low;
 while(high<last&&!Less(*high,*low)&&!Less(*low,*high))++high;
 SortItem* left=low;SortItem* right=high;
 for(;;){
  for(;right<last;++right){if(Less(*low,*right))continue;if(Less(*right,*low))break;if(high!=right)Swap(*high,*right);++high;}
  for(;first<left;){SortItem* p=left-1;if(Less(*p,*low)){--left;continue;}if(Less(*low,*p))break;--low;--left;Swap(*low,*left);}
  if(left==first&&right==last)return;
  if(left==first){if(high!=right)Swap(*low,*high);Swap(*low,*right);++low;++high;++right;}
  else if(right==last){--left;--low;--high;if(left!=low)Swap(*left,*low);Swap(*low,*high);}
  else {--left;Swap(*right,*left);++right;}
 }
}
inline void Insertion(SortItem* first,SortItem* last){for(SortItem* p=first+1;p<last;++p){SortItem value=*p;SortItem* hole=p;while(hole>first&&Less(value,*(hole-1))){*hole=*(hole-1);--hole;}*hole=value;}}
inline void PushHeap(SortItem* first,unsigned int hole,unsigned int top,const SortItem& value){while(hole>top){unsigned int parent=(hole-1)/2;if(!Less(first[parent],value))break;first[hole]=first[parent];hole=parent;}first[hole]=value;}
inline void AdjustHeap(SortItem* first,unsigned int hole,unsigned int count,const SortItem& value){unsigned int top=hole,right=hole*2+2;while(right<count){unsigned int child=Less(first[right],first[right-1])?right-1:right;first[hole]=first[child];hole=child;right=hole*2+2;}if(right==count){first[hole]=first[right-1];hole=right-1;}PushHeap(first,hole,top,value);}
inline void HeapSort(SortItem* first,SortItem* last){unsigned int n=(unsigned int)(last-first);for(unsigned int parent=n/2;parent;){--parent;SortItem value=first[parent];AdjustHeap(first,parent,n,value);}while(n>1){SortItem value=first[n-1];first[n-1]=first[0];--n;AdjustHeap(first,0,n,value);}}
inline void Sort(SortItem* first,SortItem* last,unsigned int ideal){while(last-first>32){if(!ideal){HeapSort(first,last);return;}SortItem* low;SortItem* high;Partition(first,last,low,high);ideal=ideal/2+ideal/4;if(low-first<last-high){Sort(first,low,ideal);first=high;}else{Sort(high,last,ideal);last=low;}}if(last-first>1)Insertion(first,last);}
class Builder {
 PrefabScene::Arena memory;
 bool Fail(const char* reason){memory.Clear();vertices=0;indices=0;ranges=0;order=0;declarations=0;declarationElements=0;count=vertexBytes=indexCount=materialCount=declarationCount=0;error=reason;return false;}
 Builder(const Builder&);Builder& operator=(const Builder&);
public:
 unsigned char* vertices;unsigned short* indices;Range* ranges;unsigned int* order;
 const RenderVertexPacking::Element** declarations;unsigned int* declarationElements;
 unsigned int count,vertexBytes,indexCount,materialCount,declarationCount;const char* error;
 Builder():vertices(0),indices(0),ranges(0),order(0),declarations(0),declarationElements(0),count(0),vertexBytes(0),indexCount(0),materialCount(0),declarationCount(0),error(0){}
 // shaderKey is a caller-owned ordering identity representing the original
 // cached shader pointer order. Automatic material/shader discovery is outside
 // this primitive. Caller can instead provide an explicit order. sortIdeal
 // is a diagnostic depth budget; normal original use supplies section count.
 bool Build(const Input* input,unsigned int size,bool sort,unsigned int sortIdeal=0xffffffff){
  Fail(0);if(size>MaximumSections||(!input&&size))return Fail("Unsupported packed section count");if(!size)return true;
  unsigned int capacity=0,totalIndices=0,totalVertices=0;
  for(unsigned int i=0;i<size;i++){
   const Input& section=input[i];unsigned int stride=RenderVertexPacking::Stride(section.declaration,section.elements);
   if(!Name(section.material)||!stride||(!section.vertices&&section.vertexCount)||(!section.indices&&section.triangles)||section.vertexCount>65535||section.triangles>100000||section.workingVertexStart>4000000||(section.triangles&&!section.vertexCount))return Fail("Invalid packed section");
   if(!RenderPackedLimits::Fits(section.vertexCount,totalVertices,RenderPackedLimits::MaximumVertices)||!RenderPackedLimits::Fits(section.triangles*3,totalIndices,RenderPackedLimits::MaximumIndices)||!RenderPackedLimits::Fits(section.vertexCount*stride+stride,capacity,RenderPackedLimits::MaximumVertexBytes))return Fail("Packed section allocation bound exceeded");
   for(unsigned int q=0;q<section.triangles*3;q++)if(section.indices[q]<section.workingVertexStart||section.indices[q]-section.workingVertexStart>=section.vertexCount)return Fail("Working index outside section vertex range");
   totalVertices+=section.vertexCount;totalIndices+=section.triangles*3;capacity+=section.vertexCount*stride+stride;
  }
  vertices=(unsigned char*)memory.Allocate(capacity);indices=(unsigned short*)memory.Allocate(totalIndices*2);ranges=(Range*)memory.Allocate(size*36);order=(unsigned int*)memory.Allocate(size*4);declarations=(const RenderVertexPacking::Element**)memory.Allocate(size*4);declarationElements=(unsigned int*)memory.Allocate(size*4);unsigned int* materials=(unsigned int*)memory.Allocate(size*4);
  if(!vertices||!indices||!ranges||!order||!declarations||!declarationElements||!materials)return Fail("Packed ranges allocation failed");memset(vertices,0,capacity);
  for(unsigned int i=0;i<size;i++)order[i]=i;
  if(sort){
   SortItem* items=(SortItem*)memory.Allocate(size*sizeof(SortItem));if(!items)return Fail("Packed order allocation failed");
   for(unsigned int i=0;i<size;i++){items[i].input=input+i;items[i].ordinal=i;}Sort(items,items+size,sortIdeal==0xffffffff?size:sortIdeal);
   for(unsigned int i=0;i<size;i++)order[i]=items[i].ordinal;
  }
  const char* previous="";unsigned int runVertices=0,runBase=0;
  for(unsigned int i=0;i<size;i++){
   const Input& section=input[order[i]];unsigned int stride=RenderVertexPacking::Stride(section.declaration,section.elements);
   vertexBytes+=(stride-vertexBytes%stride)%stride;unsigned int start=vertexBytes/stride;
   unsigned int material=0;for(;material<materialCount;material++)if(!CompareNames(section.material,input[materials[material]].material))break;if(material==materialCount)materials[materialCount++]=order[i];
   unsigned int definition=0;for(;definition<declarationCount;definition++)if(SameDeclaration(section.declaration,section.elements,declarations[definition],declarationElements[definition]))break;if(definition==declarationCount){declarations[declarationCount]=section.declaration;declarationElements[declarationCount++]=section.elements;}
   if(CompareNames(previous,section.material)||section.vertexCount+runVertices>65535){runVertices=0;runBase=start;}
   Range& range=ranges[i];range.vertexStart=start;range.vertexCount=section.vertexCount;range.stride=stride;range.indexStart=indexCount;range.baseVertex=runBase;range.triangles=section.triangles;range.material=material;range.reserved=0;range.declaration=definition;
   for(unsigned int first=0;first<section.vertexCount;){unsigned int n=section.vertexCount-first;if(n>10000)n=10000;if(!RenderVertexPacking::Convert(n,RenderVertexPacking::Working,10,section.vertices+first,84,section.declaration,section.elements,vertices+vertexBytes+first*stride,stride))return Fail("Declared vertex conversion failed");first+=n;}
   for(unsigned int q=0;q<section.triangles*3;q++)indices[indexCount++]=(unsigned short)(section.indices[q]-section.workingVertexStart+runVertices);
   vertexBytes+=section.vertexCount*stride;runVertices+=section.vertexCount;previous=section.material;
  }
  count=size;return true;
 }
};
}
