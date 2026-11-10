// Research-only direct writer, bounded caller-owned polygon inputs and stream.
#include "OriginalPhysicsOracle.h"
struct MeshStream {void** table;unsigned char* data;unsigned int size,capacity;};
static unsigned char* moduleBase=0;static unsigned int currentCase=0;static MeshStream* currentStream=0;
static void __cdecl Fault(unsigned int slot,unsigned int caller){using namespace OriginalPhysicsOracle;char path[MAX_PATH];if(GetEnvironmentVariableA("FEAR_HULL_OUTPUT",path,MAX_PATH)&&lstrlenA(path)<MAX_PATH-7){lstrcpyA(path+lstrlenA(path),".fault");HANDLE f=CreateFileA(path,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(f!=INVALID_HANDLE_VALUE){unsigned int fields[6]={currentCase,slot,caller-(unsigned int)moduleBase,allocationCalls,allocationBytes,currentStream?currentStream->size:0};Write(f,fields,sizeof(fields));CloseHandle(f);}}ExitProcess(21);}
template<unsigned int Slot> static __declspec(naked) void TrapSite(){__asm {push dword ptr [esp]
 mov eax,Slot
 push eax
 call Fault
 int 3
 }}
static void Put(MeshStream* s,const void* p,unsigned int n){if(!s||n>s->capacity-s->size||(!p&&n))ExitProcess(24);memcpy(s->data+s->size,p,n);s->size+=n;}
static void __fastcall Word(MeshStream* s,void*,unsigned int v){Put(s,&v,4);}
static void __fastcall Half(MeshStream* s,void*,unsigned int v){unsigned short h=(unsigned short)v;Put(s,&h,2);}
static void __fastcall Vector(MeshStream* s,void*,const void* p){Put(s,p,12);}
static void __fastcall Pose(MeshStream* s,void*,const void* p){Put(s,p,28);}
static void __fastcall Plane(MeshStream* s,void*,const void* p){Put(s,p,16);}
static bool __fastcall Raw(MeshStream* s,void*,const void* p,unsigned int n){Put(s,p,n);return true;}
static bool Invoke(void* target,void* world,MeshStream* stream,unsigned int calculate,float mass,float density,unsigned int wrapper){unsigned int result;__asm {mov ebx,stream
 push wrapper
 push density
 push mass
 push calculate
 push world
 call target
 add esp,20
 movzx eax,al
 mov result,eax
 }return result!=0;}
static bool PolygonsBounded(HANDLE file,unsigned char* world,unsigned int count,PrefabScene::Arena& memory){using namespace OriginalPhysicsOracle;unsigned char* first=0,*last=0;unsigned int references=0,triangles=0;WorldText::Vec3 unique[32];unsigned int uniqueCount=0;
 if(count>32)return false;
 for(unsigned int p=0;p<count;p++){
  unsigned int h[2];if(!Read(file,h,8)||h[1]>30||h[1]>384-references)return false;unsigned int faces=h[1]>=3?h[1]-2:0;if(faces>128-triangles)return false;triangles+=faces;
  unsigned char* polygon=(unsigned char*)memory.Allocate(0x68+h[1]*0x2c);unsigned char* surface=(unsigned char*)memory.Allocate(0xb0);if(!polygon||!surface)return false;*(void**)(polygon+0x38)=surface;*(unsigned int*)(surface+0xac)=h[0];*(unsigned short*)(polygon+0x60)=(unsigned short)h[1];
  for(unsigned int v=0;v<h[1];v++){
   WorldText::Vec3 point;if(!Read(file,&point,12)||!WorldText::FiniteVector(point))return false;bool known=false;for(unsigned int k=0;k<uniqueCount;k++)if(point.x==unique[k].x&&point.y==unique[k].y&&point.z==unique[k].z){known=true;break;}if(!known){if(uniqueCount==32)return false;unique[uniqueCount++]=point;}memcpy(polygon+0x68+v*0x2c,&point,12);
  }
  references+=h[1];if(!first)first=polygon;if(last)*(void**)(last+8)=polygon+4;last=polygon;
 }
 if(first){*(void**)(last+8)=first+4;*(void**)(world+0x24)=first+4;}*(unsigned int*)(world+0x54)=references;return true;
}
extern "C" void HarnessEntry(){using namespace OriginalPhysicsOracle;
 char input[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_HULL_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_HULL_OUTPUT",output,MAX_PATH))ExitProcess(1);
 moduleBase=Load();Install(moduleBase);const unsigned char signature[]={0x55,0x8b,0xec,0x83,0xe4,0xf0,0x6a,0xff};for(unsigned int i=0;i<sizeof(signature);i++)if(moduleBase[0x3c790+i]!=signature[i])ExitProcess(3);
 static void* allocatorTable[12]={(void*)Allocate,(void*)Release,(void*)TrapSite<102>,(void*)TrapSite<103>,(void*)Allocate,(void*)ReleaseChunk,(void*)TrapSite<106>,(void*)TrapSite<107>,(void*)TrapSite<108>,(void*)TrapSite<109>,(void*)StackRelease,(void*)TrapSite<111>};allocatorObject[0]=(unsigned int)allocatorTable;
 HANDLE file=CreateFileA(input,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);if(file==INVALID_HANDLE_VALUE||GetFileSize(file,0)>1024*1024)ExitProcess(4);unsigned int count=0;if(!Read(file,&count,4)||!count||count>16)ExitProcess(4);
 HANDLE out=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(out==INVALID_HANDLE_VALUE||!Write(out,&count,4))ExitProcess(5);
 for(unsigned int c=0;c<count;c++){
  currentCase=c;PrefabScene::Arena memory;Begin(memory);unsigned int h[9];if(!Read(file,h,sizeof(h))||h[1]>1||h[2]>1||h[3]>1)ExitProcess(6);for(unsigned int i=4;i<9;i++)if(!WorldText::Finite(((float*)h)[i]))ExitProcess(6);
  unsigned char world[0xc0]={};*(unsigned int*)world=4;memcpy(world+4,h+4,8);*(unsigned int*)(world+0xb4)=h[1];lstrcpyA((char*)world+0x64,"bounded-owner-mesh");memcpy(world+0xa8,h+6,12);if(!PolygonsBounded(file,world,h[0],memory))ExitProcess(6);
  void* table[24]={(void*)TrapSite<0>,(void*)TrapSite<1>,(void*)TrapSite<2>,(void*)TrapSite<3>,(void*)TrapSite<4>,(void*)Raw,(void*)TrapSite<6>,(void*)TrapSite<7>,(void*)Pose,(void*)Plane,(void*)Vector,(void*)Vector,(void*)TrapSite<12>,(void*)TrapSite<13>,(void*)Word,(void*)TrapSite<15>,(void*)TrapSite<16>,(void*)TrapSite<17>,(void*)Word,(void*)TrapSite<19>,(void*)Half,(void*)TrapSite<21>,(void*)TrapSite<22>,(void*)TrapSite<23>};
  MeshStream stream={table,(unsigned char*)memory.Allocate(1024*1024),0,1024*1024};currentStream=&stream;if(!stream.data)ExitProcess(23);
  unsigned int status=Invoke(moduleBase+0x3c790,world,&stream,h[2],((float*)h)[4],((float*)h)[5],h[3])?1u:0u;
  unsigned int header[4]={status,stream.size,allocationCalls,allocationBytes};if(!Write(out,header,16)||!Write(out,stream.data,stream.size))ExitProcess(7);currentStream=0;
 }
 unsigned char extra;DWORD done=0;if(!ReadFile(file,&extra,1,&done,0)||done)ExitProcess(6);CloseHandle(file);CloseHandle(out);ExitProcess(0);
}
