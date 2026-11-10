#pragma once
// Confirmed white-RGB material controls only; not a universal shader policy.
namespace SourceRenderColor {
inline bool PackWhiteRGB(float alpha,unsigned int& color){
 if(!WorldText::Finite(alpha)||alpha<0||alpha>255)return false;
 unsigned int packedAlpha=0;double rounded=double(alpha)+0.5;
 unsigned short saved=0,truncation=0;
 __asm {fnstcw saved}
 truncation=(unsigned short)(saved|0x0c00);
 __asm {fldcw truncation}
 __asm {fld rounded}
 __asm {fistp packedAlpha}
 __asm {fldcw saved}
 color=0x00ffffff|(packedAlpha<<24);return true;
}
inline void AuthoredWhiteRGB(const WorldText::PointColor& source,float* rgba){
 rgba[0]=rgba[1]=rgba[2]=255;rgba[3]=(float)source.a;
}
}

