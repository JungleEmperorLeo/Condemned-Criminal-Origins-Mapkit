#define FEAR_PRIMARY_SURFACE_PREPARATION 1
#include "WorldPacker.cpp"
#include "SourceKeyRotation.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
#ifdef FEAR_KEY_ROTATION_ORACLE
extern "C" __declspec(dllimport) HMODULE WINAPI LoadLibraryA(const char*);
#pragma comment(linker,"/alternatename:__imp__LoadLibraryA@4=__imp__LoadLibraryA")
#endif
static bool Write(HANDLE f,const void* p,unsigned int bytes){DWORD done=0;return !bytes||(WriteFile(f,p,bytes,&done,0)&&done==bytes);}
#ifndef FEAR_KEY_ROTATION_ORACLE
static bool Zero(const float* q){const unsigned int* p=(const unsigned int*)q;return !p[0]&&!p[1]&&!p[2]&&!p[3];}
static bool Equal(const float* q,const float* b){for(unsigned int i=0;i<4;i++)if(((const unsigned int*)q)[i]!=((const unsigned int*)b)[i])return false;return true;}
static unsigned int Guards(){
 unsigned int good=0;float q[4];WorldText::Vec3 zero={};
 if(!SourceKeyRotation::EulerQuaternion(zero,0))good++;
 unsigned int nan=0x7fc00000u;WorldText::Vec3 bad={};memcpy(&bad.x,&nan,4);q[0]=q[1]=q[2]=q[3]=9;
 if(!SourceKeyRotation::EulerQuaternion(bad,q)&&Zero(q))good++;
 bad.x=1000001;q[0]=q[1]=q[2]=q[3]=9;if(!SourceKeyRotation::EulerQuaternion(bad,q)&&Zero(q))good++;
 if(SourceKeyRotation::EulerQuaternion(zero,q)&&!((unsigned int*)q)[0]&&!((unsigned int*)q)[1]&&!((unsigned int*)q)[2]&&q[3]==1)good++;
 PrefabScene::Node node;memset(&node,0,sizeof(node));node.kind=2;node.orientation=PrefabScene::Matrix();
 if(SourceKeyRotation::Quaternion(node,q)&&q[3]==1)good++;
 PrefabScene::Property p;memset(&p,0,sizeof(p));lstrcpyA(p.name,"Rotation");lstrcpyA(p.type,"rotation");p.data=const_cast<char*>("(eulerangles (0.25 -0.5 0.75))");node.properties=&p;
 float expected[4];WorldText::Vec3 angles={.25f,-.5f,.75f};SourceKeyRotation::EulerQuaternion(angles,expected);if(SourceKeyRotation::Quaternion(node,q)&&Equal(q,expected))good++;
 lstrcpyA(p.type,"vector");if(!SourceKeyRotation::Quaternion(node,q)&&Zero(q))good++;lstrcpyA(p.type,"rotation");
 p.data=0;if(!SourceKeyRotation::Quaternion(node,q)&&Zero(q))good++;p.data=const_cast<char*>("(eulerangles (0 0))");if(!SourceKeyRotation::Quaternion(node,q)&&Zero(q))good++;
 p.data=const_cast<char*>("(eulerangles (0 0 0))");node.importedFromPrefab=true;memcpy(&node.orientation.m[0][0],&nan,4);if(!SourceKeyRotation::Quaternion(node,q)&&Zero(q))good++;
 node.orientation=PrefabScene::Matrix();if(SourceKeyRotation::Quaternion(node,q)&&q[3]==1)good++;
 memcpy(&node.position.x,&nan,4);if(!SourceKeyRotation::Quaternion(node,q)&&Zero(q))good++;
 return good;
}
#endif
extern "C" void HarnessEntry(){
 char input[MAX_PATH],output[MAX_PATH];DWORD ni=GetEnvironmentVariableA("FEAR_KEY_ROTATION_INPUT",input,MAX_PATH),no=GetEnvironmentVariableA("FEAR_KEY_ROTATION_OUTPUT",output,MAX_PATH);if(!ni||ni>=MAX_PATH||!no||no>=MAX_PATH)ExitProcess(1);
 HANDLE file=CreateFileA(input,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);if(file==INVALID_HANDLE_VALUE)ExitProcess(2);unsigned int bytes=GetFileSize(file,0);if(bytes<4||bytes>4+8192*12)ExitProcess(2);PrefabScene::Arena memory;unsigned char* data=(unsigned char*)memory.Allocate(bytes);DWORD done=0;if(!data||!ReadFile(file,data,bytes,&done,0)||done!=bytes)ExitProcess(2);CloseHandle(file);
 unsigned int count=*(unsigned int*)data;if(!count||count>8192||bytes!=4+count*12)ExitProcess(3);
 for(unsigned int i=0;i<count;i++){const WorldText::Vec3& a=*(const WorldText::Vec3*)(data+4+i*12);if(!WorldText::FiniteVector(a)||a.x< -1000000||a.x>1000000||a.y< -1000000||a.y>1000000||a.z< -1000000||a.z>1000000)ExitProcess(3);}
#ifdef FEAR_KEY_ROTATION_ORACLE
 LoadLibraryA("E:/fearsdk/Dev/Tools/msvcr71.dll");LoadLibraryA("E:/fearsdk/Dev/Tools/msvcp71.dll");LoadLibraryA("E:/fearsdk/Dev/Tools/MFC71.dll");unsigned char* dll=(unsigned char*)LoadLibraryA("E:/fearsdk/Dev/Tools/Packers/WorldPacker.dll");if(!dll)ExitProcess(4);
 const unsigned char signature[12]={0x55,0x8b,0xec,0x83,0xe4,0xf8,0x83,0xec,0x54,0xd9,0x45,0x0c};for(unsigned int i=0;i<12;i++)if(dll[0xc940+i]!=signature[i])ExitProcess(4);
 typedef void (__thiscall* Euler)(float*,float,float,float);Euler original=(Euler)(dll+0xc940);unsigned int guardCount=0;
#else
 unsigned int guardCount=Guards();if(guardCount!=12)ExitProcess(5);
#endif
 file=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(file==INVALID_HANDLE_VALUE||!Write(file,&count,4)||!Write(file,&guardCount,4))ExitProcess(6);
 for(unsigned int i=0;i<count;i++){const WorldText::Vec3& a=*(const WorldText::Vec3*)(data+4+i*12);float q[4]={};
#ifdef FEAR_KEY_ROTATION_ORACLE
 original(q,a.x,a.y,a.z);
#else
 if(!SourceKeyRotation::EulerQuaternion(a,q))ExitProcess(7);
#endif
 if(!Write(file,q,16))ExitProcess(6);}
 CloseHandle(file);ExitProcess(0);
}
