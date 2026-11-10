#pragma once
// SDK PrefetchUtilities.cpp:1231-1251 selects MP iff the first
// WorldProperties.WorldType is exactly "MultiPlayer" (LTStrEquals).
// Known runtime ordering is authored objects, then prefab objects, stable
// inside each pass. Explicit tutorial remains a legacy diagnostic subset.
namespace SourcePostProfile {
enum Mode {Disabled=0,Tutorial=1,Multiplayer=2,Singleplayer=3};
struct Request {bool automatic,tutorial,multiplayer,singleplayer;Request():automatic(false),tutorial(false),multiplayer(false),singleplayer(false){}};
inline bool Exact(const char* a,const char* b){if(!a||!b)return false;for(unsigned int i=0;i<4096;i++){if(a[i]!=b[i])return false;if(!a[i])return true;}return false;}
inline const char* Name(Mode mode){return mode==Tutorial?"tutorial":mode==Multiplayer?"multiplayer":mode==Singleplayer?"singleplayer":"none";}
class Selection {
 bool Fail(const char* message,unsigned int node=0xffffffffu){Clear();error=message;errorNode=node;return false;}
public:
 Mode mode;unsigned int sourceNode,errorNode;bool automatic;const char* error;
 Selection(){Clear();}
 void Clear(){mode=Disabled;sourceNode=errorNode=0xffffffffu;automatic=false;error=0;}
 bool Build(const PrefabScene::Model& model,const Request& request){
  Clear();unsigned int active=(request.automatic?1u:0u)+(request.tutorial?1u:0u)+(request.multiplayer?1u:0u)+(request.singleplayer?1u:0u);
  if(active>1)return Fail("Conflicting resource post-profile flags");if(!active)return true;
  if(request.tutorial){mode=Tutorial;return true;}
  if(model.count>100000)return Fail("Resource post-profile source node limit");
  const PrefabScene::Node *authored=0,*imported=0;unsigned int seen=0;
  for(const PrefabScene::Node* n=model.first;n;n=n->next){if(seen>=model.count||n->index!=seen)return Fail("Invalid resource post-profile source order");seen++;
   if(n->kind==2&&Exact(n->className,"WorldProperties")){if(n->importedFromPrefab){if(!imported)imported=n;}else if(!authored)authored=n;}
  }
  if(seen!=model.count)return Fail("Incomplete resource post-profile source model");const PrefabScene::Node* world=authored?authored:imported;
  // No WorldProperties (or no WorldType): GetObjectsOfClass/GetPropString leave the
  // SDK stack buffer unset, which is not "MultiPlayer" -> singleplayer resources.
  if(!world){if(request.multiplayer)return Fail("Explicit multiplayer post-profile requires WorldProperties.WorldType");mode=Singleplayer;automatic=request.automatic;return true;}
  const PrefabScene::Property* value=0;unsigned int properties=0;for(const PrefabScene::Property* p=world->properties;p;p=p->next){if(properties++==4096)return Fail("WorldProperties property count exceeds profile",world->index);if(Exact(p->name,"WorldType")){if(value)return Fail("Duplicate WorldProperties.WorldType",world->index);value=p;}}
  if(!value){if(request.multiplayer)return Fail("Explicit multiplayer post-profile requires WorldProperties.WorldType",world->index);mode=Singleplayer;sourceNode=world->index;automatic=request.automatic;return true;}
  if(!Exact(value->type,"string")||!value->data)return Fail("WorldProperties.WorldType requires a string value",world->index);
  unsigned int length=0;while(length<4096&&value->data[length])length++;if(length==4096)return Fail("WorldProperties.WorldType exceeds source string profile",world->index);
  Mode resolved=Exact(value->data,"MultiPlayer")?Multiplayer:Singleplayer;
  if((request.multiplayer&&resolved!=Multiplayer)||(request.singleplayer&&resolved!=Singleplayer))return Fail("Explicit resource post-profile contradicts WorldProperties.WorldType",world->index);
  mode=resolved;sourceNode=world->index;automatic=request.automatic;return true;
 }
};
}
