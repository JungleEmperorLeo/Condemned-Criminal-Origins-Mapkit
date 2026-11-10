#pragma once
namespace SourceRenderPlaneKey {
struct Key {int component[3];};
// Original RVA 0x6A50: x87 float loads, wide multiply/add, truncation toward
// zero, then clamp. In particular -1 maps to -499 rather than -500.
inline int Component(float input){
 double wide=double(input)*500.0+0.5;int value;
#if defined(_MSC_VER) && defined(_M_IX86)
 unsigned short control,truncation;
 __asm {fnstcw control}
 truncation=(unsigned short)(control|0x0c00);
 __asm {fldcw truncation}
 __asm {fld wide}
 __asm {fistp value}
 __asm {fldcw control}
#else
 value=(int)wide;
#endif
 return value< -500? -500:(value>500?500:value);
}
inline bool Build(const WorldText::Vec3& normal,Key& output){
 if(!WorldText::FiniteVector(normal))return false;
 // Actual normals lie in [-1,1]. Keep conversion defined on extra clamp
 // controls while rejecting huge finite values outside signed-int range.
 const float* values=&normal.x;Key key;
 for(unsigned int i=0;i<3;i++){if(values[i]< -4000000.0f||values[i]>4000000.0f)return false;key.component[i]=Component(values[i]);}
 output=key;return true;
}
inline bool Equal(const Key& a,const Key& b){return a.component[0]==b.component[0]&&a.component[1]==b.component[1]&&a.component[2]==b.component[2];}
}
