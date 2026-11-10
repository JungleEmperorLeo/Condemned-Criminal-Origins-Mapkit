#pragma once
extern "C" __declspec(dllimport) HMODULE WINAPI LoadLibraryA(const char*);
extern "C" __declspec(dllimport) void* WINAPI GetProcAddress(HMODULE,const char*);
#pragma comment(linker,"/alternatename:__imp__LoadLibraryA@4=__imp__LoadLibraryA")
#pragma comment(linker,"/alternatename:__imp__GetProcAddress@8=__imp__GetProcAddress")
namespace ShaderDependencyGate {
class Compiler {
 typedef long (WINAPI *Create)(const char*,const void*,void*,unsigned int,void**,void**);
 Create create;
 void* effects[16];unsigned int effectCount;
 Compiler(const Compiler&);Compiler& operator=(const Compiler&);
 static void Release(void* object){if(object){typedef unsigned long (WINAPI *Function)(void*);((Function)(*(void***)object)[2])(object);}}
 void Clear(){for(unsigned int i=0;i<effectCount;i++)Release(effects[i]);effectCount=0;}
public:
 long result;bool available;
 Compiler():create(0),effectCount(0),result(0),available(false){}
 ~Compiler(){Clear();}
 bool Initialize(){if(create)return true;HMODULE dll=LoadLibraryA("d3dx9_27.dll");create=dll?(Create)GetProcAddress(dll,"D3DXCreateEffectCompilerFromFileA"):0;available=create!=0;return available;}
 bool Check(const char* shader){
  Clear();if(!create)return false;char current[MAX_PATH];if(!PrefabScene::Full(shader,current))return false;
  for(unsigned int depth=0;depth<16;depth++){
   void* compiler=0;void* errors=0;result=create(current,0,0,0,&compiler,&errors);Release(errors);if(result<0||!compiler){Release(compiler);Clear();return false;}effects[effectCount++]=compiler;
   typedef const char* (WINAPI *Find)(void*,const char*,const char*);typedef long (WINAPI *String)(void*,const char*,const char**);
   const char* handle=((Find)(*(void***)compiler)[9])(compiler,0,"ParentMaterial0");if(!handle)return true;const char* parent=0;if(((String)(*(void***)compiler)[51])(compiler,handle,&parent)<0||!parent||!parent[0])return true;
   char base[MAX_PATH],next[MAX_PATH];lstrcpyA(base,current);PrefabScene::Parent(base);if(!PrefabScene::Join(base,parent,next)){Clear();return false;}lstrcpyA(current,next);
  }
  Clear();return false;
 }
 bool HasTexture(const char* name)const{
  if(!name)return false;typedef const char* (WINAPI *Find)(void*,const char*,const char*);typedef long (WINAPI *Describe)(void*,const char*,void*);
  for(unsigned int i=0;i<effectCount;i++){void* effect=effects[i];const char* handle=((Find)(*(void***)effect)[9])(effect,0,name);if(!handle)continue;
  // D3DXPARAMETER_DESC, x86: Name, Semantic, Class, Type, then seven UINTs.
  unsigned int description[11]={};if(((Describe)(*(void***)effect)[4])(effect,handle,description)>=0&&description[3]>=5&&description[3]<=9)return true;}return false;
 }
};
// Uses the same external DirectX parser as the original packer. This does not
// recreate DirectX or the original custom include handler. Texture-name/type
// reflection and ParentMaterial0 are covered; other annotation policies are not.
}
