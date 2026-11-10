#include "WorldPacker.cpp"
#include "RenderTriangleRouting.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")

// Source-owned fan records supply numeric sector IDs. The only routed payload
// is the source triangle ordinal; no original runtime pointers are consumed.
struct FanRecord {unsigned char corners[132];unsigned int owner,edgeFlags;};
static_assert(sizeof(FanRecord)==140,"source fan record size");
typedef RenderTriangleRouting::Builder<unsigned int> Routing;
static bool Read(HANDLE file,void* data,unsigned int bytes){DWORD done;return ReadFile(file,data,bytes,&done,0)&&done==bytes;}
static bool Put(HANDLE file,const void* data,unsigned int bytes){DWORD done;return WriteFile(file,data,bytes,&done,0)&&done==bytes;}
static bool Path(const char* name,char* path){DWORD size=GetEnvironmentVariableA(name,path,MAX_PATH);return size&&size<MAX_PATH;}

extern "C" void HarnessEntry(){
 char sourcePath[MAX_PATH],fanPath[MAX_PATH],outputPath[MAX_PATH];
 if(!Path("FEAR_READER_INPUT",sourcePath)||!Path("FEAR_SOURCE_NODE_FAN_INPUT",fanPath)||!Path("FEAR_SOURCE_NODE_ROUTES_OUTPUT",outputPath))ExitProcess(1);
 PrefabScene::Arena memory;
 void* expansionStorage=memory.Allocate(sizeof(PrefabScene::Expansion));
 void* visibilityStorage=memory.Allocate(sizeof(VisibilityCompiler::Builder));
 if(!expansionStorage||!visibilityStorage)ExitProcess(2);
 PrefabScene::Expansion* expansion=new(expansionStorage) PrefabScene::Expansion(true);
 // Fan owners are numeric SaveIDs of the fully expanded visibility scene.
 // Prefab geometry has already been expanded by the source contour stage;
 // routing must build the same scaffold, including prefab-authored sectors.
 {PrefabScene::Source source;if(!source.Load(sourcePath,true)||!expansion->Build(sourcePath,source.scene))ExitProcess(3);}
 // Compiler preparation removes recognized editor brushes before routing.
 // Validate the resulting model, including prefab-authored unknown types.
 for(const PrefabScene::Node* n=expansion->Scene().first;n;n=n->next)if(n->kind==1&&!n->knownType)ExitProcess(3);
 VisibilityCompiler::Builder* visibility=new(visibilityStorage) VisibilityCompiler::Builder;
 if(!visibility->Build(expansion->Scene()))ExitProcess(4);
 HANDLE input=CreateFileA(fanPath,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);
 if(input==INVALID_HANDLE_VALUE)ExitProcess(5);
 unsigned int fanHeader[4];
 if(!Read(input,fanHeader,16)||fanHeader[0]!=0x314e4652||fanHeader[1]>1000000||fanHeader[2]<fanHeader[1]||fanHeader[3]>fanHeader[2]-fanHeader[1])ExitProcess(6);
 unsigned int sourceCount=fanHeader[1],count=sourceCount;
 unsigned int* owners=(unsigned int*)memory.Allocate((sourceCount?sourceCount:1)*4);
 if(!owners)ExitProcess(2);
 FanRecord fan={};
 for(unsigned int i=0;i<sourceCount;i++){
  if(!Read(input,&fan,sizeof(fan)))ExitProcess(6);
  owners[i]=fan.owner;
 }
 unsigned char trailing;DWORD extra;
 if(!ReadFile(input,&trailing,1,&extra,0)||extra)ExitProcess(6);
 CloseHandle(input);
 // Optional source TJunction output retains the original fan ordinal through
 // subdivision. Ownership follows that ordinal; no coordinates or runtime
 // pointers are used to guess a sector, and new triangle order is preserved.
 char junctionPath[MAX_PATH];unsigned int junctionLength=GetEnvironmentVariableA("FEAR_SOURCE_NODE_TJUNCTION_INPUT",junctionPath,MAX_PATH);
 if(junctionLength>=MAX_PATH)ExitProcess(6);
 HANDLE junction=INVALID_HANDLE_VALUE;
 if(junctionLength){
  junction=CreateFileA(junctionPath,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);
  if(junction==INVALID_HANDLE_VALUE||!Read(junction,&count,4)||count>1000000)ExitProcess(6);
 }
 unsigned int* ordinals=(unsigned int*)memory.Allocate((count?count:1)*4);
 Routing::Input* records=(Routing::Input*)memory.Allocate((count?count:1)*sizeof(Routing::Input));
 if(!ordinals||!records)ExitProcess(2);
 for(unsigned int i=0;i<count;i++){
  unsigned int source=i;
  if(junctionLength){unsigned int record[65];if(!Read(junction,record,sizeof(record))||record[0]>=sourceCount||record[1]>7)ExitProcess(6);source=record[0];}
  ordinals[i]=i;records[i].triangle=&ordinals[i];records[i].owner=owners[source];
 }
 if(junctionLength){if(!ReadFile(junction,&trailing,1,&extra,0)||extra)ExitProcess(6);CloseHandle(junction);}
 Routing routing;if(!routing.Build(records,count,visibility->sectorCount))ExitProcess(7);
 HANDLE output=CreateFileA(outputPath,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);
 if(output==INVALID_HANDLE_VALUE)ExitProcess(8);
 unsigned int header[6]={0x31475253,1,routing.nodeCount,count,routing.assigned,routing.skipped};
 if(!Put(output,header,24)||!Put(output,&visibility->offset,12))ExitProcess(9);
 for(unsigned int i=0;i<visibility->sectorCount;i++){
  unsigned int ids[2]={i,visibility->streamingSectors[i].index};
  if(!Put(output,ids,8))ExitProcess(9);
 }
 for(unsigned int node=0;node<routing.nodeCount;node++){
  const Routing::Node& out=routing.nodes[node];
  if(!Put(output,&out.count,4)||(out.count&&!Put(output,out.triangles,out.count*4)))ExitProcess(9);
 }
 CloseHandle(output);visibility->~Builder();expansion->~Expansion();ExitProcess(0);
}
