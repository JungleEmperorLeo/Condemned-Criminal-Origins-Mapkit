#define FEAR_PRIMARY_SURFACE_PREPARATION 1
#include "WorldPacker.cpp"
#include "SourceRuntimeObjectOrder.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
namespace SourceKeyframerInventory {
enum {MaximumBytes=16*1024*1024};
class Packet {
 PrefabScene::Arena memory;
public:
 unsigned char* data;unsigned int size,count,framers,keys;
 Packet():data(0),size(0),count(0),framers(0),keys(0){}
 bool Put(const void* value,unsigned int bytes){if(bytes>MaximumBytes-size)return false;memcpy(data+size,value,bytes);size+=bytes;return true;}
 bool Word(unsigned int value){return Put(&value,4);}
 bool String(const char* value,unsigned int limit){if(!value)return false;unsigned int bytes=0;while(bytes<=limit&&value[bytes])bytes++;return bytes<=limit&&Word(bytes)&&Put(value,bytes);}
 static bool IsA(const GameClassMetadata::Catalog& catalog,const char* cls,const char* base){const GameClassMetadata::Class* c=catalog.Find(cls);for(unsigned int steps=0;c&&steps<catalog.count;steps++){if(GameClassMetadata::Exact(c->name,base))return true;c=c->parentIndex==0xffffffffu?0:catalog.classes+c->parentIndex;}return false;}
 bool Node(const PrefabScene::Node& n,unsigned int category,unsigned int runtimeIndex,unsigned int flags){unsigned int props=0;for(const PrefabScene::Property* p=n.properties;p;p=p->next){if(props==4096)return false;props++;}unsigned int fields[9]={category,n.index,n.parent,n.sourceId,n.importedFromPrefab?1u:0u,n.sourceKind,runtimeIndex,props,flags};if(!Put(fields,sizeof(fields))||!Put(&n.position,12)||!Put(&n.orientation,48)||!String(n.className,63)||!String(n.name,255)||!String(n.source,MAX_PATH-1))return false;for(const PrefabScene::Property* p=n.properties;p;p=p->next)if(!String(p->type,31)||!String(p->name,255)||!String(p->data,65535))return false;count++;if(category==1)framers++;else keys++;return true;}
 bool Build(const NativePreparedRenderScene::Context& scene){if(!scene.Ready())return false;data=(unsigned char*)memory.Allocate(MaximumBytes);if(!data)return false;unsigned int header[6]={0x3149464bu,1,0,0,0,0};if(!Put(header,24)||!Put(&scene.Offset(),12))return false;SourceRuntimeObjectOrder::Builder order;if(!order.Build(scene))return false;const PrefabScene::Model& model=scene.Model();const GameClassMetadata::Catalog& catalog=scene.Catalog();const PrefabScene::Node** nodes=(const PrefabScene::Node**)memory.Allocate((model.count+1)*sizeof(void*));if(!nodes)return false;for(const PrefabScene::Node* n=model.first;n;n=n->next){if(n->index>=model.count)return false;nodes[n->index]=n;}
  for(unsigned int i=0;i<order.count;i++){const PrefabScene::Node& n=*nodes[order.rows[i].sourceNode];if(IsA(catalog,n.className,"KeyFramer")&&!Node(n,1,i,order.rows[i].classFlags))return false;}
  for(const PrefabScene::Node* n=model.first;n;n=n->next)if(n->kind==2&&IsA(catalog,n->className,"Key")){const GameClassMetadata::Class* cls=catalog.Find(n->className);if(!cls||!Node(*n,2,0xffffffffu,cls->flags))return false;}
  unsigned int* words=(unsigned int*)data;words[2]=count;words[3]=framers;words[4]=keys;words[5]=size;return true;
 }
};
}
extern "C" void HarnessEntry(){char source[MAX_PATH],project[MAX_PATH],output[MAX_PATH];DWORD a=GetEnvironmentVariableA("FEAR_READER_INPUT",source,MAX_PATH),b=GetEnvironmentVariableA("FEAR_READER_PROJECT",project,MAX_PATH),c=GetEnvironmentVariableA("FEAR_KEYFRAMER_INVENTORY",output,MAX_PATH);if(!a||a>=MAX_PATH||!b||b>=MAX_PATH||!c||c>=MAX_PATH)ExitProcess(1);PrefabScene::Arena memory;void* place=memory.Allocate(sizeof(NativePreparedRenderScene::Context));if(!place)ExitProcess(2);NativePreparedRenderScene::Context* scene=new(place)NativePreparedRenderScene::Context;if(!scene->Build(source,project)){LogLine(scene->Diagnostic().message);ExitProcess(3);}SourceKeyframerInventory::Packet packet;if(!packet.Build(*scene))ExitProcess(4);scene->Clear();scene->~Context();HANDLE file=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);DWORD done=0;if(file==INVALID_HANDLE_VALUE||!WriteFile(file,packet.data,packet.size,&done,0)||done!=packet.size)ExitProcess(5);CloseHandle(file);ExitProcess(0);}
