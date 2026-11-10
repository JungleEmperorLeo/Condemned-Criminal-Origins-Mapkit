#include "WorldPacker.cpp"
#include "PhysicalNormalOrder.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
extern "C" __declspec(dllimport) HMODULE WINAPI LoadLibraryA(const char*);
extern "C" __declspec(dllimport) void* WINAPI GetProcAddress(HMODULE,const char*);
#pragma comment(linker,"/alternatename:__imp__LoadLibraryA@4=__imp__LoadLibraryA")
#pragma comment(linker,"/alternatename:__imp__GetProcAddress@8=__imp__GetProcAddress")
static bool __cdecl Greater(const PhysicalNormalOrder::Entry* a,const PhysicalNormalOrder::Entry* b){return a->uses>b->uses;}
extern "C" void HarnessEntry(){
 typedef void (__cdecl* Sort)(PhysicalNormalOrder::Entry**,PhysicalNormalOrder::Entry**,unsigned int,void*);
 LoadLibraryA("E:/fearsdk/Dev/Tools/msvcr71.dll");LoadLibraryA("E:/fearsdk/Dev/Tools/msvcp71.dll");LoadLibraryA("E:/fearsdk/Dev/Tools/MFC71.dll");unsigned char* dll=(unsigned char*)LoadLibraryA("E:/fearsdk/Dev/Tools/Packers/WorldPacker.dll");if(!dll)ExitProcess(1);
 if(dll[0x26330]!=0x83||dll[0x26331]!=0xec||dll[0x26332]!=8)ExitProcess(2);
 PrefabScene::Arena memory;PhysicalNormalOrder::Entry* values=(PhysicalNormalOrder::Entry*)memory.Allocate(4096*sizeof(PhysicalNormalOrder::Entry));PhysicalNormalOrder::Entry* native=(PhysicalNormalOrder::Entry*)memory.Allocate(4096*sizeof(PhysicalNormalOrder::Entry));PhysicalNormalOrder::Entry** original=(PhysicalNormalOrder::Entry**)memory.Allocate(4096*sizeof(void*));if(!values||!native||!original)ExitProcess(3);
 unsigned int seed=0x31514267,passed=0;const unsigned int lengths[]={0,1,2,3,6,18,31,32,33,40,41,42,59,60,87,128,257,1000,3420,4096};
 for(unsigned int idealMode=0;idealMode<3;idealMode++)for(unsigned int mode=0;mode<10;mode++)for(unsigned int test=0;test<20;test++){
  unsigned int n=lengths[test],ideal=idealMode==0?n:idealMode==1?1u:0u;
  for(unsigned int i=0;i<n;i++){seed=seed*1664525u+1013904223u;unsigned int uses=mode==0?0:mode==1?1:mode==2?i:mode==3?n-i:mode==4?i%2:mode==5?i%7:mode==6?(seed%3):mode==7?(seed%97):mode==8?(i%5?0:100):seed;values[i].identity=i;values[i].uses=uses;native[i]=values[i];original[i]=values+i;}
  ((Sort)(dll+0x26330))(original,original+n,ideal,(void*)&Greater);PhysicalNormalOrder::Range(native,native+n,ideal);
  for(unsigned int i=0;i<n;i++)if(native[i].identity!=original[i]->identity)ExitProcess(10+mode);passed++;
 }
 char output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_MAIN_BSP_OUTPUT",output,MAX_PATH))ExitProcess(4);HANDLE file=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);DWORD done;if(file==INVALID_HANDLE_VALUE||!WriteFile(file,&passed,4,&done,0)||done!=4)ExitProcess(5);CloseHandle(file);ExitProcess(0);
}
