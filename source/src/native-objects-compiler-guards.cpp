#define FEAR_PRIMARY_SURFACE_PREPARATION 1
#include "WorldPacker.cpp"
#include "NativeObjectsCompiler.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
static unsigned int values[96],used=0;
#define CHECK(name,test) values[used++]=(test)?1u:0u
static bool Include(const PrefabScene::Node&){return true;}
static bool Equal(const void* a,const void* b,unsigned int bytes){const unsigned char* x=(const unsigned char*)a,*y=(const unsigned char*)b;for(unsigned int i=0;i<bytes;i++)if(x[i]!=y[i])return false;return true;}
static bool CodecEmpty(const ObjectsBlock::Block& b){return !b.data&&!b.size&&!b.count&&!b.properties&&b.error;}
static void Property(PrefabScene::Property& p,const char* type,const char* name,const char* data){memset(&p,0,sizeof(p));lstrcpyA(p.type,type);lstrcpyA(p.name,name);p.data=const_cast<char*>(data);p.string=FearBrushFlags::Equal(type,"string");}
static bool Write(HANDLE f,const void* p,unsigned int n){DWORD done=0;return !n||(WriteFile(f,p,n,&done,0)&&done==n);}
static bool Filename(PrefabScene::Arena& memory,const char* source,const char* expected,unsigned int flags=0x4000u,const char* type="string"){
 PrefabScene::Property p;Property(p,type,"ArbitraryResourceProperty",source);p.flags=flags;return NativeObjectProperties::PrepareFilename(p,memory)&&GameClassMetadata::Exact(p.data,expected);
}
extern "C" void HarnessEntry(){
 PrefabScene::Arena memory;ObjectsBlock::Block codec;ObjectsBlock::Settings limits;WorldText::Vec3 offset={1,2,3};PrefabScene::Model model;PrefabScene::Node* node=model.Add();if(!node)ExitProcess(2);node->kind=2;lstrcpyA(node->className,"SyntheticRuntimeClass");node->position.x=4;node->position.y=6;node->position.z=8;WorldText::Vec3 angles={};if(!PrefabScene::Matrix::Euler(angles,node->orientation))ExitProcess(2);
 static PrefabScene::Property p[9];const char* types[9]={"string","vector","color","real","bool","longint","rotation","command","stringid"};const char* names[9]={"Name","Pos","Tint","Mass","Enabled","Index","Rotation","Command","LocalizedID"};const char* data[9]={"Synthetic","(vector (9 8 7))","(vector (1 2 3))","1.25","1","-2147483648","(eulerangles (0 0 0))","msg target ON","IDS_SYNTHETIC"};for(unsigned int i=0;i<9;i++){Property(p[i],types[i],names[i],data[i]);if(i<8)p[i].next=p+i+1;}node->properties=p;
 CHECK("explicit codec supports all nine disk property kinds",codec.BuildConfigured(model,Include,offset,limits)&&codec.count==1&&codec.properties==9);
 unsigned int size=codec.size;unsigned char* saved=(unsigned char*)memory.Allocate(size);if(!saved)ExitProcess(2);memcpy(saved,codec.data,size);
 CHECK("repeat explicit codec is deterministic",codec.BuildConfigured(model,Include,offset,limits)&&codec.size==size&&Equal(codec.data,saved,size));
 limits.maximumBytes=size;CHECK("exact output budget succeeds",codec.BuildConfigured(model,Include,offset,limits)&&codec.size==size);limits.maximumBytes=size-1;CHECK("short output budget clears complete prior output",!codec.BuildConfigured(model,Include,offset,limits)&&CodecEmpty(codec));limits=ObjectsBlock::Settings();
 limits.maximumProperties=8;CHECK("property count limit clears output with source diagnostic",!codec.BuildConfigured(model,Include,offset,limits)&&CodecEmpty(codec)&&codec.errorNode==node->index&&FearBrushFlags::Equal(codec.errorProperty,"LocalizedID"));limits=ObjectsBlock::Settings();
 limits.maximumPoolBytes=4;CHECK("pool budget enforced before publication",!codec.BuildConfigured(model,Include,offset,limits)&&CodecEmpty(codec));limits=ObjectsBlock::Settings();
 limits.version=2;CHECK("settings version rejects and clears",!codec.BuildConfigured(model,Include,offset,limits)&&CodecEmpty(codec));limits=ObjectsBlock::Settings();
 CHECK("missing inclusion callback rejected",!codec.BuildConfigured(model,0,offset,limits)&&CodecEmpty(codec));
 p[5].data=const_cast<char*>("2147483647");CHECK("maximum int32 accepted exactly",codec.BuildConfigured(model,Include,offset,limits));p[5].data=const_cast<char*>("2147483648");CHECK("positive int32 overflow rejected",!codec.BuildConfigured(model,Include,offset,limits)&&CodecEmpty(codec));p[5].data=const_cast<char*>("-2147483649");CHECK("negative int32 overflow rejected",!codec.BuildConfigured(model,Include,offset,limits)&&CodecEmpty(codec));p[5].data=const_cast<char*>("12.5");CHECK("nonintegral long rejected",!codec.BuildConfigured(model,Include,offset,limits)&&CodecEmpty(codec));p[5].data=const_cast<char*>(data[5]);
 p[4].data=const_cast<char*>("2");CHECK("boolean outside zero one rejected",!codec.BuildConfigured(model,Include,offset,limits)&&CodecEmpty(codec));p[4].data=const_cast<char*>(data[4]);
 p[8].data=0;CHECK("null stringid rejected with property diagnostic",!codec.BuildConfigured(model,Include,offset,limits)&&CodecEmpty(codec)&&FearBrushFlags::Equal(codec.errorProperty,"LocalizedID"));p[8].data=const_cast<char*>("");CHECK("empty stringid writes actual pooled empty string",codec.BuildConfigured(model,Include,offset,limits));p[8].data=const_cast<char*>(data[8]);
 unsigned char* longText=(unsigned char*)memory.Allocate(4097);if(!longText)ExitProcess(2);memset(longText,'x',4096);longText[4096]=0;p[8].data=(char*)longText;CHECK("oversized stringid rejected",!codec.BuildConfigured(model,Include,offset,limits)&&CodecEmpty(codec));longText[4095]=0;CHECK("4095 byte stringid boundary accepted",codec.BuildConfigured(model,Include,offset,limits));p[8].data=const_cast<char*>(data[8]);
 lstrcpyA(p[0].type,"unsupported");CHECK("unknown property type rejected",!codec.BuildConfigured(model,Include,offset,limits)&&CodecEmpty(codec));lstrcpyA(p[0].type,"string");
 unsigned int nan=0x7fc00000u;memcpy(&node->position.x,&nan,4);CHECK("nonfinite source position rejected",!codec.BuildConfigured(model,Include,offset,limits)&&CodecEmpty(codec));node->position.x=4;
 p[8].next=p;CHECK("property cycle bounded by count limit",!codec.BuildConfigured(model,Include,offset,limits)&&CodecEmpty(codec));p[8].next=0;
 model.count++;CHECK("incomplete source node count rejected",!codec.BuildConfigured(model,Include,offset,limits)&&CodecEmpty(codec));model.count--;node->next=node;CHECK("source node cycle bounded by count",!codec.BuildConfigured(model,Include,offset,limits)&&CodecEmpty(codec));node->next=0;
 CHECK("explicit codec recovers exact bytes after failures",codec.BuildConfigured(model,Include,offset,limits)&&codec.size==size&&Equal(codec.data,saved,size));model.Clear();CHECK("codec payload survives source model destruction",codec.size==size&&Equal(codec.data,saved,size));
 const char* forms="(string \"A\" (filename hidden) (data \"x\")) (string \"B\" (hidden filename model) (data \"x\")) (string \"C\" (hidden filename) (data \"x\")) (string \"D\" (Filename) (data \"x\")) (string \"E\" (hidden) (data \"filename\"))";WorldText::Document flagDoc;if(!flagDoc.Parse(forms,lstrlenA(forms)))ExitProcess(2);unsigned int form=flagDoc.First(0);
 CHECK("filename first token preserved from unlabelled source flags",PrefabScene::SourcePropertyFlags(flagDoc,form)==0x4000u);form=flagDoc.Next(form);
 CHECK("filename middle token preserved from unlabelled source flags",PrefabScene::SourcePropertyFlags(flagDoc,form)==0x4000u);form=flagDoc.Next(form);
 CHECK("filename last token preserved from unlabelled source flags",PrefabScene::SourcePropertyFlags(flagDoc,form)==0x4000u);form=flagDoc.Next(form);
 CHECK("source flag spelling is case sensitive",PrefabScene::SourcePropertyFlags(flagDoc,form)==0);form=flagDoc.Next(form);
 CHECK("filename text outside flag list is not a flag",PrefabScene::SourcePropertyFlags(flagDoc,form)==0);
 CHECK("authored and compressed world filename extensions convert",Filename(memory,"Dir/World.World00a","Dir/World.World00p")&&Filename(memory,"Dir/World.World00c","Dir/World.World00p"));
 CHECK("model filename conversion preserves prefix and folds extension case",Filename(memory,"Models\\Thing.mOdEl00C","Models\\Thing.Model00p")&&Filename(memory,"Thing.Model00a","Thing.Model00p"));
 CHECK("FX source filename extensions convert",Filename(memory,"Thing.Fx00a","Thing.Fx00p")&&Filename(memory,"Thing.Fx00c","Thing.Fx00p"));
 CHECK("texture animation extension has no packed p suffix",Filename(memory,"Thing.TxAnm00a","Thing.TxAnm00")&&Filename(memory,"Thing.TxAnm00c","Thing.TxAnm00"));
 CHECK("source textures convert to DDS",Filename(memory,"Thing.Texture00a","Thing.dds")&&Filename(memory,"Thing.Texture00c","Thing.dds"));
 CHECK("database source filenames convert",Filename(memory,"Thing.Gamdb00s","Thing.Gamdb00p")&&Filename(memory,"Thing.Strdb00s","Thing.Strdb00p"));
 CHECK("unflagged resource-looking string is preserved",Filename(memory,"Thing.Model00c","Thing.Model00c",0));
 CHECK("nonstring property is outside filename conversion",Filename(memory,"Thing.Model00c","Thing.Model00c",0x4000u,"command"));
 CHECK("dot in either directory separator is outside extension",Filename(memory,"Dir.Model00c/name","Dir.Model00c/name")&&Filename(memory,"Dir.Model00c\\name","Dir.Model00c\\name"));
 CHECK("unknown packed empty and trailing-dot filenames are preserved",Filename(memory,"Thing.Key00c","Thing.Key00c")&&Filename(memory,"Thing.Model00p","Thing.Model00p")&&Filename(memory,"","")&&Filename(memory,"Thing.","Thing."));
 CHECK("last extension alone controls conversion",Filename(memory,"Thing.Model00c.backup","Thing.Model00c.backup")&&Filename(memory,"Thing.old.Model00c","Thing.old.Model00p"));
 char boundary[261];memset(boundary,'x',260);boundary[260]=0;PrefabScene::Property bounded;Property(bounded,"string","Bounded",boundary);bounded.flags=0x4000u;
 CHECK("260 byte filename rejected instead of truncating",!NativeObjectProperties::PrepareFilename(bounded,memory));boundary[259]=0;CHECK("259 byte filename accepted",NativeObjectProperties::PrepareFilename(bounded,memory));boundary[259]='x';
 CHECK("long unflagged string retains normal string budget",Filename(memory,boundary,boundary,0));bounded.data=0;CHECK("null flagged filename rejected",!NativeObjectProperties::PrepareFilename(bounded,memory));
 const char* immutable="Folder\\Thing.Model00c";Property(bounded,"string","Resource",immutable);bounded.flags=0x4000u;CHECK("conversion owns new text and preserves borrowed source",NativeObjectProperties::PrepareFilename(bounded,memory)&&bounded.data!=immutable&&GameClassMetadata::Exact(immutable,"Folder\\Thing.Model00c")&&GameClassMetadata::Exact(bounded.data,"Folder\\Thing.Model00p"));
 char source[MAX_PATH],project[MAX_PATH],output[MAX_PATH];if(!GetEnvironmentVariableA("FEAR_READER_INPUT",source,MAX_PATH)||!GetEnvironmentVariableA("FEAR_READER_PROJECT",project,MAX_PATH)||!GetEnvironmentVariableA("FEAR_NATIVE_OBJECT_GUARDS_OUTPUT",output,MAX_PATH))ExitProcess(1);
 void* place=memory.Allocate(sizeof(NativePreparedRenderScene::Context));if(!place)ExitProcess(2);NativePreparedRenderScene::Context* scene=new(place)NativePreparedRenderScene::Context;NativeObjectsCompiler::Builder compiler;NativeObjectsCompiler::Settings settings;
 CHECK("shared object compiler rejects unprepared scene",!compiler.Build(*scene)&&!compiler.data&&!compiler.size&&!compiler.count&&compiler.error==compiler.diagnostic);
 if(!scene->Build(source,project))ExitProcess(3);CHECK("native profile defaults to recovered quaternion conversion",settings.rotationReplay);
 CHECK("shared compiler writes complete BetterBox runtime table",compiler.Build(*scene)&&compiler.count==10&&compiler.properties==577);
 unsigned int sourceSize=compiler.size;unsigned char* retained=(unsigned char*)memory.Allocate(sourceSize);if(!retained)ExitProcess(2);memcpy(retained,compiler.data,sourceSize);
 PrefabScene::Model& live=const_cast<PrefabScene::Model&>(scene->Model());PrefabScene::Node* selected=live.first;while(selected&&selected->kind!=2)selected=selected->next;if(!selected)ExitProcess(2);char className[64];lstrcpyA(className,selected->className);lstrcpyA(selected->className,"GuardUnknownClass");
 CHECK("unknown class clears previously complete shared output",!compiler.Build(*scene)&&!compiler.data&&!compiler.size&&!compiler.count&&!compiler.properties&&compiler.errorNode==selected->index);lstrcpyA(selected->className,className);
 settings.maximumObjects=1;CHECK("shared object budget enforced",!compiler.Build(*scene,settings)&&!compiler.data&&!compiler.count);settings=NativeObjectsCompiler::Settings();
 CHECK("shared compiler reuses after failure with exact bytes",compiler.Build(*scene,settings)&&compiler.size==sourceSize&&Equal(compiler.data,retained,sourceSize));scene->Clear();scene->~Context();CHECK("shared output owns bytes after source scene destruction",compiler.size==sourceSize&&compiler.count==10&&Equal(compiler.data,retained,sourceSize));compiler.Clear();CHECK("shared Clear resets diagnostics and all public state",!compiler.data&&!compiler.size&&!compiler.count&&!compiler.properties&&!compiler.error&&!compiler.diagnostic[0]&&!compiler.errorProperty[0]&&compiler.errorNode==0xffffffffu);
 HANDLE file=CreateFileA(output,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);unsigned int header[3]={0x31474f4e,used,size};if(file==INVALID_HANDLE_VALUE||!Write(file,header,12)||!Write(file,values,used*4)||!Write(file,saved,size))ExitProcess(4);CloseHandle(file);for(unsigned int i=0;i<used;i++)if(!values[i])ExitProcess(5);ExitProcess(0);
}
