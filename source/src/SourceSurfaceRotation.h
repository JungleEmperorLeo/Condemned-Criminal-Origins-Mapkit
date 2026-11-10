#pragma once
namespace SourceSurfaceRotation {
// Original quaternion vector rotation RVA 067F0. Float intermediates follow
// the four explicit stores before the second quaternion product.
inline WorldText::Vec3 Rotate(const float* q,const WorldText::Vec3& v){
 volatile float tx,ty,tz,tw;
#if defined(_MSC_VER) && defined(_M_IX86)
 const WorldText::Vec3* input=&v;
 // Keep original operand loading and FCHS position, including NaN sign rules.
 __asm {
  mov ecx,q
  mov eax,input
  fld dword ptr [ecx+4]
  fmul dword ptr [eax+8]
  fld dword ptr [ecx+12]
  fmul dword ptr [eax]
  faddp st(1),st(0)
  fld dword ptr [ecx+8]
  fmul dword ptr [eax+4]
  fsubp st(1),st(0)
  fstp tx
  fld dword ptr [ecx+12]
  fmul dword ptr [eax+4]
  fld dword ptr [ecx]
  fmul dword ptr [eax+8]
  fsubp st(1),st(0)
  fld dword ptr [ecx+8]
  fmul dword ptr [eax]
  faddp st(1),st(0)
  fstp ty
  fld dword ptr [eax+4]
  fmul dword ptr [ecx]
  fld dword ptr [ecx+12]
  fmul dword ptr [eax+8]
  faddp st(1),st(0)
  fld dword ptr [ecx+4]
  fmul dword ptr [eax]
  fsubp st(1),st(0)
  fstp tz
  fld dword ptr [eax]
  fmul dword ptr [ecx]
  fchs
  fld dword ptr [eax+4]
  fmul dword ptr [ecx+4]
  fsubp st(1),st(0)
  fld dword ptr [ecx+8]
  fmul dword ptr [eax+8]
  fsubp st(1),st(0)
  fstp tw
 }
#else
 tx=(float)((double(q[1])*v.z+double(q[3])*v.x)-double(q[2])*v.y);
 ty=(float)((double(q[3])*v.y-double(q[0])*v.z)+double(q[2])*v.x);
 tz=(float)((double(v.y)*q[0]+double(q[3])*v.z)-double(q[1])*v.x);
 tw=(float)((-double(v.x)*q[0]-double(v.y)*q[1])-double(q[2])*v.z);
#endif
 volatile float ix=-q[0],iy=-q[1],iz=-q[2];
 WorldText::Vec3 result;
#if defined(_MSC_VER) && defined(_M_IX86)
 float qw=q[3];WorldText::Vec3* output=&result;
 __asm {
  mov eax,output
  fld ix
  fmul tw
  fld iz
  fmul ty
  faddp st(1),st(0)
  fld qw
  fmul tx
  faddp st(1),st(0)
  fld iy
  fmul tz
  fsubp st(1),st(0)
  fstp dword ptr [eax]
  fld iy
  fmul tw
  fld iz
  fmul tx
  fsubp st(1),st(0)
  fld qw
  fmul ty
  faddp st(1),st(0)
  fld ix
  fmul tz
  faddp st(1),st(0)
  fstp dword ptr [eax+4]
  fld iz
  fmul tw
  fld iy
  fmul tx
  faddp st(1),st(0)
  fld ix
  fmul ty
  fsubp st(1),st(0)
  fld qw
  fmul tz
  faddp st(1),st(0)
  fstp dword ptr [eax+8]
 }
#else
 result.x=(float)(((double(ix)*tw+double(iz)*ty)+double(q[3])*tx)-double(iy)*tz);
 result.y=(float)(((double(iy)*tw-double(iz)*tx)+double(q[3])*ty)+double(ix)*tz);
 result.z=(float)(((double(iz)*tw+double(iy)*tx)-double(ix)*ty)+double(q[3])*tz);
#endif
 return result;
}
inline WorldText::Vec3 Identity(const WorldText::Vec3& v){const float q[4]={0,0,0,1};return Rotate(q,v);}
}
