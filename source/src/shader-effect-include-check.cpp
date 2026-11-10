#include "WorldPacker.cpp"
#include "ShaderEffectInclude.h"
#include "ShaderRenderDeclaration.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
extern "C" void HarnessEntry(){
 char root[MAX_PATH],shader[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_INCLUDE_ROOT",root,MAX_PATH)||!GetEnvironmentVariableA("FEAR_INCLUDE_SHADER",shader,MAX_PATH)||!GetEnvironmentVariableA("FEAR_INCLUDE_OUTPUT",output,MAX_PATH))ExitProcess(1);
 PrefabScene::Arena memory;void* include=0;unsigned int originalObject[5]={};
#ifdef FEAR_INCLUDE_ORACLE
 LoadLibraryA("E:/fearsdk/Dev/Tools/msvcr71.dll");LoadLibraryA("E:/fearsdk/Dev/Tools/msvcp71.dll");LoadLibraryA("E:/fearsdk/Dev/Tools/MFC71.dll");unsigned char* original=(unsigned char*)LoadLibraryA("E:/fearsdk/Dev/Tools/Packers/WorldPacker.dll");if(!original)ExitProcess(2);
 const unsigned char signature[4]={0x6a,0xff,0x68,0x0b};for(unsigned int i=0;i<4;i++)if(original[0x7cca0+i]!=signature[i])ExitProcess(3);typedef void* (__thiscall *Construct)(void*,const char*);include=((Construct)(original+0x7cca0))(originalObject,root);
#else
 (void)originalObject;void* resolverStorage=memory.Allocate(sizeof(ShaderEffectInclude::Resolver));if(!resolverStorage)ExitProcess(4);include=new(resolverStorage) ShaderEffectInclude::Resolver(root);if(!((ShaderEffectInclude::Resolver*)include)->depth)ExitProcess(5);
#endif
 typedef long (WINAPI *Create)(const char*,const void*,void*,unsigned int,void**,void**);HMODULE dx=LoadLibraryA("d3dx9_27.dll");Create create=dx?(Create)GetProcAddress(dx,"D3DXCreateEffectCompilerFromFileA"):0;if(!create)ExitProcess(6);void* effect=0,*errors=0;long result=create(shader,0,include,0,&effect,&errors);
 unsigned int elementsCount=0,warnings=0,reflection=0;ShaderRenderDeclaration::Element elements[64];if(result>=0&&effect)reflection=ShaderRenderDeclaration::Primary(effect,elements,64,&elementsCount,&warnings)?1:0;
 typedef unsigned long (WINAPI *Release)(void*);if(errors)((Release)(*(void***)errors)[2])(errors);if(effect)((Release)(*(void***)effect)[2])(effect);
#ifdef FEAR_INCLUDE_ORACLE
 typedef void (__thiscall *Destroy)(void*);((Destroy)(original+0x7cb50))(include);
#else
 // Compiler must close the primary file and all nested includes before return.
 if(((ShaderEffectInclude::Resolver*)include)->depth!=1)ExitProcess(7);((ShaderEffectInclude::Resolver*)include)->~Resolver();
#endif
 HANDLE file=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);DWORD done;unsigned int header[5]={0x314e4953,(unsigned int)result,reflection,elementsCount,warnings};if(file==INVALID_HANDLE_VALUE||!WriteFile(file,header,20,&done,0)||done!=20||!WriteFile(file,elements,elementsCount*8,&done,0)||done!=elementsCount*8)ExitProcess(8);CloseHandle(file);ExitProcess(0);
}
