#define FEAR_PRIMARY_SURFACE_PREPARATION 1
// Research-only structure worker: dumps every WorldModel face's local physical normal
// key (group, emitted, owner node, polygon, normal) to FEAR_PHYSNORMAL_TRACE_FILE.
#define FEAR_PHYSNORMAL_TRACE 1
#include "WorldPacker.cpp"
static HANDLE PhysTraceFile(){static HANDLE h=INVALID_HANDLE_VALUE;static bool opened=false;if(!opened){opened=true;char path[MAX_PATH];if(GetEnvironmentVariableA("FEAR_PHYSNORMAL_TRACE_FILE",path,MAX_PATH))h=CreateFileA(path,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);}return h;}
void FearPhysNormalTrace(unsigned int group,unsigned int emitted,unsigned int owner,unsigned int polygon,const WorldText::Vec3& normal){HANDLE h=PhysTraceFile();if(h==INVALID_HANDLE_VALUE)return;unsigned int head[4]={group,emitted,owner,polygon};DWORD done=0;WriteFile(h,head,16,&done,0);WriteFile(h,&normal,12,&done,0);}
static unsigned int RangeLow(){char t[16];if(!GetEnvironmentVariableA("FEAR_NODE_LOW",t,16))return 0xffffffffu;unsigned int v=0;for(const char* p=t;*p>='0'&&*p<='9';p++)v=v*10+(unsigned int)(*p-'0');return v;}
static unsigned int RangeHigh(){char t[16];if(!GetEnvironmentVariableA("FEAR_NODE_HIGH",t,16))return 0;unsigned int v=0;for(const char* p=t;*p>='0'&&*p<='9';p++)v=v*10+(unsigned int)(*p-'0');return v;}
void FearNodeInfo(const PrefabScene::Model& model,const PrefabScene::Node& n){
 static HANDLE h=INVALID_HANDLE_VALUE;static bool opened=false;if(!opened){opened=true;char path[MAX_PATH];if(GetEnvironmentVariableA("FEAR_NODEINFO_FILE",path,MAX_PATH))h=CreateFileA(path,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);}
 if(h==INVALID_HANDLE_VALUE||n.index<RangeLow()||n.index>RangeHigh())return;DWORD done=0;
 const PrefabScene::Node* chain[16];unsigned int c=0;const PrefabScene::Node* cur=&n;
 while(cur&&c<16){chain[c++]=cur;if(cur->parent==0xffffffffu)break;const PrefabScene::Node* found=0;for(const PrefabScene::Node* x=model.first;x;x=x->next)if(x->index==cur->parent){found=x;break;}cur=found;}
 for(unsigned int i=0;i<c;i++){const PrefabScene::Node* x=chain[i];char line[1400];unsigned int at=0;const char* parts[5]={x->name,x->className,x->type,x->source,""};unsigned int nums[4]={i,x->index,x->sourceId,x->importedFromPrefab?1u:0u};for(unsigned int k=0;k<4;k++){char digits[12];unsigned int d=0,v2=nums[k];do{digits[d++]=(char)('0'+v2%10);v2/=10;}while(v2);while(d)line[at++]=digits[--d];line[at++]='|';}for(unsigned int k=0;k<4;k++){for(const char* s=parts[k];*s&&at<1380;s++)line[at++]=*s;line[at++]='|';}line[at++]=13;line[at++]=10;WriteFile(h,line,at,&done,0);}
 WriteFile(h,"--\r\n",4,&done,0);
}

#include "NativeWorldModelsStructure.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
static bool Write(HANDLE file,const void* data,unsigned int size){DWORD done=0;return !size||(WriteFile(file,data,size,&done,0)&&done==size);}
static bool Cancel(void* context){return *(bool*)context;}
extern "C" void HarnessEntry(){
 char source[MAX_PATH],project[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_READER_INPUT",source,MAX_PATH)||!GetEnvironmentVariableA("FEAR_READER_PROJECT",project,MAX_PATH)||!GetEnvironmentVariableA("FEAR_MODELS_STRUCTURE_OUTPUT",output,MAX_PATH))ExitProcess(1);
 PrefabScene::Arena memory;void* sceneStorage=memory.Allocate(sizeof(NativePreparedRenderScene::Context));if(!sceneStorage)ExitProcess(2);NativePreparedRenderScene::Context* scene=new(sceneStorage)NativePreparedRenderScene::Context;
 NativeWorldModelsStructure::Builder builder;unsigned int guards=0;
 if(builder.Build(*scene)||builder.Ready()||builder.Models().data||!builder.Error().message[0])ExitProcess(10);guards++;
 if(!scene->Build(source,project)){LogLine(scene->Diagnostic().message);ExitProcess(2);}
 NativeWorldModelsStructure::Settings settings;settings.physical.fragmentLimit=500000;settings.physical.pointLimit=4000000;
 bool cancelled=true;NativeWorldModelsStructure::Observer events;events.cancel=Cancel;events.context=&cancelled;
 if(builder.Build(*scene,settings,events)||builder.Ready()||builder.Models().data||builder.Structural().Tree().data||!builder.Error().message[0])ExitProcess(11);guards++;
 if(!builder.Build(*scene,settings)){LogLine(builder.Error().message);ExitProcess(3);}
 scene->Clear();
 const NativeWorldStructure::Builder& structure=builder.Structural();const SourceWorldModels::Builder& models=builder.Models();
 const SourceWorldTree::Builder& tree=structure.Tree();const SourceBlockerPolygons::Block& blockers=structure.OrdinaryBlockers();const ParticleBlockers::Block& particles=structure.ParticleBlockersData();
 unsigned int h[12]={0x324d5357u,1,tree.size,models.size,blockers.size,particles.size,tree.nodeCount,models.modelCount,models.nameCount,blockers.count,particles.count,guards};
 HANDLE file=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(file==INVALID_HANDLE_VALUE||!Write(file,h,48)||!Write(file,tree.data,tree.size)||!Write(file,models.data,models.size)||!Write(file,blockers.data,blockers.size)||!Write(file,particles.data,particles.size))ExitProcess(4);CloseHandle(file);
 builder.Clear();if(builder.Ready()||builder.Models().data||builder.Structural().Tree().data)ExitProcess(12);scene->~Context();ExitProcess(0);
}
