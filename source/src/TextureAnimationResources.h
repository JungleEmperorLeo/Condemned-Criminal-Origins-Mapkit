#pragma once
namespace TextureAnimationResources {
class Animation {
 PrefabScene::Arena memory;
 bool Fail(const char* message){frames=0;count=0;error=message;return false;}
public:
 const char** frames;unsigned int count;const char* error;
 Animation():frames(0),count(0),error(0){}
 bool Read(const unsigned char* bytes,unsigned int size){
  memory.Clear();frames=0;count=0;error=0;
  if(!bytes||size<20||size>16*1024*1024)return Fail("Invalid texture animation input");
  const unsigned int* h=(const unsigned int*)bytes;if(h[0]!=0x4154544c||h[1]!=1)return Fail("Unsupported texture animation header");
  unsigned int pool=h[3],n=h[4];if(n>100000||pool>size-20||n>(size-20-pool)/4||20+pool+n*4!=size)return Fail("Invalid texture animation tables");
  frames=(const char**)memory.Allocate((n+1)*sizeof(char*));if(!frames)return Fail("Allocation failed");
  for(unsigned int i=0;i<n;i++){
   unsigned int offset=*(const unsigned int*)(bytes+20+pool+i*4);if(offset>=pool)return Fail("Texture frame offset outside pool");unsigned int length=0;
   while(offset+length<pool&&bytes[20+offset+length]){if(length>=4096)return Fail("Texture frame path too long");length++;}
   if(!length||offset+length>=pool)return Fail("Invalid texture frame path");char* path=(char*)memory.Allocate(length+1);if(!path)return Fail("Allocation failed");
   for(unsigned int j=0;j<=length;j++)path[j]=(char)bytes[20+offset+j];frames[count++]=path;
  }return true;
 }
};
}
