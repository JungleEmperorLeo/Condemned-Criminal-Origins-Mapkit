#include "WorldPacker.cpp"
#include "SourceRenderTJunctions.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
extern "C" __declspec(dllimport) unsigned int WINAPI SetErrorMode(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
#pragma comment(linker,"/alternatename:__imp__SetErrorMode@4=__imp__SetErrorMode")
#ifdef FEAR_TJUNCTION_ORACLE
extern "C" __declspec(dllimport) HMODULE WINAPI LoadLibraryA(const char*);
#pragma comment(linker,"/alternatename:__imp__LoadLibraryA@4=__imp__LoadLibraryA")
#endif
static bool Save(const char* path,const void* data,unsigned int size,const unsigned int* count=0){HANDLE file=CreateFileA(path,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(file==INVALID_HANDLE_VALUE)return false;DWORD done=0;bool ok=(!count||(WriteFile(file,count,4,&done,0)&&done==4))&&(!size||(WriteFile(file,data,size,&done,0)&&done==size));CloseHandle(file);return ok;}
extern "C" void HarnessEntry(){
 SetErrorMode(3);char input[MAX_PATH],output[MAX_PATH],statsPath[MAX_PATH];DWORD inputLength=GetEnvironmentVariableA("FEAR_TJUNCTION_INPUT",input,MAX_PATH),outputLength=GetEnvironmentVariableA("FEAR_TJUNCTION_OUTPUT",output,MAX_PATH),statsLength=GetEnvironmentVariableA("FEAR_TJUNCTION_STATS",statsPath,MAX_PATH);if(!inputLength||inputLength>=MAX_PATH||!outputLength||outputLength>=MAX_PATH||statsLength>=MAX_PATH)ExitProcess(1);
 HANDLE file=CreateFileA(input,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);DWORD done=0;unsigned int count=0;if(file==INVALID_HANDLE_VALUE||!ReadFile(file,&count,4,&done,0)||done!=4||!count||count>SourceRenderTJunctions::MaximumInputTriangles||GetFileSize(file,0)!=4+count*260)ExitProcess(2);
 PrefabScene::Arena memory;SourceRenderTJunctions::Triangle* triangles=(SourceRenderTJunctions::Triangle*)memory.Allocate(count*260);if(!triangles||!ReadFile(file,triangles,count*260,&done,0)||done!=count*260)ExitProcess(3);CloseHandle(file);
 SourceRenderTJunctions::Builder result;if(!result.Build(triangles,count,RenderExceptionalUV::Enabled()))ExitProcess(4);SourceRenderTJunctions::Triangle* vertices=result.vertices;unsigned int outputCount=result.count;
#ifdef FEAR_TJUNCTION_ORACLE
 // Diagnostic only: a standalone child calls this helper directly. No Process,
 // world source reader, observer injection, project callbacks or registry writes.
 LoadLibraryA("E:/fearsdk/Dev/Tools/msvcr71.dll");LoadLibraryA("E:/fearsdk/Dev/Tools/msvcp71.dll");LoadLibraryA("E:/fearsdk/Dev/Tools/MFC71.dll");unsigned char* dll=(unsigned char*)LoadLibraryA("E:/fearsdk/Dev/Tools/Packers/WorldPacker.dll");if(!dll)ExitProcess(5);
 const unsigned char sig[]={0x6a,0xff,0x68,0x4f,0x56,0x08,0x10};for(unsigned int i=0;i<sizeof(sig);i++)if(dll[0x48960+i]!=sig[i])ExitProcess(6);
 typedef void* (__cdecl* Allocate)(unsigned int);Allocate allocate=(Allocate)(dll+0x62cc4);unsigned char* working=(unsigned char*)allocate(count*280);unsigned char* section=(unsigned char*)allocate(40);if(!working||!section)ExitProcess(7);memset(working,0,count*280);memset(section,0,40);
 for(unsigned int i=0;i<count;i++){memcpy(working+i*280,triangles[i].vertices,252);memcpy(working+i*280+272,&triangles[i].edgeFlags,4);memcpy(working+i*280+276,&triangles[i].origin,4);}
 unsigned int node[141]={};node[1]=(unsigned int)section;node[2]=(unsigned int)(section+40);node[3]=node[2];node[26]=(unsigned int)working;node[27]=(unsigned int)(working+count*280);node[28]=node[27];
 typedef void (__thiscall* Fix)(void*);Fix fix=(Fix)(dll+0x48960);fix(node);if(node[27]<node[26]||(node[27]-node[26])%280)ExitProcess(8);outputCount=(node[27]-node[26])/280;if(outputCount>1000000)ExitProcess(8);vertices=(SourceRenderTJunctions::Triangle*)memory.Allocate(outputCount*260);if(outputCount&&!vertices)ExitProcess(8);
 working=(unsigned char*)node[26];for(unsigned int i=0;i<outputCount;i++){memcpy(vertices[i].vertices,working+i*280,252);memcpy(&vertices[i].edgeFlags,working+i*280+272,4);memcpy(&vertices[i].origin,working+i*280+276,4);}
#endif
 if(!Save(output,vertices,outputCount*260,&outputCount))ExitProcess(9);if(statsLength&&!Save(statsPath,&result.stats,sizeof(result.stats)))ExitProcess(10);ExitProcess(0);
}
