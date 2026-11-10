#include "WorldPacker.cpp"
#include "BspTree.h"
#include "BspNodeTable.h"
#include "BspBlock.h"
#include "WorldModelsBlock.h"
#include "WorldPrefix.h"
#include "PhysicsMeshPrefix.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
static char input[MAX_PATH];
static HANDLE report;
static void Raw(const char* s){DWORD done;if(!WriteFile(report,s,lstrlenA(s),&done,0)||done!=(unsigned int)lstrlenA(s))ExitProcess(12);}
static void Num(unsigned int n){char text[16];char* end=Decimal(text,n);*end=0;Raw(text);}
static unsigned int visited,stored;
static BspNodeTable::Record selectedRecords[6];
static bool Candidate(const PrefabScene::Node& n){return n.knownType&&(n.surfaceFlags&8)!=0;}
static bool SideR(const BspTree::Branch* b,const BspSplit::Plane& plane,int side){
 if(!b)return true;for(BspTree::Item* i=b->coplanar;i;i=i->next)for(unsigned int k=0;k<i->fragment.count;k++){int s=plane.Classify(i->fragment.vertices[k]);if(s&&s!=side)return false;}
 return SideR(b->front,plane,side)&&SideR(b->back,plane,side);
}
static bool Check(const BspTree::Branch* b,unsigned int depth){
 if(!b)return true;if(b->depth!=depth||!b->coplanar)return false;visited++;
 for(BspTree::Item* i=b->coplanar;i;i=i->next){stored++;const BspSplit::Fragment& f=i->fragment;
  for(unsigned int k=0;k<f.count;k++)if(b->plane.Classify(f.vertices[k]))return false;
  WorldText::Geometry g={};g.points=f.count;g.vertices=f.vertices;g.polygons=1;g.faces=(WorldText::Polygon*)&f.polygon;
  if(PolygonCheck::Check(g,f.polygon).issues)return false;
 }
 return SideR(b->front,b->plane,1)&&SideR(b->back,b->plane,-1)&&Check(b->front,depth+1)&&Check(b->back,depth+1);
}
static double Area(const BspSplit::Fragment& f){double x=0,y=0,z=0;for(unsigned int i=0;i<f.count;i++){const WorldText::Vec3& a=f.vertices[i];const WorldText::Vec3& b=f.vertices[(i+1)%f.count];x+=(double)a.y*b.z-(double)a.z*b.y;y+=(double)a.z*b.x-(double)a.x*b.z;z+=(double)a.x*b.y-(double)a.y*b.x;}return BspSplit::Sqrt(x*x+y*y+z*z)*.5;}
static double Total(const BspTree::Branch* b){if(!b)return 0;double sum=Total(b->front)+Total(b->back);for(BspTree::Item* i=b->coplanar;i;i=i->next)sum+=Area(i->fragment);return sum;}
static bool Validate(BspTree::Tree& tree){visited=stored=0;return Check(tree.root,0)&&visited==tree.branches&&stored==tree.storedPolygons&&stored==tree.inputPolygons+tree.splits;}
extern "C" void HarnessEntry(){
 BspBlock::Block selectedBlock;
 WorldModelsBlock::Block worldModels;
 WorldPrefix::Block prefix;
 WorldPrefix::Block relocationCheck;
 PhysicsMeshPrefix::Block meshPrefix;
 if(!GetEnvironmentVariableA("FEAR_READER_INPUT",input,MAX_PATH))ExitProcess(1);
 void* sceneStorage=VirtualAlloc(0,sizeof(PrefabScene::Expansion),MEM_COMMIT|MEM_RESERVE,4);if(!sceneStorage)ExitProcess(2);
 PrefabScene::Expansion* expansion=new(sceneStorage) PrefabScene::Expansion;{PrefabScene::Source source;if(!source.Load(input)||!expansion->Build(input,source.scene))ExitProcess(2);}
 BspTree::Tree tree;if(!tree.Build(expansion->Scene())||tree.inputPolygons!=12||!Validate(tree))ExitProcess(3);
 double area=Total(tree.root);unsigned int branchCount=tree.branches,minimalDepth=tree.maxDepth,minimalSplits=tree.splits;
 if(!tree.Build(expansion->Scene(),.001f,128,100000,Candidate)||tree.inputPolygons!=6||tree.branches!=6||tree.splits||tree.storedPolygons!=6||!Validate(tree))ExitProcess(13);
 double selectedArea=Total(tree.root);unsigned int selectedDepth=tree.maxDepth;
 if(!tree.Build(expansion->Scene(),.001f,128,100000,Candidate,true)||!Validate(tree))ExitProcess(14);
 unsigned int recordCount=0;if(!BspNodeTable::ConvexChain(tree.root,selectedRecords,6,-1,-2,recordCount)||recordCount!=6)ExitProcess(15);
 for(unsigned int i=0;i<6;i++)if(selectedRecords[i].polygon!=(int)i||selectedRecords[i].positive!=-1||selectedRecords[i].negative!=(i==5?-2:(int)i+1))ExitProcess(16);
 const PrefabScene::Node* selectedBrush=expansion->Scene().first;while(selectedBrush&&!Candidate(*selectedBrush))selectedBrush=selectedBrush->next;
 WorldText::Vec3 normals[6]={{0,0,1},{0,0,-1},{0,1,0},{0,-1,0},{1,0,0},{-1,0,0}};
 BspBlock::SurfaceMetadata metadata[6]={};for(unsigned int i=0;i<6;i++)metadata[i].occlusion=255;
 if(!selectedBrush)ExitProcess(17);
 if(selectedBlock.Build(*selectedBrush,tree,normals,1,metadata,6,1,-1,-2)||selectedBlock.data||selectedBlock.size||!selectedBlock.error)ExitProcess(18);
 if(selectedBlock.Build(*selectedBrush,tree,normals,6,metadata,5,1,-1,-2)||selectedBlock.data||selectedBlock.size||!selectedBlock.error)ExitProcess(19);
 if(!selectedBrush||!selectedBlock.Build(*selectedBrush,tree,normals,6,metadata,6,1,-1,-2)||selectedBlock.size!=390)ExitProcess(17);
 if(!worldModels.BuildSingle(*selectedBrush,tree,metadata,6,"PhysicsBSP",1,-1,-2)||worldModels.size!=514||worldModels.normalCount!=6)ExitProcess(20);
 WorldText::Vec3 offset={};if(!prefix.BuildLeaf(selectedBrush->geometry,worldModels,offset,true)||prefix.size!=611||prefix.relocated)ExitProcess(21);
 if(prefix.SetOffsets(1,2,3,4,5)||prefix.relocated)ExitProcess(22);
 if(!relocationCheck.BuildLeaf(selectedBrush->geometry,worldModels,offset,true)||!relocationCheck.SetOffsets(1000,1100,1200,1300,1400)||!relocationCheck.relocated||((WorldPrefix::Header*)relocationCheck.data)->render!=1000)ExitProcess(24);
 if(!meshPrefix.BuildStatic(selectedBrush->geometry,.01f,1,1.1f)||meshPrefix.size!=276)ExitProcess(23);
 expansion->~Expansion();VirtualFree(sceneStorage,0,MEM_RELEASE); // Tree owns contours/mappings.
 if(!Validate(tree)||Total(tree.root)!=selectedArea||!branchCount||area<=0)ExitProcess(4);
 PrefabScene::Model model;PrefabScene::Node* n=model.Add();n->kind=1;model.brushes=1;
 WorldText::Vec3 vertices[8]={{-2,-2,0},{2,-2,0},{2,2,0},{-2,2,0},{0,-2,-2},{0,2,-2},{0,2,2},{0,-2,2}};
 unsigned int indices[8]={0,1,2,3,4,5,6,7};WorldText::Polygon polys[2]={};
 for(unsigned int i=0;i<2;i++){polys[i].count=4;polys[i].indices=indices+4*i;polys[i].validPlane=WorldText::ComputePlane(vertices,polys[i].indices,4,polys[i].normal,polys[i].distance);}
 n->geometry.points=8;n->geometry.vertices=vertices;n->geometry.polygons=2;n->geometry.faces=polys;
 if(!tree.Build(model)||tree.splits!=1||tree.storedPolygons!=3||!Validate(tree))ExitProcess(5);
 double splitArea=Total(tree.root);if(splitArea<31.99999||splitArea>32.00001)ExitProcess(6);
 if(tree.Build(model,.001f,1)||tree.root||!tree.error)ExitProcess(7);
 if(tree.Build(model,.001f,128,1)||tree.root||!tree.error)ExitProcess(8);
 if(tree.Build(model,-1)||tree.root||!tree.error)ExitProcess(9);
 if(!tree.Build(model)||!Validate(tree)||Total(tree.root)!=splitArea)ExitProcess(10);
 tree.Clear();PrefabScene::Model empty;if(!tree.Build(empty)||tree.root||tree.inputPolygons)ExitProcess(11);
 if(GetEnvironmentVariableA("FEAR_PHYSICS_MESH_PREFIX",input,MAX_PATH)){
  report=CreateFileA(input,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(report==INVALID_HANDLE_VALUE)ExitProcess(12);
  DWORD written;if(!WriteFile(report,meshPrefix.data,meshPrefix.size,&written,0)||written!=meshPrefix.size)ExitProcess(12);CloseHandle(report);
 }
 if(GetEnvironmentVariableA("FEAR_WORLD_PREFIX",input,MAX_PATH)){
  report=CreateFileA(input,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(report==INVALID_HANDLE_VALUE)ExitProcess(12);
  DWORD written;if(!WriteFile(report,prefix.data,prefix.size,&written,0)||written!=prefix.size)ExitProcess(12);CloseHandle(report);
 }
 if(GetEnvironmentVariableA("FEAR_WORLD_MODELS_BLOCK",input,MAX_PATH)){
  report=CreateFileA(input,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(report==INVALID_HANDLE_VALUE)ExitProcess(12);
  DWORD written;if(!WriteFile(report,worldModels.data,worldModels.size,&written,0)||written!=worldModels.size)ExitProcess(12);CloseHandle(report);
 }
 if(GetEnvironmentVariableA("FEAR_BSP_BLOCK",input,MAX_PATH)){
  report=CreateFileA(input,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(report==INVALID_HANDLE_VALUE)ExitProcess(12);
  DWORD written;if(!WriteFile(report,selectedBlock.data,selectedBlock.size,&written,0)||written!=selectedBlock.size)ExitProcess(12);CloseHandle(report);
 }
 if(GetEnvironmentVariableA("FEAR_BSP_NODE_TABLE",input,MAX_PATH)){
  report=CreateFileA(input,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(report==INVALID_HANDLE_VALUE)ExitProcess(12);
  DWORD written;if(!WriteFile(report,selectedRecords,sizeof(selectedRecords),&written,0)||written!=sizeof(selectedRecords))ExitProcess(12);CloseHandle(report);
 }
 if(GetEnvironmentVariableA("FEAR_BSP_REPORT",input,MAX_PATH)){
  report=CreateFileA(input,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(report==INVALID_HANDLE_VALUE)ExitProcess(12);
  Raw("{\"checksPassed\":23,\"minimalBox\":{\"inputPolygons\":12,\"branches\":");Num(branchCount);Raw(",\"depth\":");Num(minimalDepth);Raw(",\"splits\":");Num(minimalSplits);
  Raw("},\"selectedMinimalBox\":{\"inputPolygons\":6,\"branches\":6,\"splits\":0,\"depth\":");Num(selectedDepth);
  Raw("},\"crossingContours\":{\"inputPolygons\":2,\"splits\":1,\"storedPolygons\":3,\"totalArea\":32},\"modelDestroyedCheckPassed\":true,\"solidLeavesImplemented\":false,\"fearParityVerified\":false}");CloseHandle(report);
 }
 ExitProcess(0);
}
