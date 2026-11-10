#define FEAR_PRIMARY_SURFACE_PREPARATION 1
#include "WorldPacker.cpp"
#include "NativePhysicsMesh.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
namespace PhysicsMeshGuards {
static bool Equal(const void* a,const void* b,unsigned int bytes){const unsigned char* x=(const unsigned char*)a;const unsigned char* y=(const unsigned char*)b;for(unsigned int i=0;i<bytes;i++)if(x[i]!=y[i])return false;return true;}
static unsigned int Word(const unsigned char* data){unsigned int value;memcpy(&value,data,4);return value;}
static float Bits(unsigned int bits){float value;memcpy(&value,&bits,4);return value;}
static bool Empty(const NativePhysicsMesh::Builder& result){return !result.data&&!result.size&&!result.points&&!result.indices&&!result.sourcePoints&&!result.sourcePolygons&&!result.Prefix().data&&!result.Prefix().size;}
static bool Rejected(NativePhysicsMesh::Builder& result,const NativePreparedRenderScene::Context& scene,const NativePhysicsMesh::Settings& settings=NativePhysicsMesh::Settings()){return !result.Build(scene,settings)&&Empty(result)&&result.error&&result.error[0];}
static bool Write(HANDLE file,const void* data,unsigned int size){DWORD done=0;return !size||(WriteFile(file,data,size,&done,0)&&done==size);}
static bool Insert(SourcePhysicsVertexIndex::Indexer& indexer,const WorldText::Vec3& point,unsigned int expected){unsigned int index=0xffffffffu;return indexer.Insert(point,index)&&index==expected;}
static bool Boundary(unsigned int axis){SourcePhysicsVertexIndex::Indexer indexer;WorldText::Vec3 first={},upper={},lower={},outside={};(&upper.x)[axis]=.5f;(&lower.x)[axis]=-.5f;(&outside.x)[axis]=Bits(0x3f000001u);return indexer.Begin(4,.5f)&&Insert(indexer,first,0)&&Insert(indexer,upper,0)&&Insert(indexer,lower,0)&&indexer.count==1&&Insert(indexer,outside,1)&&indexer.count==2;}
static bool Capacity(){SourcePhysicsVertexIndex::Indexer indexer;if(!indexer.Begin(65535,0))return false;for(unsigned int i=0;i<65535;i++){WorldText::Vec3 point={(float)i,0,0};if(!Insert(indexer,point,i))return false;}if(indexer.count!=65535)return false;for(unsigned int i=65535;i;i--){WorldText::Vec3 point={(float)(i-1),0,0};if(!Insert(indexer,point,i-1)||indexer.Point(i-1).x!=point.x)return false;}WorldText::Vec3 exhausted={65535,0,0};unsigned int index=0x12345678u;if(indexer.Insert(exhausted,index)||index!=0x12345678u||indexer.count!=65535)return false;WorldText::Vec3 duplicate={32768,0,0};return Insert(indexer,duplicate,32768)&&indexer.count==65535;}
static bool Descending(){SourcePhysicsVertexIndex::Indexer indexer;if(!indexer.Begin(1024,0))return false;for(unsigned int i=0;i<1024;i++){WorldText::Vec3 point={(float)(1023-i),0,0};if(!Insert(indexer,point,i))return false;}for(unsigned int i=0;i<1024;i++){WorldText::Vec3 point={(float)i,0,0};if(!Insert(indexer,point,1023-i)||indexer.Point(1023-i).x!=point.x)return false;}return indexer.count==1024;}
// Validate the first contour from source coordinates and disk fan framing.
// This does not use the production indexer or mesh writer for expected indices.
static bool FirstFan(const NativePhysicsMesh::Builder& result,const NativePreparedRenderScene::Context& scene){const WorldModels::Partition& partition=scene.Partition();const WorldText::Geometry* geometry=0;for(unsigned int i=0;i<partition.assignmentCount;i++)if(!partition.assignments[i].group&&(partition.assignments[i].brush->surfaceFlags&0x10)){geometry=&partition.assignments[i].brush->geometry;break;}if(!geometry||!geometry->polygons)return false;const WorldText::Polygon& face=geometry->faces[0];if(face.count<3||face.count>result.points)return false;const WorldText::Vec3& offset=scene.Offset();for(unsigned int i=0;i<face.count;i++){WorldText::Vec3 expected=geometry->vertices[face.indices[i]];expected.x=(expected.x-offset.x)*.01f;expected.y=(expected.y-offset.y)*.01f;expected.z=(expected.z-offset.z)*.01f;if(!Equal(result.data+108+i*12,&expected,12))return false;}const unsigned char* indices=result.data+108+result.points*12;for(unsigned int i=1;i+1<face.count;i++){unsigned short expected[3]={0,(unsigned short)i,(unsigned short)(i+1)};if(!Equal(indices+(i-1)*6,expected,6))return false;}return true;}
}
extern "C" void HarnessEntry(){
 using namespace PhysicsMeshGuards;
 unsigned int checks[96]={},count=0;SourcePhysicsVertexIndex::Indexer indexer;WorldText::Vec3 zero={};unsigned int index=0x12345678u;
#define CHECK(label,expression) checks[count++]=(expression)?1u:0u
 CHECK("fresh indexer rejects insert before Begin",!indexer.Insert(zero,index)&&index==0x12345678u&&!indexer.count);
 CHECK("indexer zero capacity rejected",!indexer.Begin(0,0)&&!indexer.count);
 CHECK("indexer capacity above U16 rejected",!indexer.Begin(65536,0)&&!indexer.count);
 CHECK("indexer negative tolerance rejected",!indexer.Begin(4,-.01f)&&!indexer.count);
 CHECK("indexer NaN tolerance rejected",!indexer.Begin(4,Bits(0x7fc00000u))&&!indexer.count);
 CHECK("indexer infinite tolerance rejected",!indexer.Begin(4,Bits(0x7f800000u))&&!indexer.count);
 CHECK("indexer exact capacity one accepted",indexer.Begin(1,0)&&Insert(indexer,zero,0));
 WorldText::Vec3 different={1,0,0};index=0x12345678u;CHECK("full indexer rejects new vertex without changing count or result id",!indexer.Insert(different,index)&&index==0x12345678u&&indexer.count==1);
 CHECK("full indexer still finds duplicate",Insert(indexer,zero,0)&&indexer.count==1);
 CHECK("invalid Begin clears previous indexed input",!indexer.Begin(0,0)&&!indexer.count&&!indexer.Insert(zero,index));
 CHECK("indexer valid reuse restarts first appearance ids",indexer.Begin(4,0)&&Insert(indexer,different,0)&&Insert(indexer,zero,1)&&Insert(indexer,different,0)&&indexer.count==2&&indexer.Point(0).x==1&&indexer.Point(1).x==0);
 CHECK("inclusive X tolerance and first float outside boundary",Boundary(0));
 CHECK("inclusive Y tolerance and first float outside boundary",Boundary(1));
 CHECK("inclusive Z tolerance and first float outside boundary",Boundary(2));
 WorldText::Vec3 diagonal={.5f,.5f,.5f};CHECK("tolerance is per axis rather than Euclidean distance",indexer.Begin(4,.5f)&&Insert(indexer,zero,0)&&Insert(indexer,diagonal,0)&&indexer.count==1);
 WorldText::Vec3 b={.75f,.75f,.75f},c={.4f,.4f,.4f};CHECK("overlapping tolerant equivalence uses tree candidate rather than earliest linear match",indexer.Begin(4,.5f)&&Insert(indexer,zero,0)&&Insert(indexer,b,1)&&Insert(indexer,c,1)&&indexer.count==2&&Equal(&indexer.Point(1),&b,12));
 WorldText::Vec3 negativeZero={Bits(0x80000000u),Bits(0x80000000u),Bits(0x80000000u)};CHECK("zero tolerance preserves first signed zero vertex bits",indexer.Begin(4,0)&&Insert(indexer,negativeZero,0)&&Insert(indexer,zero,0)&&indexer.count==1&&Equal(&indexer.Point(0),&negativeZero,12));
 bool finiteGuard=indexer.Begin(4,0)&&Insert(indexer,zero,0);const unsigned int invalidBits[3]={0x7fc00000u,0x7f800000u,0xff800000u};for(unsigned int axis=0;axis<3;axis++)for(unsigned int kind=0;kind<3;kind++){WorldText::Vec3 invalid={};(&invalid.x)[axis]=Bits(invalidBits[kind]);index=0x12345678u;if(indexer.Insert(invalid,index)||index!=0x12345678u||indexer.count!=1||!Equal(&indexer.Point(0),&zero,12))finiteGuard=false;}
 CHECK("all coordinate axes reject NaN and both infinities atomically",finiteGuard);
 CHECK("ascending U16 maximum capacity rotations lookup and exhaustion",Capacity());
 CHECK("descending rotation stress preserves first insertion ids",Descending());
 char source[MAX_PATH],project[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_PHYSICS_MESH_GUARDS_SOURCE",source,MAX_PATH)||!GetEnvironmentVariableA("FEAR_PHYSICS_MESH_GUARDS_PROJECT",project,MAX_PATH)||!GetEnvironmentVariableA("FEAR_PHYSICS_MESH_GUARDS_OUTPUT",output,MAX_PATH))ExitProcess(1);
 PrefabScene::Arena memory;void* place=memory.Allocate(sizeof(NativePreparedRenderScene::Context));if(!place)ExitProcess(2);NativePreparedRenderScene::Context* scene=new(place)NativePreparedRenderScene::Context;NativePhysicsMesh::Builder result;
 CHECK("fresh mesh builder empty",Empty(result)&&!result.error);
 CHECK("mesh unprepared scene rejected without output",Rejected(result,*scene));
 if(!scene->Build(source,project)){LogLine(scene->Diagnostic().message);ExitProcess(3);}if(!result.Build(*scene)){LogLine(result.error);ExitProcess(4);}
 CHECK("BetterBox mesh has independently known counts and framing",result.points==76&&result.indices==324&&result.size==1668&&result.Prefix().data==result.data&&result.Prefix().size==result.size&&Word(result.data)==1&&Word(result.data+32)==1&&Word(result.data+100)==76&&Word(result.data+104)==324);
 CHECK("first source contour owns vertex order and fan triangle order",FirstFan(result,*scene));
 unsigned int sourcePoints=0,sourcePolygons=0,references=0;for(unsigned int a=0;a<scene->Partition().assignmentCount;a++){const WorldModels::Assignment& item=scene->Partition().assignments[a];if(!item.group&&(item.brush->surfaceFlags&0x10)){sourcePoints+=item.brush->geometry.points;sourcePolygons+=item.brush->geometry.polygons;references+=item.brush->geometry.references;}}
 CHECK("source budgets count candidate MAIN geometry before welding",result.sourcePoints==sourcePoints&&result.sourcePolygons==sourcePolygons&&sourcePoints>=result.points&&references>=result.indices/3);
 const unsigned int referenceSize=result.size;unsigned char* reference=(unsigned char*)memory.Allocate(referenceSize);if(!reference)ExitProcess(2);memcpy(reference,result.data,referenceSize);
 NativePhysicsMesh::Settings settings;settings.version=0;CHECK("zero mesh version clears previous output",Rejected(result,*scene,settings));settings.version=2;CHECK("unsupported mesh version rejected",Rejected(result,*scene,settings));
 settings=NativePhysicsMesh::Settings();settings.weldTolerance=-.01f;CHECK("negative weld tolerance rejected",Rejected(result,*scene,settings));settings.weldTolerance=Bits(0x7fc00000u);CHECK("NaN weld tolerance rejected",Rejected(result,*scene,settings));settings.weldTolerance=Bits(0x7f800000u);CHECK("infinite weld tolerance rejected",Rejected(result,*scene,settings));
 settings=NativePhysicsMesh::Settings();settings.maximumSourcePoints=0;CHECK("zero source point budget rejected",Rejected(result,*scene,settings));settings.maximumSourcePoints=1000001;CHECK("source point budget above bound rejected",Rejected(result,*scene,settings));
 settings=NativePhysicsMesh::Settings();settings.maximumReferences=0;CHECK("zero reference budget rejected",Rejected(result,*scene,settings));settings.maximumReferences=3000001;CHECK("reference budget above bound rejected",Rejected(result,*scene,settings));
 settings=NativePhysicsMesh::Settings();settings.maximumSourcePoints=sourcePoints-1;CHECK("source budget exhausted before weld allocation",Rejected(result,*scene,settings));settings.maximumSourcePoints=sourcePoints;settings.maximumReferences=references-1;CHECK("reference budget exhausted before prefix output",Rejected(result,*scene,settings));settings.maximumReferences=references;
 CHECK("exact source and reference budgets accepted without output change",result.Build(*scene,settings)&&result.size==referenceSize&&Equal(result.data,reference,referenceSize));
 settings=NativePhysicsMesh::Settings();settings.weldTolerance=100000000.f;CHECK("collapsed welded contours rejected without partial prefix",Rejected(result,*scene,settings));
 CHECK("reuse after settings and contour failures exact",result.Build(*scene)&&result.size==referenceSize&&Equal(result.data,reference,referenceSize)&&!result.error);
 const WorldModels::Partition& partition=scene->Partition();PrefabScene::Node* brush=0;for(unsigned int a=0;a<partition.assignmentCount;a++)if(!partition.assignments[a].group&&(partition.assignments[a].brush->surfaceFlags&0x10)){brush=const_cast<PrefabScene::Node*>(partition.assignments[a].brush);break;}if(!brush)ExitProcess(5);
 bool knownType=brush->knownType;brush->knownType=false;CHECK("unknown candidate collision brush rejected",Rejected(result,*scene));brush->knownType=knownType;
 WorldText::Vec3* vertices=brush->geometry.vertices;brush->geometry.vertices=0;CHECK("missing collision vertex storage rejected",Rejected(result,*scene));brush->geometry.vertices=vertices;
 WorldText::Polygon* faces=brush->geometry.faces;brush->geometry.faces=0;CHECK("missing collision polygon storage rejected",Rejected(result,*scene));brush->geometry.faces=faces;
 unsigned int points=brush->geometry.points;brush->geometry.points=0;CHECK("zero collision source points rejected",Rejected(result,*scene));brush->geometry.points=points;
 unsigned int polygons=brush->geometry.polygons;brush->geometry.polygons=0;CHECK("zero collision source polygons rejected",Rejected(result,*scene));brush->geometry.polygons=polygons;
 unsigned int* corners=faces[0].indices;faces[0].indices=0;CHECK("missing collision contour references rejected",Rejected(result,*scene));faces[0].indices=corners;
 unsigned int cornersCount=faces[0].count;faces[0].count=2;CHECK("collision contour below three vertices rejected",Rejected(result,*scene));faces[0].count=cornersCount;
 bool validPlane=faces[0].validPlane;faces[0].validPlane=false;CHECK("degenerate collision contour rejected",Rejected(result,*scene));faces[0].validPlane=validPlane;
 float vertex=vertices[corners[0]].x;vertices[corners[0]].x=Bits(0x7fc00000u);CHECK("nonfinite source collision corner rejected",Rejected(result,*scene));vertices[corners[0]].x=vertex;
 unsigned int* flags=(unsigned int*)memory.Allocate((scene->Model().count+1)*4);if(!flags)ExitProcess(2);for(unsigned int a=0;a<partition.assignmentCount;a++){PrefabScene::Node* assigned=const_cast<PrefabScene::Node*>(partition.assignments[a].brush);flags[assigned->index]=assigned->surfaceFlags;if(!partition.assignments[a].group)assigned->surfaceFlags&=~0x10u;}
 CHECK("scene without MAIN collision geometry rejects empty prefix",Rejected(result,*scene));for(unsigned int a=0;a<partition.assignmentCount;a++){PrefabScene::Node* assigned=const_cast<PrefabScene::Node*>(partition.assignments[a].brush);assigned->surfaceFlags=flags[assigned->index];}
 CHECK("reuse after source geometry guards restores exact prefix",result.Build(*scene)&&result.size==referenceSize&&result.points==76&&result.indices==324&&Equal(result.data,reference,referenceSize)&&!result.error);
 const unsigned char* retained=result.data;scene->Clear();CHECK("owned mesh prefix survives prepared scene Clear",result.data==retained&&result.size==referenceSize&&Equal(result.data,reference,referenceSize)&&result.Prefix().data==retained);
 scene->~Context();CHECK("owned mesh prefix survives prepared scene destruction",result.data==retained&&result.size==referenceSize&&Equal(result.data,reference,referenceSize));
 result.Clear();CHECK("mesh Clear resets output and diagnostic",Empty(result)&&!result.error);result.Clear();CHECK("repeated mesh Clear remains empty",Empty(result)&&!result.error);
#undef CHECK
 HANDLE file=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(file==INVALID_HANDLE_VALUE)ExitProcess(8);unsigned int header[5]={0x31474d50u,count,referenceSize,sourcePoints,sourcePolygons};bool passed=Write(file,header,20)&&Write(file,checks,count*4)&&Write(file,reference,referenceSize);CloseHandle(file);for(unsigned int check=0;check<count;check++)if(!checks[check])passed=false;ExitProcess(passed?0:6);
}
