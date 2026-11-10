#pragma once
namespace ObjectsBlock {
inline float Root(float x){float y;__asm fld x
 __asm fsqrt
 __asm fstp y
 return y;}
inline bool Quaternion(const PrefabScene::Matrix& m,float* q){
 float diag=m.m[0][0]+m.m[1][1]+m.m[2][2],s;
 if(diag<-.999f){unsigned int i=0;if(m.m[1][1]>m.m[0][0])i=1;if(m.m[2][2]>m.m[i][i])i=2;unsigned int j=(i+1)%3,k=(j+1)%3;s=Root(m.m[i][i]-(m.m[j][j]+m.m[k][k])+1);if(s<=0)return false;q[i]=s*.5f;s=.5f/s;q[3]=(m.m[k][j]-m.m[j][k])*s;q[j]=(m.m[j][i]+m.m[i][j])*s;q[k]=(m.m[k][i]+m.m[i][k])*s;}
 else{s=Root(diag+1);if(s<=0)return false;q[3]=s*.5f;s=.5f/s;q[0]=(m.m[2][1]-m.m[1][2])*s;q[1]=(m.m[0][2]-m.m[2][0])*s;q[2]=(m.m[1][0]-m.m[0][1])*s;}
 for(unsigned int i=0;i<4;i++)if(!WorldText::Finite(q[i]))return false;return true;
}
inline float Atan2(float y,float x){float result;__asm fld y
 __asm fld x
 __asm fpatan
 __asm fstp result
 return result;}
inline bool PrefabRotationReplay(const PrefabScene::Matrix& m,float* q){
 WorldText::Vec3 angles;angles.y=Atan2(m.m[0][2],m.m[2][2]);
 float base=Root((float)(double(m.m[0][2])*m.m[0][2]+double(m.m[2][2])*m.m[2][2]));angles.x=-Atan2(m.m[1][2],base);
 
 PrefabScene::Matrix pitch,yaw;WorldText::Vec3 p={-angles.x,0,0},y={0,-angles.y,0};
 if(!PrefabScene::Matrix::Euler(p,pitch)||!PrefabScene::Matrix::Euler(y,yaw))return false;
 PrefabScene::Matrix reverse=pitch.Times(yaw);WorldText::Vec3 up={m.m[0][1],m.m[1][1],m.m[2][1]};up=reverse.Direction(up);angles.z=-Atan2(up.x,up.y);
 PrefabScene::Matrix replay;if(!PrefabScene::Matrix::Euler(angles,replay))return false;return Quaternion(replay,q);
}
typedef bool (*IncludeObject)(const PrefabScene::Node&);
struct Settings {
 typedef bool (*RotationConverter)(const PrefabScene::Node&,float*);
 unsigned int version,maximumBytes,maximumObjects,maximumProperties,maximumPoolBytes;bool rotationReplay;
 RotationConverter rotationConverter;
 Settings():version(1),maximumBytes(16u*1024u*1024u),maximumObjects(100000),maximumProperties(4096),maximumPoolBytes(1024u*1024u),rotationReplay(false),rotationConverter(0){}
};
class Block {
 unsigned int cursor;Settings profile;
 bool Put(const void* input,unsigned int bytes){if(bytes>size-cursor)return false;const unsigned char* p=(const unsigned char*)input;for(unsigned int i=0;i<bytes;i++)data[cursor++]=p[i];return true;}
 bool Fail(const char* msg,unsigned int node=0xffffffffu,const char* property=0){char saved[256];lstrcpynA(saved,property?property:"",256);Clear();error=msg;errorNode=node;lstrcpyA(errorProperty,saved);return false;}
 static unsigned int Padded(unsigned int n){return (n+3)&~3u;}
 static bool Length(const char* text,unsigned int limit,unsigned int& length){if(!text)return false;length=0;while(length<=limit){if(!text[length])return true;length++;}return false;}
 static bool Type(const char* name,unsigned int& type){const char* types[]={"string","vector","color","real","bool","longint","rotation","command","stringid"};for(unsigned int i=0;i<9;i++)if(FearBrushFlags::Equal(name,types[i])){type=i;return true;}return false;}
 static bool StringType(unsigned int type){return type==0||type==7||type==8;}
 static bool PoolType(unsigned int type){return StringType(type)||type==1||type==2||type==6;}
 bool Payload(const PrefabScene::Node& node,const PrefabScene::Property& p,unsigned int type,unsigned char* bytes,unsigned int& value,const WorldText::Vec3& offset){
   if(!p.data)return false;
   if(StringType(type)){unsigned int length;if(!Length(p.data,4095,length))return false;value=length+1;return true;}
   if(type==6){value=16;if(FearBrushFlags::Equal(p.name,"Rotation")){if(profile.rotationConverter)return profile.rotationConverter(node,(float*)bytes);if(profile.rotationReplay&&node.importedFromPrefab)return PrefabRotationReplay(node.orientation,(float*)bytes);return Quaternion(node.orientation,(float*)bytes);}WorldText::Document doc;WorldText::Vec3 v;PrefabScene::Matrix matrix;if(!doc.Parse(p.data,lstrlenA(p.data)))return false;unsigned int form=doc.First(0),inner=doc.Next(doc.First(form));return doc.tokens[inner].list&&doc.Vector(inner,v)&&PrefabScene::Matrix::Euler(v,matrix)&&Quaternion(matrix,(float*)bytes);}
   if(type==1||type==2){WorldText::Vec3 v;if(FearBrushFlags::Equal(p.name,"Pos")){v=node.position;v.x-=offset.x;v.y-=offset.y;v.z-=offset.z;}else{WorldText::Document doc;if(!doc.Parse(p.data,lstrlenA(p.data)))return false;unsigned int form=doc.First(0),inner=doc.Next(doc.First(form));if(!doc.tokens[inner].list||!doc.Vector(inner,v))return false;}if(!WorldText::FiniteVector(v))return false;value=12;for(unsigned int i=0;i<12;i++)bytes[i]=((unsigned char*)&v)[i];return true;}
  // longint: the original reads the integer part with 32-bit wraparound (atol),
  // keeps it as a float property (0x1005E010, +30h) and writes _ftol2 of that
  // float (0x1005DA00): 2147483647 -> 0x80000000, 4294967295 -> -1, fractions
  // truncated. Confirmed by 23 authored values (monolith-research/longint-*).
  if(type==5){const char* s=p.data;bool negative=false;if(*s=='-'||*s=='+')negative=*s++=='-';unsigned int integer=0,digits=0;while(*s>='0'&&*s<='9'){integer=integer*10+(unsigned int)(*s++-'0');digits++;}if(*s=='.'){s++;while(*s>='0'&&*s<='9')s++;}if(!digits||*s)return false;
   volatile float stored=(float)(int)(negative?0u-integer:integer); // |stored| <= 2^31: only +2^31 leaves int32
   value=stored>=2147483648.0f?0x80000000u:(unsigned int)(int)stored;return true;}
  WorldText::Document doc;float number;if(!doc.Parse(p.data,lstrlenA(p.data))||doc.Elements(0)!=1||!doc.Number(doc.First(0),number))return false;
  if(type==3){value=*(unsigned int*)&number;return true;}
  if(type==4){if(number!=0&&number!=1)return false;value=number!=0;return true;}
  return false;
 }
 bool Measure(const PrefabScene::Node& node,const WorldText::Vec3& offset,unsigned int& props,unsigned int& pool){
  props=pool=0;unsigned int length;if(!Length(node.className,63,length)||!length)return Fail("Invalid object class name",node.index);
  for(const PrefabScene::Property* p=node.properties;p;p=p->next){if(props==profile.maximumProperties)return Fail("Object property count limit",node.index,p->name);unsigned int type,value;unsigned char payload[16];
   if(!Length(p->name,255,length)||!Type(p->type,type)||!Payload(node,*p,type,payload,value,offset))return Fail("Unsupported object property payload",node.index,p->name);
   unsigned int wanted=Padded(length+1);if(PoolType(type))wanted+=Padded(value);if(wanted>profile.maximumPoolBytes-pool)return Fail("Object property pool byte limit",node.index,p->name);pool+=wanted;props++;
  }return true;
 }
public:
 PrefabScene::Arena memory;unsigned char* data;unsigned int size,count,properties,errorNode;const char* error;char errorProperty[256];
 Block():cursor(0),data(0),size(0),count(0),properties(0),errorNode(0xffffffffu),error(0){errorProperty[0]=0;}
 void Clear(){memory.Clear();data=0;size=cursor=count=properties=0;errorNode=0xffffffffu;error=0;errorProperty[0]=0;profile=Settings();}
 bool Build(const PrefabScene::Model& model,IncludeObject include,const WorldText::Vec3& offset){
  Settings settings;char mode[4]={};settings.rotationReplay=GetEnvironmentVariableA("FEAR_OBJECT_ROTATION_REPLAY",mode,4)&&mode[0]=='1';return BuildConfigured(model,include,offset,settings);
 }
 bool BuildConfigured(const PrefabScene::Model& model,IncludeObject include,const WorldText::Vec3& offset,const Settings& settings=Settings()){
  Clear();if(!include||settings.version!=1||settings.maximumBytes<4||settings.maximumBytes>16u*1024u*1024u||!settings.maximumObjects||settings.maximumObjects>100000||!settings.maximumProperties||settings.maximumProperties>4096||!settings.maximumPoolBytes||settings.maximumPoolBytes>1024u*1024u||!WorldText::FiniteVector(offset))return Fail("Invalid explicit Objects compiler settings");profile=settings;size=4;unsigned int seen=0;
  for(const PrefabScene::Node* n=model.first;n;n=n->next){if(seen==model.count)return Fail("Invalid object source list");seen++;if(n->kind!=2||!include(*n))continue;if(count==profile.maximumObjects)return Fail("Object count limit",n->index);unsigned int props=0,pool=0;if(!Measure(*n,offset,props,pool))return false;const unsigned int wanted=2+lstrlenA(n->className)+8+pool+props*12;if(wanted>profile.maximumBytes-size)return Fail("Objects section exceeds byte limit",n->index);size+=wanted;count++;properties+=props;}
  if(seen!=model.count)return Fail("Incomplete object source list");
  data=(unsigned char*)memory.Allocate(size);if(!data)return Fail("Object section allocation failed");if(!Put(&count,4))return Fail("Object count overflow");
  for(const PrefabScene::Node* n=model.first;n;n=n->next)if(n->kind==2&&include(*n)){
   unsigned int props=0,poolBytes=0,capacity=0;if(!Measure(*n,offset,props,capacity))return false;unsigned int* records=(unsigned int*)memory.Allocate(props*12);unsigned char* pool=(unsigned char*)memory.Allocate(capacity);if(!records||!pool)return Fail("Object pool allocation failed",n->index);unsigned int i=0;
   for(const PrefabScene::Property* p=n->properties;p;p=p->next,i++){unsigned int type,value;unsigned char payload[16];if(!Type(p->type,type)||!Payload(*n,*p,type,payload,value,offset))return Fail("Invalid object property");records[i*3]=poolBytes;records[i*3+1]=type;unsigned int length=lstrlenA(p->name)+1;for(unsigned int k=0;k<Padded(length);k++)pool[poolBytes+k]=k<length?(unsigned char)p->name[k]:0;poolBytes+=Padded(length);
    if(PoolType(type)){records[i*3+2]=poolBytes;const unsigned char* input=StringType(type)?(const unsigned char*)p->data:payload;for(unsigned int k=0;k<Padded(value);k++)pool[poolBytes+k]=k<value?input[k]:0;poolBytes+=Padded(value);}else records[i*3+2]=value;
   }
   unsigned short length=(unsigned short)lstrlenA(n->className);if(!Put(&length,2)||!Put(n->className,length)||!Put(&props,4)||!Put(&poolBytes,4)||!Put(pool,poolBytes)||!Put(records,props*12))return Fail("Object record overflow");
  }
   if(cursor!=size)return Fail("Objects section size mismatch");return true;
 }
};
}
