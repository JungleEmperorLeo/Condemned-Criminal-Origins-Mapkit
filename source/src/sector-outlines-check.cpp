#include "WorldPacker.cpp"
#include "SectorOutlines.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
extern "C" __declspec(dllimport) void* WINAPI LoadLibraryA(const char*);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
#pragma comment(linker,"/alternatename:__imp__LoadLibraryA@4=__imp__LoadLibraryA")
static unsigned int rawFaces[128][12],pointers[128],volume[32];static SectorOrientation::Face faces[128];
extern "C" void HarnessEntry(){
 LoadLibraryA("E:\\fearsdk\\Dev\\Tools\\msvcr71.dll");LoadLibraryA("E:\\fearsdk\\Dev\\Tools\\msvcp71.dll");LoadLibraryA("E:\\fearsdk\\Dev\\Tools\\MFC71.dll");unsigned char* dll=(unsigned char*)LoadLibraryA("E:\\fearsdk\\Dev\\Tools\\Packers\\WorldPacker.dll");if(!dll)ExitProcess(2);void* original=dll+0x2d8e0;
 char input[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_OUTLINE_INPUT",input,MAX_PATH))ExitProcess(1);HANDLE f=CreateFileA(input,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);DWORD done;unsigned int cases=0;if(f==INVALID_HANDLE_VALUE||!ReadFile(f,&cases,4,&done,0)||done!=4)ExitProcess(3);
 for(unsigned int i=0;i<cases;i++){unsigned int count=0;if(!ReadFile(f,&count,4,&done,0)||done!=4||!count||count>128)ExitProcess(4);for(unsigned int j=0;j<count;j++){if(!ReadFile(f,&faces[j].plane,16,&done,0)||done!=16)ExitProcess(4);pointers[j]=(unsigned int)rawFaces[j];for(unsigned int k=0;k<4;k++)((float*)rawFaces[j])[4+k]=(&faces[j].plane.x)[k];}volume[0]=(unsigned int)pointers;volume[1]=count;volume[2]=count;unsigned int result=0;
  __asm push ebx
  __asm lea ebx,volume
  __asm call original
  __asm movzx eax,al
  __asm mov result,eax
  __asm pop ebx
  if((SectorOutlines::Count(faces,count)==0)!=(result!=0))ExitProcess(20);
 }
 CloseHandle(f);ExitProcess(0);
}
