#define FEAR_PRIMARY_SURFACE_PREPARATION 1
#include "WorldPacker.cpp"
#include "NativePhysicsOwnerMeshTransport.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
namespace T=NativePhysicsOwnerMeshTransport;
static void Check(bool v,unsigned int id){if(!v)ExitProcess(100+id);}
static unsigned int Packet(unsigned char* p,unsigned int count){T::detail::PutWord(p,T::AssociationMagic);T::detail::PutWord(p+4,1);T::detail::PutWord(p+8,count);return 12;}
static unsigned int Add(unsigned char* p,unsigned int at,unsigned int group,unsigned int index,unsigned int mesh=1,unsigned int code=1){T::detail::PutWord(p+at,group);T::detail::PutWord(p+at+4,index);T::detail::PutWord(p+at+8,mesh);T::detail::PutWord(p+at+12,code);at+=16;for(unsigned int i=0;i<mesh+code;i++)p[at+i]=(unsigned char)(i+7);return at+mesh+code;}
static bool Empty(const T::Parser& p){return !p.data&&!p.size&&!p.count&&!p.records&&p.error;}
extern "C" void HarnessEntry(){
 T::Parser p;unsigned char packet[256]={};unsigned int size=Packet(packet,0);Check(p.Build(packet,size)&&p.count==0&&p.size==12&&p.data!=packet&&p.records==0,1);
 size=Add(packet,Packet(packet,1),7,3,2,3);Check(p.Build(packet,size)&&p.count==1&&p.records[0].group==7&&p.records[0].modelIndex==3&&p.records[0].meshBytes==2&&p.records[0].codeBytes==3,2);
 const unsigned char* mesh=(const unsigned char*)p.records[0].mesh;const unsigned char* code=(const unsigned char*)p.records[0].code;packet[28]=99;Check(mesh[0]==7&&mesh[1]==8&&code[0]==9&&code[2]==11,3);
 Check(mesh>=p.data&&code+3<=p.data+p.size&&p.data!=packet,4);
 const void* own=p.data;unsigned int ownBytes=p.size;Check(!p.Build(own,ownBytes)&&p.errorCode==T::InvalidInput&&Empty(p),5);
 size=Add(packet,Packet(packet,1),7,3);Check(p.Build(packet,size),6);const void* recordStorage=p.records;Check(!p.Build(recordStorage,12)&&p.errorCode==T::InvalidInput&&Empty(p),7);
 Check(!p.Build(0,12)&&p.errorCode==T::InvalidInput&&Empty(p),8);
 for(unsigned int n=0;n<12;n++)Check(!p.Build(packet,n)&&p.errorCode==T::TruncatedPacket&&Empty(p),9);
 Check(!p.Build(packet,T::MaximumBytes+1)&&p.errorCode==T::ResourceLimit&&Empty(p),10);
 size=Packet(packet,0);packet[0]^=1;Check(!p.Build(packet,size)&&p.errorCode==T::InvalidHeader&&Empty(p),11);size=Packet(packet,0);T::detail::PutWord(packet+4,2);Check(!p.Build(packet,size)&&p.errorCode==T::InvalidHeader&&Empty(p),12);
 size=Packet(packet,T::MaximumRecords+1);Check(!p.Build(packet,size)&&p.errorCode==T::ResourceLimit&&Empty(p),13);
 size=Add(packet,Packet(packet,1),0,1);Check(!p.Build(packet,size)&&p.errorCode==T::InvalidIdentity&&Empty(p),14);size=Add(packet,Packet(packet,1),1,0);Check(!p.Build(packet,size)&&p.errorCode==T::InvalidIdentity&&Empty(p),15);
 size=Add(packet,Packet(packet,1),1,2,0,1);Check(!p.Build(packet,size)&&Empty(p),16);size=Add(packet,Packet(packet,1),1,2,1,0);Check(!p.Build(packet,size)&&Empty(p),17);
 size=Add(packet,Packet(packet,1),1,2,1,1);T::detail::PutWord(packet+20,0xffffffffu);Check(!p.Build(packet,size)&&p.errorCode==T::TruncatedPacket&&Empty(p),18);size=Add(packet,Packet(packet,1),1,2,1,1);T::detail::PutWord(packet+24,0xffffffffu);Check(!p.Build(packet,size)&&p.errorCode==T::TruncatedPacket&&Empty(p),19);
 size=Add(packet,Packet(packet,2),1,2);size=Add(packet,size,1,3);Check(!p.Build(packet,size)&&p.errorCode==T::DuplicateIdentity&&Empty(p),20);size=Add(packet,Packet(packet,2),1,2);size=Add(packet,size,4,2);Check(!p.Build(packet,size)&&p.errorCode==T::DuplicateIdentity&&Empty(p),21);
 size=Add(packet,Packet(packet,1),1,2);packet[size]=0;Check(!p.Build(packet,size+1)&&p.errorCode==T::TrailingBytes&&Empty(p),22);
 size=Packet(packet,0);packet[size]=0;Check(!p.Build(packet,size+1)&&p.errorCode==T::TrailingBytes&&Empty(p),23);
 size=Add(packet,Packet(packet,1),1,2,3,4);for(unsigned int n=12;n<size;n++)Check(!p.Build(packet,n)&&Empty(p),24);
 size=Add(packet,Packet(packet,2),9,5,3,2);size=Add(packet,size,2,7,4,1);Check(p.Build(packet,size)&&p.count==2&&p.records[0].group==9&&p.records[1].group==2&&p.records[1].modelIndex==7,25);
 p.Clear();Check(!p.error&&!p.data&&!p.records&&!p.size&&!p.count,26);
 // The transport deliberately accepts opaque nonempty bodies: later consumers
 // must perform the real mesh/MOPP validation instead of fake checks here.
 size=Add(packet,Packet(packet,1),0xffffffffu,T::MaximumRecords);Check(p.Build(packet,size)&&p.count==1&&p.records[0].modelIndex==T::MaximumRecords,27);
 T::Catalog catalog;PrefabScene::Arena memory;void* place=memory.Allocate(sizeof(NativePreparedRenderScene::Context));Check(place!=0,28);NativePreparedRenderScene::Context* scene=new(place)NativePreparedRenderScene::Context;Check(!catalog.Build(*scene)&&catalog.errorCode==T::InvalidScene&&!catalog.data&&!catalog.count&&!catalog.first&&!catalog.size,28);
 T::Settings settings;settings.version=2;Check(!catalog.Build(*scene,settings)&&catalog.errorCode==T::InvalidSettings&&!catalog.data,29);settings=T::Settings();settings.maximumBytes=T::MaximumBytes+1;Check(!catalog.Build(*scene,settings)&&catalog.errorCode==T::InvalidSettings,30);settings=T::Settings();settings.maximumBytes=11;Check(!catalog.Build(*scene,settings)&&catalog.errorCode==T::InvalidSettings,31);settings=T::Settings();settings.maximumRecords=0;Check(!catalog.Build(*scene,settings)&&catalog.errorCode==T::InvalidSettings,32);settings.maximumRecords=T::MaximumRecords+1;Check(!catalog.Build(*scene,settings)&&catalog.errorCode==T::InvalidSettings,33);
 Check(!catalog.WriteCatalog(packet,sizeof(packet)),34);
 unsigned int manySize=12+18*T::MaximumRecords;unsigned char* many=(unsigned char*)memory.Allocate(manySize);Check(many!=0,35);unsigned int at=Packet(many,T::MaximumRecords);for(unsigned int i=0;i<T::MaximumRecords;i++)at=Add(many,at,i+1,i+1);Check(at==manySize&&p.Build(many,manySize)&&p.count==T::MaximumRecords,35);
 unsigned char* maximum=(unsigned char*)memory.Allocate(T::MaximumBytes);Check(maximum!=0,36);at=Packet(maximum,1);at=Add(maximum,at,1,1,1,T::MaximumBytes-29);Check(at==T::MaximumBytes&&p.Build(maximum,at)&&p.size==T::MaximumBytes&&p.records[0].codeBytes==T::MaximumBytes-29,36);
 Check(!p.Build(packet,0)&&Empty(p),37);
 size=Add(packet,Packet(packet,1),1,2);Check(p.Build(packet,size),38);p.Clear();p.Clear();Check(!p.error&&!p.data&&!p.records&&!p.count&&!p.size,38);
 size=Add(packet,Packet(packet,1),1,T::MaximumRecords+1);Check(!p.Build(packet,size)&&p.errorCode==T::InvalidIdentity&&Empty(p),39);
 size=Add(packet,Packet(packet,1),1,0xffffffffu);Check(!p.Build(packet,size)&&p.errorCode==T::InvalidIdentity&&Empty(p),40);
 ExitProcess(0);
}
