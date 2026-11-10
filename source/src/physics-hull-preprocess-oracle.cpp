// Research only: captures four unchanged preprocessing routines in a private process.
#include "OriginalPhysicsOracle.h"
namespace HullPreprocessOracle {
struct Vec4 {float x,y,z,w;};
struct Array {Vec4* data;int size;unsigned int capacity;};
static bool Finite(float value){unsigned int bits;memcpy(&bits,&value,4);return (bits&0x7f800000u)!=0x7f800000u;}
static void* Aligned(PrefabScene::Arena& memory,unsigned int bytes){void* p=memory.Allocate(bytes+15);if(!p)ExitProcess(23);return (void*)(((unsigned int)p+15)&~15u);}
static bool Signature(const unsigned char* dll,unsigned int rva,const unsigned char* bytes,unsigned int size){for(unsigned int i=0;i<size;i++)if(dll[rva+i]!=bytes[i])return false;return true;}
}
extern "C" void HarnessEntry(){
 using namespace OriginalPhysicsOracle;using namespace HullPreprocessOracle;
 char input[MAX_PATH],output[MAX_PATH];DWORD a=GetEnvironmentVariableA("FEAR_HULL_INPUT",input,MAX_PATH),b=GetEnvironmentVariableA("FEAR_HULL_OUTPUT",output,MAX_PATH);if(!a||a>=MAX_PATH||!b||b>=MAX_PATH)ExitProcess(1);
 HANDLE file=CreateFileA(input,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);if(file==INVALID_HANDLE_VALUE)ExitProcess(4);DWORD high=0,size=GetFileSize(file,&high);if(high||size<4||size>8u*1024u*1024u)ExitProcess(4);
 PrefabScene::Arena packetMemory;unsigned char* packet=(unsigned char*)packetMemory.Allocate(size);if(!packet||!Read(file,packet,size))ExitProcess(4);CloseHandle(file);unsigned int count;memcpy(&count,packet,4);if(!count||count>2000)ExitProcess(4);
 // Validate the complete packet before loading any original code or touching output.
 unsigned int position=4;for(unsigned int c=0;c<count;c++){if(size-position<8)ExitProcess(6);unsigned int n;float squaredDistance;memcpy(&n,packet+position,4);memcpy(&squaredDistance,packet+position+4,4);position+=8;if(!n||n>128||!Finite(squaredDistance)||squaredDistance<0||n>(size-position)/16)ExitProcess(6);const float* values=(const float*)(packet+position);for(unsigned int i=0;i<n*4;i++)if(!Finite(values[i]))ExitProcess(6);position+=n*16;}if(position!=size)ExitProcess(6);
 unsigned char* dll=Load();Install(dll);const unsigned char alignedEntry[]={0x55,0x8b,0xec,0x83,0xe4,0xf0};const unsigned char comparatorEntry[]={0x8b,0x4c,0x24,0x08,0xd9,0x01};if(!Signature(dll,0x658d0,alignedEntry,6)||!Signature(dll,0x65380,alignedEntry,6)||!Signature(dll,0x66800,alignedEntry,6)||!Signature(dll,0x65b00,alignedEntry,6)||!Signature(dll,0x65280,comparatorEntry,6))ExitProcess(3);
 typedef void(__cdecl* Normalize)(Array*,Vec4*,Vec4*);typedef void(__cdecl* Sort)(Vec4*,int,int,void*);typedef void(__cdecl* Dedup)(float,Array*,int*);typedef void(__cdecl* Unnormalize)(Array*,Vec4*,Vec4*);
 HANDLE out=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(out==INVALID_HANDLE_VALUE||!Write(out,&count,4))ExitProcess(5);position=4;
 for(unsigned int c=0;c<count;c++){PrefabScene::Arena memory;Begin(memory);unsigned int n;float squaredDistance;memcpy(&n,packet+position,4);memcpy(&squaredDistance,packet+position+4,4);position+=8;Vec4* points=(Vec4*)Aligned(memory,n*16);memcpy(points,packet+position,n*16);position+=n*16;Array array={points,(int)n,0x80000000u|n};__declspec(align(16)) Vec4 span={0,0,0,0},center={0,0,0,0};
  Vec4* normalized=(Vec4*)Aligned(memory,n*16);Vec4* sorted=(Vec4*)Aligned(memory,n*16);Vec4* deduplicated=(Vec4*)Aligned(memory,n*16);
  ((Normalize)(dll+0x658d0))(&array,&span,&center);if(array.data!=points||array.size!=(int)n)ExitProcess(7);memcpy(normalized,points,n*16);
  ((Sort)(dll+0x65380))(points,0,(int)n-1,dll+0x65280);memcpy(sorted,points,n*16);int retained=-1;((Dedup)(dll+0x66800))(squaredDistance,&array,&retained);if(array.data!=points||array.size<0||array.size>(int)n||array.size!=retained)ExitProcess(7);unsigned int kept=(unsigned int)array.size;memcpy(deduplicated,points,kept*16);
  ((Unnormalize)(dll+0x65b00))(&array,&span,&center);if(array.data!=points||array.size!=(int)kept)ExitProcess(7);unsigned int header[5]={n,kept,(unsigned int)retained,allocationCalls,allocationBytes};if(!Write(out,header,20)||!Write(out,&span,16)||!Write(out,&center,16)||!Write(out,normalized,n*16)||!Write(out,sorted,n*16)||!Write(out,deduplicated,kept*16)||!Write(out,points,kept*16))ExitProcess(8);
 }CloseHandle(out);ExitProcess(0);
}
