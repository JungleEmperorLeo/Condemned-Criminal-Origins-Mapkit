#pragma once
#include "StalkNodes.h"
// Owned source -> SDK ImportRawNavMesh v6 input for authored worlds, using the
// raw construction recovered from the original WorldPacker.dll (research notes:
// nav-sdk-native/sp-oracle/SP_NAVMESH_RAW.md, omission-rules/OMISSION_RULES.md).
// Scene pre-order traversal:
//  - roots: outermost AINavMesh objects (VA 0x100055E0 does not descend into a
//    root); restriction per root from NavMeshType, MaskType and
//    CharacterType0..15 ("%s%d", missing or None skipped) (VA 0x10004940);
//  - brushes under a root (VA 0x10005360): a direct AINavMeshLink* parent with
//    at most one child is registered first (VA 0x10003100), then predicate
//    VA 0x10004E10 omits non-Normal, multi-polygon, IsConcave, normal.y<0 and
//    multi-child-link brushes; admitted corners are f32(point - offset);
//  - bounds: X/Z of admitted brushes only (VA 0x10001D70);
//  - links: one record per admitted brush with a link (VA 0x10002660), sensory
//    iff the class is AINavMeshLinkLoseTarget (VA 0x10002698);
//  - carvers: AINavMeshCarver brushes, projected silhouette of up-facing polygons
//    (VA 0x10033320, 0x10032EA0), y = AABB max.y, height = f32(max.y - min.y);
//    empty projections are dropped (VA 0x10003C00);
//  - AIRegion objects (VA 0x10005120): valid iff a direct Normal brush with a
//    polygon count other than one (VA 0x10004C10); then every direct such brush
//    of any Type is projected (VA 0x10003DD0) and descendants are not searched;
//    an invalid region's descendants are searched;
//  - nodes: AINodeCover objects (id, Name, f32(position - offset)).
// Class/Type names compare case-insensitively (_stricmp/_strnicmp imports).
// The original writes an inconsistent link count when a registered link's brush
// is omitted, and admits vertical polygons; both still fail with a diagnostic.
namespace NativeRawNavMeshGeneral {
static const unsigned int Missing=0xffffffffu;
struct Binding {unsigned int node;int value;};
class Builder {
 PrefabScene::Arena memory;Builder(const Builder&);Builder& operator=(const Builder&);
 struct Out {unsigned char* data;unsigned int size,capacity;bool good;
  void Bytes(const void* p,unsigned int n){if(!good||n>capacity-size){good=false;return;}memcpy(data+size,p,n);size+=n;}
  void Word(unsigned int v){Bytes(&v,4);}void Real(float v){Bytes(&v,4);}void Vec(const WorldText::Vec3& v){Bytes(&v,12);}
   void PatchWord(unsigned int at,unsigned int v){if(!good)return;if(at>size||size-at<4){good=false;return;}memcpy(data+at,&v,4);}
  void Text(const char* s,unsigned int n){unsigned short l=(unsigned short)n;Bytes(&l,2);Bytes(s,n);}};
 static char Lower(char c){return c>='A'&&c<='Z'?(char)(c+32):c;}
 static bool Same(const char* a,const char* b){if(!a||!b)return false;for(unsigned int i=0;i<4096;i++){if(Lower(a[i])!=Lower(b[i]))return false;if(!a[i])return true;}return false;}
 static bool Prefix(const char* a,const char* b){if(!a||!b)return false;for(;*b;a++,b++)if(!*a||Lower(*a)!=Lower(*b))return false;return true;}
 static bool Exact(const char* a,const char* b){if(!a||!b)return false;for(unsigned int i=0;i<4096;i++){if(a[i]!=b[i])return false;if(!a[i])return true;}return false;}
  static const char* Prop(const PrefabScene::Node& n,const char* name,const char* type){unsigned int count=0;for(const PrefabScene::Property* p=n.properties;p&&count<4096;p=p->next,count++)if(Exact(p->name,name))return p->data&&Exact(p->type,type)?p->data:0;return 0;}
  static bool PropertyValid(const PrefabScene::Node& n,const char* name,const char* type){unsigned int count=0,matches=0;for(const PrefabScene::Property* p=n.properties;p;p=p->next){if(count++==4096)return false;if(Exact(p->name,name)&&(++matches>1||!p->data||!Exact(p->type,type)))return false;}return true;}
 static unsigned int Length(const char* s,unsigned int limit){unsigned int n=0;while(s&&s[n]&&n<=limit){unsigned char c=(unsigned char)s[n];if(c<32||c>126)return limit+1;n++;}return n;}
  static WorldText::Vec3 Center(const WorldText::Vec3& p,const WorldText::Vec3& o){WorldText::Vec3 r={p.x-o.x,p.y-o.y,p.z-o.z};return r;}
  static bool GeometryValid(const WorldText::Geometry& g,const WorldText::Vec3& offset){
   if(!g.points||g.points>4096||!g.polygons||g.polygons>4096||!g.vertices||!g.faces)return false;
   for(unsigned int i=0;i<g.points;i++)if(!WorldText::FiniteVector(g.vertices[i])||!WorldText::FiniteVector(Center(g.vertices[i],offset)))return false;
   for(unsigned int i=0;i<g.polygons;i++){const WorldText::Polygon& p=g.faces[i];if(p.count<3||p.count>4096||!p.indices||!p.validPlane||!WorldText::FiniteVector(p.normal))return false;for(unsigned int j=0;j<p.count;j++)if(p.indices[j]>=g.points)return false;}
   return true;
  }
 bool Fail(const char* text,unsigned int node=Missing){data=0;size=0;error=text;errorNode=node;return false;}
 // CEditPoly::IsConcave as compiled at VA 0x10058350 with Norm at 0x10006980:
 // binary32 stores after each difference, Cross component, squared length,
 // root, reciprocal and scaled component; dots accumulate at 53 bits and are
 // compared strictly with 0.001f. Returns false with state 2 for a zero edge
 // normal, where the original raises its release assert.
 static bool Concave(const WorldText::Geometry& g,const WorldText::Polygon& f,const WorldText::Vec3& offset,unsigned int& state){
  state=0;if(f.count<=3)return false;
  const double nx=f.normal.x,ny=f.normal.y,nz=f.normal.z;unsigned int previous=f.count-1;
  for(unsigned int current=0;current<f.count;current++){
   WorldText::Vec3 a=Center(g.vertices[f.indices[current]],offset),b=Center(g.vertices[f.indices[previous]],offset);
   volatile float ez=(float)(double(a.z)-double(b.z)),ey=(float)(double(a.y)-double(b.y)),ex=(float)(double(a.x)-double(b.x));
   volatile float cz=(float)(ny*double(ex)-nx*double(ey)),cy=(float)(nx*double(ez)-nz*double(ex)),cx=(float)(nz*double(ey)-ny*double(ez));
   volatile float squared=(float)((double(cx)*double(cx)+double(cy)*double(cy))+double(cz)*double(cz));
   // Binary32 input: x87 fsqrt then a binary32 store is the correctly rounded root.
   float radicand=squared,magnitude;__asm fld radicand
   __asm fsqrt
   __asm fstp magnitude
   if(!(magnitude>0.f)){state=2;return false;}
   volatile float inverse=(float)(1.0/double(magnitude));
   volatile float ux=(float)(double(inverse)*double(cx)),uy=(float)(double(inverse)*double(cy)),uz=(float)(double(inverse)*double(cz));
   for(unsigned int test=0;test<f.count;test++){if(test==current||test==previous)continue;
    WorldText::Vec3 p=Center(g.vertices[f.indices[test]],offset);
    volatile float dz=(float)(double(p.z)-double(a.z)),dy=(float)(double(p.y)-double(a.y)),dx=(float)(double(p.x)-double(a.x));
    volatile double dot=(double(uz)*double(dz)+double(uy)*double(dy))+double(dx)*double(ux);
    if(dot>double(0.001f))return true;}
   previous=current;}
  return false;
 }
 // Projected up-facing silhouette of a hull brush; false with count 0 means "empty".
 static bool Carver(const PrefabScene::Node& n,const WorldText::Vec3& offset,WorldText::Vec3* points,unsigned int& count,float& height){
  const WorldText::Geometry& g=n.geometry;count=0;if(g.polygons<2||g.polygons>=32||!g.points||g.points>4096)return true;
  unsigned int up=0;for(unsigned int i=0;i<g.polygons;i++){if(!WorldText::FiniteVector(g.faces[i].normal))return false;if(-g.faces[i].normal.y<0.f)up|=1u<<i;}
  unsigned int edges[64][2],found=0;
  for(unsigned int a=0;a<g.polygons;a++){if(!(up&(1u<<a)))continue;const WorldText::Polygon& A=g.faces[a];
   for(unsigned int b=0;b<g.polygons;b++){if(up&(1u<<b))continue;const WorldText::Polygon& B=g.faces[b];
    for(unsigned int i=0;i<A.count;i++){bool first=false,second=false;unsigned int v=A.indices[i],w=A.indices[(i+1)%A.count];for(unsigned int j=0;j<B.count;j++){if(B.indices[j]==v)first=true;}if(!first)continue;for(unsigned int j=0;j<B.count;j++)if(B.indices[j]==w)second=true;if(second){if(found==64)return false;edges[found][0]=v;edges[found][1]=w;found++;break;}}}}
  if(found<3)return true;
  WorldText::Vec3 low=Center(g.vertices[0],offset),high=low;for(unsigned int i=1;i<g.points;i++){WorldText::Vec3 p=Center(g.vertices[i],offset);if(p.x<low.x)low.x=p.x;if(p.y<low.y)low.y=p.y;if(p.z<low.z)low.z=p.z;if(p.x>high.x)high.x=p.x;if(p.y>high.y)high.y=p.y;if(p.z>high.z)high.z=p.z;}
  unsigned int chain[64];chain[0]=edges[0][0];chain[1]=edges[0][1];unsigned int n2=found;
  for(unsigned int k=2;k<n2;k++){unsigned int j=0;for(;j<found;j++)if(edges[j][0]==chain[k-1])break;if(j==found)return true;chain[k]=edges[j][1];}
  for(unsigned int k=0;k<n2;k++)if(chain[k]>=g.points)return false;
  WorldText::Vec3 prev=Center(g.vertices[chain[0]],offset);
  for(unsigned int k=1;k<n2;k++){WorldText::Vec3 cur=Center(g.vertices[chain[k]],offset);if(cur.x==prev.x&&cur.z==prev.z){for(unsigned int m=k;m+1<n2;m++)chain[m]=chain[m+1];n2--;if(k>=n2)break;}prev=Center(g.vertices[chain[k]],offset);}
  WorldText::Vec3 first=Center(g.vertices[chain[0]],offset),last=Center(g.vertices[chain[n2-1]],offset);if(last.x==first.x&&last.z==first.z)n2--;
   height=high.y-low.y;if(!WorldText::Finite(height)||n2<3)return false;for(unsigned int k=0;k<n2;k++){WorldText::Vec3 p=Center(g.vertices[chain[k]],offset);p.y=high.y;points[k]=p;}count=n2;return true;
 }
public:
 // Whether the original keeps a carver record (VA 0x10003C00 pushes only a
 // nonempty projection); such carver brushes are deleted after Nav (VA 0x100054D0).
 // The same hull builder/serializer serves AINodeStalk (VA 0x10033A40, 0x10032E20).
 static bool Hull(const PrefabScene::Node& n,const WorldText::Vec3& offset,WorldText::Vec3* points,unsigned int& count,float& height){return Carver(n,offset,points,count,height);}
 static bool ProjectsCarver(const PrefabScene::Node& n){WorldText::Vec3 points[64],zero={0,0,0};unsigned int count=0;float height=0;return !Carver(n,zero,points,count,height)||count!=0;}
 // Omission warnings of the original's collection passes, in its order (VA
 // 0x10005800): per root the subtree pre-order (0x10005360: link child count
 // 0x10004B40, then brush predicate 0x10004E10), then carvers over the world
 // (0x100050B0 -> 0x10004B70), then regions (0x10005120 -> 0x10004C10). A brush
 // is reported at GetUpperLeftCornerPos (point 0, then min x, max y, max z of its
 // centered points) plus the world offset, each sum stored as float. Kinds:
 // 0 link with several children, 1 not Normal, 2 several polygons, 3 concave,
 // 4 downward, 5 child of such a link, 6 one-polygon carver, 7 region brush with
 // one polygon, 8 region brush not Normal, 9 region without brushes.
 typedef void (*Omission)(void* context,unsigned int kind,const char* first,const char* second,const WorldText::Vec3* at);
 static void Omissions(const PrefabScene::Model& model,const WorldText::Vec3& offset,Omission sink,void* context){
  if(!sink||!model.count||model.count>200000)return;PrefabScene::Arena scratch;
  const PrefabScene::Node** nodes=(const PrefabScene::Node**)scratch.Allocate(model.count*sizeof(void*));unsigned int* rootOf=(unsigned int*)scratch.Allocate(model.count*4);unsigned int* children=(unsigned int*)scratch.Allocate(model.count*4);unsigned char* flags=(unsigned char*)scratch.Allocate(model.count);
  if(!nodes||!rootOf||!children||!flags)return;memset(flags,0,model.count);
  for(const PrefabScene::Node* n=model.first;n;n=n->next)if(const PrefabScene::Node* c=StalkNodes::ConvertedChild(*n))if(c->index<model.count)flags[c->index]=1;
  unsigned int seen=0;for(const PrefabScene::Node* n=model.first;n;n=n->next){if(seen>=model.count||n->index!=seen||(n->parent!=Missing&&n->parent>=seen))return;nodes[seen++]=n;children[n->index]=0;if(n->parent!=Missing&&!(flags[n->index]&1))children[n->parent]++;}
  if(seen!=model.count)return;bool any=false;
  for(unsigned int i=0;i<seen;i++){const PrefabScene::Node& n=*nodes[i];unsigned int p=n.parent;rootOf[i]=p==Missing?Missing:rootOf[p];if(n.kind==2&&rootOf[i]==Missing&&Same(n.className,"AINavMesh")){rootOf[i]=i;any=true;}}
  if(!any)return;
  struct Local {static bool Normal(const PrefabScene::Node& n){return Same(Prop(n,"Type","string"),"Normal");}
   static WorldText::Vec3 Corner(const PrefabScene::Node& n,const WorldText::Vec3& o){const WorldText::Geometry& g=n.geometry;WorldText::Vec3 c={0,0,0};if(!g.points||!g.vertices)return c;c=Center(g.vertices[0],o);
    for(unsigned int i=1;i<g.points;i++){WorldText::Vec3 p=Center(g.vertices[i],o);if(p.x<c.x)c.x=p.x;if(p.y>c.y)c.y=p.y;if(p.z>c.z)c.z=p.z;}
    volatile float x=c.x+o.x,y=c.y+o.y,z=c.z+o.z;WorldText::Vec3 r={x,y,z};return r;}};
  for(unsigned int i=0;i<seen;i++){const PrefabScene::Node& n=*nodes[i];if(n.kind!=1||rootOf[i]==Missing||(flags[i]&1))continue;
   unsigned int p=n.parent;bool linkParent=p!=Missing&&Prefix(nodes[p]->className,"AINavMeshLink"),split=linkParent&&children[p]>1;
   if(split)sink(context,0,nodes[p]->name,0,0);
   WorldText::Vec3 at=Local::Corner(n,offset);
   if(!Local::Normal(n)){sink(context,1,n.name,0,&at);continue;}
   if(n.geometry.polygons>1){sink(context,2,n.name,0,&at);continue;}
   if(!n.geometry.polygons||!n.geometry.faces)continue;const WorldText::Polygon& f=n.geometry.faces[0];unsigned int state=0;
   if(f.count>4096||!f.indices)continue;bool inside=true;for(unsigned int k=0;k<f.count;k++)if(f.indices[k]>=n.geometry.points)inside=false;if(!inside)continue;
   if(Concave(n.geometry,f,offset,state)){sink(context,3,n.name,0,&at);continue;}if(state)continue;
   if((double(f.normal.x)+double(f.normal.z))*0.0+double(f.normal.y)<0.0){sink(context,4,n.name,0,&at);continue;}
   if(split)sink(context,5,n.name,0,&at);}
  for(unsigned int i=0;i<seen;i++){const PrefabScene::Node& n=*nodes[i];if(n.kind==1&&!(flags[i]&1)&&Same(Prop(n,"Type","string"),"AINavMeshCarver")&&n.geometry.polygons==1)sink(context,6,n.name,0,0);}
  for(unsigned int r=0;r<seen;r++){const PrefabScene::Node& region=*nodes[r];if(region.parent!=Missing&&(flags[region.parent]&2))flags[r]|=2;
   if((flags[r]&2)||region.kind!=2||!Same(region.className,"AIRegion"))continue;unsigned int valid=0;
   for(unsigned int i=r+1;i<seen;i++){const PrefabScene::Node& n=*nodes[i];if(n.parent!=r||n.kind!=1||(flags[i]&1))continue;
    if(n.geometry.polygons==1)sink(context,7,region.name,n.name,0);else if(!Local::Normal(n))sink(context,8,region.name,0,0);else valid++;}
   if(valid)flags[r]|=2;else sink(context,9,region.name,0,0);}
 }
 unsigned char* data;unsigned int size,errorNode,rootCount,brushCount,linkCount,carverCount,regionCount,nodeCount,firstBrush;
 Binding* roots;Binding* links;Binding* regions;const char* error;WorldText::Vec3 low,high;
 Builder():data(0),size(0),errorNode(Missing),rootCount(0),brushCount(0),linkCount(0),carverCount(0),regionCount(0),nodeCount(0),firstBrush(Missing),roots(0),links(0),regions(0),error(0){low.x=low.y=low.z=high.x=high.y=high.z=0;}
 bool Build(const PrefabScene::Model& model,const WorldText::Vec3& offset){
  data=0;size=0;error=0;errorNode=Missing;rootCount=brushCount=linkCount=carverCount=regionCount=nodeCount=0;firstBrush=Missing;
  if(!model.count||model.count>200000||!WorldText::FiniteVector(offset))return Fail("Invalid general NavMesh source model");
  const PrefabScene::Node** nodes=(const PrefabScene::Node**)memory.Allocate(model.count*sizeof(void*));unsigned int* rootOf=(unsigned int*)memory.Allocate(model.count*4);unsigned int* children=(unsigned int*)memory.Allocate(model.count*4);
  unsigned int* admitted=(unsigned int*)memory.Allocate(model.count*4);unsigned int* brushLink=(unsigned int*)memory.Allocate(model.count*4);unsigned char* inside=(unsigned char*)memory.Allocate(model.count);
  roots=(Binding*)memory.Allocate(model.count*sizeof(Binding));links=(Binding*)memory.Allocate(model.count*sizeof(Binding));regions=(Binding*)memory.Allocate(model.count*sizeof(Binding));
  if(!nodes||!rootOf||!children||!admitted||!brushLink||!inside||!roots||!links||!regions)return Fail("General NavMesh index allocation failed");
  // "Converting AI Nodes" deletes converted AINodeStalk child brushes before Nav
  // (StalkNodes.h); they take no part in counts, brushes, carvers or regions.
  unsigned char* deleted=(unsigned char*)memory.Allocate(model.count);if(!deleted)return Fail("General NavMesh index allocation failed");memset(deleted,0,model.count);
  for(const PrefabScene::Node* n=model.first;n;n=n->next)if(const PrefabScene::Node* c=StalkNodes::ConvertedChild(*n))if(c->index<model.count)deleted[c->index]=1;
  unsigned int seen=0;for(const PrefabScene::Node* n=model.first;n;n=n->next){if(seen>=model.count||n->index!=seen||(n->parent!=Missing&&n->parent>=seen))return Fail("Invalid general NavMesh source ancestry",n->index);nodes[seen++]=n;children[n->index]=0;if(n->parent!=Missing&&!deleted[n->index])children[n->parent]++;}
  if(seen!=model.count)return Fail("Incomplete general NavMesh source model");
  // Outermost roots in pre-order with ids 1..N; nested AINavMesh objects are
  // ordinary descendants of the enclosing root.
  for(unsigned int i=0;i<seen;i++){const PrefabScene::Node& n=*nodes[i];unsigned int p=n.parent;rootOf[i]=p==Missing?Missing:rootOf[p];
   if(n.kind==2&&rootOf[i]==Missing&&Same(n.className,"AINavMesh")){roots[rootCount].node=i;roots[rootCount].value=(int)rootCount+1;rootOf[i]=rootCount++;}}
   if(!rootCount)return Fail("General NavMesh source has no AINavMesh");
   // Check caller framing before any geometry dereference. The recovered shape
   // and transform construction is unchanged for well-formed source scenes.
   for(unsigned int i=0;i<seen;i++){const PrefabScene::Node& n=*nodes[i];
    if(n.kind==1&&!deleted[i]){bool nav=rootOf[i]!=Missing;
     bool carver=Same(n.type,"AINavMeshCarver")&&n.geometry.polygons>=2&&n.geometry.polygons<32;
     bool region=n.parent!=Missing&&nodes[n.parent]->kind==2&&Same(nodes[n.parent]->className,"AIRegion")&&n.geometry.polygons>=2&&n.geometry.polygons<32;
     if((nav||carver||region)&&!PropertyValid(n,"Type","string"))return Fail("Malformed general NavMesh source Type property",i);
     if(nav&&Same(n.type,"Normal")&&n.geometry.polygons<1)return Fail("Normal NavMesh brush without a polygon",i);
     if(((nav&&Same(n.type,"Normal")&&n.geometry.polygons==1)||carver||region)&&!GeometryValid(n.geometry,offset))return Fail("Malformed general NavMesh source geometry",i);
    }
    if(n.kind!=2)continue;
    if(Same(n.className,"AINavMesh")){
     if(!PropertyValid(n,"NavMeshType","string")||!PropertyValid(n,"MaskType","string"))return Fail("Malformed or duplicate general NavMesh restriction property",i);
     for(unsigned int k=0;k<16;k++){char name[32];lstrcpyA(name,"CharacterType");unsigned int at=13;if(k>=10)name[at++]='1';name[at++]=(char)('0'+k%10);name[at]=0;if(!PropertyValid(n,name,"string"))return Fail("Malformed or duplicate general NavMesh character property",i);}
    }
    if(Same(n.className,"AINodeCover")&&(!PropertyValid(n,"Name","string")||!WorldText::FiniteVector(n.position)||!WorldText::FiniteVector(Center(n.position,offset))))return Fail("Malformed general NavMesh cover node",i);
   }
  unsigned int capacity=16u*1024u*1024u;unsigned char* buffer=(unsigned char*)memory.Allocate(capacity);if(!buffer)return Fail("General NavMesh output allocation failed");
  Out out={buffer,0,capacity,true};
  // Brush collection in root traversal order (VA 0x10005360/0x10004E10).
  unsigned int admittedCount=0;
  for(unsigned int i=0;i<seen;i++){const PrefabScene::Node& n=*nodes[i];if(n.kind!=1||rootOf[i]==Missing||deleted[i])continue;
   unsigned int p=n.parent,link=Missing;bool linkParent=p!=Missing&&Prefix(nodes[p]->className,"AINavMeshLink");
   if(linkParent&&children[p]<=1){links[linkCount].node=p;links[linkCount].value=(int)linkCount;link=linkCount++;}
   bool accept=Same(n.type,"Normal")&&n.geometry.polygons==1;
   if(accept){const WorldText::Polygon& f=n.geometry.faces[0];unsigned int state=0;
    if(Concave(n.geometry,f,offset,state))accept=false;
    else if(state)return Fail("NavMesh brush concavity test has a zero edge normal",i);
    else if((double(f.normal.x)+double(f.normal.z))*0.0+double(f.normal.y)<0.0)accept=false;
    else if(linkParent&&children[p]>1)accept=false;}
   if(!accept){if(link!=Missing)return Fail("Omitted NavMesh brush below a registered link; the original link count would exceed its records",i);continue;}
   admitted[admittedCount]=i;brushLink[admittedCount++]=link;}
  if(!admittedCount)return Fail("General NavMesh source has no navigation brush");
  // Bounds over admitted brush corners (XZ), as VA 0x10001D70.
  low.x=low.y=low.z=high.x=high.y=high.z=0;bool any=false;
  for(unsigned int a=0;a<admittedCount;a++){const PrefabScene::Node& n=*nodes[admitted[a]];const WorldText::Polygon& f=n.geometry.faces[0];
   for(unsigned int k=0;k<f.count;k++){WorldText::Vec3 p=Center(n.geometry.vertices[f.indices[k]],offset);if(!any){low=high=p;any=true;}if(p.x<low.x)low.x=p.x;if(p.z<low.z)low.z=p.z;if(p.x>high.x)high.x=p.x;if(p.z>high.z)high.z=p.z;}}
  unsigned int far=0x7f7fffffu;float maximum;memcpy(&maximum,&far,4);low.y=-maximum;high.y=maximum;
  out.Word(6);out.Vec(low);out.Vec(high);out.Word(rootCount);out.Word(admittedCount);
  for(unsigned int s=0;s<admittedCount;s++){unsigned int i=admitted[s];const PrefabScene::Node& n=*nodes[i];
   const WorldText::Polygon& f=n.geometry.faces[0];if(f.count<3||f.count>4096||!f.validPlane||!WorldText::FiniteVector(f.normal))return Fail("General NavMesh brush polygon is invalid",i);
   // The original admits vertical polygons; their SDK result is not yet covered.
   if(f.normal.y<=0.f)return Fail("Vertical NavMesh brush is admitted by the original but not yet supported",i);
    // A star may have positive turns at every corner. Test every authored
    // corner against every directed edge, including collinear boundary points.
    // Admitted shapes inside the original's 0.001 concavity tolerance and
    // nonadjacent repeated corners are refused rather than approximated.
    for(unsigned int k=0;k<f.count;k++){
     WorldText::Vec3 a=Center(n.geometry.vertices[f.indices[k]],offset),b=Center(n.geometry.vertices[f.indices[(k+1)%f.count]],offset);
     for(unsigned int j=0;j<f.count;j++){
      WorldText::Vec3 c=Center(n.geometry.vertices[f.indices[j]],offset);
      if(j!=k&&a.x==c.x&&a.y==c.y&&a.z==c.z)return Fail("Repeated NavMesh corner remains after original cleanup and is not supported",i);
      double side=(double(b.z)-a.z)*(double(c.x)-a.x)-(double(b.x)-a.x)*(double(c.z)-a.z);
      if(side<0)return Fail("NavMesh brush admitted within the original concavity tolerance is not supported",i);
     }
    }
   out.Word(roots[rootOf[i]].value);out.Word(brushLink[s]==Missing?Missing:(unsigned int)links[brushLink[s]].value);out.Word(f.count);for(unsigned int k=0;k<f.count;k++)out.Vec(Center(n.geometry.vertices[f.indices[k]],offset));if(!brushCount)firstBrush=i;brushCount++;}
  WorldText::Vec3 points[64];unsigned int count=0;float height=0;
  unsigned int carverAt=out.size;out.Word(0);
  for(unsigned int i=0;i<seen;i++){const PrefabScene::Node& n=*nodes[i];if(n.kind!=1||deleted[i]||!Same(n.type,"AINavMeshCarver")||n.geometry.polygons<2)continue;if(!Carver(n,offset,points,count,height))return Fail("General NavMesh carver geometry is invalid",i);if(!count)continue;out.Real(height);out.Word(count);for(unsigned int k=0;k<count;k++)out.Vec(points[k]);carverCount++;}
   out.PatchWord(carverAt,carverCount);
  unsigned int regionAt=out.size;out.Word(0);
  for(unsigned int r=0;r<seen;r++){const PrefabScene::Node& region=*nodes[r];inside[r]=region.parent!=Missing&&inside[region.parent];
   if(inside[r]||region.kind!=2||!Same(region.className,"AIRegion"))continue;bool valid=false;
   for(unsigned int i=r+1;i<seen;i++){const PrefabScene::Node& n=*nodes[i];if(n.parent==r&&n.kind==1&&!deleted[i]&&n.geometry.polygons!=1&&Same(n.type,"Normal")){valid=true;break;}}
   if(!valid)continue; // Omitted; AIRegionID keeps its authored value and descendants are searched.
   inside[r]=1;unsigned int carvers=0,at=out.size;out.Word(0);
   for(unsigned int i=r+1;i<seen;i++){const PrefabScene::Node& n=*nodes[i];if(n.parent!=r||n.kind!=1||deleted[i]||n.geometry.polygons<2)continue;if(!Carver(n,offset,points,count,height))return Fail("General NavMesh region carver geometry is invalid",i);if(!count)continue;out.Real(height);out.Word(count);for(unsigned int k=0;k<count;k++)out.Vec(points[k]);carvers++;}
    out.PatchWord(at,carvers);regions[regionCount].node=r;regions[regionCount].value=(int)regionCount;regionCount++;}
   out.PatchWord(regionAt,regionCount);
  unsigned int nodeAt=out.size;out.Word(0);
  for(unsigned int i=0;i<seen;i++){const PrefabScene::Node& n=*nodes[i];if(n.kind!=2||!Same(n.className,"AINodeCover"))continue;const char* name=Prop(n,"Name","string");unsigned int length=Length(name,63);if(!name||length>63)return Fail("General NavMesh cover node name is invalid",i);
   out.Word(nodeCount++);out.Text(name,length);out.Vec(Center(n.position,offset));}
   out.PatchWord(nodeAt,nodeCount);
  out.Word(linkCount);
  // One record per admitted linked brush, in brush order (VA 0x10002660).
  for(unsigned int a=0;a<admittedCount;a++){if(brushLink[a]==Missing)continue;const PrefabScene::Node& c=*nodes[admitted[a]];const WorldText::Polygon& f=c.geometry.faces[0];unsigned int node=links[brushLink[a]].node;
   out.Word((unsigned int)links[brushLink[a]].value);out.Word(Same(nodes[node]->className,"AINavMeshLinkLoseTarget")?1u:0u);out.Word(f.count);for(unsigned int k=0;k<f.count;k++)out.Vec(Center(c.geometry.vertices[f.indices[k]],offset));}
  out.Word(rootCount);
  for(unsigned int r=0;r<rootCount;r++){const PrefabScene::Node& n=*nodes[roots[r].node];const char* type=Prop(n,"NavMeshType","string");const char* mask=Prop(n,"MaskType","string");unsigned int typeLength=Length(type,63);
   if(!type||typeLength>63||!mask)return Fail("General NavMesh root requires NavMeshType and MaskType",roots[r].node);
   const char* names[16];unsigned int lengths[16],characters=0;
   for(unsigned int k=0;k<16;k++){char name[32];lstrcpyA(name,"CharacterType");unsigned int at=13;if(k>=10)name[at++]='1';name[at++]=(char)('0'+k%10);name[at]=0;const char* value=Prop(n,name,"string");if(!value||Same(value,"None"))continue;unsigned int l=Length(value,31);if(l>31)return Fail("General NavMesh character restriction is invalid",roots[r].node);names[characters]=value;lengths[characters++]=l;}
   out.Word((unsigned int)roots[r].value);out.Text(type,typeLength);out.Word(Same(mask,"Include")?1u:0u);out.Word(characters);for(unsigned int k=0;k<characters;k++)out.Text(names[k],lengths[k]);}
  if(!out.good)return Fail("General NavMesh raw exceeds its byte limit");
  data=buffer;size=out.size;return true;
 }
};
}
