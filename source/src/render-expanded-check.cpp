#include "WorldPacker.cpp"
#include "WorldModels.h"
#include "RenderBlock.h"
#include "RenderAttributes.h"
#include "RenderGroups.h"
#include "RenderSector.h"
#include "BrushRenderSettings.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
#include "RenderSceneProfile.h"
static bool AppendFace(PrefabScene::Arena& arena,RenderGroups::FaceInput& out,const WorldText::Geometry& g,const char* material,unsigned int tree,const PrefabScene::Node* owner,float creaseAngle,unsigned int tangentMethod,int selectedFace=-1){
 RenderAttributes::Mesh mesh;if(!mesh.Build(g,0xffffffff,1,0.0001f,creaseAngle,tangentMethod))return false;RenderAttributes::Vertex* vertices=(RenderAttributes::Vertex*)arena.Allocate(mesh.count*84);unsigned short* indices=(unsigned short*)arena.Allocate(mesh.triangles*6);if(!vertices||!indices)return false;
 for(unsigned int i=0;i<mesh.count;i++){vertices[i]=mesh.vertices[i];if(owner){WorldText::Vec3& p=vertices[i].position;p.x-=owner->position.x;p.y-=owner->position.y;p.z-=owner->position.z;}}
 unsigned int count=mesh.count,triangles=mesh.triangles;
 if(selectedFace>=0){
  // Calculate adjacency on the complete brush before selecting sector faces.
  if((unsigned int)selectedFace>=g.polygons)return false;unsigned int first=0;for(int i=0;i<selectedFace;i++)first+=g.faces[i].count-2;triangles=g.faces[selectedFace].count-2;count=0;
  unsigned short* remap=(unsigned short*)arena.Allocate(mesh.count*2);if(!remap)return false;for(unsigned int i=0;i<mesh.count;i++)remap[i]=65535;
  for(unsigned int i=0;i<triangles*3;i++){unsigned int id=mesh.indices[first*3+i];if(remap[id]==65535){remap[id]=(unsigned short)count;vertices[count++]=mesh.vertices[id];}indices[i]=remap[id];}
 }else for(unsigned int i=0;i<mesh.triangles*3;i++)indices[i]=mesh.indices[i];
 out.vertices=vertices;out.count=count;out.indices=indices;out.triangles=triangles;out.material=material;out.tree=tree;return true;
}
extern "C" void HarnessEntry(){
 char input[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_READER_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_RENDER_EXPANDED",output,MAX_PATH))ExitProcess(1);PrefabScene::Source source;if(!source.Load(input))ExitProcess(2);void* expansionStorage=VirtualAlloc(0,sizeof(PrefabScene::Expansion),MEM_COMMIT|MEM_RESERVE,4);if(!expansionStorage)ExitProcess(3);PrefabScene::Expansion* expanded=new(expansionStorage) PrefabScene::Expansion;if(!expanded->Build(input,source.scene))ExitProcess(3);const PrefabScene::Model& model=expanded->Scene();WorldModels::Partition partition;if(!partition.Build(model,Resolve,0))ExitProcess(4);if(partition.groupCount!=3)ExitProcess(5);
 PrefabScene::Arena arena;RenderGroups::FaceInput* faces=(RenderGroups::FaceInput*)arena.Allocate(model.polygons*sizeof(RenderGroups::FaceInput));unsigned int used=0;if(!faces)ExitProcess(6);
 for(unsigned int a=0;a<partition.assignmentCount;a++){const WorldModels::Assignment& assignment=partition.assignments[a];const PrefabScene::Node& node=*assignment.brush;if(!node.knownType)ExitProcess(7);if(!(node.surfaceFlags&0x20))continue;WorldText::Geometry geometry=node.geometry;const WorldText::Geometry& g=geometry;const PrefabScene::Node* owner=partition.groups[assignment.group].owner;float crease=45;unsigned int tangentMethod=BrushRenderSettings::World;
  for(const PrefabScene::Property* p=node.properties;p;p=p->next){
   if(FearBrushFlags::Equal(p->name,"TangentMethod")){
    BrushRenderSettings::TangentMethod method=BrushRenderSettings::ParseTangent(p->data);
    if(!p->string||method==BrushRenderSettings::Unknown)ExitProcess(19);
    tangentMethod=method;
   }
   if(FearBrushFlags::Equal(p->name,"ShadowLOD")&&(!p->string||!BrushRenderSettings::KnownLOD(p->data)))ExitProcess(20);
  }
  for(const PrefabScene::Property* property=node.properties;property;property=property->next)if(FearBrushFlags::Equal(property->name,"CreaseAngle")){WorldText::Document doc;if(!doc.Parse(property->data,lstrlenA(property->data))||doc.Elements(0)!=1||!doc.Number(doc.First(0),crease))ExitProcess(14);}
  if(owner){for(unsigned int i=0;i<3;i++)for(unsigned int j=0;j<3;j++)if(owner->orientation.m[i][j]!=(i==j?1.f:0.f))ExitProcess(8);
   // Keep source-space plane normals through identity prefab translation. Recomputing
   // Newell normals after adding the world offset introduces cancellation error.
   PrefabScene::Source original;if(!original.Load(node.source))ExitProcess(16);const WorldText::Geometry* originalGeometry=0;
   for(unsigned int i=0;i<original.scene.nodeCount;i++){const WorldText::SceneNode& n=original.scene.nodes[i];if(n.kind==1&&n.id==node.sourceId)originalGeometry=&original.scene.geometry[n.geometry];}
   if(!originalGeometry||originalGeometry->polygons!=g.polygons||originalGeometry->points!=g.points)ExitProcess(17);
   // Brush rotation properties are not the prefab transform. Verify the actual
   // source/expanded geometry differs only by translation before preserving planes.
   WorldText::Vec3 shift={g.vertices[0].x-originalGeometry->vertices[0].x,g.vertices[0].y-originalGeometry->vertices[0].y,g.vertices[0].z-originalGeometry->vertices[0].z};
   for(unsigned int v=0;v<g.points;v++){const WorldText::Vec3& p=originalGeometry->vertices[v];const WorldText::Vec3& q=g.vertices[v];if(q.x!=p.x+shift.x||q.y!=p.y+shift.y||q.z!=p.z+shift.z)ExitProcess(15);}
   geometry.faces=(WorldText::Polygon*)arena.Allocate(g.polygons*sizeof(WorldText::Polygon));if(!geometry.faces)ExitProcess(18);
   for(unsigned int i=0;i<g.polygons;i++){geometry.faces[i]=node.geometry.faces[i];geometry.faces[i].normal=originalGeometry->faces[i].normal;geometry.faces[i].distance=WorldText::Dot(geometry.faces[i].normal,g.vertices[geometry.faces[i].indices[0]]);}
for(unsigned int i=1;i<g.polygons;i++)if(!RenderBlock::SameMaterial(g.faces[0].material,g.faces[i].material))ExitProcess(9);if(!AppendFace(arena,faces[used++],g,g.faces[0].material,assignment.group,owner,crease,tangentMethod))ExitProcess(10);}
  else for(unsigned int i=0;i<g.polygons;i++){if(!Assigned(model,g,g.faces[i]))continue;if(!AppendFace(arena,faces[used++],g,g.faces[i].material,0,0,crease,tangentMethod,(int)i))ExitProcess(11);}
 }
 RenderGroups::Groups groups;if(!groups.Build(faces,used))ExitProcess(12);HANDLE f=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);DWORD done;if(f==INVALID_HANDLE_VALUE)ExitProcess(13);unsigned int header[3]={0x31534752,groups.count,partition.groupCount};WriteFile(f,header,12,&done,0);
 for(unsigned int i=0;i<groups.count;i++){const RenderBlock::SurfaceInput& s=groups.surfaces[i];unsigned int h[4]={s.tree,s.count,s.triangles,(unsigned int)lstrlenA(s.material)};WriteFile(f,h,16,&done,0);WriteFile(f,s.material,h[3],&done,0);WriteFile(f,s.vertices,s.count*84,&done,0);WriteFile(f,s.indices,s.triangles*6,&done,0);}CloseHandle(f);ExitProcess(0);
}
