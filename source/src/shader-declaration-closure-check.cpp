#include "WorldPacker.cpp"
#include "ShaderDeclarationClosure.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
// Source-only closure diagnostic. No original packer is loaded by this binary.
// Input: magic, keyCount, repeated (length, raw key). Output includes both final
// and primary declarations and dependency provenance, including failed loads.
extern "C" void HarnessEntry(){
 char root[MAX_PATH],input[MAX_PATH],output[MAX_PATH];
 if(!GetEnvironmentVariableA("FEAR_SHADER_CLOSURE_ROOT",root,MAX_PATH)||!GetEnvironmentVariableA("FEAR_SHADER_CLOSURE_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_SHADER_CLOSURE_OUTPUT",output,MAX_PATH))ExitProcess(1);
 HANDLE file=CreateFileA(input,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);if(file==INVALID_HANDLE_VALUE)ExitProcess(2);
 DWORD bytes=GetFileSize(file,0),done=0;if(bytes<8||bytes>1024*1024)ExitProcess(2);PrefabScene::Arena memory;unsigned char* data=(unsigned char*)memory.Allocate(bytes+1);
 if(!data||!ReadFile(file,data,bytes,&done,0)||done!=bytes)ExitProcess(3);CloseHandle(file);unsigned int* header=(unsigned int*)data;if(header[0]!=0x31434c53||header[1]>256)ExitProcess(4);
 unsigned int keys=header[1],identities[256],states[256];const unsigned char* cursor=data+8;const unsigned char* end=data+bytes;
 void* cacheStorage=memory.Allocate(sizeof(ShaderDeclarationClosure::Cache));if(!cacheStorage)ExitProcess(5);ShaderDeclarationClosure::Cache* cache=new(cacheStorage)ShaderDeclarationClosure::Cache();if(!cache->Initialize(root))ExitProcess(5);
 bool unsupported=false;for(unsigned int i=0;i<keys;i++){
  if(end-cursor<4)ExitProcess(4);unsigned int size=*(unsigned int*)cursor;cursor+=4;if(size>=264||end-cursor<(int)size)ExitProcess(4);char key[264];for(unsigned int j=0;j<size;j++)key[j]=(char)cursor[j];key[size]=0;cursor+=size;
  if(lstrlenA(key)!=(int)size)ExitProcess(4);bool resolved=cache->Resolve(key,&identities[i]);const ShaderDeclarationClosure::Record* record=cache->Get(identities[i]);states[i]=resolved&&record?(unsigned int)record->state:3;if(!resolved)unsupported=true;
 }
 if(cursor!=end)ExitProcess(4);
 HANDLE out=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(out==INVALID_HANDLE_VALUE)ExitProcess(6);
 unsigned int result[5]={0x32434c53,keys,cache->count,cache->edgeCount,unsupported?1u:0u};
 if(!WriteFile(out,result,20,&done,0)||done!=20||!WriteFile(out,identities,keys*4,&done,0)||done!=keys*4||!WriteFile(out,states,keys*4,&done,0)||done!=keys*4)ExitProcess(6);
 for(unsigned int i=0;i<cache->count;i++){
  const ShaderDeclarationClosure::Record& record=*cache->Get(i);unsigned int length=lstrlenA(record.key),errorLength=record.error?lstrlenA(record.error):0;
  unsigned int rh[10]={i,(unsigned int)record.state,length,record.count,record.warnings,(unsigned int)record.primary.result,record.includeOpens,record.includeMaximumDepth,errorLength,record.primary.count};
  if(!WriteFile(out,rh,40,&done,0)||done!=40||!WriteFile(out,record.key,length,&done,0)||done!=length||!WriteFile(out,record.elements,record.count*8,&done,0)||done!=record.count*8||!WriteFile(out,record.primary.elements,record.primary.count*8,&done,0)||done!=record.primary.count*8)ExitProcess(6);
  if(errorLength&&(!WriteFile(out,record.error,errorLength,&done,0)||done!=errorLength))ExitProcess(6);
 }
 if(!WriteFile(out,cache->edges,cache->edgeCount*sizeof(ShaderDeclarationClosure::Edge),&done,0)||done!=cache->edgeCount*sizeof(ShaderDeclarationClosure::Edge))ExitProcess(6);
 CloseHandle(out);cache->~Cache();ExitProcess(0);
}
