#include "WorldPacker.cpp"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
extern "C" void HarnessEntry(){
 char input[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_READER_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_MODEL_REPORT",output,MAX_PATH))ExitProcess(1);
 PrefabScene::Source source;if(!source.Load(input))ExitProcess(2);HANDLE f=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(f==INVALID_HANDLE_VALUE)ExitProcess(3);DWORD done;
 for(unsigned int i=0;i<source.scene.nodeCount;i++){
  const WorldText::SceneNode& n=source.scene.nodes[i];if(n.kind!=1||(!FearBrushFlags::Equal(n.type,"Sector")&&!FearBrushFlags::Equal(n.type,"ExtrudeSector")&&!FearBrushFlags::Equal(n.type,"SemiExtrudeSector")))continue;
  const WorldText::Geometry& g=source.scene.geometry[n.geometry];float bounds[6]={1e30f,1e30f,1e30f,-1e30f,-1e30f,-1e30f};for(unsigned int j=0;j<g.points;j++){const float* p=&g.vertices[j].x;for(unsigned int a=0;a<3;a++){if(p[a]<bounds[a])bounds[a]=p[a];if(p[a]>bounds[a+3])bounds[a+3]=p[a];}}
  if(!WriteFile(f,&g.polygons,4,&done,0))ExitProcess(4);
  for(unsigned int p=0;p<g.polygons;p++){const WorldText::Polygon& poly=g.faces[p];if(!WriteFile(f,&poly.count,4,&done,0))ExitProcess(4);for(unsigned int k=0;k<poly.count;k++)if(!WriteFile(f,&g.vertices[poly.indices[k]],12,&done,0))ExitProcess(4);}

 }CloseHandle(f);ExitProcess(0);
}
