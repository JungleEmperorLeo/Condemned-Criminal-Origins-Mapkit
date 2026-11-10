#include "WorldPacker.cpp"
#include "FearBspSplitPolicy.h"
#ifdef FEAR_BSP_TREE_CHECK
#include "FearBspTree.h"
#endif
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
#ifdef FEAR_BSP_POLICY_ORACLE
extern "C" __declspec(dllimport) HMODULE WINAPI LoadLibraryA(const char*);
extern "C" __declspec(dllimport) void* WINAPI GetProcAddress(HMODULE,const char*);
#pragma comment(linker,"/alternatename:__imp__LoadLibraryA@4=__imp__LoadLibraryA")
#pragma comment(linker,"/alternatename:__imp__GetProcAddress@8=__imp__GetProcAddress")
static float OriginalWeight(void* target,unsigned int depth,void* selector){float result;__asm {
 push esi
 mov esi,selector
 mov eax,depth
 mov edx,target
 call edx
 fstp result
 pop esi
}return result;}
static unsigned int OriginalQuick(void* target,void* plane,void* polygon){unsigned int result;__asm {
 push esi
 mov esi,plane
 push polygon
 mov edx,target
 call edx
 add esp,4
 mov result,eax
 pop esi
}return result;}
static void* OriginalBest(void* target,void* lists,unsigned int depth){void* result;__asm {
 push depth
 push lists
 mov ecx,lists
 mov edx,target
 call edx
 mov result,eax
}return result;}
#endif
static HANDLE input,output;
static void Read(void* p,unsigned int size){DWORD done;if(!ReadFile(input,p,size,&done,0)||done!=size)ExitProcess(4);}
static void Write(const void* p,unsigned int size){DWORD done;if(!WriteFile(output,p,size,&done,0)||done!=size)ExitProcess(5);}
extern "C" void HarnessEntry(){
 char path[MAX_PATH],out[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_BSP_POLICY_INPUT",path,MAX_PATH)||!GetEnvironmentVariableA("FEAR_BSP_POLICY_OUTPUT",out,MAX_PATH))ExitProcess(1);
#ifdef FEAR_BSP_POLICY_ORACLE
 HMODULE kernel=LoadLibraryA("kernel32.dll");typedef DWORD (WINAPI *Mode)(DWORD);Mode mode=kernel?(Mode)GetProcAddress(kernel,"SetErrorMode"):0;if(mode)mode(3);
 LoadLibraryA("E:/fearsdk/Dev/Tools/msvcr71.dll");LoadLibraryA("E:/fearsdk/Dev/Tools/msvcp71.dll");LoadLibraryA("E:/fearsdk/Dev/Tools/MFC71.dll");
 unsigned char* dll=(unsigned char*)LoadLibraryA("E:/fearsdk/Dev/Tools/Packers/WorldPacker.dll");if(!dll)ExitProcess(2);
#endif
 input=CreateFileA(path,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);output=CreateFileA(out,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(input==INVALID_HANDLE_VALUE||output==INVALID_HANDLE_VALUE)ExitProcess(3);
 unsigned int weights,scenes;Read(&weights,4);Read(&scenes,4);if(weights>1000000||scenes>10000)ExitProcess(6);
 for(unsigned int i=0;i<weights;i++){
  unsigned int w[3];Read(w,12);float result;
#ifdef FEAR_BSP_POLICY_ORACLE
  unsigned int selector[7]={};selector[1]=w[0];selector[3]=w[1];result=OriginalWeight(dll+0x8500,w[2],selector);
#else
  result=FearBspSplitPolicy::Weight(w[0],w[1],w[2]);
#endif
  Write(&result,4);
 }
 for(unsigned int scene=0;scene<scenes;scene++){
  unsigned int count,depth;Read(&count,4);Read(&depth,4);if(!count||count>4096||depth>4096)ExitProcess(7);
  PrefabScene::Arena arena;
  FearBspSplitPolicy::Polygon* polys=(FearBspSplitPolicy::Polygon*)arena.Allocate(count*sizeof(FearBspSplitPolicy::Polygon));
  void** fake=(void**)arena.Allocate(count*sizeof(void*));void** sky=(void**)arena.Allocate(count*sizeof(void*));void** normal=(void**)arena.Allocate(count*sizeof(void*));unsigned int skyCount=0,normalCount=0;
  if(!polys||!fake||!sky||!normal)ExitProcess(8);
  for(unsigned int j=0;j<count;j++){
   float p[8];unsigned int flags,n;Read(p,32);Read(&flags,4);Read(&n,4);if(n<3||n>64)ExitProcess(9);
   WorldText::Vec3* pts=(WorldText::Vec3*)arena.Allocate(n*12);if(!pts)ExitProcess(8);Read(pts,n*12);
   FearBspSplitPolicy::Polygon& poly=polys[j];memcpy(&poly.normal,p,12);poly.distance=p[3];poly.center.x=p[4];poly.center.y=p[5];poly.center.z=p[6];poly.radius=p[7];poly.flags=flags;poly.count=n;poly.points=pts;
   unsigned char* original=(unsigned char*)arena.Allocate(0x68+n*44);unsigned char* surface=(unsigned char*)arena.Allocate(0xb0);unsigned char* plane=(unsigned char*)arena.Allocate(20);if(!original||!surface||!plane)ExitProcess(8);
   memcpy(plane+8,p,12);*(void**)(surface+0xa4)=plane;*(float*)(surface+0xa8)=p[3];*(unsigned int*)(surface+0xac)=flags;*(void**)(original+0x38)=surface;
   original[0x4c]=3;memcpy(original+0x50,p+4,16);*(unsigned short*)(original+0x60)=(unsigned short)n;
   for(unsigned int k=0;k<n;k++)memcpy(original+0x68+k*44,&pts[k],12);fake[j]=original;
   if(flags&2)sky[skyCount++]=original;else if(flags&8)normal[normalCount++]=original;
  }
  unsigned int chosen=0xffffffffu;
#ifdef FEAR_BSP_TREE_CHECK
  if(count>64)ExitProcess(11);
#ifdef FEAR_BSP_POLICY_ORACLE
  unsigned char treeLists[32]={};*(void***)(treeLists+4)=sky;*(void***)(treeLists+8)=sky+skyCount;*(void***)(treeLists+12)=sky+count;*(void***)(treeLists+20)=normal;*(void***)(treeLists+24)=normal+normalCount;*(void***)(treeLists+28)=normal+count;
  for(unsigned int j=0;j<count;j++){unsigned char* p=(unsigned char*)fake[j];p[0x4c]=0;for(unsigned int k=0;k<polys[j].count;k++)memcpy(p+0x68+k*44+28,&polys[j].normal,12);}
  unsigned char world[0x80]={},parent[0x100]={},gen[8]={},owned[8]={};*(void**)(world+0x1c)=parent;*(void**)gen=world;
  void* originalRoot=0;typedef void (__thiscall *Recurse)(void*,void*,void**,unsigned int,void*);((Recurse)(dll+0x9980))(gen,treeLists,&originalRoot,1,owned);
  unsigned int nodeCount=*(unsigned int*)(world+0x30);if(nodeCount>100000)ExitProcess(12);void** nodes=(void**)arena.Allocate(nodeCount*4);if(!nodes)ExitProcess(8);
  unsigned char* head=*(unsigned char**)(world+0x34);unsigned char* current=head;
  for(unsigned int i=0;i<nodeCount;i++){nodes[i]=current;current=*(unsigned char**)(current+4);}if(current!=head)ExitProcess(13);
  unsigned int treeHeader[2]={nodeCount,0};Write(treeHeader,8);
  for(unsigned int i=0;i<nodeCount;i++){
   unsigned char* node=(unsigned char*)nodes[i];unsigned char* p=*(unsigned char**)(node+0x10);void* surface=*(void**)(p+0x38);unsigned int source=0xffffffffu;
   for(unsigned int j=0;j<count;j++)if(*(void**)((unsigned char*)fake[j]+0x38)==surface)source=j;
   if(source==0xffffffffu)ExitProcess(15);
   int children[2];for(unsigned int k=0;k<2;k++){void* child=*(void**)(node+0x14+k*4);children[k]=(int)child;if(children[k]!=-1&&children[k]!=-2){children[k]=-3;for(unsigned int j=0;j<nodeCount;j++)if(nodes[j]==child)children[k]=(int)j;}}
   typedef void (__thiscall *Radius)(void*);((Radius)(dll+0x8880))(p+0x48);
   unsigned int corners=*(unsigned short*)(p+0x60);Write(&source,4);Write(children,8);Write(&corners,4);
   Write(&polys[source].normal,12);Write(&polys[source].distance,4);Write(p+0x50,16);
   for(unsigned int j=0;j<corners;j++)Write(p+0x68+j*44,12);
  }
#else
  FearBspTree::Input* inputs=(FearBspTree::Input*)arena.Allocate(count*sizeof(FearBspTree::Input));if(!inputs)ExitProcess(8);for(unsigned int j=0;j<count;j++){inputs[j].polygon=polys[j];inputs[j].source=j;}
  FearBspTree::Builder tree;if(!tree.Build(inputs,count)){LogLine(tree.error);ExitProcess(14);}
  unsigned int treeHeader[2]={tree.nodeCount,(unsigned int)tree.root};Write(treeHeader,8);
  for(FearBspTree::Node* node=tree.first;node;node=node->next){
   FearBspTree::Fragment& p=*node->polygon;Write(&p.source,4);Write(&node->negative,4);Write(&node->positive,4);Write(&p.polygon.count,4);Write(&p.polygon.normal,12);Write(&p.polygon.distance,4);Write(&p.polygon.center,12);Write(&p.polygon.radius,4);Write(p.polygon.points,p.polygon.count*12);
  }
#endif
  continue;
#endif
#ifdef FEAR_BSP_POLICY_ORACLE
  unsigned char lists[32]={};*(void***)(lists+4)=sky;*(void***)(lists+8)=sky+skyCount;*(void***)(lists+12)=sky+count;*(void***)(lists+20)=normal;*(void***)(lists+24)=normal+normalCount;*(void***)(lists+28)=normal+count;
  void* best=OriginalBest(dll+0x92e0,lists,depth);for(unsigned int j=0;j<count;j++)if(fake[j]==best)chosen=j;
#else
  FearBspSplitPolicy::Selector selected={};if(!FearBspSplitPolicy::Select(polys,count,depth,selected))ExitProcess(10);chosen=selected.index;
#endif
  Write(&chosen,4);
  // Full pair matrix makes threshold and coplanar controls observable.
  for(unsigned int a=0;a<count&&count<=256;a++)for(unsigned int b=0;b<count;b++){
   unsigned int sides[2];
#ifdef FEAR_BSP_POLICY_ORACLE
   float plane[4];memcpy(plane,&polys[a].normal,12);plane[3]=polys[a].distance;sides[0]=OriginalQuick(dll+0x8ff0,plane,fake[b]);unsigned char scratch[0x340]={};typedef unsigned int (__cdecl *Full)(const void*,const void*,void*);sides[1]=((Full)(dll+0x8960))(plane,fake[b],scratch);
#else
   sides[0]=FearBspSplitPolicy::QuickSide(polys[a],polys[b]);sides[1]=FearBspSplitPolicy::FullSide(polys[a],polys[b]);
#endif
   Write(sides,8);
  }
 }
 CloseHandle(input);CloseHandle(output);ExitProcess(0);
}
