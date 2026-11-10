#include "WorldPacker.cpp"
#include "MaterialResources.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
static char path[MAX_PATH];
extern "C" void HarnessEntry(){
 if(!GetEnvironmentVariableA("FEAR_MATERIAL_INPUT",path,MAX_PATH))ExitProcess(1);
 HANDLE file=CreateFileA(path,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);if(file==INVALID_HANDLE_VALUE)ExitProcess(2);
 unsigned int size=GetFileSize(file,0);if(!size||size>16*1024*1024)ExitProcess(3);
 unsigned char* bytes=(unsigned char*)VirtualAlloc(0,size,MEM_COMMIT|MEM_RESERVE,4);DWORD done;
 if(!bytes||!ReadFile(file,bytes,size,&done,0)||done!=size)ExitProcess(4);CloseHandle(file);
 MaterialResources::Material material;if(!material.Read(bytes,size))ExitProcess(20);
 if(!GetEnvironmentVariableA("FEAR_MATERIAL_OUTPUT",path,MAX_PATH))ExitProcess(1);
 file=CreateFileA(path,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(file==INVALID_HANDLE_VALUE)ExitProcess(5);
 if(!WriteFile(file,&material.textureCount,4,&done,0)||done!=4)ExitProcess(6);
 for(unsigned int i=0;i<=material.textureCount;i++){
  const char* value=i?material.textures[i-1]:material.shader;unsigned int n=0;while(value[n])n++;n++;
  if(!WriteFile(file,value,n,&done,0)||done!=n)ExitProcess(7);
 }
 CloseHandle(file);ExitProcess(0);
}
