#pragma once
// Recovered FEAR WorldPacker (retail v1.08) PreProcessor noise and CRT rand
// used by ConvertScatter. VA references: Noise 0x1001DD60, init 0x1001DC00
// (srand(665)). Double values are the original x87 53-bit register values;
// every explicit (float) is one of its fst/fstp dword stores. Compile with
// SSE2 doubles (/fp:precise semantics); transcendental cos/sin use x87 fcos/fsin
// like MSVCR71's default (non-SSE2) path. See scatter-research/SCATTER_RE_NOTES.md.
namespace ScatterNoise {
// MSVCR71 rand/srand: holdrand = holdrand*214013 + 2531011; (holdrand>>16)&0x7fff.
struct Random {
 unsigned int state;
 Random():state(1){}
 void Seed(unsigned int seed){state=seed;}
 int Next(){state=state*214013u+2531011u;return (int)((state>>16)&0x7fff);}
 double Unit(){return (double)Next()/32767.0;}
};
inline int Trunc(double v){return (int)v;} // _ftol: truncation toward zero
// CRT floor is exact; x87 frndint under round-down control avoids CRT helpers.
inline double Floor(double v){double r;unsigned short saved,down;__asm{fnstcw saved
 mov ax,saved
 and ax,0F3FFh
 or ax,0400h
 mov down,ax
 fldcw down
 fld v
 frndint
 fstp r
 fldcw saved}return r;}
inline double X87Sqrt(double v){double r;__asm{fld v
 fsqrt
 fstp r}return r;}
inline float X87Cos(float v){float r;__asm{fld v
 fcos
 fstp r}return r;}
inline float X87Sin(float v){float r;__asm{fld v
 fsin
 fstp r}return r;}
class Noise {
 unsigned char perm[256];float grad[256][3];
 static float F(double v){return (float)v;}
 double Dot(const float* g,float x,float y,float z)const{return ((double)z*g[2]+(double)y*g[1])+(double)x*g[0];}
 const float* G(unsigned int i)const{return grad[perm[i&255]];}
public:
 // Init (VA 0x1001DC00). Uses its own srand(665); callers reseed afterwards.
 Noise(){
  Random r;r.Seed(665);
  for(unsigned int i=0;i<256;i++){
   float u=F(r.Unit());float z=F(1.0-((double)u+(double)u));
   double one=1.0-(double)z*(double)z;float rad=F(X87Sqrt(one));
   float theta=F(r.Unit()*6.2831854820251465);
   grad[i][0]=F((double)X87Cos(theta)*rad);grad[i][1]=F((double)X87Sin(theta)*rad);grad[i][2]=z;
  }
  unsigned char used[256]={};
  for(unsigned int i=0;i<256;i++){int k=Trunc(r.Unit()*255.0);while(used[k]){k++;if(k>=256)k=0;}perm[k]=(unsigned char)i;used[k]=1;}
 }
 // Noise (VA 0x1001DD60). Returns the unrounded register value.
 double Value(float vx,float vy,float vz)const{
  int ix=Trunc((double)F(Floor(vx)));float ixf=F((double)ix);float tx=F((double)vx-ixf);float tx1=F((double)tx-1.0);float sx=F(((3.0-((double)tx+tx))*tx)*tx);
  int iy=Trunc((double)F(Floor(vy)));float iyf=F((double)iy);float ty=F((double)vy-iyf);float ty1=F((double)ty-1.0);float sy=F(((3.0-((double)ty+ty))*ty)*ty);
  int iz=Trunc((double)F(Floor(vz)));float izf=F((double)iz);float tz=F((double)vz-izf);float tz1=F((double)tz-1.0);float sz=F(((3.0-((double)tz+tz))*tz)*tz);
  unsigned int a=perm[iz&255]+iy,b=perm[a&255]+ix;                 // z0,y0 row
  float n000=F(Dot(G(b),tx,ty,tz)),n100=F(Dot(G(b+1),tx1,ty,tz));
  float nx00=F(((double)n100-n000)*sx+n000);
  unsigned int c=perm[(a+1)&255]+ix;                                 // z0,y1 row
  float n010=F(Dot(G(c),tx,ty1,tz)),n110=F(Dot(G(c+1),tx1,ty1,tz));
  float nx10=F(((double)n110-n010)*sx+n010);
  float ny0=F(((double)nx10-nx00)*sy+nx00);
  unsigned int d=perm[(iz+1)&255]+iy,e=perm[d&255]+ix;               // z1,y0 row
  float n001=F(Dot(G(e),tx,ty,tz1)),n101=F(Dot(G(e+1),tx1,ty,tz1));
  float nx01=F(((double)n101-n001)*sx+n001);
  unsigned int f=perm[(d+1)&255]+ix;                                 // z1,y1 row
  float n011=F(Dot(G(f),tx,ty1,tz1)),n111=F(Dot(G(f+1),tx1,ty1,tz1));
  float nx11=F(((double)n111-n011)*sx+n011);
  float ny1=F(((double)nx11-nx01)*sy+nx01);
  return ((double)ny1-ny0)*sz+ny0;
 }
};
}
