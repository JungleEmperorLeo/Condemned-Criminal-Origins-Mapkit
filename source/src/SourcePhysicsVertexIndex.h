#pragma once
namespace SourcePhysicsVertexIndex {
// Original mesh preparation RVA 0x3C410 inserts polygon corners into an MSVC
// red-black set, before scaling positions by .01. Its comparator RVA 0x3AB50
// is a tolerant lexicographic comparison, not an Euclidean nearest-point weld.
// The comparison is not a strict weak order: retain the original tree search,
// predecessor check and rotations rather than merging through a linear scan.
class Indexer {
 struct Node {unsigned int left,right,parent;bool red;WorldText::Vec3 point;};
 PrefabScene::Arena memory;Node* nodes;unsigned int root,limit;
 static bool Less(const WorldText::Vec3& a,const WorldText::Vec3& b,float tolerance){
  if(double(a.x)<double(b.x)-tolerance)return true;if(double(a.x)>double(b.x)+tolerance)return false;
  if(double(a.y)<double(b.y)-tolerance)return true;if(double(a.y)>double(b.y)+tolerance)return false;
  return double(a.z)<double(b.z)-tolerance;
 }
 void Left(unsigned int x){unsigned int y=nodes[x].right;nodes[x].right=nodes[y].left;if(nodes[y].left)nodes[nodes[y].left].parent=x;nodes[y].parent=nodes[x].parent;if(!nodes[x].parent)root=y;else if(x==nodes[nodes[x].parent].left)nodes[nodes[x].parent].left=y;else nodes[nodes[x].parent].right=y;nodes[y].left=x;nodes[x].parent=y;}
 void Right(unsigned int x){unsigned int y=nodes[x].left;nodes[x].left=nodes[y].right;if(nodes[y].right)nodes[nodes[y].right].parent=x;nodes[y].parent=nodes[x].parent;if(!nodes[x].parent)root=y;else if(x==nodes[nodes[x].parent].right)nodes[nodes[x].parent].right=y;else nodes[nodes[x].parent].left=y;nodes[y].right=x;nodes[x].parent=y;}
 void Balance(unsigned int current){
  while(nodes[nodes[current].parent].red){unsigned int parent=nodes[current].parent,grand=nodes[parent].parent;
   if(parent==nodes[grand].left){unsigned int uncle=nodes[grand].right;if(nodes[uncle].red){nodes[parent].red=nodes[uncle].red=false;nodes[grand].red=true;current=grand;}else{if(current==nodes[parent].right){current=parent;Left(current);parent=nodes[current].parent;grand=nodes[parent].parent;}nodes[parent].red=false;nodes[grand].red=true;Right(grand);}}
   else{unsigned int uncle=nodes[grand].left;if(nodes[uncle].red){nodes[parent].red=nodes[uncle].red=false;nodes[grand].red=true;current=grand;}else{if(current==nodes[parent].left){current=parent;Right(current);parent=nodes[current].parent;grand=nodes[parent].parent;}nodes[parent].red=false;nodes[grand].red=true;Left(grand);}}
  }
  nodes[root].red=false;
 }
 unsigned int Previous(unsigned int at)const{if(nodes[at].left){at=nodes[at].left;while(nodes[at].right)at=nodes[at].right;return at;}unsigned int parent=nodes[at].parent;while(parent&&at==nodes[parent].left){at=parent;parent=nodes[parent].parent;}return parent;}
public:
 unsigned int count;float tolerance;
 Indexer():nodes(0),root(0),limit(0),count(0),tolerance(.01f){}
 bool Begin(unsigned int maximum,float epsilon){memory.Clear();nodes=0;root=limit=count=0;if(!maximum||maximum>1000000||!WorldText::Finite(epsilon)||epsilon<0)return false;nodes=(Node*)memory.Allocate((maximum+1)*sizeof(Node));if(!nodes)return false;limit=maximum;tolerance=epsilon;return true;}
 bool Insert(const WorldText::Vec3& point,unsigned int& index){
  if(!nodes||!WorldText::FiniteVector(point))return false;unsigned int at=root,parent=0;bool left=true;
  while(at){parent=at;left=Less(point,nodes[at].point,tolerance);at=left?nodes[at].left:nodes[at].right;}
  unsigned int candidate=parent;if(left)candidate=Previous(parent);
  if(candidate&&!Less(nodes[candidate].point,point,tolerance)){index=candidate-1;return true;}
  if(count==limit)return false;at=++count;Node& n=nodes[at];n.point=point;n.parent=parent;n.left=n.right=0;n.red=true;
  if(!parent)root=at;else if(left)nodes[parent].left=at;else nodes[parent].right=at;Balance(at);index=at-1;return true;
 }
 const WorldText::Vec3& Point(unsigned int index)const{return nodes[index+1].point;}
};
}
