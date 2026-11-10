#pragma once
// LTFX v2 disk structures from SDK Game/Shared/ClientFXDB.cpp.
// This reader preserves properties without constructing runtime FX objects.
namespace ClientFxFile {
struct Property {const char* name;const unsigned char* data;unsigned int size;};
struct Key {unsigned int type,id,linkedId,count;float start,end;Property* properties;};
struct Group {const char* name;unsigned int milliseconds,count;Key* keys;};
class Reader {
 PrefabScene::Arena memory;const unsigned char* bytes;unsigned int size,cursor;
 const char* strings;unsigned int stringBytes;
 bool Span(unsigned int n){return cursor<=size&&n<=size-cursor;}
 unsigned int U(){unsigned int v=*(const unsigned int*)(bytes+cursor);cursor+=4;return v;}
 const char* String(unsigned int offset){if(offset>=stringBytes)return 0;for(unsigned int i=offset;i<stringBytes;i++)if(!strings[i])return strings+offset;return 0;}
 bool Fail(const char* message){error=message;groups=0;count=0;return false;}
public:
 Group* groups;unsigned int count,keyCount,propertyCount,effectVersion,typeCount;const char* error;
 Reader():bytes(0),size(0),cursor(0),strings(0),stringBytes(0),groups(0),count(0),keyCount(0),propertyCount(0),effectVersion(0),typeCount(0),error(0){}
 bool Read(const unsigned char* input,unsigned int length){
  memory.Clear();bytes=input;size=length;cursor=0;groups=0;count=keyCount=propertyCount=0;error=0;
  if(!bytes||!Span(40))return Fail("Truncated effect header");
  if(U()!=0x5846544c||U()!=2)return Fail("Unsupported effect format");
  effectVersion=U();stringBytes=U();unsigned int curveBytes=U(),expectedKeys=U();count=U();typeCount=U();unsigned int stringOffset=U(),listOffset=U();
  if(effectVersion!=5||typeCount!=18)return Fail("Unsupported effect schema");
  if(count>100000||expectedKeys>100000||stringBytes>16*1024*1024||curveBytes>16*1024*1024||!Span(typeCount*4))return Fail("Invalid effect counts");
  unsigned int typeCounts[18]={},actualTypes[18]={};unsigned int sum=0;
  for(unsigned int i=0;i<typeCount;i++){typeCounts[i]=U();if(typeCounts[i]>expectedKeys-sum)return Fail("Invalid type key counts");sum+=typeCounts[i];}
  if(sum!=expectedKeys||cursor!=stringOffset||!Span(stringBytes))return Fail("Invalid string table");
  strings=(const char*)(bytes+cursor);cursor+=stringBytes;if(!Span(curveBytes))return Fail("Truncated curve block");cursor+=curveBytes;
  groups=(Group*)memory.Allocate((count+1)*sizeof(Group));if(!groups)return Fail("Allocation failed");
  for(unsigned int g=0;g<count;g++){if(!Span(12))return Fail("Truncated group");Group& group=groups[g];group.name=String(U());group.milliseconds=U();group.count=U();if(!group.name||group.count>expectedKeys-keyCount)return Fail("Invalid group");
   group.keys=(Key*)memory.Allocate((group.count+1)*sizeof(Key));if(!group.keys)return Fail("Allocation failed");
   for(unsigned int k=0;k<group.count;k++){if(!Span(24))return Fail("Truncated key");Key& key=group.keys[k];key.type=U();unsigned int start=U(),end=U();key.start=*(float*)&start;key.end=*(float*)&end;key.id=U();key.linkedId=U();key.count=U();
    if(key.type>=typeCount||key.count>4096||key.count>1000000-propertyCount)return Fail("Invalid key");actualTypes[key.type]++;keyCount++;propertyCount+=key.count;
    key.properties=(Property*)memory.Allocate((key.count+1)*sizeof(Property));if(!key.properties)return Fail("Allocation failed");
    for(unsigned int p=0;p<key.count;p++){if(!Span(8))return Fail("Truncated property");Property& prop=key.properties[p];prop.name=String(U());unsigned int next=U();if(!prop.name||next<cursor||next>listOffset||next>size)return Fail("Invalid property boundary");prop.data=bytes+cursor;prop.size=next-cursor;cursor=next;}
   }
  }
  if(keyCount!=expectedKeys||cursor!=listOffset||!Span(count*4)||size-cursor!=count*4)return Fail("Invalid effect list");
  for(unsigned int g=0;g<count;g++){const char* listed=String(U());if(!listed||!FearBrushFlags::Equal(listed,groups[g].name))return Fail("Effect list mismatch");}
  for(unsigned int i=0;i<typeCount;i++)if(typeCounts[i]!=actualTypes[i])return Fail("Type key count mismatch");return true;
 }
 // Only callers that know the SDK property type may interpret this as a string.
 const char* StringValue(const Property& property){return property.size==4?String(*(const unsigned int*)property.data):0;}
};
}
