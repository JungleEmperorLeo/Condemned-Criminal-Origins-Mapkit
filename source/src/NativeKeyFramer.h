#pragma once
#include "NativePreparedRenderScene.h"
#include "SourceRuntimeObjectOrder.h"
#include "NativeBlindDataRegistry.h"
#include "SourceKeyRotation.h"
#include "NativeExternalKeyFile.h"
#include "LTRotationOps.h"
// Source producer profile: Linear and object-key Bezier keys, recovered orientations,
// cumulative finite timestamps. No compiled key bytes or expected map table.
namespace NativeKeyFramer {
static const unsigned int TypeId=0x6aaf0884u,Missing=0xffffffffu;
struct Settings {unsigned int version,maximumBytes,maximumKeys,maximumFramers;Settings():version(1),maximumBytes(16u*1024u*1024u),maximumKeys(100000),maximumFramers(4096){}};
struct Record {unsigned int sourceNode,keyCount;unsigned char* data;unsigned int bytes;};
class Builder {
 PrefabScene::Arena memory;Builder(const Builder&);Builder& operator=(const Builder&);
 bool Fail(const char* message,unsigned int node=Missing){char saved[512];lstrcpynA(saved,message?message:"KeyFramer compilation failed",512);Clear();lstrcpyA(diagnostic,saved);error=diagnostic;errorNode=node;return false;}
 static bool Exact(const char* a,const char* b){return GameClassMetadata::Exact(a,b);}
 static const PrefabScene::Property* Property(const PrefabScene::Node& node,const char* name){for(const PrefabScene::Property* p=node.properties;p;p=p->next)if(Exact(p->name,name))return p;return 0;}
 static bool String(const PrefabScene::Node& node,const char* name,const char*& value){const PrefabScene::Property* p=Property(node,name);if(!p||!Exact(p->type,"string")||!p->data)return false;value=p->data;return lstrlenA(value)<=4095;}
 static bool Number(const PrefabScene::Node& node,const char* name,float& value){const PrefabScene::Property* p=Property(node,name);if(!p||!Exact(p->type,"real")||!p->data)return false;WorldText::Document doc;return doc.Parse(p->data,lstrlenA(p->data))&&doc.Elements(0)==1&&doc.Number(doc.First(0),value)&&WorldText::Finite(value);}
 static bool Prefix(const char* text,const char* base){for(;*base;base++,text++){if(!*text)return false;char a=*text,b=*base;if(a>='A'&&a<='Z')a=(char)(a-'A'+'a');if(b>='A'&&b<='Z')b=(char)(b-'A'+'a');if(a!=b)return false;}return true;}
 static bool KeyName(const char* base,unsigned int index,char* target){unsigned int length=lstrlenA(base);char number[16];char* p=number;unsigned int value=index;do{*p++=(char)('0'+value%10);value/=10;}while(value);unsigned int digits=(unsigned int)(p-number),zero=index>0&&index<10?1u:0u;if(length+digits+zero>=256)return false;memcpy(target,base,length);if(zero)target[length++]='0';while(p!=number)target[length++]=*--p;target[length]=0;return true;}
 static bool Canonical(const char* text,const char* base){if(!Prefix(text,base))return false;const char* suffix=text+lstrlenA(base);if(!suffix[0])return false;unsigned int index=0;for(const char* p=suffix;*p;p++){if(*p<'0'||*p>'9'||index>(100000u-(unsigned int)(*p-'0'))/10)return false;index=index*10+(unsigned int)(*p-'0');}char wanted[256];return KeyName(base,index,wanted)&&FearBrushFlags::Equal(wanted,text);}
 static void Word(unsigned char* target,unsigned int value){memcpy(target,&value,4);}
 static bool SameNoCase(const char* a,const char* b){for(;;a++,b++){char x=*a,y=*b;if(x>='A'&&x<='Z')x=(char)(x-'A'+'a');if(y>='A'&&y<='Z')y=(char)(y-'A'+'a');if(x!=y)return false;if(!x)return true;}}
 // Original interpolation lookup (VA 0x1000C470): case-insensitive {Linear, Bezier}; absent or unknown is Linear.
 static unsigned int Interpolation(const PrefabScene::Node& framer){const char* value=0;return String(framer,"Interpolation",value)&&SameNoCase(value,"Bezier")?1u:0u;}
 // Key tangent (VA 0x1000D4E8): key transform applied to BezierPrev/BezierNext, else the key position.
 static bool Tangent(const PrefabScene::Node& key,const char* name,const float* rotation,const WorldText::Vec3& position,WorldText::Vec3& out){const PrefabScene::Property* p=Property(key,name);if(!p||!Exact(p->type,"vector")||!p->data){out=position;return true;}WorldText::Document doc;WorldText::Vec3 v;if(!doc.Parse(p->data,lstrlenA(p->data)))return false;unsigned int form=doc.First(0),inner=doc.Next(doc.First(form));if(!doc.tokens[inner].list||!doc.Vector(inner,v)||!WorldText::FiniteVector(v))return false;out=LTRotationOps::Apply(rotation,position,v);return WorldText::FiniteVector(out);}
 static void Product(const float* a,const float* b,float* q){
  q[0]=(float)(((double(b[3])*a[0]+double(a[1])*b[2])+double(a[3])*b[0])-double(a[2])*b[1]);
  q[1]=(float)(((double(a[3])*b[1]-double(a[0])*b[2])+double(b[3])*a[1])+double(a[2])*b[0]);
  q[2]=(float)(((double(a[3])*b[2]+double(b[1])*a[0])-double(a[1])*b[0])+double(b[3])*a[2]);
  q[3]=(float)(((double(a[3])*b[3]-double(b[0])*a[0])-double(b[1])*a[1])-double(a[2])*b[2]);
 }
 bool External(const NativePreparedRenderScene::Context& scene,const PrefabScene::Node& framer,const char* path,const char* base,unsigned int interpolation,const Settings& settings,Record& record){const unsigned int stride=interpolation?60u:36u;
  NativeExternalKeyFile::Loader source;if(!source.Load(scene.Project().resources,path,base))return Fail(source.error,framer.index);if(!source.count||source.count>settings.maximumKeys)return Fail("External Key count exceeds profile",framer.index);
  float frameRotation[4];if(!SourceKeyRotation::Quaternion(framer,frameRotation))return Fail("Invalid external KeyFramer orientation",framer.index);WorldText::Vec3 framePosition=framer.position;framePosition.x-=scene.Offset().x;framePosition.y-=scene.Offset().y;framePosition.z-=scene.Offset().z;if(!WorldText::FiniteVector(framePosition))return Fail("Invalid centered external KeyFramer position",framer.index);
  unsigned int commands=0;for(unsigned int i=0;i<source.count;i++){const NativeExternalKeyFile::Record& key=source.records[i];if(!WorldText::Finite(key.duration)||key.duration<0||lstrlenA(key.command)>4095||lstrlenA(key.sound)>4095)return Fail("External Key duration or string exceeds profile",framer.index);if(key.command[0]||key.sound[0]){const unsigned int bytes=lstrlenA(key.command)+lstrlenA(key.sound)+2;if(bytes>settings.maximumBytes-commands)return Fail("External Key command pool byte limit",framer.index);commands+=bytes;}}
  const unsigned long long wanted=12ull+(unsigned long long)stride*source.count+commands;if(wanted>settings.maximumBytes)return Fail("External KeyFramer record byte limit",framer.index);unsigned char* output=(unsigned char*)memory.Allocate((unsigned int)wanted);if(!output)return Fail("External KeyFramer allocation failed",framer.index);const unsigned int poolStart=12+stride*source.count;Word(output,1);Word(output+4,source.count);Word(output+8,poolStart);float stamp=0;unsigned int pool=0;
  for(unsigned int i=0;i<source.count;i++){const NativeExternalKeyFile::Record& key=source.records[i];unsigned char* target=output+12+stride*i;stamp=(float)(double(stamp)+key.duration);if(!WorldText::Finite(stamp))return Fail("External Key timestamp overflow",framer.index);memcpy(target,&stamp,4);unsigned int commandIndex=Missing;if(key.command[0]||key.sound[0]){commandIndex=pool;unsigned int bytes=lstrlenA(key.command)+1;memcpy(output+poolStart+pool,key.command,bytes);pool+=bytes;bytes=lstrlenA(key.sound)+1;memcpy(output+poolStart+pool,key.sound,bytes);pool+=bytes;}Word(target+4,commandIndex);
   WorldText::Vec3 position=SourceSurfaceRotation::Rotate(frameRotation,key.position);position.x+=framePosition.x;position.y+=framePosition.y;position.z+=framePosition.z;if(!WorldText::FiniteVector(position))return Fail("Nonfinite external Key transform",framer.index);memcpy(target+8,&position,12);float local[4],rotation[4];if(!SourceKeyRotation::EulerQuaternion(key.euler,local))return Fail("Invalid external Key rotation",framer.index);Product(frameRotation,local,rotation);for(unsigned int j=0;j<4;j++)if(!WorldText::Finite(rotation[j]))return Fail("Nonfinite external Key quaternion",framer.index);memcpy(target+20,rotation,16);
    // External tangents (VA 0x1000D7CC): the final key transform applied to bezierprev/beziernext.
    if(interpolation){const WorldText::Vec3 previous=LTRotationOps::Apply(rotation,position,key.bezierPrev),next=LTRotationOps::Apply(rotation,position,key.bezierNext);if(!WorldText::FiniteVector(previous)||!WorldText::FiniteVector(next))return Fail("Nonfinite external Bezier tangent",framer.index);memcpy(target+36,&previous,12);memcpy(target+48,&next,12);}
  }
  if(pool!=commands)return Fail("External Key command pool size mismatch",framer.index);record.sourceNode=framer.index;record.keyCount=source.count;record.data=output;record.bytes=(unsigned int)wanted;return true;
 }
 bool Framer(const NativePreparedRenderScene::Context& scene,const PrefabScene::Node& framer,const Settings& settings,Record& record){
  const char *external=0,*base=0;const unsigned int interpolation=Interpolation(framer),stride=interpolation?60u:36u;
  if(!String(framer,"ExternalKeyFile",external))return Fail("Invalid external KeyFramer source property",framer.index);
  if(!String(framer,"BaseKeyName",base)||!base[0]||lstrlenA(base)>245)return Fail("Invalid KeyFramer BaseKeyName",framer.index);
  if(external[0])return External(scene,framer,external,base,interpolation,settings,record);
  unsigned int candidates=0;for(const PrefabScene::Node* n=scene.Model().first;n;n=n->next)if(n->kind==2&&Canonical(n->name,base)){if(candidates==settings.maximumKeys)return Fail("KeyFramer candidate count limit",framer.index);candidates++;}
  // Keys (0x1000E420): base+"0"+i for i 1..9, else base+i, from 0 until a name is
  // missing; the first candidate with that name (0x1000CDA0) is used. Unused
  // candidates (duplicates, gaps) only warn; without candidates the record is empty.
  const PrefabScene::Node** selected=(const PrefabScene::Node**)memory.Allocate((candidates?candidates:1)*sizeof(void*));if(!selected)return Fail("KeyFramer key allocation failed",framer.index);
  unsigned int keys=0,commands=0;for(unsigned int index=0;index<candidates;index++){char wanted[256];if(!KeyName(base,index,wanted))return Fail("KeyFramer generated name exceeds limit",framer.index);const PrefabScene::Node* key=0;
   for(const PrefabScene::Node* n=scene.Model().first;n&&!key;n=n->next)if(n->kind==2&&FearBrushFlags::Equal(n->name,wanted))key=n;
   if(!key)break;if(!Exact(key->className,"Key"))return Fail("Non-Key matching object is outside current KeyFramer profile",key->index);
   float orientation[4];if(!SourceKeyRotation::Quaternion(*key,orientation))return Fail("Invalid Key orientation",key->index);float stamp=0;if(!Number(*key,"TimeStamp",stamp)||stamp<0)return Fail("Invalid Key TimeStamp",key->index);
   const PrefabScene::Property* command=Property(*key,"Command");const char* sound=0;if(!command||!Exact(command->type,"command")||!command->data||lstrlenA(command->data)>4095||!String(*key,"SoundName",sound))return Fail("Invalid Key command or sound",key->index);
   if(command->data[0]||sound[0]){const unsigned int bytes=lstrlenA(command->data)+lstrlenA(sound)+2;if(bytes>settings.maximumBytes-commands)return Fail("Key command pool byte limit",key->index);commands+=bytes;}selected[keys++]=key;
  }
  const unsigned long long wanted=12ull+(unsigned long long)stride*keys+commands;if(wanted>settings.maximumBytes)return Fail("KeyFramer record byte limit",framer.index);unsigned char* output=(unsigned char*)memory.Allocate((unsigned int)wanted);if(!output)return Fail("KeyFramer output allocation failed",framer.index);
  const unsigned int poolStart=12+keys*stride;Word(output,1);Word(output+4,keys);Word(output+8,poolStart);unsigned int pool=0;
  float stamp=0;for(unsigned int i=0;i<keys;i++){const PrefabScene::Node& key=*selected[i];unsigned char* target=output+12+i*stride;float interval=0;if(!Number(key,"TimeStamp",interval))return Fail("Key stamp changed",key.index);stamp=(float)(double(stamp)+interval);if(!WorldText::Finite(stamp))return Fail("Cumulative Key timestamp overflow",key.index);memcpy(target,&stamp,4);const char* command=Property(key,"Command")->data;const char* sound=Property(key,"SoundName")->data;unsigned int commandIndex=Missing;
   if(command[0]||sound[0]){commandIndex=pool;unsigned int bytes=lstrlenA(command)+1;memcpy(output+poolStart+pool,command,bytes);pool+=bytes;bytes=lstrlenA(sound)+1;memcpy(output+poolStart+pool,sound,bytes);pool+=bytes;}Word(target+4,commandIndex);
   WorldText::Vec3 position=key.position;position.x-=scene.Offset().x;position.y-=scene.Offset().y;position.z-=scene.Offset().z;if(!WorldText::FiniteVector(position))return Fail("Nonfinite centered Key position",key.index);memcpy(target+8,&position,12);float rotation[4];if(!SourceKeyRotation::Quaternion(key,rotation))return Fail("Key orientation changed",key.index);memcpy(target+20,rotation,16);
   if(interpolation){if(key.importedFromPrefab)return Fail("Prefab-imported Bezier key is outside current profile",key.index);WorldText::Vec3 previous,next;if(!Tangent(key,"BezierPrev",rotation,position,previous)||!Tangent(key,"BezierNext",rotation,position,next))return Fail("Invalid Bezier Key tangent",key.index);memcpy(target+36,&previous,12);memcpy(target+48,&next,12);}
  }
  if(pool!=commands)return Fail("Key command pool changed",framer.index);record.sourceNode=framer.index;record.keyCount=keys;record.data=output;record.bytes=(unsigned int)wanted;return true;
 }
public:
 // Packer-log check of one KeyFramer in "Converting Key Data" (0x1000EB40):
 // 1 = BaseKeyName (string) or KeyDataIndex (real) missing ("Please update
 // objects", fatal); 2 = ExternalKeyFile cannot be opened below the resource
 // root (fatal); 3 = unused key candidates (warning); 0 = none.
 static unsigned int Check(const PrefabScene::Model& model,const PrefabScene::Node& framer,const char* resources){
  const char* base=0;const PrefabScene::Property* index=Property(framer,"KeyDataIndex");if(!String(framer,"BaseKeyName",base)||!index||!Exact(index->type,"real"))return 1;
  const char* external=0;if(String(framer,"ExternalKeyFile",external)&&external[0]){char full[MAX_PATH];if(!resources||!PrefabScene::Join(resources,external,full))return 2;HANDLE f=CreateFileA(full,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);if(f==INVALID_HANDLE_VALUE)return 2;CloseHandle(f);return 0;}
  unsigned int candidates=0,used=0;for(const PrefabScene::Node* n=model.first;n;n=n->next)if(n->kind==2&&Canonical(n->name,base))candidates++;
  for(;used<candidates;used++){char wanted[256];if(!KeyName(base,used,wanted))break;bool found=false;for(const PrefabScene::Node* n=model.first;n&&!found;n=n->next)if(n->kind==2&&FearBrushFlags::Equal(n->name,wanted))found=true;if(!found)break;}
  return used!=candidates?3:0;
 }
 NativeBlindDataRegistry::RecordInput* records;NativeBlindDataRegistry::BindingInput* bindings;Record* details;unsigned int count,totalKeys,totalBytes,errorNode;const char* error;char diagnostic[512];
 Builder():records(0),bindings(0),details(0),count(0),totalKeys(0),totalBytes(0),errorNode(Missing),error(0){diagnostic[0]=0;}
 void Clear(){memory.Clear();records=0;bindings=0;details=0;count=totalKeys=totalBytes=0;errorNode=Missing;error=0;diagnostic[0]=0;}
 bool Build(const NativePreparedRenderScene::Context& scene,const Settings& settings=Settings()){
  Clear();if(!scene.Ready()||settings.version!=1||settings.maximumBytes<12||settings.maximumBytes>16u*1024u*1024u||!settings.maximumKeys||settings.maximumKeys>100000||!settings.maximumFramers||settings.maximumFramers>4096)return Fail("Invalid KeyFramer scene/settings");
  SourceRuntimeObjectOrder::Builder order;if(!order.Build(scene))return Fail(order.error,order.errorNode);unsigned int wanted=0;for(unsigned int i=0;i<order.count;i++)if(Exact(order.rows[i].className,"KeyFramer")){if(wanted==settings.maximumFramers)return Fail("KeyFramer object limit");wanted++;}
  records=(NativeBlindDataRegistry::RecordInput*)memory.Allocate((wanted?wanted:1)*sizeof(NativeBlindDataRegistry::RecordInput));bindings=(NativeBlindDataRegistry::BindingInput*)memory.Allocate((wanted?wanted:1)*sizeof(NativeBlindDataRegistry::BindingInput));details=(Record*)memory.Allocate((wanted?wanted:1)*sizeof(Record));if(!records||!bindings||!details)return Fail("KeyFramer metadata allocation failed");
  for(unsigned int i=0;i<order.count;i++){if(!Exact(order.rows[i].className,"KeyFramer"))continue;const PrefabScene::Node* framer=0;for(const PrefabScene::Node* n=scene.Model().first;n;n=n->next)if(n->index==order.rows[i].sourceNode){framer=n;break;}if(!framer||!Framer(scene,*framer,settings,details[count]))return framer?false:Fail("KeyFramer source identity missing");
   const Record& record=details[count];if(record.bytes>settings.maximumBytes-totalBytes)return Fail("KeyFramer aggregate payload byte limit",framer->index);records[count].typeId=TypeId;records[count].data=record.data;records[count].bytes=record.bytes;bindings[count].sourceNode=framer->index;bindings[count].property="KeyDataIndex";bindings[count].ordinal=count;bindings[count].typeId=TypeId;totalKeys+=record.keyCount;totalBytes+=record.bytes;count++;
  }return count==wanted||Fail("KeyFramer count mismatch");
 }
};
}
