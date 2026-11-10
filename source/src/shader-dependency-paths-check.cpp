#include "WorldPacker.cpp"
#include "ShaderDependencyPaths.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
extern "C" __declspec(dllimport) HMODULE WINAPI LoadLibraryA(const char*);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
#pragma comment(linker,"/alternatename:__imp__LoadLibraryA@4=__imp__LoadLibraryA")
struct PathInput {unsigned int mode;char base[264],reference[264];};
struct PathOutput {unsigned int success;char result[528];};
extern "C" void HarnessEntry(){
 char path[MAX_PATH],dest[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_SHADER_PATH_INPUT",path,MAX_PATH)||!GetEnvironmentVariableA("FEAR_SHADER_PATH_OUTPUT",dest,MAX_PATH))ExitProcess(1);
 HANDLE in=CreateFileA(path,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);DWORD done=0;unsigned int count=0;if(in==INVALID_HANDLE_VALUE||!ReadFile(in,&count,4,&done,0)||done!=4||!count||count>100000)ExitProcess(2);
 PrefabScene::Arena memory;PathInput* inputs=(PathInput*)memory.Allocate(count*sizeof(PathInput));PathOutput* outputs=(PathOutput*)memory.Allocate(count*sizeof(PathOutput));if(!inputs||!outputs||!ReadFile(in,inputs,count*sizeof(PathInput),&done,0)||done!=count*sizeof(PathInput))ExitProcess(3);CloseHandle(in);
#ifdef FEAR_SHADER_PATH_ORACLE
 LoadLibraryA("E:/fearsdk/Dev/Tools/msvcr71.dll");LoadLibraryA("E:/fearsdk/Dev/Tools/msvcp71.dll");LoadLibraryA("E:/fearsdk/Dev/Tools/MFC71.dll");unsigned char* dll=(unsigned char*)LoadLibraryA("E:/fearsdk/Dev/Tools/Packers/WorldPacker.dll");if(!dll||dll[0x7c9e0]!=0x8b||dll[0x7ca40]!=0x53)ExitProcess(4);
 typedef void (__cdecl* Directory)(const char*,char*,unsigned int);typedef void (__cdecl* Resolve)(const char*,const char*,char*,unsigned int);
#endif
 for(unsigned int i=0;i<count;i++){
  PathInput& p=inputs[i];p.base[263]=p.reference[263]=0;memset(&outputs[i],0,sizeof(PathOutput));char* result=outputs[i].result;
  if(p.mode==0)outputs[i].success=ShaderDependencyPaths::Directory(p.base,result,528)?1:0;
  else if(p.mode==1)outputs[i].success=ShaderDependencyPaths::Resolve(p.base,p.reference,result,528)?1:0;else ExitProcess(5);
#ifdef FEAR_SHADER_PATH_ORACLE
  // Bounded ordinary cases only: no original UI/assertion or overflow path.
  if(p.mode==0)((Directory)(dll+0x7c9e0))(p.base,result,261);else ((Resolve)(dll+0x7ca40))(p.base,p.reference,result,521);
#endif
 }
 HANDLE out=CreateFileA(dest,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(out==INVALID_HANDLE_VALUE||!WriteFile(out,&count,4,&done,0)||done!=4||!WriteFile(out,outputs,count*sizeof(PathOutput),&done,0)||done!=count*sizeof(PathOutput))ExitProcess(6);CloseHandle(out);ExitProcess(0);
}
