#include "WorldPacker.cpp"
#include "StreamingSplit.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
extern "C" __declspec(dllimport) void* WINAPI LoadLibraryA(const char*);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
#pragma comment(linker,"/alternatename:__imp__LoadLibraryA@4=__imp__LoadLibraryA")
static unsigned int state=20261010;
static unsigned int Random(){state=state*1664525+1013904223;return state;}
extern "C" void HarnessEntry(){
 LoadLibraryA("E:\\fearsdk\\Dev\\Tools\\msvcr71.dll");LoadLibraryA("E:\\fearsdk\\Dev\\Tools\\msvcp71.dll");LoadLibraryA("E:\\fearsdk\\Dev\\Tools\\MFC71.dll");
 unsigned char* dll=(unsigned char*)LoadLibraryA("E:\\fearsdk\\Dev\\Tools\\Packers\\WorldPacker.dll");if(!dll)ExitProcess(2);
 typedef bool (__cdecl *Split)(void*,unsigned int*,float*,void*,void*,void*);
 static StreamingSplit::Bounds bounds[64];static unsigned int volumes[64][11],pointers[64],ids[3][64];
 for(unsigned int test=0;test<2000;test++){
  unsigned int n=1+Random()%64;
  for(unsigned int i=0;i<n;i++){pointers[i]=(unsigned int)volumes[i];for(unsigned int a=0;a<3;a++){
   float low=(float)((int)(Random()%2001)-1000);float width=(float)(Random()%601);
   if(test%4==0){low=(float)((int)(Random()%5)-2)*100;width=100;}
   bounds[i].low[a]=low;bounds[i].high[a]=low+width;*(float*)(volumes[i]+5+a)=low;*(float*)(volumes[i]+8+a)=low+width;
  }}
  unsigned int input[4]={(unsigned int)pointers,n,n,0},output[3][4]={},axis=3;float coordinate=0;
  if(!((Split)(dll+0x2af40))(input,&axis,&coordinate,output[0],output[1],output[2]))ExitProcess(3);
  StreamingSplit::Result own;for(unsigned int k=0;k<3;k++)own.ids[k]=ids[k];if(!StreamingSplit::Choose(bounds,n,own))ExitProcess(4);
  if(axis!=own.axis||*(unsigned int*)&coordinate!=*(unsigned int*)&own.coordinate)ExitProcess(20);
  for(unsigned int k=0;k<3;k++){if(output[k][1]!=own.count[k])ExitProcess(21);unsigned int* original=(unsigned int*)output[k][0];for(unsigned int j=0;j<own.count[k];j++)if(original[j]!=own.ids[k][j])ExitProcess(22);}
 }
 ExitProcess(0);
}


