#pragma once
// Source-only shadow topology. Each LOD must be supplied as a separate pass.
namespace ShadowMesh {
struct Triangle {WorldText::Vec3 p[3],normal;};
struct Vertex {WorldText::Vec3 position,normal;};
class Mesh {
 PrefabScene::Arena memory;
 static bool Same(const WorldText::Vec3& a,const WorldText::Vec3& b){return a.x==b.x&&a.y==b.y&&a.z==b.z;}
 static float Abs(float x){return x<0?-x:x;}
 unsigned short Weld(const WorldText::Vec3& p,const WorldText::Vec3& normal,const WorldText::Vec3& offset){
  Vertex v={{p.x-offset.x,p.y-offset.y,p.z-offset.z},normal};
  for(unsigned int i=0;i<count;i++){const float* a=(const float*)&v;const float* b=(const float*)&vertices[i];unsigned int j=0;for(;j<6;j++)if(Abs(a[j]-b[j])>=.0001f)break;if(j==6)return (unsigned short)i;}
  vertices[count]=v;return (unsigned short)count++;
 }
 void Tri(unsigned short a,unsigned short b,unsigned short c){indices[triangles*3]=a;indices[triangles*3+1]=b;indices[triangles*3+2]=c;triangles++;}
 void Quad(unsigned short a,unsigned short b,unsigned short c,unsigned short d,bool reverse){
  if(a==d&&b==c)return;
  if(reverse){Tri(b,a,d);Tri(b,d,c);}else{Tri(b,a,c);Tri(a,d,c);}
 }
 bool Fail(const char* msg){memory.Clear();vertices=0;indices=0;count=triangles=0;error=msg;return false;}
public:
 Vertex* vertices;unsigned short* indices;unsigned int count,triangles;const char* error;
 Mesh():vertices(0),indices(0),count(0),triangles(0),error(0){}
 bool Build(const Triangle* input,unsigned int n,const WorldText::Vec3& offset){
  memory.Clear();vertices=0;indices=0;count=triangles=0;error=0;
  if(!input||!n||n>6000)return Fail("Unsupported shadow triangle count");
  if(!WorldText::Finite(offset.x)||!WorldText::Finite(offset.y)||!WorldText::Finite(offset.z))return Fail("Invalid shadow offset");
  for(unsigned int i=0;i<n;i++){const float* values=(const float*)&input[i];for(unsigned int j=0;j<12;j++)if(!WorldText::Finite(values[j]))return Fail("Nonfinite shadow triangle");if(Same(input[i].p[0],input[i].p[1])||Same(input[i].p[1],input[i].p[2])||Same(input[i].p[2],input[i].p[0]))return Fail("Repeated shadow corner");if(input[i].normal.x==0&&input[i].normal.y==0&&input[i].normal.z==0)return Fail("Zero shadow normal");}
  int* adj=(int*)memory.Allocate(n*3*4);int* component=(int*)memory.Allocate(n*4);unsigned int* queue=(unsigned int*)memory.Allocate(n*4);bool* two=(bool*)memory.Allocate(n);unsigned short* front=(unsigned short*)memory.Allocate(n*6);unsigned short* back=(unsigned short*)memory.Allocate(n*6);
  vertices=(Vertex*)memory.Allocate(n*6*sizeof(Vertex));indices=(unsigned short*)memory.Allocate(n*24*6);
  if(!adj||!component||!queue||!two||!front||!back||!vertices||!indices)return Fail("Shadow allocation failed");
  for(unsigned int i=0;i<n;i++){
   component[i]=-1;two[i]=false;
   for(unsigned int j=0;j<3;j++){
    const WorldText::Vec3& a=input[i].p[j];const WorldText::Vec3& b=input[i].p[(j+1)%3];adj[i*3+j]=-1;bool duplicate=false;
    for(unsigned int k=0;k<i&&!duplicate;k++)for(unsigned int q=0;q<3;q++)if(Same(a,input[k].p[q])&&Same(b,input[k].p[(q+1)%3])){duplicate=true;break;}
    if(duplicate)continue;
    for(unsigned int k=0;k<n;k++){bool found=false;for(unsigned int q=0;q<3;q++)if(Same(b,input[k].p[q])&&Same(a,input[k].p[(q+1)%3])){found=true;break;}if(found){adj[i*3+j]=(int)k;break;}}
   }
  }
  for(unsigned int i=0;i<n;i++)for(unsigned int j=0;j<3;j++){int k=adj[i*3+j];if(k<0)continue;bool linked=false;for(unsigned int q=0;q<3;q++)if(adj[k*3+q]==(int)i)linked=true;if(!linked)adj[i*3+j]=-1;}
  for(unsigned int start=0;start<n;start++){
   if(component[start]!=-1)continue;unsigned int head=0,tail=1;queue[0]=start;component[start]=(int)start;bool closed=true;
   while(head<tail){unsigned int i=queue[head++];for(unsigned int j=0;j<3;j++){int k=adj[i*3+j];if(k<0){closed=false;continue;}if(component[k]==-1){component[k]=(int)start;queue[tail++]=(unsigned int)k;}}}
   float maximum[3]={-3.402823466e38f,-3.402823466e38f,-3.402823466e38f};bool positive[3]={true,true,true};
   for(unsigned int i=0;i<n;i++)if(component[i]==(int)start){const Triangle& t=input[i];WorldText::Vec3 a={t.p[1].x-t.p[0].x,t.p[1].y-t.p[0].y,t.p[1].z-t.p[0].z},b={t.p[2].x-t.p[0].x,t.p[2].y-t.p[0].y,t.p[2].z-t.p[0].z};float cross[3]={a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x};const float* p=(const float*)&t.p[0];for(unsigned int k=0;k<3;k++)if(p[k]>maximum[k]&&Abs(cross[k])>.0010000000474974513f){maximum[k]=p[k];positive[k]=cross[k]>0;}}
   bool doubles=!closed||!positive[0]||!positive[1]||!positive[2];for(unsigned int i=0;i<tail;i++)two[queue[i]]=doubles;
  }
  for(unsigned int i=0;i<n;i++){
   for(unsigned int j=0;j<3;j++)front[i*3+j]=Weld(input[i].p[j],input[i].normal,offset);Tri(front[i*3],front[i*3+1],front[i*3+2]);
   if(two[i]){WorldText::Vec3 neg={-input[i].normal.x,-input[i].normal.y,-input[i].normal.z};for(unsigned int j=0;j<3;j++)back[i*3+j]=Weld(input[i].p[j],neg,offset);Tri(back[i*3],back[i*3+2],back[i*3+1]);}
  }
  for(unsigned int i=0;i<n;i++)for(unsigned int j=0;j<3;j++){
   unsigned int end=(j+1)%3;int k=adj[i*3+j];
   if(k<0){if(!two[i])return Fail("One-sided shadow boundary");Quad(front[i*3+j],front[i*3+end],back[i*3+end],back[i*3+j],false);}
   else if(i<(unsigned int)k){unsigned int q=0;for(;q<3;q++)if(Same(input[k].p[q],input[i].p[end])&&Same(input[k].p[(q+1)%3],input[i].p[j]))break;if(q==3||two[i]!=two[k])return Fail("Inconsistent shadow neighbor");unsigned int r=(q+1)%3;Quad(front[i*3+j],front[i*3+end],front[k*3+q],front[k*3+r],false);if(two[i])Quad(back[i*3+end],back[i*3+j],back[k*3+r],back[k*3+q],true);}
  }
  return true;
 }
};
}
