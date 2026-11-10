#include "WorldPacker.cpp"
#include "SourcePhysicsOwnerVertices.h"
#include "NativePhysicsHullRecord.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
static bool Write(HANDLE f,const void* p,unsigned int n){DWORD done=0;return !n||(WriteFile(f,p,n,&done,0)&&done==n);}
static bool Read(HANDLE f,void* p,unsigned int n){DWORD done=0;return !n||(ReadFile(f,p,n,&done,0)&&done==n);}
static bool Same(const void* a,const void* b,unsigned int n){const unsigned char* x=(const unsigned char*)a;const unsigned char* y=(const unsigned char*)b;for(unsigned int i=0;i<n;i++)if(x[i]!=y[i])return false;return true;}
static bool Cleared(const NativePhysicsHullRecord::Builder& r){if(r.data||r.size||r.vertexCount||r.triangleCount||r.planeCount||r.mass||r.density||r.scaledVolume||r.wrapper)return false;const unsigned char* p=(const unsigned char*)&r.properties;for(unsigned int i=0;i<sizeof(r.properties);i++)if(p[i])return false;return true;}
static unsigned int ApiGuards(unsigned int* checks){
 using namespace NativePhysicsHullRecord;
 Vec4 p[4]={{0,0,0,0},{1,0,0,0},{0,1,0,0},{0,0,1,0}};
 Triangle t[4]={{0,2,1},{0,1,3},{0,3,2},{1,2,3}};
 Vec4 planes[4]={{-1,0,0,0},{0,-1,0,0},{0,0,-1,0},{.57735026f,.57735026f,.57735026f,-.57735026f}};
 Input good;good.points=p;good.pointCount=4;good.triangles=t;good.triangleCount=4;good.planes=planes;good.planeCount=4;good.mass=2;
 Builder record;*checks=0;unsigned char saved[512];if(!record.Build(good)||record.size>sizeof(saved))return 1;unsigned int savedSize=record.size;memcpy(saved,record.data,savedSize);(*checks)++;
#define EXPECT_FAILURE(expected,expression) do {if((expression)||record.errorCode!=(expected)||!Cleared(record))return *checks+1;(*checks)++;} while(0)
 Input bad;EXPECT_FAILURE(EmptyGeometry,record.Build(bad));bad=good;bad.points=0;EXPECT_FAILURE(InvalidInput,record.Build(bad));bad=good;bad.triangles=0;EXPECT_FAILURE(InvalidInput,record.Build(bad));bad=good;bad.planes=0;EXPECT_FAILURE(InvalidInput,record.Build(bad));
 bad=good;bad.pointCount=0;EXPECT_FAILURE(InvalidInput,record.Build(bad));bad=good;bad.pointCount=100001;EXPECT_FAILURE(ResourceLimit,record.Build(bad));bad=good;bad.triangleCount=200001;EXPECT_FAILURE(ResourceLimit,record.Build(bad));bad=good;bad.planeCount=200001;EXPECT_FAILURE(ResourceLimit,record.Build(bad));
 Settings limits;limits.maximumBytes=104;EXPECT_FAILURE(ResourceLimit,record.Build(good,limits));limits=Settings();limits.maximumVertices=3;EXPECT_FAILURE(ResourceLimit,record.Build(good,limits));limits=Settings();limits.maximumVertices=1000001;EXPECT_FAILURE(InvalidSettings,record.Build(good,limits));
 union Bits {unsigned int u;float f;} bits;bits.u=0x7fc00000u;bad=good;bad.mass=bits.f;EXPECT_FAILURE(InvalidInput,record.Build(bad));bad=good;bad.density=bits.f;EXPECT_FAILURE(InvalidInput,record.Build(bad));
 for(unsigned int i=0;i<4;i++){((float*)p)[i]=bits.f;EXPECT_FAILURE(NonfiniteGeometry,record.Build(good));((float*)p)[i]=0;}
 for(unsigned int i=0;i<4;i++){float value=((float*)planes)[i];((float*)planes)[i]=bits.f;EXPECT_FAILURE(NonfiniteGeometry,record.Build(good));((float*)planes)[i]=value;}
 t[0].a=4;EXPECT_FAILURE(InvalidInput,record.Build(good));t[0].a=0;
 p[3].z=0;EXPECT_FAILURE(MassIntegrationFailed,record.Build(good));if(record.integrationStatus!=NativeHullMassProperties::NonfiniteResult)return *checks+1;p[3].z=1;
#undef EXPECT_FAILURE
 if(!record.Build(good)||record.size!=savedSize||!Same(record.data,saved,savedSize))return *checks+1;(*checks)++;
 p[0].x=17;t[0].a=3;planes[0].x=42;if(!Same(record.data,saved,savedSize))return *checks+1;(*checks)++;p[0].x=0;t[0].a=0;planes[0].x=-1;
 bad=good;bad.wrapper=true;if(!record.Build(bad)||record.size!=savedSize+4||*(unsigned int*)record.data!=5||!Same(record.data+4,saved,savedSize))return *checks+1;(*checks)++;
 bad=good;bad.mass=-3;bad.density=-1;if(!record.Build(bad)||record.mass!=.1f||record.density!=.01f)return *checks+1;(*checks)++;
 return 0;
}
extern "C" void HarnessEntry(){
 unsigned int checks=0,failure=ApiGuards(&checks);char guardPath[MAX_PATH];DWORD ng=GetEnvironmentVariableA("FEAR_HULL_RECORD_GUARD_OUTPUT",guardPath,MAX_PATH);if(ng&&ng<MAX_PATH){unsigned int report[2]={checks,failure};HANDLE guard=CreateFileA(guardPath,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(guard==INVALID_HANDLE_VALUE||!Write(guard,report,sizeof(report)))ExitProcess(6);CloseHandle(guard);}if(failure)ExitProcess(7);
 char input[MAX_PATH],output[MAX_PATH],mode[16];if(!GetEnvironmentVariableA("FEAR_HULL_RECORD_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_HULL_RECORD_OUTPUT",output,MAX_PATH))ExitProcess(1);
 bool prepare=GetEnvironmentVariableA("FEAR_HULL_RECORD_MODE",mode,16)&&mode[0]=='p';
 HANDLE file=CreateFileA(input,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);if(file==INVALID_HANDLE_VALUE||GetFileSize(file,0)>16u*1024u*1024u)ExitProcess(4);
 unsigned int count=0;if(!Read(file,&count,4)||!count||count>2000)ExitProcess(5);
 HANDLE out=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(out==INVALID_HANDLE_VALUE||!Write(out,&count,4))ExitProcess(6);
 for(unsigned int c=0;c<count;c++){
  PrefabScene::Arena memory;
  if(prepare){
   unsigned int h[7];if(!Read(file,h,28)||h[0]>1000||h[1]>1)ExitProcess(5);
   SourcePhysicsOwnerVertices::Polygon* polygons=(SourcePhysicsOwnerVertices::Polygon*)memory.Allocate((h[0]+1)*sizeof(SourcePhysicsOwnerVertices::Polygon));if(!polygons)ExitProcess(6);
   for(unsigned int p=0;p<h[0];p++){unsigned int ph[2];if(!Read(file,ph,8)||ph[1]>512)ExitProcess(5);WorldText::Vec3* points=(WorldText::Vec3*)memory.Allocate((ph[1]+1)*12);if(!points||!Read(file,points,ph[1]*12))ExitProcess(5);memset(polygons+p,0,sizeof(*polygons));polygons[p].points=points;polygons[p].count=ph[1];polygons[p].flags=ph[0];}
   WorldText::Vec3 center;memcpy(&center,h+4,12);SourcePhysicsOwnerVertices::Builder collector;
   unsigned int status=collector.BuildPrepared(polygons,h[0],center,false)?0u:1u;unsigned int head[2]={status,collector.pointCount};if(!Write(out,head,8)||!Write(out,collector.scaledVertices,collector.pointCount*12))ExitProcess(6);
  }else{
   unsigned int h[6];if(!Read(file,h,24)||h[0]>10000||h[1]>20000||h[2]>20000||h[3]>1)ExitProcess(5);
   NativePhysicsHullRecord::Vec4* points=(NativePhysicsHullRecord::Vec4*)memory.Allocate((h[0]+1)*16);NativePhysicsHullRecord::Triangle* triangles=(NativePhysicsHullRecord::Triangle*)memory.Allocate((h[1]+1)*12);NativePhysicsHullRecord::Vec4* planes=(NativePhysicsHullRecord::Vec4*)memory.Allocate((h[2]+1)*16);
   if(!points||!triangles||!planes||!Read(file,points,h[0]*16)||!Read(file,triangles,h[1]*12)||!Read(file,planes,h[2]*16))ExitProcess(5);
   NativePhysicsHullRecord::Input in;in.points=points;in.pointCount=h[0];in.triangles=triangles;in.triangleCount=h[1];in.planes=planes;in.planeCount=h[2];in.wrapper=h[3]!=0;memcpy(&in.mass,h+4,4);memcpy(&in.density,h+5,4);
   NativePhysicsHullRecord::Builder record;record.Build(in);unsigned int head[3]={(unsigned int)record.errorCode,record.size,(unsigned int)record.integrationStatus};if(!Write(out,head,12)||!Write(out,record.data,record.size))ExitProcess(6);
  }
 }
 unsigned char extra;DWORD done=0;if(!ReadFile(file,&extra,1,&done,0)||done)ExitProcess(5);CloseHandle(file);CloseHandle(out);ExitProcess(0);
}
