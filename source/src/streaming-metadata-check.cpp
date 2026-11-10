#include "WorldPacker.cpp"

extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
extern "C" __declspec(dllimport) void* WINAPI LoadLibraryA(const char*);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
#pragma comment(linker,"/alternatename:__imp__LoadLibraryA@4=__imp__LoadLibraryA")
struct Face {float* vertices;unsigned int count,capacity,padding;float plane[4];};
struct Volume {Face** faces;unsigned int count,capacity,padding,index;float low[3],high[3];unsigned int rest[8];};
static Face faces[2][6];static Face* pointers[2][6];static float vertices[2][6][12];static Volume volumes[2];
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
 Cube(0,0,100);Cube(1,90,20);
 unsigned int region[40]={},manager[48]={};unsigned int volumePointer=(unsigned int)&volumes[0];region[4]=(unsigned int)&volumePointer;region[5]=1;region[6]=1;
 unsigned int regionPointer=(unsigned int)region;unsigned int regions[]={(unsigned int)&regionPointer,1,1,0};manager[13]=(unsigned int)regions;
 unsigned int externalPointer=(unsigned int)&volumes[1];unsigned int external[]={0,(unsigned int)&externalPointer,1,1,0};
 typedef void (__thiscall *Assign)(void*,void*);((Assign)(dll+0x37990))(manager,external);
 if(region[9]!=1||!region[8]||*(unsigned int*)region[8]!=externalPointer)ExitProcess(20);
 char output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_METADATA_OUTPUT",output,MAX_PATH))ExitProcess(3);HANDLE f=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);DWORD done;unsigned int result[]={region[9],((Volume*)*(unsigned int*)region[8])->index};if(f==INVALID_HANDLE_VALUE||!WriteFile(f,result,8,&done,0)||done!=8)ExitProcess(4);
 typedef bool (__thiscall *Intersects)(void*,void*,float);float centers[]={0,90,118,119,120,121,5000};
 for(unsigned int i=0;i<7;i++){Cube(1,centers[i],20);unsigned int pair[]={*(unsigned int*)&centers[i],((Intersects)(dll+0x13c70))(&volumes[1],&volumes[0],-2.0f)?1u:0u};if(!WriteFile(f,pair,8,&done,0)||done!=8)ExitProcess(4);}
 CloseHandle(f);ExitProcess(0);
}

