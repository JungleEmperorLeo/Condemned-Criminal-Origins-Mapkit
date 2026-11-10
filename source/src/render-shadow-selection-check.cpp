#include "WorldPacker.cpp"
#include "RenderShadowSelection.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
#ifdef FEAR_SHADOW_SELECTION_ORACLE
extern "C" __declspec(dllimport) HMODULE WINAPI LoadLibraryA(const char*);
#pragma comment(linker,"/alternatename:__imp__LoadLibraryA@4=__imp__LoadLibraryA")
// Execute only the original 60-byte selection slice in a synthetic stack frame.
// All three outgoing reject branches are redirected to a bounded local return;
// source triangle construction, allocators, Process and SDK files are untouched.
__declspec(naked) unsigned int CallSlice(void*,unsigned int,unsigned int,void*,void*){
 __asm {
  push ebp
  mov ebp,esp
  push esi
  sub esp,240h
  mov edx,dword ptr [ebp+10h]
  mov dword ptr [esp+230h],edx
  mov ecx,dword ptr [ebp+0ch]
  mov esi,dword ptr [ebp+14h]
  mov eax,dword ptr [ebp+18h]
  call dword ptr [ebp+8]
  lea esp,[ebp-4]
  pop esi
  pop ebp
  ret
 }
}
#endif
struct SelectionInput {unsigned int flags,pass,sectionState,canonicalState;char sectionName[264],canonicalName[264];};
static_assert(sizeof(SelectionInput)==544,"selection input");
extern "C" void HarnessEntry(){
 char inputPath[MAX_PATH],outputPath[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_SELECTION_INPUT",inputPath,MAX_PATH)||!GetEnvironmentVariableA("FEAR_SELECTION_OUTPUT",outputPath,MAX_PATH))ExitProcess(1);
 HANDLE in=CreateFileA(inputPath,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);DWORD done;unsigned int count=0;
 if(in==INVALID_HANDLE_VALUE||!ReadFile(in,&count,4,&done,0)||done!=4||!count||count>100000)ExitProcess(2);
 PrefabScene::Arena memory;SelectionInput* input=(SelectionInput*)memory.Allocate(count*544);unsigned int* output=(unsigned int*)memory.Allocate(count*8);if(!input||!output||!ReadFile(in,input,count*544,&done,0)||done!=count*544)ExitProcess(3);CloseHandle(in);
#ifdef FEAR_SHADOW_SELECTION_ORACLE
 LoadLibraryA("E:/fearsdk/Dev/Tools/msvcr71.dll");LoadLibraryA("E:/fearsdk/Dev/Tools/msvcp71.dll");LoadLibraryA("E:/fearsdk/Dev/Tools/MFC71.dll");unsigned char* dll=(unsigned char*)LoadLibraryA("E:/fearsdk/Dev/Tools/Packers/WorldPacker.dll");if(!dll)ExitProcess(4);
 const unsigned char expected[60]={0xf6,0xc1,0x40,0x0f,0x84,0x39,0x01,0x00,0x00,0x8b,0x94,0x24,0x34,0x02,0x00,0x00,0xc1,0xe9,0x10,0x83,0xe1,0x03,0x3b,0xca,0x0f,0x85,0x24,0x01,0x00,0x00,0x8b,0x88,0x0c,0x01,0x00,0x00,0x8d,0x14,0x89,0x8b,0x4e,0x04,0x8d,0x0c,0xd1,0x8b,0x51,0x04,0x3b,0x96,0x30,0x02,0x00,0x00,0x0f,0x84,0x06,0x01,0x00,0x00};
 for(unsigned int i=0;i<60;i++)if(dll[0x4a5e8+i]!=expected[i])ExitProcess(5);
 unsigned char* code=(unsigned char*)VirtualAlloc(0,128,MEM_COMMIT|MEM_RESERVE,0x40);if(!code)ExitProcess(6);memcpy(code,dll+0x4a5e8,60);
 const unsigned int branches[3]={3,24,54};for(unsigned int i=0;i<3;i++)*(int*)(code+branches[i]+2)=66-(int)(branches[i]+6);
 code[60]=0xb8;*(unsigned int*)(code+61)=1;code[65]=0xc3;code[66]=0xb8;*(unsigned int*)(code+67)=0;code[71]=0xc3;
 typedef void* (__thiscall* Lookup)(void*,const char*);Lookup lookup=(Lookup)(dll+0x7aa70);
#endif
 for(unsigned int i=0;i<count;i++){
  SelectionInput& item=input[i];item.sectionName[263]=item.canonicalName[263]=0;
  RenderShadowSelection::ShaderKey section={item.sectionName,(RenderShadowSelection::State)item.sectionState},canonical={item.canonicalName,(RenderShadowSelection::State)item.canonicalState};bool selected=false;
  bool valid=RenderShadowSelection::Selected(item.flags,item.pass,section,canonical,&selected);output[i*2]=valid?1:0;output[i*2+1]=selected?1:0;
#ifdef FEAR_SHADOW_SELECTION_ORACLE
  if(valid){
   // The lookup is guaranteed to hit one of these already-loaded mock Shader
   // records. No filename miss or shader loader is invoked by this oracle.
   unsigned int shader1[15]={},shader2[15]={};shader1[0]=(unsigned int)item.sectionName;shader2[0]=(unsigned int)item.canonicalName;
   unsigned int entries[2]={(unsigned int)shader1,(unsigned int)shader2};unsigned int manager[5]={0,0,(unsigned int)entries,(unsigned int)(entries+2),(unsigned int)(entries+2)};
   void* sectionPointer=section.state==RenderShadowSelection::Loaded?lookup(manager,item.sectionName):0;
   void* canonicalPointer=canonical.state==RenderShadowSelection::Loaded?lookup(manager,item.canonicalName):0;
   unsigned int node[144]={},triangle[70]={},sectionRecord[10]={};node[1]=(unsigned int)sectionRecord;node[0x230/4]=(unsigned int)canonicalPointer;sectionRecord[1]=(unsigned int)sectionPointer;
   output[i*2+1]=CallSlice(code,item.flags,item.pass,node,triangle);
  }
#endif
 }
 HANDLE out=CreateFileA(outputPath,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(out==INVALID_HANDLE_VALUE||!WriteFile(out,&count,4,&done,0)||done!=4||!WriteFile(out,output,count*8,&done,0)||done!=count*8)ExitProcess(7);CloseHandle(out);ExitProcess(0);
}
