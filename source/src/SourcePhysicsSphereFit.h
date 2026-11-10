#pragma once
// Native scalar move-to-front sphere fit recovered from the original helper.
// The support arithmetic retains the observed x87 float stores, including the
// asymmetric intermediate rounding in the three/four-point circumcenters.
// No original DLL, callback, file, mesh archive, or runtime address is consumed.
namespace SourcePhysicsSphereFit {
struct Vec4 {float x,y,z,w;};
inline bool Finite(float value){union Bits {float f;unsigned int u;};Bits b;b.f=value;return (b.u&0x7f800000u)!=0x7f800000u;}
#if defined(_MSC_VER) && defined(_M_IX86)
inline void Support2(float* output,const Vec4* p0,const Vec4* p1,float epsilon){
 float scratch[16];const float half=.5f;
 __asm {
   mov eax,output
   mov ecx,p0
   mov edx,p1
   fld         dword ptr [edx]
   fsub        dword ptr [ecx]
   fld         dword ptr [edx+4]
   fsub        dword ptr [ecx+4]
   fld         dword ptr [edx+8]
   fsub        dword ptr [ecx+8]
   fstp        dword ptr scratch[8]
   fld         dword ptr [edx+0Ch]
   fsub        dword ptr [ecx+0Ch]
   fstp        dword ptr scratch[0Ch]
   fxch        st(1)
   fmul        dword ptr half
   fstp        dword ptr scratch[10h]
   fmul        dword ptr half
   fld         dword ptr scratch[8]
   fmul        dword ptr half
   fld         dword ptr scratch[0Ch]
   fmul        dword ptr half
   fld         dword ptr scratch[10h]
   fadd        dword ptr [ecx]
   fld         st(3)
   fadd        dword ptr [ecx+4]
   fld         st(3)
   fadd        dword ptr [ecx+8]
   fstp        dword ptr scratch[8]
   fxch        st(2)
   fadd        dword ptr [ecx+0Ch]
   mov         ecx,dword ptr scratch[8]
   mov         dword ptr [eax+8],ecx
   fstp        dword ptr scratch[0Ch]
   mov         edx,dword ptr scratch[0Ch]
   mov         dword ptr [eax+0Ch],edx
   fstp        dword ptr [eax]
   fstp        dword ptr [eax+4]
   fld         st(0)
   fmul        st,st(1)
   fld         st(2)
   fmul        st,st(3)
   faddp       st(1),st
   fld         dword ptr scratch[10h]
   fmul        dword ptr scratch[10h]
   faddp       st(1),st
   fsqrt
   fstp        st(2)
   fstp        st(0)
   fadd        dword ptr epsilon
   fstp        dword ptr [eax+0Ch]
 }
}
inline void Support3(float* output,const Vec4* p0,const Vec4* p1,const Vec4* p2,float epsilon){
 float scratch[16];const float half=.5f;const float zero=0;
 __asm {
   mov eax,output
   mov ecx,p0
   mov esi,p1
   mov edx,p2
   fld         dword ptr [esi]
   fsub        dword ptr [ecx]
   fld         dword ptr [esi+4]
   fsub        dword ptr [ecx+4]
   fld         dword ptr [esi+8]
   fsub        dword ptr [ecx+8]
   fld         dword ptr [edx]
   fsub        dword ptr [ecx]
   fld         dword ptr [edx+4]
   fsub        dword ptr [ecx+4]
   fld         dword ptr [edx+8]
   fsub        dword ptr [ecx+8]
   fld         st(0)
   fmul        st,st(5)
   fld         st(2)
   fmul        st,st(5)
   fsubp       st(1),st
   fstp        dword ptr scratch[10h]
   fld         st(3)
   fmul        st,st(3)
   fld         st(1)
   fmul        st,st(7)
   fsubp       st(1),st
   fstp        dword ptr scratch[14h]
   fld         st(1)
   fmul        st,st(6)
   fld         st(5)
   fmul        st,st(4)
   fsubp       st(1),st
   fstp        dword ptr scratch[18h]
   fld         dword ptr scratch[14h]
   fmul        st,st(4)
   fld         dword ptr scratch[18h]
   fmul        st,st(6)
   fsubp       st(1),st
   fstp        dword ptr scratch[30h]
   fld         dword ptr scratch[18h]
   fmul        st,st(6)
   fld         st(4)
   fmul        dword ptr scratch[10h]
   fsubp       st(1),st
   fstp        dword ptr scratch[34h]
   fld         st(4)
   fmul        dword ptr scratch[10h]
   fld         dword ptr scratch[14h]
   fmul        st,st(7)
   fsubp       st(1),st
   fstp        dword ptr scratch[38h]
   fld         st(0)
   fmul        st,st(1)
   fld         st(2)
   fmul        st,st(3)
   faddp       st(1),st
   fld         st(3)
   fmul        st,st(4)
   faddp       st(1),st
   fstp        dword ptr scratch[0Ch]
   fld         dword ptr scratch[30h]
   fmul        dword ptr scratch[0Ch]
   fstp        dword ptr scratch[20h]
   fld         dword ptr scratch[34h]
   fmul        dword ptr scratch[0Ch]
   fstp        dword ptr scratch[24h]
   fld         dword ptr scratch[38h]
   fmul        dword ptr scratch[0Ch]
   fstp        dword ptr scratch[28h]
   fld         dword ptr scratch[0Ch]
   fmul        dword ptr zero
   fstp        dword ptr scratch[2Ch]
   fld         dword ptr scratch[18h]
   fmul        st,st(2)
   fld         dword ptr scratch[14h]
   fmul        st,st(2)
   fsubp       st(1),st
   fstp        dword ptr scratch[30h]
   fmul        dword ptr scratch[10h]
   fld         dword ptr scratch[18h]
   fmul        st,st(3)
   mov         edx,dword ptr [ecx]
   fsubp       st(1),st
   fstp        dword ptr scratch[34h]
   fld         dword ptr scratch[14h]
   fmul        st,st(2)
   fxch        st(1)
   fmul        dword ptr scratch[10h]
   fsubp       st(1),st
   fstp        dword ptr scratch[38h]
   fstp        st(0)
   fld         st(0)
   fmul        st,st(1)
   fld         st(2)
   fmul        st,st(3)
   faddp       st(1),st
   fld         st(3)
   fmul        st,st(4)
   faddp       st(1),st
   fstp        st(3)
   fstp        st(0)
   fstp        st(0)
   fld         dword ptr scratch[30h]
   mov         dword ptr scratch[30h],edx
   fmul        st,st(1)
   mov         edx,dword ptr [ecx+4]
   fadd        dword ptr scratch[20h]
   fld         dword ptr scratch[34h]
   mov         dword ptr scratch[34h],edx
   fmul        st,st(2)
   mov         edx,dword ptr [ecx+8]
   mov         ecx,dword ptr [ecx+0Ch]
   mov         dword ptr scratch[3Ch],ecx
   fadd        dword ptr scratch[24h]
   fstp        dword ptr scratch[24h]
   fld         dword ptr scratch[38h]
   mov         dword ptr scratch[38h],edx
   fmul        st,st(2)
   fadd        dword ptr scratch[28h]
   fstp        dword ptr scratch[28h]
   fxch        st(1)
   fmul        dword ptr zero
   fadd        dword ptr scratch[2Ch]
   fstp        dword ptr scratch[2Ch]
   fld         dword ptr scratch[18h]
   fmul        dword ptr scratch[18h]
   fld         dword ptr scratch[14h]
   fmul        dword ptr scratch[14h]
   faddp       st(1),st
   fld         dword ptr scratch[10h]
   fmul        dword ptr scratch[10h]
   faddp       st(1),st
   fdivr       dword ptr half
   fst         dword ptr scratch[0Ch]
   fmulp       st(1),st
   fld         dword ptr scratch[24h]
   fmul        dword ptr scratch[0Ch]
   fld         dword ptr scratch[28h]
   fmul        dword ptr scratch[0Ch]
   fld         dword ptr scratch[2Ch]
   fmul        dword ptr scratch[0Ch]
   fld         dword ptr scratch[30h]
   fadd        st,st(4)
   fld         dword ptr scratch[34h]
   fadd        st,st(4)
   fld         dword ptr scratch[38h]
   fadd        st,st(4)
   fstp        dword ptr scratch[38h]
   fld         dword ptr scratch[3Ch]
   fadd        st,st(3)
   fstp        dword ptr scratch[3Ch]
   fxch        st(1)
   fstp        dword ptr [eax]
   fstp        dword ptr [eax+4]
   mov         edx,dword ptr scratch[38h]
   mov         ecx,dword ptr scratch[3Ch]
   fstp        st(0)
   mov         dword ptr [eax+8],edx
   fld         st(0)
   mov         dword ptr [eax+0Ch],ecx
   fmul        st,st(1)
   fld         st(2)
   fmul        st,st(3)
   faddp       st(1),st
   fld         st(3)
   fmul        st,st(4)
   faddp       st(1),st
   fsqrt
   fstp        st(3)
   fstp        st(0)
   fstp        st(0)
   fadd        dword ptr epsilon
   fstp        dword ptr [eax+0Ch]
 }
}
inline void Support4(float* output,const Vec4* p0,const Vec4* p1,const Vec4* p2,const Vec4* p3,float epsilon){
 float scratch[16];const float half=.5f;const float zero=0;
 __asm {
   mov eax,output
   mov ecx,p0
   mov edi,p1
   mov esi,p2
   mov edx,p3
   fld         dword ptr [edi]
   fsub        dword ptr [ecx]
   fld         dword ptr [edi+4]
   fsub        dword ptr [ecx+4]
   fld         dword ptr [edi+8]
   fsub        dword ptr [ecx+8]
   fld         dword ptr [esi]
   fsub        dword ptr [ecx]
   fld         dword ptr [esi+4]
   fsub        dword ptr [ecx+4]
   fld         dword ptr [esi+8]
   fsub        dword ptr [ecx+8]
   fld         dword ptr [edx]
   fsub        dword ptr [ecx]
   fstp        dword ptr scratch[20h]
   fld         dword ptr [edx+4]
   fsub        dword ptr [ecx+4]
   fstp        dword ptr scratch[24h]
   fld         dword ptr [edx+8]
   fsub        dword ptr [ecx+8]
   fstp        dword ptr scratch[28h]
   fld         st(0)
   fmul        st,st(5)
   fld         st(2)
   fmul        st,st(5)
   fsubp       st(1),st
   fstp        dword ptr scratch[30h]
   fld         st(3)
   fmul        st,st(3)
   fld         st(1)
   fmul        st,st(7)
   fsubp       st(1),st
   fstp        dword ptr scratch[34h]
   fld         st(1)
   fmul        st,st(6)
   fld         st(5)
   fmul        st,st(4)
   fsubp       st(1),st
   fstp        dword ptr scratch[38h]
   fld         dword ptr scratch[28h]
   fmul        dword ptr scratch[28h]
   fld         dword ptr scratch[24h]
   fmul        dword ptr scratch[24h]
   faddp       st(1),st
   fld         dword ptr scratch[20h]
   fmul        dword ptr scratch[20h]
   faddp       st(1),st
   fstp        dword ptr scratch[4]
   fld         dword ptr scratch[30h]
   fmul        dword ptr scratch[4]
   fstp        dword ptr scratch[10h]
   fld         dword ptr scratch[34h]
   fmul        dword ptr scratch[4]
   fstp        dword ptr scratch[14h]
   fld         dword ptr scratch[38h]
   fmul        dword ptr scratch[4]
   fstp        dword ptr scratch[18h]
   fld         dword ptr scratch[4]
   fmul        dword ptr zero
   fstp        dword ptr scratch[1Ch]
   fld         dword ptr scratch[24h]
   fmul        st,st(4)
   fld         dword ptr scratch[28h]
   fmul        st,st(6)
   fsubp       st(1),st
   fstp        dword ptr scratch[30h]
   fld         dword ptr scratch[28h]
   fmul        st,st(6)
   fld         st(4)
   fmul        dword ptr scratch[20h]
   fsubp       st(1),st
   fstp        dword ptr scratch[34h]
   fld         st(4)
   fmul        dword ptr scratch[20h]
   fld         dword ptr scratch[24h]
   fmul        st,st(7)
   fsubp       st(1),st
   fstp        dword ptr scratch[38h]
   fld         st(0)
   fmul        st,st(1)
   fld         st(2)
   fmul        st,st(3)
   faddp       st(1),st
   fld         st(3)
   fmul        st,st(4)
   faddp       st(1),st
   fst         dword ptr scratch[4]
   fmul        dword ptr scratch[30h]
   fadd        dword ptr scratch[10h]
   fstp        dword ptr scratch[10h]
   fld         dword ptr scratch[34h]
   fmul        dword ptr scratch[4]
   fadd        dword ptr scratch[14h]
   fstp        dword ptr scratch[14h]
   fld         dword ptr scratch[38h]
   fmul        dword ptr scratch[4]
   fadd        dword ptr scratch[18h]
   fstp        dword ptr scratch[18h]
   fld         dword ptr scratch[4]
   fmul        dword ptr zero
   fadd        dword ptr scratch[1Ch]
   fstp        dword ptr scratch[1Ch]
   fld         dword ptr scratch[28h]
   fmul        st,st(2)
   fld         dword ptr scratch[24h]
   fmul        st,st(2)
   fsubp       st(1),st
   fstp        dword ptr scratch[4]
   mov         edx,dword ptr scratch[4]
   fmul        dword ptr scratch[20h]
   mov         dword ptr scratch[30h],edx
   fstp        dword ptr scratch[0Ch]
   fld         dword ptr scratch[28h]
   fmul        st,st(2)
   fstp        dword ptr scratch[8]
   fld         dword ptr scratch[0Ch]
   fsub        dword ptr scratch[8]
   fstp        dword ptr scratch[34h]
   fld         dword ptr scratch[24h]
   fmulp       st(2),st
   fmul        dword ptr scratch[20h]
   fsubp       st(1),st
   fld         st(0)
   fld         st(2)
   fmul        st,st(3)
   fld         st(4)
   fmul        st,st(5)
   faddp       st(1),st
   fld         st(5)
   fmul        st,st(6)
   faddp       st(1),st
   fld         dword ptr scratch[30h]
   fmul        st,st(1)
   fadd        dword ptr scratch[10h]
   fstp        dword ptr scratch[10h]
   fld         dword ptr scratch[34h]
   fmul        st,st(1)
   fadd        dword ptr scratch[14h]
   fstp        dword ptr scratch[14h]
   fxch        st(1)
   fmul        st,st(1)
   fadd        dword ptr scratch[18h]
   fstp        dword ptr scratch[18h]
   fmul        dword ptr zero
   fadd        dword ptr scratch[1Ch]
   fstp        dword ptr scratch[1Ch]
   fld         dword ptr scratch[4]
   fmulp       st(4),st
   fld         dword ptr scratch[8]
   fsub        dword ptr scratch[0Ch]
   fmul        st,st(3)
   fsubp       st(4),st
   fmul        st,st(1)
   faddp       st(3),st
   fxch        st(2)
   fdivr       dword ptr half
   fstp        st(2)
   fstp        st(0)
   fld         dword ptr scratch[10h]
   fmul        st,st(1)
   fstp        dword ptr scratch[10h]
   fld         dword ptr scratch[14h]
   fmul        st,st(1)
   fstp        dword ptr scratch[14h]
   fld         dword ptr scratch[18h]
   fmul        st,st(1)
   fstp        dword ptr scratch[18h]
   fmul        dword ptr scratch[1Ch]
   fld         dword ptr scratch[10h]
   fadd        dword ptr [ecx]
   fld         dword ptr scratch[14h]
   fadd        dword ptr [ecx+4]
   fld         dword ptr scratch[18h]
   fadd        dword ptr [ecx+8]
   fstp        dword ptr scratch[38h]
   fxch        st(2)
   fadd        dword ptr [ecx+0Ch]
   mov         ecx,dword ptr scratch[38h]
   mov         dword ptr [eax+8],ecx
   fstp        dword ptr scratch[3Ch]
   mov         edx,dword ptr scratch[3Ch]
   mov         dword ptr [eax+0Ch],edx
   fstp        dword ptr [eax]
   fstp        dword ptr [eax+4]
   fld         dword ptr scratch[18h]
   fmul        dword ptr scratch[18h]
   fld         dword ptr scratch[14h]
   fmul        dword ptr scratch[14h]
   faddp       st(1),st
   fld         dword ptr scratch[10h]
   fmul        dword ptr scratch[10h]
   faddp       st(1),st
   fsqrt
   fadd        dword ptr epsilon
   fstp        dword ptr [eax+0Ch]
 }
}

inline bool Outside(const Vec4& point,const float* sphere){
 const Vec4* candidate=&point;bool outside=false;
 __asm {
  mov ecx,candidate
  mov edx,sphere
  fld dword ptr [edx+4]
  fld dword ptr [ecx]
  fsub dword ptr [edx]
  fld dword ptr [ecx+4]
  fsub st,st(2)
  fld dword ptr [ecx+8]
  fsub dword ptr [edx+8]
  fld st(0)
  fmul st,st(1)
  fld st(2)
  fmul st,st(3)
  faddp st(1),st
  fld st(3)
  fmul st,st(4)
  faddp st(1),st
  fsqrt
  fstp st(3)
  fstp st(0)
  fstp st(0)
  fcomp dword ptr [edx+12]
  fnstsw ax
  fstp st(0)
  test ah,41h
  setz al
  mov outside,al
 }
 return outside;
}
struct Work {
 unsigned long long tests,maxTests;bool exhausted;
 Work(unsigned long long limit):tests(0),maxTests(limit),exhausted(false){}
 bool Recurse(Vec4* points,unsigned int count,unsigned int support,float epsilon,float* sphere){
  if(support==4){Support4(sphere,points-1,points-2,points-3,points-4,epsilon);return true;}
  if(!support){sphere[0]=sphere[1]=sphere[2]=0;sphere[3]=-1;}
  else if(support==1){sphere[0]=points[-1].x;sphere[1]=points[-1].y;sphere[2]=points[-1].z;sphere[3]=epsilon;}
  else if(support==2)Support2(sphere,points-1,points-2,epsilon);
  else Support3(sphere,points-1,points-2,points-3,epsilon);
  for(unsigned int i=0;i<count;i++){
   if(tests==maxTests){exhausted=true;return false;}tests++;
   if(!Outside(points[i],sphere))continue;
   Vec4 candidate=points[i];for(unsigned int j=i;j;j--)points[j]=points[j-1];points[0]=candidate;
   if(!Recurse(points+1,i,support+1,epsilon,sphere))return false;
  }
  return true;
 }
};
// Empty output (-1 radius) is the original helper's sentinel. The automatic
// shape writer has a separate empty-list path and must not serialize it.
// Singular/nonfinite fit results are rejected by this bounded native interface;
// the original support routines themselves have no determinant fallback.
inline bool Fit(Vec4* points,unsigned int count,float epsilon,float* output,unsigned long long maxTests=50000000ull){
 if(!output)return false;output[0]=output[1]=output[2]=0;output[3]=-1;
 if((count&&!points)||count>100000||!Finite(epsilon)||epsilon<0||!maxTests)return false;
 for(unsigned int i=0;i<count;i++)if(!Finite(points[i].x)||!Finite(points[i].y)||!Finite(points[i].z))return false;
 Work work(maxTests);float fitted[4];if(!work.Recurse(points,count,0,epsilon,fitted))return false;
 for(unsigned int i=0;i<4;i++)if(!Finite(fitted[i]))return false;
 for(unsigned int i=0;i<4;i++)output[i]=fitted[i];return true;
}
#else
inline bool Fit(Vec4*,unsigned int,float,float* output,unsigned long long=50000000ull){if(output){output[0]=output[1]=output[2]=0;output[3]=-1;}return false;}
#endif
}
