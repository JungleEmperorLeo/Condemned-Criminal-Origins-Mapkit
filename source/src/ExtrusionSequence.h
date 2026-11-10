#pragma once
#include "ExtrusionPrism.h"
#include "ExtrusionBoundaryCheck.h"
#include "ConvexIntersection.h"
namespace ExtrusionSequence {
struct Existing {ExtrusionBoundaryCheck::Plane* planes;unsigned int count;ConvexIntersection::Volume convex;};
inline bool ShouldSkip(const WorldText::Vec3* points,unsigned int count,const WorldText::Vec3& normal,const Existing* existing,unsigned int volumes,bool& skip,unsigned int* hit=0){
 skip=false;for(unsigned int i=0;i<volumes;i++){bool intersects;if(!ExtrusionBoundaryCheck::Evaluate(points,count,normal,existing[i].planes,existing[i].count,-0.1f,intersects))return false;if(intersects){skip=true;if(hit)*hit=i;return true;}}return true;
}
inline bool Store(PrefabScene::Arena& memory,const ExtrusionVolume::Candidate* faces,unsigned int count,Existing& output){
 if(!faces||!count||count>128)return false;output.planes=(ExtrusionBoundaryCheck::Plane*)memory.Allocate(count*16);ConvexIntersection::Face* contour=(ConvexIntersection::Face*)memory.Allocate(count*sizeof(ConvexIntersection::Face));if(!output.planes||!contour)return false;output.count=count;output.convex.faces=contour;output.convex.count=count;output.convex.low.x=output.convex.low.y=output.convex.low.z=1e30f;output.convex.high.x=output.convex.high.y=output.convex.high.z=-1e30f;
 for(unsigned int i=0;i<count;i++){const ExtrusionVolume::Plane& p=faces[i].plane;ExtrusionBoundaryCheck::Plane& q=output.planes[i];q.x=p.x;q.y=p.y;q.z=p.z;q.d=p.d;contour[i].points=faces[i].points;contour[i].count=faces[i].count;contour[i].plane=p;for(unsigned int j=0;j<faces[i].count;j++)for(unsigned int a=0;a<3;a++){float v=(&faces[i].points[j].x)[a];if(v<(&output.convex.low.x)[a])(&output.convex.low.x)[a]=v;if(v>(&output.convex.high.x)[a])(&output.convex.high.x)[a]=v;}}return true;
}
// Original final sector-pair audit: RVA 0x2DC2D passes -0.1 to 0x13C70.
inline bool HasOverlap(const Existing* existing,unsigned int count,bool& overlaps){overlaps=false;for(unsigned int i=0;i<count;i++)for(unsigned int j=i+1;j<count;j++){bool hit;if(!ConvexIntersection::Test(existing[i].convex,existing[j].convex,-0.1f,hit))return false;if(hit)overlaps=true;}return true;}
}
