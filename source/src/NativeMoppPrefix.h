#pragma once
#include "NativeMoppCompiler.h"
// The physics mesh prefix (one mesh: count, transform+type, parameters, counts,
// float xyz vertices, uint16 triangle indices; mopp-backend mesh_counts layout)
// compiled by the native MOPP compiler into the blob the physics writer embeds:
// CodeInfo (origin xyz, scale), u32 code length, code.
namespace NativeMoppPrefix {
inline bool Finite(float v){unsigned int b;memcpy(&b,&v,4);return (b&0x7F800000u)!=0x7F800000u;}
// Validates the prefix and compiles it; on success out holds 20+length bytes
// allocated with new[] (caller deletes).
inline const char* Generate(const unsigned char* mesh,unsigned int bytes,unsigned char*& out,unsigned int& outBytes){
 out=0;outBytes=0;
 if(!mesh||bytes<108||bytes>16u*1024u*1024u)return "Mesh prefix size outside supported limits";
 unsigned int count,kind,points,indices;memcpy(&count,mesh,4);memcpy(&kind,mesh+32,4);memcpy(&points,mesh+100,4);memcpy(&indices,mesh+104,4);
 const bool wide=points>65535; /* CONDEMNED FORK: 32-bit indices above 65535 vertices */ if(count!=1||kind!=1||!points||points>1000000||!indices||indices>3000000||indices%3||bytes!=108+points*12+indices*(wide?4:2))return "Expected one mesh with uint16 (uint32 above 65535 vertices) triangle indices";
 for(unsigned int i=0;i<7+16;i++){float v;memcpy(&v,mesh+(i<7?4+4*i:36+4*(i-7)),4);if(!Finite(v))return "Non-finite mesh input";}
 const float* vertices=(const float*)(mesh+108);const unsigned short* tri=(const unsigned short*)(mesh+108+points*12);const unsigned int* tri32=(const unsigned int*)(mesh+108+points*12);
 for(unsigned int i=0;i<points*3;i++)if(!Finite(vertices[i]))return "Non-finite mesh input";
 for(unsigned int i=0;i<indices;i++)if((wide?tri32[i]:tri[i])>=points)return "Triangle index outside mesh";
 NativeMopp::Mesh m;m.vertices=vertices;m.indices=wide?0:tri;m.indices32=wide?tri32:0;m.triangles=indices/3;m.radius=NativeMopp::FromBits(0x3CF5C28Fu); // packer radius (0x1003C95F)
 NativeMopp::Result r;if(!NativeMopp::Compile(m,r))return r.error?r.error:"MOPP compilation failed";
 for(int i=0;i<4;i++)if(!Finite(r.info[i]))return "Invalid generated CodeInfo";if(!(r.info[3]>0))return "Invalid generated CodeInfo";
 outBytes=20+r.size;out=new unsigned char[outBytes];memcpy(out,r.info,16);memcpy(out+16,&r.size,4);memcpy(out+20,r.code,r.size);return 0;
}
}
