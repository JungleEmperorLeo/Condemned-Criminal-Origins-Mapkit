#pragma once
// Original RVA 0x264E0 counts live surfaces, sorts descending with 0x26330,
// then drops zero-count records. Preserve MSVC's partition/heap tie behavior.
namespace PhysicalNormalOrder {
struct Entry {unsigned int identity,uses;};
inline bool Less(const Entry& a,const Entry& b){return a.uses>b.uses;}
inline void Swap(Entry& a,Entry& b){Entry t=a;a=b;b=t;}
inline void Median(Entry* a,Entry* b,Entry* c){if(Less(*b,*a))Swap(*a,*b);if(Less(*c,*b)){Swap(*b,*c);if(Less(*b,*a))Swap(*a,*b);}}
inline void Pivot(Entry* first,Entry* mid,Entry* last){unsigned int n=(unsigned int)(last-first);if(n>40){unsigned int step=(n+1)/8;Median(first,first+step,first+2*step);Median(mid-step,mid,mid+step);Median(last-2*step,last-step,last);Median(first+step,mid,last-step);}else Median(first,mid,last);}
inline void Partition(Entry* first,Entry* last,Entry*& low,Entry*& high){
 Entry* mid=first+(last-first)/2;Pivot(first,mid,last-1);low=mid;high=mid+1;
 while(first<low&&!Less(*(low-1),*low)&&!Less(*low,*(low-1)))--low;
 while(high<last&&!Less(*high,*low)&&!Less(*low,*high))++high;
 Entry* left=low;Entry* right=high;
 for(;;){
  for(;right<last;++right){if(Less(*low,*right))continue;if(Less(*right,*low))break;if(high!=right)Swap(*high,*right);++high;}
  for(;first<left;){Entry* p=left-1;if(Less(*p,*low)){--left;continue;}if(Less(*low,*p))break;--low;--left;Swap(*low,*left);}
  if(left==first&&right==last)return;
  if(left==first){if(high!=right)Swap(*low,*high);Swap(*low,*right);++low;++high;++right;}
  else if(right==last){--left;--low;--high;if(left!=low)Swap(*left,*low);Swap(*low,*high);}
  else {--left;Swap(*right,*left);++right;}
 }
}
inline void Insertion(Entry* first,Entry* last){for(Entry* p=first+1;p<last;++p){Entry value=*p;Entry* hole=p;while(hole>first&&Less(value,*(hole-1))){*hole=*(hole-1);--hole;}*hole=value;}}
inline void PushHeap(Entry* first,unsigned int hole,unsigned int top,const Entry& value){while(hole>top){unsigned int parent=(hole-1)/2;if(!Less(first[parent],value))break;first[hole]=first[parent];hole=parent;}first[hole]=value;}
inline void AdjustHeap(Entry* first,unsigned int hole,unsigned int count,const Entry& value){unsigned int top=hole,right=hole*2+2;while(right<count){unsigned int child=Less(first[right],first[right-1])?right-1:right;first[hole]=first[child];hole=child;right=hole*2+2;}if(right==count){first[hole]=first[right-1];hole=right-1;}PushHeap(first,hole,top,value);}
inline void HeapSort(Entry* first,Entry* last){unsigned int n=(unsigned int)(last-first);for(unsigned int parent=n/2;parent;){--parent;Entry value=first[parent];AdjustHeap(first,parent,n,value);}while(n>1){Entry value=first[n-1];first[n-1]=first[0];--n;AdjustHeap(first,0,n,value);}}
inline void Range(Entry* first,Entry* last,unsigned int ideal){while(last-first>32){if(!ideal){HeapSort(first,last);return;}Entry* low;Entry* high;Partition(first,last,low,high);ideal=ideal/2+ideal/4;if(low-first<last-high){Range(first,low,ideal);first=high;}else{Range(high,last,ideal);last=low;}}if(last-first>1)Insertion(first,last);}
inline bool Build(Entry* entries,unsigned int count){if(!entries||count>1000000)return false;Range(entries,entries+count,count);return true;}
}
