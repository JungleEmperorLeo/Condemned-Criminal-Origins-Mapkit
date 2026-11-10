#pragma once
#include "NativePreparedRenderScene.h"
#include "SourcePhysicalBspContours.h"
#include "NativeBlindDataRegistry.h"
#include "MsvcSort.h"
// Finite source geometry producer. Runtime debris simulation remains in the SDK;
// these bytes describe the prepared world model before any impact occurs.
namespace NativeShatter {
static const unsigned int TypeId=0x00000a85u,Missing=0xffffffffu;
struct Settings {unsigned int version,maximumBytes,maximumPolygons,maximumCorners;Settings():version(1),maximumBytes(16u*1024u*1024u),maximumPolygons(100000),maximumCorners(1000000){}};
struct Polygon {const WorldText::Vec3* points;unsigned int count;WorldText::Vec3 normal;RenderTextureCoordinates::Basis basis;};
class Payload {
 PrefabScene::Arena memory;Payload(const Payload&);Payload& operator=(const Payload&);
 bool Fail(const char* message){Clear();error=message;return false;}
 static bool Unit(const WorldText::Vec3& v,WorldText::Vec3& result){
  if(!WorldText::FiniteVector(v))return false;float squared=(float)((double(v.x)*v.x+double(v.y)*v.y)+double(v.z)*v.z);if(!WorldText::Finite(squared)||squared<=0)return false;
  float length=ObjectsBlock::Root(squared);if(!WorldText::Finite(length)||length<=0)return false;float inverse=(float)(1.0/length);result.x=(float)(double(inverse)*v.x);result.y=(float)(double(inverse)*v.y);result.z=(float)(double(inverse)*v.z);return WorldText::FiniteVector(result);
 }
public:
 unsigned char* data;unsigned int size,polygons,corners;const char* error;
 Payload():data(0),size(0),polygons(0),corners(0),error(0){}
 void Clear(){memory.Clear();data=0;size=polygons=corners=0;error=0;}
 bool Build(const Polygon* input,unsigned int count,const WorldText::Vec3& center,const Settings& settings=Settings()){
  // Caller inputs must be separate from this output. Reject before reset.
  if(settings.version!=1||settings.maximumBytes<4||settings.maximumBytes>16u*1024u*1024u||!settings.maximumPolygons||settings.maximumPolygons>100000||!settings.maximumCorners||settings.maximumCorners>1000000)return Fail("Invalid Shatter payload limits");
  if(count>settings.maximumPolygons||(count&&!input)||!WorldText::FiniteVector(center))return Fail("Invalid Shatter polygon input");
  if(NativeBlindDataRegistry::detail::Overlap(input,count*sizeof(Polygon),data,size)||NativeBlindDataRegistry::detail::Overlap(&center,sizeof(center),data,size))return Fail("Shatter polygon or center input aliases prior output");
  unsigned long long wanted=4;unsigned int points=0;
  for(unsigned int i=0;i<count;i++){const Polygon& p=input[i];if(!p.points||p.count<3||p.count>65535||p.count>settings.maximumCorners-points||!WorldText::FiniteVector(p.normal)||!WorldText::FiniteVector(p.basis.origin)||NativeBlindDataRegistry::detail::Overlap(p.points,p.count*12u,data,size))return Fail("Invalid Shatter polygon geometry or lifetime");WorldText::Vec3 unit;if(!Unit(p.basis.u,unit)||!Unit(p.basis.v,unit))return Fail("Shatter tangent basis must have finite nonzero axes");for(unsigned int j=0;j<p.count;j++)if(!WorldText::FiniteVector(p.points[j]))return Fail("Nonfinite Shatter input point");points+=p.count;wanted+=40ull+20ull*p.count;if(wanted>settings.maximumBytes)return Fail("Shatter payload byte limit");}
  Clear();data=(unsigned char*)memory.Allocate((unsigned int)wanted);if(!data)return Fail("Shatter payload allocation failed");memcpy(data,&count,4);unsigned int at=4;
  const float inverse[4]={-0.0f,-0.0f,-0.0f,1.f};WorldText::Vec3 translation=SourceSurfaceRotation::Rotate(inverse,center);translation.x=-translation.x;translation.y=-translation.y;translation.z=-translation.z;
  for(unsigned int i=0;i<count;i++){const Polygon& p=input[i];WorldText::Vec3 tangent,binormal;Unit(p.basis.u,tangent);Unit(p.basis.v,binormal);WorldText::Vec3 vectors[3]={SourceSurfaceRotation::Rotate(inverse,p.normal),SourceSurfaceRotation::Rotate(inverse,tangent),SourceSurfaceRotation::Rotate(inverse,binormal)};memcpy(data+at,vectors,36);memcpy(data+at+36,&p.count,4);at+=40;
   for(unsigned int j=0;j<p.count;j++){const WorldText::Vec3& v=p.points[j];WorldText::Vec3 position=SourceSurfaceRotation::Rotate(inverse,v);position.x+=translation.x;position.y+=translation.y;position.z+=translation.z;const WorldText::Vec3& o=p.basis.origin;const WorldText::Vec3& u=p.basis.u;const WorldText::Vec3& q=p.basis.v;
    float uv[2]={(float)(((double(u.z)*v.z+double(v.x)*u.x)+double(u.y)*v.y)-((double(o.x)*u.x+double(o.y)*u.y)+double(u.z)*o.z)),(float)(((double(v.y)*q.y+double(v.x)*q.x)+double(v.z)*q.z)-((double(o.z)*q.z+double(o.y)*q.y)+double(o.x)*q.x))};
    if(!WorldText::FiniteVector(position)||!WorldText::Finite(uv[0])||!WorldText::Finite(uv[1]))return Fail("Nonfinite Shatter output transform or UV");memcpy(data+at,&position,12);memcpy(data+at+12,uv,8);at+=20;
   }
  }
  if(at!=wanted)return Fail("Shatter payload cursor mismatch");size=at;polygons=count;corners=points;return true;
 }
};
struct Record {unsigned int sourceNode,group,modelIndex,polygons,corners;};
struct Binding {unsigned int sourceNode,ordinal,modelIndex;const char* name;};
class Builder {
 PrefabScene::Arena memory;Builder(const Builder&);Builder& operator=(const Builder&);
 bool Fail(const char* message,unsigned int source=Missing){char saved[512];lstrcpynA(saved,message?message:"Shatter source compilation failed",512);Clear();lstrcpyA(diagnostic,saved);error=diagnostic;errorNode=source;return false;}
 static const PrefabScene::Property* Property(const PrefabScene::Node& n,const char* name){for(const PrefabScene::Property* p=n.properties;p;p=p->next)if(GameClassMetadata::Exact(p->name,name))return p;return 0;}
 struct OrderedName {const char* text;unsigned int group;};
 struct OrderedLess {bool operator()(const OrderedName& a,const OrderedName& b)const{return NameCompare(a.text,b.text)<0;}};
 static int NameCompare(const char* a,const char* b){for(;;a++,b++){unsigned char x=(unsigned char)*a,y=(unsigned char)*b;if(x>='A'&&x<='Z')x+=32;if(y>='A'&&y<='Z')y+=32;if(x!=y)return x<y?-1:1;if(!x)return 0;}}
 static bool Surface(const PrefabScene::Node& brush,const PrefabScene::Node& owner,unsigned int polygon,const WorldText::Vec3& offset,bool local,RenderTextureCoordinates::Basis* bases){
  if(!local){SourceSurfacePreparation::Surface surface;if(!SourceSurfacePreparation::Build(brush,polygon,offset,surface))return false;memcpy(bases,surface.bases,sizeof(surface.bases));return true;}
  const WorldText::Polygon& face=brush.geometry.faces[polygon];if(!face.mappings||face.mappingCount>32)return false;unsigned int seen=0;float inverse[4];if(!SourceOwnerRotation::Inverse(owner,inverse))return false;
  for(unsigned int i=0;i<face.mappingCount;i++){const WorldText::Mapping& m=face.mappings[i];if(m.channel>=4)continue;unsigned int bit=1u<<m.channel;if((seen&bit)||!m.valid||!WorldText::FiniteVector(m.origin)||!WorldText::FiniteVector(m.p)||!WorldText::FiniteVector(m.q))return false;seen|=bit;RenderTextureCoordinates::Basis& b=bases[m.channel];if(!SourceOwnerRotation::LocalTextureOrigin(owner,m.origin,offset,b.origin,false))return false;b.u=SourceWorldModelTransform::Rotate(inverse,m.p);b.v=SourceWorldModelTransform::Rotate(inverse,m.q);}
  if(seen!=15)return false;SourceSurfacePreparation::Fragment fragment;if(!SourceSurfacePreparation::FragmentAt(face,0,fragment))return false;WorldText::Vec3 points[30];for(unsigned int i=0;i<fragment.count;i++){if(fragment.indices[i]>=brush.geometry.points||!SourceOwnerRotation::Local(owner,brush.geometry.vertices[fragment.indices[i]],offset,points[i]))return false;}return TextureOriginMinimize::Build(points,fragment.count,bases);
 }
 bool Geometry(const NativePreparedRenderScene::Context& scene,unsigned int group,const SourceBlockerPlanePool::Prepared& planes,const Settings& settings,Payload& payload){
  const PrefabScene::Model& model=scene.Model();const WorldModels::Partition& partition=scene.Partition();const PrefabScene::Node& owner=*partition.groups[group].owner;const bool navigation=SourcePhysicalBspContours::HasNavigation(model);unsigned int polygonCount=0,references=0;bool local=owner.importedFromPrefab;
  for(unsigned int a=0;a<partition.assignmentCount;a++){const WorldModels::Assignment& item=partition.assignments[a];if(item.group!=group||!SourcePhysicalBspContours::DiskCandidate(*item.brush,navigation))continue;if(!(item.brush->surfaceFlags&0x180))local=true;for(unsigned int p=0;p<item.brush->geometry.polygons;p++){const WorldText::Polygon& face=item.brush->geometry.faces[p];unsigned int fragments=SourceSurfacePreparation::FragmentCount(face.count);if(!fragments||fragments>settings.maximumPolygons-polygonCount)return Fail("Shatter source polygon limit",owner.index);polygonCount+=fragments;}}
  PrefabScene::Arena scratch;Polygon* polys=(Polygon*)scratch.Allocate((polygonCount?polygonCount:1)*sizeof(Polygon));if(!polys)return Fail("Shatter source polygon allocation failed",owner.index);WorldText::Vec3 low={},high={};bool hasBounds=false;unsigned int at=0;
  for(unsigned int a=0;a<partition.assignmentCount;a++){const WorldModels::Assignment& item=partition.assignments[a];if(item.group!=group||!SourcePhysicalBspContours::DiskCandidate(*item.brush,navigation))continue;const PrefabScene::Node& brush=*item.brush;
   for(unsigned int p=0;p<brush.geometry.polygons;p++){RenderTextureCoordinates::Basis bases[4];if(!Surface(brush,owner,p,scene.Offset(),local,bases))return Fail("Shatter source texture basis preparation failed",brush.index);WorldText::Vec3 normal;if(!planes.Normal(a,p,normal))return Fail("Shatter source normal preparation failed",brush.index);
    for(unsigned int f=0;f<SourceSurfacePreparation::FragmentCount(brush.geometry.faces[p].count);f++){SourceSurfacePreparation::Fragment fragment;if(!SourceSurfacePreparation::FragmentAt(brush.geometry.faces[p],f,fragment)||fragment.count>settings.maximumCorners-references)return Fail("Shatter source corner limit",brush.index);WorldText::Vec3* points=(WorldText::Vec3*)scratch.Allocate(fragment.count*12);if(!points)return Fail("Shatter source point allocation failed",brush.index);for(unsigned int i=0;i<fragment.count;i++){if(fragment.indices[i]>=brush.geometry.points||!planes.Point(a,brush.geometry.vertices[fragment.indices[i]],points[i]))return Fail("Shatter source point preparation failed",brush.index);const WorldText::Vec3& v=points[i];if(!hasBounds){low=high=v;hasBounds=true;}else{if(v.x<low.x)low.x=v.x;if(v.y<low.y)low.y=v.y;if(v.z<low.z)low.z=v.z;if(v.x>high.x)high.x=v.x;if(v.y>high.y)high.y=v.y;if(v.z>high.z)high.z=v.z;}}
     Polygon& out=polys[at++];out.points=points;out.count=fragment.count;out.normal=normal;out.basis=bases[0];references+=fragment.count;
    }
   }
  }
  if(at!=polygonCount)return Fail("Shatter source enumeration mismatch",owner.index);WorldText::Vec3 center={};if(hasBounds){volatile float x=high.x+low.x,y=high.y+low.y,z=high.z+low.z;center.x=x*.5f;center.y=y*.5f;center.z=z*.5f;}
  if(!payload.Build(polys,polygonCount,center,settings))return Fail(payload.error,owner.index);return true;
 }
public:
 NativeBlindDataRegistry::RecordInput* records;NativeBlindDataRegistry::BindingInput* registryBindings;Record* details;Binding* bindings;unsigned int count,bindingCount,totalPolygons,totalCorners,totalBytes,errorNode;const char* error;char diagnostic[512];
 const char** missingIndex;unsigned int missingIndexCount; // WorldModels warned at 0x1001244C, dispatch order
 Builder():records(0),registryBindings(0),details(0),bindings(0),count(0),bindingCount(0),totalPolygons(0),totalCorners(0),totalBytes(0),errorNode(Missing),error(0),missingIndex(0),missingIndexCount(0){diagnostic[0]=0;}
 void Clear(){memory.Clear();records=0;registryBindings=0;details=0;bindings=0;count=bindingCount=totalPolygons=totalCorners=totalBytes=0;errorNode=Missing;error=0;diagnostic[0]=0;missingIndex=0;missingIndexCount=0;}
 // demandOnly: only the dispatch walk and missingIndex (the packer log), no records.
 bool Build(const NativePreparedRenderScene::Context& scene,const Settings& settings=Settings(),bool demandOnly=false){
  Clear();if(!scene.Ready()||settings.version!=1||settings.maximumBytes<4||settings.maximumBytes>16u*1024u*1024u||!settings.maximumPolygons||settings.maximumPolygons>100000||!settings.maximumCorners||settings.maximumCorners>1000000)return Fail("Invalid Shatter source scene/settings");const PrefabScene::Model& model=scene.Model();const WorldModels::Partition& partition=scene.Partition();
  WorldModelSharing::Identity sharing;if(!sharing.Build(model,partition))return Fail("Shatter WorldModel sharing failed");unsigned int groups=partition.groupCount;
  records=(NativeBlindDataRegistry::RecordInput*)memory.Allocate(groups*sizeof(*records));registryBindings=(NativeBlindDataRegistry::BindingInput*)memory.Allocate(groups*sizeof(*registryBindings));details=(Record*)memory.Allocate(groups*sizeof(*details));bindings=(Binding*)memory.Allocate(groups*sizeof(*bindings));unsigned int* models=(unsigned int*)memory.Allocate(groups*4);unsigned int* ordinals=(unsigned int*)memory.Allocate(groups*4);if(!records||!registryBindings||!details||!bindings||!models||!ordinals)return Fail("Shatter model registry allocation failed");unsigned int used=0;
  for(unsigned int g=0;g<groups;g++){models[g]=ordinals[g]=Missing;if(g&&!SourcePhysicalBspContours::RetainedGroup(model,partition,g))continue;unsigned int representative=sharing.representative[g];if(representative==g)models[g]=used++;else{if(representative>=g||models[representative]==Missing)return Fail("Shatter alias has no emitted model",partition.groups[g].ownerIndex);models[g]=models[representative];}}
  // FEAR visits the alphabetic WorldModel name table, whose entries may alias
  // the physical models. Geometry emission order and this dispatch order differ.
  OrderedName* table=(OrderedName*)memory.Allocate(groups*sizeof(OrderedName));unsigned int* order=(unsigned int*)memory.Allocate(groups*4);if(!table||!order)return Fail("Shatter name order allocation failed");unsigned int named=0,ordered=0;for(unsigned int g=0;g<groups;g++)if(models[g]!=Missing){table[named].text=g?partition.groups[g].owner->name:"PhysicsBSP";table[named++].group=g;}MsvcSort::Sort(table,named,OrderedLess());for(unsigned int i=0;i<named;i++)if(table[i].group)order[ordered++]=table[i].group;
  // The same MSVC std::sort of the group-order name list as the WorldModels table.
  SourceBlockerPlanePool::Prepared planes;bool planesReady=false;
  missingIndex=(const char**)memory.Allocate((ordered?ordered:1)*sizeof(char*));if(!missingIndex)return Fail("Shatter warning list allocation failed");
  for(unsigned int row=0;row<ordered;row++){unsigned int g=order[row];const PrefabScene::Node& owner=*partition.groups[g].owner;
   // 0x100122E0: ShatterType is read as a string property (another type reads ""),
   // "None" compares case-insensitively; without a real BlindObjectIndex the model
   // only warns (0x1001244C) and gets no record.
   const PrefabScene::Property* type=Property(owner,"ShatterType");if(!type||!GameClassMetadata::Exact(type->type,"string")||!type->data||!type->data[0]||FearBrushFlags::Equal(type->data,"None"))continue;const PrefabScene::Property* index=Property(owner,"BlindObjectIndex");if(!index||!GameClassMetadata::Exact(index->type,"real")){missingIndex[missingIndexCount++]=owner.name;continue;}
   if(demandOnly)continue;unsigned int modelIndex=models[g],ordinal=ordinals[modelIndex];
   if(ordinal==Missing){if(!planesReady){if(!planes.Build(model,partition,scene.Offset()))return Fail(planes.error);planesReady=true;}unsigned int representative=sharing.representative[g];Payload payload;if(!Geometry(scene,representative,planes,settings,payload))return false;if(payload.size>settings.maximumBytes-totalBytes||payload.polygons>settings.maximumPolygons-totalPolygons||payload.corners>settings.maximumCorners-totalCorners)return Fail("Shatter aggregate payload limit",owner.index);unsigned char* copy=(unsigned char*)memory.Allocate(payload.size);if(!copy)return Fail("Shatter owned payload allocation failed",owner.index);memcpy(copy,payload.data,payload.size);ordinal=count;ordinals[modelIndex]=ordinal;records[count].typeId=TypeId;records[count].data=copy;records[count].bytes=payload.size;Record& detail=details[count++];detail.sourceNode=partition.groups[representative].ownerIndex;detail.group=representative;detail.modelIndex=modelIndex;detail.polygons=payload.polygons;detail.corners=payload.corners;totalBytes+=payload.size;totalPolygons+=payload.polygons;totalCorners+=payload.corners;}
   unsigned int length=lstrlenA(owner.name);if(!length||length>255)return Fail("Invalid Shatter binding owner name",owner.index);char* name=(char*)memory.Allocate(length+1);if(!name)return Fail("Shatter binding name allocation failed",owner.index);memcpy(name,owner.name,length+1);Binding& b=bindings[bindingCount];b.sourceNode=owner.index;b.ordinal=ordinal;b.modelIndex=modelIndex;b.name=name;NativeBlindDataRegistry::BindingInput& bind=registryBindings[bindingCount++];bind.sourceNode=owner.index;bind.property="BlindObjectIndex";bind.ordinal=ordinal;bind.typeId=TypeId;
  }return true;
 }
};
}
