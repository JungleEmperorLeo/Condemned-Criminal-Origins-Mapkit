#include "WorldPacker.cpp"
#include "MaterialResources.h"
#include "ShaderRenderDeclaration.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
// Source-material primary declaration diagnostic. This intentionally does not
// assert complete original shader-load success or merge dependency declarations.
extern "C" void HarnessEntry(){
 char root[MAX_PATH],input[MAX_PATH],output[MAX_PATH];
 if(!GetEnvironmentVariableA("FEAR_MATERIAL_LAYOUT_ROOT",root,MAX_PATH)||!GetEnvironmentVariableA("FEAR_MATERIAL_LAYOUT_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_MATERIAL_LAYOUT_OUTPUT",output,MAX_PATH))ExitProcess(1);
 HANDLE file=CreateFileA(input,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);if(file==INVALID_HANDLE_VALUE)ExitProcess(2);
 DWORD bytes=GetFileSize(file,0),done;if(bytes==0xffffffff||bytes>16*1024*1024)ExitProcess(2);PrefabScene::Arena memory;unsigned char* data=(unsigned char*)memory.Allocate(bytes+1);
 if(!data||!ReadFile(file,data,bytes,&done,0)||done!=bytes)ExitProcess(3);CloseHandle(file);
 MaterialResources::Material material;if(!material.Read(data,bytes))ExitProcess(4);
 char shader[MAX_PATH];if(!PrefabScene::Join(root,material.shader,shader))ExitProcess(5);ShaderRenderDeclaration::Reader declaration;
 if(!declaration.ReadPrimary(shader))ExitProcess(6);unsigned int nameBytes=lstrlenA(material.shader);
 unsigned int header[5]={0x314c524d,declaration.count,declaration.warnings,(unsigned int)declaration.result,nameBytes};
 HANDLE out=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);
 if(out==INVALID_HANDLE_VALUE||!WriteFile(out,header,20,&done,0)||done!=20||!WriteFile(out,material.shader,nameBytes,&done,0)||done!=nameBytes||!WriteFile(out,declaration.elements,declaration.count*8,&done,0)||done!=declaration.count*8)ExitProcess(7);
 CloseHandle(out);ExitProcess(0);
}
