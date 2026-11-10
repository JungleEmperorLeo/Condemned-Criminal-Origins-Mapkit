#pragma once
#include "RenderVertexPacking.h"
extern "C" __declspec(dllimport) HMODULE WINAPI LoadLibraryA(const char*);
extern "C" __declspec(dllimport) void* WINAPI GetProcAddress(HMODULE,const char*);
#pragma comment(linker,"/alternatename:__imp__LoadLibraryA@4=__imp__LoadLibraryA")
#pragma comment(linker,"/alternatename:__imp__GetProcAddress@8=__imp__GetProcAddress")
// Source shader declaration metadata. Primary reflection and parent merging
// match individual original helpers. This is not an automatic original shader
// loader-success decision: include handling, resource lookup and the complete
// shader/parameter/fallback dependency policy remain caller responsibilities.
namespace ShaderRenderDeclaration {
typedef RenderVertexPacking::Element Element;
struct Parameter {const char* name;const char* semantic;unsigned int klass,type,rows,columns,elements,annotations,members,flags,bytes;};
static_assert(sizeof(Parameter)==44,"D3DXPARAMETER_DESC x86");
static const Element End={255,0,17,0,0,0};
inline bool IsEnd(const Element& e){return e.stream==255&&!e.offset&&e.type==17&&!e.method&&!e.usage&&!e.index;}
inline unsigned char Lower(unsigned char c){return c>='A'&&c<='Z'?(unsigned char)(c+32):c;}
inline bool Prefix(const char* name,const char* prefix,unsigned int n){for(unsigned int i=0;i<n;i++)if(!name[i]||Lower((unsigned char)name[i])!=Lower((unsigned char)prefix[i]))return false;return true;}
inline bool Semantic(const char* name,unsigned int* usage,unsigned int* index,bool* clamped,bool* unsupported=0){
 if(unsupported)*unsupported=false;
 if(!name||!usage||!index||!clamped)return false;*clamped=false;
 static const char* prefixes[]={"PositionT","Position","BlendWeight","BlendIndices","Normal","PSize","TexCoord","Tangent","Binormal","TessFactor","Color","Fog","Depth","Sample"};
 static const unsigned char lengths[]={9,8,11,12,6,5,8,7,8,10,5,3,5,6};
 static const unsigned char usages[]={9,0,1,2,3,4,5,6,7,8,10,11,12,13};
 unsigned int p=0;for(;p<14;p++)if(Prefix(name,prefixes[p],lengths[p]))break;if(p==14)return false;*usage=usages[p];const char* suffix=name+lengths[p];
 while(*suffix==' '||*suffix=='\t'||*suffix=='\r'||*suffix=='\n'||*suffix=='\v'||*suffix=='\f')suffix++;bool negative=false;if(*suffix=='+'||*suffix=='-'){negative=*suffix=='-';suffix++;}
 unsigned int value=0;while(*suffix>='0'&&*suffix<='9'){if(value>214748364u||(value==214748364u&&(unsigned int)(*suffix-'0')>7)){if(unsupported)*unsupported=true;return false;}value=value*10+(unsigned int)(*suffix++-'0');}
 // Original atoi result is compared unsigned against 8. Its assertion path
 // resets out-of-range indices to 0; expose the warning without opening a UI.
 if((negative&&value)||value>8){*clamped=true;value=0;}*index=value;return true;
}
inline unsigned int Width(unsigned int type){
 if(type<4)return (type+1)*4;
 if(type==1||type==7||type==10||type==12||type==16)return 8;
 if(type==4||type==5||type==6||type==8||type==9||type==11||type==13||type==14||type==15)return 4;return 0;
}
inline bool DeclType(unsigned int components,unsigned int scalarBytes,unsigned int parameterType,unsigned int usage,unsigned int* type){
 if(!type||components<1||components>4)return false;
 if(usage==2){*type=5;return true;}if(usage==10){*type=4;return true;}
 if(parameterType==3){if(scalarBytes==4){*type=components-1;return true;}if(scalarBytes==2&&(components==2||components==4)){*type=components==2?15:16;return true;}return false;}
 if(parameterType==1||parameterType==2){if(scalarBytes==1){*type=5;return true;}if(scalarBytes==2&&(components==2||components==4)){*type=components==2?6:7;return true;}}
 return false;
}
inline bool Primary(void* effect,Element* output,unsigned int capacity,unsigned int* count,unsigned int* warnings,bool* boundExceeded=0){
 if(boundExceeded)*boundExceeded=false;
 if(!effect||!output||!capacity||!count||!warnings)return false;*count=*warnings=0;
 typedef const char* (WINAPI *Find)(void*,const char*,const char*);typedef const char* (WINAPI *Child)(void*,const char*,unsigned int);typedef long (WINAPI *Describe)(void*,const char*,Parameter*);
 void** methods=*(void***)effect;const char* handle=((Find)methods[9])(effect,0,"MaterialVertexDef");
 if(!handle){output[(*count)++]=End;return true;}Parameter structure={};if(((Describe)methods[4])(effect,handle,&structure)<0||structure.klass!=5)return false;if(structure.members>=capacity){if(boundExceeded)*boundExceeded=true;return false;}
 unsigned int offset=0;
 for(unsigned int i=0;i<structure.members;i++){
  const char* child=((Child)methods[8])(effect,handle,i);Parameter parameter={};if(!child||((Describe)methods[4])(effect,child,&parameter)<0)return false;
  unsigned int usage,index,type;bool clamped,semanticUnsupported;if(!Semantic(parameter.semantic,&usage,&index,&clamped,&semanticUnsupported)){if(boundExceeded)*boundExceeded=semanticUnsupported;return false;}
  unsigned int components=parameter.rows*parameter.columns;if(!components||parameter.rows>4||parameter.columns>4||!DeclType(components,parameter.bytes/components,parameter.type,usage,&type))return false;
  Element e={0,(unsigned short)offset,(unsigned char)type,0,(unsigned char)usage,(unsigned char)index};output[(*count)++]=e;offset+=Width(type);if(clamped)(*warnings)++;
 }
 output[(*count)++]=End;return true;
}
inline bool Merge(Element* child,unsigned int* count,unsigned int capacity,const Element* parent,unsigned int parentCount){
 if(!child||!count||!parent||!parentCount||*count>capacity)return false;
 for(unsigned int p=0;p<parentCount;p++){
  if(IsEnd(parent[p]))continue;bool existing=false;
  // Deliberately includes the child's END sentinel. The original scans the
  // entire vector, so an END-only child also suppresses parent POSITION0.
  for(unsigned int i=0;i<*count;i++)if(child[i].usage==parent[p].usage&&child[i].index==parent[p].index){existing=true;break;}
  if(existing)continue;unsigned int size=*count+1;if(size<2)size=2;if(size>capacity)return false;child[size-2]=parent[p];child[size-1]=End;*count=size;
 }
 return true;
}
class Reader {
 typedef long (WINAPI *Create)(const char*,const void*,void*,unsigned int,void**,void**);Create create;
 Reader(const Reader&);Reader& operator=(const Reader&);
 static void Release(void* object){if(object){typedef unsigned long (WINAPI *Function)(void*);((Function)(*(void***)object)[2])(object);}}
public:
 void* effect;long result;unsigned int count,warnings;Element elements[64];const char* error;bool unsupported;
 Reader():create(0),effect(0),result(0),count(0),warnings(0),error(0),unsupported(false){}~Reader(){Clear();}
 void Clear(){Release(effect);effect=0;count=warnings=0;error=0;unsupported=false;}
 bool Initialize(){if(create)return true;static Create sharedCreate=0;if(!sharedCreate){HMODULE dll=LoadLibraryA("d3dx9_27.dll");sharedCreate=dll?(Create)GetProcAddress(dll,"D3DXCreateEffectCompilerFromFileA"):0;}create=sharedCreate;if(!create)error="DirectX effect compiler unavailable";return create!=0;}
 bool ReadPrimary(const char* filename,void* include=0){
  Clear();if(!filename)return false;if(!Initialize()){unsupported=true;return false;}void* errors=0;result=create(filename,0,include,0,&effect,&errors);Release(errors);
  if(result<0||!effect){Clear();error="Shader frontend compilation failed";return false;}
  if(!Primary(effect,elements,64,&count,&warnings,&unsupported)){count=0;error="Unsupported or invalid MaterialVertexDef";return false;}return true;
 }
};
}
