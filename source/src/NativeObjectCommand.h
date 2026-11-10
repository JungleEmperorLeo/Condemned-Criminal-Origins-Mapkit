#pragma once
// Runtime command finalization, original CCommandProp RVA 5E380/5EC80.
// Scene expansion has already removed comments and expanded prefab names.
// Only a separately staged Property may be passed; source text is never edited.
namespace NativeObjectCommand {
static const unsigned int MaximumBytes=4095;
inline bool Exact(const char* a,const char* b){if(!a||!b)return false;while(*a&&*a==*b){a++;b++;}return *a==*b;}
inline bool PrepareCompiled(PrefabScene::Property& property,PrefabScene::Arena& memory){
 if(!Exact(property.type,"command"))return true;
 if(!property.data)return false;unsigned int length=0;bool change=false,commentSyntax=false;
 while(length<=MaximumBytes&&property.data[length]){char c=property.data[length];if(length&&c=='/'&&property.data[length-1]=='/')commentSyntax=true;if(c=='\r'||c=='\n'||c=='{'||c=='}'||c=='\t')change=true;length++;}
 // Fresh comment syntax introduced by a prefab prefix is outside the prepared
 // input profile. LF boundaries were already removed, so reparsing would guess.
 if(length>MaximumBytes||commentSyntax)return false;if(!change)return true;
 char* output=(char*)memory.Allocate(length+1);if(!output)return false;unsigned int used=0;
 for(unsigned int i=0;i<length;i++){char c=property.data[i];if(c=='\r'||c=='\n'||c=='{'||c=='}')continue;output[used++]=c=='\t'?' ':c;}
 output[used]=0;property.data=output;return true;
}
}
