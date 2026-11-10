// Research diagnostic only: AIRegion brush children as seen by render consumption.
#define FEAR_PRIMARY_SURFACE_PREPARATION 1
#include "WorldPacker.cpp"
#include "NativePreparedRenderScene.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
static void Put(HANDLE f,const char* s){DWORD d=0;WriteFile(f,s,lstrlenA(s),&d,0);}
static void Num(HANDLE f,int v){char b[16];int n=0;bool neg=v<0;unsigned int u=neg?(unsigned int)(-v):(unsigned int)v;char t[16];do{t[n++]=(char)('0'+u%10);u/=10;}while(u);int k=0;if(neg)b[k++]='-';while(n)b[k++]=t[--n];b[k]=0;Put(f,b);}
extern "C" void HarnessEntry(){
 char source[MAX_PATH],project[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_READER_INPUT",source,MAX_PATH)||!GetEnvironmentVariableA("FEAR_READER_PROJECT",project,MAX_PATH)||!GetEnvironmentVariableA("FEAR_DIAG_OUTPUT",output,MAX_PATH))ExitProcess(1);
 PrefabScene::Arena memory;void* place=memory.Allocate(sizeof(NativePreparedRenderScene::Context));if(!place)ExitProcess(2);NativePreparedRenderScene::Context* scene=new(place)NativePreparedRenderScene::Context;if(!scene->Build(source,project)){LogLine(scene->Diagnostic().message);ExitProcess(2);}
 HANDLE f=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(f==INVALID_HANDLE_VALUE)ExitProcess(3);
 const PrefabScene::Model& model=scene->Model();const SourceExpandedRenderContours::Consumed& consumed=scene->Consumed();
 for(const PrefabScene::Node* n=model.first;n;n=n->next){if(n->kind!=1||n->parent==0xffffffffu)continue;const PrefabScene::Node* p=model.first;while(p&&p->index!=n->parent)p=p->next;if(!p||p->kind!=2||!FearBrushFlags::Equal(p->className,"AIRegion"))continue;
  Put(f,"brush ");Num(f,(int)n->index);Put(f," region ");Put(f,p->name);Put(f," type ");Put(f,n->type);Put(f," polys ");Num(f,(int)n->geometry.polygons);Put(f," points ");Num(f,(int)n->geometry.points);
  Put(f," projects ");Num(f,NativeRawNavMeshGeneral::Builder::ProjectsCarver(*n)?1:0);Put(f," category ");Num(f,consumed.categories[n->index]);Put(f," hidden ");Num(f,n->effectiveHidden?1:0);Put(f," upY1000 [");
  for(unsigned int i=0;i<n->geometry.polygons;i++){Num(f,(int)(n->geometry.faces[i].normal.y*1000));Put(f," ");}Put(f,"]\r\n");}
 CloseHandle(f);ExitProcess(0);
}
