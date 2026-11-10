#pragma once
#include <stdarg.h>
#include "E:/fearsdk/Source/engine/sdk/inc/istringeditmgr.h"
// Public SDK adapter, not an independent Strdb00p decoder.
class VoiceDatabaseResources {
 PrefabScene::Arena data;IStringEditMgr* manager;HSTRINGEDIT database;
public:
 VoiceDatabaseResources():manager(0),database(0){}
 ~VoiceDatabaseResources(){if(database&&manager)manager->ReleaseStringEdit(database);}
 bool Initialize(const PrefabScene::Project& project){
  if(database)return true;char binary[MAX_PATH];if(!project.Resolve("Project Binaries","GameDatabase",binary))return false;
  unsigned int end=lstrlenA(binary);while(end&&binary[end-1]!='\\'&&binary[end-1]!='/')end--;if(!end||end+22>=MAX_PATH)return false;lstrcpyA(binary+end,"StringEditRuntime.dll");
  void* dll=LoadLibraryA(binary);if(!dll)return false;typedef IStringEditMgr* (*Factory)();Factory factory=(Factory)GetProcAddress(dll,"GetIStringEditMgr");if(!factory||(manager=factory())==0)return false;
  char target[MAX_PATH];if(!PrefabScene::Join(project.resources,"StringDatabase\\FEAR.Strdb00p",target))return false;
  HANDLE file=CreateFileA(target,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);if(file==INVALID_HANDLE_VALUE)return false;unsigned int n=GetFileSize(file,0);unsigned char* dataBytes=n&&n<=64*1024*1024?(unsigned char*)data.Allocate(n):0;DWORD done;
  bool ok=dataBytes&&ReadFile(file,dataBytes,n,&done,0)&&done==n;CloseHandle(file);if(!ok)return false;MemoryStream stream(dataBytes,n);database=manager->CreateFromFile(&stream);return database&&!stream.HasErrorOccurred();
 }
 const char* Path(const char* id){return database&&id?manager->GetVoicePath(database,id):0;}
 bool Exists(const char* id){return database&&id&&manager->DoesVoicePathExist(database,id);}
};
