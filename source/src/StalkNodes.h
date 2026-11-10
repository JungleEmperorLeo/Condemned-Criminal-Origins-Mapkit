#pragma once
// AINodeStalk conversion predicate (original ConvertStalkNodes, VA 0x1000B600):
// the class name is compared with strcmp over the node list; the node needs
// exactly one child, that child must be a brush, and the object must carry a
// StalkDataIndex property. Otherwise the original only logs and skips the node.
// A converted node's child brush is deleted ("Converting AI Nodes", before Nav
// and geometry import, after world centering).
namespace StalkNodes {
inline bool Same(const char* a,const char* b){if(!a)return false;for(;*a==*b;a++,b++)if(!*a)return true;return false;}
inline bool IsStalk(const PrefabScene::Node& n){return n.kind==2&&Same(n.className,"AINodeStalk");}
inline bool HasIndex(const PrefabScene::Node& n){for(const PrefabScene::Property* p=n.properties;p;p=p->next)if(Same(p->name,"StalkDataIndex"))return true;return false;}
// The converted child brush, or 0 when the original skips the node.
inline const PrefabScene::Node* ConvertedChild(const PrefabScene::Node& n){
 if(!IsStalk(n)||!HasIndex(n))return 0;const PrefabScene::Node* child=0;unsigned int children=0;
 for(const PrefabScene::Node* c=n.next;c;c=c->next)if(c->parent==n.index){child=c;children++;}
 return children==1&&child->kind==1?child:0;
}
}
