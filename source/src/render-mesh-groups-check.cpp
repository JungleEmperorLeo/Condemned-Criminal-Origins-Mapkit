#include "WorldPacker.cpp"
#include "RenderMeshGroups.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
extern "C" __declspec(dllimport) HMODULE WINAPI LoadLibraryA(const char*);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
#pragma comment(linker,"/alternatename:__imp__LoadLibraryA@4=__imp__LoadLibraryA")
// Diagnostic input has fixed 64-byte region names; the native grouping API uses
// const char* and supports larger names without depending on this file ABI.
extern "C" void HarnessEntry(){
 char input[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_MESH_GROUPS_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_MESH_GROUPS_OUTPUT",output,MAX_PATH))ExitProcess(1);HANDLE file=CreateFileA(input,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);if(file==INVALID_HANDLE_VALUE)ExitProcess(2);
 DWORD bytes=GetFileSize(file,0),done=0;PrefabScene::Arena memory;unsigned char* data=(unsigned char*)memory.Allocate(bytes+1);if(!data||bytes<16||bytes>1024*1024||!ReadFile(file,data,bytes,&done,0)||done!=bytes)ExitProcess(3);CloseHandle(file);unsigned int* h=(unsigned int*)data;if(h[0]!=0x31474d52||!h[1]||h[1]>256||h[2]>=h[1]||h[3]>64)ExitProcess(4);
 unsigned int treeCount=h[1],main=h[2],regionCount=h[3];const unsigned char* cursor=data+16;const unsigned char* end=data+bytes;if(end-cursor<(int)(treeCount*4))ExitProcess(4);RenderMeshGroups::Tree* trees=(RenderMeshGroups::Tree*)cursor;cursor+=treeCount*4;
 RenderMeshGroups::Region* regions=(RenderMeshGroups::Region*)memory.Allocate((regionCount+1)*sizeof(RenderMeshGroups::Region));if(!regions)ExitProcess(5);unsigned int references=0;for(unsigned int r=0;r<regionCount;r++){
  if(end-cursor<68)ExitProcess(4);regions[r].name=(char*)cursor;((char*)cursor)[63]=0;cursor+=64;unsigned int n=*(unsigned int*)cursor;cursor+=4;if(n>4096||end-cursor<(int)(n*4))ExitProcess(4);regions[r].count=n;regions[r].sectors=(unsigned int*)cursor;cursor+=n*4;references+=n;
 }
 if(cursor!=end)ExitProcess(4);RenderMeshGroups::Builder builder;if(!builder.Build(trees,treeCount,main,regions,regionCount))ExitProcess(6);unsigned int nodes=builder.nodeCount;
#ifdef FEAR_MESH_GROUPS_ORACLE
 LoadLibraryA("E:/fearsdk/Dev/Tools/msvcr71.dll");LoadLibraryA("E:/fearsdk/Dev/Tools/msvcp71.dll");LoadLibraryA("E:/fearsdk/Dev/Tools/MFC71.dll");unsigned char* dll=(unsigned char*)LoadLibraryA("E:/fearsdk/Dev/Tools/Packers/WorldPacker.dll");if(!dll||dll[0x4e320]!=0x6a)ExitProcess(7);
 // Only field layouts actually read by 0x4E320 are populated. Node pointers
 // are real local address tokens; no node payload is read by this grouping.
 unsigned int* treeRecords=(unsigned int*)memory.Allocate(treeCount*20);unsigned int* treePointers=(unsigned int*)memory.Allocate(treeCount*4);unsigned int* nodeRecords=(unsigned int*)memory.Allocate((nodes+1)*4);unsigned int* nodePointers=(unsigned int*)memory.Allocate((nodes+1)*4);unsigned int* sectorRecords=(unsigned int*)memory.Allocate((trees[main].nodeCount+1)*112);unsigned int* sectorPointers=(unsigned int*)memory.Allocate((references+1)*4);unsigned int* regionRecords=(unsigned int*)memory.Allocate((regionCount+1)*40);unsigned int* regionPointers=(unsigned int*)memory.Allocate((regionCount+1)*4);if(!treeRecords||!treePointers||!nodeRecords||!nodePointers||!sectorRecords||!sectorPointers||!regionRecords||!regionPointers)ExitProcess(8);
 memset(treeRecords,0,treeCount*20);memset(sectorRecords,0,(trees[main].nodeCount+1)*112);memset(regionRecords,0,(regionCount+1)*40);for(unsigned int n=0;n<nodes;n++){nodeRecords[n]=n;nodePointers[n]=(unsigned int)(nodeRecords+n);}for(unsigned int n=0;n<trees[main].nodeCount;n++)sectorRecords[n*28+27]=n;
 unsigned int first=0;for(unsigned int t=0;t<treeCount;t++){unsigned int* tree=treeRecords+t*5;treePointers[t]=(unsigned int)tree;tree[1]=(unsigned int)(nodePointers+first);first+=trees[t].nodeCount;tree[2]=tree[3]=(unsigned int)(nodePointers+first);}
 unsigned int ref=0;for(unsigned int r=0;r<regionCount;r++){unsigned int* region=regionRecords+r*10;regionPointers[r]=(unsigned int)region;region[8]=(unsigned int)(sectorPointers+ref);region[9]=regions[r].count;for(unsigned int i=0;i<regions[r].count;i++)sectorPointers[ref++]=(unsigned int)(sectorRecords+regions[r].sectors[i]*28);}
 unsigned int world[5]={0,(unsigned int)treePointers,(unsigned int)(treePointers+treeCount),(unsigned int)(treePointers+treeCount),main};unsigned int list[3]={(unsigned int)regionPointers,regionCount,regionCount},result[4]={};typedef bool (__thiscall* Group)(void*,void*,void*);if(!((Group)(dll+0x4e320))(world,result,regionCount?list:0))ExitProcess(9);unsigned int groups=(result[2]-result[1])/4;if(groups!=builder.count)ExitProcess(10);
 for(unsigned int g=0;g<groups;g++){unsigned int* original=(unsigned int*)((unsigned int*)result[1])[g];RenderMeshGroups::Group& own=builder.groups[g];own.regionIndex=0xffffffffu;for(unsigned int r=0;r<regionCount;r++)if(original[0]==regionPointers[r])own.regionIndex=r;own.count=(original[3]-original[2])/4;own.nodes=(unsigned int*)memory.Allocate((own.count+1)*4);if(!own.nodes)ExitProcess(11);for(unsigned int n=0;n<own.count;n++){unsigned int ptr=((unsigned int*)original[2])[n];if(ptr<(unsigned int)nodeRecords||ptr>=(unsigned int)(nodeRecords+nodes)||(ptr-(unsigned int)nodeRecords)%4)ExitProcess(12);own.nodes[n]=(ptr-(unsigned int)nodeRecords)/4;}}
 // Groups contain plain region pointers and owned node-pointer vectors. Free
 // only original-owned buffers with its matching allocator, then the object.
 typedef void (__cdecl* Free)(void*);Free free=(Free)(dll+0x62c60);for(unsigned int g=0;g<groups;g++){unsigned int* original=(unsigned int*)((unsigned int*)result[1])[g];if(original[2])free((void*)original[2]);free(original);}if(result[1])free((void*)result[1]);
#endif
 HANDLE out=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);unsigned int resultHeader[4]={0x31474d52,builder.count,nodes,builder.mainBegin};if(out==INVALID_HANDLE_VALUE||!WriteFile(out,resultHeader,16,&done,0)||done!=16)ExitProcess(13);
 for(unsigned int g=0;g<builder.count;g++){const RenderMeshGroups::Group& group=builder.groups[g];unsigned int gh[2]={group.regionIndex,group.count};if(!WriteFile(out,gh,8,&done,0)||done!=8||!WriteFile(out,group.nodes,group.count*4,&done,0)||done!=group.count*4)ExitProcess(13);}CloseHandle(out);ExitProcess(0);
}
