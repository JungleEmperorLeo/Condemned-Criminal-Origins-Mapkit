#include "WorldPacker.cpp"
#include "StreamingBlock.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
static char path[MAX_PATH];
static unsigned char* readEnv(const char* name,unsigned int* size){
 if(!GetEnvironmentVariableA(name,path,MAX_PATH))ExitProcess(1);
 HANDLE file=CreateFileA(path,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);if(file==INVALID_HANDLE_VALUE)ExitProcess(2);
 *size=GetFileSize(file,0);if(!*size||*size>16*1024*1024)ExitProcess(3);
 unsigned char* data=(unsigned char*)VirtualAlloc(0,*size,MEM_COMMIT|MEM_RESERVE,4);DWORD done;
 if(!data||!ReadFile(file,data,*size,&done,0)||done!=*size)ExitProcess(4);CloseHandle(file);return data;
}
extern "C" void HarnessEntry(){
 StreamingBlock::Block block;StreamingBlock::Asset assets[2]={{"","MinimalBox.xsb"},{"Materials\\Freeway","Freeway_Sign.Mat00"}};
 if(!block.BuildGlobal(assets,2)||block.size!=124)ExitProcess(20);
 // Diagnostic resource-list input, independent of the World00p binary layout.
 if(GetEnvironmentVariableA("FEAR_STREAMING_ASSETS",path,MAX_PATH)){
  unsigned int bytes=0;unsigned char* input=readEnv("FEAR_STREAMING_ASSETS",&bytes);
  if(bytes<4)ExitProcess(30);
  unsigned int count=*(unsigned int*)input;if(count>100000)ExitProcess(31);
  StreamingBlock::Asset* list=(StreamingBlock::Asset*)VirtualAlloc(0,(count+1)*sizeof(StreamingBlock::Asset),MEM_COMMIT|MEM_RESERVE,4);
  if(!list)ExitProcess(32);unsigned int cursor=4;
  for(unsigned int i=0;i<count;i++){
   for(unsigned int part=0;part<2;part++){
    if(cursor>=bytes)ExitProcess(33);const char* value=(const char*)(input+cursor);
    if(part)list[i].file=value;else list[i].folder=value;
    unsigned int begin=cursor;while(cursor<bytes&&input[cursor])cursor++;
    if(cursor>=bytes||cursor-begin>4096)ExitProcess(34);cursor++;
   }
  }
  if(cursor!=bytes||!block.BuildGlobal(list,count))ExitProcess(35);
 }
 if(!GetEnvironmentVariableA("FEAR_STREAMING_BLOCK",path,MAX_PATH))ExitProcess(1);
 HANDLE file=CreateFileA(path,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);DWORD done;
 if(file==INVALID_HANDLE_VALUE||!WriteFile(file,block.data,block.size,&done,0)||done!=block.size)ExitProcess(21);CloseHandle(file);
 if(!block.BuildGlobal(0,0)||block.size!=48)ExitProcess(22);
 if(block.BuildGlobal(0,1)||block.data||block.size)ExitProcess(23);
 StreamingBlock::Asset duplicate[2]={{"shared","file"},{"shared","file"}};
 if(!block.BuildGlobal(duplicate,2)||*(unsigned int*)(block.data+48+16)!=*(unsigned int*)(block.data+48+16+8))ExitProcess(24);
 char mutableName[]="file";StreamingBlock::Asset owned={"",mutableName};if(!block.BuildGlobal(&owned,1))ExitProcess(25);mutableName[0]='X';if(block.data[48]!='f')ExitProcess(26);
 owned.file="";if(block.BuildGlobal(&owned,1))ExitProcess(27);
 ExitProcess(0);
}
