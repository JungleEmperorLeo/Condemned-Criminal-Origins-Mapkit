#pragma once
class SceneReport {
 HANDLE file;
 void Raw(const char* s){DWORD written;WriteFile(file,s,lstrlenA(s),&written,0);}
 void Number(unsigned int value){char b[16],*p=b;p=Decimal(p,value);*p=0;Raw(b);}
 void String(const char* s){Raw("\"");while(*s){char c=*s++;if(c=='"')Raw("\\\"");else if(c=='\\')Raw("\\\\");else if(c=='\n')Raw("\\n");else if(c=='\r')Raw("\\r");else if(c=='\t')Raw("\\t");else {char b[2]={c,0};Raw(b);}}Raw("\"");}
 void Field(const char* key,unsigned int value){String(key);Raw(":");Number(value);Raw(",");}
 void Bits(const WorldText::Vec3& v){union {float f;unsigned int u;} x;x.f=v.x;Raw("[");Number(x.u);Raw(",");x.f=v.y;Number(x.u);Raw(",");x.f=v.z;Number(x.u);Raw("]");}
public:
 SceneReport():file(INVALID_HANDLE_VALUE){}
 ~SceneReport(){if(file!=INVALID_HANDLE_VALUE)CloseHandle(file);}
 bool Save(const char* path,bool success,WorldText::Scene& scene,bool compressed,unsigned int inputBytes,unsigned int decodedBytes,bool decoderError,bool shortTerminator){
  file=CreateFileA(path,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(file==INVALID_HANDLE_VALUE)return false;
  Raw("{\"success\":");Raw(success?"true":"false");Raw(",\"compressed\":");Raw(compressed?"true":"false");Raw(",\"legacyShortTerminator\":");Raw(shortTerminator?"true":"false");Raw(",\"errorDomain\":");String(decoderError?"compressed-bytes":"decoded-text");Raw(",");Field("inputBytes",inputBytes);Field("decodedBytes",decodedBytes);Raw("\"error\":");String(scene.doc.error?scene.doc.error:"");Raw(",");Field("errorOffset",scene.doc.errorOffset);
  unsigned int line=1,column=1;for(unsigned int i=0;i<scene.doc.errorOffset&&i<scene.doc.bytes;i++){if(scene.doc.source[i]=='\n'){line++;column=1;}else column++;}
  Field("errorLine",line);Field("errorColumn",column);
  Field("storedGeometries",scene.geometryCount);Field("storedPropertyLists",scene.propertyCount);Field("activeNodes",scene.nodeCount);Field("activeBrushes",scene.brushCount);Field("activeObjects",scene.objectCount);Field("prefabReferences",scene.prefabCount);Field("unknownBrushTypes",scene.unknownTypes);Field("activePoints",scene.activePoints);Field("activePolygons",scene.activePolygons);
  Raw("\"nodes\":[");
  if(success)for(unsigned int i=0;i<scene.nodeCount;i++){
   if(i)Raw(",");const WorldText::SceneNode& n=scene.nodes[i];Raw("{");Field("nodeId",n.id);Field("parentIndex",n.parent);Field("kind",n.kind);Field("propertyId",n.property);Field("geometryIndex",n.geometry);Field("surfaceFlags",n.surfaceFlags);
   Raw("\"type\":");String(n.type);Raw(",\"knownType\":");Raw(n.knownType?"true":"false");Raw(",\"hidden\":");Raw(n.hidden?"true":"false");Raw(",\"frozen\":");Raw(n.frozen?"true":"false");
   Raw(",\"className\":");String(n.className);Raw(",\"name\":");String(n.name);
   Raw(",\"positionBits\":");Bits(n.position);Raw(",\"rotationBits\":");Bits(n.rotation);
   if(n.kind==1){const WorldText::Geometry& g=scene.geometry[n.geometry];Raw(",");Field("points",g.points);Field("polygons",g.polygons);Field("vertexReferences",g.references);Raw("\"firstPointBits\":");if(g.points)Bits(g.vertices[0]);else Raw("null");}
   if(n.kind==3){char prefab[MAX_PATH];unsigned int p=scene.doc.Value(scene.doc.Form(n.form,"prefabfile"));scene.doc.Copy(p,prefab,sizeof(prefab));Raw(",\"prefabFile\":");String(prefab);}
   Raw("}");
  }
  Raw("]}");return true;
 }
};
