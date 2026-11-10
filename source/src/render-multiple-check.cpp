#include "WorldPacker.cpp"
#include "RenderBlock.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
static char path[MAX_PATH];
extern "C" void HarnessEntry(){
 if(!GetEnvironmentVariableA("FEAR_RENDER_INPUT",path,MAX_PATH))ExitProcess(1);HANDLE f=CreateFileA(path,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);if(f==INVALID_HANDLE_VALUE)ExitProcess(2);unsigned int size=GetFileSize(f,0);if(size<16||size>32*1024*1024)ExitProcess(3);unsigned char* bytes=(unsigned char*)VirtualAlloc(0,size,MEM_COMMIT|MEM_RESERVE,4);DWORD done;if(!bytes||!ReadFile(f,bytes,size,&done,0)||done!=size)ExitProcess(4);CloseHandle(f);
 unsigned int* h=(unsigned int*)bytes;unsigned int n=h[0],trees=h[1],stride=h[2],declBytes=h[3],at=16;if(!n||n>1024||declBytes>256||at+declBytes>size)ExitProcess(5);const void* decl=bytes+at;at+=declBytes;PrefabScene::Arena arena;RenderBlock::SurfaceInput* input=(RenderBlock::SurfaceInput*)arena.Allocate(n*sizeof(RenderBlock::SurfaceInput));if(!input)ExitProcess(6);
 for(unsigned int i=0;i<n;i++){if(size-at<16)ExitProcess(7);unsigned int* a=(unsigned int*)(bytes+at);at+=16;RenderBlock::SurfaceInput& s=input[i];s.count=a[0];s.triangles=a[1];s.tree=a[2];unsigned int len=a[3];if(s.count>65535||s.triangles>1000000||stride>256||len>4096)ExitProcess(8);unsigned int need=s.count*stride+s.triangles*6+len+1;if(need>size-at)ExitProcess(9);s.vertices=bytes+at;at+=s.count*stride;s.indices=(unsigned short*)(bytes+at);at+=s.triangles*6;s.material=(char*)(bytes+at);if(bytes[at+len])ExitProcess(10);at+=len+1;}
 if(at!=size)ExitProcess(11);RenderBlock::Block block;if(!block.BuildMultiple(input,n,trees,stride,decl,declBytes))ExitProcess(12);
 if(!GetEnvironmentVariableA("FEAR_RENDER_BLOCK",path,MAX_PATH))ExitProcess(13);f=CreateFileA(path,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(f==INVALID_HANDLE_VALUE||!WriteFile(f,block.data,block.size,&done,0)||done!=block.size)ExitProcess(14);CloseHandle(f);
 unsigned int old=input[0].tree;input[0].tree=trees;if(block.BuildMultiple(input,n,trees,stride,decl,declBytes)||block.data)ExitProcess(15);input[0].tree=old;
 unsigned short* ids=(unsigned short*)input[0].indices;unsigned short id=ids[0];ids[0]=(unsigned short)input[0].count;if(block.BuildMultiple(input,n,trees,stride,decl,declBytes)||block.data)ExitProcess(16);ids[0]=id;
 if(block.BuildMultiple(input,n,trees+1,stride,decl,declBytes)||block.data)ExitProcess(17);
 ExitProcess(0);
}
