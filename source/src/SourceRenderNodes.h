#pragma once
#include "VisibilityCompiler.h"
#include "SourceSkyPortals.h"
namespace SourceRenderNodes {
// Main-world nodes only. Original sub_1003F0F0 creates one node per prepared
// sector and routes staged records by sector+0x6C. Surface branches are supplied
// by a later geometry stage; empty branches here are an explicit scaffold.
struct Sector {unsigned int constructionIndex;WorldText::Vec3 low,high;};
class Builder {
 PrefabScene::Arena memory;
 bool Fail(const char* message){memory.Clear();nodes=0;sectors=0;tree.nodes=0;tree.count=0;layout.trees=0;layout.count=0;skyPortalCount=0;error=message;return false;}
 Builder(const Builder&);Builder& operator=(const Builder&);
public:
 RenderTrees::Node* nodes;Sector* sectors;RenderTrees::Tree tree;RenderTrees::Layout layout;unsigned int skyPortalCount;const char* error;
 Builder():nodes(0),sectors(0),skyPortalCount(0),error(0){tree.nodes=0;tree.count=0;layout.trees=0;layout.count=0;}
 bool Build(const StreamingMetadata::External* input,unsigned int count,const WorldText::Vec3& offset,const SourceSkyPortals::Builder* sky=0){
  Fail(0);
  // Zero sectors (every Sector brush removed or skipped): the original packs a main
  // render tree without nodes (visibility-sector-removed-all, files-only-objects).
  if((count&&!input)||count>65536||!WorldText::FiniteVector(offset))return Fail("Invalid prepared render sectors");
  const unsigned int slots=count?count:1;
  nodes=(RenderTrees::Node*)memory.Allocate(slots*sizeof(RenderTrees::Node));sectors=(Sector*)memory.Allocate(slots*sizeof(Sector));unsigned char* seen=(unsigned char*)memory.Allocate(slots);
  if(!nodes||!sectors||!seen)return Fail("Render sector allocation failed");
  for(unsigned int i=0;i<count;i++){seen[i]=0;nodes[i].branches=0;nodes[i].count=0;nodes[i].skyPortals=0;nodes[i].skyPortalCount=0;}
  for(unsigned int i=0;i<count;i++){
   const StreamingMetadata::External& s=input[i];const WorldText::Vec3& low=s.volume.low;const WorldText::Vec3& high=s.volume.high;
   if(s.index>=count||seen[s.index]||!WorldText::FiniteVector(low)||!WorldText::FiniteVector(high)||low.x>high.x||low.y>high.y||low.z>high.z)return Fail("Invalid render sector index or bounds");
   seen[s.index]=1;Sector& out=sectors[s.index];out.constructionIndex=i;out.low=low;out.high=high;
  }
  if(sky){
   if(sky->error||sky->count>100000||(sky->count&&!sky->polygons)||!WorldText::FiniteVector(sky->offset)||sky->offset.x!=offset.x||sky->offset.y!=offset.y||sky->offset.z!=offset.z||(sky->count&&sky->sectorCount!=count))return Fail("Incompatible source SkyPortal layout");
   for(unsigned int p=0;p<sky->count;p++){
    const SourceSkyPortals::Polygon& polygon=sky->polygons[p];const RenderTrees::SkyPortal& portal=polygon.portal;
    if(polygon.renderNode>=count||!portal.vertices||portal.count<3||portal.count>255)return Fail("Invalid source SkyPortal node or contour");
    for(unsigned int v=0;v<portal.count;v++)if(!WorldText::FiniteVector(portal.vertices[v]))return Fail("Nonfinite source SkyPortal vertex");
    nodes[polygon.renderNode].skyPortalCount++;
   }
   for(unsigned int i=0;i<count;i++)if(nodes[i].skyPortalCount){
    RenderTrees::SkyPortal* portals=(RenderTrees::SkyPortal*)memory.Allocate(nodes[i].skyPortalCount*sizeof(RenderTrees::SkyPortal));if(!portals)return Fail("Render portal allocation failed");nodes[i].skyPortals=portals;nodes[i].skyPortalCount=0;
   }
   for(unsigned int p=0;p<sky->count;p++){
    const SourceSkyPortals::Polygon& polygon=sky->polygons[p];RenderTrees::Node& node=nodes[polygon.renderNode];WorldText::Vec3* points=(WorldText::Vec3*)memory.Allocate(polygon.portal.count*sizeof(WorldText::Vec3));if(!points)return Fail("Render portal vertices allocation failed");
    memcpy(points,polygon.portal.vertices,polygon.portal.count*sizeof(WorldText::Vec3));RenderTrees::SkyPortal* portals=(RenderTrees::SkyPortal*)node.skyPortals;portals[node.skyPortalCount].vertices=points;portals[node.skyPortalCount++].count=polygon.portal.count;
   }
   skyPortalCount=sky->count;
  }
  tree.nodes=nodes;tree.count=count;layout.trees=&tree;layout.count=1;return true;
 }
 bool Build(const VisibilityCompiler::Builder& visibility,const SourceSkyPortals::Builder* sky=0){
  if(visibility.errorCode||!visibility.data||!visibility.size)return Fail("Source visibility preparation failed");
  return Build(visibility.streamingSectors,visibility.sectorCount,visibility.offset,sky);
 }
};
}
