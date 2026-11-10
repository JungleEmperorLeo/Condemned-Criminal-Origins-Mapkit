#define FEAR_PRIMARY_SURFACE_PREPARATION 1
#include "WorldPacker.cpp"
#include "NativePhysicsMesh.h"
#include "NativePhysicsOwnerMeshTransport.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
static void Diagnostic(const char* output,const char* message){LogLine(message);char errorPath[MAX_PATH];lstrcpynA(errorPath,output,MAX_PATH-11);lstrcpyA(errorPath+lstrlenA(errorPath),".error.txt");HANDLE f=CreateFileA(errorPath,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(f!=INVALID_HANDLE_VALUE){DWORD written=0;WriteFile(f,message,lstrlenA(message),&written,0);CloseHandle(f);}}
static bool WriteOutput(const char* path,const void* data,unsigned int size){HANDLE f=CreateFileA(path,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(f==INVALID_HANDLE_VALUE)return false;DWORD written=0;bool success=WriteFile(f,data,size,&written,0)&&written==size;CloseHandle(f);return success;}
static bool SamePath(const char* a,const char* b){for(unsigned int i=0;;i++){char x=a[i],y=b[i];if(x>='A'&&x<='Z')x=(char)(x-'A'+'a');if(y>='A'&&y<='Z')y=(char)(y-'A'+'a');if(x=='\\')x='/';if(y=='\\')y='/';if(x!=y)return false;if(!x)return true;}}
extern "C" void HarnessEntry(){
 char source[MAX_PATH],project[MAX_PATH],output[MAX_PATH],catalogPath[MAX_PATH];DWORD ns=GetEnvironmentVariableA("FEAR_READER_INPUT",source,MAX_PATH),np=GetEnvironmentVariableA("FEAR_READER_PROJECT",project,MAX_PATH),no=GetEnvironmentVariableA("FEAR_NATIVE_PHYSICS_MESH_OUTPUT",output,MAX_PATH),nc=GetEnvironmentVariableA("FEAR_PHYSICS_OWNER_MESH_CATALOG_OUTPUT",catalogPath,MAX_PATH);if(!ns||ns>=MAX_PATH||!np||np>=MAX_PATH||!no||no>=MAX_PATH||nc>=MAX_PATH)ExitProcess(1);
 if(nc&&(SamePath(output,catalogPath)||SamePath(source,catalogPath)||SamePath(project,catalogPath))){Diagnostic(output,"Owner Mesh catalog output must have a distinct path");ExitProcess(1);}
 PrefabScene::Arena memory;void* place=memory.Allocate(sizeof(NativePreparedRenderScene::Context));if(!place)ExitProcess(2);NativePreparedRenderScene::Context* scene=new(place)NativePreparedRenderScene::Context;if(!scene->Build(source,project)){LogLine(scene->Diagnostic().message);ExitProcess(2);}
 NativePhysicsMesh::Builder mesh;if(!mesh.Build(*scene)){Diagnostic(output,mesh.error);scene->Clear();scene->~Context();ExitProcess(3);}
 unsigned char* catalogData=0;unsigned int catalogBytes=0;
 if(nc){NativePhysicsOwnerMeshTransport::Catalog catalog;if(!catalog.Build(*scene)){Diagnostic(output,catalog.error);scene->Clear();scene->~Context();ExitProcess(3);}catalogBytes=catalog.size;catalogData=(unsigned char*)memory.Allocate(catalogBytes);if(!catalogData||!catalog.WriteCatalog(catalogData,catalogBytes)){Diagnostic(output,"Owner Mesh catalog owned copy failed");scene->Clear();scene->~Context();ExitProcess(3);}}
 scene->Clear();scene->~Context();
 // Catalog construction/copy completes while the source scene is live. Write
 // the optional catalog first, so catalog failures never publish a MAIN prefix.
 if(nc&&!WriteOutput(catalogPath,catalogData,catalogBytes)){Diagnostic(output,"Owner Mesh catalog output write failed");ExitProcess(4);}
 if(!WriteOutput(output,mesh.data,mesh.size)){Diagnostic(output,"MAIN Mesh output write failed");ExitProcess(4);}ExitProcess(0);
}
