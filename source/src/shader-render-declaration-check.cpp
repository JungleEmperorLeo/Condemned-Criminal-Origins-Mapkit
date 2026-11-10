#include "WorldPacker.cpp"
#include "ShaderRenderDeclaration.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
extern "C" void HarnessEntry(){
 char path[MAX_PATH],outputPath[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_DECL_OUTPUT",outputPath,MAX_PATH))ExitProcess(1);PrefabScene::Arena memory;ShaderRenderDeclaration::Element* elements=(ShaderRenderDeclaration::Element*)memory.Allocate(1024);unsigned int count=0,warnings=0,success=0;long result=0;
#ifdef FEAR_DECL_ORACLE
 LoadLibraryA("E:/fearsdk/Dev/Tools/msvcr71.dll");LoadLibraryA("E:/fearsdk/Dev/Tools/msvcp71.dll");LoadLibraryA("E:/fearsdk/Dev/Tools/MFC71.dll");unsigned char* original=(unsigned char*)LoadLibraryA("E:/fearsdk/Dev/Tools/Packers/WorldPacker.dll");if(!original)ExitProcess(2);
#endif
 if(GetEnvironmentVariableA("FEAR_DECL_SHADER",path,MAX_PATH)){
  typedef long (WINAPI *Create)(const char*,const void*,void*,unsigned int,void**,void**);HMODULE dll=LoadLibraryA("d3dx9_27.dll");Create create=dll?(Create)GetProcAddress(dll,"D3DXCreateEffectCompilerFromFileA"):0;if(!create)ExitProcess(3);void* effect=0,*errors=0;result=create(path,0,0,0,&effect,&errors);
  if(errors){typedef unsigned long (WINAPI *Release)(void*);((Release)(*(void***)errors)[2])(errors);}if(result>=0&&effect){
#ifdef FEAR_DECL_ORACLE
   const unsigned char signature[4]={0x83,0xec,0x7c,0x53};for(unsigned int i=0;i<4;i++)if(original[0x7c1b0+i]!=signature[i])ExitProcess(4);unsigned int shader[15]={};shader[0x18/4]=shader[0x1c/4]=(unsigned int)elements;shader[0x20/4]=(unsigned int)(elements+128);
   typedef bool (__thiscall *Reflect)(void*,void*);success=((Reflect)(original+0x7c1b0))(shader,effect)?1:0;count=success?(shader[0x1c/4]-shader[0x18/4])/8:0;
#else
   success=ShaderRenderDeclaration::Primary(effect,elements,128,&count,&warnings)?1:0;if(!success)count=0;
#endif
   typedef unsigned long (WINAPI *Release)(void*);((Release)(*(void***)effect)[2])(effect);
  }
 }else{
  if(!GetEnvironmentVariableA("FEAR_DECL_INPUT",path,MAX_PATH))ExitProcess(1);HANDLE file=CreateFileA(path,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);DWORD done;unsigned int header[2];if(file==INVALID_HANDLE_VALUE||!ReadFile(file,header,8,&done,0)||done!=8||header[0]>64||header[1]>64||!header[1])ExitProcess(5);
  ShaderRenderDeclaration::Element* parent=(ShaderRenderDeclaration::Element*)memory.Allocate(512);if(!elements||!parent||!ReadFile(file,elements,header[0]*8,&done,0)||done!=header[0]*8||!ReadFile(file,parent,header[1]*8,&done,0)||done!=header[1]*8)ExitProcess(6);CloseHandle(file);count=header[0];
#ifdef FEAR_DECL_ORACLE
  const unsigned char signature[4]={0x8b,0x54,0x24,0x04};for(unsigned int i=0;i<4;i++)if(original[0x7c380+i]!=signature[i])ExitProcess(4);unsigned int childShader[15]={},parentShader[15]={};childShader[0x18/4]=(unsigned int)elements;childShader[0x1c/4]=(unsigned int)(elements+count);childShader[0x20/4]=(unsigned int)(elements+128);parentShader[0x18/4]=(unsigned int)parent;parentShader[0x1c/4]=(unsigned int)(parent+header[1]);parentShader[0x20/4]=(unsigned int)(parent+header[1]);
  typedef bool (__thiscall *Merge)(void*,void*);success=((Merge)(original+0x7c380))(childShader,parentShader)?1:0;count=(childShader[0x1c/4]-childShader[0x18/4])/8;
#else
  success=ShaderRenderDeclaration::Merge(elements,&count,128,parent,header[1])?1:0;
#endif
 }
 HANDLE out=CreateFileA(outputPath,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);DWORD done;unsigned int header[5]={0x31444353,success,count,warnings,(unsigned int)result};if(out==INVALID_HANDLE_VALUE||!WriteFile(out,header,20,&done,0)||done!=20||!WriteFile(out,elements,count*8,&done,0)||done!=count*8)ExitProcess(7);CloseHandle(out);ExitProcess(0);
}
