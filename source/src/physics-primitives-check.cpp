#include "WorldPacker.cpp"
#include "PhysicsPrimitives.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
static bool Read(HANDLE f,void* p,unsigned int bytes){DWORD done;return ReadFile(f,p,bytes,&done,0)&&done==bytes;}
extern "C" void HarnessEntry(){
 char input[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_PRIMITIVE_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_PRIMITIVE_OUTPUT",output,MAX_PATH))ExitProcess(1);
 HANDLE f=CreateFileA(input,0x80000000,FILE_SHARE_READ,0,3,FILE_ATTRIBUTE_NORMAL,0);if(f==INVALID_HANDLE_VALUE)ExitProcess(2);
 unsigned int header[2];if(!Read(f,header,8)||header[0]!=0x31525050||header[1]>8192)ExitProcess(3);PrefabScene::Arena memory;unsigned char* data=(unsigned char*)memory.Allocate(header[1]*32+4);if(!data)ExitProcess(3);unsigned int cursor=4;*(unsigned int*)data=header[1];
 for(unsigned int i=0;i<header[1];i++){
  unsigned int kind;float values[5],body[6];if(!Read(f,&kind,4)||!Read(f,values,20))ExitProcess(3);unsigned int size=0;
  if(kind==2){WorldText::Vec3 half={values[2],values[3],values[4]};if(!PhysicsPrimitives::OBB(values[0],values[1],half,body))ExitProcess(4);size=24;}
  else if(kind==3){if(!PhysicsPrimitives::Sphere(values[0],values[1],values[2],body))ExitProcess(4);size=12;}else ExitProcess(4);
  *(unsigned int*)(data+cursor)=kind;*(unsigned int*)(data+cursor+4)=size;cursor+=8;for(unsigned int j=0;j<size;j++)data[cursor++]=((unsigned char*)body)[j];
 }
 DWORD done;unsigned char extra;if(!ReadFile(f,&extra,1,&done,0)||done)ExitProcess(3);CloseHandle(f);f=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(f==INVALID_HANDLE_VALUE||!WriteFile(f,data,cursor,&done,0)||done!=cursor)ExitProcess(5);CloseHandle(f);ExitProcess(0);
}
