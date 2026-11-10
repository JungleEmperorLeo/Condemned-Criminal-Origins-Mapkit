#pragma once
#include "WorldCenter.h"
#include "RenderTrees.h"
#include "VisibilityTree.h"
#include "VisibilityCompiler.h"
#include "SourceAttributedContour.h"
#include "RenderAttributedOwnership.h"
namespace SourceSkyPortals {
struct Polygon {unsigned int sectorModelIndex,portalModelIndex,sourcePolygon,renderNode;RenderTrees::SkyPortal portal;};
class Builder {
 struct Point {double x,y,z;};struct Plane {double x,y,z,d;};
 PrefabScene::Arena memory;
 static double Abs(double x){return x<0?-x:x;}
 static double Dot(const Plane& n,const Point& p){return n.x*p.x+n.y*p.y+n.z*p.z;}
 static bool Near(const Point& a,const Point& b){return Abs(a.x-b.x)<1e-6&&Abs(a.y-b.y)<1e-6&&Abs(a.z-b.z)<1e-6;}
 static bool Append(Point* output,unsigned int& count,const Point& p){if(count&&Near(output[count-1],p))return true;if(count>=256)return false;output[count++]=p;return true;}
 Point Shift(const WorldText::Vec3& v){float x=v.x-offset.x,y=v.y-offset.y,z=v.z-offset.z;Point p={x,y,z};return p;}
 bool Fail(const char* message){memory.Clear();polygons=0;count=0;error=message;return false;}
 bool BuildOwned(const PrefabScene::Model& model,const VisibilityCompiler::Builder& visibility){
  polygons=(Polygon*)memory.Allocate(100000*sizeof(Polygon));if(!polygons)return Fail("Source sky ownership allocation failed");
  // Visibility sectors are Sector brushes then accepted extrusions, each
  // with its construction-order source node and final serialized ID.
  const unsigned int total=visibility.sectorCount;if(!visibility.sectorSources)return Fail("Source sky ownership provenance missing");
  unsigned int* sourceSectorById=(unsigned int*)memory.Allocate(total*4);if(!sourceSectorById)return Fail("Source sky ownership provenance allocation failed");for(unsigned int i=0;i<total;i++)sourceSectorById[i]=0xffffffffu;
  for(unsigned int ordinal=0;ordinal<total;ordinal++){unsigned int id=visibility.streamingSectors[ordinal].index;if(id>=total||sourceSectorById[id]!=0xffffffffu||visibility.sectorSources[ordinal]>=model.count)return Fail("Source sky ownership sector ID invalid");sourceSectorById[id]=visibility.sectorSources[ordinal];}
  sectorCount=total;
  // Reuse the recovered ordered-sector clipper. A contour's outside fragments
  // are carried forward rather than independently clipping its original shape
  // against every sector. This preserves split boundaries and interpolation.
  for(const PrefabScene::Node* source=model.first;source;source=source->next)if(source->kind==1&&FearBrushFlags::Equal(source->type,"SkyPortal")){
   const WorldText::Geometry& g=source->geometry;
   for(unsigned int f=0;f<g.polygons;f++){
    const WorldText::Polygon& face=g.faces[f];if(!face.validPlane||face.count<3||face.count>128)return Fail("Invalid source sky ownership contour");
    PrefabScene::Arena local;RenderClipVertex::Vertex* vertices=(RenderClipVertex::Vertex*)local.Allocate(face.count*sizeof(RenderClipVertex::Vertex));if(!vertices||!SourceAttributedContour::BuildFlatWhiteRGB(g,face.indices,face.count,offset,face.normal,vertices,face.count))return Fail("Source sky ownership vertices failed");
    RenderAttributedOwnership::Polygon input={vertices,face.count,face.normal};RenderAttributedOwnership::Builder routed;if(!routed.Build(&input,1,visibility))return Fail(routed.error);
    for(unsigned int c=0;c<routed.count;c++){
     const RenderAttributedOwnership::Contour& contour=routed.contours[c];if(contour.owner==RenderTriangleRouting::Unassigned)continue;
     if(contour.count<3||contour.count>255||count==100000)return Fail("Source sky ownership output limit");WorldText::Vec3* points=(WorldText::Vec3*)memory.Allocate(contour.count*12);if(!points)return Fail("Source sky ownership output allocation failed");for(unsigned int v=0;v<contour.count;v++)points[v]=contour.vertices[v].point;
     // Diagnostic provenance remains a source-node identity, while routing
     // consumes the compiler's final serialized sector ID.
     if(contour.owner>=sectorCount||sourceSectorById[contour.owner]==0xffffffffu)return Fail("Source sky ownership sector provenance missing");
     Polygon& out=polygons[count++];out.sectorModelIndex=sourceSectorById[contour.owner];out.portalModelIndex=source->index;out.sourcePolygon=f;out.renderNode=contour.owner;out.portal.vertices=points;out.portal.count=contour.count;
    }
   }
  }
  for(unsigned int i=1;i<count;i++){Polygon value=polygons[i];unsigned int at=i;while(at&&polygons[at-1].renderNode>value.renderNode){polygons[at]=polygons[at-1];at--;}polygons[at]=value;}
  return true;
 }
public:
 Polygon* polygons;unsigned int count,sourceCount,sectorCount;WorldText::Vec3 offset;const char* error;
 Builder():polygons(0),count(0),sourceCount(0),sectorCount(0),error(0){}
 bool Build(const PrefabScene::Model& model,const VisibilityCompiler::Builder* preparedVisibility=0){
  memory.Clear();polygons=0;count=sourceCount=sectorCount=0;error=0;if(!WorldCenter::Compute(model,offset))return Fail("SkyPortal world centering failed");
  for(const PrefabScene::Node* n=model.first;n;n=n->next)if(n->kind==1){if(FearBrushFlags::Equal(n->type,"SkyPortal"))sourceCount+=n->geometry.polygons;if(FearBrushFlags::Equal(n->type,"Sector"))sectorCount++;}
  if(!sourceCount)return true;if(sourceCount>6000||!sectorCount||sectorCount>4096)return Fail("Unsupported source portal counts");
  unsigned int extrusions=0;for(const PrefabScene::Node* n=model.first;n;n=n->next)if(n->kind==1&&(FearBrushFlags::Equal(n->type,"ExtrudeSector")||FearBrushFlags::Equal(n->type,"SemiExtrudeSector")))extrusions++;
  if(extrusions&&!preparedVisibility)return Fail("SkyPortal extrusion profile not implemented");
  if(preparedVisibility){
   // Skipped apertures do not create a sector. The ordinary-sector clipping
   // path remains valid only for successful preparation of this exact model.
   if(preparedVisibility->sourceModel!=&model||preparedVisibility->errorCode||!preparedVisibility->data||!preparedVisibility->size||!preparedVisibility->streamingSectors||!WorldText::FiniteVector(preparedVisibility->offset))return Fail("SkyPortal skipped extrusion preparation mismatch");
   if(preparedVisibility->sectorCount!=sectorCount+preparedVisibility->acceptedExtrusions||preparedVisibility->skippedExtrusions+preparedVisibility->acceptedExtrusions!=extrusions)return Fail("SkyPortal extrusion count mismatch");
   const unsigned char* actual=(const unsigned char*)&offset;const unsigned char* expected=(const unsigned char*)&preparedVisibility->offset;for(unsigned int b=0;b<12;b++)if(actual[b]!=expected[b])return Fail("SkyPortal skipped extrusion centering mismatch");
  }
  if(preparedVisibility)return BuildOwned(model,*preparedVisibility);
  polygons=(Polygon*)memory.Allocate(100000*sizeof(Polygon));Point* work=(Point*)memory.Allocate(512*sizeof(Point));Plane* planes=(Plane*)memory.Allocate(4096*sizeof(Plane));if(!polygons||!work||!planes)return Fail("Source portal allocation failed");
  StreamingSplit::Bounds* sectorBounds=(StreamingSplit::Bounds*)memory.Allocate(sectorCount*sizeof(StreamingSplit::Bounds));if(!sectorBounds)return Fail("Sector bounds allocation failed");unsigned int sectorOrdinal=0;
  for(const PrefabScene::Node* n=model.first;n;n=n->next)if(n->kind==1&&FearBrushFlags::Equal(n->type,"Sector")){
   if(!n->geometry.points)return Fail("Empty portal sector");StreamingSplit::Bounds& bounds=sectorBounds[sectorOrdinal++];
   for(unsigned int v=0;v<n->geometry.points;v++){const WorldText::Vec3& p=n->geometry.vertices[v];if(!WorldText::FiniteVector(p))return Fail("Nonfinite sector bounds");Point q=Shift(p);double xyz[3]={q.x,q.y,q.z};for(unsigned int k=0;k<3;k++){float value=(float)xyz[k];if(!v)bounds.low[k]=bounds.high[k]=value;else{if(value<bounds.low[k])bounds.low[k]=value;if(value>bounds.high[k])bounds.high[k]=value;}}}
  }
  VisibilityTree::Builder ordering;if(!ordering.Build(sectorBounds,sectorCount))return Fail("Portal sector ordering failed");sectorOrdinal=0;
  for(const PrefabScene::Node* sector=model.first;sector;sector=sector->next){if(sector->kind!=1||!FearBrushFlags::Equal(sector->type,"Sector"))continue;const WorldText::Geometry& g=sector->geometry;if(!g.points||g.polygons<4||g.polygons>4096)return Fail("Invalid portal sector geometry");Point center={};
   for(unsigned int i=0;i<g.points;i++){Point p=Shift(g.vertices[i]);center.x+=p.x;center.y+=p.y;center.z+=p.z;}center.x/=g.points;center.y/=g.points;center.z/=g.points;
   for(unsigned int i=0;i<g.polygons;i++){const WorldText::Polygon& face=g.faces[i];if(!face.validPlane||face.count<3||face.indices[0]>=g.points)return Fail("Invalid sector plane");Plane& p=planes[i];p.x=face.normal.x;p.y=face.normal.y;p.z=face.normal.z;p.d=Dot(p,Shift(g.vertices[face.indices[0]]));if(Dot(p,center)<p.d){p.x=-p.x;p.y=-p.y;p.z=-p.z;p.d=-p.d;}
    for(unsigned int v=0;v<g.points;v++)if(Dot(p,Shift(g.vertices[v]))-p.d<-.01)return Fail("Nonconvex portal sector unsupported");
   }
   unsigned int renderNode=ordering.saveIds[sectorOrdinal++];
   for(const PrefabScene::Node* source=model.first;source;source=source->next){if(source->kind!=1||!FearBrushFlags::Equal(source->type,"SkyPortal"))continue;const WorldText::Geometry& sourceGeometry=source->geometry;
    for(unsigned int f=0;f<sourceGeometry.polygons;f++){const WorldText::Polygon& face=sourceGeometry.faces[f];if(!face.validPlane||face.count<3||face.count>255)return Fail("Invalid source portal contour");Point* current=work;Point* next=work+256;unsigned int vertices=face.count;
     for(unsigned int v=0;v<vertices;v++){if(face.indices[v]>=sourceGeometry.points)return Fail("Invalid portal vertex reference");const WorldText::Vec3& p=sourceGeometry.vertices[face.indices[v]];if(!WorldText::FiniteVector(p))return Fail("Nonfinite portal vertex");current[v]=Shift(p);}
     for(unsigned int plane=0;plane<g.polygons&&vertices;plane++){const Plane& p=planes[plane];bool coplanar=true,inside=true;
      for(unsigned int v=0;v<vertices;v++){double d=Dot(p,current[v])-p.d;if(Abs(d)>=1e-6)coplanar=false;if(d< -1e-6)inside=false;}
      if(coplanar){if(p.x*face.normal.x+p.y*face.normal.y+p.z*face.normal.z<0)vertices=0;continue;}if(inside)continue;
      // Original sub_10046CF0 seeds the previous point with the last vertex
      // (VA 10046EDE), then emits crossings before current vertices (10047121..15F).
      unsigned int used=0;for(unsigned int v=0;v<vertices;v++){const Point& a=current[v?v-1:vertices-1];const Point& b=current[v];double da=Dot(p,a)-p.d,db=Dot(p,b)-p.d;bool ia=da>=-1e-6,ib=db>=-1e-6;
       if(ia!=ib){double t=da/(da-db);Point hit={a.x+t*(b.x-a.x),a.y+t*(b.y-a.y),a.z+t*(b.z-a.z)};if(!Append(next,used,hit))return Fail("Portal clipping vertex limit");}if(ib&&!Append(next,used,b))return Fail("Portal clipping vertex limit");
      }if(used>1&&Near(next[0],next[used-1]))used--;vertices=used<3?0:used;Point* swap=current;current=next;next=swap;
     }
     if(!vertices)continue;if(vertices>255||count==100000)return Fail("Portal output limit");WorldText::Vec3* points=(WorldText::Vec3*)memory.Allocate(vertices*sizeof(WorldText::Vec3));if(!points)return Fail("Portal output allocation failed");for(unsigned int v=0;v<vertices;v++){points[v].x=(float)current[v].x;points[v].y=(float)current[v].y;points[v].z=(float)current[v].z;}
     Polygon& polygon=polygons[count++];polygon.sectorModelIndex=sector->index;polygon.portalModelIndex=source->index;polygon.sourcePolygon=f;polygon.renderNode=renderNode;polygon.portal.vertices=points;polygon.portal.count=vertices;
    }
   }
  }
  // Stable node order, preserving source polygon insertion within each sector.
  for(unsigned int i=1;i<count;i++){Polygon value=polygons[i];unsigned int at=i;while(at&&polygons[at-1].renderNode>value.renderNode){polygons[at]=polygons[at-1];at--;}polygons[at]=value;}
  return true;
 }
};
// Prepared ordinary-sector ownership, including proven skipped apertures only;
// accepted extrusions, unassigned portals, owner partitioning and exact
// broader node/owner profiles remain open. Office contour starts are verified exact.
// No reference is consumed.
}
