#define FEAR_NATIVE_NV_RUNTIME 1
#define FEAR_RENDER_INDEX_REORDER 1
#include "WorldPacker.cpp"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
#include "RenderNvRuntime.h"
#include "RenderIndexedSections.h"
extern "C" void HarnessEntry(){
 char input[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_SECTION_MESH_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_SECTION_MESH_OUTPUT",output,MAX_PATH))ExitProcess(1);
 HANDLE in=CreateFileA(input,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);DWORD done;unsigned int count=0;
 if(in==INVALID_HANDLE_VALUE||!ReadFile(in,&count,4,&done,0)||done!=4||count>RenderIndexedLimits::MaximumTriangles)ExitProcess(2);
 PrefabScene::Arena memory;RenderIndexedSections::Triangle* triangles=(RenderIndexedSections::Triangle*)memory.Allocate((count?count:1)*sizeof(RenderIndexedSections::Triangle));if(!triangles)ExitProcess(3);
 for(unsigned int i=0;i<count;i++){
  unsigned int size=0;if(!ReadFile(in,&triangles[i].category,4,&done,0)||done!=4||!ReadFile(in,&size,4,&done,0)||done!=4||!size||size>4095)ExitProcess(3);
  char* name=(char*)memory.Allocate(size+1);if(!name||!ReadFile(in,name,size,&done,0)||done!=size||!ReadFile(in,triangles[i].vertices,252,&done,0)||done!=252)ExitProcess(3);name[size]=0;triangles[i].material=name;
 }
 unsigned char tail;if(!ReadFile(in,&tail,1,&done,0)||done)ExitProcess(3);CloseHandle(in);
 RenderIndexedSections::Builder mesh;if(!mesh.Build(triangles,count,RenderExceptionalUV::Enabled()))ExitProcess(4);
 char cache[4]={};if(GetEnvironmentVariableA("FEAR_SECTION_MESH_CACHE",cache,4)&&cache[0]=='1'&&!mesh.Reorder())ExitProcess(6);
 HANDLE out=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);unsigned int header[5]={0x314d5352,mesh.sectionCount,mesh.triangleCount,mesh.vertexCount,count};
 if(out==INVALID_HANDLE_VALUE||!WriteFile(out,header,20,&done,0)||done!=20)ExitProcess(5);
 for(unsigned int s=0;s<mesh.sectionCount;s++){
  const RenderSectionRules::Record& record=mesh.sections[s];const char* name=RenderSectionRules::Material(record);unsigned int n=0;while(name[n])n++;
  unsigned int fields[5]={(unsigned int)RenderSectionRules::Kind(record),0,0,0,n};memcpy(fields+1,record.bytes+24,4);memcpy(fields+2,record.bytes+32,8);
  if(!WriteFile(out,fields,20,&done,0)||done!=20||!WriteFile(out,name,n,&done,0)||done!=n)ExitProcess(5);
 }
 if(!WriteFile(out,mesh.indices,mesh.triangleCount*12,&done,0)||done!=mesh.triangleCount*12||!WriteFile(out,mesh.vertices,mesh.vertexCount*84,&done,0)||done!=mesh.vertexCount*84||!WriteFile(out,mesh.triangleSections,count*4,&done,0)||done!=count*4)ExitProcess(5);
 CloseHandle(out);ExitProcess(0);
}
