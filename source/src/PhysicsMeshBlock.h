#pragma once
#include "PhysicsMeshPrefix.h"
namespace PhysicsMeshBlock {
class Block {
 bool Fail(const char* message){memory.Clear();data=0;size=0;error=message;return false;}
public:
 PrefabScene::Arena memory;unsigned char* data;unsigned int size;const char* error;
 Block():data(0),size(0),error(0){}
 bool Build(const PhysicsMeshPrefix::Block& mesh,const void* mopp,unsigned int bytes){
  memory.Clear();data=0;size=0;error=0;
  if(!mesh.data||mesh.size<108||!mopp||bytes<21||bytes>16*1024*1024)return Fail("Missing or oversized physics input");
  const unsigned int* header=(const unsigned int*)mesh.data;
  unsigned int points=header[25],indices=header[26];
  if(header[0]!=1||header[8]!=1||!points||points>1000000||!indices||indices%3||indices>3000000||mesh.size!=108+12*points+(points>65535?4:2)*(unsigned long long)indices)return Fail("Unsupported mesh prefix");
  const float* transform=(const float*)(mesh.data+4);for(unsigned int i=0;i<7;i++)if(!WorldText::Finite(transform[i]))return Fail("Non-finite transform");
  const float* parameters=(const float*)(mesh.data+36);for(unsigned int i=0;i<16;i++)if(!WorldText::Finite(parameters[i]))return Fail("Non-finite mesh parameter");
  const WorldText::Vec3* vertices=(const WorldText::Vec3*)(mesh.data+108);for(unsigned int i=0;i<points;i++)if(!WorldText::FiniteVector(vertices[i]))return Fail("Non-finite mesh vertex");
  const unsigned short* refs=(const unsigned short*)(mesh.data+108+12*points);const unsigned int* refs32=(const unsigned int*)(mesh.data+108+12*points);for(unsigned int i=0;i<indices;i++)if((points>65535?refs32[i]:refs[i])>=points)return Fail("Mesh index out of bounds");
  const unsigned char* code=(const unsigned char*)mopp;float info[4];unsigned int length;
  unsigned char* infoBytes=(unsigned char*)info;for(unsigned int i=0;i<16;i++)infoBytes[i]=code[i];
  unsigned char* lengthBytes=(unsigned char*)&length;for(unsigned int i=0;i<4;i++)lengthBytes[i]=code[16+i];
  for(unsigned int i=0;i<4;i++)if(!WorldText::Finite(info[i]))return Fail("Non-finite MOPP CodeInfo");
  if(info[3]<=0||!length||length!=bytes-20)return Fail("Invalid MOPP scale or length");
  if(mesh.size>16*1024*1024-bytes)return Fail("Physics block exceeds limit");
  size=mesh.size+bytes;data=(unsigned char*)memory.Allocate(size);if(!data)return Fail("Physics allocation failed");
  for(unsigned int i=0;i<mesh.size;i++)data[i]=mesh.data[i];
  for(unsigned int i=0;i<bytes;i++)data[mesh.size+i]=code[i];
  return true;
 }
};
// Complete single static mesh container (count+shape+MOPP). Other shapes unsupported.
// Caller must supply MOPP generated for this mesh; size checks alone cannot prove that.
}
