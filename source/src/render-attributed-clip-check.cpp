#include "WorldPacker.cpp"
#include "RenderAttributedClip.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
#ifdef FEAR_CLIP_ORACLE
extern "C" __declspec(dllimport) HMODULE WINAPI LoadLibraryA(const char*);
#pragma comment(linker,"/alternatename:__imp__LoadLibraryA@4=__imp__LoadLibraryA")
typedef RenderClipVertex::Vertex Vertex;
struct Vector {unsigned int unused;unsigned char* first;unsigned char* end;unsigned char* capacity;};
struct Face {unsigned int unused[4];RenderAttributedClip::Plane plane;};
struct Volume {Face** faces;unsigned int count;};
static Vertex raw[128];static Face faces[128];static Face* pointers[128];
static bool SaveVertices(HANDLE file,Vector& vector){unsigned int count=(unsigned int)(vector.end-vector.first)/44;DWORD done;if(!WriteFile(file,&count,4,&done,0)||done!=4)return false;for(unsigned int i=0;i<count;i++)if(!WriteFile(file,vector.first+i*44,44,&done,0)||done!=44)return false;return true;}
#endif
static bool Read(HANDLE f,void* p,unsigned int n){DWORD done;return ReadFile(f,p,n,&done,0)&&done==n;}
static bool Put(HANDLE f,const void* p,unsigned int n){DWORD done;return WriteFile(f,p,n,&done,0)&&done==n;}
static RenderClipVertex::Vertex points[128];static RenderAttributedClip::Plane planes[128];
#ifndef FEAR_CLIP_ORACLE
static bool Guards(){
 RenderClipVertex::Vertex quad[4]={};WorldText::Vec3 normal={0,0,1};RenderAttributedClip::Plane plane={1,0,0,0};RenderAttributedClip::Builder builder;
 quad[0].point.x=quad[3].point.x=-2;quad[1].point.x=quad[2].point.x=2;quad[2].point.y=quad[3].point.y=4;
 for(unsigned int i=0;i<4;i++)quad[i].attributes[6]=2;
 if(!builder.Build(quad,4,normal,&plane,1))return false;
 quad[1].attributes[0]=99;if(builder.inside.points[1].attributes[0]==99)return false;
 plane.x=0;if(builder.Build(quad,4,normal,&plane,1)||builder.inside.points||builder.outside||!builder.error)return false;
 plane.x=1;if(!builder.Build(quad,4,normal,&plane,1))return false;
 for(unsigned int i=0;i<4;i++)quad[i].attributes[6]=quad[i].point.x<0?-1.f:1.f;
 if(builder.Build(quad,4,normal,&plane,1)||builder.inside.points||builder.outside||!builder.error)return false;
 for(unsigned int i=0;i<4;i++)quad[i].attributes[6]=2;
 return builder.Build(quad,4,normal,&plane,1);
}
#endif
extern "C" void HarnessEntry(){
 char input[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_RENDER_CLIP_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_RENDER_CLIP_OUTPUT",output,MAX_PATH))ExitProcess(1);
#ifdef FEAR_CLIP_ORACLE
 LoadLibraryA("E:\\fearsdk\\Dev\\Tools\\msvcr71.dll");LoadLibraryA("E:\\fearsdk\\Dev\\Tools\\msvcp71.dll");LoadLibraryA("E:\\fearsdk\\Dev\\Tools\\MFC71.dll");unsigned char* dll=(unsigned char*)LoadLibraryA("E:\\fearsdk\\Dev\\Tools\\Packers\\WorldPacker.dll");if(!dll)ExitProcess(2);
 typedef bool (__stdcall* Clip)(Vector*,Volume*,WorldText::Vec3*,float,Vector*,Vector*,Vector*);Clip clip=(Clip)(dll+0x46cf0);
#else
 if(!Guards())ExitProcess(7);
#endif
 HANDLE in=CreateFileA(input,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);if(in==INVALID_HANDLE_VALUE)ExitProcess(2);unsigned int samples=0;if(!Read(in,&samples,4)||samples>10000)ExitProcess(3);
 HANDLE out=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(out==INVALID_HANDLE_VALUE)ExitProcess(6);
 for(unsigned int sample=0;sample<samples;sample++){
  unsigned int counts[2]={};WorldText::Vec3 normal={};float epsilon=0;if(!Read(in,counts,8)||counts[0]<3||counts[0]>128||!counts[1]||counts[1]>128||!Read(in,&normal,12)||!Read(in,&epsilon,4)||!Read(in,planes,counts[1]*16)||!Read(in,points,counts[0]*44))ExitProcess(3);
#ifdef FEAR_CLIP_ORACLE
  memcpy(raw,points,counts[0]*44);for(unsigned int i=0;i<counts[1];i++){faces[i].plane=planes[i];pointers[i]=faces+i;}
  Vector source={0,(unsigned char*)raw,(unsigned char*)(raw+counts[0]),(unsigned char*)(raw+counts[0])},inside={},outside={},fragments={};Volume volume={pointers,counts[1]};clip(&source,&volume,&normal,epsilon,&inside,&outside,&fragments);
  if(!SaveVertices(out,inside))ExitProcess(7);unsigned int fragmentCount=(unsigned int)(fragments.end-fragments.first)/4;if(!Put(out,&fragmentCount,4))ExitProcess(7);unsigned int start=0;for(unsigned int j=0;j<fragmentCount;j++){unsigned int n=((unsigned int*)fragments.first)[j];if(!Put(out,&n,4))ExitProcess(7);for(unsigned int k=0;k<n;k++)if(!Put(out,outside.first+(start+k)*44,44))ExitProcess(7);start+=n;}
  // Small isolated oracle batches intentionally retain original allocations until exit.
#else
  RenderAttributedClip::Builder builder;if(!builder.Build(points,counts[0],normal,planes,counts[1],epsilon))ExitProcess(5);if(!Put(out,&builder.inside.count,4)||!Put(out,builder.inside.points,builder.inside.count*44)||!Put(out,&builder.outsideCount,4))ExitProcess(7);for(unsigned int j=0;j<builder.outsideCount;j++)if(!Put(out,&builder.outside[j].count,4)||!Put(out,builder.outside[j].points,builder.outside[j].count*44))ExitProcess(7);
#endif
 }CloseHandle(in);CloseHandle(out);ExitProcess(0);
}
