#include "WorldPacker.cpp"
#include "RenderAttributes.h"
#include "RenderFrameAggregation.h"
#include "RenderClipVertex.h"
#include "SourceRenderFrameRecord.h"
#include "SourceRenderStaging.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
static void FailureReport(const char* reason,const SourceRenderStaging::Input* endpoints,unsigned int count){
 char path[MAX_PATH];DWORD length=GetEnvironmentVariableA("FEAR_SOURCE_FRAME_ERROR_OUTPUT",path,MAX_PATH);if(!length||length>=MAX_PATH)return;
 HANDLE file=CreateFileA(path,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(file==INVALID_HANDLE_VALUE)return;DWORD done;
 if(reason)WriteFile(file,reason,lstrlenA(reason),&done,0);
 for(unsigned int i=0;i<count;i++){
  RenderFrameSort::Record record;float uv[8];unsigned int color;const SourceRenderStaging::Input& in=endpoints[i];
  bool frame=SourceRenderFrameRecord::Build(in.vertex,in.bases,in.plane,in.group,in.mode,i,record,RenderExceptionalUV::Enabled());
  bool coordinates=RenderTextureCoordinates::Build(in.vertex.point,in.bases,uv,RenderExceptionalUV::Enabled());
  bool rgba=SourceRenderColor::PackWhiteRGB(in.vertex.attributes[3],color);
  if(!frame||!coordinates||!rgba){char text[96];lstrcpyA(text,"\r\nFirst invalid endpoint: ");char* at=text+lstrlenA(text);at=Decimal(at,i);*at=0;WriteFile(file,text,lstrlenA(text),&done,0);unsigned int flags[4]={i,frame?1u:0u,coordinates?1u:0u,rgba?1u:0u};WriteFile(file,"\r\nDiagnostic U32s: ",19,&done,0);for(unsigned int j=0;j<4;j++){*Decimal(text,flags[j])=0;WriteFile(file,text,lstrlenA(text),&done,0);WriteFile(file," ",1,&done,0);}break;}
 }
 WriteFile(file,"\r\n",2,&done,0);CloseHandle(file);
}
extern "C" void HarnessEntry(){
 char input[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_VERTEX_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_VERTEX_OUTPUT",output,MAX_PATH))ExitProcess(1);
 HANDLE in=CreateFileA(input,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);DWORD done;unsigned int count=0;
 if(in==INVALID_HANDLE_VALUE||!ReadFile(in,&count,4,&done,0)||done!=4||!count||count>RenderFrameLimits::MaximumEndpoints)ExitProcess(2);
 DWORD high=0,bytes=GetFileSize(in,&high);if(high||bytes!=4u+count*208u)ExitProcess(3);
 PrefabScene::Arena memory;SourceRenderStaging::Input* endpoints=(SourceRenderStaging::Input*)memory.Allocate(count*sizeof(SourceRenderStaging::Input));
 if(!endpoints||!ReadFile(in,endpoints,count*208,&done,0)||done!=count*208)ExitProcess(3);unsigned char tail;if(!ReadFile(in,&tail,1,&done,0)||done)ExitProcess(3);CloseHandle(in);
 SourceRenderStaging::Builder staging;if(!staging.Build(endpoints,count,RenderExceptionalUV::Enabled())){FailureReport(staging.error,endpoints,count);ExitProcess(4);}
 HANDLE out=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);
 if(out==INVALID_HANDLE_VALUE||!WriteFile(out,staging.frames,count*40,&done,0)||done!=count*40)ExitProcess(5);CloseHandle(out);
 char vertexOutput[MAX_PATH];if(GetEnvironmentVariableA("FEAR_SOURCE_STAGING_OUTPUT",vertexOutput,MAX_PATH)){
  HANDLE vertices=CreateFileA(vertexOutput,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);
  if(vertices==INVALID_HANDLE_VALUE||!WriteFile(vertices,staging.vertices,count*84,&done,0)||done!=count*84)ExitProcess(5);CloseHandle(vertices);
 }
 char cleanedOutput[MAX_PATH];if(GetEnvironmentVariableA("FEAR_SOURCE_CLEANED_STAGING_OUTPUT",cleanedOutput,MAX_PATH)){
  if(!staging.SnapPositions())ExitProcess(6);
  HANDLE vertices=CreateFileA(cleanedOutput,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);
  if(vertices==INVALID_HANDLE_VALUE||!WriteFile(vertices,staging.vertices,count*84,&done,0)||done!=count*84)ExitProcess(5);CloseHandle(vertices);
 }
 ExitProcess(0);
}
