#pragma once
#include "Sha256.h"
// In-process form of mopp-backend (mopp-helper.cpp + backend.py), profile
// radius003-axis005-small: pinned NifMopp.dll (NifSkope v2.0.dev6, SHA-256 below),
// the same three signature-checked in-memory patches, then the backend's checks:
// mesh framing, CodeInfo, structural bytecode decode with forward branch targets,
// leaf-key traversal and the pinned NifMopp triangle-validity policy (RVA 0x29440).
// External generator, not a recovered Havok/FEAR algorithm. Patches persist for
// the process; a second Load of the same module reuses them.
extern "C" {
__declspec(dllimport) HMODULE WINAPI LoadLibraryA(const char*);
__declspec(dllimport) void* WINAPI GetProcAddress(HMODULE,const char*);
__declspec(dllimport) BOOL WINAPI VirtualProtect(void*,unsigned int,DWORD,DWORD*);
__declspec(dllimport) BOOL WINAPI FreeLibrary(HMODULE);
}
#pragma comment(linker,"/alternatename:__imp__FreeLibrary@4=__imp__FreeLibrary")
#pragma comment(linker,"/alternatename:__imp__VirtualProtect@16=__imp__VirtualProtect")
#pragma comment(linker,"/alternatename:__imp__LoadLibraryA@4=__imp__LoadLibraryA")
#pragma comment(linker,"/alternatename:__imp__GetProcAddress@8=__imp__GetProcAddress")
namespace NifMoppBackend {
static const char* const DllSha256="0f94c29d27b86ca924d4f49e69dc3976fa6f5599a59d658954676f356efe97a8";
struct Triangle {unsigned short a,b,c;};
typedef int (WINAPI *GenerateFn)(int,const WorldText::Vec3*,int,const Triangle*);
typedef int (WINAPI *RetrieveFn)(int,char*);typedef int (WINAPI *ScaleFn)(float*);typedef int (WINAPI *OriginFn)(WorldText::Vec3*);
static float axisTolerance=.05f;
namespace detail {
inline float F(double v){volatile float r=(float)v;return r;}
inline bool Finite(float v){return WorldText::Finite(v);}
// triangle_validity.py rejection_reason; every listed store is binary32.
inline bool Rejected(const WorldText::Vec3& a,const WorldText::Vec3& b,const WorldText::Vec3& c,float cutoff,bool& ok){
 ok=true;float ab[3]={F((double)a.x-b.x),F((double)a.y-b.y),F((double)a.z-b.z)},ac[3]={F((double)a.x-c.x),F((double)a.y-c.y),F((double)a.z-c.z)};
 float ba[3]={F((double)b.x-a.x),F((double)b.y-a.y),F((double)b.z-a.z)},bc[3]={F((double)b.x-c.x),F((double)b.y-c.y),F((double)b.z-c.z)};
 float n0[3]={F((double)ab[1]*ac[2]-(double)ab[2]*ac[1]),F((double)ab[2]*ac[0]-(double)ab[0]*ac[2]),F((double)ab[0]*ac[1]-(double)ab[1]*ac[0])};
 float n1[3]={F((double)ba[1]*bc[2]-(double)ba[2]*bc[1]),F((double)ba[2]*bc[0]-(double)ba[0]*bc[2]),F((double)ba[0]*bc[1]-(double)ba[1]*bc[0])};
 for(unsigned int i=0;i<3;i++)if(!Finite(ab[i])||!Finite(ac[i])||!Finite(ba[i])||!Finite(bc[i])||!Finite(n0[i])||!Finite(n1[i])){ok=false;return false;}
 float area0=F(((double)n0[1]*n0[1]+(double)n0[0]*n0[0])+(double)n0[2]*n0[2]);if(!Finite(area0)){ok=false;return false;}if(area0<cutoff)return true;
 float area1=F(((double)n1[1]*n1[1]+(double)n1[0]*n1[0])+(double)n1[2]*n1[2]);if(!Finite(area1)){ok=false;return false;}if(area1<cutoff)return true;
 float cb[3]={F((double)c.x-b.x),F((double)c.y-b.y),F((double)c.z-b.z)};
 float dot=F(((double)cb[0]*ab[0]+(double)cb[1]*ab[1])+(double)cb[2]*ab[2]);
 float lab=F(((double)ab[1]*ab[1]+(double)ab[0]*ab[0])+(double)ab[2]*ab[2]);
 float lcb=F((double)cb[2]*cb[2]+((double)cb[1]*cb[1]+(double)cb[0]*cb[0]));
 float product=F((double)lab*lcb),square=F((double)dot*dot),gram=F((double)product-square);
 if(!Finite(cb[0])||!Finite(cb[1])||!Finite(cb[2])||!Finite(dot)||!Finite(lab)||!Finite(lcb)||!Finite(product)||!Finite(square)||!Finite(gram)){ok=false;return false;}
 return gram==0;
}
inline unsigned int Big(const unsigned char* p,unsigned int n){unsigned int v=0;for(unsigned int i=0;i<n;i++)v=v<<8|p[i];return v;}
}
class Library {
 HMODULE module;GenerateFn generate;RetrieveFn retrieve;ScaleFn scale;OriginFn origin;
 bool Fail(const char* why){error=why;return false;}
public:
 const char* error;
 Library():module(0),generate(0),retrieve(0),scale(0),origin(0),error(0){}
 bool Ready()const{return generate!=0;}
 // NifMopp keeps Havok thread memory for the calling thread; its static cleanup
 // must run on that thread (winpacker calls Process on a worker thread, and a
 // process-exit detach on the main thread faults in the Havok pool). Unload on
 // the generating thread; the next Load maps and patches a fresh image.
 void Unload(){if(module)FreeLibrary(module);module=0;generate=0;retrieve=0;scale=0;origin=0;}
 ~Library(){Unload();}
 bool Load(const char* path){
  error=0;if(Ready())return true;
  HANDLE f=CreateFileA(path,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);if(f==INVALID_HANDLE_VALUE)return Fail("NifMopp.dll is missing");
  unsigned int size=GetFileSize(f,0);unsigned char* bytes=size&&size<=16u*1024u*1024u?(unsigned char*)VirtualAlloc(0,size,MEM_COMMIT|MEM_RESERVE,4):0;DWORD done=0;bool read=bytes&&ReadFile(f,bytes,size,&done,0)&&done==size;CloseHandle(f);
  bool pinned=read&&Sha256::Matches(bytes,size,DllSha256);if(bytes)VirtualFree(bytes,0,MEM_RELEASE);if(!pinned)return Fail("Unsupported NifMopp.dll (SHA-256 differs from the pinned build)");
  module=LoadLibraryA(path);if(!module)return Fail("Could not load NifMopp.dll");unsigned char* base=(unsigned char*)module;DWORD old,unused;
  unsigned int* radius=(unsigned int*)(base+0xbd94c);if(*radius!=0x3c23d70au&&*radius!=0x3cf5c28fu)return Fail("NifMopp radius signature mismatch");
  if(*radius!=0x3cf5c28fu){if(!VirtualProtect(radius,4,4,&old))return Fail("NifMopp radius patch failed");*radius=0x3cf5c28fu;VirtualProtect(radius,4,old,&unused);}
  unsigned char* instruction=base+0x1283;unsigned int* operand=(unsigned int*)(instruction+2);
  if(instruction[0]!=0xd9||instruction[1]!=0x05||(*operand!=(unsigned int)(base+0xbd958)&&*operand!=(unsigned int)&axisTolerance))return Fail("NifMopp axis tolerance signature mismatch");
  if(*operand!=(unsigned int)&axisTolerance){if(!VirtualProtect(operand,4,0x40,&old))return Fail("NifMopp axis tolerance patch failed");*operand=(unsigned int)&axisTolerance;VirtualProtect(operand,4,old,&unused);}
  unsigned int* cutoff=(unsigned int*)(base+0xc289c);if(*cutoff!=0x33d6bf95u&&*cutoff!=0x2edbe6ffu)return Fail("NifMopp small-triangle cutoff signature mismatch");
  const unsigned int refs[3]={0x24743,0x247c3,0x2906d};for(unsigned int i=0;i<3;i++){unsigned char* at=base+refs[i];if(at[0]!=0xd9||at[1]!=0x05||*(unsigned int*)(at+2)!=(unsigned int)cutoff)return Fail("NifMopp small-triangle reference mismatch");}
  if(*cutoff!=0x2edbe6ffu){if(!VirtualProtect(cutoff,4,4,&old))return Fail("NifMopp cutoff patch failed");*cutoff=0x2edbe6ffu;VirtualProtect(cutoff,4,old,&unused);}
  generate=(GenerateFn)GetProcAddress(module,"GenerateMoppCode");retrieve=(RetrieveFn)GetProcAddress(module,"RetrieveMoppCode");scale=(ScaleFn)GetProcAddress(module,"RetrieveMoppScale");origin=(OriginFn)GetProcAddress(module,"RetrieveMoppOrigin");
  if(!generate||!retrieve||!scale||!origin){generate=0;return Fail("NifMopp exports are missing");}return true;
 }
 // mesh: one-mesh physics prefix (backend.mesh_counts layout). Output: 16-byte
 // CodeInfo (origin xyz, scale), u32 length, code; owned by memory.
 bool Generate(const unsigned char* mesh,unsigned int bytes,PrefabScene::Arena& memory,const unsigned char*& out,unsigned int& outBytes){
  error=0;out=0;outBytes=0;if(!Ready())return Fail("NifMopp backend is not loaded");
  if(!mesh||bytes<108||bytes>16u*1024u*1024u)return Fail("Mesh prefix size outside supported limits");
  unsigned int count=*(const unsigned int*)mesh,kind=*(const unsigned int*)(mesh+32),points=*(const unsigned int*)(mesh+100),indices=*(const unsigned int*)(mesh+104);
  if(count!=1||kind!=1||!points||points>=65536||!indices||indices>3000000||indices%3||bytes!=108+points*12+indices*2)return Fail("Expected one mesh with uint16 triangle indices");
  const float* header=(const float*)(mesh+4);for(unsigned int i=0;i<7;i++)if(!detail::Finite(header[i]))return Fail("Non-finite mesh input");
  const float* parameters=(const float*)(mesh+36);for(unsigned int i=0;i<16;i++)if(!detail::Finite(parameters[i]))return Fail("Non-finite mesh input");
  const WorldText::Vec3* vertices=(const WorldText::Vec3*)(mesh+108);const Triangle* triangles=(const Triangle*)(mesh+108+points*12);unsigned int triangleCount=indices/3;
  for(unsigned int i=0;i<points;i++)if(!WorldText::FiniteVector(vertices[i]))return Fail("Non-finite mesh input");
  for(unsigned int i=0;i<triangleCount;i++)if(triangles[i].a>=points||triangles[i].b>=points||triangles[i].c>=points)return Fail("Triangle index outside mesh");
  int size=generate((int)points,vertices,(int)triangleCount,triangles);if(size<=0||size>16*1024*1024)return Fail("NifMopp generation failed");
  unsigned char* result=(unsigned char*)memory.Allocate((unsigned int)size+20);if(!result)return Fail("MOPP output allocation failed");
  if(!retrieve(size,(char*)result+20)||!scale((float*)(result+12))||!origin((WorldText::Vec3*)result))return Fail("NifMopp retrieval failed");*(unsigned int*)(result+16)=(unsigned int)size;
  const float* info=(const float*)result;for(unsigned int i=0;i<4;i++)if(!detail::Finite(info[i]))return Fail("Invalid generated CodeInfo");if(!(info[3]>0))return Fail("Invalid generated CodeInfo");
  if(!Check(result,(unsigned int)size+20,mesh,points,triangleCount,memory))return false;
  out=result;outBytes=(unsigned int)size+20;return true;
 }
private:
 struct Command {unsigned int offset,size,targets[2],targetCount,base,kind;bool leaf;unsigned int leafKey;};
 enum {Plain=0,Add=1,Set=2,Leaf=3};
 // bytecode.py decode + collect_keys (maximum_key = triangle count), then
 // validate_triangle_keys with the pinned small-triangle cutoff.
 bool Check(const unsigned char* blob,unsigned int bytes,const unsigned char* mesh,unsigned int points,unsigned int triangleCount,PrefabScene::Arena& memory){
  const unsigned char* code=blob+20;unsigned int length=bytes-20;if(*(const unsigned int*)(blob+16)!=length||!length)return Fail("MOPP bytecode length mismatch");
  PrefabScene::Arena work;unsigned int maxCommands=length<1000000?length:1000000;Command* commands=(Command*)work.Allocate((maxCommands+1)*sizeof(Command));unsigned int* at=(unsigned int*)work.Allocate((length+1)*4);if(!commands||!at)return Fail("MOPP decode allocation failed");
  for(unsigned int i=0;i<=length;i++)at[i]=0xffffffffu;unsigned int n=0,pos=0;
  while(pos<length){if(n>=maxCommands)return Fail("MOPP command budget exceeded");Command& c=commands[n];c.offset=pos;c.targetCount=0;c.kind=Plain;c.base=0;c.leaf=false;c.leafKey=0;unsigned int op=code[pos++];
   #define TAKE(k) if(pos+(k)>length)return Fail("Truncated MOPP command");
   if(op>=1&&op<=4){TAKE(3)pos+=3;}
   else if(op>=5&&op<=7){unsigned int w=op-4;TAKE(w)unsigned int off=detail::Big(code+pos,w);pos+=w;c.targets[0]=pos+off;c.targetCount=1;}
   else if(op>=9&&op<=11){unsigned int w=op==9?1:op==10?2:4;TAKE(w)c.base=detail::Big(code+pos,w);pos+=w;c.kind=op==11?Set:Add;}
   else if(op>=0x10&&op<=0x1c){TAKE(3)pos+=2;unsigned int off=code[pos++];c.targets[0]=pos;c.targets[1]=pos+off;c.targetCount=2;}
   else if(op>=0x20&&op<=0x22){TAKE(2)pos+=1;unsigned int off=code[pos++];c.targets[0]=pos;c.targets[1]=pos+off;c.targetCount=2;}
   else if(op>=0x23&&op<=0x25){TAKE(6)pos+=2;unsigned int a=detail::Big(code+pos,2),b=detail::Big(code+pos+2,2);pos+=4;c.targets[0]=pos+a;c.targets[1]=pos+b;c.targetCount=2;}
   else if(op>=0x26&&op<=0x28){TAKE(2)pos+=2;}
   else if(op>=0x29&&op<=0x2b){TAKE(6)pos+=6;}
   else if(op>=0x30&&op<=0x4f){c.leaf=true;c.leafKey=op-0x30;c.kind=Leaf;}
   else if(op>=0x50&&op<=0x53){unsigned int w=op-0x4f;TAKE(w)c.leaf=true;c.leafKey=detail::Big(code+pos,w);pos+=w;c.kind=Leaf;}
   else return Fail("Unknown MOPP opcode");
   #undef TAKE
   c.size=pos-c.offset;at[c.offset]=n;n++;}
  for(unsigned int i=0;i<n;i++){const Command& c=commands[i];for(unsigned int t=0;t<c.targetCount;t++)if(c.targets[t]>length||at[c.targets[t]]==0xffffffffu||c.targets[t]<c.offset+c.size)return Fail("Invalid MOPP branch target");
   if(!c.targetCount&&!c.leaf&&c.offset+c.size>=length)return Fail("Unterminated MOPP control path");}
  // Bounded traversal over (command, base) states.
  unsigned int stateCapacity=4096;while(stateCapacity<2*2000000&&stateCapacity<8*n)stateCapacity*=2;
  unsigned long long* states=(unsigned long long*)work.Allocate(stateCapacity*8);unsigned long long* stack=(unsigned long long*)work.Allocate(2000001*8);unsigned char* keys=(unsigned char*)work.Allocate(triangleCount+1);
  if(!states||!stack||!keys)return Fail("MOPP traversal allocation failed");memset(keys,0,triangleCount+1);for(unsigned int i=0;i<stateCapacity;i++)states[i]=~0ull;
  unsigned int scheduled=0,pending=0;
  #define SCHEDULE(cmd,b) {unsigned long long s=((unsigned long long)(b)<<32)|(cmd);unsigned int hsh=(((unsigned int)(cmd)*2654435761u)^((unsigned int)(b)*0x85ebca6bu))&(stateCapacity-1);bool known=false;while(states[hsh]!=~0ull){if(states[hsh]==s){known=true;break;}hsh=(hsh+1)&(stateCapacity-1);}if(!known){if(scheduled>=2000000||scheduled*2>=stateCapacity)return Fail("MOPP traversal state budget exceeded");states[hsh]=s;scheduled++;stack[pending++]=s;}}
  SCHEDULE(0u,0u);
  while(pending){unsigned long long s=stack[--pending];unsigned int index=(unsigned int)s,base=(unsigned int)(s>>32);const Command& c=commands[index];
   if(c.leaf){unsigned long long key=(unsigned long long)base+c.leafKey;if(key>=triangleCount)return Fail("MOPP leaf key outside allowed range");keys[key]=1;continue;}
   if(c.kind==Add)base+=c.base;else if(c.kind==Set)base=c.base;
   if(c.targetCount){for(unsigned int t=0;t<c.targetCount;t++){unsigned int target=at[c.targets[t]];SCHEDULE(target,base)}}
   else{unsigned int next=at[c.offset+c.size];if(next==0xffffffffu)return Fail("Unterminated MOPP control path");SCHEDULE(next,base)}}
  #undef SCHEDULE
  const WorldText::Vec3* vertices=(const WorldText::Vec3*)(mesh+108);const Triangle* triangles=(const Triangle*)(mesh+108+points*12);float cutoff;unsigned int bits=0x2edbe6ffu;memcpy(&cutoff,&bits,4);unsigned int required=0;
  for(unsigned int k=0;k<triangleCount;k++){bool ok;bool rejected=detail::Rejected(vertices[triangles[k].a],vertices[triangles[k].b],vertices[triangles[k].c],cutoff,ok);if(!ok)return Fail("Unsupported triangle validity arithmetic");
   if(!rejected)required++;if(rejected==(keys[k]!=0))return Fail("Generated MOPP triangle keys differ from the selected source geometry policy");}
  if(!required)return Fail("No triangles supported by the selected MOPP geometry policy");(void)memory;return true;
 }
};
}
