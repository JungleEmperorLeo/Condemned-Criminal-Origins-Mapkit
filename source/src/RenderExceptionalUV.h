#pragma once
namespace RenderExceptionalUV {
// Source mapping preparation can retain authored quiet-NaN origins. The
// original UV helper transports their NaN payloads into FLOAT texture fields.
// Geometry/frame vectors and infinities are never accepted by this policy.
inline bool QuietNaN(float value){unsigned int bits;memcpy(&bits,&value,4);return (bits&0x7fc00000u)==0x7fc00000u;}
inline bool Allowed(float value,bool enabled){return WorldText::Finite(value)||(enabled&&QuietNaN(value));}
inline bool Origin(const WorldText::Vec3& value,bool enabled){return Allowed(value.x,enabled)&&Allowed(value.y,enabled)&&Allowed(value.z,enabled);}
inline bool Enabled(){char option[4]={};return GetEnvironmentVariableA("FEAR_SOURCE_EXCEPTIONAL_UV",option,4)==1&&option[0]=='1';}
}
