#define FEAR_SOURCE_GEOMETRY_REPAIR_DEFAULT 1
#include "WorldPacker.cpp"
#include "StreamingWorkerCore.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
static bool Flag(const char* name){char value[4]={};return GetEnvironmentVariableA(name,value,4)&&value[0]=='1';}
extern "C" void HarnessEntry(){
 char inputPath[MAX_PATH],outputPath[MAX_PATH],sourcePath[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_RESOURCE_INPUT",inputPath,MAX_PATH))ExitProcess(1);
 HANDLE f=CreateFileA(inputPath,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);if(f==INVALID_HANDLE_VALUE)ExitProcess(2);unsigned int bytes=GetFileSize(f,0);if(bytes<12||bytes>16*1024*1024)ExitProcess(3);
 unsigned char* data=(unsigned char*)VirtualAlloc(0,bytes,MEM_COMMIT|MEM_RESERVE,4);DWORD done;if(!data||!ReadFile(f,data,bytes,&done,0)||done!=bytes)ExitProcess(4);CloseHandle(f);
 if(!GetEnvironmentVariableA("FEAR_READER_INPUT",sourcePath,MAX_PATH))ExitProcess(23);
 StreamingWorkerCore::Request request;request.source=sourcePath;request.input=data;request.bytes=bytes;request.sourceMaterials=Flag("FEAR_ASSEMBLY_SOURCE_MATERIALS");
 char texture[4]={};if(GetEnvironmentVariableA("FEAR_MATERIAL_TEXTURES",texture,4))request.textureMode=texture[0];
 request.soundDatabase=Flag("FEAR_ASSEMBLY_SOUND_DATABASE");request.clientFx=Flag("FEAR_ASSEMBLY_CLIENTFX");request.visibility=Flag("FEAR_ASSEMBLY_VISIBILITY");
 request.automaticPost=Flag("FEAR_ASSEMBLY_AUTO_POST");request.tutorialPost=Flag("FEAR_ASSEMBLY_TUTORIAL_POST");request.multiplayerPost=Flag("FEAR_ASSEMBLY_MULTIPLAYER_POST");request.singleplayerPost=Flag("FEAR_ASSEMBLY_SINGLEPLAYER_POST");
 PrefabScene::Arena output;const unsigned char* block=0;unsigned int size=0;const char* resolved=0;unsigned int code=StreamingWorkerCore::Run(request,output,block,size,resolved);if(code)ExitProcess(code);
 if(!GetEnvironmentVariableA("FEAR_RESOURCE_OUTPUT",outputPath,MAX_PATH))ExitProcess(1);f=CreateFileA(outputPath,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(f==INVALID_HANDLE_VALUE||!WriteFile(f,block,size,&done,0)||done!=size)ExitProcess(5);CloseHandle(f);
 DWORD reportLength=GetEnvironmentVariableA("FEAR_ASSEMBLY_POST_PROFILE_REPORT",outputPath,MAX_PATH);if(reportLength){if(reportLength>=MAX_PATH)ExitProcess(41);unsigned int length=lstrlenA(resolved);f=CreateFileA(outputPath,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(f==INVALID_HANDLE_VALUE||!WriteFile(f,resolved,length,&done,0)||done!=length)ExitProcess(41);CloseHandle(f);}ExitProcess(0);
}
