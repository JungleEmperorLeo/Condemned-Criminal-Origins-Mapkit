#pragma once
#include "NativePreparedRenderScene.h"
#include "NativePhysicsProperties.h"
#include "SourceOwnerRotation.h"
// MAIN's explicit shapes live alongside its static collision mesh. Raw Pos and
// Rotation properties survive prefab expansion. Replay their source frames with
// centering before the first prefab transform rather than subtracting an offset
// from the already rounded expanded position. No source/references are reopened.
namespace NativePhysicsMainExplicit {
struct Settings {unsigned int version;Settings():version(1){}};
struct Record {
 Record* next;unsigned int sourceNode,type,bytes;unsigned char* data;
 float transform[7],body[9];char name[256];
};
class Builder {
 PrefabScene::Arena memory;
 Builder(const Builder&);Builder& operator=(const Builder&);
 bool Fail(const char* message,unsigned int node=0xffffffffu){char saved[512];lstrcpynA(saved,message?message:"MAIN explicit physics compilation failed",512);Clear();lstrcpyA(diagnostic,saved);error=diagnostic;failedNode=node;return false;}
 static bool Explicit(const PrefabScene::Node& node){return node.kind==2&&(FearBrushFlags::Equal(node.className,"PhysicsShapeOBB")||FearBrushFlags::Equal(node.className,"PhysicsShapeSphere")||FearBrushFlags::Equal(node.className,"PhysicsShapeCapsule"));}
 static bool Finite(const PrefabScene::Matrix& matrix){for(unsigned int r=0;r<3;r++)for(unsigned int c=0;c<4;c++)if(!WorldText::Finite(matrix.m[r][c]))return false;return true;}
 static bool RawVector(const PrefabScene::Node& node,const char* name,const char* type,WorldText::Vec3& value){
  value.x=value.y=value.z=0;const PrefabScene::Property* property=PhysicsExplicitShapes::Property(node,name);
  // Source.Scene uses zero for absent/mistyped Pos/Rotation; correctly typed
  // malformed values still fail, matching the source loader's contract.
  return !property||!FearBrushFlags::Equal(property->type,type)||PhysicsExplicitShapes::Vector(node,name,value);
 }
 static bool Write(unsigned char* target,unsigned int capacity,unsigned int& cursor,const void* source,unsigned int bytes){if(bytes>capacity-cursor||(!source&&bytes))return false;memcpy(target+cursor,source,bytes);cursor+=bytes;return true;}
 static bool U(unsigned char* target,unsigned int capacity,unsigned int& cursor,unsigned int value){return Write(target,capacity,cursor,&value,4);}
 static bool Primitive(const PrefabScene::Node& source,const PrefabScene::Matrix& frame,Record& record){
  float mass=0,density=0;if(!NativePhysicsProperties::MassDensity(source,false,mass,density))return false;
  WorldText::Vec3 point,rotation;if(!RawVector(source,"Pos","vector",point)||!RawVector(source,"Rotation","rotation",rotation))return false;
  PrefabScene::Matrix local;if(!PrefabScene::Matrix::Euler(rotation,local))return false;
  PrefabScene::Node pose=source;pose.position=frame.Apply(point);
  // An authored object retains its Euler matrix, including signed zeros.
  // MAIN serializes the source frame directly, without a non-MAIN owner's
  // inverse/Euler recovery. That extra recovery changes its zero signs.
  pose.orientation=source.importedFromPrefab?frame.Times(local):local;
  // An authored object's 4x4 transform (WorldNode, RVA 0x2D420) adds its own
  // translation*+0 to each 3x3 term: -0 becomes +0 for a nonnegative component.
  // SpillKill yaw-pi boxes (positive z) need +0; imported SP shapes keep theirs.
  if(!source.importedFromPrefab){volatile float bottom=0;const float* t=&pose.position.x;for(unsigned int r=0;r<3;r++)for(unsigned int c=0;c<3;c++)pose.orientation.m[r][c]=(float)(double(pose.orientation.m[r][c])+double(t[r])*bottom);}
  if(!WorldText::FiniteVector(pose.position)||!Finite(pose.orientation)||!SourceOwnerRotation::MatrixQuaternion(pose.orientation,record.transform+3))return false;
  record.transform[0]=pose.position.x;record.transform[1]=pose.position.y;record.transform[2]=pose.position.z;
  for(unsigned int i=0;i<9;i++)record.body[i]=0;
  if(FearBrushFlags::Equal(source.className,"PhysicsShapeOBB")){
   record.type=2;WorldText::Vec3 dims,fallback={100,100,100};return NativePhysicsProperties::Vector(source,"Dims",fallback,dims)&&PhysicsPrimitives::OBB(mass,density,dims,record.body);
  }
  if(FearBrushFlags::Equal(source.className,"PhysicsShapeSphere")){
   record.type=3;float radius=0;return NativePhysicsProperties::Number(source,"Radius",100,radius)&&PhysicsPrimitives::Sphere(mass,density,radius,record.body);
  }
  record.type=7;float radius=0,length=0;
  if(!NativePhysicsProperties::Number(source,"Radius",100,radius)||!NativePhysicsProperties::Number(source,"Length",100,length)||mass<0||density<0||radius<=0||length<0)return false;
  const float* q=record.transform+3;WorldText::Vec3 axis={2*(q[0]*q[1]-q[2]*q[3]),1-2*(q[0]*q[0]+q[2]*q[2]),2*(q[1]*q[2]+q[0]*q[3])};float half=length*.5f;
  record.body[0]=mass;record.body[1]=density;record.body[2]=radius;
  for(unsigned int i=0;i<3;i++){record.body[3+i]=record.transform[i]+(&axis.x)[i]*half;record.body[6+i]=record.transform[i]-(&axis.x)[i]*half;}
  for(unsigned int i=0;i<9;i++)if(!WorldText::Finite(record.body[i]))return false;
  // Capsule endpoints already contain MAIN coordinates; its child record is
  // bare type7 and does not carry a second transformed-child marker.
  record.transform[0]=record.transform[1]=record.transform[2]=record.transform[3]=record.transform[4]=record.transform[5]=0;record.transform[6]=1;
  return true;
 }
public:
 Record* first;Record* last;unsigned char* data;unsigned int size,count,obbCount,sphereCount,capsuleCount,nonMainExplicitCount,failedNode;const char* error;char diagnostic[512];
 Builder():first(0),last(0),data(0),size(0),count(0),obbCount(0),sphereCount(0),capsuleCount(0),nonMainExplicitCount(0),failedNode(0xffffffffu),error(0){diagnostic[0]=0;}
 void Clear(){memory.Clear();first=last=0;data=0;size=count=obbCount=sphereCount=capsuleCount=nonMainExplicitCount=0;failedNode=0xffffffffu;error=0;diagnostic[0]=0;}
 bool Build(const NativePreparedRenderScene::Context& scene,const Settings& settings=Settings()){
  if(!scene.Ready())return Fail("MAIN explicit physics requires a ready source scene");
  return BuildPrepared(scene.Model(),scene.Partition(),scene.Offset(),settings);
 }
 // Lower-level owned-scene adapter supports source preparation/ownership
 // guards without a game module. It never consumes a serialized World00p.
 bool BuildPrepared(const PrefabScene::Model& model,const WorldModels::Partition& partition,const WorldText::Vec3& offset,const Settings& settings=Settings()){
  Clear();if(settings.version!=1||!WorldText::FiniteVector(offset)||model.count>100000||!partition.groupCount||partition.groupCount>model.count+1||!partition.groups)return Fail("Invalid MAIN explicit scene/settings");
  const PrefabScene::Node** nodes=(const PrefabScene::Node**)memory.Allocate((model.count+1)*sizeof(PrefabScene::Node*));
  unsigned int* owners=(unsigned int*)memory.Allocate((model.count+1)*4);unsigned int* groups=(unsigned int*)memory.Allocate((model.count+1)*4);
  PrefabScene::Matrix* frames=(PrefabScene::Matrix*)memory.Allocate((model.count+1)*sizeof(PrefabScene::Matrix));
  PrefabScene::Matrix* instantiated=(PrefabScene::Matrix*)memory.Allocate((model.count+1)*sizeof(PrefabScene::Matrix));
  if(!nodes||!owners||!groups||!frames||!instantiated)return Fail("MAIN explicit frame allocation failed");
  for(unsigned int i=0;i<=model.count;i++){nodes[i]=0;owners[i]=groups[i]=0;new(frames+i)PrefabScene::Matrix;new(instantiated+i)PrefabScene::Matrix;}
  for(unsigned int g=1;g<partition.groupCount;g++){
   const WorldModels::Group& owner=partition.groups[g];if(!owner.owner||owner.ownerIndex>=model.count||owner.owner->index!=owner.ownerIndex||owners[owner.ownerIndex])return Fail("Invalid MAIN explicit owner lookup");owners[owner.ownerIndex]=g;
  }
  PrefabScene::Matrix centered;centered.m[0][3]=-offset.x;centered.m[1][3]=-offset.y;centered.m[2][3]=-offset.z;
  unsigned int seen=0;
  for(const PrefabScene::Node* node=model.first;node;node=node->next){
   if(node->index!=seen||seen>=model.count||(node->parent!=0xffffffffu&&node->parent>=seen)||node->kind>2||node->sourceKind>3)return Fail("Invalid MAIN explicit ordered hierarchy",node->index);
   nodes[seen]=node;const PrefabScene::Node* parent=node->parent==0xffffffffu?0:nodes[node->parent];
   groups[seen]=owners[seen]?owners[seen]:(parent?groups[parent->index]:0);
   // A prefab node also retains authored children from its containing file.
   // Such children retain the containing frame. Only its loaded contents
   // (different source path) use the node's instantiated frame.
   frames[seen]=parent?(parent->sourceKind==3&&!FearBrushFlags::Equal(parent->source,node->source)?instantiated[parent->index]:frames[parent->index]):centered;
   instantiated[seen]=frames[seen];
   if(node->sourceKind==3){
    if(!node->source[0])return Fail("Missing prefab source provenance",seen);
    WorldText::Vec3 position,rotation;if(!RawVector(*node,"Pos","vector",position)||!RawVector(*node,"Rotation","rotation",rotation))return Fail("Invalid MAIN prefab source transform",seen);
    PrefabScene::Matrix local;if(!PrefabScene::Matrix::Euler(rotation,local))return Fail("Unsupported MAIN prefab rotation",seen);local.m[0][3]=position.x;local.m[1][3]=position.y;local.m[2][3]=position.z;
    instantiated[seen]=frames[seen].Times(local);if(!Finite(instantiated[seen]))return Fail("Nonfinite MAIN prefab frame",seen);
   }
   if(Explicit(*node)){
    if(groups[seen])nonMainExplicitCount++;
    else{
     if(count>=100000)return Fail("MAIN explicit shape count limit",seen);
     if(node->importedFromPrefab&&!node->source[0])return Fail("Missing explicit source provenance",seen);
     Record* record=(Record*)memory.Allocate(sizeof(Record));if(!record)return Fail("MAIN explicit record allocation failed",seen);record->next=0;record->sourceNode=seen;lstrcpynA(record->name,node->name,256);
     if(!Primitive(*node,frames[seen],*record))return Fail("Invalid MAIN explicit shape properties/pose",seen);
     const unsigned int bodyBytes=record->type==2?24:record->type==3?12:36;record->bytes=record->type==7?4+bodyBytes:36+bodyBytes;
     record->data=(unsigned char*)memory.Allocate(record->bytes);if(!record->data)return Fail("MAIN explicit payload allocation failed",seen);
     unsigned int cursor=0;if(record->type!=7&&(!U(record->data,record->bytes,cursor,5)||!Write(record->data,record->bytes,cursor,record->transform,28)))return Fail("MAIN explicit wrapper write failed",seen);
     if(!U(record->data,record->bytes,cursor,record->type)||!Write(record->data,record->bytes,cursor,record->body,bodyBytes)||cursor!=record->bytes)return Fail("MAIN explicit body write failed",seen);
     if(record->bytes>16u*1024u*1024u-size)return Fail("MAIN explicit byte limit",seen);size+=record->bytes;count++;
     if(record->type==2)obbCount++;else if(record->type==3)sphereCount++;else capsuleCount++;
     if(last)last->next=record;else first=record;last=record;
    }
   }
   seen++;
  }
  if(seen!=model.count)return Fail("MAIN explicit node count mismatch");
  data=(unsigned char*)memory.Allocate(size?size:1);if(!data)return Fail("MAIN explicit final allocation failed");unsigned int cursor=0;
  for(const Record* record=first;record;record=record->next){memcpy(data+cursor,record->data,record->bytes);cursor+=record->bytes;}
  return cursor==size||Fail("MAIN explicit output size mismatch");
 }
};
}
