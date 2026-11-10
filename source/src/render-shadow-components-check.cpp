#include "WorldPacker.cpp"
#include "RenderShadowComponents.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
#ifdef FEAR_SHADOW_ORACLE
extern "C" __declspec(dllimport) HMODULE WINAPI LoadLibraryA(const char*);
#pragma comment(linker,"/alternatename:__imp__LoadLibraryA@4=__imp__LoadLibraryA")
#endif
extern "C" void HarnessEntry(){
 char inputPath[MAX_PATH],outputPath[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_SHADOW_INPUT",inputPath,MAX_PATH)||!GetEnvironmentVariableA("FEAR_SHADOW_OUTPUT",outputPath,MAX_PATH))ExitProcess(1);
 HANDLE in=CreateFileA(inputPath,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);DWORD done;unsigned int header[2];
 if(in==INVALID_HANDLE_VALUE||!ReadFile(in,header,8,&done,0)||done!=8||!header[0]||header[0]>10000)ExitProcess(2);
 PrefabScene::Arena memory;RenderShadowComponents::Triangle* input=(RenderShadowComponents::Triangle*)memory.Allocate(header[0]*52);if(!input||!ReadFile(in,input,header[0]*52,&done,0)||done!=header[0]*52)ExitProcess(3);CloseHandle(in);
 RenderShadowComponents::Mesh mesh;if(!mesh.Build(input,header[0],header[1])||!mesh.count)ExitProcess(4);
 unsigned int* all=(unsigned int*)memory.Allocate(mesh.count*4);if(!all)ExitProcess(5);for(unsigned int i=0;i<mesh.count;i++)all[i]=1;
 unsigned int outward=RenderShadowComponents::Outward(input,mesh.source,all,mesh.count)?1:0;
 WorldText::Vec3* points=mesh.points;unsigned int pointCount=mesh.pointCount;
#ifdef FEAR_SHADOW_ORACLE
 LoadLibraryA("E:/fearsdk/Dev/Tools/msvcr71.dll");LoadLibraryA("E:/fearsdk/Dev/Tools/msvcp71.dll");LoadLibraryA("E:/fearsdk/Dev/Tools/MFC71.dll");unsigned char* dll=(unsigned char*)LoadLibraryA("E:/fearsdk/Dev/Tools/Packers/WorldPacker.dll");if(!dll)ExitProcess(6);
 const unsigned char signature[4]={0x83,0xec,0x10,0x53};for(unsigned int i=0;i<4;i++)if(dll[0x487a0+i]!=signature[i])ExitProcess(7);
 const unsigned int capacity=mesh.count*3;unsigned char* hash=(unsigned char*)memory.Allocate(0x100c);unsigned int* bucketStorage=(unsigned int*)memory.Allocate(capacity*256*4);points=(WorldText::Vec3*)memory.Allocate(capacity*12);
 unsigned char* sourceTriangles=(unsigned char*)memory.Allocate(header[0]*280);unsigned char* node=(unsigned char*)memory.Allocate(0x240);int* adjacency=(int*)memory.Allocate(mesh.count*12);unsigned int* selected=(unsigned int*)memory.Allocate(mesh.count*4);
 if(!hash||!bucketStorage||!points||!sourceTriangles||!node||!adjacency||!selected)ExitProcess(8);memset(hash,0,0x100c);memset(sourceTriangles,0,header[0]*280);memset(node,0,0x240);
 for(unsigned int i=0;i<256;i++){unsigned int* slot=(unsigned int*)(hash+i*16);slot[1]=slot[2]=(unsigned int)(bucketStorage+i*capacity);slot[3]=(unsigned int)(bucketStorage+(i+1)*capacity);}
 for(unsigned int i=0;i<header[0];i++)for(unsigned int k=0;k<3;k++)memcpy(sourceTriangles+i*280+k*84,&input[i].p[k],12);
 *(unsigned int*)(node+0x68)=(unsigned int)sourceTriangles;*(unsigned int*)(node+0x6c)=(unsigned int)(sourceTriangles+header[0]*280);
 unsigned int pointsVector[4]={0,(unsigned int)points,(unsigned int)points,(unsigned int)(points+capacity)};
 typedef unsigned int (__cdecl* Insert)(void*,const void*,void*);Insert insert=(Insert)(dll+0x487a0);
 for(unsigned int i=0;i<mesh.count;i++){selected[i]=mesh.source[i];for(int k=2;k>=0;k--)mesh.results[i].point[k]=insert(hash,&input[selected[i]].p[k],pointsVector);for(unsigned int k=0;k<3;k++)adjacency[i*3+k]=mesh.results[i].adjacent[k];}
 pointCount=(pointsVector[2]-pointsVector[1])/12;
 unsigned int adjacencyVector[4]={0,(unsigned int)adjacency,(unsigned int)(adjacency+mesh.count*3),(unsigned int)(adjacency+mesh.count*3)};
 unsigned int mappingVector[4]={0,(unsigned int)selected,(unsigned int)(selected+mesh.count),(unsigned int)(selected+mesh.count)};
 unsigned int oneSided[5]={0,0,0,0,0},closed[5]={0,0,0,0,0};
 typedef void (__thiscall* Classify)(void*,void*,void*,void*,void*);Classify classify=(Classify)(dll+0x4a2a0);classify(node,adjacencyVector,mappingVector,oneSided,closed);
 unsigned int* oneBits=(unsigned int*)oneSided[2],*closedBits=(unsigned int*)closed[2];if(oneSided[0]!=mesh.count||closed[0]!=mesh.count||!oneBits||!closedBits)ExitProcess(9);
 for(unsigned int i=0;i<mesh.count;i++){mesh.results[i].closed=(closedBits[i/32]>>(i%32))&1;mesh.results[i].oneSided=(oneBits[i/32]>>(i%32))&1;}
 unsigned int words=(mesh.count+31)/32;unsigned int* allBits=(unsigned int*)memory.Allocate(words*4);if(!allBits)ExitProcess(10);for(unsigned int i=0;i<words;i++)allBits[i]=0xffffffffu;
 unsigned int componentVector[5]={mesh.count,0,(unsigned int)allBits,(unsigned int)(allBits+words),(unsigned int)(allBits+words)};
 typedef unsigned int (__thiscall* IsOutward)(void*,unsigned int,void*,void*);outward=((IsOutward)(dll+0x43440))(node,0,mappingVector,componentVector)&1;
#endif
 HANDLE out=CreateFileA(outputPath,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);unsigned int outputHeader[4]={0x31504353,mesh.count,pointCount,outward};
 if(out==INVALID_HANDLE_VALUE||!WriteFile(out,outputHeader,16,&done,0)||done!=16||!WriteFile(out,mesh.source,mesh.count*4,&done,0)||done!=mesh.count*4||!WriteFile(out,mesh.results,mesh.count*32,&done,0)||done!=mesh.count*32||!WriteFile(out,points,pointCount*12,&done,0)||done!=pointCount*12)ExitProcess(11);CloseHandle(out);ExitProcess(0);
}
