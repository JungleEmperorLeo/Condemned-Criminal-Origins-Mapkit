#pragma once
namespace RenderMeshFilename {
static const unsigned int MaximumInput=4096;
static const unsigned int MaximumFilename=MaximumInput*2+10;
inline bool Length(const char* value,unsigned int& count,unsigned int maximum=MaximumInput){if(!value)return false;for(count=0;count<=maximum&&value[count];count++)if((unsigned char)value[count]>127)return false;return count<=maximum;}
inline unsigned int WithoutExtension(const char* value,unsigned int count){for(unsigned int i=count;i;i--){char c=value[i-1];if(c=='/'||c=='\\')break;if(c=='.')return i-1;}return count;}
inline unsigned char Fold(unsigned char c){return c>='A'&&c<='Z'?(unsigned char)(c+32):c;}
inline bool Absolute(const char* value){if(*value=='/'||*value=='\\')return true;for(;*value&&*value!='/'&&*value!='\\';value++)if(*value==':')return true;return false;}
struct Plan {unsigned int base,joined,size;};
inline char Joined(const char* base,const char* region,unsigned int prefix,unsigned int i){return i<prefix?base[i]:i==prefix?'_':region[i-prefix-1];}
inline bool Prepare(const char* base,const char* region,Plan& p){unsigned int a,b;if(!Length(base,a)||!Length(region,b))return false;p.base=WithoutExtension(base,a);p.joined=p.base+1+b;for(unsigned int i=p.joined;i;i--){char c=Joined(base,region,p.base,i-1);if(c=='/'||c=='\\')break;if(c=='.'){p.joined=i-1;break;}}p.size=p.joined+8;return true;}
inline char At(const char* base,const char* region,const Plan& p,unsigned int i){return i<p.joined?Joined(base,region,p.base,i):".Mesh00p"[i-p.joined];}
inline bool Absolute(const char* base,const char* region,const Plan& p){char first=At(base,region,p,0);if(first=='/'||first=='\\')return true;for(unsigned int i=0;i<p.size;i++){char c=At(base,region,p,i);if(c=='/'||c=='\\')break;if(c==':')return true;}return false;}
// Original 4F670 removes the base extension, appends '_' and the region name,
// then replaces the extension again. Both searches stop at either separator.
inline bool Build(const char* base,const char* region,char* filename,unsigned int capacity){
 Plan p;if(!filename||!Prepare(base,region,p)||p.size>=capacity)return false;for(unsigned int i=0;i<=p.size;i++)filename[i]=At(base,region,p,i);return true;
}
// Original 53FB0 uses _strnicmp on the literal project-root byte prefix.
// It performs no separator normalization and no path-boundary check. Only an
// absolute input can be shortened; an empty root matches an absolute input.
// The world writer supplies a 260-byte destination. Oversized references are
// rejected here before writing, keeping the original assertion path excluded.
inline bool Reference(const char* filename,const char* projectRoot,char* reference,unsigned int capacity,bool* relative=0){
 unsigned int n,r;if(!reference||!Length(filename,n,MaximumFilename)||!Length(projectRoot,r))return false;
 bool match=Absolute(filename)&&r<=n;for(unsigned int i=0;match&&i<r;i++)if(Fold((unsigned char)filename[i])!=Fold((unsigned char)projectRoot[i]))match=false;
 const char* source=filename+(match?r:0);unsigned int size=n-(match?r:0);if(size>=260||size>=capacity)return false;for(unsigned int i=0;i<=size;i++)reference[i]=source[i];if(relative)*relative=match;return true;
}
inline bool Build(const char* base,const char* region,const char* projectRoot,char* filename,unsigned int filenameCapacity,char* reference,unsigned int referenceCapacity,bool* relative=0){
 Plan p;unsigned int r;if(!filename||!reference||!Prepare(base,region,p)||!Length(projectRoot,r)||p.size>=filenameCapacity)return false;
 bool match=Absolute(base,region,p)&&r<=p.size;for(unsigned int i=0;match&&i<r;i++)if(Fold((unsigned char)At(base,region,p,i))!=Fold((unsigned char)projectRoot[i]))match=false;unsigned int begin=match?r:0,size=p.size-begin;if(size>=260||size>=referenceCapacity)return false;
 for(unsigned int i=0;i<=p.size;i++)filename[i]=At(base,region,p,i);for(unsigned int i=0;i<=size;i++)reference[i]=At(base,region,p,begin+i);if(relative)*relative=match;return true;
}
}
