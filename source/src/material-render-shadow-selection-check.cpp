#include "WorldPacker.cpp"
#include "MaterialRenderLayout.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
struct SelectionQuery {unsigned int profile,flags,pass;};
struct Binding {unsigned int state,same;RenderShadowSelection::ShaderKey key;void* pointer;};
static bool ReadMaterial(PrefabScene::Arena& memory,const char* root,const char* material,MaterialResources::Material& output){
 char filename[MAX_PATH];if(!PrefabScene::Join(root,material,filename))return false;HANDLE file=CreateFileA(filename,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);if(file==INVALID_HANDLE_VALUE)return false;
 DWORD bytes=GetFileSize(file,0),done=0;unsigned char* data=(unsigned char*)memory.Allocate(bytes+1);if(!data||bytes<12||bytes>16*1024*1024||!ReadFile(file,data,bytes,&done,0)||done!=bytes){CloseHandle(file);return false;}CloseHandle(file);return output.Read(data,bytes);
}
#ifdef FEAR_MATERIAL_SHADOW_ORACLE
__declspec(naked) unsigned int MaterialCallSlice(void*,unsigned int,unsigned int,void*,void*){
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
extern "C" void HarnessEntry(){
 char root[MAX_PATH],input[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_MATERIAL_SHADOW_ROOT",root,MAX_PATH)||!GetEnvironmentVariableA("FEAR_MATERIAL_SHADOW_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_MATERIAL_SHADOW_OUTPUT",output,MAX_PATH))ExitProcess(1);
 HANDLE in=CreateFileA(input,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);if(in==INVALID_HANDLE_VALUE)ExitProcess(2);DWORD bytes=GetFileSize(in,0),done=0;PrefabScene::Arena memory;unsigned char* data=(unsigned char*)memory.Allocate(bytes+1);
 if(!data||bytes<12||bytes>2*1024*1024||!ReadFile(in,data,bytes,&done,0)||done!=bytes)ExitProcess(3);CloseHandle(in);unsigned int* header=(unsigned int*)data;if(header[0]!=0x3149534d||header[1]>64||header[2]>100000)ExitProcess(4);
 unsigned int profiles=header[1],queries=header[2];Binding bindings[64]={},canonical={};const unsigned char* cursor=data+12;const unsigned char* end=data+bytes;
#ifdef FEAR_MATERIAL_SHADOW_ORACLE
 LoadLibraryA("E:/fearsdk/Dev/Tools/msvcr71.dll");LoadLibraryA("E:/fearsdk/Dev/Tools/msvcp71.dll");LoadLibraryA("E:/fearsdk/Dev/Tools/MFC71.dll");unsigned char* dll=(unsigned char*)LoadLibraryA("E:/fearsdk/Dev/Tools/Packers/WorldPacker.dll");if(!dll)ExitProcess(5);
 const unsigned char expected[60]={0xf6,0xc1,0x40,0x0f,0x84,0x39,0x01,0x00,0x00,0x8b,0x94,0x24,0x34,0x02,0x00,0x00,0xc1,0xe9,0x10,0x83,0xe1,0x03,0x3b,0xca,0x0f,0x85,0x24,0x01,0x00,0x00,0x8b,0x88,0x0c,0x01,0x00,0x00,0x8d,0x14,0x89,0x8b,0x4e,0x04,0x8d,0x0c,0xd1,0x8b,0x51,0x04,0x3b,0x96,0x30,0x02,0x00,0x00,0x0f,0x84,0x06,0x01,0x00,0x00};for(unsigned int i=0;i<60;i++)if(dll[0x4a5e8+i]!=expected[i])ExitProcess(6);
 unsigned char* code=(unsigned char*)VirtualAlloc(0,128,MEM_COMMIT|MEM_RESERVE,0x40);if(!code)ExitProcess(7);memcpy(code,dll+0x4a5e8,60);const unsigned int branches[3]={3,24,54};for(unsigned int i=0;i<3;i++)*(int*)(code+branches[i]+2)=66-(int)(branches[i]+6);code[60]=0xb8;*(unsigned int*)(code+61)=1;code[65]=0xc3;code[66]=0xb8;*(unsigned int*)(code+67)=0;code[71]=0xc3;
 typedef void* (__cdecl* Getter)();typedef void (__thiscall* Setter)(void*,const char*);typedef void* (__thiscall* Lookup)(void*,const char*);typedef void (__thiscall* Clear)(void*);void* manager=((Getter)(dll+0x7abe0))();((Setter)(dll+0x7a740))(manager,root);Lookup lookup=(Lookup)(dll+0x7aa70);
#else
 void* cacheMemory=memory.Allocate(sizeof(ShaderDeclarationClosure::Cache));if(!cacheMemory)ExitProcess(8);ShaderDeclarationClosure::Cache* cache=new(cacheMemory)ShaderDeclarationClosure::Cache();if(!cache->Initialize(root))ExitProcess(8);
#endif
 for(unsigned int p=0;p<=profiles;p++){
  char name[264];if(!p)lstrcpyA(name,"engine\\shadowvolume.Mat00");else{if(end-cursor<4)ExitProcess(4);unsigned int size=*(unsigned int*)cursor;cursor+=4;if(size>=264||end-cursor<(int)size)ExitProcess(4);for(unsigned int i=0;i<size;i++)name[i]=(char)cursor[i];name[size]=0;cursor+=size;if(lstrlenA(name)!=(int)size)ExitProcess(4);}
  MaterialResources::Material material;if(!ReadMaterial(memory,root,name,material))ExitProcess(9);Binding& binding=p?bindings[p-1]:canonical;
#ifdef FEAR_MATERIAL_SHADOW_ORACLE
  binding.pointer=lookup(manager,material.shader);binding.state=binding.pointer?2u:1u;binding.same=p?(binding.pointer==canonical.pointer?1u:0u):1u;
#else
  MaterialRenderLayout::Layout layout;if(!MaterialRenderLayout::Resolve(*cache,material,&layout))ExitProcess(10);binding.key=layout.shader;binding.state=(unsigned int)layout.shader.state;binding.same=p?(RenderShadowSelection::EqualResolved(binding.key,canonical.key)?1u:0u):1u;
#endif
 }
 if((unsigned int)(end-cursor)!=queries*sizeof(SelectionQuery))ExitProcess(4);const SelectionQuery* query=(const SelectionQuery*)cursor;unsigned int* results=(unsigned int*)memory.Allocate(queries*8);if(!results)ExitProcess(11);
 for(unsigned int i=0;i<queries;i++){
  if(query[i].profile>=profiles||query[i].pass>2)ExitProcess(4);Binding& section=bindings[query[i].profile];unsigned int selected=0;
#ifdef FEAR_MATERIAL_SHADOW_ORACLE
  unsigned int node[144]={},triangle[70]={},sectionRecord[10]={};node[1]=(unsigned int)sectionRecord;node[0x230/4]=(unsigned int)canonical.pointer;sectionRecord[1]=(unsigned int)section.pointer;selected=MaterialCallSlice(code,query[i].flags,query[i].pass,node,triangle);results[i*2]=1;
#else
  bool value=false;bool valid=RenderShadowSelection::Selected(query[i].flags,query[i].pass,section.key,canonical.key,&value);selected=value?1:0;results[i*2]=valid?1:0;
#endif
  results[i*2+1]=selected;
 }
 HANDLE out=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(out==INVALID_HANDLE_VALUE)ExitProcess(12);unsigned int resultHeader[4]={0x314f534d,profiles,queries,canonical.state};if(!WriteFile(out,resultHeader,16,&done,0)||done!=16)ExitProcess(12);
 for(unsigned int i=0;i<profiles;i++){unsigned int profile[2]={bindings[i].state,bindings[i].same};if(!WriteFile(out,profile,8,&done,0)||done!=8)ExitProcess(12);}if(!WriteFile(out,results,queries*8,&done,0)||done!=queries*8)ExitProcess(12);CloseHandle(out);
#ifdef FEAR_MATERIAL_SHADOW_ORACLE
 ((Clear)(dll+0x7a9b0))(manager);
#else
 cache->~Cache();
#endif
 ExitProcess(0);
}
