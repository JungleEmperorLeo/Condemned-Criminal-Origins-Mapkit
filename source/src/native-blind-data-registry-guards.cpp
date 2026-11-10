#include "NativeBlindDataRegistry.h"
extern "C" void* __cdecl memcpy(void* target,const void* source,unsigned int bytes){unsigned char* t=(unsigned char*)target;const unsigned char* s=(const unsigned char*)source;while(bytes--)*t++=*s++;return target;}
extern "C" void* __cdecl memset(void* target,int value,unsigned int bytes){unsigned char* t=(unsigned char*)target;while(bytes--)*t++=(unsigned char)value;return target;}
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
#pragma comment(linker,"/alternatename:__imp__VirtualAlloc@16=__imp__VirtualAlloc")
#pragma comment(linker,"/alternatename:__imp__VirtualFree@12=__imp__VirtualFree")
#pragma comment(linker,"/alternatename:__imp__CreateFileA@28=__imp__CreateFileA")
#pragma comment(linker,"/alternatename:__imp__WriteFile@20=__imp__WriteFile")
#pragma comment(linker,"/alternatename:__imp__CloseHandle@4=__imp__CloseHandle")
namespace B=NativeBlindDataRegistry;
static unsigned int checks[128],checkCount=0;
static void Check(bool value){if(checkCount==128)ExitProcess(80);checks[checkCount++]=value?1u:0u;}
#define CHECK(name,expression) Check((expression))
static bool Empty(const B::Registry& r){return !r.Data()&&!r.Size()&&!r.Count()&&!r.Records()&&!r.Bindings()&&!r.BindingCount()&&!r.OwnedBytes()&&r.error;}
static bool Same(const void* a,const void* b,unsigned int n){const unsigned char* x=(const unsigned char*)a,*y=(const unsigned char*)b;for(unsigned int i=0;i<n;i++)if(x[i]!=y[i])return false;return true;}
static bool Failed(B::Registry& r,B::ErrorCode code){return Empty(r)&&r.errorCode==code;}
extern "C" void HarnessEntry(){
 B::Registry registry;unsigned char output[128];for(unsigned int i=0;i<sizeof(output);i++)output[i]=0xcc;
 CHECK("fresh registry has no published output",!registry.Data()&&!registry.Size()&&!registry.error&&!registry.Write(output,sizeof(output)));
 CHECK("empty structural profile is eight zero bytes",registry.Build(0,0)&&registry.Size()==8&&B::detail::Get(registry.Data())==0&&B::detail::Get(registry.Data()+4)==0&&registry.OwnedBytes()==8);
 B::Settings limits;limits.maximumBytes=8;CHECK("empty profile accepts exact eight byte budget",registry.Build(0,0,0,0,limits)&&registry.Size()==8);
 unsigned char payloadA[3]={0x00,0xfe,0x81},payloadB[5]={9,8,7,6,5};char propA[]="KeyDataIndex",propB[]="BlindDataIndex";
 B::RecordInput records[3]={{0x6aaf0884u,payloadA,3},{0xffffffffu,0,0},{0x83f47c31u,payloadB,5}};
 B::BindingInput bindings[2]={{0,propA,0,records[0].typeId},{7,propB,2,records[2].typeId}};
 CHECK("typed records preserve append ordinal and aligned starts",registry.Build(records,3,bindings,2)&&registry.Count()==3&&registry.Size()==56&&registry.Records()[0].start==0&&registry.Records()[1].start==4&&registry.Records()[2].start==4);
 CHECK("ordinal zero and highest nonzero type are structural identities",registry.Bindings()[0].ordinal==0&&registry.Records()[1].typeId==0xffffffffu&&registry.Records()[1].bytes==0);
 CHECK("bindings preserve source identity type and ordinal",registry.FindBinding(0,"KeyDataIndex")&&registry.FindBinding(0,"KeyDataIndex")->typeId==0x6aaf0884u&&registry.FindBinding(7,"BlindDataIndex")->ordinal==2&&!registry.FindBinding(8,"BlindDataIndex"));
 CHECK("serialization preserves unpadded sizes and aligned table offsets",B::detail::Get(registry.Data())==3&&B::detail::Get(registry.Data()+4)==12&&Same(registry.Data()+8,payloadA,3)&&Same(registry.Data()+12,payloadB,5)&&B::detail::Get(registry.Data()+20)==3&&B::detail::Get(registry.Data()+24)==0x6aaf0884u&&B::detail::Get(registry.Data()+28)==0&&B::detail::Get(registry.Data()+32)==0&&B::detail::Get(registry.Data()+36)==0xffffffffu&&B::detail::Get(registry.Data()+40)==4&&B::detail::Get(registry.Data()+44)==5&&B::detail::Get(registry.Data()+48)==0x83f47c31u&&B::detail::Get(registry.Data()+52)==4);
 CHECK("alignment bytes are zero including the final payload",registry.Data()[11]==0&&registry.Data()[17]==0&&registry.Data()[18]==0&&registry.Data()[19]==0);
 unsigned char example[56];registry.Write(example,sizeof(example));payloadA[1]=33;payloadB[0]=44;propA[0]='X';records[0].typeId=22;bindings[0].ordinal=2;
 CHECK("caller payload metadata and property mutation cannot change output",Same(example,registry.Data(),56)&&registry.Records()[0].typeId==0x6aaf0884u&&registry.FindBinding(0,"KeyDataIndex")&&registry.FindBinding(0,"KeyDataIndex")->ordinal==0);
 CHECK("external write owns a stable copy",registry.Write(output,sizeof(output))&&Same(output,example,56));
 CHECK("short write leaves destination unchanged",!registry.Write(output,55)&&Same(output,example,56));
 CHECK("write refuses overlap with packet",!registry.Write((void*)registry.Data(),registry.Size())&&Same(example,registry.Data(),56));
 CHECK("write refuses overlap with record views",!registry.Write((void*)registry.Records(),128)&&Same(example,registry.Data(),56));
 CHECK("write refuses overlap with binding views",!registry.Write((void*)registry.Bindings(),128)&&registry.FindBinding(7,"BlindDataIndex"));
 const void* old=registry.Data();unsigned int oldSize=registry.Size();CHECK("rebuild rejects owned record-array alias before freeing",!registry.Build((const B::RecordInput*)old,1)&&Failed(registry,B::InputAlias)&&oldSize==56);
 records[0].typeId=0x6aaf0884u;propA[0]='K';bindings[0].ordinal=0;
 CHECK("successful reuse follows cleared failure",registry.Build(records,3,bindings,2)&&!registry.error&&registry.failedIndex==0xffffffffu);
 B::RecordInput alias={1,registry.Data()+8,1};CHECK("rebuild rejects owned payload alias and clears prior output",!registry.Build(&alias,1)&&Failed(registry,B::InputAlias));
 registry.Build(records,3,bindings,2);B::BindingInput nameAlias={3,registry.Bindings()[0].property,0,records[0].typeId};CHECK("rebuild rejects owned property alias",!registry.Build(records,3,&nameAlias,1)&&Failed(registry,B::InputAlias));
 registry.Build(records,3,bindings,2);const B::BindingInput* arrayAlias=(const B::BindingInput*)registry.Bindings();CHECK("rebuild rejects owned binding-array alias",!registry.Build(records,3,arrayAlias,1)&&Failed(registry,B::InputAlias));
 registry.Build(records,3,bindings,2);const B::RecordInput* viewAlias=(const B::RecordInput*)registry.Records();CHECK("rebuild rejects owned record-view alias",!registry.Build(viewAlias,1)&&Failed(registry,B::InputAlias));
 CHECK("missing nonempty record array rejected",!registry.Build(0,1)&&Failed(registry,B::InvalidInput));
 CHECK("missing nonempty binding array rejected",!registry.Build(records,3,0,1)&&Failed(registry,B::InvalidInput));
 B::RecordInput badRecord={0,payloadA,3};CHECK("zero type ID rejected",!registry.Build(&badRecord,1)&&Failed(registry,B::InvalidType)&&registry.failedIndex==0);
 badRecord.typeId=1;badRecord.data=0;CHECK("missing nonempty payload rejected",!registry.Build(&badRecord,1)&&Failed(registry,B::InvalidInput));
 badRecord.bytes=0;CHECK("empty typed payload accepts null data",registry.Build(&badRecord,1)&&registry.Size()==20&&registry.Records()[0].bytes==0);
 badRecord.bytes=0xffffffffu;CHECK("overflow sized payload rejected before read",!registry.Build(&badRecord,1)&&Failed(registry,B::ResourceLimit));
 B::BindingInput badBinding={1,0,0,records[0].typeId};CHECK("null binding property rejected",!registry.Build(records,3,&badBinding,1)&&Failed(registry,B::InvalidBinding));
 badBinding.property="";CHECK("empty binding property rejected",!registry.Build(records,3,&badBinding,1)&&Failed(registry,B::InvalidBinding));
 char name[257];for(unsigned int i=0;i<256;i++)name[i]='p';name[256]=0;badBinding.property=name;CHECK("property over 255 bytes rejected",!registry.Build(records,3,&badBinding,1)&&Failed(registry,B::InvalidBinding));
 name[255]=0;CHECK("255 byte property accepted and owned",registry.Build(records,3,&badBinding,1)&&registry.FindBinding(1,name));
 badBinding.property="p";badBinding.sourceNode=0xffffffffu;CHECK("invalid source sentinel rejected",!registry.Build(records,3,&badBinding,1)&&Failed(registry,B::InvalidBinding));
 badBinding.sourceNode=0xfffffffeu;CHECK("source identity has no map-specific upper bound",registry.Build(records,3,&badBinding,1)&&registry.FindBinding(0xfffffffeu,"p"));
 badBinding.sourceNode=1;badBinding.ordinal=3;CHECK("out of range ordinal rejected",!registry.Build(records,3,&badBinding,1)&&Failed(registry,B::InvalidBinding));
 badBinding.ordinal=0xffffffffu;CHECK("overflow ordinal rejected",!registry.Build(records,3,&badBinding,1)&&Failed(registry,B::InvalidBinding));
 badBinding.ordinal=0;badBinding.typeId=0;CHECK("binding zero type rejected",!registry.Build(records,3,&badBinding,1)&&Failed(registry,B::InvalidBinding));
 badBinding.typeId=records[2].typeId;CHECK("binding type must match exact record ordinal",!registry.Build(records,3,&badBinding,1)&&Failed(registry,B::InvalidBinding));
 B::BindingInput pair[2]={{1,"p",0,records[0].typeId},{1,"p",2,records[2].typeId}};
 CHECK("duplicate source property rejected across different records",!registry.Build(records,3,pair,2)&&Failed(registry,B::DuplicateBinding)&&registry.failedIndex==1);
 pair[1].sourceNode=2;CHECK("same property on distinct source nodes accepted",registry.Build(records,3,pair,2)&&registry.BindingCount()==2);
 pair[1].sourceNode=1;pair[1].property="q";CHECK("distinct properties on same source node accepted",registry.Build(records,3,pair,2)&&registry.BindingCount()==2);
 pair[1].property="P";CHECK("property identity uses exact bytes without hidden case normalization",registry.Build(records,3,pair,2)&&registry.FindBinding(1,"p")&&registry.FindBinding(1,"P"));
 CHECK("unknown property lookup has no fallback binding",!registry.FindBinding(1,"z")&&!registry.FindBinding(1,0)&&!registry.FindBinding(1,""));
 limits=B::Settings();limits.version=2;CHECK("unsupported settings version rejected",!registry.Build(0,0,0,0,limits)&&Failed(registry,B::InvalidSettings));
 limits=B::Settings();limits.maximumBytes=7;CHECK("budget below empty framing rejected",!registry.Build(0,0,0,0,limits)&&Failed(registry,B::InvalidSettings));
 limits=B::Settings();limits.maximumBytes=B::MaximumBytes+1;CHECK("oversized byte budget rejected",!registry.Build(0,0,0,0,limits)&&Failed(registry,B::InvalidSettings));
 limits=B::Settings();limits.maximumRecords=0;CHECK("zero record budget rejected",!registry.Build(0,0,0,0,limits)&&Failed(registry,B::InvalidSettings));
 limits=B::Settings();limits.maximumRecords=B::MaximumRecords+1;CHECK("oversized record budget rejected",!registry.Build(0,0,0,0,limits)&&Failed(registry,B::InvalidSettings));
 limits=B::Settings();limits.maximumBindings=0;CHECK("zero binding budget rejected",!registry.Build(0,0,0,0,limits)&&Failed(registry,B::InvalidSettings));
 limits=B::Settings();limits.maximumBindings=B::MaximumBindings+1;CHECK("oversized binding budget rejected",!registry.Build(0,0,0,0,limits)&&Failed(registry,B::InvalidSettings));
 limits=B::Settings();limits.maximumRecords=2;CHECK("configured record count is enforced",!registry.Build(records,3,0,0,limits)&&Failed(registry,B::ResourceLimit));
 limits=B::Settings();limits.maximumBindings=1;CHECK("configured binding count is enforced",!registry.Build(records,3,pair,2,limits)&&Failed(registry,B::ResourceLimit));
 CHECK("100001 records rejected before array access",!registry.Build(records,B::MaximumRecords+1)&&Failed(registry,B::ResourceLimit));
 CHECK("100001 bindings rejected before array access",!registry.Build(records,3,pair,B::MaximumBindings+1)&&Failed(registry,B::ResourceLimit));
 B::RecordInput small={1,payloadA,3};limits=B::Settings();limits.maximumBytes=8+12+4+sizeof(B::Record);CHECK("exact aligned total owned byte budget accepted",registry.Build(&small,1,0,0,limits)&&registry.OwnedBytes()==limits.maximumBytes);
 limits.maximumBytes--;CHECK("metadata cannot bypass total owned byte budget",!registry.Build(&small,1,0,0,limits)&&Failed(registry,B::ResourceLimit));
 limits=B::Settings();limits.maximumBytes=20;CHECK("table bytes are counted before allocation",!registry.Build(records,3,0,0,limits)&&Failed(registry,B::ResourceLimit));
 const unsigned int largeBytes=B::MaximumBytes-8-12-sizeof(B::Record);unsigned char* large=(unsigned char*)B::detail::Allocate(largeBytes+1);if(!large)ExitProcess(81);large[0]=13;large[largeBytes-1]=17;B::RecordInput largeRecord={1,large,largeBytes};
 CHECK("sixteen MiB total owned boundary accepted",registry.Build(&largeRecord,1)&&registry.OwnedBytes()==B::MaximumBytes&&((const unsigned char*)registry.Records()[0].data)[largeBytes-1]==17);
 largeRecord.bytes++;CHECK("one byte beyond total owned boundary rejected",!registry.Build(&largeRecord,1)&&Failed(registry,B::ResourceLimit));B::detail::Release(large);
 B::RecordInput* manyRecords=(B::RecordInput*)B::detail::Allocate(sizeof(B::RecordInput)*B::MaximumRecords);if(!manyRecords)ExitProcess(82);for(unsigned int i=0;i<B::MaximumRecords;i++){manyRecords[i].typeId=i+1;manyRecords[i].data=0;manyRecords[i].bytes=0;}
 CHECK("100000 append ordinals accepted without payload deduplication",registry.Build(manyRecords,B::MaximumRecords)&&registry.Count()==B::MaximumRecords&&registry.Records()[99999].typeId==100000&&registry.Records()[99999].start==0);B::detail::Release(manyRecords);
 B::BindingInput* manyBindings=(B::BindingInput*)B::detail::Allocate(sizeof(B::BindingInput)*B::MaximumBindings);if(!manyBindings)ExitProcess(83);for(unsigned int i=0;i<B::MaximumBindings;i++){manyBindings[i].sourceNode=i;manyBindings[i].property="p";manyBindings[i].ordinal=0;manyBindings[i].typeId=small.typeId;}
 CHECK("100000 bindings hash lookup preserves every source identity",registry.Build(&small,1,manyBindings,B::MaximumBindings)&&registry.BindingCount()==B::MaximumBindings&&registry.FindBinding(0,"p")&&registry.FindBinding(99999,"p")&&!registry.FindBinding(100000,"p"));
 manyBindings[99999].sourceNode=0;CHECK("late duplicate binding fails atomically",!registry.Build(&small,1,manyBindings,B::MaximumBindings)&&Failed(registry,B::DuplicateBinding)&&registry.failedIndex==99999);B::detail::Release(manyBindings);
 unsigned char survivingCopy[56];bool lifetime=false;{B::Registry local;lifetime=local.Build(records,3,bindings,2)&&local.Write(survivingCopy,sizeof(survivingCopy));}CHECK("external copy survives registry destruction",lifetime&&B::detail::Get(survivingCopy)==3&&B::detail::Get(survivingCopy+48)==0x83f47c31u);
 bool repeated=true;for(unsigned int i=0;i<32;i++){if(!registry.Build(records,3,bindings,2))repeated=false;registry.Clear();registry.Clear();}CHECK("repeated clear and rebuild resets diagnostics and ownership",repeated&&!registry.error&&!registry.Data()&&!registry.Bindings()&&!registry.Count()&&!registry.OwnedBytes());
 char target[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_BLIND_REGISTRY_GUARDS_OUTPUT",target,MAX_PATH))ExitProcess(84);HANDLE file=CreateFileA(target,0x40000000u,0,0,2,FILE_ATTRIBUTE_NORMAL,0);if(file==INVALID_HANDLE_VALUE)ExitProcess(85);DWORD written=0;unsigned int header[3]={0x3147424eu,checkCount,sizeof(example)};bool ok=WriteFile(file,header,sizeof(header),&written,0)&&written==sizeof(header);ok=ok&&WriteFile(file,checks,checkCount*4,&written,0)&&written==checkCount*4;ok=ok&&WriteFile(file,example,sizeof(example),&written,0)&&written==sizeof(example);CloseHandle(file);if(!ok)ExitProcess(86);for(unsigned int i=0;i<checkCount;i++)if(!checks[i])ExitProcess(100+i);ExitProcess(0);
}
