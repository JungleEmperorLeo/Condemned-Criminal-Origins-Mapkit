#define FEAR_PRIMARY_SURFACE_PREPARATION 1
#include "WorldPacker.cpp"
#include "NativePhysicsOwners.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
static void Product(const float* b,const float* a,float* q){
 q[0]=(float)(((double(a[3])*b[0]+double(b[1])*a[2])+double(b[3])*a[0])-double(b[2])*a[1]);
 q[1]=(float)(((double(b[3])*a[1]-double(b[0])*a[2])+double(a[3])*b[1])+double(b[2])*a[0]);
 q[2]=(float)(((double(b[3])*a[2]+double(a[1])*b[0])-double(b[1])*a[0])+double(a[3])*b[2]);
 q[3]=(float)(((double(b[3])*a[3]-double(a[0])*b[0])-double(a[1])*b[1])-double(b[2])*a[2]);
}
static bool SourceQuaternion(const PrefabScene::Node& node,float* q){WorldText::Vec3 r;PrefabScene::Matrix m;return PhysicsExplicitShapes::Vector(node,"Rotation",r)&&PrefabScene::Matrix::Euler(r,m)&&SourceOwnerRotation::MatrixQuaternion(m,q);}
struct OwnerRow {unsigned int group,index;char name[256];WorldText::Vec3 all,physics,disk,weldedPhysics,physicsLow,physicsHigh,allLow,allHigh,finished;};
struct ShapeRow {unsigned int group,index;char owner[256],name[256];float relative[12],positions[12];};
struct CapsuleRow {unsigned int group,index;char owner[256],name[256];float values[45];};
static bool Write(HANDLE file,const void* data,unsigned int bytes){DWORD written=0;return WriteFile(file,data,bytes,&written,0)&&written==bytes;}
static bool Text(HANDLE file,const char* text){return Write(file,text,lstrlenA(text));}
static bool UInt(HANDLE file,unsigned int value){char text[16];unsigned int used=0;do{text[used++]=(char)('0'+value%10);value/=10;}while(value);for(unsigned int i=0;i<used/2;i++){char c=text[i];text[i]=text[used-i-1];text[used-i-1]=c;}return Write(file,text,used);}
static bool NodeSource(HANDLE file,const char* role,const PrefabScene::Node& node,const PrefabScene::Model& model){
 if(!Text(file,role)||!Text(file," ")||!Text(file,node.name)||!Text(file," class=")||!Text(file,node.className)||!Text(file," source=")||!Text(file,node.source)||!Text(file," sourceId=")||!UInt(file,node.sourceId)||!Text(file," sourceKind=")||!UInt(file,node.sourceKind)||!Text(file," prefabDepth=")||!UInt(file,NativePhysicsExplicit::PrefabDepth(model,node))||!Text(file,"\r\n"))return false;
 for(const PrefabScene::Property* prop=node.properties;prop;prop=prop->next)if(FearBrushFlags::Equal(prop->name,"Name")||FearBrushFlags::Equal(prop->name,"Pos")||FearBrushFlags::Equal(prop->name,"Rotation")){if(!Text(file,"  expandedProperty ")||!Text(file,prop->name)||!Text(file,"=")||!Text(file,prop->data)||!Text(file,"\r\n"))return false;}
 PrefabScene::Source source;if(!source.LoadExplicit(node.source,false,true))return Text(file,"  Could not decode original source\r\n");
 for(unsigned int i=0;i<source.scene.nodeCount;i++)if(source.scene.nodes[i].id==node.sourceId){const WorldText::SceneNode& original=source.scene.nodes[i];if(!Text(file,"  Original name=")||!Text(file,original.name)||!Text(file," parentOrdinal=")||!UInt(file,original.parent)||!Text(file,"\r\n"))return false;}
 return true;
}
extern "C" void HarnessEntry(){
 char source[MAX_PATH],project[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_READER_INPUT",source,MAX_PATH)||!GetEnvironmentVariableA("FEAR_READER_PROJECT",project,MAX_PATH)||!GetEnvironmentVariableA("FEAR_PHYSICS_CONTRACT_AUDIT",output,MAX_PATH))ExitProcess(1);
 PrefabScene::Arena memory;void* place=memory.Allocate(sizeof(NativePreparedRenderScene::Context));if(!place)ExitProcess(2);NativePreparedRenderScene::Context* scene=new(place)NativePreparedRenderScene::Context;if(!scene->Build(source,project))ExitProcess(2);
 const PrefabScene::Model& model=scene->Model();const WorldModels::Partition& p=scene->Partition();WorldModelSharing::Identity sharing;if(!sharing.Build(model,p))ExitProcess(3);
 HANDLE provenance=INVALID_HANDLE_VALUE;unsigned int selected=0;char selection[32],path[MAX_PATH];if(GetEnvironmentVariableA("FEAR_PHYSICS_AUDIT_SOURCE_GROUP",selection,32)&&GetEnvironmentVariableA("FEAR_PHYSICS_AUDIT_PROVENANCE",path,MAX_PATH)){for(const char* c=selection;*c;c++){if(*c<'0'||*c>'9')ExitProcess(3);selected=selected*10+*c-'0';}provenance=CreateFileA(path,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(provenance==INVALID_HANDLE_VALUE)ExitProcess(3);}
 unsigned int* ownerAt=(unsigned int*)memory.Allocate((model.count+1)*4),*groups=(unsigned int*)memory.Allocate((model.count+1)*4);OwnerRow* owners=(OwnerRow*)memory.Allocate((p.groupCount+1)*sizeof(OwnerRow));ShapeRow* shapes=(ShapeRow*)memory.Allocate((model.count+1)*sizeof(ShapeRow));CapsuleRow* capsules=(CapsuleRow*)memory.Allocate((model.count+1)*sizeof(CapsuleRow));if(!ownerAt||!groups||!owners||!shapes||!capsules)ExitProcess(3);
 for(unsigned int g=1;g<p.groupCount;g++)ownerAt[p.groups[g].ownerIndex]=g;
 for(const PrefabScene::Node* n=model.first;n;n=n->next)groups[n->index]=ownerAt[n->index]?ownerAt[n->index]:(n->parent==0xffffffffu?0:groups[n->parent]);
 unsigned int ownerCount=0,shapeCount=0,capsuleCount=0,modelIndex=1;const bool nav=SourcePhysicalBspContours::HasNavigation(model);
 for(unsigned int g=1;g<p.groupCount;g++){if(sharing.representative[g]!=g||!SourcePhysicalBspContours::RetainedGroup(model,p,g))continue;const PrefabScene::Node& owner=*p.groups[g].owner;PhysicsOwnerBounds::Bounds all,physics,disk;
  if(provenance!=INVALID_HANDLE_VALUE&&selected==g){if(!NodeSource(provenance,"OWNER",owner,model))ExitProcess(3);unsigned int parent=owner.parent;while(parent!=0xffffffffu){const PrefabScene::Node* found=0;for(const PrefabScene::Node* n=model.first;n;n=n->next)if(n->index==parent){found=n;break;}if(!found||!NodeSource(provenance,"PARENT",*found,model))ExitProcess(3);parent=found->parent;}for(const PrefabScene::Node* n=model.first;n;n=n->next)if(n->kind==2&&groups[n->index]==g&&FearBrushFlags::Equal(n->className,"PhysicsShapeOBB"))if(!NodeSource(provenance,"SHAPE",*n,model))ExitProcess(3);}
  for(unsigned int a=0;a<p.assignmentCount;a++){const WorldModels::Assignment& assignment=p.assignments[a];if(assignment.group!=g)continue;const WorldText::Geometry& geom=assignment.brush->geometry;
   for(unsigned int v=0;v<geom.points;v++){if(!PhysicsOwnerBounds::Referenced(geom,v))continue;WorldText::Vec3 local;if(!SourceOwnerRotation::Local(owner,geom.vertices[v],scene->Offset(),local)||!all.Add(local))ExitProcess(4);if((assignment.brush->surfaceFlags&0x10)&&!physics.Add(local))ExitProcess(4);if(SourcePhysicalBspContours::DiskCandidate(*assignment.brush,nav)&&!disk.Add(local))ExitProcess(4);}
  }
  OwnerRow& r=owners[ownerCount++];r.group=g;r.index=modelIndex;lstrcpynA(r.name,owner.name,256);if(all.count){r.all=all.Center();r.allLow=all.low;r.allHigh=all.high;}if(physics.count){r.physics=physics.Center();r.physicsLow=physics.low;r.physicsHigh=physics.high;}if(disk.count)r.disk=disk.Center();float body[6];if(all.count&&physics.count&&!PhysicsOwnerBounds::Finish(all,physics,1,1.1f,body,r.finished))ExitProcess(4);
  unsigned int referenceCount=0;for(unsigned int a=0;a<p.assignmentCount;a++){const WorldModels::Assignment& item=p.assignments[a];if(item.group==g&&SourcePhysicalBspContours::DiskCandidate(*item.brush,nav))referenceCount+=item.brush->geometry.references;}
  if(referenceCount){PrefabScene::Arena temporary;WorldText::Vec3* input=(WorldText::Vec3*)temporary.Allocate(referenceCount*12);unsigned char* physicsPoint=(unsigned char*)temporary.Allocate(referenceCount);if(!input||!physicsPoint)ExitProcess(4);unsigned int used=0;
   for(unsigned int a=0;a<p.assignmentCount;a++){const WorldModels::Assignment& item=p.assignments[a];if(item.group!=g||!SourcePhysicalBspContours::DiskCandidate(*item.brush,nav))continue;const WorldText::Geometry& geom=item.brush->geometry;
    for(unsigned int f=0;f<geom.polygons;f++)for(unsigned int v=0;v<geom.faces[f].count;v++){if(used>=referenceCount||!SourceOwnerRotation::Local(owner,geom.vertices[geom.faces[f].indices[v]],scene->Offset(),input[used]))ExitProcess(4);physicsPoint[used++]=(item.brush->surfaceFlags&0x10)?1:0;}
   }
   if(used!=referenceCount)ExitProcess(4);SourceRenderPointTags::Indexer tags;if(!tags.Build(input,used))ExitProcess(4);PhysicsOwnerBounds::Bounds welded;
   for(unsigned int i=0;i<used;i++)if(physicsPoint[i]&&!welded.Add(tags.points[tags.indices[i]]))ExitProcess(4);if(welded.count)r.weldedPhysics=welded.Center();
  }
  for(const PrefabScene::Node* n=model.first;n;n=n->next){if(n->kind!=2||groups[n->index]!=g||!(FearBrushFlags::Equal(n->className,"PhysicsShapeOBB")||FearBrushFlags::Equal(n->className,"PhysicsShapeSphere")))continue;ShapeRow& s=shapes[shapeCount++];s.group=g;s.index=modelIndex;lstrcpynA(s.owner,owner.name,256);lstrcpynA(s.name,n->name,256);
   float b[4],a[4];if(!SourceOwnerRotation::Quaternion(owner,b)||!SourceOwnerRotation::Quaternion(*n,a))ExitProcess(5);b[0]=-b[0];b[1]=-b[1];b[2]=-b[2];Product(b,a,s.relative);
   if(!SourceQuaternion(owner,b)||!SourceQuaternion(*n,a))ExitProcess(5);b[0]=-b[0];b[1]=-b[1];b[2]=-b[2];Product(b,a,s.relative+4);
   float t[7];if(!NativePhysicsExplicit::Transform(model,p,g,*n,scene->Offset(),t))ExitProcess(5);memcpy(s.relative+8,t+3,16);
   WorldText::Vec3 rawPosition={},shapePosition={},ownerPosition={},local;
   if(!PhysicsExplicitShapes::SourcePosition(owner,*n,rawPosition)||!PhysicsExplicitShapes::Vector(*n,"Pos",shapePosition)||!PhysicsExplicitShapes::Vector(owner,"Pos",ownerPosition))ExitProcess(5);
   memcpy(s.positions,&rawPosition,12);
   WorldText::Vec3 delta={shapePosition.x-ownerPosition.x,shapePosition.y-ownerPosition.y,shapePosition.z-ownerPosition.z};local=SourceWorldModelTransform::Rotate(b,delta);memcpy(s.positions+3,&local,12);
   WorldText::Vec3 sp=SourceWorldModelTransform::Rotate(b,shapePosition),op=SourceWorldModelTransform::Rotate(b,ownerPosition);local.x=sp.x-op.x;local.y=sp.y-op.y;local.z=sp.z-op.z;memcpy(s.positions+6,&local,12);
   if(!SourceOwnerRotation::Local(owner,n->position,scene->Offset(),local))ExitProcess(5);memcpy(s.positions+9,&local,12);
   for(unsigned int k=0;k<12;k++)s.positions[k]-=(&r.all.x)[k%3];
  }
  for(const PrefabScene::Node* n=model.first;n;n=n->next){if(n->kind!=2||groups[n->index]!=g||!FearBrushFlags::Equal(n->className,"PhysicsShapeCapsule"))continue;CapsuleRow& s=capsules[capsuleCount++];s.group=g;s.index=modelIndex;lstrcpynA(s.owner,owner.name,256);lstrcpynA(s.name,n->name,256);
   unsigned int kind;float transform[7],body[9],length=0,radius=0;if(!NativePhysicsExplicit::Build(model,p,g,*n,scene->Offset(),kind,body,transform)||!NativePhysicsProperties::Number(*n,"Length",100,length)||!NativePhysicsProperties::Number(*n,"Radius",100,radius))ExitProcess(7);
   memcpy(s.values,body,36);memcpy(s.values+9,body,36);memcpy(s.values+18,body,36);memcpy(s.values+27,transform,28);s.values[34]=length;s.values[35]=radius;
   memcpy(s.values+36,body,36);float x=transform[3],y=transform[4],z=transform[5],w=transform[6];WorldText::Vec3 matrixUp={(float)(2.*(double(x)*y-double(w)*z)),(float)(1.-2.*(double(x)*x+double(z)*z)),(float)(2.*(double(y)*z+double(w)*x))};
   for(unsigned int k=0;k<3;k++){s.values[39+k]=transform[k]+(&matrixUp.x)[k]*(length*.5f);s.values[42+k]=transform[k]-(&matrixUp.x)[k]*(length*.5f);}
   WorldText::Vec3 pos={0,length*.5f,0},neg={0,-length*.5f,0};pos=SourceWorldModelTransform::Rotate(transform+3,pos);neg=SourceWorldModelTransform::Rotate(transform+3,neg);
   for(unsigned int k=0;k<3;k++){s.values[12+k]=transform[k]+(&pos.x)[k];s.values[15+k]=transform[k]+(&neg.x)[k];}
   float inv[4],q[4];WorldText::Vec3 sourceShape={},sourceOwner={};
   if(!SourceQuaternion(owner,inv)||!SourceQuaternion(*n,q)||!PhysicsExplicitShapes::Vector(*n,"Pos",sourceShape)||!PhysicsExplicitShapes::Vector(owner,"Pos",sourceOwner))ExitProcess(7);inv[0]=-inv[0];inv[1]=-inv[1];inv[2]=-inv[2];
   WorldText::Vec3 ownerRot=SourceWorldModelTransform::Rotate(inv,sourceOwner);
   for(unsigned int e=0;e<2;e++){WorldText::Vec3 endpoint={0,e?-length*.5f:length*.5f,0};endpoint=SourceWorldModelTransform::Rotate(q,endpoint);endpoint.x+=sourceShape.x;endpoint.y+=sourceShape.y;endpoint.z+=sourceShape.z;endpoint=SourceWorldModelTransform::Rotate(inv,endpoint);
    for(unsigned int k=0;k<3;k++)s.values[21+e*3+k]=((&endpoint.x)[k]-(&ownerRot.x)[k])-(&r.all.x)[k];
   }
  }
  modelIndex++;
 }
 HANDLE file=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);unsigned int header[6]={0x32445541u,ownerCount,shapeCount,sizeof(ShapeRow),capsuleCount,sizeof(CapsuleRow)};if(file==INVALID_HANDLE_VALUE||!Write(file,header,24)||!Write(file,owners,ownerCount*sizeof(OwnerRow))||!Write(file,shapes,shapeCount*sizeof(ShapeRow))||!Write(file,capsules,capsuleCount*sizeof(CapsuleRow)))ExitProcess(6);CloseHandle(file);if(provenance!=INVALID_HANDLE_VALUE)CloseHandle(provenance);ExitProcess(0);
}
