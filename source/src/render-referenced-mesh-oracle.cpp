#include "WorldPacker.cpp"
#include "RenderPackedRanges.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
extern "C" __declspec(dllimport) HMODULE WINAPI LoadLibraryA(const char*);
extern "C" __declspec(dllimport) void* WINAPI GetProcAddress(HMODULE,const char*);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
#pragma comment(linker,"/alternatename:__imp__LoadLibraryA@4=__imp__LoadLibraryA")
#pragma comment(linker,"/alternatename:__imp__GetProcAddress@8=__imp__GetProcAddress")
// Only local bounded synthetic records are supplied to the original helpers.
struct Bytes {unsigned char* data;unsigned int count,capacity;void Put(const void* p,unsigned int n){if(n>capacity-count)ExitProcess(20);memcpy(data+count,p,n);count+=n;}void U32(unsigned int v){Put(&v,4);}void U16(unsigned int v){unsigned short s=(unsigned short)v;Put(&s,2);}void U8(unsigned int v){unsigned char b=(unsigned char)v;Put(&b,1);}};
struct Stream {void** table;Bytes* bytes;};
static void __fastcall Unexpected(void*,void*){ExitProcess(21);}
template<unsigned int Slot> static void __fastcall Trap(void*,void*){ExitProcess(100+Slot);}
static void __fastcall Write32(Stream* s,void*,unsigned int v){s->bytes->U32(v);}
static void __fastcall Write16(Stream* s,void*,unsigned int v){s->bytes->U16(v);}
static void __fastcall Write8(Stream* s,void*,unsigned int v){s->bytes->U8(v);}
static void __fastcall WriteVec(Stream* s,void*,const void* v){s->bytes->Put(v,12);}
static unsigned __int64 __fastcall Tell(Stream* s,void*){return s->bytes->count;}
struct Output {void** table;const char* allowed[4];unsigned int calls;};
static bool Same(const char* a,const char* b){if(!a||!b)return false;while(*a&&*a==*b){a++;b++;}return *a==*b;}
static void lstrcatA(char* a,const char* b){unsigned int n=(unsigned int)lstrlenA(a),m=(unsigned int)lstrlenA(b);if(n+m>=MAX_PATH)ExitProcess(23);memcpy(a+n,b,m+1);}
static bool __fastcall CheckOutput(Output* o,void*,const char* filename){for(unsigned int i=0;i<4;i++)if(o->allowed[i]&&Same(o->allowed[i],filename)){o->calls++;return true;}ExitProcess(22);return false;}
static bool __fastcall Log(Output*,void*,unsigned int,const char*){return true;}
static bool Save(const char* path,const Bytes& b){HANDLE f=CreateFileA(path,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);DWORD n;bool ok=f!=INVALID_HANDLE_VALUE&&WriteFile(f,b.data,b.count,&n,0)&&n==b.count;if(f!=INVALID_HANDLE_VALUE)CloseHandle(f);return ok;}
static bool External(void* target,const char* path,const char* display,void* meshes){unsigned int result;
 __asm {
  push meshes
  push display
  mov eax,path
  call target
  add esp,8
  movzx eax,al
  mov result,eax
 }
 return result!=0;
}
struct Pair {void* node;void* section;};
static RenderVertexPacking::Element decl0[]={{0,0,2,0,0,0},{0,12,2,0,3,0},{255,0,17,0,0,0}};
static RenderVertexPacking::Element decl1[]={{0,0,2,0,0,0},{0,12,2,0,3,0},{0,24,1,0,5,0},{255,0,17,0,0,0}};
static const char* materials[]={"mat\\Z","MAT\\shared","mat\\a","mat\\Shared","mat\\z","mat\\B"};
extern "C" void HarnessEntry(){
 char folder[MAX_PATH],caseText[16];if(!GetEnvironmentVariableA("FEAR_REFERENCE_FOLDER",folder,MAX_PATH)||!GetEnvironmentVariableA("FEAR_REFERENCE_CASE",caseText,16))ExitProcess(1);
 const char* bound="C:/Users/user/Documents/Codex/2026-10-09/ckeif/outputs/worldpacker-rebuild/render-verification/referenced-mesh/";unsigned int k=0;while(bound[k]&&bound[k]==folder[k])k++;if(bound[k])ExitProcess(2);unsigned int which=0;for(unsigned int i=0;caseText[i];i++){if(caseText[i]<'0'||caseText[i]>'9'||i>1)ExitProcess(2);which=which*10+(unsigned int)(caseText[i]-'0');}if(which>12)ExitProcess(2);
 unsigned int sizes[3]={2,1,0},treeCount=2,main=0;const char* regionName[3]={};unsigned int claims[3][4]={},claimCounts[3]={},regionCount=0;
 if(which==1){regionCount=1;regionName[0]="Alpha";claimCounts[0]=1;claims[0][0]=0;}
 if(which==2){regionCount=2;regionName[0]="Alpha";regionName[1]="Beta";claimCounts[0]=claimCounts[1]=1;claims[0][0]=1;claims[1][0]=0;}
 if(which==3){regionCount=2;regionName[0]="All";regionName[1]="Duplicate";claimCounts[0]=2;claimCounts[1]=1;claims[0][0]=0;claims[0][1]=1;claims[1][0]=1;}
 if(which==4){sizes[0]=2;sizes[1]=3;sizes[2]=1;treeCount=3;main=1;regionCount=1;regionName[0]="Alpha";claimCounts[0]=1;claims[0][0]=1;}
 if(which==5||which==6){sizes[0]=which==5?2:1;treeCount=1;regionCount=1;regionName[0]="All";claimCounts[0]=sizes[0];claims[0][0]=which==5?1:0;claims[0][1]=0;}
 if(which==7){sizes[0]=1;sizes[1]=2;sizes[2]=2;treeCount=3;main=2;regionCount=1;regionName[0]="Alpha";claimCounts[0]=1;claims[0][0]=0;}
 if(which==8){sizes[0]=3;treeCount=1;regionCount=3;regionName[0]="Skipped";regionName[1]="Alpha";regionName[2]="Beta";claimCounts[1]=3;claims[1][0]=0;claims[1][1]=0;claims[1][2]=2;claimCounts[2]=2;claims[2][0]=2;claims[2][1]=1;}
 if(which>=10){regionCount=1;regionName[0]=which==10?"Alpha.One":"Alpha";claimCounts[0]=1;claims[0][0]=0;}
 unsigned int nodeCount=0;for(unsigned int t=0;t<treeCount;t++)nodeCount+=sizes[t];PrefabScene::Arena memory;
 unsigned char* nodes=(unsigned char*)memory.Allocate(nodeCount*0x74);unsigned char* sections=(unsigned char*)memory.Allocate(nodeCount*40);unsigned char* shaders=(unsigned char*)memory.Allocate(72);RenderAttributes::Vertex* vertices=(RenderAttributes::Vertex*)memory.Allocate(nodeCount*5*84);unsigned int* indices=(unsigned int*)memory.Allocate(nodeCount*12);unsigned int* nodePtrs=(unsigned int*)memory.Allocate(nodeCount*4);Pair* pairs=(Pair*)memory.Allocate(nodeCount*8);if(!nodes||!sections||!shaders||!vertices||!indices||!nodePtrs||!pairs)ExitProcess(3);memset(nodes,0,nodeCount*0x74);memset(sections,0,nodeCount*40);memset(shaders,0,72);memset(vertices,0,nodeCount*5*84);
 *(void**)(shaders+24)=decl0;*(void**)(shaders+28)=*(void**)(shaders+32)=decl0+3;*(void**)(shaders+36+24)=decl1;*(void**)(shaders+36+28)=*(void**)(shaders+36+32)=decl1+4;
 WorldText::Vec3 skyVertices[4]={{1,2,3},{4,5,6},{7,8,9},{10,11,12}};unsigned char sky[60]={};*(void**)(sky+4)=skyVertices;*(void**)(sky+8)=*(void**)(sky+12)=skyVertices+4;
 for(unsigned int n=0;n<nodeCount;n++){
  unsigned char* node=nodes+n*0x74;unsigned char* section=sections+n*40;RenderAttributes::Vertex* verts=vertices+n*5;unsigned int nv=3+n%3;nodePtrs[n]=(unsigned int)node;indices[n*3]=0;indices[n*3+1]=nv/2;indices[n*3+2]=nv-1;
  for(unsigned int v=0;v<nv;v++){RenderAttributes::Vertex& value=verts[v];value.position.x=(float)n+.25f;value.position.y=(float)v-.5f;value.position.z=(float)(n+v)+.75f;value.uv[0]=.25f;value.uv[1]=.5f;value.color=0xd3ffffff;value.normal.z=1;value.tangent.x=1;value.binormal.y=1;}
  bool empty=which==6;*(void**)(node+4)=empty?0:section;*(void**)(node+8)=*(void**)(node+12)=empty?0:section+40;*(void**)(node+0x14)=indices+n*3;*(void**)(node+0x18)=*(void**)(node+0x1c)=indices+n*3+(empty?0:3);*(void**)(node+0x24)=verts;*(void**)(node+0x28)=*(void**)(node+0x2c)=verts+nv;
  *(const char**)section=materials[n%6];*(void**)(section+4)=shaders+(n%2)*36;*(unsigned int*)(section+8)=n;*(unsigned int*)(section+12)=*(unsigned int*)(section+16)=0xffffffffu;*(unsigned int*)(section+20)=n%4;*(unsigned int*)(section+24)=1;*(unsigned int*)(section+36)=nv;pairs[n].node=node;pairs[n].section=section;
  if(n==0){*(void**)(node+0x58)=sky;*(void**)(node+0x5c)=*(void**)(node+0x60)=sky+60;}
 }
 unsigned int treeRecords[15]={},treePtrs[3]={},at=0;for(unsigned int t=0;t<treeCount;t++){unsigned int* tree=treeRecords+t*5;treePtrs[t]=(unsigned int)tree;tree[1]=(unsigned int)(nodePtrs+at);at+=sizes[t];tree[2]=tree[3]=(unsigned int)(nodePtrs+at);}unsigned int world[5]={0,(unsigned int)treePtrs,(unsigned int)(treePtrs+treeCount),(unsigned int)(treePtrs+treeCount),main};
 unsigned char sectors[3*112]={},regions[3*112]={};unsigned int sectorPtrs[3][4]={},regionPtrs[3]={};for(unsigned int s=0;s<3;s++)*(unsigned int*)(sectors+s*112+0x6c)=s;
 char base[MAX_PATH],worldOutput[MAX_PATH],meshPaths[3][MAX_PATH];lstrcpyA(base,folder);lstrcatA(base,which==11?"/Fixture.v2.World00p":"/Fixture.World00p");lstrcpyA(worldOutput,folder);lstrcatA(worldOutput,"/oracle.world.bin");
 void* outTable[8];for(unsigned int i=0;i<8;i++)outTable[i]=(void*)Unexpected;outTable[5]=(void*)CheckOutput;outTable[6]=(void*)Log;Output output={outTable,{0,0,0,0},0};
 Bytes bytes={(unsigned char*)memory.Allocate(1024*1024),0,1024*1024};void* table[25]={(void*)Trap<0>,(void*)Trap<1>,(void*)Trap<2>,(void*)Trap<3>,(void*)Trap<4>,(void*)Trap<5>,(void*)Trap<6>,(void*)Trap<7>,(void*)Trap<8>,(void*)Trap<9>,(void*)Trap<10>,(void*)Trap<11>,(void*)Trap<12>,(void*)Trap<13>,(void*)Trap<14>,(void*)Trap<15>,(void*)Trap<16>,(void*)Trap<17>,(void*)Trap<18>,(void*)Trap<19>,(void*)Trap<20>,(void*)Trap<21>,(void*)Trap<22>,(void*)Trap<23>,(void*)Trap<24>};table[4]=(void*)Tell;table[11]=(void*)WriteVec;table[18]=table[19]=(void*)Write32;table[20]=(void*)Write16;table[22]=table[23]=(void*)Write8;Stream stream={table,&bytes};
 HMODULE kernel=LoadLibraryA("kernel32.dll");typedef unsigned int(WINAPI* Mode)(unsigned int);Mode errorMode=kernel?(Mode)GetProcAddress(kernel,"SetErrorMode"):0;if(!errorMode)ExitProcess(4);errorMode(3);
 LoadLibraryA("E:/fearsdk/Dev/Tools/msvcr71.dll");LoadLibraryA("E:/fearsdk/Dev/Tools/msvcp71.dll");LoadLibraryA("E:/fearsdk/Dev/Tools/MFC71.dll");unsigned char* dll=(unsigned char*)LoadLibraryA("E:/fearsdk/Dev/Tools/Packers/WorldPacker.dll");if(!dll||dll[0x50440]!=0x6a||dll[0x4f1c0]!=0x83||dll[0x53580]!=0x8b||dll[0x27530]!=0x53)ExitProcess(5);
 table[24]=dll+0x27530;
 typedef void(__cdecl* SetOutput)(void*);SetOutput setOutput=(SetOutput)(dll+0x53580);setOutput(&output);
 typedef void(__thiscall* StringConstruct)(void*,const char*);typedef void(__thiscall* StringDestroy)(void*);StringConstruct stringConstruct=*(StringConstruct*)(dll+0x88188);StringDestroy stringDestroy=*(StringDestroy*)(dll+0x88194);
 for(unsigned int r=0;r<regionCount;r++){unsigned char* region=regions+r*112;regionPtrs[r]=(unsigned int)region;*(void**)(region+0x20)=sectorPtrs[r];*(unsigned int*)(region+0x24)=claimCounts[r];for(unsigned int i=0;i<claimCounts[r];i++)sectorPtrs[r][i]=(unsigned int)(sectors+claims[r][i]*112);stringConstruct(region+0x50,regionName[r]);lstrcpyA(meshPaths[r],folder);lstrcatA(meshPaths[r],which==11?"/Fixture":"/Fixture_");if(which!=11)lstrcatA(meshPaths[r],which==10?"Alpha":regionName[r]);lstrcatA(meshPaths[r],".Mesh00p");output.allowed[r]=meshPaths[r];}
 if(which==9){
  // The container accepts several mesh records. Metadata is overwritten by
  // each shared writer call; this distinguishes that quirk from summation.
  unsigned char meshes[96]={};*(void**)(meshes+0x24)=pairs;*(void**)(meshes+0x28)=*(void**)(meshes+0x2c)=pairs+2;*(void**)(meshes+48+0x24)=pairs+2;*(void**)(meshes+48+0x28)=*(void**)(meshes+48+0x2c)=pairs+3;unsigned int meshPtrs[2]={(unsigned int)meshes,(unsigned int)(meshes+48)},meshVector[4]={0,(unsigned int)meshPtrs,(unsigned int)(meshPtrs+2),(unsigned int)(meshPtrs+2)};
  lstrcpyA(meshPaths[0],folder);lstrcatA(meshPaths[0],"/Direct.Mesh00p");output.allowed[0]=meshPaths[0];if(!External(dll+0x4f1c0,meshPaths[0],"Direct",meshVector))ExitProcess(6);
 }else{
  unsigned int list[3]={(unsigned int)regionPtrs,regionCount,regionCount};unsigned char project[84]={}; // Empty root strips no part of the absolute fixture name.
  char projectRoot[MAX_PATH];if(which==12){lstrcpyA(projectRoot,folder);lstrcatA(projectRoot,"/");*(const char**)(project+0x3c)=projectRoot;*(unsigned int*)(project+0x4c)=(unsigned int)lstrlenA(projectRoot);*(unsigned int*)(project+0x50)=(unsigned int)lstrlenA(projectRoot);}
  typedef bool(__thiscall* Writer)(void*,void*,void*,const char*,void*);if(!((Writer)(dll+0x50440))(world,&stream,regionCount?list:0,base,project))ExitProcess(6);if(!Save(worldOutput,bytes))ExitProcess(7);
 }
 for(unsigned int r=0;r<regionCount;r++)stringDestroy(regions+r*112+0x50);setOutput(0);
 // Prove the original common string method separately beyond byte length 255.
 bytes.count=0;char longName[514];for(unsigned int i=0;i<513;i++)longName[i]=(char)('A'+i%26);longName[513]=0;typedef void(__thiscall* StringWrite)(void*,const char*);((StringWrite)(dll+0x27530))(&stream,longName);lstrcpyA(worldOutput,folder);lstrcatA(worldOutput,"/oracle.string.bin");if(!Save(worldOutput,bytes))ExitProcess(8);
 ExitProcess(0);
}
