#include "WorldPacker.cpp"
#include "RenderBlock.h"
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
 unsigned int n;unsigned char* input=readEnv("FEAR_RENDER_INPUT",&n);if(n<24)ExitProcess(20);
 unsigned int* h=(unsigned int*)input;unsigned int nv=h[0],stride=h[1],nt=h[2],decl=h[3],name=h[4];
 if(nv>65535||nt>1000000||stride>256||decl>256||name>4096||n!=20+nv*stride+nt*6+decl+name+1)ExitProcess(21);
 unsigned char* v=input+20;unsigned short* ids=(unsigned short*)(v+nv*stride);unsigned char* d=(unsigned char*)ids+nt*6;char* m=(char*)(d+decl);if(m[name])ExitProcess(22);
 RenderBlock::Block block;if(!block.BuildSingle(v,nv,stride,ids,nt,d,decl,m))ExitProcess(23);
 if(!GetEnvironmentVariableA("FEAR_RENDER_BLOCK",path,MAX_PATH))ExitProcess(1);HANDLE file=CreateFileA(path,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);DWORD done;
 if(file==INVALID_HANDLE_VALUE||!WriteFile(file,block.data,block.size,&done,0)||done!=block.size)ExitProcess(24);CloseHandle(file);
 unsigned short old=ids[0];ids[0]=(unsigned short)nv;if(block.BuildSingle(v,nv,stride,ids,nt,d,decl,m)||block.data)ExitProcess(25);ids[0]=old;
 unsigned char type=d[4];d[4]=4;if(block.BuildSingle(v,nv,stride,ids,nt,d,decl,m))ExitProcess(26);d[4]=type;
 if(block.BuildSingle(v,nv,stride,ids,nt,d,decl,""))ExitProcess(27);
 ExitProcess(0);
}
