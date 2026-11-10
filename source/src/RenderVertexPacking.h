#pragma once
#include "RenderAttributes.h"
namespace RenderVertexPacking {
struct Element {unsigned short stream,offset;unsigned char type,method,usage,index;};
static_assert(sizeof(Element)==8,"D3D declaration element");
static const Element Working[]={{0,0,2,0,0,0},{0,12,1,0,5,0},{0,20,1,0,5,1},{0,28,1,0,5,2},{0,36,1,0,5,3},{0,44,4,0,10,0},{0,48,2,0,3,0},{0,60,2,0,6,0},{0,72,2,0,7,0},{255,0,17,0,0,0}};
inline unsigned int Width(unsigned int type){return type<4?(type+1)*4:(type==4||type==5?4:0);}
inline bool Validate(const Element* declaration,unsigned int elements,unsigned int stride){
 if(!declaration||elements<1||elements>64||!stride||stride>256)return false;
 for(unsigned int i=0;i<elements;i++){
  const Element& e=declaration[i];if(i+1==elements){if(e.stream!=255||e.offset||e.type!=17||e.method||e.usage||e.index)return false;continue;}
  unsigned int width=Width(e.type);if(e.stream||e.method>6||e.usage>13||e.index>15||(e.usage==5&&e.index>8)||!width||e.offset+width>stride)return false;
 }return true;
}
inline unsigned int Stride(const Element* declaration,unsigned int elements){
 if(!declaration||elements<2||elements>64)return 0;unsigned int result=0;
 for(unsigned int i=0;i+1<elements;i++){unsigned int width=Width(declaration[i].type);if(!width)return 0;unsigned int end=declaration[i].offset+width;if(end>result)result=end;}
 return Validate(declaration,elements,result)?result:0;
}
inline float DecodeByte(unsigned int integer){float inverse=0.003921568859368562698f,result;__asm {
 fild integer
 fmul inverse
 fstp result
 }return result;}
inline int Truncate(float value,float scale){int result;unsigned short oldControl,newControl;__asm {fnstcw oldControl}newControl=(unsigned short)((oldControl&~0xC00)|0xC00);__asm {
 fldcw newControl
 fld value
 fmul scale
 fistp result
 fldcw oldControl
 }return result;}
inline int Find(const Element* source,unsigned int elements,const Element& destination){
 int best=-1,bestScore=0;for(unsigned int i=0;i+1<elements;i++)if(source[i].usage==destination.usage){int delta=(int)source[i].index-destination.index;if(delta<0)delta=-delta;int score=115-delta;if(score>bestScore){bestScore=score;best=(int)i;}}return best;
}
inline void Decode(const unsigned char* bytes,unsigned int type,float* value){
 value[0]=value[1]=value[2]=0;value[3]=1;
 if(type<4)memcpy(value,bytes,(type+1)*4);
 else if(type==4)for(unsigned int i=0;i<4;i++)value[i]=DecodeByte(bytes[i]);
 else for(unsigned int i=0;i<4;i++)value[i]=(float)bytes[i];
}
inline bool Encode(const float* value,unsigned int type,unsigned char* bytes){
 if(type<4){memcpy(bytes,value,(type+1)*4);return true;}
 float scale=type==4?255.f:1.f,bound=type==4?8000000.f:2000000000.f;
 for(unsigned int i=0;i<4;i++)if(!WorldText::Finite(value[i])||value[i]<-bound||value[i]>bound)return false;
 for(unsigned int i=0;i<4;i++)bytes[i]=(unsigned char)Truncate(value[i],scale);return true;
}
// Equivalent to 0x79480 for the supported declarations. Caller initializes
// destination storage, since the original leaves gaps/padding untouched.
// FLOAT1..4, D3DCOLOR and UBYTE4 are supported; other types fail explicitly.
inline bool Convert(unsigned int count,const Element* source,unsigned int sourceElements,const void* vertices,unsigned int sourceStride,const Element* destination,unsigned int destinationElements,void* output,unsigned int destinationStride){
 if(!vertices||!output||!count||count>10000||!Validate(source,sourceElements,sourceStride)||!Validate(destination,destinationElements,destinationStride))return false;
 int mappings[64];for(unsigned int i=0;i+1<destinationElements;i++)mappings[i]=Find(source,sourceElements,destination[i]);
 const unsigned char* input=(const unsigned char*)vertices;unsigned char* packed=(unsigned char*)output;
 for(unsigned int v=0;v<count;v++)for(unsigned int e=0;e+1<destinationElements;e++){
  float value[4]={0,0,0,1};int from=mappings[e];if(from>=0)Decode(input+v*sourceStride+source[from].offset,source[from].type,value);
  // The recovered missing-source branch omits the destination offset.
  // Preserve that original quirk; it can overwrite the first element.
  unsigned int offset=from>=0?destination[e].offset:0;
  if(!Encode(value,destination[e].type,packed+v*destinationStride+offset))return false;
 }return true;
}
}
