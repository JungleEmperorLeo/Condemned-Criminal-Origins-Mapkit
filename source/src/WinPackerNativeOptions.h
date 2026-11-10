#pragma once
#include "win_min.h"
// x86 ABI recovered from tools/shared/packer headers and FEAR RVAs
// 53B80 (Bool), 53C20 (PropList), 22960 (UI callback slots). The frontend
// clones each temporary property synchronously; these descriptors borrow
// static strings and never transfer allocator-owned memory across the DLL.
#ifndef __PACKERPROPERTY_H__
#define __PACKERPROPERTY_H__
class CPackerProperty {
 const char* name;const char* help;bool enabled;
public:
 CPackerProperty(const char* n,const char* h):name(n),help(h),enabled(true){}
 virtual ~CPackerProperty(){}
 const char* GetName()const{return name;}const char* GetHelp()const{return help;}
 virtual unsigned int GetType()const=0;
 bool IsEnabled()const{return enabled;}void SetEnabled(bool value){enabled=value;}
 virtual bool SaveValue(char*,unsigned int)const=0;
 virtual bool LoadValue(char**,unsigned int)=0;
};
class CPackerBoolProperty:public CPackerProperty {
 bool value;
public:
 CPackerBoolProperty(const char* n,bool v,const char* h):CPackerProperty(n,h),value(v){}
 unsigned int GetType()const{return 0;}bool GetValue()const{return value;}
 bool SaveValue(char* p,unsigned int n)const{const char* s=value?"TRUE":"FALSE";unsigned int i=0;if(!p||!n)return false;while(s[i]){if(i+1>=n){p[0]=0;return false;}p[i]=s[i];i++;}p[i]=0;return true;}
 bool LoadValue(char**,unsigned int){return false;}
};
class CPackerStringProperty:public CPackerProperty {
 bool filename,fileLoad,stripFilename;const char* filter;const char* value;
public:
 CPackerStringProperty(const char* n,const char* v,bool file,bool load,bool strip,const char* f,const char* h):CPackerProperty(n,h),filename(file),fileLoad(load),stripFilename(strip),filter(f),value(v){}
 unsigned int GetType()const{return 1;}const char* GetValue()const{return value;}
 bool IsFilename()const{return filename;}bool IsFileLoad()const{return fileLoad;}bool ShouldStripFilename()const{return stripFilename;}const char* GetFileFilter()const{return filter;}
 bool SaveValue(char* p,unsigned int n)const{if(!p||n<3)return false;unsigned int at=0;p[at++]='"';for(unsigned int i=0;value&&value[i];i++){unsigned int wanted=value[i]=='\\'?2u:1u;if(wanted>n-at-2){p[0]=0;return false;}p[at++]=value[i];if(wanted==2)p[at++]='\\';}p[at++]='"';p[at]=0;return true;}
 bool LoadValue(char**,unsigned int){return false;}
};
#endif
#ifndef __PACKERPROPLIST_H__
#define __PACKERPROPLIST_H__
// Nonpolymorphic frontend-owned layout; the DLL only reads it.
class CPackerPropList {public:CPackerProperty** properties;unsigned int count;};
#endif
#ifndef __IPACKERUI_H__
#define __IPACKERUI_H__
class IPackerUI {public:virtual ~IPackerUI(){}virtual bool CreateProperty(const CPackerProperty&,const char*)=0;virtual bool CreateReference(const char*,const char*)=0;};
#endif
static_assert(sizeof(void*)==4&&sizeof(CPackerProperty)==16&&sizeof(CPackerBoolProperty)==20&&sizeof(CPackerStringProperty)==28&&sizeof(CPackerPropList)==8,"WinPacker options require recovered x86 property ABI");
namespace WinPackerNativeOptions {
struct Settings {
 char project[MAX_PATH],resources[MAX_PATH],output[MAX_PATH],postProfile[16],worldStem[MAX_PATH];bool sectionsOnly,centerWorld,splitMeshes;char error[256];
 Settings(){Clear();}
  void Clear(){project[0]=resources[0]=output[0]=worldStem[0]=error[0]=0;lstrcpyA(postProfile,"auto");sectionsOnly=true;centerWorld=true;splitMeshes=false;}
};
inline bool Equal(const char* a,const char* b){if(!a||!b)return false;for(unsigned int i=0;i<256;i++){char x=a[i],y=b[i];if(x>='A'&&x<='Z')x=(char)(x+32);if(y>='A'&&y<='Z')y=(char)(y+32);if(x!=y)return false;if(!x)return true;}return false;}
inline bool Fail(Settings& out,const char* message){out.Clear();lstrcpynA(out.error,message,256);return false;}
inline bool Register(IPackerUI* ui){
 if(!ui)return false;const char* group="Native compiler";
 CPackerStringProperty project("NativeProject","SDKResources.proj00",true,true,false,"FEAR Projects (*.proj00)|*.proj00|All Files (*.*)|*.*||","Project used for source prefabs, class metadata and materials. Relative paths use the compiler folder.");
 CPackerStringProperty resources("NativeResources","",false,false,false,"","Optional resource folder override; blank uses ResourceRoot in the selected project.");
 CPackerStringProperty output("NativeOutput","compiled",false,false,false,"","Output folder. Relative paths use the compiler folder. Source files are never replaced.");
 CPackerBoolProperty sections("NativeSectionsOnly",true,"Compile source sections and diagnostics. Disable only for the supported complete-world profile.");
 CPackerBoolProperty center("CenterWorld",true,"The first native compiler profile requires centered source coordinates.");
  CPackerBoolProperty split("SplitMeshes",false,"Write assigned streaming-region geometry to external Mesh00p files. Files use Game-relative paths beneath the output folder; no assigned regions means embedded geometry.");
  CPackerStringProperty profile("NativePostProfile","auto",false,false,false,"","Leave auto to select game resources from WorldProperties.WorldType. Advanced diagnostic overrides: tutorial, multiplayer, singleplayer.");
 CPackerStringProperty worldStem("NativeWorldStem","",false,false,false,"","Optional Game-relative world path without an extension, for example Worlds\\Tutorials\\MyWorld. Blank derives it from the source location; use this for source copies outside ResourceRoot.");
 return ui->CreateProperty(project,group)&&ui->CreateProperty(resources,group)&&ui->CreateProperty(output,group)&&ui->CreateProperty(sections,group)&&ui->CreateProperty(center,group)&&ui->CreateProperty(split,group)&&ui->CreateProperty(profile,group)&&ui->CreateProperty(worldStem,group);
}
inline const CPackerProperty* Find(const CPackerPropList* list,const char* name,bool& duplicate){
 const CPackerProperty* found=0;duplicate=false;for(unsigned int i=0;i<list->count;i++){const CPackerProperty* p=list->properties[i];if(p&&Equal(p->GetName(),name)){if(found){duplicate=true;return 0;}found=p;}}return found;
}
inline bool Read(const CPackerPropList* list,Settings& out){
 out.Clear();if(!list||list->count>1024||(list->count&&!list->properties))return Fail(out,"Invalid WinPacker option list");
 const char* strings[]={"NativeProject","NativeResources","NativeOutput"};char* targets[]={out.project,out.resources,out.output};
 for(unsigned int i=0;i<3;i++){bool duplicate=false;const CPackerProperty* p=Find(list,strings[i],duplicate);if(duplicate||!p||p->GetType()!=1)return Fail(out,"Missing, duplicate or invalid native path option");const char* value=*(const char* const*)((const unsigned char*)p+24);if(!value)return Fail(out,"Null native path option");unsigned int n=0;while(n<MAX_PATH&&value[n])n++;if(n>=MAX_PATH||(!n&&i!=1))return Fail(out,"Native path option is empty or exceeds MAX_PATH");for(unsigned int j=0;j<=n;j++)targets[i][j]=value[j];}
 const char* bools[]={"NativeSectionsOnly","CenterWorld","SplitMeshes"};bool* flags[]={&out.sectionsOnly,&out.centerWorld,&out.splitMeshes};
 for(unsigned int i=0;i<3;i++){bool duplicate=false;const CPackerProperty* p=Find(list,bools[i],duplicate);if(duplicate||!p||p->GetType()!=0)return Fail(out,"Missing, duplicate or invalid native mode option");*flags[i]=*((const unsigned char*)p+16)!=0;}
 // Older saved lists contain the original six options. Their missing profile
  // retains the automatic default; explicit malformed/duplicate values reject.
 bool duplicate=false;const CPackerProperty* profile=Find(list,"NativePostProfile",duplicate);if(duplicate)return Fail(out,"Duplicate NativePostProfile option");
  if(profile){if(profile->GetType()!=1)return Fail(out,"NativePostProfile must be a string");const char* value=*(const char* const*)((const unsigned char*)profile+24);if(!value)return Fail(out,"Null NativePostProfile option");unsigned int n=0;while(n<sizeof(out.postProfile)&&value[n])n++;if(!n||n>=sizeof(out.postProfile))return Fail(out,"NativePostProfile is empty or exceeds the supported length");const char* canonical=Equal(value,"auto")?"auto":Equal(value,"tutorial")?"tutorial":Equal(value,"multiplayer")?"multiplayer":Equal(value,"singleplayer")?"singleplayer":0;if(!canonical)return Fail(out,"NativePostProfile must be auto, tutorial, multiplayer, or singleplayer");lstrcpyA(out.postProfile,canonical);}
 // The eighth property is optional for saved six/seven-option selections.
 // Preserve its game-relative text; the driver validates the resource path.
 duplicate=false;const CPackerProperty* worldStem=Find(list,"NativeWorldStem",duplicate);if(duplicate)return Fail(out,"Duplicate NativeWorldStem option");
 if(worldStem){if(worldStem->GetType()!=1)return Fail(out,"NativeWorldStem must be a string");const char* value=*(const char* const*)((const unsigned char*)worldStem+24);if(!value)return Fail(out,"Null NativeWorldStem option");unsigned int n=0;while(n<MAX_PATH&&value[n])n++;if(n>=MAX_PATH)return Fail(out,"NativeWorldStem exceeds MAX_PATH");for(unsigned int i=0;i<=n;i++)out.worldStem[i]=value[i];}
 if(!out.centerWorld)return Fail(out,"CenterWorld=false is outside the first native compiler profile");
  return true;
}
}
