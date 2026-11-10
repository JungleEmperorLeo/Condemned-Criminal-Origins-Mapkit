#pragma once
// Minimal SHA-256 (FIPS 180-4) for binary dependency pinning; no CRT.
namespace Sha256 {
class Hash {
 unsigned int h[8],w[64];unsigned char block[64];unsigned int used,total; // inputs below 512 MiB
 static unsigned int R(unsigned int x,unsigned int n){return (x>>n)|(x<<(32-n));}
 void Compress(){
  static const unsigned int k[64]={0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
   0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
   0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
   0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2};
  for(unsigned int i=0;i<16;i++)w[i]=(unsigned int)block[4*i]<<24|(unsigned int)block[4*i+1]<<16|(unsigned int)block[4*i+2]<<8|block[4*i+3];
  for(unsigned int i=16;i<64;i++){unsigned int s0=R(w[i-15],7)^R(w[i-15],18)^(w[i-15]>>3),s1=R(w[i-2],17)^R(w[i-2],19)^(w[i-2]>>10);w[i]=w[i-16]+s0+w[i-7]+s1;}
  unsigned int a=h[0],b=h[1],c=h[2],d=h[3],e=h[4],f=h[5],g=h[6],x=h[7];
  for(unsigned int i=0;i<64;i++){unsigned int t1=x+(R(e,6)^R(e,11)^R(e,25))+((e&f)^(~e&g))+k[i]+w[i],t2=(R(a,2)^R(a,13)^R(a,22))+((a&b)^(a&c)^(b&c));x=g;g=f;f=e;e=d+t1;d=c;c=b;b=a;a=t1+t2;}
  h[0]+=a;h[1]+=b;h[2]+=c;h[3]+=d;h[4]+=e;h[5]+=f;h[6]+=g;h[7]+=x;
 }
public:
 Hash():used(0),total(0){const unsigned int init[8]={0x6a09e667,0xbb67ae85,0x3c6ef372,0xa54ff53a,0x510e527f,0x9b05688c,0x1f83d9ab,0x5be0cd19};for(unsigned int i=0;i<8;i++)h[i]=init[i];}
 void Update(const void* data,unsigned int size){const unsigned char* p=(const unsigned char*)data;total+=size;while(size--){block[used++]=*p++;if(used==64){Compress();used=0;}}}
 void Final(unsigned char out[32]){unsigned int bitsHigh=total>>29,bitsLow=total<<3;unsigned char pad=0x80;Update(&pad,1);total-=1;pad=0;while(used!=56){Update(&pad,1);total-=1;}
  unsigned char length[8]={(unsigned char)(bitsHigh>>24),(unsigned char)(bitsHigh>>16),(unsigned char)(bitsHigh>>8),(unsigned char)bitsHigh,(unsigned char)(bitsLow>>24),(unsigned char)(bitsLow>>16),(unsigned char)(bitsLow>>8),(unsigned char)bitsLow};Update(length,8);for(unsigned int i=0;i<8;i++){out[4*i]=(unsigned char)(h[i]>>24);out[4*i+1]=(unsigned char)(h[i]>>16);out[4*i+2]=(unsigned char)(h[i]>>8);out[4*i+3]=(unsigned char)h[i];}}
};
// Compares the digest with a 64-character lowercase hexadecimal string.
inline bool Matches(const void* data,unsigned int size,const char* hex){Hash hash;hash.Update(data,size);unsigned char d[32];hash.Final(d);const char* digits="0123456789abcdef";for(unsigned int i=0;i<32;i++)if(hex[2*i]!=digits[d[i]>>4]||hex[2*i+1]!=digits[d[i]&15])return false;return hex[64]==0;}
}
