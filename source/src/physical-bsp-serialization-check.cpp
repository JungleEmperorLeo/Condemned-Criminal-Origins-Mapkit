#include "WorldPacker.cpp"
#include "BspTree.h"
#include "BspNodeTable.h"
#include "BspBlock.h"
#include "WorldModelsBlock.h"
#include "WorldModelsSerialization.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
static bool Read(HANDLE file,void* target,unsigned int size){DWORD done=0;return !size||(ReadFile(file,target,size,&done,0)&&done==size);}
static bool Write(HANDLE file,const void* source,unsigned int size){DWORD done=0;return !size||(WriteFile(file,source,size,&done,0)&&done==size);}
static void Error(const char* message,unsigned int code){LogLine(message?message:"physical BSP serializer failure");ExitProcess(code);}
extern "C" void HarnessEntry(){
 char input[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_BSP_PREPARED_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_BSP_PREPARED_OUTPUT",output,MAX_PATH))ExitProcess(1);
 HANDLE file=CreateFileA(input,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);unsigned int header[4];
 if(file==INVALID_HANDLE_VALUE||!Read(file,header,16)||header[0]!=0x31534250u||header[1]>WorldModelsSerialization::MaximumNormals||header[2]>WorldModelsSerialization::MaximumModels||header[3]>WorldModelsSerialization::MaximumNames)ExitProcess(2);
 PrefabScene::Arena memory;WorldText::Vec3* normals=(WorldText::Vec3*)memory.Allocate((header[1]+1)*12);WorldModelsSerialization::Name* names=(WorldModelsSerialization::Name*)memory.Allocate((header[3]+1)*sizeof(WorldModelsSerialization::Name));
 PhysicalBspSerialization::Block** models=(PhysicalBspSerialization::Block**)memory.Allocate((header[2]+1)*sizeof(void*));if(!normals||!names||!models||!Read(file,normals,header[1]*12))ExitProcess(3);
 for(unsigned int n=0;n<header[3];n++){
  unsigned int fields[2];if(!Read(file,fields,8)||!fields[0]||fields[0]>255)ExitProcess(3);char* text=(char*)memory.Allocate(fields[0]+1);if(!text||!Read(file,text,fields[0]))ExitProcess(3);text[fields[0]]=0;if(lstrlenA(text)!=(int)fields[0])ExitProcess(3);names[n].text=text;names[n].model=fields[1];
 }
 for(unsigned int m=0;m<header[2];m++){
  PhysicalBspSerialization::Header fields;if(!Read(file,&fields,48)||fields.points>PhysicalBspSerialization::MaximumPoints||fields.polygons>PhysicalBspSerialization::MaximumPolygons||fields.nodes>PhysicalBspSerialization::MaximumNodes||fields.references>PhysicalBspSerialization::MaximumReferences)ExitProcess(3);
  WorldText::Vec3* points=(WorldText::Vec3*)memory.Allocate((fields.points+1)*12);PhysicalBspSerialization::Polygon* polygons=(PhysicalBspSerialization::Polygon*)memory.Allocate((fields.polygons+1)*sizeof(PhysicalBspSerialization::Polygon));BspNodeTable::Record* nodes=(BspNodeTable::Record*)memory.Allocate((fields.nodes+1)*12);if(!points||!polygons||!nodes||!Read(file,points,fields.points*12))ExitProcess(3);
  for(unsigned int p=0;p<fields.polygons;p++){
   PhysicalBspSerialization::Polygon& polygon=polygons[p];unsigned int parameters[3];if(!Read(file,parameters,12)||parameters[0]>255)ExitProcess(3);polygon.count=parameters[0];polygon.normal=parameters[1];polygon.flags=(unsigned char)parameters[2];polygon.occlusion=(unsigned char)(parameters[2]>>8);polygon.material=(unsigned short)(parameters[2]>>16);
   unsigned int* indices=(unsigned int*)memory.Allocate((polygon.count+1)*4);polygon.indices=indices;if(!indices||!Read(file,&polygon.distance,4)||!Read(file,indices,polygon.count*4))ExitProcess(3);
  }
  if(!Read(file,nodes,fields.nodes*12))ExitProcess(3);
  PhysicalBspSerialization::Input prepared;prepared.points=points;prepared.pointCount=fields.points;prepared.polygons=polygons;prepared.polygonCount=fields.polygons;prepared.nodes=nodes;prepared.nodeCount=fields.nodes;prepared.normalCount=header[1];prepared.referenceReservation=fields.references;prepared.flags=fields.flags;prepared.half=fields.half;prepared.center=fields.center;prepared.root=fields.root;
  void* modelStorage=memory.Allocate(sizeof(PhysicalBspSerialization::Block));if(!modelStorage)ExitProcess(3);models[m]=new(modelStorage)PhysicalBspSerialization::Block;
  if(!models[m]->Build(prepared))Error(models[m]->error,4);
 }
 unsigned char tail;DWORD done=0;if(!ReadFile(file,&tail,1,&done,0)||done)ExitProcess(3);CloseHandle(file);
 WorldModelsSerialization::Block block;if(!block.Build(names,header[3],normals,header[1],models,header[2]))Error(block.error,5);
 // Input contains prepared metadata; it may be a source fixture or an explicitly
 // labelled comparison-only writer replay. This harness is not a source compiler.
 file=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(file==INVALID_HANDLE_VALUE||!Write(file,block.data,block.size))ExitProcess(6);CloseHandle(file);
 for(unsigned int m=0;m<header[2];m++)models[m]->~Block();ExitProcess(0);
}
