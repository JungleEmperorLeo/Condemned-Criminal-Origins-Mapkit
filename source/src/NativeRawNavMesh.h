#pragma once
#include "NativeNavSlopeGeometry.h"
#include "NativeRawNavMeshGeneral.h"
// Owned source -> SDK ImportRawNavMesh v6 input, WITHOUT the outer processed
// marker or BlindData container. Up to four authored identity AINavMesh roots; a single
// rectangle, one strict convex horizontal or bounded rational-slope 3..8-gon,
// 2..8 coplanar horizontal convex polygons or 2..8 convex faces on one exact
// rational slope plane under one root, or <=16 uniform cells in a 4x4 lattice; no auxiliaries.
namespace NativeRawNavMesh {
static const unsigned int Missing=0xffffffffu;
struct Settings {unsigned int version,maximumBytes,maximumNodes;Settings():version(1),maximumBytes(16u*1024u*1024u),maximumNodes(100000){}};
struct GeneratedLong {unsigned int sourceNode;const char* property;int value;};
class Builder {
 PrefabScene::Arena memory;Builder(const Builder&);Builder& operator=(const Builder&);
 bool Fail(const char* text,unsigned int node=Missing){Clear();error=text;errorNode=node;return false;}
 static bool Exact(const char* a,const char* b){if(!a||!b)return false;for(unsigned int i=0;i<4096;i++){if(a[i]!=b[i])return false;if(!a[i])return true;}return false;}
 static bool Prefix(const char* a,const char* b){if(!a||!b)return false;for(;*b;a++,b++){char x=*a,y=*b;if(!x)return false;if(x>='A'&&x<='Z')x=(char)(x+32);if(y>='A'&&y<='Z')y=(char)(y+32);if(x!=y)return false;}return true;}
 static bool Identity(const PrefabScene::Node& n){if(n.importedFromPrefab)return false;for(unsigned int i=0;i<3;i++)for(unsigned int j=0;j<4;j++)if(n.orientation.m[i][j]!=(i==j?1.f:0.f))return false;return true;}
 static const PrefabScene::Property* Find(const PrefabScene::Node& n,const char* name,bool& valid){const PrefabScene::Property* found=0;unsigned int count=0;valid=true;for(const PrefabScene::Property* p=n.properties;p;p=p->next){if(count++==4096){valid=false;return 0;}if(Exact(p->name,name)){if(found){valid=false;return 0;}found=p;}}return found;}
 static bool String(const PrefabScene::Node& n,const char* name,unsigned int limit,const char*& value,unsigned int& length){bool valid=false;const PrefabScene::Property* p=Find(n,name,valid);if(!valid||!p||!Exact(p->type,"string")||!p->data)return false;value=p->data;length=0;while(length<=limit&&value[length]){unsigned char c=(unsigned char)value[length];if(c<32||c>126)return false;length++;}return length<=limit;}
 static bool Rectangle(const WorldText::Vec3* p){for(unsigned int i=0;i<4;i++){const WorldText::Vec3& a=p[i];const WorldText::Vec3& b=p[(i+1)&3];const WorldText::Vec3& c=p[(i+2)&3];if(!WorldText::FiniteVector(a)||a.y!=p[0].y)return false;double dx=double(b.x)-a.x,dz=double(b.z)-a.z,nx=double(c.x)-b.x,nz=double(c.z)-b.z;if((dx==0)==(dz==0)||dz*nx-dx*nz<=0)return false;}return true;}
  static void Bounds(const WorldText::Vec3* p,WorldText::Vec3& low,WorldText::Vec3& high){low=high=p[0];for(unsigned int i=1;i<4;i++){if(p[i].x<low.x)low.x=p[i].x;if(p[i].z<low.z)low.z=p[i].z;if(p[i].x>high.x)high.x=p[i].x;if(p[i].z>high.z)high.z=p[i].z;}}
  static bool Cells(const WorldText::Vec3* points,unsigned int count){
   if(count<2)return true;WorldText::Vec3 lo,hi;Bounds(points,lo,hi);const double width=double(hi.x)-lo.x,depth=double(hi.z)-lo.z;if(width<64||depth<64||width>4*depth||depth>4*width)return false;
   float anchorX=lo.x,anchorZ=lo.z;for(unsigned int i=0;i<count*4;i++){const WorldText::Vec3& p=points[i];if(p.x<-1048576.f||p.x>1048576.f||p.y<-1048576.f||p.y>1048576.f||p.z<-1048576.f||p.z>1048576.f||(float)(int)p.x!=p.x||(float)(int)p.y!=p.y||(float)(int)p.z!=p.z||p.y!=points[0].y)return false;if(p.x<anchorX)anchorX=p.x;if(p.z<anchorZ)anchorZ=p.z;}
   unsigned int occupied=0;for(unsigned int i=0;i<count;i++){Bounds(points+4*i,lo,hi);if(double(hi.x)-lo.x!=width||double(hi.z)-lo.z!=depth)return false;double x=(double(lo.x)-anchorX)/width,z=(double(lo.z)-anchorZ)/depth;if(x<0||x>3||z<0||z>3||x!=(int)x||z!=(int)z)return false;unsigned int bit=1u<<((unsigned int)(int)x+4*(unsigned int)(int)z);if(occupied&bit)return false;occupied|=bit;}return true;
  }
  static bool Convex(const WorldText::Vec3* p,unsigned int count){
   if(count<3||count>8)return false;WorldText::Vec3 low=p[0],high=p[0];double twiceArea=0;
   for(unsigned int i=0;i<count;i++){const WorldText::Vec3& a=p[i];if(!WorldText::FiniteVector(a)||a.y!=p[0].y||a.x<-1048576.f||a.x>1048576.f||a.y<-1048576.f||a.y>1048576.f||a.z<-1048576.f||a.z>1048576.f||(float)(int)a.x!=a.x||(float)(int)a.y!=a.y||(float)(int)a.z!=a.z)return false;if(a.x<low.x)low.x=a.x;if(a.z<low.z)low.z=a.z;if(a.x>high.x)high.x=a.x;if(a.z>high.z)high.z=a.z;
    for(unsigned int j=0;j<i;j++){double dx=double(a.x)-p[j].x,dz=double(a.z)-p[j].z;if(dx*dx+dz*dz<4096)return false;}
    const WorldText::Vec3& b=p[(i+1)%count];twiceArea+=double(a.z)*b.x-double(a.x)*b.z;
   }
   double width=double(high.x)-low.x,depth=double(high.z)-low.z;if(width<64||depth<64||width>4*depth||depth>4*width||twiceArea<8192)return false;
   for(unsigned int i=0;i<count;i++){const WorldText::Vec3& a=p[i];const WorldText::Vec3& b=p[(i+1)%count];const WorldText::Vec3& c=p[(i+2)%count];double dx=double(b.x)-a.x,dz=double(b.z)-a.z,nx=double(c.x)-b.x,nz=double(c.z)-b.z,cross=dz*nx-dx*nz,first=dx*dx+dz*dz,second=nx*nx+nz*nz;if(cross<=0||cross*cross<.0025*first*second)return false;
    for(unsigned int j=0;j<count;j++)if(j!=i&&j!=(i+1)%count){double x=double(p[j].x)-a.x,z=double(p[j].z)-a.z;if(dz*x-dx*z<=0)return false;}
    // SDK simplification compares the incoming direction with the chord from
    // its previous vertex, rather than the outgoing edge. Keep a 2x margin.
    double chordX=double(c.x)-a.x,chordZ=double(c.z)-a.z,dot=dx*chordX+dz*chordZ,chord=chordX*chordX+chordZ*chordZ;if(dot>0&&dot*dot>.99960004*first*chord)return false;
   }return true;
  }
  static bool Separated(const WorldText::Vec3* a,unsigned int na,const WorldText::Vec3* b,unsigned int nb){
   for(unsigned int i=0;i<na;i++){const WorldText::Vec3& p=a[i];const WorldText::Vec3& q=a[(i+1)%na];double dx=double(q.x)-p.x,dz=double(q.z)-p.z;bool outside=true;for(unsigned int j=0;j<nb;j++)if(dz*(double(b[j].x)-p.x)-dx*(double(b[j].z)-p.z)>0){outside=false;break;}if(outside)return true;}return false;
  }
  // Sloped sets were already admitted face-by-face on one exact plane.
  static bool Planar(const WorldText::Vec3* points,const unsigned int* starts,const unsigned int* counts,unsigned int count,unsigned int total,bool slope){
   if(count<2||count>8||total>32)return false;WorldText::Vec3 low=points[0],high=points[0];
   for(unsigned int i=0;i<total;i++){const WorldText::Vec3& p=points[i];if((!slope&&p.y!=points[0].y)||p.x<-262144.f||p.x>262144.f||p.y<-262144.f||p.y>262144.f||p.z<-262144.f||p.z>262144.f)return false;if(p.x<low.x)low.x=p.x;if(p.x>high.x)high.x=p.x;if(p.z<low.z)low.z=p.z;if(p.z>high.z)high.z=p.z;for(unsigned int j=0;j<i;j++){double x=double(p.x)-points[j].x,z=double(p.z)-points[j].z;if((x||z)&&x*x+z*z<4096)return false;}}
   if(double(high.x)-low.x>16384||double(high.z)-low.z>16384)return false;
   for(unsigned int i=0;i<count;i++){const WorldText::Vec3* a=points+starts[i];if(!slope&&!Convex(a,counts[i]))return false;for(unsigned int j=0;j<i;j++){const WorldText::Vec3* b=points+starts[j];if(!Separated(a,counts[i],b,counts[j])&&!Separated(b,counts[j],a,counts[i]))return false;}}return true;
  }
 // Faces on two or more exact planes: each horizontal face passes Convex, each
 // slope already passed Check; XZ corners stay injective and >=64 apart, and
 // faces on different planes touch only at one corner or one complete edge.
 static bool Transition(const WorldText::Vec3* points,const unsigned int* starts,const unsigned int* counts,unsigned int count,unsigned int total,const NativeNavSlopeGeometry::FacePlane* faces){
  if(count<2||count>8||total>32)return false;WorldText::Vec3 low=points[0],high=points[0];
  for(unsigned int i=0;i<total;i++){const WorldText::Vec3& p=points[i];if(p.x<-262144.f||p.x>262144.f||p.y<-262144.f||p.y>262144.f||p.z<-262144.f||p.z>262144.f)return false;if(p.x<low.x)low.x=p.x;if(p.x>high.x)high.x=p.x;if(p.z<low.z)low.z=p.z;if(p.z>high.z)high.z=p.z;for(unsigned int j=0;j<i;j++){double x=double(p.x)-points[j].x,z=double(p.z)-points[j].z;if((x||z)&&x*x+z*z<4096)return false;}}
  if(double(high.x)-low.x>16384||double(high.z)-low.z>16384)return false;
  for(unsigned int i=0;i<count;i++){const WorldText::Vec3* a=points+starts[i];if(!faces[i].p&&!faces[i].r&&!Convex(a,counts[i]))return false;for(unsigned int j=0;j<i;j++){const WorldText::Vec3* b=points+starts[j];if(!Separated(a,counts[i],b,counts[j])&&!Separated(b,counts[j],a,counts[i]))return false;if(!NativeNavSlopeGeometry::SameFace(faces[i],faces[j])&&!NativeNavSlopeGeometry::CrossPlaneContact(a,counts[i],b,counts[j]))return false;}}return true;
 }
 struct Writer {unsigned char* data;unsigned int cursor;Writer(unsigned char* p):data(p),cursor(0){}void Word(unsigned int v){memcpy(data+cursor,&v,4);cursor+=4;}void Vector(const WorldText::Vec3& v){memcpy(data+cursor,&v,12);cursor+=12;}void String16(const char* s,unsigned int n){unsigned short length=(unsigned short)n;memcpy(data+cursor,&length,2);cursor+=2;memcpy(data+cursor,s,n);cursor+=n;}};
  struct RestrictionData {const char* navType;unsigned int navTypeBytes,include,count,lengths[16];const char* names[16];};
  static bool ReadRestriction(const PrefabScene::Node& root,RestrictionData& row,unsigned int& bytes){
   const char* mask=0;unsigned int maskBytes=0;if(!String(root,"NavMeshType",63,row.navType,row.navTypeBytes)||!row.navTypeBytes||!String(root,"MaskType",31,mask,maskBytes)||(!Exact(mask,"Include")&&!Exact(mask,"Exclude")))return false;
   row.include=Exact(mask,"Include")?1u:0u;row.count=0;bytes=14+row.navTypeBytes;
   for(unsigned int i=0;i<16;i++){char name[32];lstrcpyA(name,"CharacterType");unsigned int at=13;if(i>=10)name[at++]='1';name[at++]=(char)('0'+i%10);name[at]=0;const char* value=0;unsigned int length=0;if(!String(root,name,31,value,length)||!length)return false;if(Exact(value,"None"))continue;row.names[row.count]=value;row.lengths[row.count++]=length;bytes+=2+length;}return true;
  }
public:
  unsigned char* data;unsigned int size,sourceNode,sourceId,brushNode,brushSourceId,brushCount,rootCount,singlePolygonVertexCount,errorNode;bool convexPolygonProfile,planarPolygonProfile,slopePolygonProfile,transitionPolygonProfile,generalProfile;struct GeneralReal {unsigned int sourceNode;const char* property;float value;};GeneralReal* generalReals;unsigned int generalRealCount;GeneratedLong generatedLong;GeneratedLong* generatedLongs;WorldText::Vec3 offset,boundsMin,boundsMax;const char* error;
  Builder():data(0),size(0),sourceNode(Missing),sourceId(Missing),brushNode(Missing),brushSourceId(Missing),brushCount(0),rootCount(0),singlePolygonVertexCount(0),errorNode(Missing),convexPolygonProfile(false),planarPolygonProfile(false),slopePolygonProfile(false),transitionPolygonProfile(false),generalProfile(false),generalReals(0),generalRealCount(0),generatedLongs(0),error(0){generatedLong.sourceNode=Missing;generatedLong.property=0;generatedLong.value=0;offset.x=offset.y=offset.z=boundsMin.x=boundsMin.y=boundsMin.z=boundsMax.x=boundsMax.y=boundsMax.z=0;}
  void Clear(){memory.Clear();data=0;generatedLongs=0;size=brushCount=rootCount=singlePolygonVertexCount=0;convexPolygonProfile=planarPolygonProfile=slopePolygonProfile=transitionPolygonProfile=generalProfile=false;generalReals=0;generalRealCount=0;sourceNode=sourceId=brushNode=brushSourceId=errorNode=Missing;generatedLong.sourceNode=Missing;generatedLong.property=0;generatedLong.value=0;offset.x=offset.y=offset.z=boundsMin.x=boundsMin.y=boundsMin.z=boundsMax.x=boundsMax.y=boundsMax.z=0;error=0;}
 // Proved profiles first; any other source uses the raw construction recovered
 // from the original WorldPacker (NativeRawNavMeshGeneral). If both reject,
 // the profile diagnostic is kept.
 bool Build(const PrefabScene::Model& model,const WorldText::Vec3& worldOffset,const Settings& supplied=Settings()){
   // The fallback must not weaken caller limits, and a failed profile probe
   // must not release prior output that the caller may still be borrowing.
   const Settings settings=supplied;const WorldText::Vec3 center=worldOffset;
   if(settings.version!=1||settings.maximumBytes<4||settings.maximumBytes>16u*1024u*1024u||!settings.maximumNodes||settings.maximumNodes>100000||model.count>settings.maximumNodes||!WorldText::FiniteVector(center))return Fail("Invalid raw NavMesh source/settings");
   Builder profile;
   if(profile.BuildProfile(model,center,settings)){
    Clear();data=(unsigned char*)memory.Allocate(profile.size);generatedLongs=(GeneratedLong*)memory.Allocate(profile.rootCount*sizeof(GeneratedLong));
    if(!data||!generatedLongs)return Fail("Raw NavMesh owned output allocation failed",profile.sourceNode);
    memcpy(data,profile.data,profile.size);memcpy(generatedLongs,profile.generatedLongs,profile.rootCount*sizeof(GeneratedLong));
    size=profile.size;sourceNode=profile.sourceNode;sourceId=profile.sourceId;brushNode=profile.brushNode;brushSourceId=profile.brushSourceId;brushCount=profile.brushCount;rootCount=profile.rootCount;singlePolygonVertexCount=profile.singlePolygonVertexCount;
    convexPolygonProfile=profile.convexPolygonProfile;planarPolygonProfile=profile.planarPolygonProfile;slopePolygonProfile=profile.slopePolygonProfile;transitionPolygonProfile=profile.transitionPolygonProfile;
    generatedLong=generatedLongs[0];offset=profile.offset;boundsMin=profile.boundsMin;boundsMax=profile.boundsMax;return true;
   }
   const char* profileError=profile.error;unsigned int profileNode=profile.errorNode;
  NativeRawNavMeshGeneral::Builder general;
   if(!general.Build(model,center)||general.size>settings.maximumBytes){Clear();error=profileError;errorNode=profileNode;return false;}
   unsigned int rootSourceId=Missing,firstBrushSourceId=Missing;
   for(const PrefabScene::Node* n=model.first;n;n=n->next){if(n->index==general.roots[0].node)rootSourceId=n->sourceId;if(n->index==general.firstBrush)firstBrushSourceId=n->sourceId;}
  Clear();
  data=(unsigned char*)memory.Allocate(general.size);generatedLongs=(GeneratedLong*)memory.Allocate(general.rootCount*sizeof(GeneratedLong));generalReals=(GeneralReal*)memory.Allocate((general.linkCount+general.regionCount+1)*sizeof(GeneralReal));
  if(!data||!generatedLongs||!generalReals)return Fail("General NavMesh owned output allocation failed");
  memcpy(data,general.data,general.size);size=general.size;
  for(unsigned int r=0;r<general.rootCount;r++){generatedLongs[r].sourceNode=general.roots[r].node;generatedLongs[r].property="AINavMeshIndex";generatedLongs[r].value=general.roots[r].value;}
  for(unsigned int l=0;l<general.linkCount;l++){GeneralReal& v=generalReals[generalRealCount++];v.sourceNode=general.links[l].node;v.property="AINavMeshLinkID";v.value=(float)general.links[l].value;}
  for(unsigned int g=0;g<general.regionCount;g++){GeneralReal& v=generalReals[generalRealCount++];v.sourceNode=general.regions[g].node;v.property="AIRegionID";v.value=(float)general.regions[g].value;}
   sourceId=rootSourceId;brushSourceId=firstBrushSourceId;
   rootCount=general.rootCount;brushCount=general.brushCount;sourceNode=general.roots[0].node;brushNode=general.firstBrush;generatedLong=generatedLongs[0];offset=center;boundsMin=general.low;boundsMax=general.high;generalProfile=true;return true;
 }
 bool BuildProfile(const PrefabScene::Model& model,const WorldText::Vec3& worldOffset,const Settings& supplied=Settings()){
  // Read and serialize the entire caller view into independent scratch BEFORE
  // resetting our prior output, so borrowed prior-output values cannot dangle.
  Settings settings=supplied;WorldText::Vec3 center=worldOffset;if(settings.version!=1||settings.maximumBytes<4||settings.maximumBytes>16u*1024u*1024u||!settings.maximumNodes||settings.maximumNodes>100000||model.count>settings.maximumNodes||!WorldText::FiniteVector(center))return Fail("Invalid raw NavMesh source/settings");
  PrefabScene::Arena scratch;const PrefabScene::Node** nodes=(const PrefabScene::Node**)scratch.Allocate((model.count?model.count:1)*sizeof(void*));unsigned char* owner=(unsigned char*)scratch.Allocate(model.count?model.count:1);const PrefabScene::Node** roots=(const PrefabScene::Node**)scratch.Allocate(4*sizeof(void*));if(!nodes||!owner||!roots)return Fail("Raw NavMesh source index allocation failed");
  unsigned int seen=0,rootsCount=0;for(const PrefabScene::Node* n=model.first;n;n=n->next){if(seen>=model.count||n->index!=seen||(n->parent!=Missing&&n->parent>=seen))return Fail("Invalid raw NavMesh source ancestry",n->index);nodes[seen++]=n;owner[n->index]=n->parent==Missing?0:owner[n->parent];
   if(n->kind==2){if(FearBrushFlags::Equal(n->className,"AINavMesh")){if(owner[n->index])return Fail("Nested navigation roots require a later source profile",n->index);if(rootsCount==4)return Fail("Raw NavMesh profile exceeds four source roots",n->index);if(!Identity(*n))return Fail("Raw NavMesh root transform is outside authored identity profile",n->index);roots[rootsCount++]=n;owner[n->index]=(unsigned char)rootsCount;}if(Prefix(n->className,"AINavMeshLink")||FearBrushFlags::Equal(n->className,"AIRegion")||FearBrushFlags::Equal(n->className,"AINodeCover"))return Fail("Raw NavMesh auxiliary links/regions/Cover nodes require a later profile",n->index);}
   if(n->kind==1&&FearBrushFlags::Equal(n->type,"AINavMeshCarver"))return Fail("Raw NavMesh carver hull projection requires a later profile",n->index);
  }
  if(seen!=model.count)return Fail("Incomplete raw NavMesh source model");if(!rootsCount)return Fail("Raw NavMesh source has no navigation root");const PrefabScene::Node* root=roots[0];
  const PrefabScene::Node** brushes=(const PrefabScene::Node**)scratch.Allocate(16*sizeof(void*));unsigned int* brushRoots=(unsigned int*)scratch.Allocate(16*4);WorldText::Vec3* points=(WorldText::Vec3*)scratch.Allocate(64*sizeof(WorldText::Vec3));if(!brushes||!brushRoots||!points)return Fail("Raw NavMesh cell allocation failed");unsigned int cells=0;
  for(unsigned int r=0;r<rootsCount;r++){unsigned int begin=cells;for(unsigned int i=0;i<seen;i++){const PrefabScene::Node& n=*nodes[i];if(owner[i]!=r+1||&n==roots[r])continue;if(n.kind==1){if(cells==16)return Fail("Raw NavMesh profile exceeds sixteen source cells",n.index);brushRoots[cells]=r+1;brushes[cells++]=&n;}else if(n.kind!=0)return Fail("Raw NavMesh nested object is outside the simple source profile",n.index);}if(cells==begin)return Fail("Raw NavMesh root has no source quad",roots[r]->index);}
   unsigned int pointCounts[16]={},pointStarts[16]={},pointTotal=0;bool convexProfile=false,planarProfile=false,slopeProfile=false,transitionProfile=false,allRectangles=true;NativeNavSlopeGeometry::FacePlane facePlanes[16]={};
   for(unsigned int b=0;b<cells;b++){const PrefabScene::Node* brush=brushes[b];if(!Identity(*brush)||!FearBrushFlags::Equal(brush->type,"Normal"))return Fail("Raw NavMesh polygon requires authored identity Normal brush",brush->index);
   const char* brushType=0;unsigned int brushTypeBytes=0;if(!String(*brush,"Type",31,brushType,brushTypeBytes)||!FearBrushFlags::Equal(brushType,"Normal"))return Fail("Raw NavMesh quad requires typed Normal source property",brush->index);
    const WorldText::Geometry& geometry=brush->geometry;if(geometry.polygons!=1||geometry.points<3||geometry.points>8||!geometry.vertices||!geometry.faces)return Fail("Raw NavMesh profile requires one polygon with three to eight points",brush->index);const WorldText::Polygon& face=geometry.faces[0];const unsigned int count=geometry.points;if(face.count!=count||!face.indices||!face.validPlane||!WorldText::FiniteVector(face.normal)||face.normal.y<=0||((cells>8||rootsCount!=1)&&(face.normal.x!=0||face.normal.z!=0)))return Fail("Raw NavMesh polygon requires an admitted finite upward source plane",brush->index);if((count!=4&&(rootsCount!=1||cells>8))||pointTotal>64-count)return Fail("Multiple non-cell NavMesh polygons exceed their source profile",brush->index);
    pointStarts[b]=pointTotal;pointCounts[b]=count;unsigned int mask=0;for(unsigned int i=0;i<count;i++){unsigned int at=face.indices[i];if(at>=count||(mask&(1u<<at))||!WorldText::FiniteVector(geometry.vertices[at]))return Fail("Invalid raw NavMesh polygon point references",brush->index);mask|=1u<<at;WorldText::Vec3& point=points[pointTotal+i];point=geometry.vertices[at];point.x-=center.x;point.y-=center.y;point.z-=center.z;}
    if(mask!=((1u<<count)-1))return Fail("Incomplete raw NavMesh polygon points",brush->index);bool horizontal=true;for(unsigned int i=1;i<count;i++)if(points[pointTotal+i].y!=points[pointTotal].y)horizontal=false;NativeNavSlopeGeometry::FacePlane& facePlane=facePlanes[b];facePlane.p=facePlane.r=0;facePlane.q=1;facePlane.k=points[pointTotal].y;if(!horizontal){NativeNavSlopeGeometry::Plane plane={};WorldText::Vec3 slopeLow,slopeHigh;if(cells>8||rootsCount!=1||!NativeNavSlopeGeometry::Check(points+pointTotal,count,plane,slopeLow,slopeHigh))return Fail("Raw NavMesh slope exceeds the bounded convex rational-plane profile",brush->index);const WorldText::Vec3& anchor=points[pointTotal];facePlane.p=plane.p;facePlane.q=plane.q;facePlane.r=plane.r;facePlane.k=plane.q*double(anchor.y)-plane.p*double(anchor.x)-plane.r*double(anchor.z);slopeProfile=true;allRectangles=false;}if(b&&!NativeNavSlopeGeometry::SameFace(facePlane,facePlanes[0])){if(cells>8||rootsCount!=1)return Fail("Raw NavMesh transitions between source planes require one root and at most eight faces",brush->index);transitionProfile=true;}if(horizontal){bool rectangle=count==4&&Rectangle(points+pointTotal);if(!rectangle){allRectangles=false;if(cells==1){if(rootsCount!=1||!Convex(points+pointTotal,count))return Fail("Raw NavMesh non-rectangle requires one safe strict-convex integral horizontal polygon",brush->index);convexProfile=true;}}}pointTotal+=count;
  }
   if(transitionProfile){if(!Transition(points,pointStarts,pointCounts,cells,pointTotal,facePlanes))return Fail("Raw NavMesh plane transitions require bounded convex faces touching only at shared corners or complete edges",root->index);planarProfile=true;}
   else if(cells>1&&(!allRectangles||!Cells(points,cells))){if(rootsCount!=1||!Planar(points,pointStarts,pointCounts,cells,pointTotal,slopeProfile))return Fail("Raw NavMesh multiple brushes require a bounded nonoverlapping coplanar convex set or uniform cells",root->index);planarProfile=true;}
   RestrictionData* restrictions=(RestrictionData*)scratch.Allocate(rootsCount*sizeof(RestrictionData));GeneratedLong* generated=(GeneratedLong*)scratch.Allocate(rootsCount*sizeof(GeneratedLong));if(!restrictions||!generated)return Fail("Raw NavMesh root metadata allocation failed");unsigned int wanted=56+12*cells+12*pointTotal;
  for(unsigned int r=0;r<rootsCount;r++){unsigned int bytes=0;if(!ReadRestriction(*roots[r],restrictions[r],bytes))return Fail("Raw NavMesh requires bounded typed navigation and sixteen character restrictions",roots[r]->index);wanted+=bytes;generated[r].sourceNode=roots[r]->index;generated[r].property="AINavMeshIndex";generated[r].value=(int)r+1;}
  if(wanted>settings.maximumBytes)return Fail("Raw NavMesh byte limit",root->index);unsigned char* raw=(unsigned char*)scratch.Allocate(wanted);if(!raw)return Fail("Raw NavMesh serialization allocation failed",root->index);
   WorldText::Vec3 low=points[0],high=points[0];for(unsigned int i=1;i<pointTotal;i++){if(points[i].x<low.x)low.x=points[i].x;if(points[i].z<low.z)low.z=points[i].z;if(points[i].x>high.x)high.x=points[i].x;if(points[i].z>high.z)high.z=points[i].z;}unsigned int maximum=0x7f7fffffu;float far;memcpy(&far,&maximum,4);low.y=-far;high.y=far;
   Writer out(raw);out.Word(6);out.Vector(low);out.Vector(high);out.Word(rootsCount);out.Word(cells);for(unsigned int b=0;b<cells;b++){out.Word(brushRoots[b]);out.Word(Missing);out.Word(pointCounts[b]);for(unsigned int i=0;i<pointCounts[b];i++)out.Vector(points[pointStarts[b]+i]);}out.Word(0);out.Word(0);out.Word(0);out.Word(0);out.Word(rootsCount);for(unsigned int r=0;r<rootsCount;r++){const RestrictionData& row=restrictions[r];out.Word(r+1);out.String16(row.navType,row.navTypeBytes);out.Word(row.include);out.Word(row.count);for(unsigned int i=0;i<row.count;i++)out.String16(row.names[i],row.lengths[i]);}if(out.cursor!=wanted)return Fail("Raw NavMesh serialization cursor mismatch",root->index);
   const unsigned int rootIndex=root->index,rootId=root->sourceId,brushIndex=brushes[0]->index,brushId=brushes[0]->sourceId,singleVertices=cells==1?pointCounts[0]:0;Clear();data=(unsigned char*)memory.Allocate(wanted);generatedLongs=(GeneratedLong*)memory.Allocate(rootsCount*sizeof(GeneratedLong));if(!data||!generatedLongs)return Fail("Raw NavMesh owned output allocation failed",rootIndex);memcpy(data,raw,wanted);memcpy(generatedLongs,generated,rootsCount*sizeof(GeneratedLong));size=wanted;sourceNode=rootIndex;sourceId=rootId;brushNode=brushIndex;brushSourceId=brushId;brushCount=cells;rootCount=rootsCount;singlePolygonVertexCount=singleVertices;convexPolygonProfile=convexProfile;planarPolygonProfile=planarProfile;slopePolygonProfile=slopeProfile;transitionPolygonProfile=transitionProfile;generatedLong=generatedLongs[0];offset=center;boundsMin=low;boundsMax=high;return true;
 }
};
}
