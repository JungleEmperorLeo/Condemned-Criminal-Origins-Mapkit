#include "WorldPacker.cpp"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
extern "C" __declspec(dllimport) HMODULE WINAPI LoadLibraryA(const char*);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
#pragma comment(linker,"/alternatename:__imp__LoadLibraryA@4=__imp__LoadLibraryA")
// Isolated original cache worker. The original object's own setter, loader and
// clear methods perform normal state changes; no module data/code is patched.
extern "C" void HarnessEntry(){
 char root[MAX_PATH],input[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_SHADER_CLOSURE_ROOT",root,MAX_PATH)||!GetEnvironmentVariableA("FEAR_SHADER_CLOSURE_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_SHADER_CLOSURE_OUTPUT",output,MAX_PATH))ExitProcess(1);
 HANDLE in=CreateFileA(input,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);if(in==INVALID_HANDLE_VALUE)ExitProcess(2);DWORD bytes=GetFileSize(in,0),done=0;PrefabScene::Arena memory;unsigned char* data=(unsigned char*)memory.Allocate(bytes+1);if(!data||bytes<8||bytes>1024*1024||!ReadFile(in,data,bytes,&done,0)||done!=bytes)ExitProcess(3);CloseHandle(in);
 unsigned int* header=(unsigned int*)data;if(header[0]!=0x31434c53||header[1]>256)ExitProcess(4);unsigned int keyCount=header[1],identities[256]={},loaded[256]={};const unsigned char* cursor=data+8;const unsigned char* end=data+bytes;
 LoadLibraryA("E:/fearsdk/Dev/Tools/msvcr71.dll");LoadLibraryA("E:/fearsdk/Dev/Tools/msvcp71.dll");LoadLibraryA("E:/fearsdk/Dev/Tools/MFC71.dll");unsigned char* dll=(unsigned char*)LoadLibraryA("E:/fearsdk/Dev/Tools/Packers/WorldPacker.dll");if(!dll||dll[0x7abe0]!=0x64||dll[0x7a740]!=0x56||dll[0x7aa70]!=0x6a)ExitProcess(5);
 typedef void* (__cdecl* Getter)();typedef void (__thiscall* Setter)(void*,const char*);typedef void* (__thiscall* Lookup)(void*,const char*);typedef void (__thiscall* Clear)(void*);
 unsigned int* manager=(unsigned int*)((Getter)(dll+0x7abe0))();((Setter)(dll+0x7a740))(manager,root);
 for(unsigned int i=0;i<keyCount;i++){
  if(end-cursor<4)ExitProcess(4);unsigned int size=*(unsigned int*)cursor;cursor+=4;if(size>=264||end-cursor<(int)size)ExitProcess(4);char key[264];for(unsigned int j=0;j<size;j++)key[j]=(char)cursor[j];key[size]=0;cursor+=size;if(lstrlenA(key)!=(int)size)ExitProcess(4);
  void* shader=((Lookup)(dll+0x7aa70))(manager,key);loaded[i]=shader?1:0;identities[i]=0xffffffffu;
  if(shader){unsigned int* entries=(unsigned int*)manager[2];unsigned int count=(manager[3]-manager[2])/4;for(unsigned int n=0;n<count;n++)if(entries[n]==(unsigned int)shader){identities[i]=n;break;}if(identities[i]==0xffffffffu)ExitProcess(7);}
 }
 if(cursor!=end)ExitProcess(4);unsigned int cacheCount=(manager[3]-manager[2])/4;HANDLE out=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(out==INVALID_HANDLE_VALUE)ExitProcess(6);
 unsigned int result[3]={0x314f4353,keyCount,cacheCount};if(!WriteFile(out,result,12,&done,0)||done!=12||!WriteFile(out,identities,keyCount*4,&done,0)||done!=keyCount*4||!WriteFile(out,loaded,keyCount*4,&done,0)||done!=keyCount*4)ExitProcess(6);
 unsigned int* entries=(unsigned int*)manager[2];for(unsigned int i=0;i<cacheCount;i++){
  unsigned int* shader=(unsigned int*)entries[i];const char* name=(const char*)shader[0];unsigned int length=lstrlenA(name),declarationCount=(shader[7]-shader[6])/8;if(declarationCount>256||length>4096)ExitProcess(8);
  unsigned int rh[3]={i,length,declarationCount};if(!WriteFile(out,rh,12,&done,0)||done!=12||!WriteFile(out,name,length,&done,0)||done!=length||!WriteFile(out,(const void*)shader[6],declarationCount*8,&done,0)||done!=declarationCount*8)ExitProcess(6);
 }
 CloseHandle(out);((Clear)(dll+0x7a9b0))(manager);ExitProcess(0);
}
