#define FEAR_PRIMARY_SURFACE_PREPARATION 1
#include "WorldPacker.cpp"
#include "SourcePhysicsHullPreprocess.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
namespace HullPreprocessGuards {
using namespace SourcePhysicsHullPreprocess;
static Vec4 large[65535];
static bool Equal(const void* a,const void* b,unsigned int bytes){const unsigned char* p=(const unsigned char*)a;const unsigned char* q=(const unsigned char*)b;for(unsigned int i=0;i<bytes;i++)if(p[i]!=q[i])return false;return true;}
static float Bits(unsigned int bits){float value;memcpy(&value,&bits,4);return value;}
static bool Same(float a,float b){return Equal(&a,&b,4);}
static bool RejectedEverywhere(Array& a){
 const Array initial=a;Vec4 span={11,12,13,14},center={21,22,23,24};const Vec4 oldSpan=span,oldCenter=center;int retained=123;
 return Normalize(&a,&span,&center)==InvalidInput&&Equal(&a,&initial,sizeof(a))&&Equal(&span,&oldSpan,16)&&Equal(&center,&oldCenter,16)&&Sort(&a)==InvalidInput&&Equal(&a,&initial,sizeof(a))&&Deduplicate(&a,.001f,&retained)==InvalidInput&&retained==123&&Equal(&a,&initial,sizeof(a))&&Unnormalize(&a,span,center)==InvalidInput&&Equal(&a,&initial,sizeof(a));
}
static bool RejectBadComponents(unsigned int bits){Vec4 p[2]={{0,0,0,0},{1,2,3,4}};for(unsigned int axis=0;axis<4;axis++){float* values=(float*)&p[1];float previous=values[axis];values[axis]=Bits(bits);Vec4 saved[2];memcpy(saved,p,32);Array a={p,2,2};if(!RejectedEverywhere(a)||!Equal(p,saved,32))return false;values[axis]=previous;}return true;}
static bool InvalidTolerance(float tolerance){Vec4 points[2]={{0,0,0,0},{0,.125f,0,0}},saved[2];memcpy(saved,points,32);Array a={points,2,2};int retained=99;return Deduplicate(&a,tolerance,&retained)==InvalidInput&&retained==99&&a.size==2&&a.capacity==2&&a.data==points&&Equal(points,saved,32);}
static bool InvalidUnnormalizeComponent(unsigned int bits,bool spanSide){for(unsigned int axis=0;axis<4;axis++){Vec4 p={1,2,3,4},saved=p,span={2,3,4,0},center={5,6,7,8};Array a={&p,1,1};float* values=(float*)(spanSide?&span:&center);values[axis]=Bits(bits);if(Unnormalize(&a,span,center)!=InvalidInput||!Equal(&p,&saved,16)||a.data!=&p||a.size!=1||a.capacity!=1)return false;}return true;}
static bool Write(HANDLE f,const void* bytes,unsigned int size){DWORD done=0;return WriteFile(f,bytes,size,&done,0)&&done==size;}
}
extern "C" void HarnessEntry(){using namespace HullPreprocessGuards;using namespace SourcePhysicsHullPreprocess;
 unsigned int checks[128]={},count=0;
#define CHECK(label,expression) checks[count++]=(expression)?1u:0u
 Vec4 points[4]={{2,3,4,7},{0,1,2,9},{1,2,3,11},{1,2,3,13}};const Vec4 reference[4]={{2,3,4,7},{0,1,2,9},{1,2,3,11},{1,2,3,13}};Array a={points,4,4};Vec4 span={0,0,0,0},center={0,0,0,0};int retained=77;
 CHECK("null normalize array preserves outputs",Normalize(0,&span,&center)==InvalidInput&&span.x==0&&center.x==0);
 CHECK("null normalize span preserves points",Normalize(&a,0,&center)==InvalidInput&&Equal(points,reference,64));
 CHECK("null normalize center preserves points",Normalize(&a,&span,0)==InvalidInput&&Equal(points,reference,64));
 CHECK("null sort array rejected",Sort(0)==InvalidInput);
 CHECK("null deduplicate array preserves retained count",Deduplicate(0,.01f,&retained)==InvalidInput&&retained==77);
 CHECK("null retained pointer preserves points",Deduplicate(&a,.01f,0)==InvalidInput&&Equal(points,reference,64)&&a.size==4);
 CHECK("null unnormalize array rejected",Unnormalize(0,span,center)==InvalidInput);
 Array bad={points,0,4};CHECK("zero size rejected by all operations",RejectedEverywhere(bad));
 bad.size=-1;CHECK("negative size rejected by all operations",RejectedEverywhere(bad));
 bad.size=65536;bad.capacity=65536;CHECK("size budget checked before reading data",RejectedEverywhere(bad));
 bad.size=0x7fffffff;bad.capacity=0x7fffffffu;CHECK("huge size rejected before arithmetic",RejectedEverywhere(bad));
 bad.data=0;bad.size=1;bad.capacity=1;CHECK("null point array rejected",RejectedEverywhere(bad));
 bad.data=points;bad.size=4;bad.capacity=3;CHECK("insufficient capacity rejected",RejectedEverywhere(bad));
 bad.capacity=0x80000003u;CHECK("ownership flag cannot hide insufficient capacity",RejectedEverywhere(bad));
 bad.capacity=0;CHECK("zero capacity rejected",RejectedEverywhere(bad));
 bad.capacity=0x80000000u;CHECK("flag only capacity rejected",RejectedEverywhere(bad));
 CHECK("all invalid descriptor checks preserve source points",Equal(points,reference,64));
 CHECK("NaN in any point component rejected without mutation",RejectBadComponents(0x7fc12345u));
 CHECK("positive infinity in any component rejected",RejectBadComponents(0x7f800000u));
 CHECK("negative infinity in any component rejected",RejectBadComponents(0xff800000u));
 CHECK("negative squared distance rejected before mutation",InvalidTolerance(-1.f));
 CHECK("NaN squared distance rejected before mutation",InvalidTolerance(Bits(0x7fc01234u)));
 CHECK("positive infinite squared distance rejected",InvalidTolerance(Bits(0x7f800000u)));
 CHECK("negative infinite squared distance rejected",InvalidTolerance(Bits(0xff800000u)));
 CHECK("NaN span components rejected before mutation",InvalidUnnormalizeComponent(0x7fc00123u,true));
 CHECK("infinite span components rejected before mutation",InvalidUnnormalizeComponent(0x7f800000u,true));
 CHECK("NaN center components rejected before mutation",InvalidUnnormalizeComponent(0x7fc00123u,false));
 CHECK("infinite center components rejected before mutation",InvalidUnnormalizeComponent(0xff800000u,false));
 a.capacity=0x80000004u;CHECK("caller owned capacity flag accepted",Normalize(&a,&span,&center)==Success&&a.data==points&&a.size==4&&a.capacity==0x80000004u);
 CHECK("independent rectangle bounds and first W center",span.x==2&&span.y==2&&span.z==2&&span.w==0&&center.x==1&&center.y==2&&center.z==3&&center.w==7);
 CHECK("normalize produces independent half cube coordinates",points[0].x==.5f&&points[0].y==.5f&&points[0].z==.5f&&points[1].x==-.5f&&points[1].y==-.5f&&points[1].z==-.5f);
 CHECK("normalization clears every W",points[0].w==0&&points[1].w==0&&points[2].w==0&&points[3].w==0);
 CHECK("sort orders lexicographically",Sort(&a)==Success&&points[0].x==-.5f&&points[1].x==0&&points[2].x==0&&points[3].x==.5f);
 CHECK("positive threshold merges exact duplicate",Deduplicate(&a,.001f,&retained)==Success&&retained==3&&a.size==3&&a.capacity==0x80000004u);
 CHECK("unnormalize restores xyz and first W",Unnormalize(&a,span,center)==Success&&points[0].x==0&&points[0].y==1&&points[0].z==2&&points[1].x==1&&points[1].y==2&&points[1].z==3&&points[2].x==2&&points[2].y==3&&points[2].z==4&&points[0].w==7&&points[1].w==7&&points[2].w==7);
 Vec4 singleton={3,-4,5,7};Array single={&singleton,1,1};CHECK("singleton normalize accepted with zero span",Normalize(&single,&span,&center)==Success&&span.x==0&&span.y==0&&span.z==0&&span.w==0&&singleton.x==0&&singleton.y==0&&singleton.z==0&&singleton.w==0);
 CHECK("singleton sort accepted",Sort(&single)==Success);
 CHECK("singleton zero distance retains one",Deduplicate(&single,0,&retained)==Success&&retained==1&&single.size==1);
 CHECK("singleton round trip restores W",Unnormalize(&single,span,center)==Success&&singleton.x==3&&singleton.y==-4&&singleton.z==5&&singleton.w==7);
 Vec4 twins[2]={{0,0,0,7},{0,0,0,9}};Array pair={twins,2,2};CHECK("zero threshold does not merge exact duplicates",Deduplicate(&pair,0,&retained)==Success&&retained==2&&twins[0].w==7&&twins[1].w==9);
 CHECK("negative zero threshold is accepted and strict",Deduplicate(&pair,Bits(0x80000000u),&retained)==Success&&retained==2);
 CHECK("minimum positive threshold merges identical xyz",Deduplicate(&pair,Bits(1),&retained)==Success&&retained==1&&twins[0].w==7);
 twins[0]=Vec4{0,0,0,7};twins[1]=Vec4{0,.125f,0,9};pair.size=2;CHECK("exact squared distance below threshold boundary retained",Deduplicate(&pair,Bits(0x3c7fffffu),&retained)==Success&&retained==2);
 CHECK("exact squared distance equal threshold retained",Deduplicate(&pair,.015625f,&retained)==Success&&retained==2);
 CHECK("exact squared distance above threshold merges",Deduplicate(&pair,Bits(0x3c800001u),&retained)==Success&&retained==1&&twins[0].w==7);
 twins[0]=Vec4{0,0,0,7};twins[1]=Vec4{.02f,0,0,9};pair.size=2;CHECK("fixed X search window limits even large squared threshold",Deduplicate(&pair,1.f,&retained)==Success&&retained==2);
 twins[0]=Vec4{0,0,0,7};twins[1]=Vec4{0,0,.02f,9};pair.size=2;CHECK("same X large threshold merges Z separation",Deduplicate(&pair,1.f,&retained)==Success&&retained==1);
 for(unsigned int i=0;i<4;i++)points[i]=reference[i];a.size=4;a.capacity=0xffffffffu;CHECK("large flagged capacity does not change logical extent",Sort(&a)==Success&&a.size==4&&a.capacity==0xffffffffu&&points[0].x==0&&points[3].x==2);
 CHECK("sort preserves all four W payload values",(points[0].w==9)&&(points[3].w==7)&&((points[1].w==11&&points[2].w==13)||(points[1].w==13&&points[2].w==11)));
 Vec4 threshold[2]={{0,0,0,0},{Bits(0x34000000u),2,2,0}};Array narrow={threshold,2,2};CHECK("equal normalization threshold uses unit inverse",Normalize(&narrow,&span,&center)==Success&&Same(threshold[0].x,-Bits(0x33800000u))&&Same(threshold[1].x,Bits(0x33800000u)));
 threshold[0]=Vec4{0,0,0,0};threshold[1]=Vec4{Bits(0x34000001u),2,2,0};CHECK("next float above normalization threshold scales to half",Normalize(&narrow,&span,&center)==Success&&threshold[0].x==-.5f&&threshold[1].x==.5f);
 Vec4 first[2]={{0,0,0,1},{2,4,6,3}},second[2]={{10,20,30,9},{20,40,60,11}};Array aa={first,2,2},bb={second,2,2};Vec4 spanA,centerA,spanB,centerB;
 CHECK("independent arrays normalize without shared state",Normalize(&aa,&spanA,&centerA)==Success&&Normalize(&bb,&spanB,&centerB)==Success&&centerA.x==1&&centerB.x==15&&centerA.w==1&&centerB.w==9);
 CHECK("interleaved inverse uses caller held first transform",Unnormalize(&aa,spanA,centerA)==Success&&first[0].x==0&&first[1].x==2&&first[0].w==1);
 CHECK("interleaved inverse uses caller held second transform",Unnormalize(&bb,spanB,centerB)==Success&&second[0].x==10&&second[1].x==20&&second[0].w==9);
 Vec4 emptySpan={0,0,0,0},chosenCenter={7,8,9,10};CHECK("finite zero inverse span maps to supplied center",Unnormalize(&aa,emptySpan,chosenCenter)==Success&&first[0].x==7&&first[1].x==7&&first[0].y==8&&first[0].z==9&&first[0].w==10);
 Vec4 negativeSpan={-1,-2,-3,123},zeroCenter={0,0,0,0};CHECK("finite negative span is valid arithmetic input",Unnormalize(&aa,negativeSpan,zeroCenter)==Success&&first[0].x==-7&&first[0].y==-16&&first[0].z==-27&&first[0].w==0);
 Vec4 huge[2]={{Bits(0x7f7fffffu),0,0,0},{Bits(0x7f7fffffu),1,1,0}};Array overflow={huge,2,2};CHECK("center addition overflow returns nonfinite result",Normalize(&overflow,&span,&center)==NonfiniteResult&&!Finite(center.x));
 CHECK("arithmetic failure is explicitly in place and may mutate points",!Valid(overflow));
 huge[0]=Vec4{-Bits(0x7f7fffffu),0,0,0};huge[1]=Vec4{Bits(0x7f7fffffu),1,1,0};CHECK("span subtraction overflow returns nonfinite result",Normalize(&overflow,&span,&center)==NonfiniteResult&&!Finite(span.x));
 singleton=Vec4{2,0,0,0};Vec4 hugeSpan={Bits(0x7f7fffffu),1,1,0};CHECK("inverse multiplication overflow returns nonfinite result",Unnormalize(&single,hugeSpan,zeroCenter)==NonfiniteResult&&!Finite(singleton.x));
 singleton=Vec4{Bits(0x7f7fffffu),0,0,0};Vec4 oneSpan={1,1,1,0},hugeCenter={Bits(0x7f7fffffu),0,0,0};CHECK("inverse translation overflow returns nonfinite result",Unnormalize(&single,oneSpan,hugeCenter)==NonfiniteResult&&!Finite(singleton.x));
 singleton=Vec4{1,2,3,4};CHECK("same API reusable after arithmetic failure",Normalize(&single,&span,&center)==Success&&Unnormalize(&single,span,center)==Success&&singleton.x==1&&singleton.y==2&&singleton.z==3&&singleton.w==4);
 Array maximum={large,65535,0x8000ffffu};CHECK("maximum logical count accepted without allocation",Valid(maximum)&&Sort(&maximum)==Success&&maximum.size==65535);
 CHECK("maximum duplicate run compacts inside original capacity",Deduplicate(&maximum,.001f,&retained)==Success&&retained==1&&maximum.size==1&&maximum.capacity==0x8000ffffu&&maximum.data==large);
 CHECK("caller buffers remain independent after maximum run",singleton.x==1&&singleton.y==2&&singleton.z==3&&singleton.w==4&&aa.data==first&&bb.data==second);
 char output[MAX_PATH];DWORD length=GetEnvironmentVariableA("FEAR_HULL_PREPROCESS_GUARDS_OUTPUT",output,MAX_PATH);if(!length||length>=MAX_PATH)ExitProcess(1);HANDLE file=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);unsigned int header[2]={0x31485047u,count};if(file==INVALID_HANDLE_VALUE||!Write(file,header,8)||!Write(file,checks,count*4))ExitProcess(2);CloseHandle(file);for(unsigned int i=0;i<count;i++)if(!checks[i])ExitProcess(9);ExitProcess(0);
}
