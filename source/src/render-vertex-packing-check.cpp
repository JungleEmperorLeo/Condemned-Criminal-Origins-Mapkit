#include "WorldPacker.cpp"
#include "RenderVertexPacking.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
#ifdef FEAR_PACKING_ORACLE
extern "C" __declspec(dllimport) HMODULE WINAPI LoadLibraryA(const char*);
extern "C" __declspec(dllimport) void* WINAPI GetProcAddress(HMODULE,const char*);
#pragma comment(linker,"/alternatename:__imp__LoadLibraryA@4=__imp__LoadLibraryA")
#pragma comment(linker,"/alternatename:__imp__GetProcAddress@8=__imp__GetProcAddress")
#endif
extern "C" void HarnessEntry(){
 char input[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_PACKING_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_PACKING_OUTPUT",output,MAX_PATH))ExitProcess(1);
 HANDLE in=CreateFileA(input,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);DWORD done;unsigned int header[6];
 if(in==INVALID_HANDLE_VALUE||!ReadFile(in,header,24,&done,0)||done!=24||header[0]!=0x314b5056||header[1]>10000||!header[2]||header[2]>64||!header[3]||header[3]>64||!header[4]||header[4]>256||!header[5]||header[5]>256)ExitProcess(2);
 unsigned int count=header[1],sourceCount=header[2],destinationCount=header[3],sourceStride=header[4],destinationStride=header[5];PrefabScene::Arena memory;
 RenderVertexPacking::Element* source=(RenderVertexPacking::Element*)memory.Allocate(sourceCount*8);RenderVertexPacking::Element* destination=(RenderVertexPacking::Element*)memory.Allocate(destinationCount*8);unsigned char* vertices=(unsigned char*)memory.Allocate(count*sourceStride+1);unsigned char* packed=(unsigned char*)memory.Allocate(count*destinationStride+1);
 if(!source||!destination||!vertices||!packed||!ReadFile(in,source,sourceCount*8,&done,0)||done!=sourceCount*8||!ReadFile(in,destination,destinationCount*8,&done,0)||done!=destinationCount*8||!ReadFile(in,vertices,count*sourceStride,&done,0)||done!=count*sourceStride)ExitProcess(3);CloseHandle(in);memset(packed,0xA5,count*destinationStride);
 if(!RenderVertexPacking::Validate(source,sourceCount,sourceStride)||!RenderVertexPacking::Validate(destination,destinationCount,destinationStride))ExitProcess(4);
#ifdef FEAR_PACKING_ORACLE
 HMODULE system=LoadLibraryA("kernel32.dll");typedef unsigned int (WINAPI *ErrorMode)(unsigned int);ErrorMode setErrorMode=system?(ErrorMode)GetProcAddress(system,"SetErrorMode"):0;if(!setErrorMode)ExitProcess(5);setErrorMode(3);
 LoadLibraryA("E:/fearsdk/Dev/Tools/msvcr71.dll");LoadLibraryA("E:/fearsdk/Dev/Tools/msvcp71.dll");LoadLibraryA("E:/fearsdk/Dev/Tools/MFC71.dll");unsigned char* dll=(unsigned char*)LoadLibraryA("E:/fearsdk/Dev/Tools/Packers/WorldPacker.dll");if(!dll)ExitProcess(5);
 const unsigned char signature[8]={0x64,0xa1,0,0,0,0,0x6a,0xff};for(unsigned int i=0;i<8;i++)if(dll[0x79480+i]!=signature[i])ExitProcess(6);
 for(unsigned int i=0;i<sizeof(RenderVertexPacking::Working);i++)if(dll[0x9c0f0+i]!=((const unsigned char*)RenderVertexPacking::Working)[i])ExitProcess(6);
 typedef bool (__cdecl* Convert)(unsigned int,const void*,const void*,unsigned int,const void*,void*,unsigned int);
 if(count&&!((Convert)(dll+0x79480))(count,source,vertices,sourceStride,destination,packed,destinationStride))ExitProcess(7);
#else
 if(!RenderVertexPacking::Convert(count,source,sourceCount,vertices,sourceStride,destination,destinationCount,packed,destinationStride))ExitProcess(5);
#endif
 HANDLE out=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);unsigned int outHeader[3]={0x314b5056,count,destinationStride};if(out==INVALID_HANDLE_VALUE||!WriteFile(out,outHeader,12,&done,0)||done!=12||!WriteFile(out,packed,count*destinationStride,&done,0)||done!=count*destinationStride)ExitProcess(10);CloseHandle(out);ExitProcess(0);
}
