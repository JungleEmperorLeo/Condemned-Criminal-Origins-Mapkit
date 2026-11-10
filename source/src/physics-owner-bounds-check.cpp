#include "WorldPacker.cpp"
#include "PhysicsOwnerBounds.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
static bool Read(HANDLE f,void* p,unsigned int bytes){DWORD done;return ReadFile(f,p,bytes,&done,0)&&done==bytes;}
extern "C" void HarnessEntry(){
 char input[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_OWNER_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_OWNER_OUTPUT",output,MAX_PATH))ExitProcess(1);
 HANDLE f=CreateFileA(input,0x80000000,FILE_SHARE_READ,0,3,FILE_ATTRIBUTE_NORMAL,0);if(f==INVALID_HANDLE_VALUE)ExitProcess(2);unsigned int header[2];if(!Read(f,header,8)||header[0]!=0x31424f50||header[1]>8192)ExitProcess(3);
 PrefabScene::Arena memory;unsigned int size=4+header[1]*36;unsigned char* data=(unsigned char*)memory.Allocate(size);if(!data)ExitProcess(3);*(unsigned int*)data=header[1];
 for(unsigned int i=0;i<header[1];i++){
  float meta[17];unsigned int count=0;if(!Read(f,meta,68)||!Read(f,&count,4)||!count||count>1000000)ExitProcess(3);WorldText::Vec3 pos={meta[12],meta[13],meta[14]};PhysicsOwnerBounds::Bounds all,physics;
  for(unsigned int j=0;j<count;j++){unsigned int flags;WorldText::Vec3 point,local;if(!Read(f,&flags,4)||!Read(f,&point,12))ExitProcess(3);if(!PhysicsOwnerBounds::LocalPoint(meta,pos,point,local)||!all.Add(local))ExitProcess(4);if((flags&0x10)&&!physics.Add(local))ExitProcess(4);}
  float* result=(float*)(data+4+i*36);WorldText::Vec3 translation;if(!PhysicsOwnerBounds::Finish(all,physics,meta[15],meta[16],result,translation))ExitProcess(4);result[6]=translation.x;result[7]=translation.y;result[8]=translation.z;
 }
 DWORD done;unsigned char extra;if(!ReadFile(f,&extra,1,&done,0)||done)ExitProcess(3);CloseHandle(f);f=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(f==INVALID_HANDLE_VALUE||!WriteFile(f,data,size,&done,0)||done!=size)ExitProcess(5);CloseHandle(f);ExitProcess(0);
}
