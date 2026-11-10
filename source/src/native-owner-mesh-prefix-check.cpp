#include "WorldPacker.cpp"
#include "NativePhysicsOwnerMeshPrefix.h"
#include "PhysicsMeshBlock.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
namespace MP=NativePhysicsOwnerMeshPrefix;
typedef SourcePhysicsOwnerVertices::Polygon Polygon;
static bool Write(HANDLE f,const void* p,unsigned int n){DWORD done=0;return !n||(WriteFile(f,p,n,&done,0)&&done==n);}
static bool Read(HANDLE f,void* p,unsigned int n){DWORD done=0;return !n||(ReadFile(f,p,n,&done,0)&&done==n);}
static bool Same(const void* a,const void* b,unsigned int n){const unsigned char* x=(const unsigned char*)a;const unsigned char* y=(const unsigned char*)b;for(unsigned int i=0;i<n;i++)if(x[i]!=y[i])return false;return true;}
static bool Near(float a,float b,float tolerance){float d=a-b;return d>=-tolerance&&d<=tolerance;}
static bool Cleared(const MP::Builder& r){if(r.data||r.size||r.pointCount||r.indexCount||r.triangleCount||r.massVertexCount||r.massTriangleCount||r.convexAttempts||r.mass||r.density||r.scaledVolume||r.mainWorld||r.convexValidated||r.Prefix().data||r.Prefix().size)return false;const unsigned char* p=(const unsigned char*)&r.properties;for(unsigned int i=0;i<sizeof(r.properties);i++)if(p[i])return false;return true;}
static unsigned int Guards(unsigned int* checks){
 *checks=0;WorldText::Vec3 zero={0,0,0};
 WorldText::Vec3 points[4]={{0,0,0},{100,0,0},{0,100,0},{0,0,100}};
 WorldText::Vec3 faces[4][3]={{points[0],points[2],points[1]},{points[0],points[1],points[3]},{points[0],points[3],points[2]},{points[1],points[2],points[3]}};
 Polygon polygons[4]={};for(unsigned int i=0;i<4;i++){polygons[i].points=faces[i];polygons[i].count=3;polygons[i].flags=0x10;}
 MP::Builder record;if(!record.BuildPrepared(polygons,4,zero,false,1,1.1f))return 1;unsigned char original[512];unsigned int originalSize=record.size;if(originalSize>sizeof(original))return 1;memcpy(original,record.data,originalSize);(*checks)++;
#define BAD(expression) do {if((expression)||!record.error||!Cleared(record))return *checks+1;(*checks)++;} while(0)
 BAD(record.BuildPrepared(0,4,zero,false,1,1));BAD(record.BuildPrepared(0,0,zero,false,1,1));
 MP::Settings settings;settings.version=2;BAD(record.BuildPrepared(polygons,4,zero,false,1,1,settings));settings=MP::Settings();settings.maximumBytes=108;BAD(record.BuildPrepared(polygons,4,zero,false,1,1,settings));
 settings=MP::Settings();settings.collector.maximumUniqueVertices=3;BAD(record.BuildPrepared(polygons,4,zero,false,1,1,settings));settings=MP::Settings();settings.collector.maximumReferences=11;BAD(record.BuildPrepared(polygons,4,zero,false,1,1,settings));
 union Bits {unsigned int u;float f;} bits;bits.u=0x7fc00000u;BAD(record.BuildPrepared(polygons,4,zero,false,bits.f,1));BAD(record.BuildPrepared(polygons,4,zero,false,1,bits.f));
 WorldText::Vec3 invalidCenter=zero;invalidCenter.x=bits.f;BAD(record.BuildPrepared(polygons,4,invalidCenter,false,1,1));
 for(unsigned int k=0;k<3;k++){float saved=(&faces[0][0].x)[k];(&faces[0][0].x)[k]=bits.f;BAD(record.BuildPrepared(polygons,4,zero,false,1,1));(&faces[0][0].x)[k]=saved;}
 for(unsigned int p=0;p<4;p++)polygons[p].flags=8;BAD(record.BuildPrepared(polygons,4,zero,false,1,1));for(unsigned int p=0;p<4;p++)polygons[p].flags=0x10;
 for(unsigned int p=0;p<4;p++)for(unsigned int v=0;v<3;v++)faces[p][v].z=0;BAD(record.BuildPrepared(polygons,4,zero,false,1,1));
 faces[0][0]=points[0];faces[0][1]=points[2];faces[0][2]=points[1];faces[1][0]=points[0];faces[1][1]=points[1];faces[1][2]=points[3];faces[2][0]=points[0];faces[2][1]=points[3];faces[2][2]=points[2];faces[3][0]=points[1];faces[3][1]=points[2];faces[3][2]=points[3];
#undef BAD
 if(!record.BuildPrepared(polygons,4,zero,false,1,1.1f)||record.size!=originalSize||!Same(record.data,original,originalSize))return *checks+1;(*checks)++;
 faces[0][0].x=99;if(!Same(record.data,original,originalSize))return *checks+1;(*checks)++;faces[0][0].x=0;
 if(!record.BuildPrepared(polygons,4,zero,true,-3,-1)||record.mass!=.1f||record.density!=.01f||record.massVertexCount||record.convexAttempts)return *checks+1;
 for(unsigned int i=2;i<15;i++)if(((float*)(record.data+36))[i]!=0)return *checks+1;(*checks)++;
 unsigned char mopp[21]={};float info[4]={0,0,0,1};memcpy(mopp,info,16);*(unsigned int*)(mopp+16)=1;mopp[20]=42;PhysicsMeshBlock::Block complete;if(!complete.Build(record.Prefix(),mopp,21)||complete.size!=record.size+21)return *checks+1;(*checks)++;
 // Concave L prism: the fan mesh retains its reflex corner and all 20 fan
 // triangles; only dynamic mass uses the convex cloud (volume 3.5, not 3).
 WorldText::Vec3 l[12]={{0,0,0},{200,0,0},{200,100,0},{100,100,0},{100,200,0},{0,200,0},{0,0,100},{200,0,100},{200,100,100},{100,100,100},{100,200,100},{0,200,100}};
 WorldText::Vec3 lf[8][6]={};Polygon concave[8]={};
 for(unsigned int i=0;i<6;i++){lf[0][i]=l[5-i];lf[1][i]=l[6+i];unsigned int n=(i+1)%6;lf[2+i][0]=l[i];lf[2+i][1]=l[n];lf[2+i][2]=l[6+n];lf[2+i][3]=l[6+i];}
 for(unsigned int i=0;i<8;i++){concave[i].points=lf[i];concave[i].count=i<2?6u:4u;concave[i].flags=0x10;}
 SourcePhysicsOwnerVertices::Settings cs;cs.generateIndices=true;SourcePhysicsOwnerVertices::Builder collected;if(!collected.BuildPrepared(concave,8,zero,false,cs)||!record.BuildPrepared(concave,8,zero,false,12,1))return *checks+1;
 if(record.pointCount!=12||record.indexCount!=60||record.massVertexCount>=record.pointCount||!Near(record.scaledVolume,3500000,10)||!Same(record.data+108,collected.scaledVertices,record.pointCount*12))return *checks+1;
 for(unsigned int i=0;i<record.indexCount;i++)if(((unsigned short*)(record.data+108+record.pointCount*12))[i]!=collected.indices[i])return *checks+1;(*checks)++;
 // A unit tetrahedron has COM .25 and diagonal mass*3/40, offdiagonal mass/80.
 if(!record.BuildPrepared(polygons,4,zero,false,8,1)||!Near(record.properties.center.x,.25f,.00001f)||!Near(record.properties.center.y,.25f,.00001f)||!Near(record.properties.center.z,.25f,.00001f)||!Near(record.properties.inertia[0],.6f,.00001f)||!Near(record.properties.inertia[1],.1f,.00001f)||!Near(record.scaledVolume,1000000.f/6,1))return *checks+1;(*checks)++;
 // A static surface need not have a volume; a dynamic one must fit the
 // separately declared convex mass profile instead of inventing a solid.
 if(!record.BuildPrepared(polygons,1,zero,true,1,1)||record.pointCount!=3||record.indexCount!=3)return *checks+1;(*checks)++;
 if(record.BuildPrepared(polygons,1,zero,false,1,1)||record.errorCode!=MP::ResourceLimit||!Cleared(record))return *checks+1;(*checks)++;
 // The u16 disk mesh profile is wider than the current dynamic mass profile.
 // Static meshes retain all positions; dynamic over-limit clouds clear output.
 PrefabScene::Arena largeMemory;const unsigned int largeCount=NativeHullConvex::MaximumPoints+1,largePolygons=(largeCount+29)/30;
 WorldText::Vec3* many=(WorldText::Vec3*)largeMemory.Allocate(largeCount*12);Polygon* manyPolygons=(Polygon*)largeMemory.Allocate(largePolygons*sizeof(Polygon));if(!many||!manyPolygons)return *checks+1;
 for(unsigned int i=0;i<largeCount;i++){many[i].x=(float)i*2;many[i].y=(float)(i%17)*3;many[i].z=(float)(i%31)*4;}
 for(unsigned int i=0;i<largePolygons;i++){manyPolygons[i]=polygons[0];manyPolygons[i].points=many+i*30;manyPolygons[i].count=i+1==largePolygons?largeCount-i*30:30;}
 if(!record.BuildPrepared(manyPolygons,largePolygons,zero,true,1,1)||record.pointCount!=largeCount)return *checks+1;(*checks)++;
 if(record.BuildPrepared(manyPolygons,largePolygons,zero,false,1,1)||record.errorCode!=MP::ResourceLimit||!Cleared(record))return *checks+1;(*checks)++;
 return 0;
}
extern "C" void HarnessEntry(){
 unsigned int checks=0,failure=Guards(&checks);char guardPath[MAX_PATH];DWORD ng=GetEnvironmentVariableA("FEAR_OWNER_MESH_PREFIX_GUARDS",guardPath,MAX_PATH);if(ng&&ng<MAX_PATH){unsigned int report[2]={checks,failure};HANDLE guard=CreateFileA(guardPath,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(guard==INVALID_HANDLE_VALUE||!Write(guard,report,sizeof(report)))ExitProcess(6);CloseHandle(guard);}if(failure)ExitProcess(7);
 char input[MAX_PATH],output[MAX_PATH];DWORD ni=GetEnvironmentVariableA("FEAR_OWNER_MESH_PREFIX_INPUT",input,MAX_PATH),no=GetEnvironmentVariableA("FEAR_OWNER_MESH_PREFIX_OUTPUT",output,MAX_PATH);if(!ni||ni>=MAX_PATH||!no||no>=MAX_PATH)ExitProcess(1);
 HANDLE source=CreateFileA(input,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);if(source==INVALID_HANDLE_VALUE||GetFileSize(source,0)>16u*1024u*1024u)ExitProcess(4);unsigned int count=0;if(!Read(source,&count,4)||!count||count>2000)ExitProcess(5);
 HANDLE out=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(out==INVALID_HANDLE_VALUE||!Write(out,&count,4))ExitProcess(6);
 for(unsigned int c=0;c<count;c++){
  MP::Builder record;unsigned int h[9];unsigned int calculateMass=0;
  {
   PrefabScene::Arena memory;if(!Read(source,h,36)||h[0]>1000||h[1]>1||h[2]>1||h[3]>1)ExitProcess(5);calculateMass=h[2];if(calculateMass==h[1])ExitProcess(8);
   Polygon* polygons=(Polygon*)memory.Allocate((h[0]+1)*sizeof(Polygon));if(!polygons)ExitProcess(6);
   for(unsigned int p=0;p<h[0];p++){unsigned int ph[2];if(!Read(source,ph,8)||ph[1]>512)ExitProcess(5);WorldText::Vec3* points=(WorldText::Vec3*)memory.Allocate((ph[1]+1)*12);if(!points||!Read(source,points,ph[1]*12))ExitProcess(5);memset(polygons+p,0,sizeof(*polygons));polygons[p].points=points;polygons[p].count=ph[1];polygons[p].flags=ph[0];}
   WorldText::Vec3 center;float mass,density;memcpy(&center,h+6,12);memcpy(&mass,h+4,4);memcpy(&density,h+5,4);record.BuildPrepared(polygons,h[0],center,h[1]!=0,mass,density);
  }
  unsigned int metadata[10]={(unsigned int)record.errorCode,record.size,(unsigned int)record.convexStatus,(unsigned int)record.integrationStatus,record.pointCount,record.indexCount,record.massVertexCount,record.massTriangleCount,record.convexValidated?1u:0u,record.convexAttempts};
  if(!Write(out,metadata,sizeof(metadata))||!Write(out,record.data,record.size))ExitProcess(6);
 }
 unsigned char extra;DWORD done=0;if(!ReadFile(source,&extra,1,&done,0)||done)ExitProcess(5);CloseHandle(source);CloseHandle(out);ExitProcess(0);
}
