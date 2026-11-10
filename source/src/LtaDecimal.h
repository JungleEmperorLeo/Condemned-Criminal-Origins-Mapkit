#pragma once
namespace LtaDecimal {
// CLTAConverter::StrToReal accumulates signed 32-bit integer/fraction parts.
// Unsigned arithmetic explicitly reproduces its x86 wrap without C++ signed UB.
inline double Signed(unsigned int value){return value&0x80000000u?double(value)-4294967296.0:double(value);}
inline double Read(const char* text,unsigned int length){
 unsigned int at=0,integer=0,fraction=0,divisor=1;bool negative=false;
 if(at<length&&(text[at]=='-'||text[at]=='+'))negative=text[at++]=='-';
 if(at==length||text[at]<'0'||text[at]>'9')return 0;
 while(at<length&&text[at]>='0'&&text[at]<='9')integer=integer*10u+unsigned(text[at++]-'0');
 if(at<length&&text[at]=='.'){
  at++;while(at<length&&text[at]>='0'&&text[at]<='9'){fraction=fraction*10u+unsigned(text[at++]-'0');divisor*=10u;}
 }
 double value=Signed(integer)+Signed(fraction)/Signed(divisor);return negative?-value:value;
}
}
