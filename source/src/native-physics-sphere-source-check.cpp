#define FEAR_PRIMARY_SURFACE_PREPARATION 1
#include "WorldPacker.cpp"
#include "NativePhysicsOwners.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
static bool Write(HANDLE f,const void* p,unsigned int n){DWORD done=0;return !n||(WriteFile(f,p,n,&done,0)&&done==n);}
static bool U(HANDLE f,unsigned int n){return Write(f,&n,4);}
extern "C" void HarnessEntry(){
 char source[MAX_PATH],project[MAX_PATH],input[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_READER_INPUT",source,MAX_PATH)||!GetEnvironmentVariableA("FEAR_READER_PROJECT",project,MAX_PATH)||!GetEnvironmentVariableA("FEAR_SPHERE_SOURCE_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_SPHERE_SOURCE_OUTPUT",output,MAX_PATH))ExitProcess(1);
 PrefabScene::Arena memory;void* place=memory.Allocate(sizeof(NativePreparedRenderScene::Context));if(!place)ExitProcess(2);NativePreparedRenderScene::Context* scene=new(place)NativePreparedRenderScene::Context;if(!scene->Build(source,project)){LogLine(scene->Diagnostic().message);ExitProcess(2);}
 const PrefabScene::Model& model=scene->Model();const WorldModels::Partition& partition=scene->Partition();const bool navigation=SourcePhysicalBspContours::HasNavigation(model);
 PrefabScene::Property* properties=(PrefabScene::Property*)memory.Allocate(partition.groupCount*sizeof(PrefabScene::Property));if(!properties)ExitProcess(2);
 // A test-only source property override exercises the universal mode on all
 // retained owners. No override exists in the production writer or worker.
 for(unsigned int g=1;g<partition.groupCount;g++){PrefabScene::Node& owner=*const_cast<PrefabScene::Node*>(partition.groups[g].owner);PrefabScene::Property& p=properties[g];lstrcpyA(p.name,"PhysicsShape");lstrcpyA(p.type,"string");p.data=const_cast<char*>("None");p.string=true;p.next=owner.properties;owner.properties=&p;}
 NativePhysicsOwners::Builder base;if(!base.Build(*scene)){LogLine(base.error);ExitProcess(3);}
 HANDLE inp=CreateFileA(input,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(inp==INVALID_HANDLE_VALUE||!U(inp,base.count))ExitProcess(4);
 // Independent source enumeration exports the original writer's CPoly input.
 // Cached bounds are computed before extraction, then blocker polygons are
 // omitted from the linked list, matching the stage3 source contour contract.
 for(const NativePhysicsOwners::Record* record=base.first;record;record=record->next){
  unsigned int g=record->group,polygonCount=0;const PrefabScene::Node& owner=*partition.groups[g].owner;PhysicsOwnerBounds::Bounds bounds;
  for(unsigned int a=0;a<partition.assignmentCount;a++){const WorldModels::Assignment& item=partition.assignments[a];if(item.group!=g||!SourcePhysicalBspContours::DiskCandidate(*item.brush,navigation))continue;const WorldText::Geometry& geometry=item.brush->geometry;
   for(unsigned int p=0;p<geometry.polygons;p++)for(unsigned int f=0;f<SourceSurfacePreparation::FragmentCount(geometry.faces[p].count);f++){SourceSurfacePreparation::Fragment fragment;if(!SourceSurfacePreparation::FragmentAt(geometry.faces[p],f,fragment))ExitProcess(5);if(!(item.brush->surfaceFlags&0x180))polygonCount++;
    for(unsigned int v=0;v<fragment.count;v++){WorldText::Vec3 local;if(!SourceOwnerRotation::Local(owner,geometry.vertices[fragment.indices[v]],scene->Offset(),local)||!bounds.Add(local))ExitProcess(5);}
   }
  }
  WorldText::Vec3 center={};if(bounds.count)for(unsigned int k=0;k<3;k++){volatile float sum=(&bounds.high.x)[k]+(&bounds.low.x)[k];(&center.x)[k]=sum*.5f;}
  float values[5];if(!NativePhysicsProperties::MassDensity(owner,false,values[0],values[1]))ExitProcess(5);memcpy(values+2,&center,12);unsigned int wrapper=record->children?1:0;
  if(!U(inp,polygonCount)||!U(inp,wrapper)||!Write(inp,values,20))ExitProcess(4);
  for(unsigned int a=0;a<partition.assignmentCount;a++){const WorldModels::Assignment& item=partition.assignments[a];if(item.group!=g||!SourcePhysicalBspContours::DiskCandidate(*item.brush,navigation)||(item.brush->surfaceFlags&0x180))continue;const WorldText::Geometry& geometry=item.brush->geometry;
   for(unsigned int p=0;p<geometry.polygons;p++)for(unsigned int f=0;f<SourceSurfacePreparation::FragmentCount(geometry.faces[p].count);f++){SourceSurfacePreparation::Fragment fragment;if(!SourceSurfacePreparation::FragmentAt(geometry.faces[p],f,fragment)||!U(inp,item.brush->surfaceFlags)||!U(inp,fragment.count))ExitProcess(5);
    for(unsigned int v=0;v<fragment.count;v++){WorldText::Vec3 local;if(!SourceOwnerRotation::Local(owner,geometry.vertices[fragment.indices[v]],scene->Offset(),local)||!Write(inp,&local,12))ExitProcess(5);}
   }
  }
 }
 CloseHandle(inp);HANDLE out=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(out==INVALID_HANDLE_VALUE||!U(out,base.count))ExitProcess(4);
 // Exercise the actual aggregate owner writer one requested Sphere at a time.
 // This preserves clear-on-failure while allowing every valid record to be
 // compared even when another model makes the original emit a NaN sphere.
 NativePhysicsOwners::Builder result;
 for(const NativePhysicsOwners::Record* baseRecord=base.first;baseRecord;baseRecord=baseRecord->next){
  const unsigned int group=baseRecord->group;properties[group].data=const_cast<char*>("Sphere");bool built=result.Build(*scene);properties[group].data=const_cast<char*>("None");
  if(!built){if(result.first||result.last||result.data||result.size||result.count||result.sphereCount||result.failedGroup!=group||!result.error)ExitProcess(7);unsigned int n=(unsigned int)lstrlenA(result.error);if(!U(out,0)||!U(out,group)||!U(out,n)||!Write(out,result.error,n))ExitProcess(4);continue;}
  if(result.count!=base.count||result.sphereCount!=1)ExitProcess(7);const NativePhysicsOwners::Record* record=result.first;while(record&&record->group!=group)record=record->next;if(!record)ExitProcess(7);
  const unsigned int wrapper=record->type==6?1u:0u,n=wrapper?48u:44u;const unsigned char* sphere=record->data+(wrapper?36u:0u);if(record->bytes<n+(wrapper?36u:0u)||*(unsigned int*)(sphere+(wrapper?32u:28u))!=3||!U(out,n)||!Write(out,sphere,n))ExitProcess(6);
 }
 CloseHandle(out);scene->Clear();if(!result.error&&(!result.data||!result.size||result.sphereCount!=1))ExitProcess(7);scene->~Context();ExitProcess(0);
}
