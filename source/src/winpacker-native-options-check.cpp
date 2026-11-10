#include "WorldPacker.cpp"
#include "WinPackerNativeOptions.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
namespace OptionsHost {
static unsigned int checks[128],used=0;
#define CHECK(name,condition) checks[used++]=(condition)?1u:0u
static bool Same(const char* a,const char* b){if(!a||!b)return false;while(*a&&*a==*b){a++;b++;}return *a==*b;}
class Host:public IPackerUI {
 unsigned char storage[8][32];char names[8][64],helps[8][512],values[8][MAX_PATH],filters[8][128];
public:
 CPackerProperty* pointers[10];CPackerPropList list;unsigned int limit;bool rawLayout;
 Host():limit(8),rawLayout(true){list.properties=pointers;list.count=0;memset(pointers,0,sizeof(pointers));}
 ~Host(){for(unsigned int i=0;i<list.count&&i<8;i++)if(pointers[i])pointers[i]->~CPackerProperty();}
 bool CreateProperty(const CPackerProperty& source,const char* group){
  if(list.count>=limit||list.count>=8)return false;unsigned int i=list.count;const unsigned char* p=(const unsigned char*)&source;
  // Independent frontend clone reads the documented raw layout, as the old
  // frontend's concrete copy constructors do. All strings are copied here.
  const char* name=*(const char* const*)(p+4),*help=*(const char* const*)(p+8);if(!name||!help||!Same(group,"Native compiler")||p[12]!=1)rawLayout=false;lstrcpynA(names[i],name?name:"",64);lstrcpynA(helps[i],help?help:"",512);
  const unsigned int type=source.GetType();if(type==0)pointers[i]=new(storage[i])CPackerBoolProperty(names[i],p[16]!=0,helps[i]);
  else if(type==1){const char* filter=*(const char* const*)(p+20),*value=*(const char* const*)(p+24);lstrcpynA(filters[i],filter?filter:"",128);lstrcpynA(values[i],value?value:"",MAX_PATH);pointers[i]=new(storage[i])CPackerStringProperty(names[i],values[i],p[16]!=0,p[17]!=0,p[18]!=0,filters[i],helps[i]);if(i==0&&(!p[16]||!p[17]||p[18]))rawLayout=false;}
  else return false;list.count++;return true;
 }
 bool CreateReference(const char*,const char*){return false;}
 void String(unsigned int i,const char* value){*(const char**)((unsigned char*)pointers[i]+24)=value;}
 void Flag(unsigned int i,bool value){*((unsigned char*)pointers[i]+16)=value?1u:0u;}
};
 static bool Cleared(const WinPackerNativeOptions::Settings& s){return !s.project[0]&&!s.resources[0]&&!s.output[0]&&!s.worldStem[0]&&Same(s.postProfile,"auto")&&s.error[0]&&s.centerWorld&&!s.splitMeshes&&s.sectionsOnly;}
static bool Write(HANDLE f,const void* p,unsigned int bytes){DWORD done=0;return WriteFile(f,p,bytes,&done,0)&&done==bytes;}
}
extern "C" void HarnessEntry(){using namespace OptionsHost;
 CHECK("null frontend is rejected",!WinPackerNativeOptions::Register(0));
 static unsigned int shortMemory[(sizeof(Host)+3)/4],hostMemory[(sizeof(Host)+3)/4];Host& shortHost=*new(shortMemory)Host;shortHost.limit=2;CHECK("frontend property creation failure propagates",!WinPackerNativeOptions::Register(&shortHost)&&shortHost.list.count==2);
 Host& host=*new(hostMemory)Host;CHECK("eight native properties clone through recovered UI slots",WinPackerNativeOptions::Register(&host)&&host.list.count==8&&host.rawLayout&&Same(host.pointers[6]->GetName(),"NativePostProfile")&&host.pointers[6]->GetType()==1&&Same(host.pointers[7]->GetName(),"NativeWorldStem")&&host.pointers[7]->GetType()==1);
 WinPackerNativeOptions::Settings out;CHECK("frontend clones retain strings after descriptor destruction",WinPackerNativeOptions::Read(&host.list,out)&&Same(out.project,"SDKResources.proj00")&&!out.resources[0]&&Same(out.output,"compiled")&&Same(out.postProfile,"auto")&&!out.worldStem[0]&&out.sectionsOnly&&out.centerWorld&&!out.splitMeshes);
 char selected[MAX_PATH]="Project With Spaces.proj00";host.String(0,selected);host.String(1,"Resources Folder");host.String(2,"Output Folder");host.Flag(3,false);CHECK("explicit path and full candidate mode values are copied",WinPackerNativeOptions::Read(&host.list,out)&&Same(out.project,selected)&&Same(out.resources,"Resources Folder")&&Same(out.output,"Output Folder")&&!out.sectionsOnly);
 selected[0]='X';CHECK("settings own frontend path values",Same(out.project,"Project With Spaces.proj00"));
 CHECK("null property list clears prior selected settings",!WinPackerNativeOptions::Read(0,out)&&Cleared(out));
 unsigned int count=host.list.count;host.list.count=1025;CHECK("oversized property count rejects before array access",!WinPackerNativeOptions::Read(&host.list,out)&&Cleared(out));host.list.count=count;
 CPackerProperty** saved=host.list.properties;host.list.properties=0;CHECK("missing nonempty property array rejects",!WinPackerNativeOptions::Read(&host.list,out)&&Cleared(out));host.list.properties=saved;
 host.list.count=5;CHECK("missing required mode option rejects",!WinPackerNativeOptions::Read(&host.list,out)&&Cleared(out));host.list.count=count;
 host.pointers[8]=host.pointers[0];host.list.count=9;CHECK("duplicate native property rejects",!WinPackerNativeOptions::Read(&host.list,out)&&Cleared(out));host.list.count=count;
 host.String(0,"");CHECK("empty project rejects",!WinPackerNativeOptions::Read(&host.list,out)&&Cleared(out));host.String(0,"Project.proj00");
 host.String(2,"");CHECK("empty output folder rejects",!WinPackerNativeOptions::Read(&host.list,out)&&Cleared(out));host.String(2,"compiled");
 char longPath[MAX_PATH+1];memset(longPath,'a',MAX_PATH);longPath[MAX_PATH]=0;host.String(0,longPath);CHECK("MAX_PATH length is rejected without truncation",!WinPackerNativeOptions::Read(&host.list,out)&&Cleared(out));longPath[MAX_PATH-1]=0;CHECK("MAX_PATH minus one source string fits",WinPackerNativeOptions::Read(&host.list,out));host.String(0,"Project.proj00");
 host.String(1,0);CHECK("null optional path pointer rejects",!WinPackerNativeOptions::Read(&host.list,out)&&Cleared(out));host.String(1,"");
  host.Flag(4,false);CHECK("unsupported uncentered profile rejects explicitly",!WinPackerNativeOptions::Read(&host.list,out)&&Cleared(out));host.Flag(4,true);host.Flag(5,true);CHECK("enabled split mesh option is copied for native compilation",WinPackerNativeOptions::Read(&host.list,out)&&out.splitMeshes&&out.centerWorld);host.Flag(5,false);
 host.list.count=6;CHECK("saved six option lists use automatic resource selection and inferred world path",WinPackerNativeOptions::Read(&host.list,out)&&Same(out.postProfile,"auto")&&!out.worldStem[0]);host.list.count=7;CHECK("saved seven option lists keep inferred world path",WinPackerNativeOptions::Read(&host.list,out)&&!out.worldStem[0]);host.list.count=count;
 host.String(6,"AuTo");CHECK("automatic resource option normalizes without a map name",WinPackerNativeOptions::Read(&host.list,out)&&Same(out.postProfile,"auto"));
 char profileText[16]="MuLtIpLaYeR";host.String(6,profileText);CHECK("mixed case multiplayer profile normalizes to canonical value",WinPackerNativeOptions::Read(&host.list,out)&&Same(out.postProfile,"multiplayer"));profileText[0]='X';CHECK("normalized profile owns storage independent of frontend value",Same(out.postProfile,"multiplayer"));host.String(6,"SINGLEPLAYER");CHECK("singleplayer profile is accepted and normalized",WinPackerNativeOptions::Read(&host.list,out)&&Same(out.postProfile,"singleplayer"));host.String(6,"TuToRiAl");CHECK("tutorial profile is accepted and normalized",WinPackerNativeOptions::Read(&host.list,out)&&Same(out.postProfile,"tutorial"));
 host.String(6,"");CHECK("explicit empty post profile rejects instead of silently defaulting",!WinPackerNativeOptions::Read(&host.list,out)&&Cleared(out));host.String(6,0);CHECK("null post profile pointer rejects",!WinPackerNativeOptions::Read(&host.list,out)&&Cleared(out));char longProfile[17];memset(longProfile,'a',16);longProfile[16]=0;host.String(6,longProfile);CHECK("post profile input length is bounded without truncation",!WinPackerNativeOptions::Read(&host.list,out)&&Cleared(out));host.String(6,"unknown");CHECK("unknown profile rejects the entire option selection",!WinPackerNativeOptions::Read(&host.list,out)&&Cleared(out));host.String(6,"tutorial");
 host.pointers[8]=host.pointers[6];host.list.count=9;CHECK("duplicate post profile rejects even when values agree",!WinPackerNativeOptions::Read(&host.list,out)&&Cleared(out));host.list.count=count;CPackerProperty* validProfile=host.pointers[6];CPackerBoolProperty wrongProfile("NativePostProfile",true,"type guard");host.pointers[6]=&wrongProfile;CHECK("post profile with wrong property type rejects",!WinPackerNativeOptions::Read(&host.list,out)&&Cleared(out));host.pointers[6]=validProfile;
 CHECK("world path UI is a plain optional string without file picker behavior",!((CPackerStringProperty*)host.pointers[7])->IsFilename()&&!((CPackerStringProperty*)host.pointers[7])->IsFileLoad()&&!((CPackerStringProperty*)host.pointers[7])->ShouldStripFilename());
 char worldPath[MAX_PATH]="Worlds\\Tutorials\\Working Copy";host.String(7,worldPath);CHECK("explicit game relative world path is copied without resolving against compiler folder",WinPackerNativeOptions::Read(&host.list,out)&&Same(out.worldStem,"Worlds\\Tutorials\\Working Copy"));worldPath[0]='X';CHECK("world path settings own frontend bytes",Same(out.worldStem,"Worlds\\Tutorials\\Working Copy"));
 host.String(7,"");CHECK("empty explicit world path preserves inference mode",WinPackerNativeOptions::Read(&host.list,out)&&!out.worldStem[0]);host.String(7,0);CHECK("null world path pointer rejects",!WinPackerNativeOptions::Read(&host.list,out)&&Cleared(out));
 memset(longPath,'a',MAX_PATH);longPath[MAX_PATH]=0;host.String(7,longPath);CHECK("world path rejects MAX_PATH bytes without truncation",!WinPackerNativeOptions::Read(&host.list,out)&&Cleared(out));longPath[MAX_PATH-1]=0;CHECK("world path accepts bounded MAX_PATH minus one bytes",WinPackerNativeOptions::Read(&host.list,out)&&lstrlenA(out.worldStem)==MAX_PATH-1);host.String(7,"");
 host.pointers[8]=host.pointers[7];host.list.count=9;CHECK("duplicate optional world path rejects",!WinPackerNativeOptions::Read(&host.list,out)&&Cleared(out));host.list.count=count;CPackerProperty* validWorld=host.pointers[7];CPackerBoolProperty wrongWorld("NativeWorldStem",true,"type guard");host.pointers[7]=&wrongWorld;CHECK("world path with wrong property type rejects",!WinPackerNativeOptions::Read(&host.list,out)&&Cleared(out));host.pointers[7]=validWorld;
 CHECK("valid options recover after every bounded failure",WinPackerNativeOptions::Read(&host.list,out)&&Same(out.project,"Project.proj00")&&Same(out.output,"compiled"));
 char escaped[32];CPackerStringProperty text("Text","a\\b",false,false,false,"","test");CHECK("temporary string virtual save uses frontend escaped form",text.SaveValue(escaped,32)&&Same(escaped,"\"a\\\\b\""));char shortBuffer[2]={1,1};CHECK("temporary string save refuses insufficient space",!text.SaveValue(shortBuffer,2));
 char output[MAX_PATH];DWORD length=GetEnvironmentVariableA("FEAR_WINPACKER_OPTIONS_GUARDS_OUTPUT",output,MAX_PATH);if(!length||length>=MAX_PATH)ExitProcess(1);HANDLE f=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);unsigned int header[2]={0x314f5057u,used};if(f==INVALID_HANDLE_VALUE||!Write(f,header,8)||!Write(f,checks,used*4))ExitProcess(2);CloseHandle(f);for(unsigned int i=0;i<used;i++)if(!checks[i])ExitProcess(3);ExitProcess(0);
}
