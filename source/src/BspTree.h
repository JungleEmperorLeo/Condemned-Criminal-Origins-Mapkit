#pragma once
#include "BspSplit.h"
namespace BspTree {
typedef bool (*IncludeBrush)(const PrefabScene::Node&);
struct Item {Item* next;BspSplit::Fragment fragment;};
struct Branch {BspSplit::Plane plane;Item* coplanar;Branch* front;Branch* back;unsigned int depth;};
// Surface partition tree only: null children are empty sets, not solid/empty leaves.
class Tree {
 unsigned int depthLimit,fragmentLimit;bool sourceOrder;
 bool Fail(const char* message){error=message;return false;}
 Item* Add(Item*& list,const BspSplit::Fragment& fragment){
  if(createdFragments>=fragmentLimit){Fail("BSP fragment limit exceeded");return 0;}
  Item* item=(Item*)memory.Allocate(sizeof(Item));if(!item){Fail("BSP allocation failed");return 0;}
  item->fragment=fragment;item->next=list;list=item;createdFragments++;return item;
 }
 __declspec(noinline) bool BuildBranch(Item* list,Branch*& out,unsigned int depth){
  if(!list){out=0;return true;}if(depth>=depthLimit)return Fail("BSP depth limit exceeded");
  Branch* branch=(Branch*)memory.Allocate(sizeof(Branch));if(!branch)return Fail("BSP allocation failed");
  Item* selected=list;
  if(sourceOrder)for(Item* i=list->next;i;i=i->next)if(i->fragment.ownerIndex<selected->fragment.ownerIndex||(i->fragment.ownerIndex==selected->fragment.ownerIndex&&i->fragment.sourcePolygonIndex<selected->fragment.sourcePolygonIndex))selected=i;
  branch->depth=depth;if(!branch->plane.Set(selected->fragment.polygon.normal,selected->fragment.polygon.distance,epsilon))return Fail("Invalid BSP splitter");
  branches++;if(depth+1>maxDepth)maxDepth=depth+1;
  Item* front=0;Item* back=0;
  for(Item* item=list;item;item=item->next){
   const BspSplit::Fragment& f=item->fragment;
   // Adapter permits the checked splitter to operate on a fragment, retaining provenance.
   PrefabScene::Node* adapter=(PrefabScene::Node*)memory.Allocate(sizeof(PrefabScene::Node));if(!adapter)return Fail("BSP allocation failed");
   adapter->kind=1;adapter->index=f.ownerIndex;adapter->surfaceFlags=f.surfaceFlags;
   adapter->geometry.points=f.count;adapter->geometry.vertices=f.vertices;adapter->geometry.polygons=1;adapter->geometry.faces=(WorldText::Polygon*)&f.polygon;
   BspSplit::Result result=BspSplit::Split(*adapter,0,branch->plane,memory);
   if(!result.success)return Fail(result.error);
   if(result.side==BspSplit::Coplanar){result.coplanar.sourcePolygonIndex=f.sourcePolygonIndex;if(!Add(branch->coplanar,result.coplanar))return false;storedPolygons++;}
   else if(result.side==BspSplit::Front){result.front.sourcePolygonIndex=f.sourcePolygonIndex;if(!Add(front,result.front))return false;}
   else if(result.side==BspSplit::Back){result.back.sourcePolygonIndex=f.sourcePolygonIndex;if(!Add(back,result.back))return false;}
   else {splits++;result.front.sourcePolygonIndex=result.back.sourcePolygonIndex=f.sourcePolygonIndex;if(!Add(front,result.front)||!Add(back,result.back))return false;}
  }
  if(!branch->coplanar)return Fail("BSP splitter did not consume a polygon");
  if(!BuildBranch(front,branch->front,depth+1)||!BuildBranch(back,branch->back,depth+1))return false;out=branch;return true;
 }
public:
 PrefabScene::Arena memory;Branch* root;const char* error;
 unsigned int branches,maxDepth,splits,storedPolygons,createdFragments,inputPolygons;float epsilon;
 Tree():root(0),error(0),branches(0),maxDepth(0),splits(0),storedPolygons(0),createdFragments(0),inputPolygons(0),epsilon(0){}
 void Clear(){memory.Clear();root=0;error=0;branches=maxDepth=splits=storedPolygons=createdFragments=inputPolygons=0;}
 bool Build(const PrefabScene::Model& model,float tolerance=0.001f,unsigned int maxAllowedDepth=128,unsigned int maxAllowedFragments=100000,IncludeBrush include=0,bool useSourceOrder=false){
  Clear();epsilon=tolerance;depthLimit=maxAllowedDepth;fragmentLimit=maxAllowedFragments;sourceOrder=useSourceOrder;
  if(!WorldText::Finite(tolerance)||tolerance<0)return Fail("Invalid BSP tolerance");Item* list=0;
  for(const PrefabScene::Node* n=model.first;n;n=n->next)if(n->kind==1&&(!include||include(*n)))for(unsigned int i=0;i<n->geometry.polygons;i++){
   const WorldText::Polygon& p=n->geometry.faces[i];BspSplit::Plane plane;if(!plane.Set(p.normal,p.distance,tolerance))return Fail("Invalid input polygon plane");
   BspSplit::Result copy=BspSplit::Split(*n,i,plane,memory);if(!copy.success)return Fail(copy.error);
   if(copy.side!=BspSplit::Coplanar)return Fail("Input polygon disagrees with its plane");
   if(!Add(list,copy.coplanar))return false;inputPolygons++;
  }
  Branch* built=0;if(!BuildBranch(list,built,0)){root=0;return false;}root=built;return true;
 }
};
// Default: first working contour selects plane. Optional: lowest provenance ID.
// Neither policy is a recovered FEAR heuristic. Optional callback selects brushes.
}
