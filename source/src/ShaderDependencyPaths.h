#pragma once
// Pure bounded spelling rules from WorldPacker 0x7C9E0 / 0x7CA40 / 0x7C930.
// These paths are shader cache keys, not filesystem canonical identities.
namespace ShaderDependencyPaths {
inline unsigned int Length(const char* p){unsigned int n=0;while(p&&p[n])n++;return n;}
inline void Remove(char* p,unsigned int start,unsigned int count){unsigned int i=start;do{p[i]=p[i+count];}while(p[i++]);}
inline bool Directory(const char* shader,char* output,unsigned int capacity){
 if(!shader||!output||capacity<2)return false;unsigned int length=Length(shader);if(length>=capacity)return false;unsigned int slash=0;
 for(unsigned int i=0;i<length;i++){output[i]=shader[i];if(shader[i]=='/'||shader[i]=='\\')slash=i;}
 if(slash+2>capacity)return false;output[slash]='\\';output[slash+1]=0;return true;
}
inline bool Resolve(const char* directory,const char* reference,char* output,unsigned int capacity){
 if(!directory||!reference||!output||!capacity)return false;while(*reference=='/'||*reference=='\\')reference++;
 unsigned int base=Length(directory),tail=Length(reference);if(base>=capacity||tail>=capacity-base)return false;
 for(unsigned int i=0;i<base;i++)output[i]=directory[i];for(unsigned int i=0;i<=tail;i++)output[base+i]=reference[i];
 for(unsigned int i=0;output[i];i++)if(output[i]=='/')output[i]='\\';
 unsigned int i=0;while(output[i]){
  bool component=i==0||output[i-1]=='\\';
  if(component&&output[i]=='.'&&output[i+1]=='\\'){Remove(output,i,2);continue;}
  if(component&&output[i]=='.'&&output[i+1]=='.'&&output[i+2]=='\\'){
   unsigned int end=i+2;if(i<2){Remove(output,0,i+3);i=0;continue;}
   char prior=output[i-2];i-=2;if(prior!='\\')while(i){char c=output[i-1];i--;if(c=='\\')break;}
   Remove(output,i,end-i);continue;
  }
  i++;
 }
 return true;
}
}
