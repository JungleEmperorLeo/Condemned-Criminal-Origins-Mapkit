#include "WorldPacker.cpp"
#include "RenderBlock.h"
#include "RenderAttributes.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
static char path[MAX_PATH];
static unsigned char* readEnv(const char* name,unsigned int* size){
 if(!GetEnvironmentVariableA(name,path,MAX_PATH))ExitProcess(1);
 HANDLE file=CreateFileA(path,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);if(file==INVALID_HANDLE_VALUE)ExitProcess(2);
 *size=GetFileSize(file,0);if(!*size||*size>16*1024*1024)ExitProcess(3);
 unsigned char* data=(unsigned char*)VirtualAlloc(0,*size,MEM_COMMIT|MEM_RESERVE,4);DWORD done;
 if(!data||!ReadFile(file,data,*size,&done,0)||done!=*size)ExitProcess(4);CloseHandle(file);return data;
}
extern "C" void HarnessEntry(){
 if(!GetEnvironmentVariableA("FEAR_READER_INPUT",path,MAX_PATH))ExitProcess(1);PrefabScene::Source source;if(!source.Load(path))ExitProcess(2);
 unsigned int method=0;float crease=-1;char modeEnv[4];bool testModes=GetEnvironmentVariableA("FEAR_RENDER_TANGENT_TEST",modeEnv,4)!=0;const WorldText::Geometry* g=0;for(unsigned int i=0;i<source.scene.nodeCount;i++){const WorldText::SceneNode& n=source.scene.nodes[i];if(n.kind==1&&n.knownType&&(n.surfaceFlags&8)){g=&source.scene.geometry[n.geometry];if(testModes){char value[64]="World";unsigned int prop=source.scene.Property(n.property,"TangentMethod");if(prop)source.scene.doc.Copy(source.scene.Data(prop),value,64);method=FearBrushFlags::Equal(value,"Smooth")?4:FearBrushFlags::Equal(value,"Kaldera")?8:2;prop=source.scene.Property(n.property,"CreaseAngle");crease=45;if(prop&&!source.scene.doc.Number(source.scene.Data(prop),crease))ExitProcess(7);}break;}}
 char colorProfile[4]={};bool authoredWhiteRGB=GetEnvironmentVariableA("FEAR_RENDER_AUTHORED_WHITE_RGB",colorProfile,4)&&colorProfile[0]=='1';
 if(!g)ExitProcess(3);RenderAttributes::Mesh mesh;if(!mesh.Build(*g,0xffffffff,4,0,crease,method,authoredWhiteRGB))ExitProcess(4);
 const unsigned short offsets[9]={0,12,20,28,36,44,48,60,72};const unsigned char types[9]={2,1,1,1,1,4,2,2,2},usage[9]={0,5,5,5,5,10,3,6,7},channels[9]={0,0,1,2,3,0,0,0,0};unsigned char declaration[80]={};
 for(unsigned int i=0;i<9;i++){*(unsigned short*)(declaration+i*8+2)=offsets[i];declaration[i*8+4]=types[i];declaration[i*8+6]=usage[i];declaration[i*8+7]=channels[i];}declaration[72]=255;declaration[76]=17;
 RenderBlock::Block block;if(!block.BuildSingle(mesh.vertices,mesh.count,84,mesh.indices,mesh.triangles,declaration,80,g->faces[0].material))ExitProcess(5);
 if(!GetEnvironmentVariableA("FEAR_RENDER_BLOCK",path,MAX_PATH))ExitProcess(1);HANDLE file=CreateFileA(path,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);DWORD done;
 if(file==INVALID_HANDLE_VALUE||!WriteFile(file,block.data,block.size,&done,0)||done!=block.size)ExitProcess(6);CloseHandle(file);ExitProcess(0);
}
