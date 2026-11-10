#pragma once
// Observed FEAR LTMI v1: 1 texture, 2 vector3, 3 vector4, 4 integer, 5 float.
// Unknown versions/types fail rather than guessing their payload size.
namespace MaterialResources {
class Material {
 const unsigned char* input;unsigned int length,cursor;
 bool Take(unsigned int bytes,const unsigned char** value){if(bytes>length-cursor)return false;*value=input+cursor;cursor+=bytes;return true;}
 bool U32(unsigned int* value){const unsigned char* p;if(!Take(4,&p))return false;*value=*(const unsigned int*)p;return true;}
 bool String(const char** value){const unsigned char* p;if(!Take(2,&p))return false;unsigned int n=*(const unsigned short*)p;if(n>4096||!Take(n,&p))return false;char* s=(char*)memory.Allocate(n+1);if(!s)return false;for(unsigned int i=0;i<n;i++){if(!p[i])return false;s[i]=(char)p[i];}s[n]=0;*value=s;return true;}
 bool Fail(const char* message){shader=0;textures=textureNames=0;textureCount=0;error=message;return false;}
public:
 PrefabScene::Arena memory;const char* shader;const char** textures;const char** textureNames;unsigned int textureCount,surfaceFlags;const char* error;
 Material():input(0),length(0),cursor(0),shader(0),textures(0),textureNames(0),textureCount(0),surfaceFlags(0),error(0){}
 bool Read(const unsigned char* bytes,unsigned int size){
  memory.Clear();input=bytes;length=size;cursor=0;shader=0;textures=textureNames=0;textureCount=0;surfaceFlags=0;error=0;
  if(!bytes||size<12||size>16*1024*1024)return Fail("Invalid material input");
  unsigned int magic,version,count;if(!U32(&magic)||magic!=0x494d544c||!U32(&version)||version!=1)return Fail("Unsupported material header");
  if(!String(&shader)||!U32(&count)||count>4096)return Fail("Invalid material parameters");
  textures=(const char**)memory.Allocate((count+1)*sizeof(char*));textureNames=(const char**)memory.Allocate((count+1)*sizeof(char*));if(!textures||!textureNames)return Fail("Material allocation failed");
  for(unsigned int i=0;i<count;i++){
   unsigned int type;const char* name;if(!U32(&type)||!String(&name))return Fail("Truncated material parameter");
   if(type==1){const char* path;if(!String(&path))return Fail("Truncated texture path");if(path[0]){textureNames[textureCount]=name;textures[textureCount++]=path;}}
   else {const unsigned char* value;unsigned int n=type==2?12:(type==3?16:(type==4||type==5?4:0));if(!n)return Fail("Unsupported material parameter type");if(!Take(n,&value))return Fail("Truncated material value");if(type==4&&FearBrushFlags::Equal(name,"SurfaceFlags"))surfaceFlags=*(const unsigned int*)value;}
  }
  if(cursor!=size)return Fail("Unexpected trailing material data");return true;
 }
};
}
