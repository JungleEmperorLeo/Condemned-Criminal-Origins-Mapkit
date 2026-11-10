#include "win_min.h"
#include "IPackerImpl.h"
extern "C" __declspec(dllimport) IPackerImpl* AssociatePacker(const char*);
extern "C" void WINAPI ExitProcess(unsigned int);
extern "C" void __cdecl HarnessEntry(){char input[MAX_PATH];DWORD n=GetEnvironmentVariableA("FEAR_READER_INPUT",input,MAX_PATH);const char* path=n&&n<MAX_PATH?input:"E:\\fearsdk\\Dev\\Tools\\Packers\\BoxWorld.World00a";IPackerImpl* p=AssociatePacker(path);if(!p)ExitProcess(1);p->Process(path,0,0);ExitProcess(0);}
#pragma comment(linker,"/alternatename:_ExitProcess@4=_ExitProcess")
