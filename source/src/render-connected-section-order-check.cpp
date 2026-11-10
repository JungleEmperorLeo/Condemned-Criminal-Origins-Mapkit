#include "WorldPacker.cpp"
#include "RenderConnectedSectionOrder.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
extern "C" __declspec(dllimport) HMODULE WINAPI LoadLibraryA(const char*);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
#pragma comment(linker,"/alternatename:__imp__LoadLibraryA@4=__imp__LoadLibraryA")
extern "C" void HarnessEntry(){
 char source[MAX_PATH],destination[MAX_PATH],material[64];if(!GetEnvironmentVariableA("FEAR_CONNECTED_ORDER_INPUT",source,MAX_PATH)||!GetEnvironmentVariableA("FEAR_CONNECTED_ORDER_OUTPUT",destination,MAX_PATH)||!GetEnvironmentVariableA("FEAR_CONNECTED_ORDER_MATERIAL",material,64))ExitProcess(1);
 HANDLE file=CreateFileA(source,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);unsigned int header[4]={};DWORD done=0;if(file==INVALID_HANDLE_VALUE||!ReadFile(file,header,16,&done,0)||done!=16||header[0]!=0x314f4352||header[1]>256||header[2]>4096||header[3]>8192)ExitProcess(2);
 unsigned int nodes=header[1],sections=header[2],edges=header[3];PrefabScene::Arena memory;RenderConnectedSectionOrder::Node* input=(RenderConnectedSectionOrder::Node*)memory.Allocate((nodes+1)*16);RenderConnectedSectionOrder::Section* names=(RenderConnectedSectionOrder::Section*)memory.Allocate((sections+1)*sizeof(RenderConnectedSectionOrder::Section));char* namePacket=(char*)memory.Allocate((sections+1)*64);unsigned int* adjacency=(unsigned int*)memory.Allocate((edges+1)*4);unsigned int* order=(unsigned int*)memory.Allocate((sections+1)*4);
 if(!input||!names||!namePacket||!adjacency||!order||!ReadFile(file,input,nodes*16,&done,0)||done!=nodes*16||!ReadFile(file,namePacket,sections*64,&done,0)||done!=sections*64||!ReadFile(file,adjacency,edges*4,&done,0)||done!=edges*4)ExitProcess(3);CloseHandle(file);for(unsigned int i=0;i<sections;i++){order[i]=0xffffffffu;names[i].material=namePacket+i*64;namePacket[i*64+63]=0;}
 RenderConnectedSectionOrder::Builder builder;if(!builder.Assign(input,nodes,names,sections,adjacency,edges,material,order))ExitProcess(4);unsigned int assigned=builder.assigned;
#ifdef FEAR_CONNECTED_ORDER_ORACLE
 LoadLibraryA("E:/fearsdk/Dev/Tools/msvcr71.dll");LoadLibraryA("E:/fearsdk/Dev/Tools/msvcp71.dll");LoadLibraryA("E:/fearsdk/Dev/Tools/MFC71.dll");unsigned char* dll=(unsigned char*)LoadLibraryA("E:/fearsdk/Dev/Tools/Packers/WorldPacker.dll");if(!dll||dll[0x4f9d0]!=0x6a)ExitProcess(5);
 unsigned int* nodeRecords=(unsigned int*)memory.Allocate((nodes+1)*84);unsigned int* sectorRecords=(unsigned int*)memory.Allocate((nodes+1)*112);unsigned int* sectionRecords=(unsigned int*)memory.Allocate((sections+1)*40);unsigned int* portals=(unsigned int*)memory.Allocate((edges+1)*40);unsigned int* portalPointers=(unsigned int*)memory.Allocate((edges+1)*4);unsigned int* nodePointers=(unsigned int*)memory.Allocate((nodes+1)*4);if(!nodeRecords||!sectorRecords||!sectionRecords||!portals||!portalPointers||!nodePointers)ExitProcess(6);
 memset(nodeRecords,0,(nodes+1)*84);memset(sectorRecords,0,(nodes+1)*112);memset(sectionRecords,0,(sections+1)*40);memset(portals,0,(edges+1)*40);
 for(unsigned int n=0;n<nodes;n++){unsigned int* node=nodeRecords+n*21;unsigned int* sector=sectorRecords+n*28;nodePointers[n]=(unsigned int)node;node[1]=(unsigned int)(sectionRecords+input[n].sectionStart*10);node[2]=(unsigned int)(sectionRecords+(input[n].sectionStart+input[n].sectionCount)*10);node[3]=node[2];node[20]=(unsigned int)sector;sector[19]=(unsigned int)(portalPointers+input[n].neighborStart);sector[20]=input[n].neighborCount;sector[27]=n;
  for(unsigned int i=0;i<input[n].neighborCount;i++){unsigned int edge=input[n].neighborStart+i;unsigned int* portal=portals+edge*10;portal[8]=(unsigned int)sector;portal[9]=(unsigned int)(sectorRecords+adjacency[edge]*28);portalPointers[edge]=(unsigned int)portal;}
 }
 for(unsigned int i=0;i<sections;i++){sectionRecords[i*10]=(unsigned int)names[i].material;sectionRecords[i*10+2]=0xffffffffu;}
 unsigned int tree[5]={0,(unsigned int)nodePointers,(unsigned int)(nodePointers+nodes),(unsigned int)(nodePointers+nodes),0};typedef void (__stdcall* Assign)(void*,const char*);((Assign)(dll+0x4f9d0))(tree,material);assigned=0;
 for(unsigned int i=0;i<sections;i++){order[i]=sectionRecords[i*10+2];if(order[i]!=0xffffffffu)assigned++;}
#endif
 HANDLE out=CreateFileA(destination,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);unsigned int result[3]={0x314f4352,sections,assigned};if(out==INVALID_HANDLE_VALUE||!WriteFile(out,result,12,&done,0)||done!=12||!WriteFile(out,order,sections*4,&done,0)||done!=sections*4)ExitProcess(7);CloseHandle(out);ExitProcess(0);
}
