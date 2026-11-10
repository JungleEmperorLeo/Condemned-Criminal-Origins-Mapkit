#pragma once
namespace AssetList {
struct Entry {unsigned int region;const char* path;}; // 0=Global; named regions use index+1.
// Packer-log notes of the original reader (0x10039590): line numbers count every
// '\n'-terminated line from 1; kind 0 = unexpected section code (text = the
// section name, 0x10035E58), kind 1 = resource without a current region.
struct Note {unsigned int line,kind;const char* text;};
class List {
 bool Equal(const char* a,const char* b){while(*a&&*a==*b){a++;b++;}return *a==*b;}
 static bool Same(const char* a,const char* b){for(;;a++,b++){char x=*a,y=*b;if(x>='A'&&x<='Z')x=(char)(x+32);if(y>='A'&&y<='Z')y=(char)(y+32);if(x!=y)return false;if(!x)return true;}}
 bool Fail(const char* message){entries=0;count=0;notes=0;noteCount=0;error=message;return false;}
public:
 PrefabScene::Arena memory;Entry* entries;Note* notes;unsigned int count,noteCount,warnings;const char* error;
 List():entries(0),notes(0),count(0),noteCount(0),warnings(0),error(0){}
 bool Read(const unsigned char* input,unsigned int size,const char* const* regions,unsigned int regionCount){
  memory.Clear();entries=0;notes=0;count=0;noteCount=0;warnings=0;error=0;
  if(!input||size>16*1024*1024||regionCount>100000||(!regions&&regionCount))return Fail("Invalid asset list input");
  for(unsigned int i=0;i<regionCount;i++)if(!regions[i]||!regions[i][0])return Fail("Invalid region name");
  unsigned int lines=1;for(unsigned int i=0;i<size;i++){if(!input[i])return Fail("NUL in asset list");if(input[i]=='\n'||input[i]=='\r')lines++;}if(lines>100000)return Fail("Too many asset list lines");
  entries=(Entry*)memory.Allocate(lines*sizeof(Entry));notes=(Note*)memory.Allocate(lines*sizeof(Note));if(!entries||!notes)return Fail("Asset list allocation failed");unsigned int cursor=0,current=0xffffffff,line=0;
  while(cursor<size){unsigned int begin=cursor;while(cursor<size&&input[cursor]!='\n')cursor++;unsigned int end=cursor;if(cursor<size)cursor++;line++;
   while(begin<end&&(input[begin]==' '||input[begin]=='\t'||input[begin]=='\r'))begin++;while(end>begin&&(input[end-1]==' '||input[end-1]=='\t'||input[end-1]=='\r'))end--;
   if(begin==end||input[begin]==';')continue;unsigned int length=end-begin;if(length>4096)return Fail("Asset list line too long");char* text=(char*)memory.Allocate(length+1);if(!text)return Fail("Asset list allocation failed");for(unsigned int i=0;i<length;i++)text[i]=(char)input[begin+i];text[length]=0;
   if(text[0]=='['){if(length<2||text[length-1]!=']')return Fail("Malformed asset section");text[length-1]=0;const char* section=text+1;
    current=0xffffffff;if(Equal(section,"Global"))current=0;else for(unsigned int i=0;i<regionCount;i++)if(Equal(section,regions[i])){current=i+1;break;}
    if(current==0xffffffff){warnings++;if(!Same(section,"Manual Assets")&&!Same(section,"Automatic Assets")){Note& n=notes[noteCount++];n.line=line;n.kind=0;n.text=section;}}continue;
   }
   if(current==0xffffffff){warnings++;Note& n=notes[noteCount++];n.line=line;n.kind=1;n.text=text;continue;}
   // Preserve entries; resource-set insertion/deduplication is a separate stage.
   entries[count].region=current;entries[count].path=text;count++;
  }
  return true;
 }
};
// Supported observed text syntax only; case/slash normalization and region-name
// matching beyond exact spelling still require original-reader experiments.
}
