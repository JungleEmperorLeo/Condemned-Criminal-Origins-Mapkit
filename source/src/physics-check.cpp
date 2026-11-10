#include "WorldPacker.cpp"
#include "PhysicsMeshBlock.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
static char path[MAX_PATH];
static unsigned char* readEnv(const char* name,unsigned int* size){
 if(!GetEnvironmentVariableA(name,path,MAX_PATH))ExitProcess(1);
 HANDLE file=CreateFileA(path,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);if(file==INVALID_HANDLE_VALUE)ExitProcess(2);
 *size=GetFileSize(file,0);if(!*size||*size>16*1024*1024)ExitProcess(3);
 unsigned char* data=(unsigned char*)VirtualAlloc(0,*size,MEM_COMMIT|MEM_RESERVE,4);DWORD done;
 if(!data||!ReadFile(file,data,*size,&done,0)||done!=*size)ExitProcess(4);CloseHandle(file);return data;
}
extern "C" void HarnessEntry(){
 unsigned int meshBytes,moppBytes;
 unsigned char* meshData=readEnv("FEAR_PHYSICS_MESH_PREFIX",&meshBytes);unsigned char* mopp=readEnv("FEAR_MOPP_INPUT",&moppBytes);
 PhysicsMeshPrefix::Block mesh;mesh.size=meshBytes;mesh.data=(unsigned char*)mesh.memory.Allocate(meshBytes);if(!mesh.data)ExitProcess(6);memcpy(mesh.data,meshData,meshBytes);
 PhysicsMeshBlock::Block block;if(!block.Build(mesh,mopp,moppBytes)||block.size!=meshBytes+moppBytes)ExitProcess(20);
 // Ownership: changing the input does not change the completed block.
 unsigned char first=mopp[0];mopp[0]^=1;if(block.data[meshBytes]!=first)ExitProcess(21);mopp[0]=first;
 if(block.Build(mesh,mopp,20)||block.data||block.size)ExitProcess(22);
 if(block.Build(mesh,mopp,moppBytes-1))ExitProcess(23);
 unsigned int oldScale=*(unsigned int*)(mopp+12);*(unsigned int*)(mopp+12)=0x7fc00000;
 if(block.Build(mesh,mopp,moppBytes))ExitProcess(24);*(unsigned int*)(mopp+12)=oldScale;
 unsigned short* indices=(unsigned short*)(mesh.data+108+12**(unsigned int*)(mesh.data+100));unsigned short oldIndex=*indices;*indices=(unsigned short)*(unsigned int*)(mesh.data+100);
 if(block.Build(mesh,mopp,moppBytes))ExitProcess(25);*indices=oldIndex;
 if(!block.Build(mesh,mopp,moppBytes))ExitProcess(26);
 if(!GetEnvironmentVariableA("FEAR_PHYSICS_BLOCK",path,MAX_PATH))ExitProcess(1);
 HANDLE file=CreateFileA(path,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);DWORD done;
 if(file==INVALID_HANDLE_VALUE||!WriteFile(file,block.data,block.size,&done,0)||done!=block.size)ExitProcess(27);CloseHandle(file);ExitProcess(0);
}
