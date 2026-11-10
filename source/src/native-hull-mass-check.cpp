#include "WorldPacker.cpp"
#include "NativeHullMassProperties.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
static NativeHullMassProperties::Vec4 points[128];
static NativeHullMassProperties::Triangle triangles[256];
static bool Write(HANDLE f,const void* p,unsigned int n){DWORD done=0;return !n||(WriteFile(f,p,n,&done,0)&&done==n);}
static bool Cleared(const NativeHullMassProperties::Result& r){const unsigned char* b=(const unsigned char*)&r;for(unsigned int i=0;i<sizeof(r);i++)if(b[i])return false;return true;}
static bool Same(const void* a,const void* b,unsigned int size){const unsigned char* x=(const unsigned char*)a;const unsigned char* y=(const unsigned char*)b;for(unsigned int i=0;i<size;i++)if(x[i]!=y[i])return false;return true;}
static unsigned int ApiGuards(unsigned int* checks){
 using namespace NativeHullMassProperties;
 Vec4 p[4]={{0,0,0,0},{1,0,0,0},{0,1,0,0},{0,0,1,0}},saved[4];
 Triangle t[4]={{0,2,1},{0,1,3},{0,3,2},{1,2,3}},savedTriangles[4];
 memcpy(saved,p,sizeof(p));memcpy(savedTriangles,t,sizeof(t));
 Result baseline,result;*checks=0;
 if(Integrate(p,4,t,4,2,&baseline)!=Ok)return 1;
 (*checks)++;
#define EXPECT_FAILURE(expected,expression) do {memset(&result,0x5a,sizeof(result));if((expression)!=(expected)||!Cleared(result))return *checks+1;(*checks)++;} while(0)
 EXPECT_FAILURE(InvalidInput,Integrate(0,4,t,4,2,&result));
 EXPECT_FAILURE(InvalidInput,Integrate(p,4,0,4,2,&result));
 EXPECT_FAILURE(InvalidInput,Integrate(p,0,t,4,2,&result));
 EXPECT_FAILURE(InvalidInput,Integrate(p,4,t,0,2,&result));
 EXPECT_FAILURE(InvalidInput,Integrate(p,4,t,4,2,&result,3,4));
 EXPECT_FAILURE(InvalidInput,Integrate(p,4,t,4,2,&result,4,3));
 EXPECT_FAILURE(InvalidInput,Integrate(p,1000001,t,4,2,&result,2000000,4));
 EXPECT_FAILURE(InvalidInput,Integrate(p,4,t,2000001,2,&result,4,3000000));
 union Bits {unsigned int u;float f;} bits;
 const unsigned int invalidMass[]={0,0x80000000u,0xbf800000u,0x7fc00000u,0x7f800000u,0xff800000u};
 for(unsigned int i=0;i<6;i++){bits.u=invalidMass[i];EXPECT_FAILURE(InvalidMass,Integrate(p,4,t,4,bits.f,&result));}
 for(unsigned int i=0;i<4;i++){bits.u=i==3?0x7f800000u:0x7fc00000u;((float*)p)[i]=bits.f;EXPECT_FAILURE(InvalidInput,Integrate(p,4,t,4,2,&result));memcpy(p,saved,sizeof(p));}
 for(unsigned int i=0;i<3;i++){((unsigned int*)t)[i]=4;EXPECT_FAILURE(InvalidInput,Integrate(p,4,t,4,2,&result));memcpy(t,savedTriangles,sizeof(t));}
 p[3].z=0;EXPECT_FAILURE(NonfiniteResult,Integrate(p,4,t,4,2,&result));memcpy(p,saved,sizeof(p));
 p[1].x=p[2].y=p[3].z=1e30f;EXPECT_FAILURE(NonfiniteResult,Integrate(p,4,t,4,2,&result));memcpy(p,saved,sizeof(p));
#undef EXPECT_FAILURE
 if(Integrate(p,4,t,4,2,0)!=InvalidInput)return *checks+1;(*checks)++;
 for(unsigned int i=0;i<16;i++)((float*)p)[i]*=2;
 if(Integrate(p,4,t,4,3,&result)!=Ok)return *checks+1;(*checks)++;
 memcpy(p,saved,sizeof(p));if(Integrate(p,4,t,4,2,&result)!=Ok||!Same(&result,&baseline,sizeof(result)))return *checks+1;(*checks)++;
 if(!Same(p,saved,sizeof(p))||!Same(t,savedTriangles,sizeof(t)))return *checks+1;(*checks)++;
 return 0;
}
extern "C" void HarnessEntry(){
 unsigned int checks=0,failure=ApiGuards(&checks);unsigned short control=0;__asm fnstcw control
 char guardPath[MAX_PATH];DWORD ng=GetEnvironmentVariableA("FEAR_HULL_MASS_GUARD_OUTPUT",guardPath,MAX_PATH);
 if(ng&&ng<MAX_PATH){unsigned int report[3]={checks,control,failure};HANDLE guard=CreateFileA(guardPath,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(guard==INVALID_HANDLE_VALUE||!Write(guard,report,sizeof(report)))ExitProcess(6);CloseHandle(guard);}
 if(failure)ExitProcess(7);
 char input[MAX_PATH],output[MAX_PATH];DWORD ni=GetEnvironmentVariableA("FEAR_HULL_MASS_INPUT",input,MAX_PATH),no=GetEnvironmentVariableA("FEAR_HULL_MASS_OUTPUT",output,MAX_PATH);if(!ni||ni>=MAX_PATH||!no||no>=MAX_PATH)ExitProcess(1);
 HANDLE file=CreateFileA(input,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);if(file==INVALID_HANDLE_VALUE)ExitProcess(4);unsigned int size=GetFileSize(file,0);if(size<4||size>4u*1024u*1024u)ExitProcess(4);PrefabScene::Arena memory;unsigned char* data=(unsigned char*)memory.Allocate(size);DWORD done=0;if(!data||!ReadFile(file,data,size,&done,0)||done!=size)ExitProcess(4);CloseHandle(file);
 unsigned int count=*(unsigned int*)data,at=4;if(!count||count>5000)ExitProcess(5);
 for(unsigned int c=0;c<count;c++){if(size-at<12)ExitProcess(5);unsigned int nv=*(unsigned int*)(data+at),nt=*(unsigned int*)(data+at+4);float mass=*(float*)(data+at+8);at+=12;if(nv>128||nt>256||!WorldText::Finite(mass)||nv>(size-at)/16)ExitProcess(5);
  for(unsigned int i=0;i<nv;i++){NativeHullMassProperties::Vec4 p;memcpy(&p,data+at,16);at+=16;if(!WorldText::Finite(p.x)||!WorldText::Finite(p.y)||!WorldText::Finite(p.z)||!WorldText::Finite(p.w))ExitProcess(5);}
  if(nt>(size-at)/12)ExitProcess(5);for(unsigned int i=0;i<nt;i++){NativeHullMassProperties::Triangle t;memcpy(&t,data+at,12);at+=12;if(t.a>=nv||t.b>=nv||t.c>=nv)ExitProcess(5);}
 }if(at!=size)ExitProcess(5);
 file=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(file==INVALID_HANDLE_VALUE||!Write(file,&count,4))ExitProcess(6);at=4;
 for(unsigned int c=0;c<count;c++){unsigned int nv=*(unsigned int*)(data+at),nt=*(unsigned int*)(data+at+4);float mass=*(float*)(data+at+8);at+=12;memcpy(points,data+at,nv*16);at+=nv*16;memcpy(triangles,data+at,nt*12);at+=nt*12;
  NativeHullMassProperties::Result result;memset(&result,0x5a,sizeof(result));unsigned int status=NativeHullMassProperties::Integrate(points,nv,triangles,nt,mass,&result);if(!Write(file,&status,4)||!Write(file,&result,sizeof(result)))ExitProcess(6);
 }CloseHandle(file);ExitProcess(0);
}
