#define FEAR_PRIMARY_SURFACE_PREPARATION 1
#include "WorldPacker.cpp"
#include "NativeExternalKeyFile.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
namespace E=NativeExternalKeyFile;
static unsigned int checks[96],count=0;
static void Check(bool result){checks[count++]=result?1u:0u;}
#define CHECK(name,value) Check((value))
static bool Empty(const E::Loader& loader){return !loader.records&&!loader.count&&!loader.sourceEntryCount&&!loader.encodedBytes&&!loader.decodedBytes&&!loader.path[0]&&loader.error;}
static bool Text(E::Loader& loader,const char* source,const char* base="K"){return loader.BuildText(source,(unsigned int)lstrlenA(source),base);}
static const char* Entry="(keys ((\"K0\" (pos 1 2 3)(rot 0 0 0)(time 0.25)(sound \"snd\")(command \"cmd;\")(bezierprev 0 0 0)(beziernext 1 0 0)(soundradius 12))))";
static bool CopyWrite(HANDLE file,const void* bytes,unsigned int size){DWORD written=0;return WriteFile(file,bytes,size,&written,0)&&written==size;}
extern "C" void HarnessEntry(){
 E::Loader loader;
 CHECK("explicit full source entry parses owned raw fields",Text(loader,Entry)&&loader.count==1&&loader.sourceEntryCount==1&&loader.records[0].index==0&&loader.records[0].position.x==1&&loader.records[0].duration==0.25f&&loader.records[0].soundRadius==12&&E::detail::Equal(loader.records[0].command,"cmd;"));
 char mutableText[512];lstrcpyA(mutableText,Entry);const bool parsedMutable=Text(loader,mutableText);mutableText[12]='X';CHECK("caller text mutation cannot change parsed records",parsedMutable&&E::detail::Equal(loader.records[0].sound,"snd")&&loader.records[0].position.z==3);
 const char* alias=loader.records[0].sound;CHECK("owned string alias fails and clears prior output",!loader.BuildText(alias,3,"K")&&loader.errorCode==E::InputAlias&&Empty(loader));
 CHECK("loader can reuse after a typed failure",Text(loader,Entry)&&!loader.error);
 CHECK("no text input rejects",!loader.BuildText(0,1,"K")&&loader.errorCode==E::InvalidInput&&Empty(loader));
 CHECK("zero byte text rejects",!loader.BuildText(Entry,0,"K")&&loader.errorCode==E::InvalidInput&&Empty(loader));
 CHECK("source byte budget rejects before reading",!loader.BuildText(Entry,E::MaximumBytes+1,"K")&&loader.errorCode==E::ResourceLimit&&Empty(loader));
 CHECK("empty base name rejects",!Text(loader,Entry,"")&&loader.errorCode==E::InvalidTrack&&Empty(loader));
 CHECK("null base name rejects",!Text(loader,Entry,0)&&loader.errorCode==E::InvalidTrack&&Empty(loader));
 CHECK("missing keys root rejects",!Text(loader,"(world ())")&&loader.errorCode==E::SyntaxFailure&&Empty(loader));
 CHECK("extra root rejects",!Text(loader,"(keys ())(keys ())")&&loader.errorCode==E::SyntaxFailure&&Empty(loader));
 CHECK("unclosed source syntax rejects",!Text(loader,"(keys ((\"K0\")")&&loader.errorCode==E::SyntaxFailure&&Empty(loader));
 CHECK("atom keys body rejects",!Text(loader,"(keys abc)")&&loader.errorCode==E::SyntaxFailure&&Empty(loader));
 CHECK("empty matching track rejects",!Text(loader,"(keys ())")&&loader.errorCode==E::InvalidTrack&&Empty(loader));
 CHECK("required source fields are explicit profile",!Text(loader,"(keys ((\"K0\" (time 1))))")&&loader.errorCode==E::InvalidField&&Empty(loader));
 const char* duplicate="(keys ((\"K0\" (pos 0 0 0)(rot 0 0 0)(time 0)(sound \"\")(command \"\")(bezierprev 0 0 0)(beziernext 0 0 0))(\"k0\")))";
 CHECK("duplicate canonical name rejects case insensitively",!Text(loader,duplicate)&&loader.errorCode==E::DuplicateKey&&Empty(loader));
 const char* gap="(keys ((\"K01\" (pos 0 0 0)(rot 0 0 0)(time 0)(sound \"\")(command \"\")(bezierprev 0 0 0)(beziernext 0 0 0))))";
 CHECK("track must begin at canonical index zero",!Text(loader,gap)&&loader.errorCode==E::SequenceGap&&Empty(loader));
 CHECK("noncanonical padding is not accepted as zero",!Text(loader,"(keys ((\"K00\")))")&&loader.errorCode==E::InvalidTrack&&Empty(loader));
 CHECK("nonnumeric prefix entries do not form selected keys",!Text(loader,"(keys ((\"KeyFramer00\")))")&&loader.errorCode==E::InvalidTrack&&Empty(loader));
 CHECK("absolute external path is rejected without file access",!loader.Load("C:/","E:/x.Key00c","K")&&loader.errorCode==E::InvalidPath&&Empty(loader));
 CHECK("parent traversal external path is rejected",!loader.Load("C:/","../x.Key00c","K")&&loader.errorCode==E::InvalidPath&&Empty(loader));
 CHECK("nested traversal external path is rejected",!loader.Load("C:/","a/../x.Key00c","K")&&loader.errorCode==E::InvalidPath&&Empty(loader));
 CHECK("empty root is rejected",!loader.Load("","x.Key00c","K")&&loader.errorCode==E::InvalidPath&&Empty(loader));
 CHECK("empty relative source path is rejected",!loader.Load("C:/","","K")&&loader.errorCode==E::InvalidPath&&Empty(loader));
 CHECK("missing external source reports file failure",!loader.Load("C:/","__FEAR_missing_external_key_fixture__.Key00c","K")&&loader.errorCode==E::FileFailure&&Empty(loader));
 char root[MAX_PATH],folder[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_EXTERNAL_KEY_RESOURCES",root,MAX_PATH)||!GetEnvironmentVariableA("FEAR_EXTERNAL_KEY_GUARD_FOLDER",folder,MAX_PATH))ExitProcess(90);
 CHECK("Office compressed source selects one of two tracks",loader.Load(root,"KFAnimations/Demo/TableShoveKeys.Key00c","SlideTableAKey")&&loader.count==26&&loader.sourceEntryCount==52&&loader.compressed&&loader.encodedBytes==2923&&loader.decodedBytes==12704);
 CHECK("Office canonical order is independent of another track",loader.count==26&&loader.records[0].position.x==0.000001f&&loader.records[25].index==25&&loader.records[1].duration==0.033333f&&loader.records[0].command[0]==0&&loader.records[0].sound[0]==0);
 E::Record savedFirst=loader.records[0],savedLast=loader.records[25];unsigned int officeBytes=26*64;unsigned char office[26*64]={};for(unsigned int i=0;i<26;i++){const E::Record& r=loader.records[i];unsigned char* p=office+i*64;E::detail::Copy(p,&r.position,12);E::detail::Copy(p+12,&r.euler,12);E::detail::Copy(p+24,&r.duration,4);E::detail::Copy(p+28,&r.bezierPrev,12);E::detail::Copy(p+40,&r.bezierNext,12);E::detail::Copy(p+52,&r.soundRadius,4);E::detail::Copy(p+56,&r.index,4);}
 CHECK("Office separate track shares file but not selected fields",loader.Load(root,"KFAnimations/Demo/TableShoveKeys.Key00c","Table-SlideKey")&&loader.count==26&&loader.records[0].position.y==-55&&loader.records[0].position.z==30&&savedFirst.position.y==45&&savedLast.index==25);
 char plainPath[MAX_PATH];lstrcpyA(plainPath,folder);lstrcpyA(plainPath+lstrlenA(plainPath),"\\scrambled.Key00a");
 CHECK("plain LTA source supports interleaved tracks and reordered keys",loader.Load(folder,"scrambled.Key00a","K")&&loader.count==2&&loader.sourceEntryCount==3&&!loader.compressed&&loader.records[0].position.x==1&&loader.records[1].position.x==2);
 CHECK("plain source path remains explicit and owned",E::detail::Equal(loader.path,plainPath)&&loader.records[0].duration==0.5f&&loader.records[1].duration==0.25f);
 CHECK("native parser rejects nonfinite vector",!loader.Load(folder,"nonfinite.Key00a","K")&&loader.errorCode==E::InvalidField&&Empty(loader));
 CHECK("native parser rejects duplicate source field",!loader.Load(folder,"duplicate-field.Key00a","K")&&loader.errorCode==E::InvalidField&&Empty(loader));
 CHECK("native parser rejects string with unsupported high bytes",!loader.Load(folder,"nonascii.Key00a","K")&&loader.errorCode==E::InvalidField&&Empty(loader));
 CHECK("native decoder rejects malformed LTC",!loader.Load(folder,"malformed.Key00c","K")&&loader.errorCode==E::DecodeFailure&&Empty(loader));
 loader.Clear();loader.Clear();CHECK("repeated clear removes records and diagnostics",!loader.records&&!loader.count&&!loader.error&&!loader.path[0]);
 char target[MAX_PATH];lstrcpyA(target,folder);lstrcpyA(target+lstrlenA(target),"/guards.bin");HANDLE file=CreateFileA(target,0x40000000u,0,0,2,FILE_ATTRIBUTE_NORMAL,0);if(file==INVALID_HANDLE_VALUE)ExitProcess(91);unsigned int header[3]={0x314B5845u,count,officeBytes};bool ok=CopyWrite(file,header,12)&&CopyWrite(file,checks,count*4)&&CopyWrite(file,office,officeBytes);CloseHandle(file);if(!ok)ExitProcess(92);for(unsigned int i=0;i<count;i++)if(!checks[i])ExitProcess(100+i);ExitProcess(0);
}
