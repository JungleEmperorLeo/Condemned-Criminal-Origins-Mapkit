#include "WorldPacker.cpp"
#include "BspSplit.h"
extern "C" __declspec(dllimport) __declspec(noreturn) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
struct SplitCase {SplitCase* next;char name[256];BspSplit::Fragment source;BspSplit::Result result;};
class SplitReport {
 HANDLE file;bool ok;
 void Raw(const char* text){DWORD done;unsigned int size=lstrlenA(text);if(!WriteFile(file,text,size,&done,0)||done!=size)ok=false;}
 void Number(unsigned int value){char text[16],*end=Decimal(text,value);*end=0;Raw(text);}
 void String(const char* text){Raw("\"");for(;*text;text++){unsigned char c=(unsigned char)*text;if(c=='"')Raw("\\\"");else if(c=='\\')Raw("\\\\");else if(c<32){const char hex[]="0123456789abcdef";char s[7]={'\\','u','0','0',hex[c>>4],hex[c&15],0};Raw(s);}else{char s[2]={(char)c,0};Raw(s);}}Raw("\"");}
 void Bits(float value){union {float f;unsigned int u;} n;n.f=value;Number(n.u);}
 void Double(double value){union {double f;unsigned int u[2];} n;n.f=value;Raw("[");Number(n.u[0]);Raw(",");Number(n.u[1]);Raw("]");}
 void Vector(const WorldText::Vec3& v){Raw("[");Bits(v.x);Raw(",");Bits(v.y);Raw(",");Bits(v.z);Raw("]");}
 void Fragment(const BspSplit::Fragment& f){
  if(!f.count){Raw("null");return;}Raw("{\"count\":");Number(f.count);Raw(",\"ownerIndex\":");Number(f.ownerIndex);Raw(",\"sourcePolygonIndex\":");Number(f.sourcePolygonIndex);Raw(",\"surfaceFlags\":");Number(f.surfaceFlags);Raw(",\"material\":");String(f.polygon.material);Raw(",\"occlusion\":");String(f.polygon.occlusion);
  Raw(",\"verticesBits\":[");for(unsigned int i=0;i<f.count;i++){if(i)Raw(",");Vector(f.vertices[i]);}Raw("],\"vertexIds\":[");for(unsigned int i=0;i<f.count;i++){if(i)Raw(",");Number(f.polygon.indices[i]);}Raw("],\"normalBits\":");Vector(f.polygon.normal);Raw(",\"distanceBits\":");Bits(f.polygon.distance);Raw(",\"mappings\":[");
  for(unsigned int i=0;i<f.polygon.mappingCount;i++){if(i)Raw(",");const WorldText::Mapping& m=f.polygon.mappings[i];Raw("{\"channel\":");Number(m.channel);Raw(",\"validMapping\":");Raw(m.valid?"true":"false");Raw(",\"originBits\":");Vector(m.origin);Raw(",\"pBits\":");Vector(m.p);Raw(",\"qBits\":");Vector(m.q);Raw("}");}Raw("]}");
 }
public:
 SplitReport():file(INVALID_HANDLE_VALUE),ok(true){}
 bool Save(const char* path,SplitCase* first,const BspSplit::Plane& plane){
  file=CreateFileA(path,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);if(file==INVALID_HANDLE_VALUE)return false;
  Raw("{\"format\":\"bsp-split-primitives-v1\",\"sourceModelDestroyed\":true,\"plane\":{\"valid\":");Raw(plane.valid?"true":"false");Raw(",\"xBits\":");Double(plane.x);Raw(",\"yBits\":");Double(plane.y);Raw(",\"zBits\":");Double(plane.z);Raw(",\"distanceBits\":");Double(plane.distance);Raw(",\"epsilonBits\":");Double(plane.epsilon);Raw("},\"cases\":[");
  bool separate=false;
  for(SplitCase* item=first;item;item=item->next){if(separate)Raw(",");separate=true;Raw("{\"name\":");String(item->name);Raw(",\"side\":");Number(item->result.side);Raw(",\"routingSide\":");Number(item->result.routingSide);Raw(",\"success\":");Raw(item->result.success?"true":"false");Raw(",\"error\":");String(item->result.error?item->result.error:"");Raw(",\"source\":");Fragment(item->source);Raw(",\"front\":");Fragment(item->result.front);Raw(",\"back\":");Fragment(item->result.back);Raw(",\"coplanar\":");Fragment(item->result.coplanar);Raw(",\"intersections\":[");
   for(unsigned int i=0;i<item->result.intersectionCount;i++){if(i)Raw(",");const BspSplit::Intersection& hit=item->result.intersections[i];Raw("{\"firstIndex\":");Number(hit.firstIndex);Raw(",\"secondIndex\":");Number(hit.secondIndex);Raw(",\"fractionBits\":");Double(hit.fraction);Raw(",\"pointBits\":");Vector(hit.point);Raw("}");}Raw("]}");
  }Raw("]}");CloseHandle(file);file=INVALID_HANDLE_VALUE;return ok;
 }
};
static bool CopySource(BspSplit::Fragment& f,const PrefabScene::Node& n,unsigned int index,PrefabScene::Arena& arena){
 const WorldText::Polygon& p=n.geometry.faces[index];if(!BspSplit::Allocate(f,arena,p.count))return false;
 for(unsigned int i=0;i<p.count;i++)f.vertices[f.count++]=n.geometry.vertices[p.indices[i]];
 f.polygon=p;f.polygon.form=0;f.polygon.indices=0;f.polygon.mappings=0;
 f.polygon.indices=(unsigned int*)arena.Allocate(p.count*sizeof(unsigned int));if(!f.polygon.indices)return false;for(unsigned int i=0;i<p.count;i++)f.polygon.indices[i]=p.indices[i];
 if(p.mappingCount){f.polygon.mappings=(WorldText::Mapping*)arena.Allocate(p.mappingCount*sizeof(WorldText::Mapping));if(!f.polygon.mappings)return false;for(unsigned int i=0;i<p.mappingCount;i++)f.polygon.mappings[i]=p.mappings[i];}
 f.ownerIndex=n.index;f.sourcePolygonIndex=index;f.surfaceFlags=n.surfaceFlags;return true;
}
extern "C" void HarnessEntry(){
 static char input[MAX_PATH],output[MAX_PATH],text[512];
 if(!GetEnvironmentVariableA("FEAR_READER_INPUT",input,MAX_PATH)||!GetEnvironmentVariableA("FEAR_SPLIT_REPORT",output,MAX_PATH))ExitProcess(3);
 if(!GetEnvironmentVariableA("FEAR_SPLIT_PLANE",text,512))lstrcpyA(text,"(plane (1 0 0) 0 0.001)");
 WorldText::Document doc;if(!doc.Parse(text,lstrlenA(text)))ExitProcess(4);unsigned int form=doc.Form(0,"plane"),vector=doc.Value(form),d=doc.Next(vector),eps=doc.Next(d);
 WorldText::Vec3 normal;float distance=0,tolerance=0;if(!form||doc.Elements(form)!=4||!doc.Vector(vector,normal)||!doc.Number(d,distance)||!doc.Number(eps,tolerance))ExitProcess(4);
 BspSplit::Plane plane;plane.Set(normal,distance,tolerance);
 void* expansionMemory=VirtualAlloc(0,sizeof(PrefabScene::Expansion),MEM_COMMIT|MEM_RESERVE,4);if(!expansionMemory)ExitProcess(5);PrefabScene::Expansion* expansion=new(expansionMemory) PrefabScene::Expansion;
 {PrefabScene::Source source;if(!source.Load(input)||!expansion->Build(input,source.scene))ExitProcess(6);}
 PrefabScene::Arena arena;SplitCase* first=0;SplitCase** last=&first;const PrefabScene::Model& model=expansion->Scene();
 for(PrefabScene::Node* n=model.first;n;n=n->next)if(n->kind==1)for(unsigned int i=0;i<n->geometry.polygons;i++){
  SplitCase* item=(SplitCase*)arena.Allocate(sizeof(SplitCase));if(!item)ExitProcess(7);*last=item;last=&item->next;lstrcpyA(item->name,n->name);
  if(!CopySource(item->source,*n,i,arena))ExitProcess(7);item->result=BspSplit::Split(*n,i,plane,arena);
  if(item->result.success){const BspSplit::Fragment* fragments[]={&item->result.front,&item->result.back,&item->result.coplanar};for(unsigned int k=0;k<3;k++)if(fragments[k]->count){if(fragments[k]->vertices==n->geometry.vertices||fragments[k]->polygon.mappings==n->geometry.faces[i].mappings&&fragments[k]->polygon.mappingCount)ExitProcess(8);}}
 }
 expansion->~Expansion();VirtualFree(expansionMemory,0,MEM_RELEASE);
 SplitReport report;bool saved=report.Save(output,first,plane);arena.Clear();ExitProcess(saved?0:9);
}
