#pragma once
// Tangent selector recovered at WorldPacker RVA 0x6550..0x65B9.
// These bits belong to the internal render-surface selector, NOT brush surfaceFlags.
namespace BrushRenderSettings {
enum TangentMethod { World=2, Smooth=4, Kaldera=8, Unknown=0 };
inline TangentMethod ParseTangent(const char* value){
 if(!value||FearBrushFlags::Equal(value,"World"))return World;
 if(FearBrushFlags::Equal(value,"Smooth"))return Smooth;
 if(FearBrushFlags::Equal(value,"Kaldera"))return Kaldera;
 return Unknown;
}
inline bool KnownLOD(const char* value){return !value||FearBrushFlags::Equal(value,"Low")||FearBrushFlags::Equal(value,"Medium")||FearBrushFlags::Equal(value,"High")||FearBrushFlags::Equal(value,"Never");}
inline bool SupportedTangent(TangentMethod value){return value==World||value==Smooth||value==Kaldera;}
}
