#define FEAR_SOURCE_GEOMETRY_REPAIR_DEFAULT 1
#include "WorldPacker.cpp"
#include "VisibilityCompiler.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
extern "C" void HarnessEntry(){
 void* compilerStorage=VirtualAlloc(0,sizeof(VisibilityCompiler::Builder),MEM_COMMIT|MEM_RESERVE,4);if(!compilerStorage)ExitProcess(6);VisibilityCompiler::Builder& compiler=*new(compilerStorage) VisibilityCompiler::Builder;
 char input[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_READER_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_VISIBILITY_OUTPUT",output,MAX_PATH))ExitProcess(1);
 {
 PrefabScene::Source source;if(!source.Load(input))ExitProcess(3);void* expansionStorage=VirtualAlloc(0,sizeof(PrefabScene::Expansion),MEM_COMMIT|MEM_RESERVE,4);if(!expansionStorage)ExitProcess(6);
 PrefabScene::Expansion* expansion=new(expansionStorage) PrefabScene::Expansion;if(!expansion->Build(input,source.scene))ExitProcess(40);
 if(!compiler.Build(expansion->Scene()))ExitProcess(compiler.errorCode);
 char reuse[4]={};if(GetEnvironmentVariableA("FEAR_COMPILER_REUSE_CHECK",reuse,4)&&reuse[0]=='1'){
  unsigned int hash=2166136261u,bytes=compiler.size;for(unsigned int i=0;i<bytes;i++)hash=(hash^compiler.data[i])*16777619u;
  WorldText::Vec3 invalid={};*(unsigned int*)&invalid.x=0x7fc00000u;if(compiler.Build(expansion->Scene(),&invalid)||compiler.errorCode!=2||compiler.data||compiler.size||compiler.streamingSectors)ExitProcess(50);
  if(!compiler.Build(expansion->Scene())||compiler.size!=bytes)ExitProcess(51);unsigned int again=2166136261u;for(unsigned int i=0;i<compiler.size;i++)again=(again^compiler.data[i])*16777619u;if(again!=hash)ExitProcess(52);
 }
 // Check the exported source-order volumes against serialized final-ID bounds.
 // This runs after the source scene has been destroyed.
 const unsigned char* cursor=compiler.data+32;const unsigned int* header=(const unsigned int*)compiler.data;
 for(unsigned int i=0;i<header[1];i++){unsigned int count=cursor[0]|(cursor[1]<<8);cursor+=34+count*12;}
 for(unsigned int id=0;id<compiler.sectorCount;id++){
  unsigned int length=cursor[0]|(cursor[1]<<8);cursor+=2+length;const StreamingSplit::Bounds* bounds=(const StreamingSplit::Bounds*)cursor;cursor+=24;
  const StreamingMetadata::External* external=0;for(unsigned int i=0;i<compiler.sectorCount;i++)if(compiler.streamingSectors[i].index==id){if(external)ExitProcess(53);external=compiler.streamingSectors+i;}
  if(!external)ExitProcess(54);for(unsigned int a=0;a<3;a++)if(bounds->low[a]!=(&external->volume.low.x)[a]||bounds->high[a]!=(&external->volume.high.x)[a])ExitProcess(55);
  unsigned int planes=*(const unsigned int*)cursor;cursor+=4+planes*16;unsigned int refs=*(const unsigned int*)cursor;cursor+=4+refs*4;if(*(const unsigned int*)cursor!=id)ExitProcess(56);cursor+=4;
 }
 // Exercise the recovered assignment using all owned volumes as one region.
 ConvexIntersection::Volume* volumes=(ConvexIntersection::Volume*)VirtualAlloc(0,compiler.sectorCount*sizeof(ConvexIntersection::Volume),MEM_COMMIT|MEM_RESERVE,4);if(!volumes)ExitProcess(6);
 for(unsigned int i=0;i<compiler.sectorCount;i++)volumes[i]=compiler.streamingSectors[i].volume;
 StreamingMetadata::Region region={volumes,compiler.sectorCount,0,0};StreamingMetadata::Assignment assignment;
 if(!assignment.Build(&region,1,compiler.streamingSectors,compiler.sectorCount)||assignment.regions[0].count!=compiler.sectorCount||assignment.unassigned||assignment.ambiguous)ExitProcess(57);
 for(unsigned int i=0;i<compiler.sectorCount;i++)if(assignment.regions[0].indices[i]!=compiler.streamingSectors[i].index)ExitProcess(58);
 VirtualFree(volumes,0,MEM_RELEASE);
 // Serialized bytes must remain valid after every source/prefab allocation dies.
 expansion->~Expansion();VirtualFree(expansionStorage,0,MEM_RELEASE);
 }
 char offsetPath[MAX_PATH];DWORD done;if(GetEnvironmentVariableA("FEAR_OFFSET_OUTPUT",offsetPath,MAX_PATH)){HANDLE f=CreateFileA(offsetPath,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(f==INVALID_HANDLE_VALUE||!WriteFile(f,&compiler.offset,12,&done,0)||done!=12)ExitProcess(31);CloseHandle(f);}
 HANDLE f=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(f==INVALID_HANDLE_VALUE||!WriteFile(f,compiler.data,compiler.size,&done,0)||done!=compiler.size)ExitProcess(31);CloseHandle(f);ExitProcess(0);
}
