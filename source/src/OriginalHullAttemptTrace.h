#pragma once
// Read-only observations of configuration/validation at original call boundaries.
// Only the eight relative CALL operands inside the retry driver are redirected;
// each observer calls the untouched original target with identical arguments.
namespace OriginalHullAttemptTrace {
struct Attempt {unsigned int inputCount,config[13],validated;};
static Attempt attempts[16];static unsigned int count=0;
static void* buildTarget=0;static void* validateTarget=0;
static void __cdecl Build(const unsigned int* config,void* points,int n,void* graph,void* geometry){if(count==16)ExitProcess(28);Attempt& attempt=attempts[count++];attempt.inputCount=(unsigned int)n;memcpy(attempt.config,config,52);attempt.config[0]&=255u;attempt.validated=0xffffffffu;typedef void(__cdecl *Call)(const void*,void*,int,void*,void*);((Call)buildTarget)(config,points,n,graph,geometry);}
static void* __cdecl Validate(void* result,const void* config,void* points,int n,void* graph,void* geometry){typedef void*(__cdecl *Call)(void*,const void*,void*,int,void*,void*);void* value=((Call)validateTarget)(result,config,points,n,graph,geometry);if(!count||value!=result)ExitProcess(29);attempts[count-1].validated=*(unsigned char*)value?1u:0u;return value;}
static void Install(unsigned char* dll){HMODULE kernel=LoadLibraryA("kernel32.dll");typedef int(WINAPI *Protect)(void*,unsigned int,DWORD,DWORD*);typedef int(WINAPI *Flush)(HANDLE,const void*,unsigned int);Protect protect=(Protect)GetProcAddress(kernel,"VirtualProtect");Flush flush=(Flush)GetProcAddress(kernel,"FlushInstructionCache");if(!protect||!flush)ExitProcess(30);buildTarget=dll+0x6a4c0;validateTarget=dll+0x6b460;
 const unsigned int offsets[8]={0x6a6a5,0x6a6da,0x6a710,0x6a746,0x6a6b8,0x6a6ed,0x6a723,0x6a759};
 for(unsigned int i=0;i<8;i++){unsigned char* call=dll+offsets[i];void* original=i<4?buildTarget:validateTarget;void* observer=i<4?(void*)Build:(void*)Validate;if(call[0]!=0xe8||call+5+*(int*)(call+1)!=original)ExitProcess(31);DWORD old=0,unused=0;if(!protect(call,5,0x40,&old))ExitProcess(30);*(int*)(call+1)=(int)((unsigned char*)observer-(call+5));if(!protect(call,5,old,&unused)||!flush((HANDLE)-1,call,5))ExitProcess(30);}
}
}
