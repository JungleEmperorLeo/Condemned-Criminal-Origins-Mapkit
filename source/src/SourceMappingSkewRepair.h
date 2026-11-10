#pragma once
namespace SourceMappingSkewRepair {
inline float Root(double value){float result;
 __asm {fld value}
 __asm {fsqrt}
 __asm {fstp result}
 return result;
}
// Original setter RVA5F750: sign-dependent quadratic normal displacement.
inline void Repair(const WorldText::Vec3& n,WorldText::Mapping& m){
 const WorldText::Vec3 p=m.p,q=m.q;
 float dot=(float)((double(q.z)*p.z+double(q.y)*p.y)+double(q.x)*p.x);
 if(!(dot>0.001f||dot< -0.001f))return;
#ifdef FEAR_SKEW_INPUT_TRACE_EXPERIMENT
 char trace[MAX_PATH];if(GetEnvironmentVariableA("FEAR_SKEW_INPUT_OUTPUT",trace,MAX_PATH)){
  HANDLE file=CreateFileA(trace,FILE_APPEND_DATA,FILE_SHARE_READ,0,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,0);
  if(file!=INVALID_HANDLE_VALUE){DWORD done;WriteFile(file,&n,12,&done,0);WriteFile(file,&m.origin,36,&done,0);CloseHandle(file);}
 }
#endif
 float a,b;
 if(dot<0){
  a=(float)((double(n.x)*n.x+double(n.y)*n.y)+double(n.z)*n.z);
  b=(float)(((((double(n.z)*q.z+double(n.y)*q.y)+double(n.y)*p.y)+double(n.z)*p.z)+double(n.x)*p.x)+double(q.x)*n.x);
 }else{
  a=(float)((-double(n.x)*n.x-double(n.y)*n.y)-double(n.z)*n.z);
  b=(float)(((((double(n.x)*p.x-double(q.x)*n.x)+double(n.y)*p.y)-double(n.y)*q.y)+double(n.z)*p.z)-double(n.z)*q.z);
 }
 float root=Root(double(b)*b-(double(dot)*a)*4);
 float amount=(float)((double(root)-b)/(double(a)+a));
 WorldText::Vec3 shift={(float)(double(amount)*n.x),(float)(double(amount)*n.y),(float)(double(amount)*n.z)};
 if(dot<0){m.p.x=shift.x+p.x;m.p.y=shift.y+p.y;m.p.z=shift.z+p.z;}
 else {m.p.x=p.x-shift.x;m.p.y=p.y-shift.y;m.p.z=p.z-shift.z;}
 m.q.x=shift.x+q.x;m.q.y=shift.y+q.y;m.q.z=shift.z+q.z;
}
inline void Prepare(WorldText::Scene& scene){
 for(unsigned int g=0;g<scene.geometryCount;g++)for(unsigned int p=0;p<scene.geometry[g].polygons;p++){
  WorldText::Polygon& face=scene.geometry[g].faces[p];
  for(unsigned int i=0;i<face.mappingCount;i++)Repair(face.normal,face.mappings[i]);
 }
}
}
