#include "WorldPacker.cpp"
#include "SectorPreparation.h"
#include "VisibilityTree.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
struct Sector {SectorOrientation::Face* faces;unsigned int count;bool flipped;};
extern "C" void HarnessEntry(){
 char input[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_READER_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_TREE_OUTPUT",output,MAX_PATH))ExitProcess(1);
 HANDLE in=CreateFileA(input,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);DWORD done;unsigned int count=0;if(in==INVALID_HANDLE_VALUE||!ReadFile(in,&count,4,&done,0)||done!=4||!count||count>5000)ExitProcess(2);
 PrefabScene::Arena memory;Sector* sectors=(Sector*)memory.Allocate(count*sizeof(Sector));StreamingSplit::Bounds* boxes=(StreamingSplit::Bounds*)memory.Allocate(count*sizeof(StreamingSplit::Bounds));if(!sectors||!boxes)ExitProcess(3);
 for(unsigned int i=0;i<count;i++){
  Sector& s=sectors[i];if(!ReadFile(in,&s.count,4,&done,0)||done!=4||s.count<4||s.count>128)ExitProcess(4);s.faces=(SectorOrientation::Face*)memory.Allocate(s.count*sizeof(SectorOrientation::Face));if(!s.faces)ExitProcess(3);
  for(unsigned int a=0;a<3;a++){boxes[i].low[a]=1e30f;boxes[i].high[a]=-1e30f;}
  for(unsigned int j=0;j<s.count;j++){SectorOrientation::Face& f=s.faces[j];if(!ReadFile(in,&f.count,4,&done,0)||done!=4||f.count<3||f.count>128)ExitProcess(5);f.points=(WorldText::Vec3*)memory.Allocate(f.count*12);if(!f.points||!ReadFile(in,f.points,f.count*12,&done,0)||done!=f.count*12)ExitProcess(6);for(unsigned int k=0;k<f.count;k++)for(unsigned int a=0;a<3;a++){float v=(&f.points[k].x)[a];if(v<boxes[i].low[a])boxes[i].low[a]=v;if(v>boxes[i].high[a])boxes[i].high[a]=v;}}
  SectorPreparation::Failure failure;if(!SectorPreparation::Prepare(s.faces,s.count,s.flipped,failure))ExitProcess(20+(unsigned int)failure);
 }
 CloseHandle(in);VisibilityTree::Builder tree;if(!tree.Build(boxes,count))ExitProcess(7);HANDLE out=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(out==INVALID_HANDLE_VALUE||!WriteFile(out,&count,4,&done,0)||done!=4)ExitProcess(8);
 for(unsigned int i=0;i<count;i++){Sector& s=sectors[i];unsigned int header[]={tree.saveIds[i],s.count,s.flipped?1u:0u};if(!WriteFile(out,header,12,&done,0)||done!=12)ExitProcess(9);for(unsigned int j=0;j<s.count;j++){SectorOrientation::Face& f=s.faces[j];if(!WriteFile(out,&f.count,4,&done,0)||done!=4||!WriteFile(out,f.points,f.count*12,&done,0)||done!=f.count*12||!WriteFile(out,&f.plane,16,&done,0)||done!=16)ExitProcess(9);}}
 CloseHandle(out);ExitProcess(0);
}
