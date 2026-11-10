#pragma once
// CMaterialInfo parameter descriptor classifier, original RVA 0x7B1C0.
namespace ShaderInstanceParameter {
enum Kind {Unsupported=0,Texture=1,Float3=2,Float4=3,Integer=4,ScalarFloat=5,Boolean=6,Matrix4=7};
struct Descriptor {unsigned int name,semantic,klass,type,rows,columns,elements,annotations,members,flags,bytes;};
static_assert(sizeof(Descriptor)==44,"D3DXPARAMETER_DESC x86");
inline Kind Classify(const Descriptor& p){
 if(p.klass==1&&p.type==3){unsigned int n=p.rows<p.columns?p.columns:p.rows;if(n==3)return Float3;if(n==4)return Float4;}
 if(p.klass==4&&p.type==5)return Texture;
 if(p.klass==2&&p.type==3&&p.rows==4&&p.columns==4)return Matrix4;
 if(p.klass==0){if(p.type==2)return Integer;if(p.type==3)return ScalarFloat;if(p.type==1)return Boolean;}
 return Unsupported;
}
}
