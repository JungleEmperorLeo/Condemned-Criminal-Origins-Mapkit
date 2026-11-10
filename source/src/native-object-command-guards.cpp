#define FEAR_PRIMARY_SURFACE_PREPARATION 1
#include "WorldPacker.cpp"
#include "NativeObjectCommand.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
static unsigned int results[64],used=0;
#define CHECK(name,test) results[used++]=(test)?1u:0u
static void Property(PrefabScene::Property& p,const char* value,const char* type="command"){memset(&p,0,sizeof(p));lstrcpyA(p.name,"SyntheticCommand");lstrcpyA(p.type,type);p.data=const_cast<char*>(value);p.flags=0x12345678;}
static bool Value(const PrefabScene::Property& p,const char* expected){return NativeObjectCommand::Exact(p.data,expected);}
static bool Prepare(PrefabScene::Property& p,PrefabScene::Arena& memory,const char* input,const char* expected){Property(p,input);return NativeObjectCommand::PrepareCompiled(p,memory)&&Value(p,expected);}
static bool Write(HANDLE file,const void* data,unsigned int bytes){DWORD done=0;return !bytes||(WriteFile(file,data,bytes,&done,0)&&done==bytes);}
static bool Text(HANDLE file,const char* value){unsigned int length=lstrlenA(value);return Write(file,&length,4)&&Write(file,value,length);}
extern "C" void HarnessEntry(){
 PrefabScene::Arena memory;PrefabScene::Property p;const char* packet[6];
 CHECK("trailing tab becomes one trailing space",Prepare(p,memory,"msg object ON\t","msg object ON "));packet[0]=p.data;
 CHECK("tabs at each position become spaces without collapsing",Prepare(p,memory,"\tmsg\t\tobject\t"," msg  object "));packet[1]=p.data;
 CHECK("newlines and braces are removed rather than replaced",Prepare(p,memory,"{msg\r\nobject} ON","msgobject ON"));packet[2]=p.data;
 CHECK("braces and tabs inside quote bytes follow the same rule",Prepare(p,memory,"msg target \"a{b}\tc\"","msg target \"ab c\""));packet[3]=p.data;
 CHECK("semicolons and spaces are not compacted",Prepare(p,memory,"  msg x ON;  msg y OFF  ","  msg x ON;  msg y OFF  "));packet[4]=p.data;
 CHECK("only removed bytes produce a valid empty command",Prepare(p,memory,"{\r\n}",""));packet[5]=p.data;
 char source[]="msg immutable\t{ON}";Property(p,source);const char* borrowed=p.data;
 CHECK("changed command uses an owned text allocation",NativeObjectCommand::PrepareCompiled(p,memory)&&p.data!=borrowed&&Value(p,"msg immutable ON"));
 CHECK("input command bytes remain immutable",NativeObjectCommand::Exact(source,"msg immutable\t{ON}"));
 CHECK("property name flags and kind remain unchanged",p.flags==0x12345678&&NativeObjectCommand::Exact(p.type,"command")&&NativeObjectCommand::Exact(p.name,"SyntheticCommand"));
 borrowed=p.data;CHECK("repeated compilation is idempotent without another copy",NativeObjectCommand::PrepareCompiled(p,memory)&&p.data==borrowed);
 Property(p,"already compiled");borrowed=p.data;CHECK("unchanged command retains its borrowed pointer",NativeObjectCommand::PrepareCompiled(p,memory)&&p.data==borrowed);
 Property(p,"");borrowed=p.data;CHECK("empty command is accepted without allocation",NativeObjectCommand::PrepareCompiled(p,memory)&&p.data==borrowed&&Value(p,""));
 Property(p,"text\t{value}","string");borrowed=p.data;CHECK("ordinary strings retain tabs and braces",NativeObjectCommand::PrepareCompiled(p,memory)&&p.data==borrowed&&Value(p,"text\t{value}"));
 Property(p,"text\t{value}","stringid");CHECK("stringid properties are untouched",NativeObjectCommand::PrepareCompiled(p,memory)&&Value(p,"text\t{value}"));
 Property(p,"text\t{value}","Command");CHECK("noncanonical type spelling is outside helper gate",NativeObjectCommand::PrepareCompiled(p,memory)&&Value(p,"text\t{value}"));
 Property(p,0);CHECK("null command text fails before publication",!NativeObjectCommand::PrepareCompiled(p,memory)&&p.data==0);
 Property(p,0,"string");CHECK("unrelated null payload is left for its own codec",NativeObjectCommand::PrepareCompiled(p,memory)&&p.data==0);
 static char boundary[4097];for(unsigned int i=0;i<4095;i++)boundary[i]='x';boundary[0]='\t';boundary[4095]=0;Property(p,boundary);
 CHECK("4095 byte commands fit exactly",NativeObjectCommand::PrepareCompiled(p,memory)&&lstrlenA(p.data)==4095&&p.data[0]==' '&&boundary[0]=='\t');
 boundary[4095]='y';boundary[4096]=0;Property(p,boundary);borrowed=p.data;CHECK("4096 byte commands fail atomically",!NativeObjectCommand::PrepareCompiled(p,memory)&&p.data==borrowed&&boundary[0]=='\t');
 const char bytes[]={'m','s','g',' ',-32,'\t',-1,0},expected[]={'m','s','g',' ',-32,' ',-1,0};CHECK("non ASCII bytes pass through unchanged",Prepare(p,memory,bytes,expected));
 char freshSyntax[]="msg prefix//target\tON";Property(p,freshSyntax);borrowed=p.data;
 CHECK("fresh comment syntax introduced by a prefix is rejected",!NativeObjectCommand::PrepareCompiled(p,memory));
 CHECK("fresh syntax rejection preserves pointer text and flags",p.data==borrowed&&Value(p,"msg prefix//target\tON")&&NativeObjectCommand::Exact(freshSyntax,"msg prefix//target\tON")&&p.flags==0x12345678);
 Property(p,"http://resource\t{value}","string");borrowed=p.data;CHECK("double slashes in noncommand strings are untouched",NativeObjectCommand::PrepareCompiled(p,memory)&&p.data==borrowed&&Value(p,"http://resource\t{value}"));
 CHECK("comment syntax created only by brace removal is not reparsed",Prepare(p,memory,"msg target /{/","msg target //"));
 CHECK("helper recovers after invalid input",Prepare(p,memory,"msg recovered\tON","msg recovered ON"));
 char path[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_COMMAND_GUARDS_OUTPUT",path,MAX_PATH))ExitProcess(1);HANDLE file=CreateFileA(path,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);unsigned int header[3]={0x31474d43,used,6};if(file==INVALID_HANDLE_VALUE||!Write(file,header,12)||!Write(file,results,used*4))ExitProcess(3);for(unsigned int i=0;i<6;i++)if(!Text(file,packet[i]))ExitProcess(3);CloseHandle(file);for(unsigned int i=0;i<used;i++)if(!results[i])ExitProcess(4);ExitProcess(0);
}
