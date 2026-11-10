#pragma once
// Included inside PrefabScene, after Matrix and Source.
class Arena {
 struct Block {Block* next;unsigned int used,capacity,padding;};Block* head;
public:
 Arena():head(0){}~Arena(){Clear();}
 void Clear(){while(head){Block* next=head->next;VirtualFree(head,0,MEM_RELEASE);head=next;}}
 unsigned int Capacity()const{unsigned int result=0;for(Block* b=head;b;b=b->next)result+=b->capacity;return result;}
 void* Allocate(unsigned int size){
  if(size>0xffffffe0u)return 0;size=(size+15)&~15u;if(!size)size=16;
  if(!head||size>head->capacity-head->used){unsigned int capacity=size>65520?size:65520;Block* b=(Block*)VirtualAlloc(0,capacity+sizeof(Block),MEM_COMMIT|MEM_RESERVE,4);if(!b)return 0;b->next=head;b->capacity=capacity;head=b;}
  void* result=(char*)(head+1)+head->used;head->used+=size;return result;
 }
};
struct Property {
 Property* next;char type[32],name[256];char* data;bool string;unsigned int flags;
};
static unsigned int SourcePropertyFlags(const WorldText::Document& doc,unsigned int form){
 unsigned int result=0,flags=doc.At(form,2);if(flags&&doc.tokens[flags].list)for(unsigned int flag=doc.First(flags);flag;flag=doc.Next(flag))if(doc.Eq(flag,"filename"))result|=0x4000u;return result;
}
struct Node {
 Node* next;unsigned int index,parent,sourceId,kind,sourceKind,surfaceFlags,preparedStart;
 char source[MAX_PATH],name[256],className[64],type[64];
 bool hidden,frozen,effectiveHidden,effectiveFrozen,knownType,importedFromPrefab,unknownType;
 unsigned short localName; // offset of the name as written in its own source file (after the instance prefix)
 WorldText::Vec3 position;Matrix orientation;Property* properties;
 WorldText::Geometry geometry;
};
struct PreparedPolygon {Node* owner;unsigned int polygonIndex;PolygonCheck::Result check;bool bspSerializationCandidate;};
class Model {
 Node* tail;
public:
 Node* first;unsigned int count;unsigned int brushes,objects,points,polygons,instances,editorOnlyBrushesRemoved;
 // Main-file prefab references whose file opens under neither name (0x10027880 walks
 // the world tree only; a missing nested prefab is silent):
 // instance name and authored prefab file, in instantiation order.
 struct MissingPrefab {MissingPrefab* next;char name[256],file[MAX_PATH];};MissingPrefab* missingPrefabs;
 bool uncentered; // CenterWorld=FALSE (0x10028A2F): every world offset is zero
 Arena memory;
 PreparedPolygon* prepared;unsigned int cleanContours,problemContours,bspCandidates,bspCleanContours;
 Model():uncentered(false),tail(0),first(0),count(0),brushes(0),objects(0),points(0),polygons(0),instances(0),editorOnlyBrushesRemoved(0),missingPrefabs(0),prepared(0),cleanContours(0),problemContours(0),bspCandidates(0),bspCleanContours(0){}
 ~Model(){Clear();}
 void Clear(){memory.Clear();uncentered=false;prepared=0;first=tail=0;missingPrefabs=0;count=brushes=objects=points=polygons=instances=editorOnlyBrushesRemoved=cleanContours=problemContours=bspCandidates=bspCleanContours=0;}
 Node* Add(){void* storage=memory.Allocate(sizeof(Node));if(!storage)return 0;Node* n=new(storage) Node;n->index=count++;n->parent=0xffffffffu;if(tail)tail->next=n;else first=n;tail=n;return n;}
 bool Prepare(){
  prepared=(PreparedPolygon*)memory.Allocate((polygons+1)*sizeof(PreparedPolygon));if(!prepared)return false;unsigned int used=0;
  for(Node* n=first;n;n=n->next)if(n->kind==1){n->preparedStart=used;for(unsigned int i=0;i<n->geometry.polygons;i++){
   PreparedPolygon& p=prepared[used++];p.owner=n;p.polygonIndex=i;p.check=PolygonCheck::Check(n->geometry,n->geometry.faces[i]);p.bspSerializationCandidate=n->knownType&&(n->surfaceFlags&8)!=0;
   if(p.check.issues)problemContours++;else cleanContours++;if(p.bspSerializationCandidate){bspCandidates++;if(!p.check.issues)bspCleanContours++;}
  }}return true;
 }
};
struct NodeMap {
 Node** data;NodeMap():data(0){}~NodeMap(){if(data)VirtualFree(data,0,MEM_RELEASE);}
 bool Allocate(unsigned int count){data=(Node**)VirtualAlloc(0,(count+1)*sizeof(Node*),MEM_COMMIT|MEM_RESERVE,4);return data!=0;}
};
class Expansion {
 public:typedef bool (*ResolveOwnerClass)(const char*,void*);
 private:
 Project project;char stack[17][MAX_PATH],errorPath[MAX_PATH];const char* error;
 unsigned int topPrefix; // name length of the world-level prefab instance being expanded (packer log names)
 // "Removing hidden and frozen objects" (0x100287A0): world-file nodes whose own
 // hidden (IgnoreHidden) or frozen (IgnoreFrozen) flag is set are deleted before
 // prefab instantiation (0x1005BCD0); each deletion appends the node's children, in
 // order, to the end of its parent's child list and the scene is rewritten as the
 // pre-order of the resulting tree. Null (folder) nodes are never deleted: authored
 // controls options-hidden-* (hidden/frozen groups keep their brushes in place).
 __declspec(noinline) bool RemoveHiddenFrozen(WorldText::Scene& s,bool hidden,bool frozen){
  if(!hidden&&!frozen)return true;unsigned int n=s.nodeCount;if(!n)return true;
  Arena scratch;unsigned int* parent=(unsigned int*)scratch.Allocate(n*4);unsigned int* head=(unsigned int*)scratch.Allocate(n*4);unsigned int* last=(unsigned int*)scratch.Allocate(n*4);unsigned int* next=(unsigned int*)scratch.Allocate(n*4);unsigned char* removed=(unsigned char*)scratch.Allocate(n);
  unsigned int* order=(unsigned int*)scratch.Allocate(n*4);unsigned int* work=(unsigned int*)scratch.Allocate((n+1)*4);unsigned int* map=(unsigned int*)scratch.Allocate(n*4);
  if(!parent||!head||!last||!next||!removed||!order||!work||!map)return Fail("Could not allocate hidden/frozen removal tables","");
  const unsigned int none=0xffffffffu;unsigned int rootFirst=none,rootLast=none;
  for(unsigned int i=0;i<n;i++){head[i]=last[i]=next[i]=none;removed[i]=0;parent[i]=s.nodes[i].parent;if(parent[i]!=none&&parent[i]>=i)return Fail("Invalid source node order","");}
  for(unsigned int i=0;i<n;i++){unsigned int p=parent[i];if(p==none){if(rootLast==none)rootFirst=i;else next[rootLast]=i;rootLast=i;}else{if(last[p]==none)head[p]=i;else next[last[p]]=i;last[p]=i;}}
  // Pre-order collection over the tree; the world root itself is never deleted.
  unsigned int collected=0,used=0;for(unsigned int r=rootFirst;r!=none;r=next[r]){work[used++]=r;while(used){unsigned int x=work[--used];if(parent[x]!=none&&s.nodes[x].kind!=0&&((hidden&&s.nodes[x].hidden)||(frozen&&s.nodes[x].frozen)))order[collected++]=x;
    unsigned int count=0;for(unsigned int c=head[x];c!=none;c=next[c])count++;if(used+count>n)return Fail("Hidden/frozen removal work overflow","");unsigned int at=used+count;for(unsigned int c=head[x];c!=none;c=next[c])work[--at]=c;used+=count;}}
  for(unsigned int k=0;k<collected;k++){unsigned int x=order[k],p=parent[x];
   // The deleted node's children are appended, in order, to the end of its parent's
   // child list (0x1005BB00).
   unsigned int previous=none;for(unsigned int c=head[p];c!=none&&c!=x;c=next[c])previous=c;if(previous==none)head[p]=next[x];else next[previous]=next[x];if(last[p]==x)last[p]=previous;
   for(unsigned int c=head[x];c!=none;){unsigned int following=next[c];next[c]=none;parent[c]=p;if(last[p]==none)head[p]=c;else next[last[p]]=c;last[p]=c;c=following;}
   removed[x]=1;head[x]=last[x]=none;}
  unsigned int kept=0;used=0;for(unsigned int r=rootFirst;r!=none;r=next[r]){work[used++]=r;while(used){unsigned int x=work[--used];map[x]=kept;order[kept++]=x;
    unsigned int count=0;for(unsigned int c=head[x];c!=none;c=next[c])count++;unsigned int at=used+count;for(unsigned int c=head[x];c!=none;c=next[c])work[--at]=c;used+=count;}}
  WorldText::SceneNode* nodes=(WorldText::SceneNode*)removalMemory.Allocate((kept+1)*sizeof(WorldText::SceneNode));if(!nodes)return Fail("Could not allocate hidden/frozen node table","");
  for(unsigned int k=0;k<kept;k++){nodes[k]=s.nodes[order[k]];nodes[k].parent=parent[order[k]]==none?none:map[parent[order[k]]];}
  s.nodes=nodes;s.nodeCount=kept;return true;
 }
 Arena removalMemory;bool ignoreHidden,ignoreFrozen;
 HANDLE file;bool writeOK,first,compilerPreparation,explicitPreparation,explicitRepair,explicitBrushReplay;unsigned int mappings,invalidPlanes,skewMappings,invalidMappings;
 Model model;
  ResolveOwnerClass resolveOwner;void* ownerContext;
  unsigned int BrushOwner(const Node* node)const{while(node){if(node->kind==2&&resolveOwner&&resolveOwner(node->className,ownerContext))return node->index+1;const unsigned int parent=node->parent;node=0;if(parent!=0xffffffffu)for(const Node* value=model.first;value;value=value->next)if(value->index==parent){node=value;break;}}return 0;}
 bool Fail(const char* message,const char* path){error=message;lstrcpynA(errorPath,path,MAX_PATH);return false;}
 bool Name(const char* prefix,const char* name,char* result){unsigned int a=lstrlenA(prefix),b=lstrlenA(name);if(a+b+(a?1:0)>=256)return Fail("Expanded name exceeds 255 bytes",name);lstrcpyA(result,prefix);if(a)result[a++]='.';lstrcpyA(result+a,name);return true;}
 bool CopyProperties(Node& node,WorldText::Scene& scene,const WorldText::SceneNode& original,const char* prefix,bool imported){
  unsigned int list=scene.doc.Container(scene.properties[original.property]);Property** last=&node.properties;
  for(unsigned int form=scene.doc.First(list);form;form=scene.doc.Next(form)){
   unsigned int key=scene.doc.Next(scene.doc.First(form)),value=scene.Data(form);
   if(scene.doc.tokens[key].length>=256||scene.doc.tokens[scene.doc.First(form)].length>=32)return Fail("Property name or type exceeds model limit",node.source);
   Property* p=(Property*)model.memory.Allocate(sizeof(Property));if(!p)return Fail("Could not allocate owned property",node.source);*last=p;last=&p->next;
   scene.doc.Copy(scene.doc.First(form),p->type,32);scene.doc.Copy(key,p->name,256);p->string=scene.doc.Eq(scene.doc.First(form),"string");p->flags=SourcePropertyFlags(scene.doc,form);
   if(FearBrushFlags::Equal(p->type,"command")){
    unsigned int length=value?scene.doc.tokens[value].length:0,used=0;
    if(length>4095)return Fail("Command exceeds 4095 bytes",node.source);
    char* expanded=(char*)model.memory.Allocate(4096);if(!expanded)return Fail("Could not allocate expanded command",node.source);
    const char* input=scene.doc.source+scene.doc.tokens[value].offset;
    for(unsigned int i=0;i<length;i++){
     char c=input[i];
     if(c=='/'&&i+1<length&&input[i+1]=='/'){while(i<length&&input[i]!='\r'&&input[i]!='\n')i++;continue;}
     if(c=='\r'||c=='\n')continue;
     const char* replacement=c=='#'&&imported?prefix:0;unsigned int bytes=replacement?lstrlenA(replacement):1;
     if(used+bytes>=4096)return Fail("Expanded command exceeds 4095 bytes",node.source);
     if(replacement){for(unsigned int k=0;k<bytes;k++)expanded[used++]=replacement[k];}else expanded[used++]=c;
    }
    expanded[used]=0;p->data=expanded;
   }else if(p->string){
    if(scene.doc.tokens[value].length>=4096)return Fail("String property exceeds 4095 bytes",node.source);
    char* text=(char*)model.memory.Allocate(4096);char* expanded=(char*)model.memory.Allocate(4096);if(!text||!expanded)return Fail("Could not allocate string property",node.source);scene.doc.Copy(value,text,4096);const char* input=FearBrushFlags::Equal(p->name,"Name")?node.name:text;unsigned int used=0;
    for(const char* c=input;*c;c++){const char* insert=(*c=='#'&&imported)?prefix:0;unsigned int length=insert?lstrlenA(insert):1;if(used+length>=4096)return Fail("Expanded property exceeds 4095 bytes",node.source);if(insert){lstrcpyA(expanded+used,insert);used+=length;}else expanded[used++]=*c;}expanded[used]=0;
    p->data=(char*)model.memory.Allocate(used+1);if(!p->data)return Fail("Could not allocate property data",node.source);lstrcpyA(p->data,expanded);
   }else{
    unsigned int length=value?scene.doc.tokens[value].length:0;
    p->data=(char*)model.memory.Allocate(length+1);if(!p->data)return Fail("Could not allocate raw property data",node.source);
    for(unsigned int i=0;i<length;i++)p->data[i]=scene.doc.source[scene.doc.tokens[value].offset+i];p->data[length]=0;
   }
  }return true;
 }
 bool CopyGeometry(Node& node,const WorldText::Geometry& original,const Matrix& transform,const WorldText::Vec3& offset,bool imported){
  WorldText::Geometry& g=node.geometry;g.points=original.points;g.polygons=original.polygons;g.references=original.references;
  g.vertices=(WorldText::Vec3*)model.memory.Allocate((g.points+1)*sizeof(WorldText::Vec3));
  g.colors=(WorldText::PointColor*)model.memory.Allocate((g.points+1)*sizeof(WorldText::PointColor));
  g.faces=(WorldText::Polygon*)model.memory.Allocate((g.polygons+1)*sizeof(WorldText::Polygon));
  g.indices=(unsigned int*)model.memory.Allocate((g.references+1)*sizeof(unsigned int));
  if(!g.vertices||!g.faces||!g.indices||!g.colors)return Fail("Could not allocate owned brush geometry",node.source);
  for(unsigned int i=0;i<g.points;i++){if(original.colors)g.colors[i]=original.colors[i];else g.colors[i].r=g.colors[i].g=g.colors[i].b=g.colors[i].a=255;}
  WorldText::Vec3 brushOffset=offset;char replayMode[4]={};bool replay=imported&&g.points&&(explicitPreparation?explicitBrushReplay:(GetEnvironmentVariableA("FEAR_PREFAB_BRUSH_MOVE_REPLAY",replayMode,4)&&replayMode[0]=='1'));
#ifdef FEAR_PREFAB_IGNORE_BRUSH_POSITION_EXPERIMENT
  brushOffset.x=brushOffset.y=brushOffset.z=0;
#endif
  if(replay){WorldText::Vec3 corner=transform.Apply(original.vertices[0]);for(unsigned int i=1;i<g.points;i++){WorldText::Vec3 v=transform.Apply(original.vertices[i]);if(v.x<corner.x)corner.x=v.x;if(v.y>corner.y)corner.y=v.y;if(v.z>corner.z)corner.z=v.z;}WorldText::Vec3 target={corner.x+offset.x,corner.y+offset.y,corner.z+offset.z};brushOffset.x=target.x-corner.x;brushOffset.y=target.y-corner.y;brushOffset.z=target.z-corner.z;}
  for(unsigned int i=0;i<g.points;i++){WorldText::Vec3 v=imported?transform.Apply(original.vertices[i]):original.vertices[i];if(imported){v.x+=brushOffset.x;v.y+=brushOffset.y;v.z+=brushOffset.z;}if(!WorldText::FiniteVector(v))return Fail("Could not build finite transformed vertices",node.source);g.vertices[i]=v;}
  for(unsigned int i=0;i<g.references;i++)g.indices[i]=original.indices[i];unsigned int used=0;
  for(unsigned int i=0;i<g.polygons;i++){
   const WorldText::Polygon& source=original.faces[i];WorldText::Polygon& p=g.faces[i];p.count=source.count;p.indices=g.indices+used;used+=p.count;p.mappingCount=source.mappingCount;
   lstrcpyA(p.material,source.material);lstrcpyA(p.occlusion,source.occlusion);p.validPlane=WorldText::ComputePlane(g.vertices,p.indices,p.count,p.normal,p.distance);
   if(p.mappingCount){p.mappings=(WorldText::Mapping*)model.memory.Allocate(p.mappingCount*sizeof(WorldText::Mapping));if(!p.mappings)return Fail("Could not allocate owned texture mappings",node.source);}
   for(unsigned int k=0;k<p.mappingCount;k++){
    p.mappings[k]=source.mappings[k];WorldText::Mapping& m=p.mappings[k];bool transformMapping=imported&&m.valid;
#ifdef FEAR_NONFINITE_ORIGIN_EXPERIMENT
    if(imported&&WorldText::FiniteVector(m.p)&&WorldText::FiniteVector(m.q))transformMapping=true;
#endif
    if(transformMapping){m.origin=transform.TextureOrigin(m.origin);m.origin.x+=brushOffset.x;m.origin.y+=brushOffset.y;m.origin.z+=brushOffset.z;m.p=transform.TextureDirection(m.p);m.q=transform.TextureDirection(m.q);
     if(!WorldText::FiniteVector(m.p)||!WorldText::FiniteVector(m.q))return Fail("Non-finite transformed texture directions",node.source);
#ifndef FEAR_NONFINITE_ORIGIN_EXPERIMENT
     if(!WorldText::FiniteVector(m.origin))return Fail("Non-finite transformed texture origin",node.source);
#endif
    }
#ifdef FEAR_MAPPING_SKEW_REPAIR_EXPERIMENT
    SourceMappingSkewRepair::Repair(p.normal,m);
#endif
#ifdef FEAR_LOCAL_MAPPING_ZERO_EXPERIMENT
    if(!imported)for(unsigned int component=0;component<3;component++){
     if((&m.origin.x)[component]==0)(&m.origin.x)[component]=0;
     if((&m.p.x)[component]==0)(&m.p.x)[component]=0;
     if((&m.q.x)[component]==0)(&m.q.x)[component]=0;
    }
#endif
   }
  }return true;
 }
 static bool FileExists(const char* path){HANDLE file=CreateFileA(path,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);if(file==INVALID_HANDLE_VALUE)return false;CloseHandle(file);return true;}
 static bool PrefabFileExists(const char* path){
  if(FileExists(path))return true;
  char alternative[MAX_PATH];unsigned int length=lstrlenA(path);
  if(length<MAX_PATH&&length>=9&&FearBrushFlags::Equal(path+length-9,".World00a")){lstrcpyA(alternative,path);alternative[length-1]='c';return FileExists(alternative);}
  return false;
 }
 bool Walk(WorldText::Scene& scene,const char* path,const char* prefix,const Matrix& transform,unsigned int depth,Node* parent,bool imported,unsigned int firstSource=0xffffffffu,unsigned int lastSource=0xffffffffu,Node** sourceBrushes=0,unsigned char* brushSeen=0){
  NodeMap map;if(!map.Allocate(scene.nodeCount))return Fail("Could not allocate hierarchy map",path);if(imported)map.data[0]=parent;
  // Repeated references in MAIN/same ownership keep the first brush. Distinct
  // non-MAIN WorldModels own separate copies of the same source polyhedron.
  // Classification is an explicit project metadata callback, never a name rule.
  // Newly instantiated prefabs own a new set; authored descendants share it.
  NodeMap brushMap;Arena brushMemory;
  if(!sourceBrushes){if(!brushMap.Allocate(scene.geometryCount))return Fail("Could not allocate source brush identity map",path);sourceBrushes=brushMap.data;brushSeen=(unsigned char*)brushMemory.Allocate(scene.geometryCount+1);if(!brushSeen)return Fail("Could not allocate source brush reference marks",path);}
  unsigned int begin=firstSource==0xffffffffu?(imported?1u:0u):firstSource,limit=lastSource==0xffffffffu?scene.nodeCount:lastSource;
  for(unsigned int i=begin;i<limit;i++){
   const WorldText::SceneNode& original=scene.nodes[i];if(model.count>=100000)return Fail("Expanded node limit exceeded",path);
   Node* ancestor=original.parent==0xffffffffu||original.parent<begin?parent:map.data[original.parent];
   bool repeatedBrush=false;
   if(original.kind==1){if(brushSeen[original.geometry]){Node* existing=sourceBrushes[original.geometry];const unsigned int firstOwner=existing&&existing->kind==1?BrushOwner(existing):0,currentOwner=BrushOwner(ancestor);if(!firstOwner||!currentOwner||firstOwner==currentOwner){map.data[i]=existing;continue;}repeatedBrush=true;}else brushSeen[original.geometry]=1;}
   // Original visualization removal reparents children rather than deleting
   // their subtree. Compiler preparation omits only the brush itself before
   // model counts, ownership, geometry bounds, tags and normal pools exist.
   // 0x10028670 deletes a brush whose string Type is missing as well.
   if(compilerPreparation&&original.kind==1&&(FearBrushFlags::EditorOnly(original.type)||original.typeless)){
    map.data[i]=ancestor;sourceBrushes[original.geometry]=ancestor;model.editorOnlyBrushesRemoved++;continue;
   }
   Node* n=model.Add();if(!n)return Fail("Could not allocate owned scene node",path);map.data[i]=n;
   if(original.kind==1&&!repeatedBrush)sourceBrushes[original.geometry]=n;
   n->sourceId=original.id;n->sourceKind=original.kind;n->kind=original.kind==3?0:original.kind;n->surfaceFlags=original.surfaceFlags;n->knownType=original.knownType;n->unknownType=original.unknownType;
   n->hidden=original.hidden;n->frozen=original.frozen;n->effectiveHidden=original.hidden||(ancestor&&ancestor->effectiveHidden);n->effectiveFrozen=original.frozen||(ancestor&&ancestor->effectiveFrozen);
   if(ancestor)n->parent=ancestor->index;lstrcpyA(n->source,path);lstrcpyA(n->className,original.className);lstrcpyA(n->type,original.type);
   if(!Name(original.kind?prefix:"",original.name,n->name))return false;n->localName=(unsigned short)(original.kind&&depth==1&&prefix[0]?topPrefix+1:0);n->position=imported?transform.Apply(original.position):original.position;
   if(!WorldText::FiniteVector(n->position))return Fail("Non-finite transformed node position",path);
   if(!Matrix::Euler(original.rotation,n->orientation))return Fail(original.kind==3?"Prefab rotation outside supported range":"Object rotation outside supported range",path);
   // Root nodes retain their authored Euler matrix. Even an identity product
   // changes signed zero components and thus later quaternion/normal bytes.
   // Only prefab instantiation performs the orientation composition.
   if(imported)n->orientation=transform.Times(n->orientation);n->importedFromPrefab=imported;
   if(!CopyProperties(*n,scene,original,prefix,imported))return false;
   if(original.kind==3){
    // Existing authored descendants retain their source transform and names.
    // Instantiated prefab content is appended after this subtree, before the
    // next source sibling. The global render frame sort depends on that order.
    unsigned int subtreeEnd=i+1;while(subtreeEnd<limit&&scene.nodes[subtreeEnd].parent!=0xffffffffu&&scene.nodes[subtreeEnd].parent>=i)subtreeEnd++;
    if(subtreeEnd>i+1&&!Walk(scene,path,prefix,transform,depth,n,imported,i+1,subtreeEnd,sourceBrushes,brushSeen))return false;
    if(depth>=16||model.instances>=4096)return Fail("Prefab expansion limit exceeded",path);
    if(!project.resources[0])return Fail(project.error?project.error:"No resource root for prefab",path);
    char relative[MAX_PATH],target[MAX_PATH];unsigned int token=scene.doc.Value(scene.doc.Form(original.form,"prefabfile"));if(scene.doc.tokens[token].length>=MAX_PATH)return Fail("Prefab filename exceeds MAX_PATH",path);scene.doc.Copy(token,relative,MAX_PATH);
    if(!Join(project.resources,relative,target))return Fail("Prefab path exceeds MAX_PATH",path);for(unsigned int j=0;j<=depth;j++)if(FearBrushFlags::Equal(stack[j],target))return Fail("Recursive prefab cycle",target);
    Matrix local;if(!Matrix::Euler(original.rotation,local))return Fail("Prefab rotation outside supported range",path);local.m[0][3]=original.position.x;local.m[1][3]=original.position.y;local.m[2][3]=original.position.z;
    Matrix combined=transform.Times(local);
    // Original prefab cache (VA 0x1005C990 -> 0x1005C7D0) tries the file and its
    // alternative name and, when neither opens, silently yields no prefab: the
    // instance contributes nothing. Only an absent file is treated so here; a
    // present but unreadable/invalid prefab still fails.
    if(!PrefabFileExists(target)){Model::MissingPrefab* missing=depth?0:(Model::MissingPrefab*)model.memory.Allocate(sizeof(Model::MissingPrefab));if(missing){missing->next=0;lstrcpynA(missing->name,n->name,256);lstrcpynA(missing->file,relative,MAX_PATH);Model::MissingPrefab** tail=&model.missingPrefabs;while(*tail)tail=&(*tail)->next;*tail=missing;}i=subtreeEnd-1;continue;}
    // Both names reach the same cached loaded prefab, so its instances share
    // prototypes (SDK Cafeteria: vase_01.World00a and .World00c references).
    if(!FileExists(target)){unsigned int length=lstrlenA(target);target[length-1]='c';}
    Source child;bool loaded=explicitPreparation?child.LoadExplicit(target,compilerPreparation,explicitRepair):child.Load(target,compilerPreparation);if(!loaded)return Fail(child.scene.doc.error,target);model.instances++;lstrcpyA(stack[depth+1],target);if(!depth)topPrefix=lstrlenA(n->name);if(!Walk(child.scene,target,n->name,combined,depth+1,n,true))return false;i=subtreeEnd-1;
   }else if(original.kind==1){
    const WorldText::Geometry& g=scene.geometry[original.geometry];if(g.points>1000000-model.points)return Fail("Expanded vertex limit exceeded",path);if(!CopyGeometry(*n,g,transform,original.position,imported))return false;model.brushes++;model.points+=g.points;model.polygons+=g.polygons;
   }else if(original.kind==2)model.objects++;
  }return true;
 }
 void Raw(const char* text){DWORD done;unsigned int size=lstrlenA(text);if(!WriteFile(file,text,size,&done,0)||done!=size)writeOK=false;}
 void Number(unsigned int value){char text[16],*p=Decimal(text,value);*p=0;Raw(text);}
 void String(const char* text){Raw("\"");for(;*text;text++){unsigned char c=(unsigned char)*text;char s[2]={(char)c,0};if(c=='"')Raw("\\\"");else if(c=='\\')Raw("\\\\");else if(c<32){const char hex[]="0123456789abcdef";char esc[7]={'\\','u','0','0',hex[c>>4],hex[c&15],0};Raw(esc);}else Raw(s);}Raw("\"");}
 void Bits(float f){union {float f;unsigned int u;} v;v.f=f;Number(v.u);}
 void Vector(const WorldText::Vec3& v){Raw("[");Bits(v.x);Raw(",");Bits(v.y);Raw(",");Bits(v.z);Raw("]");}
 void Mat(const Matrix& m){Raw("[");for(unsigned int i=0;i<3;i++)for(unsigned int j=0;j<4;j++){if(i||j)Raw(",");Bits(m.m[i][j]);}Raw("]");}
 void Emit(const Node& n){
  if(!first)Raw(",");first=false;Raw("{\"source\":");String(n.source);Raw(",\"sourceNodeId\":");Number(n.sourceId);Raw(",\"kind\":");Number(n.kind);Raw(",\"modelIndex\":");Number(n.index);Raw(",\"parentModelIndex\":");Number(n.parent);Raw(",\"name\":");String(n.name);Raw(",\"className\":");String(n.className);
  Raw(",\"hidden\":");Raw(n.hidden?"true":"false");Raw(",\"frozen\":");Raw(n.frozen?"true":"false");Raw(",\"effectiveHidden\":");Raw(n.effectiveHidden?"true":"false");Raw(",\"effectiveFrozen\":");Raw(n.effectiveFrozen?"true":"false");Raw(",\"positionBits\":");Vector(n.position);Raw(",\"orientationMatrixBits\":");Mat(n.orientation);
  Raw(",\"stringProperties\":{");bool propFirst=true;for(Property* p=n.properties;p;p=p->next)if(p->string){if(!propFirst)Raw(",");propFirst=false;String(p->name);Raw(":");String(p->data);}Raw("},\"rawProperties\":[");propFirst=true;for(Property* p=n.properties;p;p=p->next)if(!p->string){if(!propFirst)Raw(",");propFirst=false;Raw("{\"name\":");String(p->name);Raw(",\"type\":");String(p->type);Raw(",\"sourceData\":");String(p->data);Raw("}");}Raw("]");
  if(n.kind==1){
   const WorldText::Geometry& g=n.geometry;Raw(",\"surfaceFlags\":");Number(n.surfaceFlags);Raw(",\"knownType\":");Raw(n.knownType?"true":"false");Raw(",\"type\":");String(n.type);Raw(",\"verticesBits\":[");for(unsigned int i=0;i<g.points;i++){if(i)Raw(",");Vector(g.vertices[i]);}Raw("],\"faces\":[");for(unsigned int i=0;i<g.polygons;i++){if(i)Raw(",");Raw("[");for(unsigned int k=0;k<g.faces[i].count;k++){if(k)Raw(",");Number(g.faces[i].indices[k]);}Raw("]");}Raw("],\"polygonData\":[");
   for(unsigned int i=0;i<g.polygons;i++){
    if(i)Raw(",");const WorldText::Polygon& p=g.faces[i];PolygonCheck::Result check=model.prepared[n.preparedStart+i].check;if(!p.validPlane)invalidPlanes++;
    Raw("{\"material\":");String(p.material);Raw(",\"occlusion\":");String(p.occlusion);Raw(",\"validPlane\":");Raw(p.validPlane?"true":"false");Raw(",\"normalBits\":");Vector(p.normal);Raw(",\"distanceBits\":");Bits(p.distance);Raw(",\"maxPlaneResidualBits\":");Bits(check.residual);Raw(",\"preBspIssues\":");Number(check.issues);Raw(",\"mappings\":[");
    for(unsigned int k=0;k<p.mappingCount;k++){if(k)Raw(",");const WorldText::Mapping& m=p.mappings[k];float dot=m.valid?WorldText::Dot(m.p,m.q):0;bool skew=dot>0.001f||dot< -0.001f;mappings++;if(skew)skewMappings++;if(!m.valid)invalidMappings++;Raw("{\"channel\":");Number(m.channel);Raw(",\"validMapping\":");Raw(m.valid?"true":"false");Raw(",\"originBits\":");Vector(m.origin);Raw(",\"pBits\":");Vector(m.p);Raw(",\"qBits\":");Vector(m.q);Raw(",\"legacyOrthogonalizationRequired\":");Raw(skew?"true":"false");Raw("}");}Raw("]}");
   }Raw("]");
  }Raw("}");
 }
public:
  Expansion(bool prepareForCompiler=false):ignoreHidden(false),ignoreFrozen(false),error(0),file(INVALID_HANDLE_VALUE),writeOK(true),first(true),compilerPreparation(prepareForCompiler),explicitPreparation(false),explicitRepair(false),explicitBrushReplay(false),mappings(0),invalidPlanes(0),skewMappings(0),invalidMappings(0),resolveOwner(0),ownerContext(0){errorPath[0]=0;}
 ~Expansion(){if(file!=INVALID_HANDLE_VALUE)CloseHandle(file);}
 const char* Error()const{return error;}
 const char* ErrorPath()const{return errorPath;}
 const Model& Scene()const{return model;}
  bool Build(const char* input,WorldText::Scene& source){model.Clear();error=0;errorPath[0]=0;explicitPreparation=false;resolveOwner=0;ownerContext=0;project=Project();project.Locate(input);Matrix identity;bool ok=Full(input,stack[0])&&Walk(source,stack[0],"",identity,0,0,false);if(ok&&!model.Prepare())ok=Fail("Could not allocate prepared polygon list",input);if(!ok)model.Clear();return ok;}
 // Explicit compile-local settings also apply to every recursively loaded
 // prefab. Environment overrides cannot alter this production preparation.
  bool Build(const char* input,WorldText::Scene& source,const Project& selected,bool repairEnabled=true,bool brushMoveReplay=false,ResolveOwnerClass owners=0,void* ownerMetadata=0){
   model.Clear();error=0;errorPath[0]=0;explicitPreparation=true;explicitRepair=repairEnabled;explicitBrushReplay=brushMoveReplay;project=selected;resolveOwner=owners;ownerContext=ownerMetadata;
  if(!input||!input[0]||!project.file[0]||!project.resources[0]||project.error)return Fail("Invalid explicit expansion project",input?input:"");
  if(!Full(input,stack[0]))return Fail("Invalid explicit expansion source path",input);
  if(!RemoveHiddenFrozen(source,ignoreHidden,ignoreFrozen))return false;
  Matrix identity;bool ok=Walk(source,stack[0],"",identity,0,0,false);if(ok&&!model.Prepare())ok=Fail("Could not allocate prepared polygon list",input);if(!ok)model.Clear();return ok;
 }
 void SetUncentered(bool value){model.uncentered=value;}
 void SetIgnore(bool hidden,bool frozen){ignoreHidden=hidden;ignoreFrozen=frozen;}
 bool SaveBuilt(const char* output,bool success){
  writeOK=true;first=true;mappings=invalidPlanes=skewMappings=invalidMappings=0;
  file=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(file==INVALID_HANDLE_VALUE)return false;
  Raw("{\"format\":\"expanded-source-scene-v1\",\"ownedModel\":true,\"project\":");String(project.file);Raw(",\"resourceRoot\":");String(project.resources);Raw(",\"nodes\":[");for(Node* n=model.first;n;n=n->next)if(n->kind==1||n->kind==2)Emit(*n);
  Raw("],\"hierarchy\":[");bool groupFirst=true;for(Node* n=model.first;n;n=n->next){if(!groupFirst)Raw(",");groupFirst=false;Raw("{\"index\":");Number(n->index);Raw(",\"parent\":");Number(n->parent);Raw(",\"kind\":");Number(n->kind);Raw(",\"sourceKind\":");Number(n->sourceKind);Raw(",\"source\":");String(n->source);Raw(",\"sourceNodeId\":");Number(n->sourceId);Raw(",\"name\":");String(n->name);Raw(",\"effectiveHidden\":");Raw(n->effectiveHidden?"true":"false");Raw(",\"effectiveFrozen\":");Raw(n->effectiveFrozen?"true":"false");Raw("}");}
  Raw("],\"success\":");Raw(success?"true":"false");Raw(",\"error\":");String(error?error:"");Raw(",\"errorPath\":");String(errorPath);
  Raw(",\"modelNodes\":");Number(model.count);Raw(",\"brushes\":");Number(model.brushes);Raw(",\"objects\":");Number(model.objects);Raw(",\"points\":");Number(model.points);Raw(",\"polygons\":");Number(model.polygons);Raw(",\"prefabInstances\":");Number(model.instances);
  Raw(",\"editorOnlyBrushesRemoved\":");Number(model.editorOnlyBrushesRemoved);
  Raw(",\"arenaCapacityBytes\":");Number(model.memory.Capacity());
  Raw(",\"textureMappings\":");Number(mappings);Raw(",\"invalidPlanes\":");Number(invalidPlanes);Raw(",\"skewMappings\":");Number(skewMappings);Raw(",\"invalidMappings\":");Number(invalidMappings);
  Raw(",\"preparation\":{\"planeToleranceBits\":");Bits(0.01f);Raw(",\"cleanContours\":");Number(model.cleanContours);Raw(",\"problemContours\":");Number(model.problemContours);Raw(",\"bspSerializationCandidates\":");Number(model.bspCandidates);Raw(",\"bspCleanContours\":");Number(model.bspCleanContours);Raw("}");
  Raw(",\"limitations\":[\"No BSP builder or World00p writer\",\"Legacy texture orthogonalization, non-string property transforms and Euler recovery pending\",\"Visibility inheritance recorded but compiler filtering unverified\",\"Floating-point parity with FEAR unverified\"]}");CloseHandle(file);file=INVALID_HANDLE_VALUE;return success&&writeOK;
 }
 bool Save(const char* output,const char* input,WorldText::Scene& source){bool success=Build(input,source);return SaveBuilt(output,success);}
};


