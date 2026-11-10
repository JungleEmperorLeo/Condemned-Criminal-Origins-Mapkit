#pragma once
// Recovered from WorldPacker RVA 0x2AF40. Input order breaks equal-score ties.
namespace StreamingSplit {
struct Bounds {float low[3],high[3];};
struct Event {float position;bool enter;};
struct Result {unsigned int axis;float coordinate;unsigned int count[3];unsigned int* ids[3];};
inline float Round(double v){volatile float r=(float)v;return r;}
inline bool Choose(const Bounds* bounds,unsigned int n,Result& result){
 result.axis=3;result.coordinate=0;for(unsigned int k=0;k<3;k++)result.count[k]=0;
 if(!n||n>100000)return false;
 Event* events=(Event*)VirtualAlloc(0,n*2*sizeof(Event),MEM_COMMIT|MEM_RESERVE,4);if(!events)return false;
 unsigned int best=0xffffffff;
 if(n>=4)for(unsigned int axis=0;axis<3;axis++){
  for(unsigned int i=0;i<n;i++){events[2*i].position=Round((double)bounds[i].low[axis]-(double)0.1f);events[2*i].enter=true;events[2*i+1].position=Round((double)bounds[i].high[axis]+(double)0.1f);events[2*i+1].enter=false;}
  // Stable insertion order matches equal-coordinate comparisons in the original.
  for(unsigned int i=1;i<2*n;i++){Event e=events[i];unsigned int j=i;while(j&&e.position<events[j-1].position){events[j]=events[j-1];j--;}events[j]=e;}
  unsigned int left=0,crossing=0;
  for(unsigned int i=0;i<2*n-1;i++){
   if(events[i].enter)crossing++;else{crossing--;left++;}
   unsigned int right=n-left-crossing;if(!left||!right)continue;
   unsigned int difference=right>left?right-left:left-right;
   float ratio=Round((double)(float)difference/(float)(right+left));
   unsigned int score=6*crossing-(int)((double)ratio*(double)-100.0f);
   if(score<best){best=score;result.axis=axis;result.coordinate=Round(((double)events[i].position+events[i+1].position)/2.0);}
  }
 }
 VirtualFree(events,0,MEM_RELEASE);
 for(unsigned int i=0;i<n;i++){
  unsigned int group=0;if(result.axis!=3){if((double)bounds[i].high[result.axis]+(double)0.1f<result.coordinate)group=1;else if((double)bounds[i].low[result.axis]-(double)0.1f>result.coordinate)group=2;}
  result.ids[group][result.count[group]++]=i;
 }
 if(result.axis!=3&&(!result.count[1]||!result.count[2])){
  result.axis=3;result.coordinate=0;for(unsigned int k=1;k<3;k++){for(unsigned int j=0;j<result.count[k];j++)result.ids[0][result.count[0]++]=result.ids[k][j];result.count[k]=0;}
 }
 return true;
}
}
