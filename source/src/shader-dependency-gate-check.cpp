#include "WorldPacker.cpp"
#include "ShaderDependencyGate.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
extern "C" void HarnessEntry(){
 // Keep a failed isolated test from displaying a Windows crash dialog.
 HMODULE system=LoadLibraryA("kernel32.dll");typedef unsigned int (WINAPI *ErrorMode)(unsigned int);ErrorMode setErrorMode=system?(ErrorMode)GetProcAddress(system,"SetErrorMode"):0;if(setErrorMode)setErrorMode(3);
 char shader[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_SHADER_INPUT",shader,MAX_PATH)||!GetEnvironmentVariableA("FEAR_SHADER_OUTPUT",output,MAX_PATH))ExitProcess(1);
 // Removed direct calls into the original effect-object loader: a zeroed
 // allocation is not a constructed object and is invalid for general shaders.
 char oracle[4]={};if(GetEnvironmentVariableA("FEAR_SHADER_ORACLE",oracle,4)&&oracle[0]=='1')ExitProcess(4);
 ShaderDependencyGate::Compiler compiler;if(!compiler.Initialize())ExitProcess(2);unsigned int values[]={compiler.Check(shader)?1u:0u,(unsigned int)compiler.result};DWORD done;
 HANDLE f=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(f==INVALID_HANDLE_VALUE||!WriteFile(f,values,8,&done,0)||done!=8)ExitProcess(3);CloseHandle(f);ExitProcess(0);
}
