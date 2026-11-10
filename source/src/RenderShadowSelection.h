#pragma once
#include "MaterialResources.h"
// Scalar input selection performed before topology in one original 0x4A450
// ShadowLOD pass. Material/shader loading is a separate caller responsibility.
namespace RenderShadowSelection {
enum State {Unresolved=0,Failed=1,Loaded=2};
struct ShaderKey {const char* name;State state;};
inline unsigned char Lower(unsigned char c){return c>='A'&&c<='Z'?(unsigned char)(c+32):c;}
inline bool ValidName(const char* name){if(!name||!name[0])return false;unsigned int n=0;for(;name[n];n++)if(n>=260||(unsigned char)name[n]>=128)return false;return true;}
inline bool ValidKey(const ShaderKey& key){return key.state==Failed||(key.state==Loaded&&ValidName(key.name));}
inline bool EqualNames(const char* a,const char* b){while(*a&&Lower((unsigned char)*a)==Lower((unsigned char)*b)){a++;b++;}return Lower((unsigned char)*a)==Lower((unsigned char)*b);}
inline bool EqualResolved(const ShaderKey& a,const ShaderKey& b){
 if(a.state!=b.state)return false;if(a.state==Failed)return true;
 // The original cache compares shader filename text with _stricmp. It does
 // not normalize '/' to '\\', collapse dot segments or compare file contents.
 return EqualNames(a.name,b.name);
}
inline bool Selected(unsigned int surfaceFlags,unsigned int pass,const ShaderKey& section,const ShaderKey& canonical,bool* selected){
 if(!selected)return false;*selected=false;if(pass>2)return false;
 if(!(surfaceFlags&0x40)||((surfaceFlags>>16)&3)!=pass)return true;
 if(!ValidKey(section)||!ValidKey(canonical))return false;
 *selected=!EqualResolved(section,canonical);return true;
}
// This function only reads LTMI metadata. A syntactically valid shader filename
// does not establish successful original shader compilation/cache insertion.
inline bool ReadShaderName(MaterialResources::Material& material,const unsigned char* bytes,unsigned int size,const char** name){
 if(!name)return false;*name=0;if(!material.Read(bytes,size))return false;*name=material.shader;return true;
}
}
