#include "WorldPacker.cpp"
#include "VisibilityBlock.h"
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
 unsigned int bytes;unsigned char* input=readEnv("FEAR_VISIBILITY_INPUT",&bytes);if(bytes<4)ExitProcess(30);unsigned int count=*(unsigned int*)input;if(!count||count>5000||bytes<4+count*24)ExitProcess(31);
 const StreamingSplit::Bounds* bounds=(const StreamingSplit::Bounds*)(input+4);const char** names=(const char**)VirtualAlloc(0,count*4,MEM_COMMIT|MEM_RESERVE,4);if(!names)ExitProcess(32);unsigned int cursor=4+count*24;
 for(unsigned int i=0;i<count;i++){if(cursor>=bytes)ExitProcess(33);names[i]=(const char*)(input+cursor);while(cursor<bytes&&input[cursor])cursor++;if(cursor==bytes)ExitProcess(34);cursor++;}if(cursor!=bytes)ExitProcess(35);
 unsigned int contourBytes;unsigned char* contours=readEnv("FEAR_CONTOUR_INPUT",&contourBytes);unsigned int position=0;if(contourBytes<4||*(unsigned int*)contours!=count)ExitProcess(40);position=4;PrefabScene::Arena memory;SectorPortals::PreparedSector* prepared=(SectorPortals::PreparedSector*)memory.Allocate(count*sizeof(SectorPortals::PreparedSector));if(!prepared)ExitProcess(41);
 for(unsigned int i=0;i<count;i++){
  if(position+4>contourBytes)ExitProcess(42);unsigned int n=*(unsigned int*)(contours+position);position+=4;if(n!=6)ExitProcess(43);prepared[i].count=n;prepared[i].faces=(SectorOrientation::Face*)memory.Allocate(n*sizeof(SectorOrientation::Face));if(!prepared[i].faces)ExitProcess(41);
  for(unsigned int j=0;j<n;j++){if(position+4>contourBytes)ExitProcess(42);SectorOrientation::Face& face=prepared[i].faces[j];face.count=*(unsigned int*)(contours+position);position+=4;if(face.count!=4||position+face.count*12>contourBytes)ExitProcess(44);face.points=(WorldText::Vec3*)memory.Allocate(face.count*12);if(!face.points)ExitProcess(41);for(unsigned int k=0;k<face.count;k++){face.points[k]=*(WorldText::Vec3*)(contours+position);position+=12;const float* v=&face.points[k].x;for(unsigned int a=0;a<3;a++)if(v[a]<bounds[i].low[a]||v[a]>bounds[i].high[a])ExitProcess(45);}}
  bool flipped;SectorPreparation::Failure failure;if(!SectorPreparation::Prepare(prepared[i].faces,n,flipped,failure))ExitProcess(50+(unsigned int)failure);
 }
 if(position!=contourBytes)ExitProcess(46);
 VisibilityBlock::Rectangular block;if(!block.Build(bounds,names,count,prepared))ExitProcess(36);if(!GetEnvironmentVariableA("FEAR_VISIBILITY_OUTPUT",path,MAX_PATH))ExitProcess(37);HANDLE f=CreateFileA(path,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);DWORD done;if(f==INVALID_HANDLE_VALUE||!WriteFile(f,block.data,block.size,&done,0)||done!=block.size)ExitProcess(38);CloseHandle(f);ExitProcess(0);
}
