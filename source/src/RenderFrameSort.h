#pragma once
#include "RenderFrameLimits.h"
namespace RenderFrameSort {
struct Record {unsigned char bytes[72];};
inline float X(const Record& v){float x;memcpy(&x,v.bytes,4);return x;}
inline bool Less(const Record& a,const Record& b){return X(a)<X(b);}
inline void Copy(Record& a,const Record& b){memcpy(a.bytes,b.bytes,53);memcpy(a.bytes+56,b.bytes+56,16);}
inline void Swap(Record& a,Record& b){Record t;Copy(t,a);Copy(a,b);Copy(b,t);}
inline void Median(Record* a,Record* b,Record* c){if(Less(*b,*a))Swap(*a,*b);if(Less(*c,*b)){Swap(*b,*c);if(Less(*b,*a))Swap(*a,*b);}}
inline void Pivot(Record* first,Record* mid,Record* last){unsigned int n=(unsigned int)(last-first);if(n>40){unsigned int step=(n+1)/8;Median(first,first+step,first+2*step);Median(mid-step,mid,mid+step);Median(last-2*step,last-step,last);Median(first+step,mid,last-step);}else Median(first,mid,last);}
inline void Partition(Record* first,Record* last,Record*& low,Record*& high){
 Record* mid=first+(last-first)/2;Pivot(first,mid,last-1);low=mid;high=mid+1;
 while(first<low&&!Less(*(low-1),*low)&&!Less(*low,*(low-1)))--low;
 while(high<last&&!Less(*high,*low)&&!Less(*low,*high))++high;
 Record* left=low;Record* right=high;
 for(;;){
  for(;right<last;++right){if(Less(*low,*right))continue;if(Less(*right,*low))break;if(high!=right)Swap(*high,*right);++high;}
  for(;first<left;){Record* p=left-1;if(Less(*p,*low)){--left;continue;}if(Less(*low,*p))break;--low;--left;Swap(*low,*left);}
  if(left==first&&right==last)return;
  if(left==first){if(high!=right)Swap(*low,*high);Swap(*low,*right);++low;++high;++right;}
  else if(right==last){--left;--low;--high;if(left!=low)Swap(*left,*low);Swap(*low,*high);}
  else {--left;Swap(*right,*left);++right;}
 }
}
inline void Insertion(Record* first,Record* last){for(Record* p=first+1;p<last;++p){Record value;Copy(value,*p);Record* hole=p;while(hole>first&&Less(value,*(hole-1))){Copy(*hole,*(hole-1));--hole;}Copy(*hole,value);}}
inline void PushHeap(Record* first,unsigned int hole,unsigned int top,const Record& value){while(hole>top){unsigned int parent=(hole-1)/2;if(!Less(first[parent],value))break;Copy(first[hole],first[parent]);hole=parent;}Copy(first[hole],value);}
inline void AdjustHeap(Record* first,unsigned int hole,unsigned int count,const Record& value){unsigned int top=hole,right=hole*2+2;while(right<count){unsigned int child=Less(first[right],first[right-1])?right-1:right;Copy(first[hole],first[child]);hole=child;right=hole*2+2;}if(right==count){Copy(first[hole],first[right-1]);hole=right-1;}PushHeap(first,hole,top,value);}
inline void HeapSort(Record* first,Record* last){unsigned int n=(unsigned int)(last-first);for(unsigned int parent=n/2;parent;){--parent;Record value;Copy(value,first[parent]);AdjustHeap(first,parent,n,value);}while(n>1){Record value;Copy(value,first[n-1]);Copy(first[n-1],first[0]);--n;AdjustHeap(first,0,n,value);}}
inline bool Range(Record* first,Record* last,unsigned int ideal){while(last-first>32){if(!ideal){HeapSort(first,last);return true;}Record* low;Record* high;Partition(first,last,low,high);ideal=ideal/2+ideal/4;if(low-first<last-high){if(!Range(first,low,ideal))return false;first=high;}else{if(!Range(high,last,ideal))return false;last=low;}}if(last-first>1)Insertion(first,last);return true;}
#ifdef FEAR_FRAMESORT_DUMP
}
void FearFrameSortDump(const RenderFrameSort::Record*,unsigned int);
namespace RenderFrameSort {
inline bool Build(Record* records,unsigned int count){if(!records||count>RenderFrameLimits::MaximumEndpoints)return false;FearFrameSortDump(records,count);bool ok=Range(records,records+count,count);FearFrameSortDump(records,count);return ok;}
#else
inline bool Build(Record* records,unsigned int count){return records&&count<=RenderFrameLimits::MaximumEndpoints&&Range(records,records+count,count);}
#endif
}
