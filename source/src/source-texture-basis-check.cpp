#include "WorldPacker.cpp"
#include "SourceTextureBasis.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
extern "C" void HarnessEntry(){
 char path[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_READER_INPUT",path,MAX_PATH))ExitProcess(1);PrefabScene::Source source;if(!source.Load(path))ExitProcess(2);
 unsigned int checked=0;WorldText::Vec3 offset={128,-256,512};
 for(unsigned int g=0;g<source.scene.geometryCount;g++){
 const WorldText::Geometry& geometry=source.scene.geometry[g];for(unsigned int p=0;p<geometry.polygons;p++){
 const WorldText::Polygon& polygon=geometry.faces[p];RenderTextureCoordinates::Basis base[4],centered[4];WorldText::Vec3 zero={};
 if(!SourceTextureBasis::Build(polygon,zero,base))continue;if(!SourceTextureBasis::Build(polygon,offset,centered))ExitProcess(3);
 for(unsigned int j=0;j<polygon.count;j++){WorldText::Vec3 point=geometry.vertices[polygon.indices[j]],moved={point.x-offset.x,point.y-offset.y,point.z-offset.z};float a[8],b[8];if(!RenderTextureCoordinates::Build(point,base,a)||!RenderTextureCoordinates::Build(moved,centered,b))ExitProcess(4);for(unsigned int k=0;k<8;k++){float difference=a[k]-b[k];if(difference<0)difference=-difference;float magnitude=a[k]<0?-a[k]:a[k];if(difference>0.0001f*(1+magnitude))ExitProcess(5);}}checked++;
 }}if(!checked)ExitProcess(6);ExitProcess(0);
}

