// Research-only render worker: records render ownership input polygons and their
// contours whose input lies in the box FEAR_OWNERSHIP_TRACE_BOX (minx miny minz maxx
// maxy maxz as 6 raw float hex words) into FEAR_OWNERSHIP_TRACE_FILE. Not a production worker.
#define FEAR_OWNERSHIP_TRACE 1
#define FEAR_SMOOTH_TRACE 1
#define FEAR_FRAME_TRACE 1
#define FEAR_FRAMESORT_DUMP 1
#define FEAR_SECTOR_PLANES_DUMP 1
#define FEAR_SHARED_PICK 1
#include "native-render-worker.cpp"
static HANDLE TraceFile(){static HANDLE h=INVALID_HANDLE_VALUE;static bool opened=false;if(!opened){opened=true;char path[MAX_PATH];if(GetEnvironmentVariableA("FEAR_OWNERSHIP_TRACE_FILE",path,MAX_PATH))h=CreateFileA(path,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);}return h;}
static bool TraceBox(float* box){static float b[6];static int state=0;if(!state){state=2;char text[128];if(GetEnvironmentVariableA("FEAR_OWNERSHIP_TRACE_BOX",text,128)){const char* p=text;for(int i=0;i<6;i++){unsigned int v=0;while(*p==' ')p++;for(;*p&&*p!=' ';p++)v=v*16+(unsigned int)(*p<='9'?*p-'0':(*p|32)-'a'+10);memcpy(&b[i],&v,4);}state=1;}}if(state!=1)return false;memcpy(box,b,24);return true;}
static void Put(HANDLE h,const void* d,unsigned int n){DWORD done=0;WriteFile(h,d,n,&done,0);}
void FearOwnershipTrace(const RenderAttributedOwnership::Polygon& polygon,const RenderAttributedOwnership::Builder& ownership,unsigned int node,unsigned int index,unsigned int fragment){
 float box[6];HANDLE h=TraceFile();if(h==INVALID_HANDLE_VALUE||!TraceBox(box))return;bool hit=false;
 for(unsigned int i=0;i<polygon.count&&!hit;i++){const WorldText::Vec3& p=polygon.points[i].point;hit=p.x>=box[0]&&p.y>=box[1]&&p.z>=box[2]&&p.x<=box[3]&&p.y<=box[4]&&p.z<=box[5];}
 if(!hit)return;unsigned int head[4]={node,index,fragment,polygon.count};Put(h,head,16);Put(h,&polygon.normal,12);
 for(unsigned int i=0;i<polygon.count;i++)Put(h,&polygon.points[i].point,12);
 Put(h,&ownership.count,4);for(unsigned int c=0;c<ownership.count;c++){const RenderAttributedOwnership::Contour& k=ownership.contours[c];Put(h,&k.owner,4);Put(h,&k.count,4);for(unsigned int i=0;i<k.count;i++)Put(h,&k.vertices[i].point,12);}
}

// Smooth-normal trace: records corners whose output normal equals FEAR_SMOOTH_TARGET
// (3 raw float hex words): point, polygon, then every face sharing the point.
static bool SmoothTarget(float* t){static float b[3];static int state=0;if(!state){state=2;char text[64];if(GetEnvironmentVariableA("FEAR_SMOOTH_TARGET",text,64)){const char* p=text;for(int i=0;i<3;i++){unsigned int v=0;while(*p==' ')p++;for(;*p&&*p!=' ';p++)v=v*16+(unsigned int)(*p<='9'?*p-'0':(*p|32)-'a'+10);memcpy(&b[i],&v,4);}state=1;}}if(state!=1)return false;memcpy(t,b,12);return true;}
void FearSmoothTrace(const WorldText::Geometry& g,const WorldText::Vec3* faces,unsigned int polygon,unsigned int point,float threshold,const WorldText::Vec3& output){
 float t[3];HANDLE h=TraceFile();if(h==INVALID_HANDLE_VALUE||!SmoothTarget(t)){return;}{const unsigned char* a=(const unsigned char*)t;const unsigned char* b=(const unsigned char*)&output;for(int i=0;i<12;i++)if(a[i]!=b[i])return;}
 unsigned int head[3]={0x534d4f4fu,polygon,point};Put(h,head,12);Put(h,&g.vertices[point],12);Put(h,&threshold,4);
 unsigned int n=0;for(unsigned int p=0;p<g.polygons;p++){const WorldText::Polygon& f=g.faces[p];for(unsigned int j=0;j<f.count;j++)if(f.indices[j]==point){n++;break;}}Put(h,&n,4);
 for(unsigned int p=0;p<g.polygons;p++){const WorldText::Polygon& f=g.faces[p];for(unsigned int j=0;j<f.count;j++)if(f.indices[j]==point){Put(h,&p,4);Put(h,&faces[p],12);Put(h,&f.count,4);for(unsigned int k=0;k<f.count;k++)Put(h,&g.vertices[f.indices[k]],12);break;}}
}
// Frame trace: when a final normal/tangent/binormal equals FEAR_SMOOTH_TARGET, dump the
// index, all records within x +-0.2 of it (sorted order) and the final frame.
void FearFrameTrace(const RenderFrameSort::Record* records,unsigned int count,unsigned int i,const WorldText::Vec3& n,const WorldText::Vec3& p,const WorldText::Vec3& q){
 float t[3];HANDLE h=TraceFile();if(h==INVALID_HANDLE_VALUE||!SmoothTarget(t))return;
 bool hit=false;const WorldText::Vec3* v[3]={&n,&p,&q};for(int k=0;k<3&&!hit;k++){const unsigned char* a=(const unsigned char*)t;const unsigned char* b=(const unsigned char*)v[k];bool same=true;for(int z=0;z<12;z++)if(a[z]!=b[z])same=false;hit=same;}
 if(!hit)return;float x=RenderFrameSort::X(records[i]);unsigned int first=i,last=i;while(first&&RenderFrameSort::X(records[first-1])>=x-0.2f)first--;while(last+1<count&&RenderFrameSort::X(records[last+1])<=x+0.2f)last++;
 unsigned int head[4]={0x4652414du,i,first,last-first+1};Put(h,head,16);Put(h,&n,12);Put(h,&p,12);Put(h,&q,12);for(unsigned int j=first;j<=last;j++)Put(h,records[j].bytes,72);
}
// Frame sort dump: every RenderFrameSort::Build call writes u32 bytes + records before
// and after sorting into FEAR_FRAMESORT_TRACE (same framing as the original proxy).
void FearFrameSortDump(const RenderFrameSort::Record* records,unsigned int count){
 static HANDLE h=INVALID_HANDLE_VALUE;static bool opened=false;if(!opened){opened=true;char path[MAX_PATH];if(GetEnvironmentVariableA("FEAR_FRAMESORT_TRACE",path,MAX_PATH))h=CreateFileA(path,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);}
 if(h==INVALID_HANDLE_VALUE)return;unsigned int bytes=count*72;Put(h,&bytes,4);Put(h,records,bytes);
}
// Sector planes (once, first ownership build): 'PLNS', count, count*16 bytes, in our order.
void FearSectorPlanesDump(const float* planes,unsigned int count){
 static HANDLE h=INVALID_HANDLE_VALUE;static bool opened=false;if(!opened){opened=true;char path[MAX_PATH];if(GetEnvironmentVariableA("FEAR_SECTORPLANES_TRACE",path,MAX_PATH))h=CreateFileA(path,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);}
 if(h==INVALID_HANDLE_VALUE)return;unsigned int head[2]={0x534e4c50u,count};Put(h,head,8);Put(h,planes,count*16);
}void FearSectorSourceDump(unsigned int index,unsigned int node){
 static HANDLE h=INVALID_HANDLE_VALUE;static bool opened=false;if(!opened){opened=true;char path[MAX_PATH];if(GetEnvironmentVariableA("FEAR_SECTORSOURCE_TRACE",path,MAX_PATH))h=CreateFileA(path,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);}
 if(h==INVALID_HANDLE_VALUE)return;unsigned int pair[2]={index,node};Put(h,pair,8);
}// Shared prototype pick: for a representative with exactly FEAR_SHARED_COUNT members,
// compile the FEAR_SHARED_PICK-th member (group order) instead of the first.
static unsigned int EnvNumber(const char* name,unsigned int fallback){char text[16];if(!GetEnvironmentVariableA(name,text,16))return fallback;unsigned int v=0;for(const char* p=text;*p>='0'&&*p<='9';p++)v=v*10+(unsigned int)(*p-'0');return v;}
unsigned int FearSharedPick(unsigned int g,const unsigned int* representative,unsigned int groups,const unsigned int* counts){
 unsigned int want=EnvNumber("FEAR_SHARED_PICK",0),size=EnvNumber("FEAR_SHARED_COUNT",0),members[64],n=0;
 for(unsigned int h=g;h<groups&&n<64;h++)if(representative[h]==g)members[n++]=h;
 if(!want||n!=size||want>=n||counts[members[want]]!=counts[g])return g;return members[want];
}