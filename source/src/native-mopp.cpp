// native-mopp.exe MESH OUTPUT: compiles a physics mesh prefix with the native MOPP
// compiler (src/NativeMoppCompiler.h) and writes CodeInfo, u32 length and code.
// Used by mopp-backend/backend.py (profile "native"). Exit 0 on success.
#include <stdio.h>
#include "NativeMoppPrefix.h"
int main(int argc,char** argv){
 if(argc!=3){fprintf(stderr,"Usage: native-mopp MESH OUTPUT\n");return 2;}
 FILE* in=fopen(argv[1],"rb");if(!in){fprintf(stderr,"Cannot read mesh\n");return 3;}
 fseek(in,0,SEEK_END);long size=ftell(in);fseek(in,0,SEEK_SET);if(size<0||size>16*1024*1024+1){fclose(in);fprintf(stderr,"Mesh prefix size outside supported limits\n");return 4;}
 unsigned char* mesh=new unsigned char[size?size:1];size_t got=fread(mesh,1,size,in);fclose(in);if((long)got!=size){fprintf(stderr,"Cannot read mesh\n");return 3;}
 unsigned char* out=0;unsigned int bytes=0;const char* error=NativeMoppPrefix::Generate(mesh,(unsigned int)size,out,bytes);delete[] mesh;
 if(error){fprintf(stderr,"%s\n",error);return 5;}
 FILE* o=fopen(argv[2],"wb");if(!o||fwrite(out,1,bytes,o)!=bytes){if(o)fclose(o);fprintf(stderr,"Cannot write output\n");return 6;}
 fclose(o);delete[] out;return 0;
}
