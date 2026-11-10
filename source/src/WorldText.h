#pragma once
#include "LtaDecimal.h"
namespace WorldText {
struct Token {unsigned int offset,length,child,next;bool list,quoted;};
struct Vec3 {float x,y,z;};
class SourceArena {
 struct Chunk {Chunk* next;unsigned int used,capacity;};Chunk* first;
public:
 SourceArena():first(0){}~SourceArena(){while(first){Chunk* next=first->next;VirtualFree(first,0,MEM_RELEASE);first=next;}}
 void* Allocate(unsigned int bytes){if(bytes>128*1024*1024)return 0;bytes=(bytes+7)&~7u;if(!first||bytes>first->capacity-first->used){unsigned int size=bytes+sizeof(Chunk);if(size<1024*1024)size=1024*1024;Chunk* c=(Chunk*)VirtualAlloc(0,size,MEM_COMMIT|MEM_RESERVE,4);if(!c)return 0;c->next=first;c->used=sizeof(Chunk);c->capacity=size;first=c;}void* result=(unsigned char*)first+first->used;first->used+=bytes;return result;}
};
class Document {
public:
 const char* source;unsigned int bytes,pos,count,capacity,errorOffset;const char* error;Token* tokens;
 SourceArena geometryMemory;
 Document():source(0),bytes(0),pos(0),count(0),capacity(0),errorOffset(0),error(0),tokens(0){}
 ~Document(){if(tokens)VirtualFree(tokens,0,MEM_RELEASE);}
 bool Fail(const char* text,unsigned int at){if(!error){error=text;errorOffset=at;}return false;}
 bool Eq(unsigned int id,const char* text)const{if(!id||tokens[id].list)return false;const Token& t=tokens[id];unsigned int i=0;while(text[i]){if(i==t.length||source[t.offset+i]!=text[i])return false;i++;}return i==t.length;}
 void Copy(unsigned int id,char* out,unsigned int size)const{if(!size)return;unsigned int n=id?tokens[id].length:0;if(n>=size)n=size-1;for(unsigned int i=0;i<n;i++)out[i]=source[tokens[id].offset+i];out[n]=0;}
 unsigned int First(unsigned int id)const{return tokens[id].child;}
 unsigned int Next(unsigned int id)const{return tokens[id].next;}
 unsigned int Value(unsigned int id)const{return id?Next(First(id)):0;}
 unsigned int Form(unsigned int id,const char* name)const{for(unsigned int c=First(id);c;c=Next(c))if(tokens[c].list&&Eq(First(c),name))return c;return 0;}
 unsigned int Container(unsigned int id)const{unsigned int v=Value(id);return v&&tokens[v].list?v:0;}
 unsigned int Elements(unsigned int id)const{unsigned int n=0;for(unsigned int c=First(id);c;c=Next(c))n++;return n;}
 unsigned int At(unsigned int id,unsigned int index)const{unsigned int c=First(id);while(c&&index--)c=Next(c);return c;}
 bool UInt(unsigned int id,unsigned int& out){if(!id||tokens[id].list||!tokens[id].length)return Fail("Expected unsigned integer",id?tokens[id].offset:pos);out=0;const Token& t=tokens[id];for(unsigned int i=0;i<t.length;i++){unsigned char c=source[t.offset+i];if(c<'0'||c>'9'||out>(0xffffffffu-(c-'0'))/10)return Fail("Invalid or overflowing integer",t.offset);out=out*10+c-'0';}return true;}
 bool Number(unsigned int id,float& out,bool allowSpecialBits=false){
  if(!id||tokens[id].list)return Fail("Expected number",id?tokens[id].offset:pos);
  const Token& t=tokens[id];const char* s=source+t.offset;unsigned int n=t.length,i=0;
  union {unsigned int bits;float value;} v;bool hex=false;
  if(n>=2&&s[0]=='0'&&(s[1]=='x'||s[1]=='X')){
   hex=true;if(n!=10)return Fail("Float bit pattern must contain eight hex digits",t.offset);v.bits=0;
   for(i=2;i<n;i++){unsigned char c=s[i];unsigned int d;if(c>='0'&&c<='9')d=c-'0';else if(c>='a'&&c<='f')d=c-'a'+10;else if(c>='A'&&c<='F')d=c-'A'+10;else return Fail("Invalid float bit pattern",t.offset);v.bits=(v.bits<<4)|d;}
  }else{
#ifdef FEAR_LTA_DECIMAL_COMPAT
   v.value=(float)LtaDecimal::Read(s,n);
#else
   bool negative=false;if(i<n&&(s[i]=='-'||s[i]=='+'))negative=s[i++]=='-';double result=0,scale=1;unsigned int digits=0;
   while(i<n&&s[i]>='0'&&s[i]<='9'){result=result*10+s[i++]-'0';digits++;}
   if(i<n&&s[i]=='.'){i++;while(i<n&&s[i]>='0'&&s[i]<='9'){scale*=0.1;result+=(s[i++]-'0')*scale;digits++;}}
   if(!digits)return Fail("Invalid decimal number",t.offset);
   if(i<n&&(s[i]=='e'||s[i]=='E')){i++;bool neg=false;if(i<n&&(s[i]=='-'||s[i]=='+'))neg=s[i++]=='-';int exponent=0;unsigned int start=i;while(i<n&&s[i]>='0'&&s[i]<='9'){exponent=exponent*10+s[i++]-'0';if(exponent>38)return Fail("Exponent outside supported range",t.offset);}if(i==start)return Fail("Missing exponent",t.offset);while(exponent--)result*=neg?0.1:10.0;}
   if(i!=n)return Fail("Trailing characters in number",t.offset);v.value=(float)(negative?-result:result);
#endif
  }
  if((v.bits&0x7f800000u)==0x7f800000u&&!(allowSpecialBits&&hex))return Fail("Non-finite geometry coordinate",t.offset);
#ifdef FEAR_HEX_MAPPING_ZERO_EXPERIMENT
  if(hex&&allowSpecialBits&&v.bits==0x80000000u)v.bits=0;
#endif
  out=v.value;return true;
 }
 bool Vector(unsigned int list,Vec3& out,bool allowSpecialBits=false){if(Elements(list)!=3)return Fail("Vector must have three components",tokens[list].offset);unsigned int a=First(list),b=Next(a),c=Next(b);return Number(a,out.x,allowSpecialBits)&&Number(b,out.y,allowSpecialBits)&&Number(c,out.z,allowSpecialBits);}
 bool Parse(const char* text,unsigned int size){
  source=text;bytes=size;pos=0;count=1;capacity=CountTokens(text,size)+2; // exact token count of a first pass (large exported sources)
  tokens=(Token*)VirtualAlloc(0,capacity*sizeof(Token),MEM_COMMIT|MEM_RESERVE,4);if(!tokens)return Fail("Could not allocate syntax tree",0);
  tokens[0].list=true;
  if(size>=3&&(unsigned char)text[0]==0xef&&(unsigned char)text[1]==0xbb&&(unsigned char)text[2]==0xbf)pos=3;
  return Children(0,0,false);
 }
private:
 // Upper bound of the tokens Children() creates: every '(' list, quoted string
 // and bare atom (same delimiters); malformed text fails later in Children().
 static unsigned int CountTokens(const char* s,unsigned int n){
  unsigned int tokens=0,i=0;
  while(i<n){char c=s[i];
   if(c==' '||c=='\t'||c=='\r'||c=='\n'||c==')'){i++;continue;}
   tokens++;if(c=='('){i++;continue;}
   if(c=='"'){i++;while(i<n&&s[i]!='"')i++;i++;continue;}
   while(i<n&&s[i]!=' '&&s[i]!='\t'&&s[i]!='\r'&&s[i]!='\n'&&s[i]!='('&&s[i]!=')'&&s[i]!='"')i++;
  }
  return tokens;
 }
 bool Space(char c)const{return c==' '||c=='\t'||c=='\r'||c=='\n';}
 bool Children(unsigned int parent,unsigned int depth,bool closing){
  if(depth>128)return Fail("Syntax nesting exceeds 128",pos);unsigned int last=0;
  while(pos<bytes){
   while(pos<bytes&&Space(source[pos]))pos++;if(pos==bytes)break;
   if(source[pos]==')'){if(!closing)return Fail("Unexpected closing parenthesis",pos);pos++;return true;}
   if(count==capacity)return Fail("Syntax token limit exceeded",pos);
   unsigned int id=count++;Token& t=tokens[id];t.offset=pos;
   if(last)tokens[last].next=id;else tokens[parent].child=id;last=id;
   if(source[pos]=='('){t.list=true;pos++;if(!Children(id,depth+1,true))return false;t.length=pos-t.offset;}
   else if(source[pos]=='"'){t.quoted=true;t.offset=++pos;while(pos<bytes&&source[pos]!='"'){if(!source[pos])return Fail("NUL inside string",pos);pos++;}if(pos==bytes)return Fail("Unterminated string",t.offset-1);t.length=pos-t.offset;pos++;}
   else{while(pos<bytes&&!Space(source[pos])&&source[pos]!='('&&source[pos]!=')'&&source[pos]!='"'){if(!source[pos])return Fail("Unexpected NUL byte",pos);pos++;}t.length=pos-t.offset;}
  }
  return closing?Fail("Unterminated list",tokens[parent].offset):true;
 }
};

#include "PolygonData.h"
// typeless: a brush without a string Type property (deleted by the original at
// 0x10028670); unknownType: a Type outside GetBrushFlags_Type (0x1001BE30), whose
// flags are 0 there after a one-time warning; such a brush stays knownType.
struct SceneNode {unsigned int form,id,parent,property,geometry,kind,surfaceFlags;bool knownType,hidden,frozen,typeless,unknownType;char type[64],className[64],name[256];Vec3 position,rotation;};
class Scene {
public:
 Document doc;Geometry* geometry;unsigned int* properties;SceneNode* nodes;
 unsigned int geometryCount,propertyCount,nodeCount,brushCount,objectCount,prefabCount,unknownTypes,activePoints,activePolygons;
 Scene():geometry(0),properties(0),nodes(0),geometryCount(0),propertyCount(0),nodeCount(0),brushCount(0),objectCount(0),prefabCount(0),unknownTypes(0),activePoints(0),activePolygons(0){}
 ~Scene(){} // Geometry allocations belong to doc.geometryMemory.

 unsigned int Property(unsigned int id,const char* name)const{unsigned int list=doc.Container(properties[id]);for(unsigned int p=doc.First(list);p;p=doc.Next(p)){unsigned int key=doc.Next(doc.First(p));if(doc.Eq(key,name))return p;}return 0;}
 unsigned int Data(unsigned int property)const{return doc.Value(doc.Form(property,"data"));}
 bool Load(const char* source,unsigned int bytes){
  if(!doc.Parse(source,bytes))return false;
  unsigned int world=doc.Form(0,"world");if(!world)return doc.Fail("Missing world root",0);
  unsigned int header=doc.Container(doc.Form(world,"header")),version=doc.Value(doc.Form(header,"versioncode"));unsigned int number;
  if(!doc.UInt(version,number))return false;if(number!=2)return doc.Fail("Unsupported World00a version",doc.tokens[version].offset);
  unsigned int gl=doc.Container(doc.Form(world,"polyhedronlist"));
  unsigned int pl=doc.Container(doc.Form(world,"globalproplist"));
  if(!gl||!pl)return doc.Fail("Missing geometry or property table",doc.tokens[world].offset);
  geometryCount=doc.Elements(gl);propertyCount=doc.Elements(pl);
  geometry=(Geometry*)doc.geometryMemory.Allocate((geometryCount+1)*sizeof(Geometry));
  properties=(unsigned int*)doc.geometryMemory.Allocate((propertyCount+1)*sizeof(unsigned int));
  unsigned int nodeCapacity=0;for(unsigned int t=1;t<doc.count;t++)if(doc.tokens[t].list&&doc.Eq(doc.First(t),"worldnode"))nodeCapacity++;
  nodes=(SceneNode*)doc.geometryMemory.Allocate((nodeCapacity+1)*sizeof(SceneNode));
  if(!geometry||!properties||!nodes)return doc.Fail("Could not allocate scene tables",0);
  unsigned int i=0;
  for(unsigned int g=doc.First(gl);g;g=doc.Next(g),i++){
   if(!doc.Eq(doc.First(g),"polyhedron"))return doc.Fail("Expected polyhedron",doc.tokens[g].offset);
   Geometry& geom=geometry[i];geom.form=g;unsigned int body=doc.Container(g);
   if(!body)return doc.Fail("Missing polyhedron body",doc.tokens[g].offset);
   unsigned int points=doc.Form(body,"pointlist");
   for(unsigned int p=doc.Value(points);p;p=doc.Next(p))geom.points++;
   geom.vertices=(Vec3*)doc.geometryMemory.Allocate((geom.points+1)*sizeof(Vec3));if(!geom.vertices)return doc.Fail("Could not allocate vertices",doc.tokens[g].offset);
   unsigned int vi=0;
   geom.colors=(PointColor*)doc.geometryMemory.Allocate((geom.points+1)*sizeof(PointColor));if(!geom.colors)return doc.Fail("Could not allocate source point colors",doc.tokens[g].offset);
   for(unsigned int p=doc.Value(points);p;p=doc.Next(p),vi++){
    unsigned int a=doc.First(p),b=doc.Next(a),c=doc.Next(b);if(!doc.Number(a,geom.vertices[vi].x)||!doc.Number(b,geom.vertices[vi].y)||!doc.Number(c,geom.vertices[vi].z))return false;
    unsigned int color=doc.Next(c);unsigned char* channels=&geom.colors[vi].r;
    for(unsigned int k=0;k<4;k++){
     unsigned int value=255;if(color){if(!doc.UInt(color,value)||value>255)return doc.Fail("Point color outside byte range",doc.tokens[color].offset);color=doc.Next(color);}channels[k]=(unsigned char)value;
    }
   }
   unsigned int polygons=doc.Container(doc.Form(body,"polylist"));
   if(!polygons)return doc.Fail("Missing polylist in geometry",doc.tokens[g].offset);
   for(unsigned int p=doc.First(polygons);p;p=doc.Next(p)){
    if(!doc.Eq(doc.First(p),"editpoly"))return doc.Fail("Expected editpoly",doc.tokens[p].offset);geom.polygons++;
    unsigned int face=doc.Form(p,"f"),refs=0;
    for(unsigned int r=doc.Value(face);r;r=doc.Next(r)){unsigned int index;if(!doc.UInt(r,index))return false;if(index>=geom.points)return doc.Fail("Polygon point index outside geometry",doc.tokens[r].offset);refs++;}
    if(refs<3)return doc.Fail("Polygon has fewer than three vertices",doc.tokens[p].offset);geom.references+=refs;
   }
   if(!ReadPolygons(doc,geom,polygons))return false;
  }
  i=0;for(unsigned int p=doc.First(pl);p;p=doc.Next(p),i++){if(!doc.Eq(doc.First(p),"proplist"))return doc.Fail("Expected proplist",doc.tokens[p].offset);properties[i]=p;}
  unsigned int hierarchy=doc.Form(world,"nodehierarchy"),root=doc.Form(hierarchy,"worldnode");if(!root)return doc.Fail("Missing node hierarchy",doc.tokens[world].offset);
  return Walk(root,0xffffffffu,0);
 }
private:
 bool PropertyVector(unsigned int prop,const char* key,Vec3& result){unsigned int p=Property(prop,key);if(!p)return true;unsigned int data=Data(p),values=doc.Value(data);if(!data||!doc.tokens[data].list||!values||!doc.tokens[values].list)return doc.Fail("Invalid property vector",doc.tokens[p].offset);return doc.Vector(values,result);}
 bool Walk(unsigned int form,unsigned int parent,unsigned int depth){
  if(depth>128)return doc.Fail("Scene nesting exceeds 128",doc.tokens[form].offset);
  unsigned int idx=nodeCount++;SceneNode& n=nodes[idx];n.form=form;n.parent=parent;n.geometry=0xffffffffu;n.typeless=n.unknownType=false;
  if(!doc.UInt(doc.Value(doc.Form(form,"nodeid")),n.id))return false;
  unsigned int props=doc.Form(form,"properties"),propid=doc.Value(doc.Form(props,"propid"));
  if(!doc.UInt(propid,n.property))return false;if(n.property>=propertyCount)return doc.Fail("Property ID outside global table",doc.tokens[propid].offset);
  doc.Copy(doc.Value(doc.Form(props,"name")),n.className,sizeof(n.className));
  unsigned int named=Property(n.property,"Name");if(named)doc.Copy(Data(named),n.name,sizeof(n.name));
  unsigned int flags=doc.Container(doc.Form(form,"flags"));
  for(unsigned int f=flags?doc.First(flags):0;f;f=doc.Next(f)){if(doc.Eq(f,"hidden"))n.hidden=true;if(doc.Eq(f,"frozen"))n.frozen=true;}
  unsigned int type=doc.Value(doc.Form(form,"type"));
  if(doc.Eq(type,"brush")){
   n.kind=1;brushCount++;if(!doc.UInt(doc.Value(doc.Form(form,"brushindex")),n.geometry))return false;
   if(n.geometry>=geometryCount)return doc.Fail("Brush index outside geometry table",doc.tokens[form].offset);
   activePoints+=geometry[n.geometry].points;activePolygons+=geometry[n.geometry].polygons;
   unsigned int tp=Property(n.property,"Type");if(tp)doc.Copy(Data(tp),n.type,sizeof(n.type));else lstrcpyA(n.type,"Normal");n.typeless=!tp||!doc.Eq(doc.First(tp),"string");
   unsigned int base=0;n.knownType=FearBrushFlags::Base(n.type,base);n.unknownType=false;
   if(!n.knownType){unknownTypes++;n.unknownType=true;n.knownType=true;base=0;}
   unsigned int step=Property(n.property,"NotAStep"),clip=Property(n.property,"ClipLight"),lod=Property(n.property,"ShadowLOD");
   unsigned int stepValue=0,clipValue=1;if(step&&!doc.UInt(Data(step),stepValue))return false;if(clip&&!doc.UInt(Data(clip),clipValue))return false;
   char lodName[64]="Low";if(lod)doc.Copy(Data(lod),lodName,sizeof(lodName));n.surfaceFlags=FearBrushFlags::Modify(base,stepValue!=0,clip!=0,clipValue!=0,lodName);
  }else if(doc.Eq(type,"object")){n.kind=2;objectCount++;}
  else if(doc.Eq(type,"prefabref")){n.kind=3;prefabCount++;}
  else if(!doc.Eq(type,"null"))return doc.Fail("Unsupported hierarchy node type",doc.tokens[type].offset);
  if(!PropertyVector(n.property,"Pos",n.position)||!PropertyVector(n.property,"Rotation",n.rotation))return false;
  unsigned int children=doc.Container(doc.Form(form,"childlist"));
  for(unsigned int child=children?doc.First(children):0;child;child=doc.Next(child)){if(!doc.Eq(doc.First(child),"worldnode"))return doc.Fail("Expected child worldnode",doc.tokens[child].offset);if(!Walk(child,idx,depth+1))return false;}
  return true;
 }
};
}

