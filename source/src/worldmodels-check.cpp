#include "WorldPacker.cpp"
#include "WorldModels.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
static WorldModels::ClassKind Resolve(const PrefabScene::Node& n,void*){
 if(FearBrushFlags::Equal(n.className,"Moving"))return WorldModels::WorldModel;
 if(FearBrushFlags::Equal(n.className,"Ordinary"))return WorldModels::Ordinary;
 return WorldModels::Unknown;
}
static bool Ignore(const PrefabScene::Node& n,void*){return n.effectiveHidden;}
static PrefabScene::Node* Add(PrefabScene::Model& m,unsigned int kind,unsigned int parent,const char* cls=""){
 PrefabScene::Node* n=m.Add();if(!n)ExitProcess(100);n->kind=kind;n->parent=parent;lstrcpyA(n->className,cls);
 if(kind==1){m.brushes++;n->geometry.polygons=6;}return n;
}
extern "C" void HarnessEntry(){
 PrefabScene::Model m;WorldModels::Partition p;
 if(!p.Build(m,Resolve,0)||p.groupCount!=1||p.assignmentCount)ExitProcess(1);
 Add(m,0,0xffffffffu);Add(m,1,0);Add(m,2,0,"Moving");Add(m,0,2);Add(m,1,3);
 Add(m,2,3,"Moving");Add(m,1,5);Add(m,2,2,"Ordinary");Add(m,1,7);Add(m,2,0,"Moving");
 if(!p.Build(m,Resolve,0)||p.groupCount!=4||p.assignmentCount!=4)ExitProcess(2);
 if(p.assignments[0].group!=0||p.assignments[1].group!=1||p.assignments[2].group!=2||p.assignments[3].group!=1)ExitProcess(3);
 if(p.groups[1].brushes!=2||p.groups[1].polygons!=12||p.groups[2].brushes!=1||p.groups[3].brushes)ExitProcess(4);
 m.first->next->effectiveHidden=true;
 if(!p.Build(m,Resolve,0,Ignore)||p.ignoredBrushes!=1||p.assignmentCount!=3||p.groups[0].brushes)ExitProcess(5);
 if(p.Build(m,0,0)||p.groups||p.assignments||p.groupCount||!p.error)ExitProcess(6);
 if(!p.Build(m,Resolve,0))ExitProcess(7);
 PrefabScene::Node* last=m.first;while(last->next)last=last->next;last->parent=last->index;
 if(p.Build(m,Resolve,0)||p.groups||!p.error)ExitProcess(8);
 last->parent=0;lstrcpyA(last->className,"UnknownClass");
 if(p.Build(m,Resolve,0)||p.assignments||!p.error)ExitProcess(9);
 lstrcpyA(last->className,"Moving");m.brushes++;
 if(p.Build(m,Resolve,0)||p.groups||!p.error)ExitProcess(10);
 m.brushes--;if(!p.Build(m,Resolve,0)||p.assignmentCount!=4)ExitProcess(11);
 p.Clear();if(p.groups||p.assignments||p.error||p.groupCount)ExitProcess(12);
 ExitProcess(0);
}
