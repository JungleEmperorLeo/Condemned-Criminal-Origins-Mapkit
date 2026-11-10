#include "WorldPacker.cpp"
#include "StreamingMetadata.h"

extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
extern "C" __declspec(dllimport) void* WINAPI LoadLibraryA(const char*);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
#pragma comment(linker,"/alternatename:__imp__LoadLibraryA@4=__imp__LoadLibraryA")
struct Face {float* vertices;unsigned int count,capacity,padding;float plane[4];};
struct Volume {Face** faces;unsigned int count,capacity,padding,index;float low[3],high[3];unsigned int rest[8];};
static Face faces[5][6];static Face* pointers[5][6];static float vertices[5][6][12];static Volume volumes[5];
static void Cube(unsigned int id,float center,float half){
 Volume& v=volumes[id];v.faces=pointers[id];v.count=v.capacity=6;v.index=77;
 for(unsigned int a=0;a<3;a++){v.low[a]=(a==0?center:0)-half;v.high[a]=(a==0?center:0)+half;}
 for(unsigned int face=0;face<6;face++){
  unsigned int a=face/2,b=(a+1)%3,c=(a+2)%3;Face& f=faces[id][face];pointers[id][face]=&f;f.vertices=vertices[id][face];f.count=f.capacity=4;float coordinate=face%2?v.high[a]:v.low[a];f.plane[a]=face%2?-1.0f:1.0f;f.plane[3]=f.plane[a]*coordinate;
  for(unsigned int j=0;j<4;j++){f.vertices[3*j+a]=coordinate;f.vertices[3*j+b]=(j==1||j==2)?v.high[b]:v.low[b];f.vertices[3*j+c]=j>=2?v.high[c]:v.low[c];}
 }
}
extern "C" void HarnessEntry(){
 LoadLibraryA("E:\\fearsdk\\Dev\\Tools\\msvcr71.dll");LoadLibraryA("E:\\fearsdk\\Dev\\Tools\\msvcp71.dll");LoadLibraryA("E:\\fearsdk\\Dev\\Tools\\MFC71.dll");unsigned char* dll=(unsigned char*)LoadLibraryA("E:\\fearsdk\\Dev\\Tools\\Packers\\WorldPacker.dll");if(!dll)ExitProcess(2);
 typedef void (__thiscall *Assign)(void*,void*);
 unsigned int rng=20261010;
 for(unsigned int test=0;test<1000;test++){
  Cube(0,0,100);Cube(1,200,100);Cube(2,400,100);rng=rng*1664525+1013904223;Cube(3,(float)((int)(rng%701)-150),20);volumes[3].index=77;rng=rng*1664525+1013904223;Cube(4,(float)((int)(rng%701)-150),20);volumes[4].index=88;
  unsigned int region[2][40]={},manager[48]={},link[16]={};unsigned int vptr[3]={(unsigned int)&volumes[0],(unsigned int)&volumes[1],(unsigned int)&volumes[2]};
  region[0][4]=(unsigned int)&vptr[0];region[0][5]=region[0][6]=1;region[1][4]=(unsigned int)&vptr[1];region[1][5]=region[1][6]=1;
  link[8]=(unsigned int)&vptr[2];link[9]=link[10]=1;unsigned int linkptr=(unsigned int)link;region[0][0]=(unsigned int)&linkptr;region[0][1]=region[0][2]=1;
  unsigned int rptr[2]={(unsigned int)region[0],(unsigned int)region[1]};if(test%2){unsigned int swap=rptr[0];rptr[0]=rptr[1];rptr[1]=swap;}
  unsigned int regions[]={(unsigned int)rptr,2,2,0};manager[13]=(unsigned int)regions;
  unsigned int eptr[]={(unsigned int)&volumes[3],(unsigned int)&volumes[4],(unsigned int)&volumes[3]},external[]={0,(unsigned int)eptr,3,3,0};
  ((Assign)(dll+0x37990))(manager,external);
  ConvexIntersection::Face nf[5][6];ConvexIntersection::Volume nv[5];
  for(unsigned int k=0;k<5;k++){nv[k].faces=nf[k];nv[k].count=6;nv[k].low=*(WorldText::Vec3*)volumes[k].low;nv[k].high=*(WorldText::Vec3*)volumes[k].high;for(unsigned int j=0;j<6;j++){nf[k][j].points=(WorldText::Vec3*)faces[k][j].vertices;nf[k][j].count=4;nf[k][j].plane=*(ExtrusionVolume::Plane*)faces[k][j].plane;}}
  StreamingMetadata::Region nr[2]={{nv,1,nv+2,1},{nv+1,1,0,0}};if(test%2){StreamingMetadata::Region swap=nr[0];nr[0]=nr[1];nr[1]=swap;}
  StreamingMetadata::External ne[]={{nv[3],77},{nv[4],88},{nv[3],77}};StreamingMetadata::Assignment own;if(!own.Build(nr,2,ne,3))ExitProcess(30);
  for(unsigned int j=0;j<2;j++){unsigned int* original=(unsigned int*)rptr[j];if(original[9]!=own.regions[j].count)ExitProcess(31);for(unsigned int i=0;i<original[9];i++)if(((Volume*)((unsigned int*)original[8])[i])->index!=own.regions[j].indices[i])ExitProcess(32);}
 }
 char output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_METADATA_OUTPUT",output,MAX_PATH))ExitProcess(3);HANDLE f=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);DWORD done;unsigned int result[]={1000,0};if(f==INVALID_HANDLE_VALUE||!WriteFile(f,result,8,&done,0)||done!=8)ExitProcess(4);CloseHandle(f);ExitProcess(0);
}
