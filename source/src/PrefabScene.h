#pragma once
// Independent project/source loading and instantiation into an owned model.
// Matrices follow Jupiter PrefabRef.cpp and gr_SetupMatrixEuler.
namespace PrefabScene {
inline bool Full(const char* path,char* result){DWORD n=GetFullPathNameA(path,MAX_PATH,result,0);return n&&n<MAX_PATH;}
inline void Parent(char* path){char* last=0;for(char* p=path;*p;p++)if(*p=='\\'||*p=='/')last=p;if(last)last[1]=0;else path[0]=0;}
inline bool Join(const char* base,const char* relative,char* result){
 char temp[MAX_PATH];unsigned int a=lstrlenA(base),b=lstrlenA(relative);
 if((relative[0]&&relative[1]==':')||relative[0]=='\\'||relative[0]=='/')return Full(relative,result);
 if(a+b+2>=MAX_PATH)return false;lstrcpyA(temp,base);if(a&&temp[a-1]!='\\'&&temp[a-1]!='/')temp[a++]='\\';lstrcpyA(temp+a,relative);return Full(temp,result);
}
struct Project {
 char file[MAX_PATH],resources[MAX_PATH];const char* error;
 Project():error(0){file[0]=resources[0]=0;}
 // Production callers select the project explicitly. The old Locate adapter
 // remains for research harnesses, whose environment is intentionally variable.
 bool LoadPath(const char* path){
  error=0;file[0]=resources[0]=0;
  if(!path||!path[0]||!Full(path,file)){error="Invalid explicit project path";return false;}
  return LoadResources();
 }
 bool LoadResources(){
  char relative[MAX_PATH],base[MAX_PATH];resources[0]=0;
  DWORD n=GetPrivateProfileStringA("Project Directories","ResourceRoot","",relative,MAX_PATH,file);
  if(!n||n>=MAX_PATH-1){error="Missing or oversized ResourceRoot in project";return false;}
  lstrcpyA(base,file);Parent(base);if(!Join(base,relative,resources)){error="Resource root path exceeds MAX_PATH";return false;}return true;
 }
 // Project entries such as FEAR.proj00 "GameDatabase=GameDatabase.dll" and
 // "ToolsRoot=..\Tools\" are relative to the project file, like ResourceRoot.
 bool Resolve(const char* section,const char* key,char* result) const {
  char value[MAX_PATH],base[MAX_PATH];DWORD n=GetPrivateProfileStringA(section,key,"",value,MAX_PATH,file);
  if(!n||n>=MAX_PATH-1)return false;lstrcpyA(base,file);Parent(base);return Join(base,value,result);
 }
 bool Locate(const char* input){
  error=0;file[0]=resources[0]=0;
  char selected[MAX_PATH];DWORD overrideSize=GetEnvironmentVariableA("FEAR_READER_PROJECT",selected,MAX_PATH);
  if(overrideSize>=MAX_PATH){error="Project override path exceeds MAX_PATH";return false;}
  if(overrideSize){if(!Full(selected,file)){error="Invalid project override path";return false;}}
  else {
   char dir[MAX_PATH];if(!Full(input,dir)){error="Invalid source path";return false;}Parent(dir);
   while(dir[0]){
    char pattern[MAX_PATH];if(!Join(dir,"*.proj00",pattern)){error="Project search path exceeds MAX_PATH";return false;}
    FindDataA found;HANDLE search=FindFirstFileA(pattern,&found);unsigned int count=0;
    if(search!=INVALID_HANDLE_VALUE){do {if(!(found.fields[0]&16)){count++;if(!Join(dir,found.name,file)){FindClose(search);error="Project path exceeds MAX_PATH";return false;}}}while(FindNextFileA(search,&found));FindClose(search);}
    if(count>1){error="Multiple projects in nearest directory; set FEAR_READER_PROJECT";return false;}if(count)break;
    unsigned int length=lstrlenA(dir);if(length<=3)break;dir[length-1]=0;Parent(dir);
   }
   if(!file[0]){error="No .proj00 project found above source world";return false;}
  }
  return LoadResources();
 }
};
struct Matrix {
 float m[3][4];
 Matrix(){for(unsigned int i=0;i<3;i++)for(unsigned int j=0;j<4;j++)m[i][j]=i==j?1.0f:0.0f;}
 static float Sin(float x){float result;__asm fld x
  __asm fsin
  __asm fstp result
  return result;}
 static float Cos(float x){float result;__asm fld x
  __asm fcos
  __asm fstp result
  return result;}
 static bool Euler(const WorldText::Vec3& v,Matrix& r){
  // x87 trig has a finite range; refuse huge source angles explicitly.
  if(v.x>1000000||v.x< -1000000||v.y>1000000||v.y< -1000000||v.z>1000000||v.z< -1000000)return false;
  float yc=Cos(v.y),ys=Sin(v.y),pc=Cos(v.x),ps=Sin(v.x),rc=Cos(v.z),rs=Sin(v.z);
#ifdef FEAR_EULER_PARTIAL_PRODUCT_EXPERIMENT
  volatile float rsps=rs*ps,rcps=rc*ps;
  r.m[0][0]=(float)(double(rc)*yc+double(rsps)*ys);r.m[0][1]=(float)(-double(rs)*yc+double(rcps)*ys);r.m[0][2]=(float)(double(pc)*ys);
  r.m[1][0]=(float)(double(rs)*pc);r.m[1][1]=(float)(double(rc)*pc);r.m[1][2]=-ps;
  r.m[2][0]=(float)(-double(rc)*ys+double(rsps)*yc);r.m[2][1]=(float)(double(rs)*ys+double(rcps)*yc);r.m[2][2]=(float)(double(pc)*yc);return true;
#elif defined(FEAR_EULER_DOUBLE_EXPERIMENT)
  r.m[0][0]=(float)(double(rc)*yc+double(rs)*ps*ys);r.m[0][1]=(float)(-double(rs)*yc+double(rc)*ps*ys);r.m[0][2]=(float)(double(pc)*ys);
  r.m[1][0]=(float)(double(rs)*pc);r.m[1][1]=(float)(double(rc)*pc);r.m[1][2]=-ps;
  r.m[2][0]=(float)(-double(rc)*ys+double(rs)*ps*yc);r.m[2][1]=(float)(double(rs)*ys+double(rc)*ps*yc);r.m[2][2]=(float)(double(pc)*yc);return true;
#else
  r.m[0][0]=rc*yc+rs*ps*ys;r.m[0][1]=-rs*yc+rc*ps*ys;r.m[0][2]=pc*ys;
  r.m[1][0]=rs*pc;r.m[1][1]=rc*pc;r.m[1][2]=-ps;
  r.m[2][0]=-rc*ys+rs*ps*yc;r.m[2][1]=rs*ys+rc*ps*yc;r.m[2][2]=pc*yc;return true;
#endif
 }
 Matrix Times(const Matrix& b)const{Matrix r;for(unsigned int i=0;i<3;i++){
#ifdef FEAR_MATRIX_COMPOSE_DOUBLE_EXPERIMENT
 for(unsigned int j=0;j<3;j++)r.m[i][j]=(float)((double(m[i][0])*b.m[0][j]+double(m[i][1])*b.m[1][j])+double(m[i][2])*b.m[2][j]);r.m[i][3]=(float)(((double(m[i][0])*b.m[0][3]+double(m[i][1])*b.m[1][3])+double(m[i][2])*b.m[2][3])+m[i][3]);
#else
 for(unsigned int j=0;j<3;j++)r.m[i][j]=m[i][0]*b.m[0][j]+m[i][1]*b.m[1][j]+m[i][2]*b.m[2][j];r.m[i][3]=m[i][0]*b.m[0][3]+m[i][1]*b.m[1][3]+m[i][2]*b.m[2][3]+m[i][3];
#endif
 }return r;}
 WorldText::Vec3 Apply(const WorldText::Vec3& v)const{
#ifdef FEAR_MATRIX_APPLY_X87_EXPERIMENT
#ifdef FEAR_MATRIX_APPLY_X87_64_EXPERIMENT
 unsigned short savedControl,wideControl;__asm fnstcw savedControl
 wideControl=(savedControl&0xfcffu)|0x0300u;__asm fldcw wideControl
#endif
 WorldText::Vec3 result;const float* values=&v.x;for(unsigned int i=0;i<3;i++){const float* row=m[i];float component;
 __asm mov ecx,row
 __asm mov edx,values
 __asm fld dword ptr [ecx]
 __asm fmul dword ptr [edx]
 __asm fld dword ptr [ecx+4]
 __asm fmul dword ptr [edx+4]
 __asm faddp st(1),st(0)
 __asm fld dword ptr [ecx+8]
 __asm fmul dword ptr [edx+8]
 __asm faddp st(1),st(0)
 __asm fadd dword ptr [ecx+12]
 __asm fstp component
 (&result.x)[i]=component;}
#ifdef FEAR_MATRIX_APPLY_X87_64_EXPERIMENT
 __asm fldcw savedControl
#endif
 return result;
#elif defined(FEAR_TRANSFORM_SEPARATE_TRANSLATION_EXPERIMENT)
 WorldText::Vec3 result;for(unsigned int i=0;i<3;i++){volatile float rotated=(float)((double(m[i][0])*v.x+double(m[i][1])*v.y)+double(m[i][2])*v.z);(&result.x)[i]=rotated+m[i][3];}return result;
#elif defined(FEAR_GEOMETRY_DOUBLE_EXPERIMENT)
 WorldText::Vec3 result;for(unsigned int i=0;i<3;i++)(&result.x)[i]=(float)(((double(m[i][0])*v.x+double(m[i][1])*v.y)+double(m[i][2])*v.z)+m[i][3]);return result;
#else
WorldText::Vec3 r={m[0][0]*v.x+m[0][1]*v.y+m[0][2]*v.z+m[0][3],m[1][0]*v.x+m[1][1]*v.y+m[1][2]*v.z+m[1][3],m[2][0]*v.x+m[2][1]*v.y+m[2][2]*v.z+m[2][3]};return r;
#endif
 }
 WorldText::Vec3 Direction(const WorldText::Vec3& v)const{WorldText::Vec3 r={m[0][0]*v.x+m[0][1]*v.y+m[0][2]*v.z,m[1][0]*v.x+m[1][1]*v.y+m[1][2]*v.z,m[2][0]*v.x+m[2][1]*v.y+m[2][2]*v.z};return r;}
 WorldText::Vec3 TextureDirection(const WorldText::Vec3& v)const{
#ifdef FEAR_TEXTURE_TRANSFORM_DOUBLE_EXPERIMENT
  WorldText::Vec3 r;for(unsigned int i=0;i<3;i++)(&r.x)[i]=(float)((double(m[i][0])*v.x+double(m[i][1])*v.y)+double(m[i][2])*v.z);return r;
#else
  return Direction(v);
#endif
 }
 WorldText::Vec3 TextureOrigin(const WorldText::Vec3& v)const{
#ifdef FEAR_TRANSFORM_SEPARATE_TRANSLATION_EXPERIMENT
  return Apply(v);
#elif defined(FEAR_TEXTURE_ORIGIN_DOUBLE_EXPERIMENT)
  WorldText::Vec3 r;for(unsigned int i=0;i<3;i++)(&r.x)[i]=(float)(((double(m[i][0])*v.x+double(m[i][1])*v.y)+double(m[i][2])*v.z)+m[i][3]);return r;
#else
  return Apply(v);
#endif
 }
};
#include "InaccurateGeometry.h"
#include "SourceMissingTextureMappings.h"
#include "SourceMappingFiniteRepair.h"
#include "SourceMappingSkewRepair.h"
struct Source {
 char* raw;LtcDecode decoder;WorldText::Scene scene;
 Source():raw(0){}~Source(){if(raw)VirtualFree(raw,0,MEM_RELEASE);}
 bool Load(const char* path,bool compilerPreparation=false){
  bool repairEnabled=false;
#ifdef FEAR_SOURCE_GEOMETRY_REPAIR_DEFAULT
  repairEnabled=FEAR_SOURCE_GEOMETRY_REPAIR_DEFAULT!=0;
#endif
  char repair[4]={};if(GetEnvironmentVariableA("FEAR_SOURCE_GEOMETRY_REPAIR",repair,4))repairEnabled=repair[0]=='1';
  return LoadExplicit(path,compilerPreparation,repairEnabled);
 }
 bool LoadExplicit(const char* path,bool compilerPreparation=false,bool repairEnabled=false){
  if(!path||!path[0])return scene.doc.Fail("Invalid explicit source path",0);
  // A Source owns a single parse. Expansion creates a fresh Source per prefab;
  // refusing reuse prevents stale Document/decoder storage from escaping.
  if(raw)return scene.doc.Fail("Source already loaded",0);
  HANDLE file=CreateFileA(path,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);
  // Shipped SDK prefabs can retain text references while only compressed files exist.
  if(file==INVALID_HANDLE_VALUE){
   char alternative[MAX_PATH];unsigned int length=lstrlenA(path);
   if(length<MAX_PATH&&length>=9&&FearBrushFlags::Equal(path+length-9,".World00a")){
    lstrcpyA(alternative,path);alternative[length-1]='c';file=CreateFileA(alternative,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);
   }
  }
  if(file==INVALID_HANDLE_VALUE)return scene.doc.Fail("Could not open prefab",0);
  DWORD size=GetFileSize(file,0);if(size==0xffffffffu||size>256u*1024u*1024u){CloseHandle(file);return scene.doc.Fail("Prefab exceeds 256 MiB",0);}
  raw=(char*)VirtualAlloc(0,size+1,MEM_COMMIT|MEM_RESERVE,4);if(!raw){CloseHandle(file);return scene.doc.Fail("Could not allocate prefab source",0);}
  DWORD read;BOOL ok=ReadFile(file,raw,size,&read,0);CloseHandle(file);if(!ok||read!=size)return scene.doc.Fail("Could not read complete prefab",0);
  raw[size]=0;bool compressed=size>=4&&!raw[0]&&!raw[1]&&!raw[2]&&!raw[3];
  if(compressed&&!decoder.Decode(raw,size))return scene.doc.Fail(decoder.error,decoder.errorOffset);
  if(!scene.Load(compressed?decoder.data:raw,compressed?decoder.size:size))return false;
#ifdef FEAR_MAPPING_FINITE_REPAIR_EXPERIMENT
  SourceMappingFiniteRepair::Prepare(scene);
#endif
#ifdef FEAR_MISSING_TEXTURE_MAPPINGS_EXPERIMENT
  if(!SourceMissingTextureMappings::Prepare(scene))return scene.doc.Fail("Could not prepare missing texture mappings",0);
#endif
  if(repairEnabled||compilerPreparation)InaccurateGeometry::Prepare(scene);
  return true;
 }
};
#include "UnifiedScene.h"
}
