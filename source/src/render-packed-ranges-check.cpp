#include "WorldPacker.cpp"
#include "RenderPackedRanges.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
struct Bytes {
 unsigned char* data;unsigned int count,capacity;bool failed;
 void Put(const void* value,unsigned int size){if(size>capacity-count){failed=true;return;}memcpy(data+count,value,size);count+=size;}
 void U32(unsigned int value){Put(&value,4);}void U16(unsigned short value){Put(&value,2);}void U8(unsigned char value){Put(&value,1);}
};
#ifdef FEAR_RANGES_ORACLE
extern "C" __declspec(dllimport) HMODULE WINAPI LoadLibraryA(const char*);
extern "C" __declspec(dllimport) void* WINAPI GetProcAddress(HMODULE,const char*);
#pragma comment(linker,"/alternatename:__imp__LoadLibraryA@4=__imp__LoadLibraryA")
#pragma comment(linker,"/alternatename:__imp__GetProcAddress@8=__imp__GetProcAddress")
struct MockStream {void** table;Bytes* bytes;};
static void __fastcall Write32(MockStream* stream,void*,unsigned int value){stream->bytes->U32(value);}
static void __fastcall Write16(MockStream* stream,void*,unsigned int value){stream->bytes->U16((unsigned short)value);}
static void __fastcall Write8(MockStream* stream,void*,unsigned int value){stream->bytes->U8((unsigned char)value);}
static void __fastcall Unexpected(MockStream*,void*){ExitProcess(19);}
struct Pair {void* node;void* section;};
static void CallWriter(void* target,MockStream* stream,void* mesh,unsigned int* declarations,unsigned int* allocation,unsigned int* declarationBytes){
 __asm {
  push esi
  mov esi,stream
  push declarationBytes
  push allocation
  push declarations
  push mesh
  call target
  add esp,16
  pop esi
 }
}
#endif
static bool Read(HANDLE file,void* out,unsigned int size){DWORD done;return ReadFile(file,out,size,&done,0)&&done==size;}
extern "C" void HarnessEntry(){
 char input[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_RANGES_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_RANGES_OUTPUT",output,MAX_PATH))ExitProcess(1);
 HANDLE in=CreateFileA(input,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);unsigned int fileHeader[3];if(in==INVALID_HANDLE_VALUE||!Read(in,fileHeader,12)||fileHeader[0]!=0x31474E52||fileHeader[1]>RenderPackedRanges::MaximumSections||fileHeader[2]>1)ExitProcess(2);
 PrefabScene::Arena memory;unsigned int size=fileHeader[1];RenderPackedRanges::Input* sections=(RenderPackedRanges::Input*)memory.Allocate(size*sizeof(RenderPackedRanges::Input));unsigned int* prefixes=(unsigned int*)memory.Allocate(size*4);if(!sections||!prefixes)ExitProcess(3);
 unsigned int totalVertices=0,totalTriangles=0,shaderCount=0;
 for(unsigned int i=0;i<size;i++){
  unsigned int h[8];if(!Read(in,h,32)||!h[0]||h[0]>4096||h[1]>=RenderPackedRanges::MaximumSections||h[3]<2||h[3]>64||h[4]>65535||h[5]>4000000||h[6]>100000||h[7]>300000||(h[6]&&!h[4]))ExitProcess(3);
#ifdef FEAR_RANGES_ORACLE
  // The private oracle materializes the unused working prefixes. Retain its
  // small diagnostic allocation profile; native packing uses localized slices.
  if(h[5]>1000||h[7]>1000||h[4]>200000-totalVertices||h[6]>300000-totalTriangles)ExitProcess(3);
#else
  if(!RenderPackedLimits::Fits(h[4],totalVertices,RenderPackedLimits::MaximumVertices)||!RenderPackedLimits::Fits(h[6],totalTriangles,RenderPackedLimits::MaximumTriangles))ExitProcess(3);
#endif
  totalVertices+=h[4];totalTriangles+=h[6];if(h[1]+1>shaderCount)shaderCount=h[1]+1;
  char* name=(char*)memory.Allocate(h[0]+1);RenderVertexPacking::Element* declaration=(RenderVertexPacking::Element*)memory.Allocate(h[3]*8);RenderAttributes::Vertex* vertices=(RenderAttributes::Vertex*)memory.Allocate(h[4]*84);unsigned int* indices=(unsigned int*)memory.Allocate(h[6]*12);
  if(!name||!declaration||!vertices||!indices||!Read(in,name,h[0])||!Read(in,declaration,h[3]*8)||!Read(in,vertices,h[4]*84)||!Read(in,indices,h[6]*12))ExitProcess(3);name[h[0]]=0;prefixes[i]=h[7];
  RenderPackedRanges::Input& s=sections[i];s.material=name;s.shaderKey=h[1];s.orderKey=h[2];s.declaration=declaration;s.elements=h[3];s.vertices=vertices;s.vertexCount=h[4];s.workingVertexStart=h[5];s.indices=indices;s.triangles=h[6];
  if(!RenderPackedRanges::Name(name)||!RenderVertexPacking::Stride(declaration,h[3]))ExitProcess(4);for(unsigned int q=0;q<h[6]*3;q++)if(indices[q]<h[5]||indices[q]-h[5]>=h[4])ExitProcess(4);
  for(unsigned int j=0;j<i;j++)if(sections[j].shaderKey==s.shaderKey&&!RenderPackedRanges::SameDeclaration(s.declaration,s.elements,sections[j].declaration,sections[j].elements))ExitProcess(4);
 }
 unsigned char extra;DWORD extraBytes=0;if(!ReadFile(in,&extra,1,&extraBytes,0)||extraBytes)ExitProcess(3);CloseHandle(in);Bytes bytes={(unsigned char*)memory.Allocate(32*1024*1024),0,32*1024*1024,false};if(!bytes.data)ExitProcess(5);unsigned int* order=(unsigned int*)memory.Allocate(size*4);unsigned int declarationCount=0,allocationBytes=0,declarationBytes=0;
 char idealText[16];unsigned int ideal=size;if(GetEnvironmentVariableA("FEAR_RANGES_SORT_IDEAL",idealText,16)){ideal=0;for(unsigned int i=0;idealText[i];i++){if(idealText[i]<'0'||idealText[i]>'9'||ideal>1000000)ExitProcess(4);ideal=ideal*10+(unsigned int)(idealText[i]-'0');}}
#ifdef FEAR_RANGES_ORACLE
 HMODULE system=LoadLibraryA("kernel32.dll");typedef unsigned int (WINAPI *ErrorMode)(unsigned int);ErrorMode setErrorMode=system?(ErrorMode)GetProcAddress(system,"SetErrorMode"):0;if(!setErrorMode)ExitProcess(6);setErrorMode(3);
 LoadLibraryA("E:/fearsdk/Dev/Tools/msvcr71.dll");LoadLibraryA("E:/fearsdk/Dev/Tools/msvcp71.dll");LoadLibraryA("E:/fearsdk/Dev/Tools/MFC71.dll");unsigned char* dll=(unsigned char*)LoadLibraryA("E:/fearsdk/Dev/Tools/Packers/WorldPacker.dll");if(!dll)ExitProcess(6);
 const unsigned char signature[7]={0x6a,0xff,0x68,0x69,0x59,0x08,0x10};if(dll[0x4e880]!=signature[0]||dll[0x4e881]!=signature[1]||dll[0x4e882]!=signature[2])ExitProcess(7);
 // Absolute SEH handler bytes are relocated; verify the non-address opcode.
 if(dll[0x4e6f0]!=0x83||dll[0x4e6f1]!=0xec||dll[0x4e6f2]!=0x08)ExitProcess(7);
 // The count-zero converter returns false through an assertion dispatcher.
 // Without its optional AssertWin32DLL it returns quietly. If that module is
 // available, do not invoke a zero-vertex original probe or risk an assertion UI.
 for(unsigned int i=0;i<size;i++)if(!sections[i].vertexCount){if(LoadLibraryA("AssertWin32DLL.dll"))ExitProcess(11);break;}
 unsigned char* shaders=(unsigned char*)memory.Allocate(shaderCount*36);Pair* pairs=(Pair*)memory.Allocate(size*8);unsigned char* nodes=(unsigned char*)memory.Allocate(size*0x74);unsigned char* records=(unsigned char*)memory.Allocate(size*80);if(!shaders||!pairs||!nodes||!records)ExitProcess(5);memset(shaders,0,shaderCount*36);memset(nodes,0,size*0x74);memset(records,0,size*80);
 for(unsigned int i=0;i<size;i++){
  const RenderPackedRanges::Input& s=sections[i];unsigned char* shader=shaders+s.shaderKey*36;*(void**)(shader+24)=(void*)s.declaration;*(void**)(shader+28)=(void*)(s.declaration+s.elements);*(void**)(shader+32)=(void*)(s.declaration+s.elements);
  unsigned char* node=nodes+i*0x74;unsigned char* list=records+i*80;unsigned char* section=list+40;
  RenderAttributes::Vertex* vertices=(RenderAttributes::Vertex*)memory.Allocate((s.workingVertexStart+s.vertexCount)*84);unsigned int* indices=(unsigned int*)memory.Allocate((prefixes[i]+s.triangles)*12);if(!vertices||!indices)ExitProcess(5);memset(vertices,0,(s.workingVertexStart+s.vertexCount)*84);memcpy(vertices+s.workingVertexStart,s.vertices,s.vertexCount*84);memset(indices,0,(prefixes[i]+s.triangles)*12);memcpy(indices+prefixes[i]*3,s.indices,s.triangles*12);
  *(void**)(node+4)=list;*(void**)(node+8)=list+80;*(void**)(node+12)=list+80;*(void**)(node+0x14)=indices;*(void**)(node+0x18)=indices+(prefixes[i]+s.triangles)*3;*(void**)(node+0x1c)=indices+(prefixes[i]+s.triangles)*3;*(void**)(node+0x24)=vertices;*(void**)(node+0x28)=vertices+s.workingVertexStart+s.vertexCount;*(void**)(node+0x2c)=vertices+s.workingVertexStart+s.vertexCount;
  *(unsigned int*)(list+24)=prefixes[i];*(const char**)section=s.material;*(void**)(section+4)=shader;*(unsigned int*)(section+8)=s.orderKey;*(unsigned int*)(section+24)=s.triangles;*(unsigned int*)(section+32)=s.workingVertexStart;*(unsigned int*)(section+36)=s.vertexCount;pairs[i].node=node;pairs[i].section=section;
 }
 if(fileHeader[2]){typedef void (__cdecl* Sort)(void*,void*,unsigned int);((Sort)(dll+0x4e6f0))(pairs,pairs+size,ideal);}
 for(unsigned int i=0;i<size;i++)order[i]=((unsigned char*)pairs[i].node-nodes)/0x74;
 unsigned char mesh[48]={};*(void**)(mesh+0x24)=pairs;*(void**)(mesh+0x28)=pairs+size;*(void**)(mesh+0x2c)=pairs+size;
 void* table[24];for(unsigned int i=0;i<24;i++)table[i]=(void*)Unexpected;table[18]=(void*)Write32;table[20]=(void*)Write16;table[22]=(void*)Write8;MockStream stream={table,&bytes};CallWriter(dll+0x4e880,&stream,mesh,&declarationCount,&declarationBytes,&allocationBytes);
#else
 RenderPackedRanges::Builder builder;if(!builder.Build(sections,size,fileHeader[2]!=0,ideal))ExitProcess(6);for(unsigned int i=0;i<size;i++)order[i]=builder.order[i];
 declarationCount=builder.declarationCount;allocationBytes=(builder.vertexBytes+3)/4*4+(builder.indexCount*2+3)/4*4;
 bytes.U32(size);bytes.U32(builder.materialCount);bytes.U32(builder.vertexBytes);bytes.U32(builder.indexCount*2);bytes.Put(builder.vertices,builder.vertexBytes);bytes.Put(builder.indices,builder.indexCount*2);bytes.U32(builder.declarationCount);
 for(unsigned int i=0;i<builder.declarationCount;i++){unsigned int n=builder.declarationElements[i]*8;declarationBytes+=builder.declarationElements[i];bytes.U32(n);bytes.Put(builder.declarations[i],n);}
 bytes.U32(size);bytes.Put(builder.ranges,size*36);
#endif
 if(bytes.failed)ExitProcess(8);HANDLE out=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);unsigned int header[7]={0x31474e52,size,bytes.count,declarationCount,allocationBytes,declarationBytes,fileHeader[2]};DWORD done;if(out==INVALID_HANDLE_VALUE||!WriteFile(out,header,28,&done,0)||done!=28||!WriteFile(out,order,size*4,&done,0)||done!=size*4||!WriteFile(out,bytes.data,bytes.count,&done,0)||done!=bytes.count)ExitProcess(9);CloseHandle(out);ExitProcess(0);
}
