#include "WorldPacker.cpp"
#include "ExtrusionPrism.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
static bool Write(HANDLE f,const void* p,unsigned int size){DWORD done=0;return WriteFile(f,p,size,&done,0)&&done==size;}
extern "C" void HarnessEntry(){
 char input[MAX_PATH],output[MAX_PATH],offsetPath[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_READER_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_EXTRUSION_OUTPUT",output,MAX_PATH)||!GetEnvironmentVariableA("FEAR_OFFSET_INPUT",offsetPath,MAX_PATH))ExitProcess(1);
 WorldText::Vec3 offset={};DWORD done;HANDLE f=CreateFileA(offsetPath,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);if(f==INVALID_HANDLE_VALUE||GetFileSize(f,0)!=12||!ReadFile(f,&offset,12,&done,0)||done!=12||!WorldText::FiniteVector(offset))ExitProcess(2);CloseHandle(f);
 PrefabScene::Source source;if(!source.Load(input))ExitProcess(3);void* expansionStorage=VirtualAlloc(0,sizeof(PrefabScene::Expansion),MEM_COMMIT|MEM_RESERVE,4);if(!expansionStorage)ExitProcess(6);PrefabScene::Expansion* expanded=new(expansionStorage) PrefabScene::Expansion;if(!expanded->Build(input,source.scene))ExitProcess(40);
 PrefabScene::Arena memory;unsigned int candidateCount=0,apertureCount=0;const PrefabScene::Model& model=expanded->Scene();
 for(const PrefabScene::Node* n=model.first;n;n=n->next)if(n->kind==1){if(FearBrushFlags::Equal(n->type,"Sector"))candidateCount+=n->geometry.polygons;if(FearBrushFlags::Equal(n->type,"ExtrudeSector")||FearBrushFlags::Equal(n->type,"SemiExtrudeSector"))apertureCount++;}
 if(!candidateCount||candidateCount>8192||!apertureCount)ExitProcess(5);ExtrusionVolume::Candidate* candidates=(ExtrusionVolume::Candidate*)memory.Allocate((candidateCount+apertureCount*130)*sizeof(ExtrusionVolume::Candidate));if(!candidates)ExitProcess(6);unsigned int index=0;
 for(const PrefabScene::Node* n=model.first;n;n=n->next)if(n->kind==1&&FearBrushFlags::Equal(n->type,"Sector"))for(unsigned int j=0;j<n->geometry.polygons;j++){
  const WorldText::Polygon& poly=n->geometry.faces[j];if(poly.count<3||poly.count>128)ExitProcess(7);WorldText::Vec3* points=(WorldText::Vec3*)memory.Allocate(poly.count*12);if(!points)ExitProcess(6);for(unsigned int k=0;k<poly.count;k++){if(poly.indices[k]>=n->geometry.points)ExitProcess(8);const WorldText::Vec3& p=n->geometry.vertices[poly.indices[k]];points[k].x=p.x-offset.x;points[k].y=p.y-offset.y;points[k].z=p.z-offset.z;}ExtrusionVolume::Candidate& c=candidates[index++];c.points=points;c.count=poly.count;if(!PortalContour::Clean(points,c.count,c.plane))ExitProcess(9);
 }
 char volumePath[MAX_PATH];HANDLE volumeFile=INVALID_HANDLE_VALUE;if(GetEnvironmentVariableA("FEAR_EXTRUSION_VOLUME_OUTPUT",volumePath,MAX_PATH)){volumeFile=CreateFileA(volumePath,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(volumeFile==INVALID_HANDLE_VALUE||!Write(volumeFile,&apertureCount,4))ExitProcess(31);}
 f=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(f==INVALID_HANDLE_VALUE||!Write(f,&apertureCount,4)||!Write(f,&candidateCount,4))ExitProcess(31);
 for(const PrefabScene::Node* n=model.first;n;n=n->next)if(n->kind==1&&(FearBrushFlags::Equal(n->type,"ExtrudeSector")||FearBrushFlags::Equal(n->type,"SemiExtrudeSector"))){
  if(n->geometry.polygons!=1)ExitProcess(10);const WorldText::Polygon& poly=n->geometry.faces[0];if(poly.count<3||poly.count>128)ExitProcess(7);WorldText::Vec3* points=(WorldText::Vec3*)memory.Allocate(poly.count*12);if(!points)ExitProcess(6);for(unsigned int k=0;k<poly.count;k++){if(poly.indices[k]>=n->geometry.points)ExitProcess(8);const WorldText::Vec3& p=n->geometry.vertices[poly.indices[k]];points[k].x=p.x-offset.x;points[k].y=p.y-offset.y;points[k].z=p.z-offset.z;}unsigned int count=poly.count;ExtrusionVolume::Plane plane;if(!PortalContour::Clean(points,count,plane))ExitProcess(9);WorldText::Vec3 normal={plane.x,plane.y,plane.z};ExtrusionVolume::Ends ends={};if(!ExtrusionVolume::SelectEnds(points,count,normal,candidates,candidateCount,ends)||!ends.hasPositive||!ends.hasNegative)ExitProcess(11);unsigned int full=FearBrushFlags::Equal(n->type,"ExtrudeSector")?1:0;
  ExtrusionPrism::Volume volume;if(!ExtrusionPrism::Build(memory,points,count,plane,ends,full!=0,volume))ExitProcess(12);unsigned int nameLength=lstrlenA(n->name);if(volumeFile!=INVALID_HANDLE_VALUE&&(!Write(volumeFile,&nameLength,4)||!Write(volumeFile,n->name,nameLength)||!Write(volumeFile,&volume.count,4)))ExitProcess(31);for(unsigned int j=0;j<volume.count;j++){const SectorOrientation::Face& face=volume.faces[j];if(volumeFile!=INVALID_HANDLE_VALUE&&(!Write(volumeFile,&face.count,4)||!Write(volumeFile,&face.plane,16)||!Write(volumeFile,face.points,face.count*12)))ExitProcess(31);ExtrusionVolume::Candidate& c=candidates[candidateCount++];c.points=face.points;c.count=face.count;c.plane=face.plane;}
  if(!Write(f,&full,4)||!Write(f,&count,4)||!Write(f,&plane,16)||!Write(f,points,count*12)||!Write(f,&ends.positiveDistance,4)||!Write(f,&ends.negativeDistance,4)||!Write(f,&ends.positive,16)||!Write(f,&ends.negative,16))ExitProcess(31);
 }
 if(volumeFile!=INVALID_HANDLE_VALUE)CloseHandle(volumeFile);CloseHandle(f);ExitProcess(0);
}




