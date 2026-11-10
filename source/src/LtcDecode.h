#pragma once
// Based on CLTACompressedFile / CLTABitFile / lzsslimits from Jupiter.
// Bytes contain bits LSB-first; token fields are assembled MSB-first.
class LtcDecode {
 const unsigned char* source;unsigned int bytes,bit;unsigned char* window;unsigned char* valid;
 bool Read(unsigned int count,unsigned int& value){value=0;for(unsigned int i=0;i<count;i++){if(bit>=bytes*8)return Fail("Truncated LTC token");value=(value<<1)|((source[bit>>3]>>(bit&7))&1);bit++;}return true;}
 bool Fail(const char* text){error=text;errorOffset=bit/8;return false;}
 bool Pass(char* output,unsigned int& length){
  for(unsigned int i=0;i<4096;i++){window[i]=0;valid[i]=0;}bit=32;length=0;unsigned int write=1;
  for(;;){unsigned int literal;if(!Read(1,literal))return false;
   unsigned int offset=0,count=1,value=0;
   if(literal){if(!Read(8,value))return false;}
   else {
    unsigned int available=bytes*8-bit;
    if(available<12){
     // The legacy writer drops its final partial byte. Its 17-bit zero EOS
     // can leave only 9..11 offset bits on disk; do not invent payload bits.
     if(available>=9){unsigned int prefix;if(!Read(available,prefix))return false;if(!prefix){shortTerminator=true;return true;}}
     return Fail("Truncated LTC offset");
    }
    if(!Read(12,offset))return false;if(!offset)return true;if(!Read(4,count))return false;count+=2;
   }
   for(unsigned int i=0;i<count;i++){
    if(!literal){if(!valid[offset])return Fail("LTC reference to uninitialized window");value=window[offset];offset=(offset+1)&4095;}
    if(length==64*1024*1024)return Fail("Decoded LTC exceeds 64 MiB limit");
    window[write]=(unsigned char)value;valid[write]=1;write=(write+1)&4095;
    if(output)output[length]=(char)value;length++;
   }
  }
 }
public:
 char* data;unsigned int size,errorOffset;const char* error;bool shortTerminator;
 LtcDecode():source(0),bytes(0),bit(0),window(0),valid(0),data(0),size(0),errorOffset(0),error(0),shortTerminator(false){}
 ~LtcDecode(){if(window)VirtualFree(window,0,MEM_RELEASE);if(data)VirtualFree(data,0,MEM_RELEASE);}
 bool Decode(const char* input,unsigned int inputBytes){
  source=(const unsigned char*)input;bytes=inputBytes;
  if(bytes<4)return Fail("Truncated LTC version header");
  if(source[0]||source[1]||source[2]||source[3])return Fail("Unsupported LTC version");
  window=(unsigned char*)VirtualAlloc(0,8192,MEM_COMMIT|MEM_RESERVE,4);if(!window)return Fail("Could not allocate LTC window");valid=window+4096;
  if(!Pass(0,size))return false;
  data=(char*)VirtualAlloc(0,size+1,MEM_COMMIT|MEM_RESERVE,4);if(!data)return Fail("Could not allocate decoded LTC");
  unsigned int actual;if(!Pass(data,actual))return false;if(actual!=size)return Fail("LTC pass size mismatch");data[size]=0;return true;
 }
};
