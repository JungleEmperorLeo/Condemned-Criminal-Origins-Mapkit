#pragma once
// Include after the common source reader (WorldText::Document, LtcDecode).
// Source parser only: no compiled payloads, transform, path stamps or bindings.
namespace NativeExternalKeyFile {
enum {MaximumBytes=16u*1024u*1024u,MaximumKeys=100000,MaximumStringBytes=65535};
enum ErrorCode {NoError=0,InvalidInput,InvalidPath,FileFailure,ResourceLimit,DecodeFailure,SyntaxFailure,InvalidTrack,DuplicateKey,SequenceGap,InvalidField,AllocationFailure,InputAlias};
struct Record {unsigned int index;WorldText::Vec3 position,euler,bezierPrev,bezierNext;float duration,soundRadius;const char* sound;const char* command;};
namespace detail {
inline unsigned char Fold(unsigned char c){return c>='A'&&c<='Z'?(unsigned char)(c+32):c;}
inline bool Equal(const char* a,const char* b){for(unsigned int i=0;;i++){if(Fold((unsigned char)a[i])!=Fold((unsigned char)b[i]))return false;if(!a[i])return true;}}
inline unsigned int Length(const char* s,unsigned int max){if(!s)return max+1;for(unsigned int i=0;i<=max;i++)if(!s[i])return i;return max+1;}
inline bool Overlap(const void* a,unsigned int an,const void* b,unsigned int bn){if(!a||!an||!b||!bn)return false;unsigned long long x=(unsigned long long)a,y=(unsigned long long)b;return x<y+bn&&y<x+an;}
inline void Copy(void* a,const void* b,unsigned int n){unsigned char* to=(unsigned char*)a;const unsigned char* from=(const unsigned char*)b;for(unsigned int i=0;i<n;i++)to[i]=from[i];}
inline bool RelativePath(const char* path,unsigned int n){if(!n)return false;unsigned int segment=0;for(unsigned int i=0;i<=n;i++){const char c=path[i];if(c==':'||((unsigned char)c<32&&c))return false;if(c=='/'||c=='\\'||!c){if(i==segment||(i-segment==1&&path[segment]=='.')||(i-segment==2&&path[segment]=='.'&&path[segment+1]=='.'))return false;segment=i+1;}}return true;}
}
class Loader {
 char* source;unsigned int sourceBytes;Record* storage;char* strings;unsigned int stringsBytes;
 Loader(const Loader&);Loader& operator=(const Loader&);
 bool Aliases(const void* p,unsigned int n)const{return detail::Overlap(p,n,source,sourceBytes+1)||detail::Overlap(p,n,storage,count*sizeof(Record))||detail::Overlap(p,n,strings,stringsBytes);}
 bool Fail(ErrorCode code,const char* message,unsigned int at=0){Clear();errorCode=code;error=message;errorOffset=at;return false;}
 static unsigned int Canonical(WorldText::Document& d,unsigned int token,const char* base,unsigned int baseLength){
  const WorldText::Token& t=d.tokens[token];if(t.list||t.length<=baseLength)return 0xffffffffu;const char* text=d.source+t.offset;
  for(unsigned int i=0;i<baseLength;i++)if(detail::Fold((unsigned char)text[i])!=detail::Fold((unsigned char)base[i]))return 0xffffffffu;
  unsigned int index=0;for(unsigned int i=baseLength;i<t.length;i++){unsigned char c=text[i];if(c<'0'||c>'9'||index>(0xffffffffu-(c-'0'))/10)return 0xffffffffu;index=index*10+c-'0';}
  const unsigned int digits=t.length-baseLength;if(!index)return digits==1?0:0xffffffffu;if(index<10)return digits==2&&text[baseLength]=='0'?index:0xffffffffu;if(text[baseLength]=='0')return 0xffffffffu;return index;
 }
 bool Vector(WorldText::Document& d,unsigned int form,WorldText::Vec3& result){const unsigned int a=d.Value(form),b=d.Next(a),c=d.Next(b);return d.Elements(form)==4&&a&&b&&c&&d.Number(a,result.x)&&d.Number(b,result.y)&&d.Number(c,result.z);}
 bool Parse(const char* base){
  const unsigned int baseLength=detail::Length(base,250);if(!baseLength||baseLength>250)return Fail(InvalidTrack,"External key base name must contain 1..250 bytes");
  for(unsigned int i=0;i<baseLength;i++)if((unsigned char)base[i]<32||(unsigned char)base[i]>126)return Fail(InvalidTrack,"External key base name must be ASCII");
  WorldText::Document d;if(!d.Parse(source,sourceBytes))return Fail(SyntaxFailure,"Invalid external key source syntax",d.errorOffset);
  const unsigned int root=d.First(0);if(!root||d.Next(root)||!d.tokens[root].list||!d.Eq(d.First(root),"keys")||d.Elements(root)!=2)return Fail(SyntaxFailure,"External key source requires one keys root");
  const unsigned int list=d.Value(root);if(!list||!d.tokens[list].list)return Fail(SyntaxFailure,"External keys body must be a list");
  sourceEntryCount=d.Elements(list);if(sourceEntryCount>MaximumKeys)return Fail(ResourceLimit,"External key source entry limit");
  unsigned int selected=0,highest=0,totalStrings=0;
  for(unsigned int item=d.First(list);item;item=d.Next(item)){
   if(!d.tokens[item].list||!d.First(item)||d.tokens[d.First(item)].list)return Fail(SyntaxFailure,"External key entry requires a name",d.tokens[item].offset);
   const unsigned int index=Canonical(d,d.First(item),base,baseLength);if(index==0xffffffffu)continue;if(index>=MaximumKeys)return Fail(ResourceLimit,"External key canonical index limit",d.tokens[item].offset);selected++;if(index>highest)highest=index;
  }
  if(!selected)return Fail(InvalidTrack,"No canonical external keys match BaseKeyName");
  storage=(Record*)VirtualAlloc(0,(highest+1)*sizeof(Record),MEM_COMMIT|MEM_RESERVE,4);unsigned char* seen=(unsigned char*)VirtualAlloc(0,highest+1,MEM_COMMIT|MEM_RESERVE,4);
  if(!storage||!seen){if(seen)VirtualFree(seen,0,MEM_RELEASE);return Fail(AllocationFailure,"Cannot allocate external source key records");}
  count=highest+1;
  for(unsigned int item=d.First(list);item;item=d.Next(item)){
   const unsigned int index=Canonical(d,d.First(item),base,baseLength);if(index==0xffffffffu)continue;
   if(seen[index]){VirtualFree(seen,0,MEM_RELEASE);return Fail(DuplicateKey,"Duplicate canonical external key",d.tokens[item].offset);}seen[index]=1;Record& record=storage[index];record.index=index;unsigned int fields=0;
   for(unsigned int f=d.Next(d.First(item));f;f=d.Next(f)){
    const unsigned int key=d.First(f),value=d.Value(f);unsigned int flag=0;if(!d.tokens[f].list||!key||d.tokens[key].list){VirtualFree(seen,0,MEM_RELEASE);return Fail(InvalidField,"Invalid external key field",d.tokens[f].offset);}
    if(d.Eq(key,"pos"))flag=1;else if(d.Eq(key,"rot"))flag=2;else if(d.Eq(key,"time"))flag=4;else if(d.Eq(key,"sound"))flag=8;else if(d.Eq(key,"command"))flag=16;else if(d.Eq(key,"bezierprev"))flag=32;else if(d.Eq(key,"beziernext"))flag=64;else if(d.Eq(key,"soundradius"))flag=128;
    if(!flag||(fields&flag)){VirtualFree(seen,0,MEM_RELEASE);return Fail(InvalidField,"Unknown or duplicate external key field",d.tokens[f].offset);}fields|=flag;bool ok=false;
    if(flag==1)ok=Vector(d,f,record.position);else if(flag==2)ok=Vector(d,f,record.euler);else if(flag==32)ok=Vector(d,f,record.bezierPrev);else if(flag==64)ok=Vector(d,f,record.bezierNext);
    else if(flag==4||flag==128){float& result=flag==4?record.duration:record.soundRadius;ok=value&&d.Elements(f)==2&&d.Number(value,result);}
    else {ok=value&&d.Elements(f)==2&&!d.tokens[value].list&&d.tokens[value].quoted&&d.tokens[value].length<=MaximumStringBytes;if(ok){const unsigned int n=d.tokens[value].length;for(unsigned int j=0;j<n;j++)if((unsigned char)source[d.tokens[value].offset+j]>127)ok=false;if(n+1>MaximumBytes-totalStrings)ok=false;else totalStrings+=n+1;}}
    if(!ok){VirtualFree(seen,0,MEM_RELEASE);return Fail(InvalidField,"Invalid external key value or unsupported string encoding",d.tokens[f].offset);}
   }
   // Deliberately explicit parser profile: all fields consumed by FEAR exist.
   if((fields&127)!=127){VirtualFree(seen,0,MEM_RELEASE);return Fail(InvalidField,"External key source is missing a required field",d.tokens[item].offset);}
  }
  for(unsigned int i=0;i<count;i++)if(!seen[i]){VirtualFree(seen,0,MEM_RELEASE);return Fail(SequenceGap,"External key sequence must begin at zero without gaps");}VirtualFree(seen,0,MEM_RELEASE);
  strings=(char*)VirtualAlloc(0,totalStrings,MEM_COMMIT|MEM_RESERVE,4);if(!strings)return Fail(AllocationFailure,"Cannot allocate external key strings");stringsBytes=totalStrings;unsigned int at=0;
  for(unsigned int item=d.First(list);item;item=d.Next(item)){const unsigned int index=Canonical(d,d.First(item),base,baseLength);if(index==0xffffffffu)continue;Record& record=storage[index];for(unsigned int f=d.Next(d.First(item));f;f=d.Next(f)){const unsigned int key=d.First(f),value=d.Value(f);if(!d.Eq(key,"sound")&&!d.Eq(key,"command"))continue;const unsigned int n=d.tokens[value].length;detail::Copy(strings+at,source+d.tokens[value].offset,n);strings[at+n]=0;if(d.Eq(key,"sound"))record.sound=strings+at;else record.command=strings+at;at+=n+1;}}
  records=storage;return true;
 }
public:
 const Record* records;unsigned int count,sourceEntryCount,encodedBytes,decodedBytes,errorOffset;bool compressed;ErrorCode errorCode;const char* error;char path[MAX_PATH];
 Loader():source(0),sourceBytes(0),storage(0),strings(0),stringsBytes(0),records(0),count(0),sourceEntryCount(0),encodedBytes(0),decodedBytes(0),errorOffset(0),compressed(false),errorCode(NoError),error(0){path[0]=0;}
 ~Loader(){Clear();}
 void Clear(){if(source)VirtualFree(source,0,MEM_RELEASE);if(storage)VirtualFree(storage,0,MEM_RELEASE);if(strings)VirtualFree(strings,0,MEM_RELEASE);source=0;storage=0;strings=0;records=0;sourceBytes=stringsBytes=count=sourceEntryCount=encodedBytes=decodedBytes=errorOffset=0;compressed=false;errorCode=NoError;error=0;path[0]=0;}
 bool BuildText(const char* text,unsigned int bytes,const char* base){
  if(Aliases(text,bytes)||Aliases(base,251))return Fail(InputAlias,"External key input aliases owned loader storage");Clear();
  if(!text||!bytes||bytes>MaximumBytes)return Fail(bytes>MaximumBytes?ResourceLimit:InvalidInput,"Invalid external key text size");
  source=(char*)VirtualAlloc(0,bytes+1,MEM_COMMIT|MEM_RESERVE,4);if(!source)return Fail(AllocationFailure,"Cannot own external key text");detail::Copy(source,text,bytes);source[bytes]=0;sourceBytes=encodedBytes=decodedBytes=bytes;return Parse(base);
 }
 bool Load(const char* resources,const char* externalFile,const char* base){
  if(Aliases(resources,MAX_PATH)||Aliases(externalFile,MAX_PATH)||Aliases(base,251))return Fail(InputAlias,"External key path/base aliases owned loader storage");Clear();
  const unsigned int rootBytes=detail::Length(resources,MAX_PATH-1),relativeBytes=detail::Length(externalFile,MAX_PATH-1);
  if(!rootBytes||rootBytes>=MAX_PATH||relativeBytes>=MAX_PATH||!detail::RelativePath(externalFile,relativeBytes))return Fail(InvalidPath,"External key path must be relative to explicit resource root");
  char selected[MAX_PATH];unsigned int at=rootBytes;if(at+relativeBytes+2>=MAX_PATH)return Fail(InvalidPath,"External key resource path exceeds MAX_PATH");detail::Copy(selected,resources,rootBytes);if(selected[at-1]!='/'&&selected[at-1]!='\\')selected[at++]='\\';detail::Copy(selected+at,externalFile,relativeBytes+1);
  HANDLE file=CreateFileA(selected,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);if(file==INVALID_HANDLE_VALUE)return Fail(FileFailure,"Cannot open external key source");const DWORD size=GetFileSize(file,0);if(size==0xffffffffu||!size||size>MaximumBytes){CloseHandle(file);return Fail(ResourceLimit,"External key source exceeds size profile");}
  char* raw=(char*)VirtualAlloc(0,size+1,MEM_COMMIT|MEM_RESERVE,4);if(!raw){CloseHandle(file);return Fail(AllocationFailure,"Cannot allocate external key source");}DWORD read=0;bool ok=ReadFile(file,raw,size,&read,0)&&read==size;CloseHandle(file);raw[size]=0;if(!ok){VirtualFree(raw,0,MEM_RELEASE);return Fail(FileFailure,"Cannot read complete external key source");}
  const bool ltc=size>=4&&!raw[0]&&!raw[1]&&!raw[2]&&!raw[3];bool parsed=false;
  if(ltc){LtcDecode decoder;if(!decoder.Decode(raw,size)){VirtualFree(raw,0,MEM_RELEASE);return Fail(DecodeFailure,"Cannot decode external key LTC source",decoder.errorOffset);}parsed=BuildText(decoder.data,decoder.size,base);}else parsed=BuildText(raw,size,base);VirtualFree(raw,0,MEM_RELEASE);if(!parsed)return false;
  encodedBytes=size;compressed=ltc;detail::Copy(path,selected,at+relativeBytes+1);return true;
 }
};
}
