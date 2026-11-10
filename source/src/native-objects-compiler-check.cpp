#define FEAR_PRIMARY_SURFACE_PREPARATION 1
#include "WorldPacker.cpp"
#include "NativeObjectsCompiler.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
extern "C" void HarnessEntry(){
 char source[MAX_PATH],project[MAX_PATH],output[MAX_PATH];DWORD a=GetEnvironmentVariableA("FEAR_READER_INPUT",source,MAX_PATH),b=GetEnvironmentVariableA("FEAR_READER_PROJECT",project,MAX_PATH),c=GetEnvironmentVariableA("FEAR_NATIVE_OBJECTS_OUTPUT",output,MAX_PATH);if(!a||a>=MAX_PATH||!b||b>=MAX_PATH||!c||c>=MAX_PATH)ExitProcess(1);
 PrefabScene::Arena memory;void* place=memory.Allocate(sizeof(NativePreparedRenderScene::Context));if(!place)ExitProcess(2);NativePreparedRenderScene::Context* scene=new(place)NativePreparedRenderScene::Context;
 if(!scene->Build(source,project)){LogLine(scene->Diagnostic().message);ExitProcess(3);}
 NativeObjectsCompiler::Settings settings;char mode[4];DWORD modeLength=GetEnvironmentVariableA("FEAR_NATIVE_OBJECTS_ROTATION_REPLAY",mode,4);if(modeLength){if(modeLength!=1||(mode[0]!='0'&&mode[0]!='1'))ExitProcess(1);settings.rotationReplay=mode[0]=='1';}
 NativeObjectsCompiler::Builder compiler;if(!compiler.Build(*scene,settings)){LogLine(compiler.error);LogLine(compiler.errorProperty);static char path[MAX_PATH];static struct Failure {unsigned int node;char diagnostic[512],property[256],className[64],name[256],type[32],sourceData[512];} failure;failure.node=compiler.errorNode;lstrcpynA(failure.diagnostic,compiler.error?compiler.error:"",512);lstrcpynA(failure.property,compiler.errorProperty,256);for(const PrefabScene::Node* n=scene->Model().first;n;n=n->next)if(n->index==compiler.errorNode){lstrcpynA(failure.className,n->className,64);lstrcpynA(failure.name,n->name,256);for(const PrefabScene::Property* p=n->properties;p;p=p->next)if(FearBrushFlags::Equal(p->name,compiler.errorProperty)){lstrcpynA(failure.type,p->type,32);lstrcpynA(failure.sourceData,p->data?p->data:"",512);break;}break;}unsigned int length=lstrlenA(output);if(length<MAX_PATH-11){lstrcpyA(path,output);lstrcpyA(path+length,".error.bin");HANDLE file=CreateFileA(path,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);DWORD done=0;if(file!=INVALID_HANDLE_VALUE){WriteFile(file,&failure,sizeof(failure),&done,0);CloseHandle(file);}}ExitProcess(4);}
 scene->Clear();scene->~Context();
 HANDLE file=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);DWORD done=0;if(file==INVALID_HANDLE_VALUE||!WriteFile(file,compiler.data,compiler.size,&done,0)||done!=compiler.size)ExitProcess(5);CloseHandle(file);ExitProcess(0);
}
