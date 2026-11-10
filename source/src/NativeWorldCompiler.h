#pragma once
// In-process whole-world compiler (monolithic DLL core). It reproduces
// compile-source-world.py::compile_world for inline meshes with the same
// builders the worker EXEs use: one primary-profile scene for structure,
// Objects/BlindData, render and physics (streaming keeps its own scene through
// StreamingWorkerCore, as its worker does), the native MOPP compiler, and the
// assemble_world layout. AINavMesh sources are refused until the Nav generator
// is linked in (MONOLITHIC_DLL_PLAN.md step 3).
#include "NativePreparedRenderScene.h"
#include "SourceWorldTree.h"
#include "StreamingConvex.h"
#include "StreamingSettings.h"
#include "NativeWorldModelsStructure.h"
#include "NativeObjectsBlindData.h"
#include "RenderNvRuntime.h"
#include "NativeRenderCompiler.h"
#include "NativePhysicsMesh.h"
#include "NativePhysicsOwnerMeshTransport.h"
#include "NativePhysicsCompiler.h"
#include "StreamingWorkerCore.h"
#include "NativeMoppPrefix.h"
#include "NavDatabaseManifest.h"
#include "NativeWorldLog.h"
extern "C" __declspec(dllimport) BOOL WINAPI SetEnvironmentVariableA(const char*,const char*);
// nav-sdk-native/adapter/native-nav-library.cpp (SDK generator with the selected
// retail-2006 arithmetic objects; separate CRT translation unit).
extern "C" int __cdecl NativeNavProcessBuffer(const unsigned char*,unsigned,const unsigned char*,unsigned,float,float,float,unsigned char**,unsigned*,unsigned*,unsigned*,unsigned,int*,char*,unsigned);
extern "C" void __cdecl NativeNavFree(unsigned char*);
extern "C" void __cdecl NativeNavSetMessageSink(void (__cdecl*)(int,const char*,void*),void*);
#pragma comment(linker,"/alternatename:__imp__SetEnvironmentVariableA@8=__imp__SetEnvironmentVariableA")
namespace NativeWorldCompiler {
struct ExternalMesh {const char* name;const unsigned char* data;unsigned int size;};
struct Request {
 const char* source;const char* project;const char* stem;const char* moppDll;const char* postProfile; // auto|tutorial|multiplayer|singleplayer
 bool splitMeshes; // streaming-region geometry into Game-relative Mesh00p files
 void (*stage)(void*,const char*);void* context;
 // Packer log only: the World00p path and output folder announced with
 // CheckOutputFile, and the GetTickCount of "Processing" for "Done in".
 const char* outputWorld;const char* outputFolder;unsigned long startTicks;
 // Original packer options (WinPackerOriginalOptions.h).
 bool centerWorld,ignoreHidden,ignoreFrozen,optimizeVertices,rawNavMesh,objectsOnly,createObjectFile;
 const char* outputObjects; // CreateObjectFile target (<stem>.Objects00p)
 Request():source(0),project(0),stem(0),moppDll(0),postProfile("auto"),splitMeshes(false),stage(0),context(0),outputWorld(0),outputFolder(0),startTicks(0),centerWorld(true),ignoreHidden(false),ignoreFrozen(false),optimizeVertices(true),rawNavMesh(false),objectsOnly(false),createObjectFile(false),outputObjects(0){}
};
namespace detail {
inline void Put(unsigned char*& at,const void* p,unsigned int n){memcpy(at,p,n);at+=n;}
// Scoped compile_world environment: the readers locate the project through
// FEAR_READER_PROJECT; research/experiment switches stay unset.
class Environment {
 static const unsigned int Count=11;char saved[Count][MAX_PATH];bool present[Count];
 static const char* Name(unsigned int i){static const char* names[Count]={"FEAR_READER_INPUT","FEAR_READER_PROJECT","FEAR_OBJECT_ROTATION_REPLAY","FEAR_HULL_OUTPUT","FEAR_OWNER_QUATERNION_EXPERIMENT","FEAR_SOURCE_GEOMETRY_REPAIR","FEAR_SOURCE_EXCEPTIONAL_UV","FEAR_TRACE_REQUEST_OUTPUT","FEAR_SKEW_INPUT_OUTPUT","FEAR_PREFAB_BRUSH_MOVE_REPLAY","FEAR_TRACE_SEED_OUTPUT"};return names[i];}
public:
 Environment(const char* source,const char* project){for(unsigned int i=0;i<Count;i++){DWORD n=GetEnvironmentVariableA(Name(i),saved[i],MAX_PATH);present[i]=n&&n<MAX_PATH;SetEnvironmentVariableA(Name(i),0);}SetEnvironmentVariableA("FEAR_READER_INPUT",source);SetEnvironmentVariableA("FEAR_READER_PROJECT",project);}
 ~Environment(){for(unsigned int i=0;i<Count;i++)SetEnvironmentVariableA(Name(i),present[i]?saved[i]:0);}
};
// The recovered x87 helpers assume the default process control word 0x027F.
class Fpu {
 unsigned short saved;
public:
 Fpu(){unsigned short value=0x027F,old=0;__asm{fnstcw old}__asm{fldcw value}saved=old;}
 ~Fpu(){unsigned short value=saved;__asm{fldcw value}}
};
// native-winpacker-sections.cpp AssetListText: <stem>.Asset00 beside the source.
inline bool AssetListText(const char* source,PrefabScene::Arena& memory,const unsigned char*& text,unsigned int& size,char* pathOut=0){
 text=0;size=0;char path[MAX_PATH];if(!PrefabScene::Full(source,path))return false;unsigned int dot=0,length=lstrlenA(path);
 for(unsigned int i=0;i<length;i++){if(path[i]=='.')dot=i;else if(path[i]=='\\'||path[i]=='/')dot=0;}if(!dot)dot=length;if(dot+9>=MAX_PATH)return false;lstrcpyA(path+dot,".Asset00");if(pathOut)lstrcpyA(pathOut,path);
 HANDLE f=CreateFileA(path,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);if(f==INVALID_HANDLE_VALUE)return true;
 DWORD high=0,low=GetFileSize(f,&high),done=0;unsigned char* bytes=!high&&low<=8*1024*1024?(unsigned char*)memory.Allocate(low+1):0;
 bool ok=bytes&&(!low||(ReadFile(f,bytes,low,&done,0)&&done==low));CloseHandle(f);if(!ok)return false;text=bytes;size=low;return true;
}
}
class Compiler {
 PrefabScene::Arena memory;
 Compiler(const Compiler&);Compiler& operator=(const Compiler&);
 bool Fail(const char* stage,const char* why){char* at=error;const char* parts[3]={stage,": ",why?why:"failed"};for(unsigned int p=0;p<3;p++)for(const char* s=parts[p];*s&&at<error+sizeof(error)-1;s++)*at++=*s;*at=0;world=0;size=0;return false;}
 void Stage(const Request& r,const char* name){if(r.stage)r.stage(r.context,name);}
 const unsigned char* Copy(const void* data,unsigned int bytes){unsigned char* p=(unsigned char*)memory.Allocate(bytes?bytes:1);if(p&&bytes)memcpy(p,data,bytes);return p;}
public:
 unsigned char* world;unsigned int size;char error[512],postProfile[16];ExternalMesh* meshes;unsigned int meshCount;
 // Packer log statistics of the last compile.
 bool objectFile; // the result is a CreateObjectFile Objects00p, not a World00p
 bool reported;   // the failure is already in the packer log
 unsigned int moppVertices,moppTriangles;const char** meshRegion;unsigned int* meshSectors;NativeWorldLog::MainBsp mainBsp;
 Compiler():world(0),size(0),meshes(0),meshCount(0),objectFile(false),reported(false),moppVertices(0),moppTriangles(0),meshRegion(0),meshSectors(0){error[0]=0;postProfile[0]=0;mainBsp.valid=false;}
 void Clear(){memory.Clear();world=0;size=0;error[0]=0;postProfile[0]=0;meshes=0;meshCount=0;objectFile=false;reported=false;moppVertices=moppTriangles=0;meshRegion=0;meshSectors=0;mainBsp.valid=false;}
 bool Compile(const Request& request){
  Clear();if(!request.source||!request.project||!request.stem)return Fail("request","Explicit source, project and Game-relative world stem are required");
  char source[MAX_PATH],project[MAX_PATH];if(!PrefabScene::Full(request.source,source)||!PrefabScene::Full(request.project,project))return Fail("request","Invalid source or project path");
  char stem[MAX_PATH];unsigned int n=lstrlenA(request.stem);if(!n||n>=240)return Fail("request","Invalid Game-relative world stem");for(unsigned int i=0;i<=n;i++)stem[i]=request.stem[i]=='/'?'\\':request.stem[i];
  detail::Environment environment(source,project);detail::Fpu fpu;
  Stage(request,"source-capabilities");
  void* place=memory.Allocate(sizeof(NativePreparedRenderScene::Context));if(!place)return Fail("scene","allocation failed");NativePreparedRenderScene::Context* scene=new(place)NativePreparedRenderScene::Context;
  bool good=Run(request,*scene,source,stem);scene->~Context();return good;
 }
private:
 // Packer log phase clock (NativePreparedRenderScene::Context phase hook).
 DWORD phaseTicks[7];
 static void PhaseTick(void* self,unsigned int phase){if(phase<7)((Compiler*)self)->phaseTicks[phase]=GetTickCount();}
 unsigned long PhaseTime(unsigned int from,unsigned int to)const{return phaseTicks[to]-phaseTicks[from];}
 // SDK generator messages (IGameWorldPacker::eSeverity) go to the packer log in order.
 static void __cdecl NavMessage(int severity,const char* text,void*){if(severity==2)PackerLog::Error("%s",text);else if(severity==1)PackerLog::Warning("%s",text);else if(severity==3)PackerLog::Debug("%s",text);else PackerLog::Normal("%s",text);}
 // GetBrushFlags_Type (0x1001BFB9) warns once per process about an unknown Type
 // and gives the brush flags 0: first met while centering (0x1000AB50), or while
 // converting brushes (0x10007F5D) when CenterWorld is off.
 static void UnknownTypeWarning(const PrefabScene::Model& model){static bool warned=false;if(warned)return;for(const PrefabScene::Node* n=model.first;n;n=n->next)if(n->kind==1&&n->unknownType){PackerLog::Warning("Warning: Encountered an unexpected brush type '%s'",n->type);warned=true;return;}}
 // 0x10008E72: a split polygon crossing the plane a third time (its vertex plus the world offset).
 static void BadPolys(const SourceWorldModels::Builder& models,unsigned int block,const WorldText::Vec3& o){if(!models.blockBadCount||!models.blockBadPoints)return;for(unsigned int k=0;k<models.blockBadCount[block];k++){const WorldText::Vec3& p=models.blockBadPoints[block][k];volatile float x=p.x+o.x,y=p.y+o.y,z=p.z+o.z;PackerLog::Error("Bad poly found! > 2 edge splits! (%0.1f %0.1f %0.1f)",(double)x,(double)y,(double)z);}}
 static void GroupOverflow(void*,const char* group,const char* object){PackerLog::Error("There was not enough room in group object %s to add the object %s",group,object);}
 // Omission warnings of the Nav collection passes (VA 0x10004B40..0x10005088).
 static void NavOmission(void*,unsigned int kind,const char* first,const char* second,const WorldText::Vec3* at){
  static const char* const brush[6]={0,"Brush '%s' (at %f %f %f) is not of type \"Normal\"! Omitting from AINavMesh.","Brush '%s' (at %f %f %f) has more than 1 poly! Omitting from AINavMesh.","Brush '%s' (at %f %f %f) is concave! Omitting from AINavMesh.",
   "Brush '%s' (at %f %f %f) is downward facing! Omitting from AINavMesh.","Brush '%s' (at %f %f %f) is the child of a NavMeshLink with more than 1 child! Omitting from AINavMesh."};
  if(kind==0)PackerLog::Warning("NavMeshLink '%s' has more than 1 child! Omitting from AINavMesh.",first);
  else if(kind<=5&&at)PackerLog::Warning(brush[kind],first,(double)at->x,(double)at->y,(double)at->z);
  else if(kind==6)PackerLog::Warning("Carver '%s' has only than 1 poly! Omitting from AINavMesh.",first);
  else if(kind==7)PackerLog::Warning("Region '%s' contains brush '%s' with only than 1 poly! Omitting from AIRegion.",first,second);
  else if(kind==8)PackerLog::Warning("Region '%s' contains brush of type other than Normal! Omitting from AIRegion.",first);
  else if(kind==9)PackerLog::Warning("Region '%s' has no brushes! Omitting from AINavMesh.",first);
 }
 // WorldTree root size (0x10062540): the smaller horizontal extent of the root box.
 static float RootSize(const SourceWorldTree::Builder& tree){volatile float x=tree.high.x-tree.low.x,z=tree.high.z-tree.low.z;return x>z?z:x;}
 // Progress (0x10053600: the caller's float fraction times 100). n==0 sends
 // 0/0, the x87 default NaN, as the original does for a single MOPP model.
 static void ProgressFraction(unsigned int i,unsigned int n){if(!n){float nan;unsigned int bits=0xFFC00000u;memcpy(&nan,&bits,4);PackerLog::Progress(nan);return;}volatile float r=(float)i/(float)n;PackerLog::Progress((float)((double)r*100.0));}
 // Gathering a render tree (0x1003D8B4): every 500th polygon of its BSP world,
 // then 100. Trees follow the BSP blocks (main first).
 static void Gathering(const unsigned int* polygons,unsigned int blocks,unsigned int tree){unsigned int n=polygons&&tree<blocks?polygons[tree]:0;for(unsigned int i=0;i<n;i+=500)ProgressFraction(i,n);PackerLog::Progress(100);}
 // Closing lines (0x100272F0).
 static void Totals(const Request& request,unsigned int inputPolygons,unsigned int inputPoints,unsigned int outputPolygons,unsigned int outputPoints,unsigned int depth,unsigned int objectCount){
  PackerLog::Normal("");PackerLog::Normal("-------------------");
  if(request.startTicks){volatile float ms=(float)(int)(GetTickCount()-request.startTicks);PackerLog::Normal("Done in %.2f minutes",(double)ms/(double)1000.0f/(double)60.0f);}
  PackerLog::Normal("");PackerLog::Normal("Number of input polies: %d",inputPolygons);PackerLog::Normal("Number of input vertices: %d",inputPoints);PackerLog::Normal("");
  PackerLog::Normal("Number of output polies: %d",outputPolygons);PackerLog::Normal("Number of output vertices: %d",outputPoints);PackerLog::Normal("Tree depth: %d",depth);
  PackerLog::Normal("");PackerLog::Normal("Number of objects: %d",objectCount);PackerLog::Normal("");
 }
 // ObjectsOnly: Objects and BlindData replace those of the existing World00p
 // beside the source (header word 4, the BlindData start, follows); with
 // CreateObjectFile an "OBJT", 1, Objects file is written instead.
 bool ObjectsOnly(const Request& request,NativePreparedRenderScene::Context& scene,NativeObjectsBlindData::Settings& objectSettings,unsigned int inputPolygons,unsigned int inputPoints,unsigned int objectCount){
  NativeObjectsBlindData::Builder objects;if(!objects.BuildKnownTypes(scene,objectSettings))return ObjectsFail(scene,objects);
  PackerLog::Task("Packing");PackerLog::Scope packing("Packing For PC");
  if(request.createObjectFile){
   if(request.outputObjects)PackerLog::CheckFile(request.outputObjects);
   unsigned char* data=(unsigned char*)memory.Allocate(objects.objectsBytes+8);if(!data)return Fail("objects-and-blind-data","allocation failed");
   memcpy(data,"OBJT",4);unsigned int version=1;memcpy(data+4,&version,4);memcpy(data+8,objects.objects,objects.objectsBytes);world=data;size=objects.objectsBytes+8;objectFile=true;
   PackerLog::Normal("Successfully created object file %s",request.outputObjects?request.outputObjects:"");
  }else{
   PackerLog::Normal("  ");if(request.outputWorld)PackerLog::CheckFile(request.outputWorld);
   bool good=false;{
    PackerLog::Scope saving("Saving New Objects");
    HANDLE f=request.outputWorld?CreateFileA(request.outputWorld,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0):INVALID_HANDLE_VALUE;
    if(f==INVALID_HANDLE_VALUE)PackerLog::Error("Unable to open up world file %s for reading, unable to perform object only processing",request.outputWorld?request.outputWorld:"");
    else{DWORD high=0,bytes=GetFileSize(f,&high),done=0;unsigned char* old=!high&&bytes>=56&&bytes<=256u*1024u*1024u?(unsigned char*)memory.Allocate(bytes):0;bool read=old&&ReadFile(f,old,bytes,&done,0)&&done==bytes;CloseHandle(f);
     unsigned int objectsStart=read?NativeWorldLog::U32(old+12):0;
     if(!read||NativeWorldLog::U32(old)!=113||objectsStart<56||objectsStart>bytes)PackerLog::Error("Unable to open up world file %s for reading, unable to perform object only processing",request.outputWorld);
     else{unsigned int total=objectsStart+objects.objectsBytes+objects.blindBytes;unsigned char* data=(unsigned char*)memory.Allocate(total);if(!data)return Fail("objects-and-blind-data","allocation failed");
      memcpy(data,old,objectsStart);memcpy(data+objectsStart,objects.objects,objects.objectsBytes);memcpy(data+objectsStart+objects.objectsBytes,objects.blind,objects.blindBytes);
      unsigned int blindStart=objectsStart+objects.objectsBytes;memcpy(data+16,&blindStart,4);world=data;size=total;good=true;}}
   }
   if(!good){PackerLog::Error("An error occurred packing the file for the PC platform");packing.Close();lstrcpyA(error,"objects-only: the existing world could not be read");reported=true;return false;}
  }
  packing.Close();Totals(request,inputPolygons,inputPoints,0,0,0,objectCount);return true;
 }
 static void StreamingWarning(const char* text,unsigned int& warnings){PackerLog::Warning("%s:%s","Global",text);warnings++;}
 // Streaming reporter (0x10009E60): severity 1 warning, 2..3 error; "Global" without a node.
 static void StreamingMessage(unsigned int severity,const char* node,const char* text,unsigned int& warnings,unsigned int& errors){
  const char* name=node&&node[0]?node:"Global";if(severity==1){PackerLog::Warning("%s:%s",name,text);warnings++;}else{PackerLog::Error("%s:%s",name,text);errors++;}}
 // Stages run in the original's order (driver 0x10029880) so the log is live:
 // scene preparation, AINavMesh, Visibility, World Streaming, Import Geometry,
 // Packing. Every section is produced by the same builders as before.
 bool Run(const Request& request,NativePreparedRenderScene::Context& scene,const char* source,const char* stem){
  for(unsigned int i=0;i<7;i++)phaseTicks[i]=0;scene.phaseHook=PhaseTick;scene.phaseContext=this;
  NativePreparedRenderScene::Settings sceneSettings;sceneSettings.centerWorld=request.centerWorld;sceneSettings.ignoreHidden=request.ignoreHidden;sceneSettings.ignoreFrozen=request.ignoreFrozen;sceneSettings.objectsOnly=request.objectsOnly;
  const bool built=scene.Build(source,request.project,sceneSettings);scene.phaseHook=0;scene.phaseContext=0;
  if(!built&&!scene.VisibilityFatal())return Fail("scene",scene.Diagnostic().message);
  const PrefabScene::Model& model=scene.Model();
  // 0x10028A8A, after the decal pass: brush polygons/points and objects.
  unsigned int inputPolygons=0,inputPoints=0,objectCount=0,navRoots=0;
  for(const PrefabScene::Node* n=model.first;n;n=n->next){if(n->kind==1){inputPolygons+=n->geometry.polygons;inputPoints+=n->geometry.points;}else if(n->kind==2)objectCount++;}
  {
   using namespace PackerLog;
   Completed("Loading World",PhaseTime(1,2));Completed("Loading Prefabs",PhaseTime(2,3));Normal("Removing hidden and frozen objects");
   // 0x10027880: a prefab reference whose file cannot be loaded is reported and contributes nothing.
   Normal("Instantiating Prefabs");SubTask("Instantiating Prefabs");Indent(1);
   for(const PrefabScene::Model::MissingPrefab* m=model.missingPrefabs;m;m=m->next)Error("Unable to instantiate prefab %s (%s)",m->name,m->file);
   Finished("Instantiating Prefabs",0);Indent(-1);
   // 0x10028A2F: without CenterWorld only a warning replaces the centering scope.
   const WorldText::Vec3& o=scene.Offset();if(request.centerWorld){Normal("Centering World");SubTask("Centering World");Indent(1);
    UnknownTypeWarning(model);
    Normal("World offset by (%.2f %.2f %.2f)",(double)o.x,(double)o.y,(double)o.z);Finished("Centering World",PhaseTime(3,4));Indent(-1);}
   else Warning("Not offsetting world. This is only recommended for engineering and should not be used for other purposes");
   // 0x10028A4C: the original turns every Decal object (stricmp) into projected
   // geometry of its own world model (CreateDecalGeometry); that projection is not
   // recovered, so a Decal is refused instead of being packed as an ordinary object.
   for(const PrefabScene::Node* n=model.first;n;n=n->next)if(n->kind==2&&FearBrushFlags::Equal(n->className,"Decal")){
    Normal("Creating Decal Geometry");SubTask("Creating Decal Geometry");Indent(1);Error("Decal object %s: decal geometry is not supported by this packer",n->name);Finished("Creating Decal Geometry",0);Indent(-1);
    lstrcpyA(error,"decal geometry is not supported");reported=true;world=0;size=0;return false;}
   Completed("Creating Decal Geometry",0);Completed("Loading Game Server",PhaseTime(0,1));Completed("Applying Shadow Overrides",0);
   // FillInGroupObjects (0x1001BBE0): a runtime object (no CF_NORUNTIME) that finds
   // no empty Object slot in its enclosing Group is reported and left out.
   Normal("Filling in Group Objects");SubTask("Filling in Group Objects");Indent(1);
   {unsigned char* runtime=(unsigned char*)memory.Allocate(model.count+1);if(runtime){for(const PrefabScene::Node* n=model.first;n;n=n->next){const GameClassMetadata::Class* c=n->kind==2?scene.Catalog().Find(n->className):0;runtime[n->index]=(unsigned char)(n->kind==2&&!(c&&(c->flags&2)));}
    NativeObjectHierarchyLists::Overflows(model,runtime,GroupOverflow,0);}}
   Finished("Filling in Group Objects",0);Indent(-1);
   // ConvertKeyData (0x1000EB40) per KeyFramer, authored objects first: missing
   // BaseKeyName/KeyDataIndex or an unopenable external key file stop packing.
   Normal("Converting Key Data");SubTask("Converting Key Data");Indent(1);
   for(unsigned int pass=0;pass<2;pass++)for(const PrefabScene::Node* n=model.first;n;n=n->next){if(n->kind!=2||!GameClassMetadata::Exact(n->className,"KeyFramer")||(unsigned int)n->importedFromPrefab!=pass)continue;
    unsigned int check=NativeKeyFramer::Builder::Check(model,*n,scene.Project().resources);
    if(check==3)Warning("There were unused keys (duplicates or out of sequence) on KeyFramer: %s",n->name);
    else if(check==1||check==2){
     if(check==1)Error("Found out of date object.  Please update objects before reprocessing.");
     else{const char* external="";for(const PrefabScene::Property* p=n->properties;p;p=p->next)if(GameClassMetadata::Exact(p->name,"ExternalKeyFile")&&p->data){external=p->data;break;}Error("Error: unable to open the keyframe file %s",external);}
     Finished("Converting Key Data",0);Indent(-1);lstrcpyA(error,"objects-and-blind-data: KeyFramer conversion failed");reported=true;world=0;size=0;return false;}}
   Finished("Converting Key Data",0);Indent(-1);
   // ConvertStalkNodes (VA 0x1000B600) skips a node with a warning: child count,
   // child kind, then the StalkDataIndex property; authored objects first.
   Normal("Converting AI Nodes");SubTask("Converting AI Nodes");Indent(1);
   for(unsigned int pass=0;pass<2;pass++)for(const PrefabScene::Node* n=model.first;n;n=n->next){if(!StalkNodes::IsStalk(*n)||(unsigned int)n->importedFromPrefab!=pass)continue;
    const PrefabScene::Node* child=0;unsigned int children=0;for(const PrefabScene::Node* c=n->next;c;c=c->next)if(c->parent==n->index){child=c;children++;}
    if(children!=1)Warning("Object '%s', a '%s', requires a single brush child, with thickness.",n->name,"AINodeStalk");
    else if(child->kind!=1)Warning("Object '%s', a '%s', child is not a brush.",n->name,"AINodeStalk");
    else if(!StalkNodes::HasIndex(*n))Warning("Found out of date object.  Please resave this level before reprocessing.");}
   // Then the Node lists of AINodeGuard, Follow, Search, Safety (0x1000BED0): the
   // first node whose children find no empty Node slot is logged (0x1000BD24),
   // the remaining passes end and packing continues (NativeAINodeLists::Plan).
   {const PrefabScene::Node** nodes=(const PrefabScene::Node**)memory.Allocate((model.count+1)*sizeof(void*));unsigned char* apply=(unsigned char*)memory.Allocate(model.count+1);
    if(nodes&&apply){unsigned int k=0;for(const PrefabScene::Node* n=model.first;n;n=n->next)nodes[k++]=n;
     unsigned int overflowNode;const char* failure=0;
     if(NativeAINodeLists::Plan(nodes,k,apply,overflowNode,failure)&&overflowNode!=NativeAINodeLists::Missing)Error("AINode: %s has too many child Nodes!",nodes[overflowNode]->name);}}
   Finished("Converting AI Nodes",0);Indent(-1);
  }
  for(const PrefabScene::Node* n=model.first;n;n=n->next){if(n->kind!=2)continue;
   if(GameClassMetadata::Exact(n->className,"AINavMesh"))navRoots++;
   if(GameClassMetadata::Exact(n->className,"ScatterFX"))return Fail("source-capabilities","Native BlindData producer is not integrated for this source class");}
  const char* resources=scene.Project().resources;if(!resources[0])return Fail("source-capabilities","Project has no ResourceRoot");
  SourceWorldTree::Builder tree;if(!tree.Build(model,scene.Partition(),scene.Offset()))return Fail("source-capabilities",tree.error);
  float vectors[9];memcpy(vectors,&tree.low,12);memcpy(vectors+3,&tree.high,12);memcpy(vectors+6,&scene.Offset(),12);tree.Clear();
  const unsigned char* visibility=Copy(scene.Visibility().data,scene.Visibility().size);unsigned int visibilityBytes=scene.Visibility().size;
  // Streaming seed transport (sections worker layout).
  // Reserved/duplicate region names fail later, in the World Streaming stage.
  StreamingSettings::Settings streaming;const bool streamingRead=streaming.Read(model);
  const unsigned char* assetText=0;unsigned int assetBytes=0;char assetPath[MAX_PATH];assetPath[0]=0;if(!detail::AssetListText(source,memory,assetText,assetBytes,assetPath))return Fail("source-capabilities","Asset list read failed");
  unsigned int seedBytes=lstrlenA(stem)+1+4+8;for(unsigned int i=0;i<streaming.count;i++)seedBytes+=lstrlenA(streaming.names[i])+1+4;seedBytes+=assetBytes;
  unsigned char* seed=(unsigned char*)memory.Allocate(seedBytes);if(!seed)return Fail("source-capabilities","Streaming seed allocation failed");unsigned char* at=seed;unsigned int zero=0;
  detail::Put(at,stem,lstrlenA(stem)+1);detail::Put(at,&streaming.count,4);for(unsigned int i=0;i<streaming.count;i++){detail::Put(at,streaming.names[i],lstrlenA(streaming.names[i])+1);detail::Put(at,streaming.budgets+i,4);}
  detail::Put(at,&zero,4);detail::Put(at,&assetBytes,4);detail::Put(at,assetText,assetBytes);if((unsigned int)(at-seed)!=seedBytes)return Fail("source-capabilities","Streaming seed framing mismatch");
  // AINavMesh (0x10028AE0): SDK messages, then the GameServer summary (0x100017D0).
  Stage(request,"objects-and-blind-data");
  NativeObjectsBlindData::Settings objectSettings;
  PackerLog::Task("AINavMesh");
  {
   PackerLog::Scope scope("Building AINavMesh");
   if(navRoots){
    NativeRawNavMeshGeneral::Builder::Omissions(model,scene.Offset(),NavOmission,0);
    // RawNavMesh (0x10028B81): no SDK processing; the record keeps the raw packet.
    if(request.rawNavMesh)objectSettings.rawNavigation=true;else if(!Navigation(request,scene,objectSettings.navigation))return false;
    NativeRawNavMeshGeneral::Builder counts;
    if(counts.Build(model,scene.Offset())){PackerLog::Normal("\nAINavMesh successfully compiled.");PackerLog::Normal("Brushes: %d",counts.brushCount);PackerLog::Normal("Carvers: %d",counts.carverCount);PackerLog::Normal("Regions: %d",counts.regionCount);PackerLog::Normal("Nodes: %d",counts.nodeCount);}
   }else PackerLog::Normal("No AINavMesh found in level.");
  }
  // ObjectsOnly / CreateObjectFile (0x10029B95, 0x1005265A): objects only, no geometry stages.
  if(request.objectsOnly)return ObjectsOnly(request,scene,objectSettings,inputPolygons,inputPoints,objectCount);
  // Visibility (0x10027C10): skipped extrusion warnings, then the block header.
  PackerLog::Task("Visibility");
  {
   PackerLog::Scope scope("Creating Visibility");const VisibilityCompiler::Builder& v=scene.Visibility();
   // Sector builder notes in order (VisibilityCompiler.h), reporter 0x10027B80:
   // "Sector %s: " for a sector node, warning for severity 1, else error.
   for(unsigned int i=0;i<v.noteCount;i++){const VisibilityCompiler::Note& n=v.notes[i];
    switch(n.kind){
     case VisibilityCompiler::UnusablePolygon:PackerLog::Warning("Sector %s: Found polygon that was not usable. This could be because the polygon has no area, is concave, or is nonplanar. Poly midpoint: (%.2f %.2f %.2f). Skipping polygon.",n.first,(double)n.midpoint.x,(double)n.midpoint.y,(double)n.midpoint.z);break;
     case VisibilityCompiler::ConcaveSector:PackerLog::Warning("Sector %s: Found concave sector. Sector brushes must be convex. Skipping sector.",n.first);break;
     case VisibilityCompiler::OpenSector:PackerLog::Warning("Sector %s: Found open sector. Sectors must be closed brushes. Skipping sector.",n.first);break;
     case VisibilityCompiler::ExtrusionBrush:PackerLog::Warning("Invalid brush %s used for extrude sector. Extrusion sectors must have exactly one polygon in the brush. Ignoring sector.",n.first);break;
     case VisibilityCompiler::ExtrusionPolygon:PackerLog::Warning("Invalid polygon used for extrude sector %s. Ignoring extrude sector.",n.first);break;
     case VisibilityCompiler::ExtrusionMissed:PackerLog::Warning("Extrusion sector %s failed to hit a sector on one or more sides. Ignoring extrusion sector.",n.first);break;
     case VisibilityCompiler::ExtrusionIntersects:PackerLog::Warning("Sector %s: Found sector that intersects extrusion sector %s. Unable to extrude, ignoring extrude sector.",n.first,n.second);break;
     case VisibilityCompiler::SectorsIntersect:PackerLog::Error("Sector %s: Sector intersects with another sector: %s",n.first,n.second);break;
     case VisibilityCompiler::InvalidPortal:PackerLog::Warning("Sector %s: Generated invalid portal to sector %s",n.first,n.second);break;
     case VisibilityCompiler::IntersectingSets:PackerLog::Error("Found %d set(s) of intersecting sectors. Unable to proceed with visibility.",n.number);break;}}
   // 0x10027CA5: a failed visibility build stops packing.
   if(scene.VisibilityFatal()){PackerLog::Error("Error building visibility");scope.Close();lstrcpyA(error,"visibility: intersecting sectors");reported=true;world=0;size=0;return false;}
   PackerLog::Normal("Visibility successfully compiled.");
   if(visibilityBytes>=32){PackerLog::Normal("Sectors: %d",NativeWorldLog::U32(visibility));PackerLog::Normal("Portals: %d",NativeWorldLog::U32(visibility+4));
    // 0x10027D20: the depth line needs a sector tree root (none without sectors).
    if(NativeWorldLog::U32(visibility))PackerLog::Normal("Sector Tree Depth: %d",NativeWorldLog::U32(visibility+12));}
   scope.CloseAfter(PhaseTime(4,5));
  }
  // World Streaming (0x10009EE0): streaming info, region and asset list checks.
  Stage(request,"streaming-and-resources");
  PackerLog::Task("World Streaming");
  const unsigned char* sStreaming=0;unsigned int streamingBytes=0;
  {
   PackerLog::Scope scope("Creating World Streaming");unsigned int warnings=0,errors=0;
   // Region/link/info conversion, info checks, names, removals and links in the
   // original's order (StreamingStructure.h); reporter "%s:%s" with the node name.
   WorldText::Vec3 center=scene.Offset();StreamingStructure::Builder structure;if(!structure.Build(model,center,StreamingConvex::Block::InsideBrush,&center))return Fail("streaming-and-resources",structure.error);
   for(unsigned int i=0;i<structure.messageCount;i++){const StreamingStructure::Message& m=structure.messages[i];StreamingMessage(m.severity,m.node,m.text,warnings,errors);}
   if(structure.fatal){PackerLog::Normal("Streaming Errors: %d",errors);PackerLog::Normal("Streaming Warnings: %d",warnings);scope.Close();lstrcpyA(error,"streaming-and-resources: reserved or duplicate streaming region name");reported=true;world=0;size=0;return false;}
   if(!streamingRead)return Fail("streaming-and-resources","Streaming settings read failed");
   StreamingConvex::Block convex;if(!convex.Build(model))return Fail("streaming-and-resources",convex.error);
   // Asset list (0x10039590): open failure, then per-line notes.
   if(!assetText){char message[MAX_PATH+160];lstrcpyA(message,"Failed to open asset list ");lstrcatA(message,assetPath);lstrcatA(message," for reading. The file may either not exist or may be in use by another application.");StreamingWarning(message,warnings);}
   else{AssetList::List notes;if(notes.Read(assetText,assetBytes,streaming.names,streaming.count))for(unsigned int i=0;i<notes.noteCount;i++){const AssetList::Note& note=notes.notes[i];
    if(note.kind==0)PackerLog::Warning("%s:Asset List (%d): Encountered an unexpected section code in the level asset file: %s","Global",note.line,note.text);
    else PackerLog::Warning("%s:Asset List (%d): Found resource that did not have an associated streaming region. This resource will be ignored.","Global",note.line);warnings++;}}
   // Sector placement (0x10037990) in visibility save order, reported on the sector brush.
   if(convex.regionCount){
    const VisibilityCompiler::Builder& v=scene.Visibility();StreamingSourceMetadata::Builder metadata;
    if(metadata.Build(convex,v.streamingSectors,v.sectorCount,v.offset))for(unsigned int id=0;id<v.sectorCount;id++){
     unsigned int i=0;while(i<v.sectorCount&&v.streamingSectors[i].index!=id)i++;if(i==v.sectorCount)continue;
     bool outside=metadata.assignment.owners[i]<0,multiple=metadata.assignment.multiple&&metadata.assignment.multiple[id];if(!outside&&!multiple)continue;
     const char* name="Global";for(const PrefabScene::Node* n=model.first;n;n=n->next)if(v.sectorSources&&n->index==v.sectorSources[i]){name=n->name;break;}
     const ConvexIntersection::Volume& box=v.streamingSectors[i].volume;const WorldText::Vec3& o=scene.Offset();
     volatile float cx=(box.low.x+box.high.x)*0.5f,cy=(box.low.y+box.high.y)*0.5f,cz=(box.low.z+box.high.z)*0.5f;volatile float x=cx+o.x,y=cy+o.y,z=cz+o.z;
     if(outside){PackerLog::Warning("%s:Found sector (%.2f, %.2f, %.2f) that is not located within any streaming regions.",name,(double)x,(double)y,(double)z);warnings++;}
     else{PackerLog::Error("%s:Found sector (%.2f, %.2f, %.2f) that is located within multiple streaming regions, this sector will only be placed in one of the regions!",name,(double)x,(double)y,(double)z);errors++;}}
   }
   StreamingWorkerCore::Request streamingRequest;streamingRequest.source=source;streamingRequest.input=seed;streamingRequest.bytes=seedBytes;streamingRequest.model=&model;streamingRequest.sourceMaterials=true;streamingRequest.textureMode='2';
   // CONDEMNED FORK: profile "none" = no FEAR game resource post-gathering (FEAR sound/ClientFX databases are not Condemned's).
   const bool gather=request.postProfile&&FearBrushFlags::Equal(request.postProfile,"condemned");
   const bool noPost=gather||(request.postProfile&&FearBrushFlags::Equal(request.postProfile,"none"));
   streamingRequest.soundDatabase=!noPost;streamingRequest.clientFx=!noPost;streamingRequest.visibility=true;streamingRequest.condemnedGather=gather;
   // compile_world FEAR_ASSEMBLY_<PROFILE>_POST; the resolved name must agree with an explicit request.
   const char* wanted=request.postProfile?request.postProfile:"auto";streamingRequest.automaticPost=FearBrushFlags::Equal(wanted,"auto");streamingRequest.tutorialPost=FearBrushFlags::Equal(wanted,"tutorial");
   streamingRequest.multiplayerPost=FearBrushFlags::Equal(wanted,"multiplayer");streamingRequest.singleplayerPost=FearBrushFlags::Equal(wanted,"singleplayer");
   if(!noPost&&!streamingRequest.automaticPost&&!streamingRequest.tutorialPost&&!streamingRequest.multiplayerPost&&!streamingRequest.singleplayerPost)return Fail("streaming-and-resources","Unknown resource post-profile");
   const char* post=0;unsigned int code=StreamingWorkerCore::Run(streamingRequest,memory,sStreaming,streamingBytes,post);
   if(code){char why[64];lstrcpyA(why,"worker exit ");*Decimal(why+12,code)=0;return Fail("streaming-and-resources",why);}
   if(!post||!(noPost&&(FearBrushFlags::Equal(post,"none")||gather))&&!(FearBrushFlags::Equal(post,"tutorial")||FearBrushFlags::Equal(post,"multiplayer")||FearBrushFlags::Equal(post,"singleplayer"))||(!streamingRequest.automaticPost&&!gather&&!FearBrushFlags::Equal(post,wanted)))return Fail("streaming-and-resources","Inconsistent resource post-profile");lstrcpynA(postProfile,post,sizeof(postProfile));
   PackerLog::Normal("Streaming Errors: %d",errors);PackerLog::Normal("Streaming Warnings: %d",warnings);
  }
  // Import Geometry (0x100293F0): physics BSPs, world tree, texture flags.
  Stage(request,"world-structure");
  PackerLog::Task("Import Geometry");
  // 0x1001CE87: every WorldModel without a brush is reported, then packing stops.
  {const WorldModels::Partition& groups=scene.Partition();bool empty=false;
   for(unsigned int g=1;g<groups.groupCount;g++)if(!groups.groups[g].brushes&&groups.groups[g].owner){PackerLog::Error("Found a world model '%s' with no brushes beneath it. This could be caused by having a world model with all brushes underneath it either hidden or frozen or of the type visualization. A world model must have at least one brush.",groups.groups[g].owner->name);empty=true;}
   if(empty){lstrcpyA(error,"world-structure: WorldModel without brushes");reported=true;world=0;size=0;return false;}}
  PackerLog::Completed("Extracting Physics Shapes",0);
  NativeWorldModelsStructure::Builder structure;NativeWorldModelsStructure::Settings structureSettings;structureSettings.physical.fragmentLimit=500000;structureSettings.physical.pointLimit=4000000;
  const unsigned char *sTree=0,*sModels=0,*sOrdinary=0,*sParticles=0;unsigned int treeBytes=0,modelBytes=0,ordinaryBytes=0,particleBytes=0,modelCount=0,texturePolygons=0,mainInternalPolygons=0,blockCount=0,*blockPolygons=0;
  {
   PackerLog::Scope scope("Creating Physics BSPs",false);DWORD started=GetTickCount();
   if(!structure.Build(scene,structureSettings))return Fail("world-structure",structure.Error().message);
   const SourceWorldTree::Builder& treeBlock=structure.Structural().Tree();const SourceWorldModels::Builder& models=structure.Models();const SourceBlockerPolygons::Block& blockers=structure.Structural().OrdinaryBlockers();const ParticleBlockers::Block& particles=structure.Structural().ParticleBlockersData();
   if(!treeBlock.size||!models.size||!blockers.size||!particles.size||!models.modelCount)return Fail("world-structure","Empty native structural stream");
   sTree=Copy(treeBlock.data,treeBlock.size);sModels=Copy(models.data,models.size);sOrdinary=Copy(blockers.data,blockers.size);sParticles=Copy(particles.data,particles.size);
   treeBytes=treeBlock.size;modelBytes=models.size;ordinaryBytes=blockers.size;particleBytes=particles.size;modelCount=models.modelCount;blockCount=models.modelCount;
   blockPolygons=(unsigned int*)memory.Allocate((models.modelCount+1)*4);if(!blockPolygons)return Fail("world-structure","allocation failed");for(unsigned int b=0;b<models.modelCount;b++)blockPolygons[b]=models.blockPolygons?models.blockPolygons[b]:0;texturePolygons=models.internalPolygons;mainInternalPolygons=models.mainInternalPolygons;
   PackerLog::SubTask("PhysicsBSP BSP");PackerLog::SubTask("Converting Brushes");PackerLog::Normal("Transferring brush polygons to world");if(!request.centerWorld)UnknownTypeWarning(model);
   if(models.blockBrushes)for(unsigned int i=0;i<models.blockBrushes[0];i++)ProgressFraction(i,models.blockBrushes[0]);
   PackerLog::Normal("Converting brushes took %.2f seconds",(double)(int)(GetTickCount()-started)/1000.0);PackerLog::SubTask("Physics BSP");if(models.blockProgress)for(unsigned int k=0;k<models.blockProgressCount[0];k++)ProgressFraction(1000*(k+1),models.blockProgress[0][k]);BadPolys(models,0,scene.Offset());
   PrefabScene::Arena scratch;NativeWorldLog::Bsp bsp;if(bsp.Read(sModels,modelBytes,scratch,mainBsp))PackerLog::Normal("BSP tree %0.2f%% balanced.",mainBsp.balance*100.0);
   for(unsigned int b=1;b<models.modelCount&&models.blockNames;b++){char name[300];lstrcpynA(name,models.blockNames[b],64)/* the original's name buffer holds 63 characters (Factory) */;lstrcatA(name," BSP");PackerLog::SubTask(name);if(models.blockBrushes)for(unsigned int i=0;i<models.blockBrushes[b];i++)ProgressFraction(i,models.blockBrushes[b]);if(models.blockProgress)for(unsigned int k=0;k<models.blockProgressCount[b];k++)ProgressFraction(1000*(k+1),models.blockProgress[b][k]);BadPolys(models,b,scene.Offset());}
   PackerLog::Normal("Generated %d unique planes for the world and world models",models.initialNormalCount);
   scope.Close();
   PackerLog::Scope worldTree("Creating World Tree");PackerLog::Normal("WorldTree nodes: %d",treeBlock.nodeCount);PackerLog::Normal("WorldTree depth: %d",treeBlock.maxDepth);
   PackerLog::Normal("WorldTree root size: %.2f",(double)RootSize(treeBlock));worldTree.Close();
   structure.Clear();
  }
  {PackerLog::Scope textures("Applying Textures");PackerLog::Normal("Imported %d texture flags",texturePolygons);}
  // 0x1002522C: Blocker polygons below a WorldModel are reported (not fatal).
  {const WorldModels::Partition& groups=scene.Partition();
   for(unsigned int g=1;g<groups.groupCount;g++){unsigned int blockers=0;for(unsigned int a=0;a<groups.assignmentCount;a++)if(groups.assignments[a].group==g&&FearBrushFlags::Equal(groups.assignments[a].brush->type,"Blocker"))blockers+=groups.assignments[a].brush->geometry.polygons;
    if(blockers&&groups.groups[g].owner)PackerLog::Error("Found %d physics blockers under world model %s. Blockers under world models will cause issues with the world model bounding box. Please remove these blockers.",blockers,groups.groups[g].owner->name);}}
  // 0x10029C4A -> 0x100122E0: Shatter WorldModels without a real BlindObjectIndex.
  {NativeShatter::Builder demand;if(demand.Build(scene,objectSettings.shatter,true))for(unsigned int i=0;i<demand.missingIndexCount;i++)PackerLog::Warning("Unable to find a property to store the blind object index inside of when generating shatter information for the world model %s.",demand.missingIndex[i]);}
  NativeObjectsBlindData::Builder objects;if(!objects.BuildKnownTypes(scene,objectSettings))return ObjectsFail(scene,objects);
  const unsigned char* sObjects=Copy(objects.objects,objects.objectsBytes);const unsigned char* sBlind=Copy(objects.blind,objects.blindBytes);unsigned int objectBytes=objects.objectsBytes,blindBytes=objects.blindBytes;objects.Clear();
  // Packing (0x10027A70).
  PackerLog::Task("Packing");PackerLog::Scope packing("Packing For PC");PackerLog::Normal("  ");if(request.outputWorld)PackerLog::CheckFile(request.outputWorld);
  {
   NativeWorldLog::WorldModelsHeader h;if(NativeWorldLog::ReadWorldModels(sModels,modelBytes,h)){
    PackerLog::Normal("World Bsp Information:");PackerLog::Indent(1);PackerLog::Normal("World Model Objects: %d",h.names);PackerLog::Normal("World BSP Objects: %d",h.models);PackerLog::Normal("World Planes: %d",h.normals);
    PackerLog::Normal("World BSP Nodes: %d",h.nodes);PackerLog::Normal("World Polys: %d",h.polygons);PackerLog::Normal("World Vert Refs: %d",h.references);PackerLog::Normal("World Verts: %d",h.points);PackerLog::Indent(-1);}
  }
  Stage(request,"physics");
  const unsigned char* sPhysics=0;unsigned int physicsBytes=0;
  {PackerLog::Scope shapes("Creating Physics Shapes");if(!Physics(request,scene,sPhysics,physicsBytes))return false;PackerLog::Normal("Generated MOPP with %d vertices and %d trianges",moppVertices,moppTriangles);for(unsigned int i=0;i<blockCount;i++)ProgressFraction(i,blockCount-1);}
  Stage(request,"render");
  const unsigned char* sRender=0;unsigned int renderBytes=0,mainTreeBytes=0,modelTreeBytes=0;
  {
   PackerLog::Scope rendering("Generating Rendering Data");
   char renderStem[MAX_PATH],full[MAX_PATH];if(!PrefabScene::Join(resources,stem,renderStem)||!PrefabScene::Full(renderStem,full))return Fail("render","World stem path exceeds MAX_PATH");
   // compile_world passes the split resource root with a trailing separator.
   char root[MAX_PATH],slashed[MAX_PATH];lstrcpynA(slashed,resources,MAX_PATH-2);if(request.splitMeshes)lstrcpyA(slashed+lstrlenA(slashed),"/");if(!PrefabScene::Full(slashed,root))return Fail("render","Resource root exceeds MAX_PATH");
   void* p=memory.Allocate(sizeof(NativeMaterialLayouts::Cache));if(!p)return Fail("render","allocation failed");NativeMaterialLayouts::Cache* materials=new(p)NativeMaterialLayouts::Cache;
   if(!materials->Initialize(root)){const char* m=materials->Error().message;materials->~Cache();return Fail("render",m);}
   NativeRenderCompiler::Resolver resolver;resolver.resolve=NativeMaterialLayouts::Cache::Callback;resolver.context=materials;NativeRenderCompiler::Settings settings;settings.worldStem=full;settings.projectRoot=root;settings.splitMeshes=request.splitMeshes;settings.cacheReorder=request.optimizeVertices;
   p=memory.Allocate(sizeof(NativeRenderCompiler::Builder));if(!p){materials->~Cache();return Fail("render","allocation failed");}NativeRenderCompiler::Builder* render=new(p)NativeRenderCompiler::Builder;
   bool good=render->Build(scene,settings,resolver);unsigned int trees=0,shadow=0,base=0,mainNodes=0;
   if(good){const RenderSourceGlobalMeshes::Block& block=render->Result();if(!block.meshCount||block.meshCount>4096||(!request.splitMeshes&&block.meshCount!=1))good=false;else{
     sRender=Copy(block.data,block.size);renderBytes=block.size;if(!CopyMeshes(block)){render->~Builder();materials->~Cache();return false;}
     meshRegion=(const char**)memory.Allocate((meshCount+1)*sizeof(char*));meshSectors=(unsigned int*)memory.Allocate((meshCount+1)*4);if(!meshRegion||!meshSectors){render->~Builder();materials->~Cache();return Fail("render","allocation failed");}
     trees=block.size>=4?NativeWorldLog::U32(block.data):0;NativeWorldLog::RenderTreeBytes(block.data,block.size,block.meshes,block.meshCount,mainTreeBytes,modelTreeBytes);
     const NativeRenderCompiler::NodeStats* reports=render->NodeReports();// 0x1003F547 counts the main world tree only (BetterBox, SampleWorld, Expanded).
    for(unsigned int n=0;reports&&n<scene.Trees().totalNodeCount;n++)if(!reports[n].group){mainNodes++;shadow+=reports[n].shadowTriangles;base+=reports[n].shadowBase;}
    for(unsigned int m=0;m<meshCount;m++){const RenderGlobalMeshes::Mesh& mesh=block.meshes[m+1];{const char* region=mesh.regionIndex<block.regionCount&&block.regions[mesh.regionIndex].name?block.regions[mesh.regionIndex].name:"";char* copy=(char*)memory.Allocate(lstrlenA(region)+1);if(copy)lstrcpyA(copy,region);meshRegion[m]=copy?copy:"";}meshSectors[m]=mesh.regionIndex<block.regionCount?block.regions[mesh.regionIndex].count:0;}}}
   if(!good){lstrcpynA(error,render->Error().message[0]?render->Error().message:"Inline render request returned external meshes",sizeof(error));render->~Builder();materials->~Cache();char why[512];lstrcpynA(why,error,512);return Fail("render",why);}
   render->~Builder();materials->~Cache();
   // 0x1003F440: the main tree is gathered and optimized first, then the WorldModel trees.
   if(trees){PackerLog::SubTask("Gathering Processed Data");PackerLog::SubTask("Gathering Processed Data");Gathering(blockPolygons,blockCount,0);}
   PackerLog::SubTask("Optimizing Sections");for(unsigned int i=0;i<mainNodes;i++)ProgressFraction(i+1,mainNodes);
   volatile float total=(float)(shadow+base),denominator=(float)base;PackerLog::Normal("%d triangles added for shadow volumes  (+ %d = %d (x%0.2f) total)",shadow,base,shadow+base,(double)total/(double)denominator);
   for(unsigned int t=1;t<trees;t++){PackerLog::SubTask("Gathering Processed Data");PackerLog::SubTask("Gathering Processed Data");Gathering(blockPolygons,blockCount,t);}
   PackerLog::Completed("Generating Material Ordering Keys",0);
  }
  for(unsigned int m=0;m<meshCount;m++){
   if(request.outputFolder){char target[MAX_PATH];if(PrefabScene::Join(request.outputFolder,meshes[m].name,target)){for(char* q=target;*q;q++)if(*q=='/')*q='\\';PackerLog::CheckFile(target);}}
   PackerLog::Normal("Created %.2fMB mesh for %d sectors in region %s",(double)meshes[m].size/1048576.0,meshSectors[m],meshRegion[m]);
  }
  PackerLog::Normal("Main world mesh size: %.2fMB",(double)mainTreeBytes/1048576.0);PackerLog::Normal("World model mesh size: %.2fMB",(double)modelTreeBytes/1048576.0);
  Stage(request,"assembly");
  const unsigned char* parts[10]={sTree,sModels,sOrdinary,sPhysics,sStreaming,sParticles,sRender,visibility,sObjects,sBlind};
  unsigned int bytes[10]={treeBytes,modelBytes,ordinaryBytes,physicsBytes,streamingBytes,particleBytes,renderBytes,visibilityBytes,objectBytes,blindBytes};
  if(meshCount&&!StreamingHasMeshes(sStreaming,streamingBytes))return false;
  if(!Assemble(vectors,parts,bytes,modelCount))return false;
  // Size table (0x1005251A), packing scope end, totals (0x100272F0).
  PackerLog::Normal("  ");NativeWorldLog::Size("FEAR World .World00p file Size",size);
  const char* labels[10]={"  World Tree","  World Models","  Blockers","  Physics Shapes","  Streaming","  Particle Blockers","  Render Data","  Visibility Data","  Objects","  Blind Object Data"};
  for(unsigned int i=0;i<10;i++)NativeWorldLog::Size(labels[i],bytes[i]);
  packing.Close();
  Totals(request,inputPolygons,inputPoints,mainInternalPolygons,mainBsp.valid?mainBsp.points:0,mainBsp.valid?mainBsp.depth:0,objectCount);
  return true;
 }
 // compile-source-world.py _mesh_game_path: relative Game path without traversal,
 // drive, device names or unsafe characters, with the Mesh00p extension.
 static bool SafeMeshPath(const char* name){
  unsigned int n=name?lstrlenA(name):0;if(!n||n>259||name[0]=='\\'||name[0]=='/')return false;if(n<8)return false;const char* ext=name+n-8;const char* want=".mesh00p";for(unsigned int i=0;i<8;i++){char c=ext[i];if(c>='A'&&c<='Z')c=(char)(c+32);if(c!=want[i])return false;}
  unsigned int start=0;for(unsigned int i=0;i<=n;i++){char c=name[i];if(c&&c!='\\'&&c!='/'){if((unsigned char)c<32||(unsigned char)c>126||c==':'||c=='*'||c=='?'||c=='"'||c=='<'||c=='>'||c=='|')return false;continue;}
   unsigned int length=i-start;if(!length)return false;const char* part=name+start;if(part[length-1]=='.'||part[length-1]==' ')return false;if(length==1&&part[0]=='.')return false;if(length==2&&part[0]=='.'&&part[1]=='.')return false;
   char device[8];unsigned int d=0;while(d<length&&d<7&&part[d]!='.'){char c=part[d];device[d]=c>='a'&&c<='z'?(char)(c-32):c;d++;}device[d]=0;
   if(d<=length&&(part[d]=='.'||d==length)){const char* devices[6]={"CON","PRN","AUX","NUL","CONIN$","CONOUT$"};for(unsigned int k=0;k<6;k++)if(FearBrushFlags::Equal(device,devices[k])&&lstrlenA(devices[k])==(int)d)return false;if(d==4&&(device[0]=='C'&&device[1]=='O'&&device[2]=='M'||device[0]=='L'&&device[1]=='P'&&device[2]=='T')&&device[3]>='1'&&device[3]<='9')return false;}
   start=i+1;}
  return true;
 }
 static bool SameText(const char* a,const char* b){for(;;a++,b++){char x=*a,y=*b;if(x>='A'&&x<='Z')x=(char)(x+32);if(y>='A'&&y<='Z')y=(char)(y+32);if(x=='/')x='\\';if(y=='/')y='\\';if(x!=y)return false;if(!x)return true;}}
 bool CopyMeshes(const RenderSourceGlobalMeshes::Block& block){
  meshCount=0;meshes=(ExternalMesh*)memory.Allocate(block.meshCount*sizeof(ExternalMesh));if(!meshes)return Fail("render","allocation failed");
  for(unsigned int m=1;m<block.meshCount;m++){const RenderGlobalMeshes::Mesh& mesh=block.meshes[m];if(!mesh.name||!SafeMeshPath(mesh.name)||!mesh.external||!mesh.externalBytes)return Fail("render","External mesh path must be a safe Game-relative Mesh00p");
   for(unsigned int k=0;k<meshCount;k++)if(SameText(meshes[k].name,mesh.name))return Fail("render","Duplicate external mesh Game path (Windows identity)");
   unsigned int length=lstrlenA(mesh.name);char* name=(char*)memory.Allocate(length+1);const unsigned char* data=Copy(mesh.external,mesh.externalBytes);if(!name||!data)return Fail("render","allocation failed");lstrcpyA(name,mesh.name);
   meshes[meshCount].name=name;meshes[meshCount].data=data;meshes[meshCount].size=mesh.externalBytes;meshCount++;}
  return true;
 }
 // compile-source-world.py validate_streaming_meshes: every external render
 // artifact must also be a Streaming asset (pool folder\leaf names).
 bool StreamingHasMeshes(const unsigned char* data,unsigned int bytes){
  if(bytes<44)return Fail("streaming-and-resources","Invalid generated Streaming mesh table limits");const unsigned int* header=(const unsigned int*)data;unsigned int poolBytes=header[0],regions=header[3];if(poolBytes>bytes-44||regions>10000)return Fail("streaming-and-resources","Invalid generated Streaming mesh table limits");
  const char* pool=(const char*)data+44;unsigned int at=44+poolBytes,total=0;unsigned char* found=(unsigned char*)memory.Allocate(meshCount+1);if(!found)return Fail("streaming-and-resources","allocation failed");memset(found,0,meshCount+1);
  for(unsigned int list=0;list<=regions;list++){
   if(list){if(bytes-at<12)return Fail("streaming-and-resources","Truncated Streaming region");unsigned int members=*(const unsigned int*)(data+at+8);at+=12;if(members>1000000||members>(bytes-at)/4)return Fail("streaming-and-resources","Streaming region membership limit");at+=members*4;}
   if(bytes-at<4)return Fail("streaming-and-resources","Truncated Streaming assets");unsigned int count=*(const unsigned int*)(data+at);at+=4;if(count>100000-total||count>(bytes-at)/8)return Fail("streaming-and-resources","Streaming asset count limit");total+=count;
   for(unsigned int i=0;i<count;i++){unsigned int folder=*(const unsigned int*)(data+at),leaf=*(const unsigned int*)(data+at+4);at+=8;if(folder>=poolBytes||leaf>=poolBytes)return Fail("streaming-and-resources","Streaming asset string offset outside pool");
    char name[600];unsigned int n=0;for(const char* s=pool+folder;*s&&n<280&&s<pool+poolBytes;s++)name[n++]=*s;if(n)name[n++]='\\';for(const char* s=pool+leaf;*s&&n<590&&s<pool+poolBytes;s++)name[n++]=*s;name[n]=0;
    for(unsigned int k=0;k<meshCount;k++)if(!found[k]&&SameText(meshes[k].name,name))found[k]=1;}}
  if(total!=header[4])return Fail("streaming-and-resources","Streaming aggregate asset count mismatch");
  for(unsigned int k=0;k<meshCount;k++)if(!found[k])return Fail("streaming-and-resources","External render mesh is absent from source Streaming assets");return true;
 }
 // nav-sdk-native/process-navmesh.py _compile_source + _process_raw/_process_general:
 // raw (recovered original construction) -> NDB1 -> SDK -> single-root identity
 // restore. The independent geometry proofs stay offline verification.
 bool Navigation(const Request&,const NativePreparedRenderScene::Context& scene,NativeProcessedNavMesh::Input& input){
  NativeRawNavMesh::Builder raw;if(!raw.Build(scene.Model(),scene.Offset()))return Fail("navigation",raw.error);
  const unsigned char* rawData=Copy(raw.data,raw.size);unsigned int rawBytes=raw.size;if(!rawData)return Fail("navigation","allocation failed");
  NavDatabaseManifest::Builder database;if(!database.Build(scene.Project().file))return Fail("navigation",database.error);
  const WorldText::Vec3& offset=scene.Offset();unsigned char* sdk=0;unsigned int sdkBytes=0,roots=0,masks[4096];int general=0;char why[512];why[0]=0;
  NativeNavSetMessageSink(NavMessage,0);int navFailed=NativeNavProcessBuffer(rawData,rawBytes,database.data,database.size,offset.x,offset.y,offset.z,&sdk,&sdkBytes,&roots,masks,4096,&general,why,sizeof(why));NativeNavSetMessageSink(0,0);
  if(navFailed)return Fail("navigation",why);
  unsigned char* processed=(unsigned char*)memory.Allocate(sdkBytes?sdkBytes:1);if(!processed){NativeNavFree(sdk);return Fail("navigation","allocation failed");}memcpy(processed,sdk,sdkBytes);NativeNavFree(sdk);
  if(!roots)return Fail("navigation","Missing SDK root restriction-mask evidence");
  // Walk the processed v6 arrays to the poly rows (72 bytes, parent id at +16)
  // and the region rows (28 bytes, poly count at +4).
  unsigned int at=12;const unsigned int strides[4]={72,4,4,12};
  if(sdkBytes<12||*(unsigned int*)processed!=1)return Fail("navigation","Processed NavMesh marker mismatch");
  for(unsigned int i=0;i<4;i++){if(sdkBytes-at<4)return Fail("navigation","Truncated processed NavMesh");unsigned int count=*(unsigned int*)(processed+at);at+=4;if(count>(sdkBytes-at)/strides[i])return Fail("navigation","Truncated processed NavMesh");at+=count*strides[i];}
  if(sdkBytes-at<4)return Fail("navigation","Truncated processed NavMesh");unsigned int polys=*(unsigned int*)(processed+at);at+=4;if(!polys||polys>(sdkBytes-at)/72)return Fail("navigation","Processed NavMesh polygon count outside its bound");unsigned int polyStart=at;at+=polys*72;
  if(roots==1){for(unsigned int p=0;p<polys;p++){unsigned int& parent=*(unsigned int*)(processed+polyStart+72*p+16);if(parent!=0)return Fail("navigation","Single-root SDK output lost its compact identity");parent=1;}}
  else for(unsigned int p=0;p<polys;p++){unsigned int parent=*(unsigned int*)(processed+polyStart+72*p+16);if(!parent||parent>roots)return Fail("navigation","Authored NavMesh identity restoration failed");}
  if(sdkBytes-at<4)return Fail("navigation","Truncated processed NavMesh");unsigned int refs=*(unsigned int*)(processed+at);at+=4;if(refs>(sdkBytes-at)/4)return Fail("navigation","Truncated processed NavMesh");at+=refs*4;
  if(sdkBytes-at<4)return Fail("navigation","Truncated processed NavMesh");unsigned int regions=*(unsigned int*)(processed+at);at+=4;if(regions>(sdkBytes-at)/28)return Fail("navigation","Truncated processed NavMesh");
  // PackRunTimeNavMesh leaves LTSphere uninitialized for a region without polygons.
  if(general)for(unsigned int r=0;r<regions;r++)if(!*(unsigned int*)(processed+at+28*r+4))return Fail("navigation","General SDK region contains no polygon; the original writes an uninitialized region sphere");
  input.sourceRaw=rawData;input.sourceRawBytes=rawBytes;input.processed=processed;input.bytes=sdkBytes;return true;
 }
 // Objects/BlindData failure with the offending object (name and class) appended.
 bool ObjectsFail(const NativePreparedRenderScene::Context& scene,const NativeObjectsBlindData::Builder& objects){
  char text[768];lstrcpynA(text,objects.error?objects.error:"Objects/BlindData compilation failed",512);
  if(objects.errorNode!=0xffffffffu)for(const PrefabScene::Node* n=scene.Model().first;n;n=n->next)if(n->index==objects.errorNode){lstrcatA(text," (object ");lstrcpynA(text+lstrlenA(text),n->name?n->name:"",100);lstrcatA(text,", class ");lstrcpynA(text+lstrlenA(text),n->className?n->className:"",64);lstrcatA(text,")");break;}
  return Fail("objects-and-blind-data",text);
 }
 bool Physics(const Request& request,const NativePreparedRenderScene::Context& scene,const unsigned char*& out,unsigned int& outBytes){
  return PhysicsWithMopp(request,scene,out,outBytes);
 }
 // Native MOPP (src/NativeMoppCompiler.h, the recovered original builder); the blob
 // is copied into the compiler arena.
 bool Mopp(const unsigned char* mesh,unsigned int bytes,const unsigned char*& out,unsigned int& outBytes){
  unsigned char* blob=0;unsigned int size=0;const char* error=NativeMoppPrefix::Generate(mesh,bytes,blob,size);if(error)return Fail("physics",error);
  out=Copy(blob,size);outBytes=size;delete[] blob;return out!=0||Fail("physics","allocation failed");
 }
 bool PhysicsWithMopp(const Request& request,const NativePreparedRenderScene::Context& scene,const unsigned char*& out,unsigned int& outBytes){
  (void)request;
  NativePhysicsMesh::Builder mesh;if(!mesh.Build(scene))return Fail("physics",mesh.error);moppVertices=mesh.points;moppTriangles=mesh.indices/3;
  NativePhysicsOwnerMeshTransport::Catalog catalog;if(!catalog.Build(scene))return Fail("physics",catalog.error);
  unsigned char* catalogData=(unsigned char*)memory.Allocate(catalog.size?catalog.size:1);if(!catalogData||!catalog.WriteCatalog(catalogData,catalog.size))return Fail("physics","Owner Mesh catalog owned copy failed");
  const unsigned char* code=0;unsigned int codeBytes=0;if(!Mopp(mesh.data,mesh.size,code,codeBytes))return false;
  // compile-source-physics.py: catalog (magic, version, count, then group,
  // model index, mesh prefix) -> association packet with each MOPP blob.
  if(catalog.size<12)return Fail("physics","Owner Mesh catalog size is outside supported limits");
  unsigned int magic=*(unsigned int*)catalogData,version=*(unsigned int*)(catalogData+4),count=*(unsigned int*)(catalogData+8);
  if(magic!=NativePhysicsOwnerMeshTransport::CatalogMagic||version!=1||count>NativePhysicsOwnerMeshTransport::MaximumRecords)return Fail("physics","Owner Mesh catalog magic/version/count mismatch");
  unsigned int total=12,cursor=12;const unsigned char** codes=(const unsigned char**)memory.Allocate((count+1)*sizeof(void*));unsigned int* codeSizes=(unsigned int*)memory.Allocate((count+1)*4);if(!codes||!codeSizes)return Fail("physics","allocation failed");
  for(unsigned int i=0;i<count;i++){if(catalog.size-cursor<12)return Fail("physics","Owner Mesh catalog record header is truncated");unsigned int group=*(unsigned int*)(catalogData+cursor),index=*(unsigned int*)(catalogData+cursor+4),prefix=*(unsigned int*)(catalogData+cursor+8);cursor+=12;
   if(!group||!index||!prefix||prefix>catalog.size-cursor)return Fail("physics","Owner Mesh catalog has invalid group/model identity or prefix");
   if(!Mopp(catalogData+cursor,prefix,codes[i],codeSizes[i]))return false;cursor+=prefix;total+=16+prefix+codeSizes[i];}
  if(cursor!=catalog.size)return Fail("physics","Owner Mesh catalog has trailing bytes");
  unsigned char* packet=(unsigned char*)memory.Allocate(total);if(!packet)return Fail("physics","allocation failed");unsigned char* at=packet;unsigned int header[3]={NativePhysicsOwnerMeshTransport::AssociationMagic,1,count};detail::Put(at,header,12);cursor=12;
  for(unsigned int i=0;i<count;i++){unsigned int group=*(unsigned int*)(catalogData+cursor),index=*(unsigned int*)(catalogData+cursor+4),prefix=*(unsigned int*)(catalogData+cursor+8);cursor+=12;unsigned int record[4]={group,index,prefix,codeSizes[i]};detail::Put(at,record,16);detail::Put(at,catalogData+cursor,prefix);detail::Put(at,codes[i],codeSizes[i]);cursor+=prefix;}
  NativePhysicsOwnerMeshTransport::Parser owners;if(!owners.Build(packet,total))return Fail("physics",owners.error?owners.error:"Cannot read owner Mesh MOPP associations");
  NativePhysicsCompiler::MoppInput input;input.mesh=mesh.data;input.meshBytes=mesh.size;input.code=code;input.codeBytes=codeBytes;
  NativePhysicsCompiler::Settings settings;settings.owners.meshAssociations=owners.records;settings.owners.meshAssociationCount=owners.count;
  NativePhysicsCompiler::Builder compiler;if(!compiler.Build(scene,input,settings))return Fail("physics",compiler.error);
  if(compiler.size<4||!*(unsigned int*)compiler.data||*(unsigned int*)compiler.data>4096)return Fail("physics","Native Physics section has invalid shape-count framing");
  out=Copy(compiler.data,compiler.size);outBytes=compiler.size;return out!=0;
 }
 // compile-source-world.py assemble_world.
 bool Assemble(const float vectors[9],const unsigned char* const parts[10],const unsigned int bytes[10],unsigned int modelCount){
  for(unsigned int i=0;i<9;i++)if(!WorldText::Finite(vectors[i]))return Fail("assembly","Non-finite world bounds/offset");for(unsigned int i=0;i<3;i++)if(vectors[i]>vectors[i+3])return Fail("assembly","Inverted world bounds");
  unsigned int total=56;for(unsigned int i=0;i<10;i++){if(!parts[i]||!bytes[i]||bytes[i]>64u*1024u*1024u||bytes[i]>256u*1024u*1024u-total)return Fail("assembly","Invalid owned world section");total+=bytes[i];}
  for(unsigned int i=0;i<24;i++)if(parts[0][i]!=((const unsigned char*)vectors)[i])return Fail("assembly","Header and source tree bounds differ");
  unsigned int physicsCount=*(const unsigned int*)parts[3];if(!physicsCount||physicsCount>4096||physicsCount!=modelCount)return Fail("assembly","Physics and source WorldModels membership differ");
  if(bytes[4]<48||bytes[6]<40||bytes[8]<4||bytes[9]<8)return Fail("assembly","Incomplete generated world section framing");
  unsigned int blindCount=*(const unsigned int*)parts[9],payload=*(const unsigned int*)(parts[9]+4);if(blindCount>100000||payload>bytes[9]-8||bytes[9]!=8+payload+12*blindCount)return Fail("assembly","BlindData framing/count mismatch");
  unsigned int starts[10],cursor=56;for(unsigned int i=0;i<10;i++){starts[i]=cursor;cursor+=bytes[i];}
  unsigned char* data=(unsigned char*)memory.Allocate(total);if(!data)return Fail("assembly","World allocation failed");unsigned char* at=data;
  unsigned int head[5]={113,starts[6],starts[7],starts[8],starts[9]};detail::Put(at,head,20);detail::Put(at,vectors,36);for(unsigned int i=0;i<10;i++)detail::Put(at,parts[i],bytes[i]);
  if((unsigned int)(at-data)!=total)return Fail("assembly","Whole-world final cursor mismatch");world=data;size=total;return true;
 }
};
}
