#pragma once
// CONDEMNED FORK: resource gathering through the game's own GameServer.dll, as the original packer does.
// The packer implements IObjectResourceGatherer (FEAR SDK engine/sdk/inc/iobjectresourcegatherer.h). Condemned's
// vtable has one extra slot right after the destructor (verified: GetObjectsOfClass at [+0x80], AddResourceToRegion
// at [+0x44], GetPropString at [+0x74], DoesObjectExist called by class gather functions). Slot 1 is never called by
// GatherResourcesPre/PostObjects or by the 30 class functions probed (research/gather_probe.cpp).
// Order: GatherResourcesPreObjects, each object's class gather function (model order), GatherResourcesPostObjects.
// Resources keep their target: a named region (AddResourceToRegion), a box/OBB (AddResourceToAABB/OBB) or global. Resolve() maps them
// to streaming regions (named: by name; box: every region whose convex volume intersects the box, else global).
#include "PrefabScene.h"
#include "UnifiedScene.h"
#include "PackerLog.h"
#include "StreamingConvex.h"
#include "ResourceAreaIntersection.h"
namespace CondemnedGatherer {
struct V3 {float x,y,z;};
struct Rect3 {V3 lo,hi;};
struct Rigid {V3 pos;float rot[4];};
struct ClassDef {const char* name;const char* desc;ClassDef* parent;unsigned int flags;void* construct;void* destruct;void* plugin;void* gather;unsigned int nProps;void* props;long objectSize;void* internal[2];};
static void Trace(const char* a,const char* b=0){char path[MAX_PATH];if(!GetEnvironmentVariableA("CONDEMNED_GATHER_TRACE",path,MAX_PATH))return;HANDLE h=CreateFileA(path,0x4,FILE_SHARE_READ,0,4,FILE_ATTRIBUTE_NORMAL,0);if(h==INVALID_HANDLE_VALUE)return;DWORD w;WriteFile(h,a,lstrlenA(a),&w,0);if(b){WriteFile(h," ",1,&w,0);WriteFile(h,b,lstrlenA(b),&w,0);}WriteFile(h,"\r\n",2,&w,0);CloseHandle(h);}
class Gatherer;
typedef ClassDef** (*SetupFn)(int*,void*,int*);
typedef void (*ClassGatherFn)(const char*,Gatherer*);
typedef void (*PhaseFn)(Gatherer*);
class Gatherer {
 const PrefabScene::Model& model;PrefabScene::Arena& memory;const char* const* regionNames;unsigned int regionCount;char worldName[MAX_PATH];
public:
 struct Entry {const char* path;const char* region;int kind;Rect3 box;}; // kind 0 global, 1 named region, 2 box
private:
 Entry* entries;unsigned int pathCount,pathCapacity;bool failed;
 static bool SameText(const char* a,const char* b){for(;;a++,b++){char x=*a,y=*b;if(x>='A'&&x<='Z')x+=32;if(y>='A'&&y<='Z')y+=32;if(x!=y)return false;if(!x)return true;}}
 const PrefabScene::Node* Find(const char* name) const {if(!name)return 0;for(const PrefabScene::Node* n=model.first;n;n=n->next)if(n->className[0]&&FearBrushFlags::Equal(n->name,name))return n;return 0;}
 const PrefabScene::Property* Prop(const char* object,const char* prop,const char* type=0) const {
  const PrefabScene::Node* n=Find(object);if(!n||!prop)return 0;for(const PrefabScene::Property* p=n->properties;p;p=p->next)if(FearBrushFlags::Equal(p->name,prop)){if(type&&!FearBrushFlags::Equal(p->type,type))return 0;return p;}return 0;}
 static float ParseReal(const char* s,float fallback){
  if(!s)return fallback;while(*s==' ')s++;bool neg=false;if(*s=='-'){neg=true;s++;}else if(*s=='+')s++;
  double v=0;bool digits=false;while(*s>='0'&&*s<='9'){v=v*10+(*s-'0');s++;digits=true;}
  if(*s=='.'){s++;double f=0.1;while(*s>='0'&&*s<='9'){v+=(*s-'0')*f;f*=0.1;s++;digits=true;}}
  if(!digits)return fallback;if(*s=='e'||*s=='E'){s++;bool en=false;if(*s=='-'){en=true;s++;}else if(*s=='+')s++;int e=0;while(*s>='0'&&*s<='9'){e=e*10+(*s-'0');s++;}double m=1;for(int i=0;i<e&&i<60;i++)m*=10;v=en?v/m:v*m;}
  return (float)(neg?-v:v);}
 static bool SameBox(const Rect3& a,const Rect3& b){return a.lo.x==b.lo.x&&a.lo.y==b.lo.y&&a.lo.z==b.lo.z&&a.hi.x==b.hi.x&&a.hi.y==b.hi.y&&a.hi.z==b.hi.z;}
 bool Add(const char* resource,const char* region,int kind,const Rect3& box){
  if(!resource||!resource[0])return true;if(kind==1&&(!region||!region[0]))kind=0;
  for(unsigned int i=0;i<pathCount;i++){const Entry& e=entries[i];if(e.kind==kind&&SameText(e.path,resource)&&(kind!=1||SameText(e.region,region))&&(kind!=2||SameBox(e.box,box)))return true;}
  if(pathCount==pathCapacity){unsigned int next=pathCapacity?pathCapacity*2:1024;Entry* grown=(Entry*)memory.Allocate(next*sizeof(Entry));if(!grown){failed=true;return false;}for(unsigned int i=0;i<pathCount;i++)grown[i]=entries[i];entries=grown;pathCapacity=next;}
  unsigned int n=lstrlenA(resource);char* copy=(char*)memory.Allocate(n+1);if(!copy){failed=true;return false;}lstrcpyA(copy,resource);
  char* regionCopy=0;if(kind==1){unsigned int m=lstrlenA(region);regionCopy=(char*)memory.Allocate(m+1);if(!regionCopy){failed=true;return false;}lstrcpyA(regionCopy,region);}
  Entry& e=entries[pathCount++];e.path=copy;e.region=regionCopy;e.kind=kind;e.box=box;return true;}
public:
 Gatherer(const PrefabScene::Model& m,PrefabScene::Arena& a,const char* stem,const char* const* regions,unsigned int regionTotal):model(m),memory(a),regionNames(regions),regionCount(regionTotal),entries(0),pathCount(0),pathCapacity(0),failed(false){
  unsigned int n=0;for(const char* p=stem;p&&*p&&n<MAX_PATH-12;p++)worldName[n++]=*p=='/'?'\\':*p;worldName[n]=0;lstrcatA(worldName,".World00p");}
 const Entry* Entries() const {return entries;}unsigned int Count() const {return pathCount;}bool Failed() const {return failed;}
 virtual ~Gatherer(){}
 virtual void Slot1(){}  // Condemned-only slot (index 1); no known caller
 virtual const char* GetWorldName(){return worldName;}
 virtual unsigned int GetNumRegions(){return regionCount;}
 virtual const char* GetRegionName(unsigned int i){return i<regionCount?regionNames[i]:0;}
 virtual bool DoesRegionHaveSkyPortal(const char*){return false;}
 virtual unsigned int GetNumRegionSurfaceTypes(const char*){return 0;}
 virtual unsigned int GetRegionSurfaceType(const char*,unsigned int){return 0;}
 virtual bool ContainedByResourceArea(const char*){return false;}
 virtual const char* GetResourceAreaName(const char*){return 0;}
 virtual bool GetResourceAreaIsGlobal(const char*){return false;}
 virtual bool ResourceAreaHasDimensions(const char*){return false;}
 virtual void GetResourceAreaDimensions(const char*,Rect3&){}
 virtual unsigned int GetNumResourceAreaRegions(const char*){return 0;}
 virtual const char* GetResourceAreaRegionName(const char*,unsigned int){return 0;}
 virtual void WarningMsg(const char* format,...){if(format)PackerLog::Warning("%s",format);}
 virtual void ErrorMsg(const char* format,...){if(format)PackerLog::Error("%s",format);}
 virtual bool AddResourceToRegion(const char* resource,const char* region){Rect3 none={{0,0,0},{0,0,0}};return Add(resource,region,region&&region[0]?1:0,none);}
 virtual bool AddWorldModelToRegion(const char*,const char*){return true;}  // world model resources come from the source seeds
 virtual bool AddResourceToOBB(const char* resource,const Rigid& t,const V3& half){
  // enclosing AABB of the rotated box: extent_i = sum_j |R_ij| * half_j (R from the quaternion x,y,z,w)
  float x=t.rot[0],y=t.rot[1],z=t.rot[2],w=t.rot[3];float r[3][3]={{1-2*(y*y+z*z),2*(x*y-z*w),2*(x*z+y*w)},{2*(x*y+z*w),1-2*(x*x+z*z),2*(y*z-x*w)},{2*(x*z-y*w),2*(y*z+x*w),1-2*(x*x+y*y)}};
  float h[3]={half.x,half.y,half.z},e[3];for(int a=0;a<3;a++){e[a]=0;for(int b=0;b<3;b++){float c=r[a][b];if(c<0)c=-c;e[a]+=c*h[b];}}
  Rect3 box={{t.pos.x-e[0],t.pos.y-e[1],t.pos.z-e[2]},{t.pos.x+e[0],t.pos.y+e[1],t.pos.z+e[2]}};return Add(resource,0,2,box);}
 virtual bool AddResourceToAABB(const char* resource,const Rect3& box){return Add(resource,0,2,box);}
 virtual bool AddWorldModelToAABB(const char*,const Rect3&){return true;}
 virtual Rigid GetTransform(const char* object){Rigid t={{0,0,0},{0,0,0,1}};const PrefabScene::Node* n=Find(object);if(!n)return t;t.pos.x=n->position.x;t.pos.y=n->position.y;t.pos.z=n->position.z;
  const float (*m)[4]=n->orientation.m;float trace=m[0][0]+m[1][1]+m[2][2];
  // 3x3 rotation to quaternion (x,y,z,w); the row/column convention only matters for resource areas
  if(trace>0){float s=1.0f;float r=trace+1.0f;float w=0;{float x=r;float g=x*0.5f;for(int i=0;i<12;i++)g=0.5f*(g+x/g);w=g*0.5f;}s=0.25f/w;t.rot[3]=w;t.rot[0]=(m[2][1]-m[1][2])*s;t.rot[1]=(m[0][2]-m[2][0])*s;t.rot[2]=(m[1][0]-m[0][1])*s;}
  return t;}
 virtual Rigid GetPrefabTransform(const char*){Rigid t={{0,0,0},{0,0,0,1}};return t;}
 virtual bool GetWorldModelAABB(const char*,Rect3&){return false;}
 virtual bool GetPropBool(const char* o,const char* p,bool d){const PrefabScene::Property* q=Prop(o,p,"bool");if(!q)return d;return q->data&&q->data[0]!='0'&&q->data[0]!=0;}
 virtual int GetPropLongInt(const char* o,const char* p,int d){const PrefabScene::Property* q=Prop(o,p,"longint");if(!q)return d;return (int)ParseReal(q->data,(float)d);}
 virtual float GetPropReal(const char* o,const char* p,float d){const PrefabScene::Property* q=Prop(o,p,"real");if(!q)return d;return ParseReal(q->data,d);}
 virtual V3 GetPropVector(const char* o,const char* p,const V3& d){const PrefabScene::Property* q=Prop(o,p);if(!q||!q->data)return d;WorldText::Document doc;if(!doc.Parse(q->data,lstrlenA(q->data)))return d;unsigned int form=doc.First(0),inner=doc.Next(doc.First(form));WorldText::Vec3 v;if(!inner||!doc.Vector(inner,v))return d;V3 r={v.x,v.y,v.z};return r;}
 virtual bool GetPropString(const char* o,const char* p,char* buffer,unsigned int size,const char* d){const PrefabScene::Property* q=Prop(o,p,"string");const char* v=q?q->data:d;if(buffer&&size){unsigned int i=0;while(v&&v[i]&&i+1<size){buffer[i]=v[i];i++;}buffer[i]=0;}return q!=0;}
 virtual bool GetPropCommand(const char* o,const char* p,char* buffer,unsigned int size,const char* d){const PrefabScene::Property* q=Prop(o,p,"command");const char* v=q?q->data:d;if(buffer&&size){unsigned int i=0;while(v&&v[i]&&i+1<size){buffer[i]=v[i];i++;}buffer[i]=0;}return q!=0;}
 virtual const char* GetObjectClass(const char* o){const PrefabScene::Node* n=Find(o);return n?n->className:0;}
 virtual unsigned int GetObjectsOfClass(const char* cls,const char** list,unsigned int size,unsigned int* total){unsigned int found=0,written=0;for(const PrefabScene::Node* n=model.first;n;n=n->next)if(n->className[0]&&cls&&FearBrushFlags::Equal(n->className,cls)){if(list&&written<size)list[written++]=n->name;found++;}if(total)*total=found;return written;}
 virtual bool DoesObjectExist(const char* o){return Find(o)!=0;}
 virtual bool DoesPropExist(const char* o,const char* p){return Prop(o,p)!=0;}
};
// Runs Pre -> class functions -> Post. Returns false (and a message) when the game module cannot be used.
inline bool Run(const char* gameServer,const char* resourceRoot,const PrefabScene::Model& model,PrefabScene::Arena& memory,const char* stem,const char* const* regions,unsigned int regionCount,const Gatherer::Entry*& files,unsigned int& count,const char*& error){
 files=0;count=0;error=0;
 HMODULE module=LoadLibraryExA(gameServer,0,8);if(!module){error="Could not load the project GameServer.dll";return false;}
 // The game module opens its databases (GameDatabase.dll, Dark.Gamdb00p, sound/FX/weapon DBs) exactly as WorldEdit does:
 // DEditLoadModule(resource root without trailing backslash); GameDatabase.dll is one directory above it.
 typedef bool (*LoadFn)(const char*);LoadFn load=(LoadFn)GetProcAddress(module,"DEditLoadModule");if(load){char root[MAX_PATH];lstrcpynA(root,resourceRoot,MAX_PATH);unsigned int n=lstrlenA(root);while(n>3&&(root[n-1]=='\\'||root[n-1]=='/'))root[--n]=0;Trace("DEditLoadModule",root);if(!load(root)){error="DEditLoadModule failed (project databases could not be opened)";return false;}}
 SetupFn setup=(SetupFn)GetProcAddress(module,"ObjectDLLSetup");PhaseFn pre=(PhaseFn)GetProcAddress(module,"GatherResourcesPreObjects"),post=(PhaseFn)GetProcAddress(module,"GatherResourcesPostObjects");
 if(!setup){error="GameServer.dll has no ObjectDLLSetup";return false;}
 int classCount=0,version=0;ClassDef** defs=setup(&classCount,0,&version);if(!defs||classCount<=0||classCount>4096||version!=1){error="Unexpected ObjectDLLSetup result";return false;}
 Gatherer gatherer(model,memory,stem,regions,regionCount);
 Trace("PRE");if(pre)pre(&gatherer);
 for(const PrefabScene::Node* n=model.first;n&&!gatherer.Failed();n=n->next){
  if(!n->className[0]||!n->name[0])continue;
  for(int c=0;c<classCount;c++)if(defs[c]->gather&&FearBrushFlags::Equal(defs[c]->name,n->className)){Trace(n->className,n->name);((ClassGatherFn)defs[c]->gather)(n->name,&gatherer);break;}
 }
 Trace("POST");if(post&&!gatherer.Failed())post(&gatherer);Trace("DONE");
 if(gatherer.Failed()){error="Resource gathering ran out of memory";return false;}
 files=gatherer.Entries();count=gatherer.Count();return true;
}

// Turns gathered entries into streaming seeds: path list + region index (0 = global, r+1 = streaming region r).
inline bool Resolve(const PrefabScene::Model& model,PrefabScene::Arena& memory,const char* const* names,unsigned int nameCount,const Gatherer::Entry* entries,unsigned int n,const char**& paths,unsigned int*& regionOf){
 paths=0;regionOf=0;paths=(const char**)memory.Allocate((n?n:1)*sizeof(char*));regionOf=(unsigned int*)memory.Allocate((n?n:1)*sizeof(unsigned int));if(!paths||!regionOf)return false;
 StreamingConvex::Block convex;bool haveConvex=nameCount&&convex.Build(model);
 for(unsigned int i=0;i<n;i++){
  paths[i]=entries[i].path;regionOf[i]=0;
  if(entries[i].kind==1){for(unsigned int r=0;r<nameCount;r++){const char* a=names[r];const char* b=entries[i].region;bool same=true;for(;;a++,b++){char x=*a,y=*b;if(x>='A'&&x<='Z')x+=32;if(y>='A'&&y<='Z')y+=32;if(x!=y){same=false;break;}if(!x)break;}if(same){regionOf[i]=r+1;break;}}}
  else if(entries[i].kind==2&&haveConvex){
   WorldText::Vec3 low,high;const Rect3& b=entries[i].box;low.x=b.lo.x-convex.offset.x;low.y=b.lo.y-convex.offset.y;low.z=b.lo.z-convex.offset.z;high.x=b.hi.x-convex.offset.x;high.y=b.hi.y-convex.offset.y;high.z=b.hi.z-convex.offset.z;
   if(!WorldText::FiniteVector(low)||!WorldText::FiniteVector(high))continue;
   for(unsigned int r=0;r<convex.regionCount&&r<nameCount;r++){bool matched=false;
    for(unsigned int v=0;v<convex.volumeCount&&!matched;v++)if(convex.volumes[v].owner==r){
     const StreamingConvex::Volume& source=convex.volumes[v];ConvexIntersection::Volume volume;volume.count=source.count;volume.low=*(const WorldText::Vec3*)convex.bounds[v].low;volume.high=*(const WorldText::Vec3*)convex.bounds[v].high;
     ConvexIntersection::Face* faces=(ConvexIntersection::Face*)memory.Allocate((source.count?source.count:1)*sizeof(ConvexIntersection::Face));if(!faces)return false;volume.faces=faces;
     for(unsigned int f=0;f<source.count;f++){faces[f].points=source.faces[f].points;faces[f].count=source.faces[f].count;faces[f].plane=source.faces[f].plane;}
     if(!ResourceAreaIntersection::Test(volume,low,high,matched))return false;}
    if(matched){regionOf[i]=r+1;break;}   // first intersecting region; the same entry is not duplicated into others
   }
  }
 }
 return true;
}
}
