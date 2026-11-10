#include "WorldPacker.cpp"
#include "RenderBlock.h"
#include "RenderAttributes.h"
#include "RenderGroups.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
extern "C" void HarnessEntry(){
 RenderAttributes::Vertex vertices[3]={};vertices[1].position.x=1;vertices[2].position.y=1;unsigned short ids[3]={0,1,2};char material[]="Materials/A.Mat00";
 RenderGroups::FaceInput faces[4]={{vertices,3,ids,1,material,0},{vertices,3,ids,1,material,0},{vertices,3,ids,1,"Materials/B.Mat00",0},{vertices,3,ids,1,material,1}};
 RenderGroups::Groups groups;if(!groups.Build(faces,4)||groups.count!=3||groups.surfaces[0].count!=3||groups.surfaces[0].triangles!=2||groups.surfaces[1].tree!=0||groups.surfaces[2].tree!=1)ExitProcess(1);
 material[0]='X';vertices[0].position.x=99;ids[0]=2;if(groups.surfaces[0].material[0]!='M'||((RenderAttributes::Vertex*)groups.surfaces[0].vertices)[0].position.x!=0||groups.surfaces[0].indices[0]!=0)ExitProcess(2);
 unsigned char decl[16]={0,0,0,0,2,0,0,0,255,0,0,0,17,0,0,0};RenderBlock::Block block;if(!block.BuildMultiple(groups.surfaces,groups.count,2,84,decl,16))ExitProcess(3);
 ids[0]=3;if(groups.Build(faces,4)||groups.surfaces||groups.count)ExitProcess(4);ids[0]=0;faces[0].material="";if(groups.Build(faces,4)||groups.surfaces)ExitProcess(5);
 ExitProcess(0);
}
