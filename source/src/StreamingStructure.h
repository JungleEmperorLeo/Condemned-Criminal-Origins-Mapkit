#pragma once
#include "StalkNodes.h"
// Streaming region/link/info conversion of the original (World Streaming,
// VA 0x1003A3E0), shared by every consumer of the region table:
//  - walk 0x10037410 in pre-order with the enclosing region R and link L:
//    a brush below R or L joins L, else R (0x10036DB0); StreamingRegion
//    (_stricmp): inside R -> error "embedded", children stay with R; else it
//    needs a real MemoryLimit (0x10035860, MB*1048576+0.5) or it is ignored
//    with an error and its children have no region; StreamingLink: outside R
//    or inside L -> error and ignored (children keep R, L); without a LinkTo
//    property -> error and ignored (children keep R only); any other object
//    below R -> error "non streaming object"; StreamingInfo outside R: a second
//    one -> error; GlobalLimit then StreamingLimit (each stored as read) or
//    error (severity 3); the info position is the node position (default 0);
//  - no info -> warning; info position outside every region's own brushes
//    (0x10034480, before the removals below) -> warning;
//  - names 0x10035A90: "Global" or a later equal name (_stricmp) -> error, fatal;
//  - 0x10036A30: links without brushes, then regions without brushes and links
//    are removed with a warning;
//  - 0x10037740: a link whose LinkTo names no region or its own region -> error;
//    its brushes are appended to its region's own brushes.
// Messages carry severity 1 (warning) or 2/3 (error) and the reporting node's name.
// After the stage the original deletes every StreamingRegion, StreamingLink and
// StreamingInfo node with its subtree (0x10036CF0): Removed() marks them.
namespace StreamingStructure {
static const unsigned int None=0xffffffffu,MaximumMessages=4096;
struct Message {unsigned int severity;const char* node;const char* text;};
struct Region {const PrefabScene::Node* node;unsigned int budget;bool removed;};
struct Link {const PrefabScene::Node* node;const char* target;unsigned int region;int linked;bool removed;};
struct Entry {const PrefabScene::Node* brush;unsigned int region,link,order;bool moved;};
// One volume of the serialized region table, in the original's order.
struct Volume {const PrefabScene::Node* brush;unsigned int region;int linked;bool moved;};
inline bool Same(const char* a,const char* b){return FearBrushFlags::Equal(a,b);}
class Builder {
 PrefabScene::Arena memory;Builder(const Builder&);Builder& operator=(const Builder&);
 bool Fail(const char* why){error=why;return false;}
 static const PrefabScene::Property* Find(const PrefabScene::Node& n,const char* name){for(const PrefabScene::Property* p=n.properties;p;p=p->next)if(FearBrushFlags::Equal(p->name,name))return p;return 0;}
 // 0x100350A0: a real property, value*1048576+0.5 truncated (the former Budget reader).
 static bool Real(const PrefabScene::Node& n,const char* name,unsigned int& out){
  const PrefabScene::Property* p=Find(n,name);if(!p||!p->data)return false;WorldText::Document doc;float value;
  if(!doc.Parse(p->data,lstrlenA(p->data))||!doc.Number(doc.First(0),value)||value<0||value>=4096)return false;double bytes=(double)value*1048576.0+0.5;if(bytes>=4294967296.0)return false;
  out=bytes>=2147483648.0?0x80000000u+(unsigned int)(int)(bytes-2147483648.0):(unsigned int)(int)bytes;return true;
 }
 void Report(unsigned int severity,const char* node,const char* text){if(messageCount<MaximumMessages){Message& m=messages[messageCount++];m.severity=severity;m.node=node;m.text=text;}if(severity==1)warnings++;else errors++;}
public:
 Region* regions;Link* links;Entry* entries;Message* messages;unsigned char* removed;
 unsigned int regionCount,linkCount,entryCount,messageCount,errors,warnings,globalBudget,streamingBudget,count;
 const PrefabScene::Node* info;WorldText::Vec3 infoPosition;bool fatal;const char* error;
 Builder():regions(0),links(0),entries(0),messages(0),removed(0),regionCount(0),linkCount(0),entryCount(0),messageCount(0),errors(0),warnings(0),globalBudget(0),streamingBudget(0),count(0),info(0),fatal(false),error(0){infoPosition.x=infoPosition.y=infoPosition.z=0;}
 // Walk, info checks before the removals happen in Inside (caller supplies the
 // volume test over the region's own brushes), then names, removals and links.
 typedef bool (*InsideTest)(void* context,const PrefabScene::Node& brush,const WorldText::Vec3& centered);
 bool Build(const PrefabScene::Model& model,const WorldText::Vec3& offset,InsideTest inside=0,void* context=0){
  memory.Clear();regions=0;links=0;entries=0;messages=0;removed=0;regionCount=linkCount=entryCount=messageCount=errors=warnings=globalBudget=streamingBudget=count=0;info=0;fatal=false;error=0;infoPosition.x=infoPosition.y=infoPosition.z=0;
  if(!model.count||model.count>200000)return Fail("Invalid streaming source model");count=model.count;
  regions=(Region*)memory.Allocate(model.count*sizeof(Region));links=(Link*)memory.Allocate(model.count*sizeof(Link));entries=(Entry*)memory.Allocate((model.brushes+1)*sizeof(Entry));messages=(Message*)memory.Allocate(MaximumMessages*sizeof(Message));
  unsigned int* contextRegion=(unsigned int*)memory.Allocate(model.count*4);unsigned int* contextLink=(unsigned int*)memory.Allocate(model.count*4);removed=(unsigned char*)memory.Allocate(model.count);unsigned char* deleted=(unsigned char*)memory.Allocate(model.count);
  if(!regions||!links||!entries||!messages||!contextRegion||!contextLink||!removed||!deleted)return Fail("Streaming structure allocation failed");
  memset(removed,0,model.count);memset(deleted,0,model.count);
  // ConvertStalkNodes deleted converted child brushes before this stage.
  for(const PrefabScene::Node* n=model.first;n;n=n->next)if(const PrefabScene::Node* c=StalkNodes::ConvertedChild(*n))if(c->index<model.count)deleted[c->index]=1;
  bool infoFound=false;unsigned int seen=0;
  for(const PrefabScene::Node* n=model.first;n;n=n->next){
   if(n->index!=seen||seen>=model.count||(n->parent!=None&&n->parent>=seen))return Fail("Invalid streaming source ancestry");seen++;
   unsigned int R=n->parent==None?None:contextRegion[n->parent],L=n->parent==None?None:contextLink[n->parent];
   // 0x10036CF0: the first Region/Link/Info on a path is deleted with its subtree.
   removed[n->index]=(unsigned char)(n->parent!=None&&removed[n->parent]?1:0);
   if(!removed[n->index]&&n->kind==2&&(Same(n->className,"StreamingRegion")||Same(n->className,"StreamingLink")||Same(n->className,"StreamingInfo")))removed[n->index]=2;
   contextRegion[n->index]=R;contextLink[n->index]=L;
   if(n->kind==1){if(deleted[n->index]||(R==None&&L==None))continue;if(entryCount>model.brushes)return Fail("Streaming brush count");Entry& e=entries[entryCount];e.brush=n;e.region=L!=None?links[L].region:R;e.link=L;e.order=entryCount;e.moved=false;entryCount++;continue;}
   if(n->kind!=2)continue;
   if(Same(n->className,"StreamingRegion")){
    if(R!=None){Report(2,n->name,"Found a streaming region object embedded within another streaming region object. This object will be ignored");continue;}
    unsigned int budget=0;if(!Real(*n,"MemoryLimit",budget)){Report(2,n->name,"Encountered streaming region without a memory limit property associated with it. Ignoring region.");continue;}
    Region& r=regions[regionCount];r.node=n;r.budget=budget;r.removed=false;contextRegion[n->index]=regionCount++;contextLink[n->index]=None;continue;
   }
   if(Same(n->className,"StreamingLink")){
    if(R==None){Report(2,n->name,"Found a linking region outside of a streaming region. Ignoring this linking region");continue;}
    if(L!=None){Report(2,n->name,"Found a linking region within another linking region. Ignoring this linking region");continue;}
    const PrefabScene::Property* target=Find(*n,"LinkTo");if(!target){Report(2,n->name,"Encountered linking region without a link to property associated with it. Ignoring region.");contextLink[n->index]=None;continue;}
    Link& l=links[linkCount];l.node=n;l.target=target->data?target->data:"";l.region=R;l.linked=-1;l.removed=false;contextLink[n->index]=linkCount++;continue;
   }
   if(R!=None){Report(2,n->name,"Found a non streaming object beneath a streaming region. This is invalid and the object must be removed from beneath the streaming region");continue;}
   if(Same(n->className,"StreamingInfo")){
    if(infoFound){Report(2,n->name,"Found more than one streaming info object. This object will be ignored.");continue;}
    if(!Real(*n,"GlobalLimit",globalBudget)||!Real(*n,"StreamingLimit",streamingBudget)){Report(3,n->name,"Found a streaming info object that did not contain all of the necessary properties. Please make sure to get the latest object code and update world properties.");continue;}
    infoFound=true;info=n;infoPosition.x=n->position.x-offset.x;infoPosition.y=n->position.y-offset.y;infoPosition.z=n->position.z-offset.z;
   }
  }
  if(!infoFound)Report(1,0,"Failed to find any streaming info objects. Make sure that one is placed within the level.");
  bool located=false;
  if(inside)for(unsigned int r=0;r<regionCount&&!located;r++)for(unsigned int e=0;e<entryCount&&!located;e++)if(entries[e].region==r&&entries[e].link==None&&inside(context,*entries[e].brush,infoPosition))located=true;
  if(!located)Report(1,0,"Streaming info object is not located within any streaming region so no streaming region will be loaded by default.");
  for(unsigned int r=0;r<regionCount;r++){const char* name=regions[r].node->name;
   if(Same(name,"Global")){Report(2,name,"Encountered a streaming region that was named the same as the reserved name for the global streaming region. This causes severe issues and therefore further processing cannot occur.");fatal=true;}
   for(unsigned int s=r+1;s<regionCount;s++)if(Same(regions[s].node->name,name)){Report(2,regions[s].node->name,"Encountered a streaming region with a duplicate name. This causes severe issues and therefore further processing cannot occur.");fatal=true;}}
  if(fatal)return true;
  for(unsigned int r=0;r<regionCount;r++){unsigned int own=0,kept=0;
   for(unsigned int l=0;l<linkCount;l++){if(links[l].region!=r)continue;unsigned int n=0;for(unsigned int e=0;e<entryCount;e++)if(entries[e].link==l)n++;
    if(!n){links[l].removed=true;Report(1,links[l].node->name,"Found a linking region with no contained brushes. This linking region will be ignored");}else kept++;}
   for(unsigned int e=0;e<entryCount;e++)if(entries[e].region==r&&entries[e].link==None)own++;
   if(!own&&!kept){regions[r].removed=true;Report(1,regions[r].node->name,"Found a streaming region with no contained brushes. This streaming region will be ignored");}}
  unsigned int moved=entryCount;
  for(unsigned int r=0;r<regionCount;r++)if(!regions[r].removed)for(unsigned int l=0;l<linkCount;l++){Link& link=links[l];if(link.region!=r||link.removed)continue;
   for(unsigned int t=0;t<regionCount&&link.linked<0;t++)if(!regions[t].removed&&Same(regions[t].node->name,link.target))link.linked=(int)t;
   if(link.linked>=0&&(unsigned int)link.linked!=r)continue;
   Report(2,link.node->name,"This linking region was either pointing to an invalid streaming region or to its parent streaming region. This linking region will be ignored.");
   link.removed=true;link.linked=-1;for(unsigned int e=0;e<entryCount;e++)if(entries[e].link==l){entries[e].link=None;entries[e].moved=true;entries[e].order=moved++;}}
  return true;
 }
 // Valid regions in table order; index of a region in that table, None if removed.
 unsigned int TableIndex(unsigned int region)const{if(region>=regionCount||regions[region].removed)return None;unsigned int k=0;for(unsigned int r=0;r<region;r++)if(!regions[r].removed)k++;return k;}
 unsigned int TableCount()const{unsigned int k=0;for(unsigned int r=0;r<regionCount;r++)if(!regions[r].removed)k++;return k;}
 // Volumes per valid region: own brushes (walk order, then moved link brushes),
 // then each valid link's brushes in link and walk order.
 unsigned int Volumes(Volume* out,unsigned int capacity)const{
  unsigned int n=0;
  for(unsigned int r=0;r<regionCount;r++){if(regions[r].removed)continue;unsigned int table=TableIndex(r);
   for(unsigned int pass=0;pass<2;pass++){unsigned int next=0;for(;;){unsigned int best=None,bestOrder=None;for(unsigned int e=0;e<entryCount;e++){const Entry& x=entries[e];if(x.region!=r||x.link!=None||x.moved!=(pass==1)||x.order<next)continue;if(x.order<bestOrder){bestOrder=x.order;best=e;}}if(best==None)break;if(n==capacity)return None;out[n].brush=entries[best].brush;out[n].region=table;out[n].linked=-1;out[n].moved=pass==1;n++;next=bestOrder+1;}}
   for(unsigned int l=0;l<linkCount;l++){if(links[l].region!=r||links[l].removed)continue;for(unsigned int e=0;e<entryCount;e++)if(entries[e].link==l){if(n==capacity)return None;out[n].brush=entries[e].brush;out[n].region=table;out[n].linked=(int)TableIndex((unsigned int)links[l].linked);out[n].moved=false;n++;}}
  }
  return n;
 }
};
}
