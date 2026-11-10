#pragma once
#include "ClientFxFile.h"
namespace ClientFxResources {
class Reader {
 struct File {ClientFxFile::Reader reader;File* next;File():next(0){}};
 struct History {const char* name;History* next;};
 PrefabScene::Arena data,results;File* first;History* history;bool initialized;
 const char** output;unsigned int used;const ClientFxFile::Group* active[64];
 bool Fail(const char* message){error=message;LogLine(message);return false;}
 bool Add(const char* path){if(!path||!path[0])return true;if(used==100000)return Fail("Too many FX resources");output[used++]=path;return true;}
 bool EqualExact(const char* a,const char* b){while(*a&&*a==*b){a++;b++;}return *a==*b;}
 bool ResourceProperty(unsigned int type,const char* name){
  if(type==0)return FearBrushFlags::Equal(name,"Material")||FearBrushFlags::Equal(name,"SplatEffect");
  if(type==1||type==8||type==9||type==10||type==11||type==14||type==16)return FearBrushFlags::Equal(name,"Material");
  if(type==3)return FearBrushFlags::Equal(name,"Texture");if(type==13)return FearBrushFlags::Equal(name,"VideoName");
  if(type==15)return FearBrushFlags::Equal(name,"OverlayMaterial")||FearBrushFlags::Equal(name,"EdgeMaterial");
  if(type==2&&FearBrushFlags::Equal(name,"Model"))return true;if(type==12&&FearBrushFlags::Equal(name,"CollisionEffect"))return true;
  char prop[32];char* end;unsigned int count=type==2?5:type==4?10:(type==6||type==7)?16:type==12?5:0;
  for(unsigned int i=0;i<count;i++)for(unsigned int alt=0;alt<(type==4?2u:1u);alt++){
   const char* prefix=type==2?"Material":type==4?(alt?"AltSound":"Sound"):type==12?"Effect_":"Effect";lstrcpyA(prop,prefix);end=Decimal(prop+lstrlenA(prefix),i+(type==4||type==12?1:0));*end=0;if(FearBrushFlags::Equal(name,prop))return true;
  }return false;
 }
 bool Equivalent(File& a,const ClientFxFile::Group& x,File& b,const ClientFxFile::Group& y){
  if(x.count!=y.count)return false;for(unsigned int k=0;k<x.count;k++){
   const ClientFxFile::Key& p=x.keys[k];const ClientFxFile::Key& q=y.keys[k];if(p.type!=q.type)return false;unsigned int i=0,j=0;
   for(;;){while(i<p.count&&!ResourceProperty(p.type,p.properties[i].name))i++;while(j<q.count&&!ResourceProperty(q.type,q.properties[j].name))j++;if(i==p.count||j==q.count){if(i!=p.count||j!=q.count)return false;break;}
    const char* left=a.reader.StringValue(p.properties[i]);const char* right=b.reader.StringValue(q.properties[j]);if(!left||!right||!EqualExact(p.properties[i].name,q.properties[j].name)||!EqualExact(left,right))return false;i++;j++;
   }
  }return true;
 }
 const char* Value(File& file,const ClientFxFile::Key& key,const char* name,bool& ok){
  const ClientFxFile::Property* found=0;for(unsigned int i=0;i<key.count;i++)if(FearBrushFlags::Equal(key.properties[i].name,name))found=&key.properties[i];
  if(!found)return "";const char* value=file.reader.StringValue(*found);if(!value){ok=false;error="Invalid FX string property";}return value;
 }
 bool Resource(File& file,const ClientFxFile::Key& key,const char* name){bool ok=true;const char* value=Value(file,key,name,ok);return ok&&Add(value);}
 bool Effect(File& file,const ClientFxFile::Key& key,const char* name,unsigned int depth){bool ok=true;const char* value=Value(file,key,name,ok);return ok&&Effects(value,depth);}
 bool Effects(const char* list,unsigned int depth){
  if(!list)return true;char name[129];unsigned int n=0;
  for(const unsigned char* p=(const unsigned char*)list;;p++){
   if(!*p||*p==';'){if(n){name[n]=0;if(!Collect(name,depth))return false;n=0;}if(!*p)break;}
   else if(!(*p==' '||(*p>=9&&*p<=13))){if(n<128)name[n++]=(char)*p;}
  }return true;
 }
 bool Collect(const char* name,unsigned int depth){
  File* owner=0;const ClientFxFile::Group* group=0;
  for(File* f=first;f;f=f->next)for(unsigned int g=0;g<f->reader.count;g++)if(FearBrushFlags::Equal(f->reader.groups[g].name,name)){if(group){if(!Equivalent(*owner,*group,*f,f->reader.groups[g]))return Fail("Ambiguous FX group name");continue;}owner=f;group=&f->reader.groups[g];}
  if(!group)return true;if(depth>=64)return Fail("FX recursion limit");for(unsigned int i=0;i<depth;i++)if(active[i]==group)return Fail("Cyclic FX resources");active[depth]=group;
  for(unsigned int k=0;k<group->count;k++){
   const ClientFxFile::Key& key=group->keys[k];char prop[32];char* end;
   switch(key.type){
   case 0:if(!Resource(*owner,key,"Material")||!Effect(*owner,key,"SplatEffect",depth+1))return false;break;
   case 1:case 8:case 9:case 10:case 11:case 14:case 16:if(!Resource(*owner,key,"Material"))return false;break;
   case 2:if(!Resource(*owner,key,"Model"))return false;for(unsigned int i=0;i<5;i++){lstrcpyA(prop,"Material");end=Decimal(prop+8,i);*end=0;if(!Resource(*owner,key,prop))return false;}break;
   case 3:if(!Resource(*owner,key,"Texture"))return false;break;
   case 4:
    for(unsigned int alt=0;alt<2;alt++){
     unsigned int count=0;for(unsigned int i=0;i<key.count;i++)for(unsigned int j=1;j<=10;j++){lstrcpyA(prop,alt?"AltSound":"Sound");end=Decimal(prop+(alt?8:5),j);*end=0;if(FearBrushFlags::Equal(key.properties[i].name,prop)){const char* value=owner->reader.StringValue(key.properties[i]);if(!value)return Fail("Invalid sound FX property");if(value[0])count++;}}
     if(count>10)return Fail("Invalid sound FX count");for(unsigned int i=1;i<=count;i++){lstrcpyA(prop,alt?"AltSound":"Sound");end=Decimal(prop+(alt?8:5),i);*end=0;if(!Resource(*owner,key,prop))return false;}
    }break;
   case 5:case 17:break; // CameraShake/Rumble inherit the empty base collector.
   case 6:case 7:
    // Create appends nonempty EffectN values while loading, in property order.
    for(unsigned int p=0;p<key.count;p++)for(unsigned int i=0;i<16;i++){lstrcpyA(prop,"Effect");end=Decimal(prop+6,i);*end=0;if(FearBrushFlags::Equal(key.properties[p].name,prop)){const char* value=owner->reader.StringValue(key.properties[p]);if(!value)return Fail("Invalid create FX property");if(!Effects(value,depth+1))return false;}}break;
   case 12:if(!Effect(*owner,key,"CollisionEffect",depth+1))return false;for(unsigned int i=1;i<=5;i++){lstrcpyA(prop,"Effect_");end=Decimal(prop+7,i);*end=0;if(!Effect(*owner,key,prop,depth+1))return false;}break;
   case 13:if(!Resource(*owner,key,"VideoName"))return false;break;
   case 15:if(!Resource(*owner,key,"OverlayMaterial")||!Resource(*owner,key,"EdgeMaterial"))return false;break;
   default:return Fail("Unsupported FX type");
   }
  }return true;
 }
public:
 const char* error;unsigned int fileCount,groupCount;
 Reader():first(0),history(0),initialized(false),output(0),used(0),error(0),fileCount(0),groupCount(0){}
 ~Reader(){for(File* f=first;f;f=f->next)f->~File();}
 bool Initialize(const PrefabScene::Project& project){
  if(initialized)return true;error=0;char pattern[MAX_PATH],directory[MAX_PATH];if(!PrefabScene::Join(project.resources,"ClientFX",directory)||!PrefabScene::Join(directory,"*.Fx00p",pattern))return Fail("FX directory path too long");
  FindDataA item;HANDLE scan=FindFirstFileA(pattern,&item);if(scan==INVALID_HANDLE_VALUE){initialized=true;return true;}
  bool ok=true;
  do{if(item.fields[0]&16)continue;if(fileCount>=512){ok=Fail("Too many FX files");break;}char path[MAX_PATH];if(!PrefabScene::Join(directory,item.name,path)){ok=Fail("FX path too long");break;}
   HANDLE f=CreateFileA(path,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);if(f==INVALID_HANDLE_VALUE){ok=Fail("Could not open FX file");break;}unsigned int size=GetFileSize(f,0);DWORD done=0;
   unsigned char* bytes=size&&size<32*1024*1024?(unsigned char*)data.Allocate(size):0;bool read=bytes&&ReadFile(f,bytes,size,&done,0)&&done==size;CloseHandle(f);if(!read){ok=Fail("Could not read FX file");break;}
   void* fileStorage=data.Allocate(sizeof(File));if(!fileStorage){ok=Fail("Allocation failed");break;}File* loaded=new(fileStorage) File;loaded->next=first;first=loaded;
   if(!loaded->reader.Read(bytes,size)){ok=Fail(loaded->reader.error);break;}fileCount++;groupCount+=loaded->reader.count;
  }while(FindNextFileA(scan,&item));FindClose(scan);initialized=ok;return ok;
 }
 bool Resolve(const char* name,const char**& paths,unsigned int& count,bool& found){
  results.Clear();paths=0;count=0;found=false;error=0;if(!initialized)return Fail("FX database not initialized");
  // PrefetchUtilities maintains an exact-string top-level history across callbacks.
  for(History* h=history;h;h=h->next)if(EqualExact(h->name,name)){found=true;return true;}
  output=(const char**)results.Allocate(100000*sizeof(char*));used=0;if(!output)return Fail("Allocation failed");if(!Collect(name,0))return false;
  unsigned int length=lstrlenA(name);if(length>4096)return Fail("FX name too long");History* h=(History*)data.Allocate(sizeof(History));char* copy=(char*)data.Allocate(length+1);if(!h||!copy)return Fail("Allocation failed");lstrcpyA(copy,name);h->name=copy;h->next=history;history=h;
  found=true;paths=output;count=used;return true;
 }
};
}
