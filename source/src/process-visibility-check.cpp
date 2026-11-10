#include "win_min.h"
#include "IPackerImpl.h"
#include "IPackerOutput.h"
extern "C" __declspec(dllimport) IPackerImpl* AssociatePacker(const char*);
extern "C" void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:_ExitProcess@4=_ExitProcess")
#pragma comment(linker,"/alternatename:__imp__CreateFileA@28=__imp__CreateFileA")
#pragma comment(linker,"/alternatename:__imp__WriteFile@20=__imp__WriteFile")
#pragma comment(linker,"/alternatename:__imp__CloseHandle@4=__imp__CloseHandle")
class Output:public IPackerOutput {
public:
 unsigned int tasks,visibilityTask,messages,errors;Output():tasks(0),visibilityTask(0),messages(0),errors(0){}
 bool CreateTask(const char* name){tasks++;if(name[0]=='C')visibilityTask++;return true;}
 bool ActivateTask(const char*){return true;}bool ActivateSubTask(const char*){return true;}bool UpdateProgress(float){return true;}
 bool CheckOutputFile(const char*){return true;}bool LogMessage(EMsgType level,const char*){messages++;if(level==MSG_ERROR)errors++;return true;}
 bool LogMessageH(EMsgType level,const char*,const char* text){return LogMessage(level,text);}
};
extern "C" void HarnessEntry(){
 char input[MAX_PATH],status[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_READER_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_PROCESS_STATUS",status,MAX_PATH))ExitProcess(1);
 IPackerImpl* packer=AssociatePacker(input);if(!packer)ExitProcess(2);Output output;bool result=packer->Process(input,0,&output);
 HANDLE f=CreateFileA(status,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);DWORD done;unsigned int values[]={result?1u:0u,output.tasks,output.visibilityTask,output.messages,output.errors};if(f==INVALID_HANDLE_VALUE||!WriteFile(f,values,sizeof(values),&done,0)||done!=sizeof(values))ExitProcess(3);CloseHandle(f);ExitProcess(0);
}
extern "C" int __cdecl _purecall(){return 0;}
void operator delete(void*,unsigned int){}
