#pragma once
namespace RenderSectionRules {
// Runtime section layout is 40 bytes on x86. It is not serialized directly.
// Keys are material text plus pass category. Material/shader IDs, counts and
// original indices do not enter the comparator or matching scan.
enum Category {ShadowLow=0,ShadowMedium=1,ShadowHigh=2,Visible=3};
// Original0x460D0 retains named ordinary records and removes pure shadow-maker
// inputs after their shadow generation. Surface+0x98 is material text.
inline bool RetainTriangle(unsigned int flags,const char* material){return ((flags&0x22)!=0||(flags&0x40)==0)&&material&&material[0];}
struct Record {unsigned char bytes[40];};
// Reproduce 0x3F5F0 with an already resolved cached Shader* represented as U32
// in this x86 record. Material-resource loading is a separate caller concern.
inline void Initialize(Record& r,const char* material,unsigned int cachedShaderPointer,int category,unsigned int triangles){memset(r.bytes,0,40);memcpy(r.bytes,&material,4);memcpy(r.bytes+4,&cachedShaderPointer,4);memset(r.bytes+8,0xff,12);memcpy(r.bytes+20,&category,4);memcpy(r.bytes+24,&triangles,4);}
// FearBrushFlags::Modify maps Low/Medium/High to flag bits 16..17. Never and
// ClipLight=False clear 0x40. Category 3 belongs to ordinary visible geometry.
inline bool ShadowCategory(unsigned int surfaceFlags,int& category){if(!(surfaceFlags&0x40))return false;category=(int)((surfaceFlags>>16)&3);return category<3;}
inline const char* Material(const Record& v){const char* p;memcpy(&p,v.bytes,4);return p;}
inline int Kind(const Record& v){int n;memcpy(&n,v.bytes+20,4);return n;}
inline unsigned char Lower(unsigned char v){return v>='A'&&v<='Z'?(unsigned char)(v+32):v;}
// C-locale ASCII material names. No slash normalization is performed here.
inline int TextCompare(const char* a,const char* b){if(!a||!b)return a?1:b?-1:0;for(;;++a,++b){unsigned char x=Lower((unsigned char)*a),y=Lower((unsigned char)*b);if(x!=y)return x<y?-1:1;if(!x)return 0;}}
inline bool Match(const Record& a,const Record& b){return Kind(a)==Kind(b)&&TextCompare(Material(a),Material(b))==0;}
inline unsigned int Find(const Record* records,unsigned int count,const Record& query){for(unsigned int i=0;i<count;i++)if(Match(records[i],query))return i;return 0xffffffff;}
inline bool Count(Record* records,unsigned int& count,unsigned int capacity,const Record& query,unsigned int add,unsigned int& index){index=Find(records,count,query);if(index==0xffffffff){if(count==capacity)return false;index=count++;memcpy(records[index].bytes,query.bytes,40);memcpy(records[index].bytes+24,&add,4);}else{unsigned int n;memcpy(&n,records[index].bytes+24,4);n+=add;memcpy(records[index].bytes+24,&n,4);}return true;}

inline bool Less(const Record& a,const Record& b){int cmp=TextCompare(Material(a),Material(b));return cmp?cmp<0:Kind(a)<Kind(b);}
inline void Copy(Record& a,const Record& b){memcpy(a.bytes,b.bytes,40);}
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
inline bool Build(Record* records,unsigned int count){return records&&count<=100000&&Range(records,records+count,count);}
}
