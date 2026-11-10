#pragma once
// CAI::GetPrefetchResourceList: model, eight child models, four arsenal slots,
// then localized sounds. Input consists of 13 consecutive NUL-terminated fields.
class AiPrefetchResources {
 MaterialReader* reader;const char** output;unsigned int count;
 bool Add(const char* value){if(!value||!value[0])return true;if(count>=100000)return false;unsigned int n=lstrlenA(value);if(n>4096)return false;char* copy=(char*)reader->combined.Allocate(n+1);if(!copy)return false;lstrcpyA(copy,value);output[count++]=copy;return true;}
 bool Record(HRECORD record,bool fx){if(!record)return true;const char** files=0;unsigned int n=0;if(!reader->sounds.ResolveRecord(record,files,n,true,fx?DatabaseEffectResources:0,reader))return false;for(unsigned int i=0;i<n;i++)if(!Add(files[i]))return false;return true;}
 const char* Field(const char*& fieldPosition,unsigned int capacity,char* buffer){unsigned int length=lstrlenA(fieldPosition),n=length<capacity?length:capacity-1;for(unsigned int i=0;i<n;i++)buffer[i]=fieldPosition[i];buffer[n]=0;fieldPosition+=length+1;return buffer;}
public:
 bool Build(MaterialReader& context,const char* packed,const char**& paths,unsigned int& total,bool& found,bool fx){
  reader=&context;reader->combined.Clear();count=total=0;paths=0;found=true;
  const char* fieldPosition=packed;char value[MAX_PATH];Field(fieldPosition,256,value);if(!value[0])return true;
  output=(const char**)reader->combined.Allocate(100000*sizeof(char*));if(!output)return false;
  HRECORD model=reader->sounds.Record("Character/Models",value);if(!Record(model,fx))return false;
  for(unsigned int i=0;i<8;i++)if(!Add(Field(fieldPosition,MAX_PATH,value)))return false;
  for(unsigned int i=0;i<4;i++){Field(fieldPosition,MAX_PATH,value);if(value[0]&&!FearBrushFlags::Equal(value,"<none>")&&!Record(reader->sounds.Record("Arsenal/Weapons",value),fx))return false;}
  HRECORD sounds=reader->sounds.Link(model,"SoundTemplate");
  if(sounds){
   if(!reader->voices.Initialize(reader->project))return false;
   static const char* types[]={
#define AISOUND_TYPE_AS_STRING 1
#include "E:/fearsdk/Source/Game/ObjectDLL/AISoundTypeEnums.h"
#undef ADD_AISOUND_TYPE
#undef AISOUND_TYPE_AS_STRING
   };
   for(unsigned int i=0;i<sizeof(types)/sizeof(types[0]);i++){
    unsigned int sets=reader->sounds.ValueCount(sounds,types[i]);if(sets>4096)return false;
    for(unsigned int j=0;j<sets;j++){HRECORD set=reader->sounds.Link(sounds,types[i],j);unsigned int voices=reader->sounds.ValueCount(set,"SoundFile");if(voices>4096)return false;
     for(unsigned int v=0;v<voices;v++){const char* id=reader->sounds.String(set,"SoundFile",v);if(id[0]&&!Add(reader->voices.Path(id)))return false;}
    }
   }
  }paths=output;total=count;return true;
 }
};
