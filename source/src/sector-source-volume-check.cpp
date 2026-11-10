#include "WorldPacker.cpp"
#include "SectorPreparation.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
static bool Put(HANDLE f,const void* p,unsigned int n){DWORD done;return WriteFile(f,p,n,&done,0)&&done==n;}
extern "C" void HarnessEntry(){
 char input[MAX_PATH],output[MAX_PATH],offsetPath[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_READER_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_SECTOR_OUTPUT",output,MAX_PATH)||!GetEnvironmentVariableA("FEAR_OFFSET_INPUT",offsetPath,MAX_PATH))ExitProcess(1);
 WorldText::Vec3 offset={};DWORD done;HANDLE f=CreateFileA(offsetPath,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);if(f==INVALID_HANDLE_VALUE||GetFileSize(f,0)!=12||!ReadFile(f,&offset,12,&done,0)||done!=12)ExitProcess(2);CloseHandle(f);
 PrefabScene::Source source;if(!source.Load(input))ExitProcess(3);void* expansionStorage=VirtualAlloc(0,sizeof(PrefabScene::Expansion),MEM_COMMIT|MEM_RESERVE,4);if(!expansionStorage)ExitProcess(6);PrefabScene::Expansion* expanded=new(expansionStorage) PrefabScene::Expansion;if(!expanded->Build(input,source.scene))ExitProcess(40);PrefabScene::Arena memory;unsigned int ordinary=0,extrusions=0;
 for(const PrefabScene::Node* n=expanded->Scene().first;n;n=n->next)if(n->kind==1){if(FearBrushFlags::Equal(n->type,"Sector"))ordinary++;if(FearBrushFlags::Equal(n->type,"SemiExtrudeSector")||FearBrushFlags::Equal(n->type,"ExtrudeSector"))extrusions++;}
 f=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(f==INVALID_HANDLE_VALUE||!Put(f,&ordinary,4)||!Put(f,&extrusions,4))ExitProcess(31);
 for(const PrefabScene::Node* n=expanded->Scene().first;n;n=n->next)if(n->kind==1&&FearBrushFlags::Equal(n->type,"Sector")){
  const WorldText::Geometry& g=n->geometry;if(g.polygons<4||g.polygons>128)ExitProcess(7);SectorOrientation::Face* faces=(SectorOrientation::Face*)memory.Allocate(g.polygons*sizeof(SectorOrientation::Face));if(!faces)ExitProcess(6);
  for(unsigned int j=0;j<g.polygons;j++){const WorldText::Polygon& poly=g.faces[j];if(poly.count<3||poly.count>128)ExitProcess(7);faces[j].count=poly.count;faces[j].points=(WorldText::Vec3*)memory.Allocate(poly.count*12);if(!faces[j].points)ExitProcess(6);for(unsigned int k=0;k<poly.count;k++){if(poly.indices[k]>=g.points)ExitProcess(8);const WorldText::Vec3& p=g.vertices[poly.indices[k]];WorldText::Vec3& q=faces[j].points[k];q.x=p.x-offset.x;q.y=p.y-offset.y;q.z=p.z-offset.z;}}
  bool flipped;SectorPreparation::Failure failure;if(!SectorPreparation::Prepare(faces,g.polygons,flipped,failure))ExitProcess(20+(unsigned int)failure);unsigned int length=lstrlenA(n->name),flip=flipped?1:0;if(!Put(f,&length,4)||!Put(f,n->name,length)||!Put(f,&g.polygons,4)||!Put(f,&flip,4))ExitProcess(31);for(unsigned int j=0;j<g.polygons;j++)if(!Put(f,&faces[j].count,4)||!Put(f,&faces[j].plane,16)||!Put(f,faces[j].points,faces[j].count*12))ExitProcess(31);
 }
 CloseHandle(f);ExitProcess(0);
}
