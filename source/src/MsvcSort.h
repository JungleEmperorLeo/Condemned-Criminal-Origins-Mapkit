#pragma once
// MSVC 7.1 std::sort (introsort: median-of-nine pivot, fat partition, heap
// fallback, insertion below 32), generic copy of PhysicalNormalOrder. Equal
// elements land where this algorithm puts them; it is not a stable sort.
namespace MsvcSort {
template<class T,class L> inline void Swap(T& a,T& b){T t=a;a=b;b=t;}
template<class T,class L> inline void Median(T* a,T* b,T* c,L less){if(less(*b,*a))Swap<T,L>(*a,*b);if(less(*c,*b)){Swap<T,L>(*b,*c);if(less(*b,*a))Swap<T,L>(*a,*b);}}
template<class T,class L> inline void Pivot(T* first,T* mid,T* last,L less){unsigned int n=(unsigned int)(last-first);if(n>40){unsigned int step=(n+1)/8;Median(first,first+step,first+2*step,less);Median(mid-step,mid,mid+step,less);Median(last-2*step,last-step,last,less);Median(first+step,mid,last-step,less);}else Median(first,mid,last,less);}
template<class T,class L> inline void Partition(T* first,T* last,T*& low,T*& high,L less){
 T* mid=first+(last-first)/2;Pivot(first,mid,last-1,less);low=mid;high=mid+1;
 while(first<low&&!less(*(low-1),*low)&&!less(*low,*(low-1)))--low;
 while(high<last&&!less(*high,*low)&&!less(*low,*high))++high;
 T* left=low;T* right=high;
 for(;;){
  for(;right<last;++right){if(less(*low,*right))continue;if(less(*right,*low))break;if(high!=right)Swap<T,L>(*high,*right);++high;}
  for(;first<left;){T* p=left-1;if(less(*p,*low)){--left;continue;}if(less(*low,*p))break;--low;--left;Swap<T,L>(*low,*left);}
  if(left==first&&right==last)return;
  if(left==first){if(high!=right)Swap<T,L>(*low,*high);Swap<T,L>(*low,*right);++low;++high;++right;}
  else if(right==last){--left;--low;--high;if(left!=low)Swap<T,L>(*left,*low);Swap<T,L>(*low,*high);}
  else {--left;Swap<T,L>(*right,*left);++right;}
 }
}
template<class T,class L> inline void Insertion(T* first,T* last,L less){for(T* p=first+1;p<last;++p){T value=*p;T* hole=p;while(hole>first&&less(value,*(hole-1))){*hole=*(hole-1);--hole;}*hole=value;}}
template<class T,class L> inline void PushHeap(T* first,unsigned int hole,unsigned int top,const T& value,L less){while(hole>top){unsigned int parent=(hole-1)/2;if(!less(first[parent],value))break;first[hole]=first[parent];hole=parent;}first[hole]=value;}
template<class T,class L> inline void AdjustHeap(T* first,unsigned int hole,unsigned int count,const T& value,L less){unsigned int top=hole,right=hole*2+2;while(right<count){unsigned int child=less(first[right],first[right-1])?right-1:right;first[hole]=first[child];hole=child;right=hole*2+2;}if(right==count){first[hole]=first[right-1];hole=right-1;}PushHeap(first,hole,top,value,less);}
template<class T,class L> inline void HeapSort(T* first,T* last,L less){unsigned int n=(unsigned int)(last-first);for(unsigned int parent=n/2;parent;){--parent;T value=first[parent];AdjustHeap(first,parent,n,value,less);}while(n>1){T value=first[n-1];first[n-1]=first[0];--n;AdjustHeap(first,0,n,value,less);}}
template<class T,class L> inline void Range(T* first,T* last,unsigned int ideal,L less){while(last-first>32){if(!ideal){HeapSort(first,last,less);return;}T* low;T* high;Partition(first,last,low,high,less);ideal=ideal/2+ideal/4;if(low-first<last-high){Range(first,low,ideal,less);first=high;}else{Range(high,last,ideal,less);last=low;}}if(last-first>1)Insertion(first,last,less);}
template<class T,class L> inline void Sort(T* first,unsigned int count,L less){if(count>1)Range(first,first+count,count,less);}
}
