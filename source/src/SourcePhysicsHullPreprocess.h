#pragma once
// Recovered deterministic Hull preprocessing, x86 scalar x87 profile.
// Source implementation: no original DLL, runtime address or binary blob.
// Private instruction order preserves float stores and sorted duplicate choices.
namespace SourcePhysicsHullPreprocess {
struct Vec4 {float x,y,z,w;};
struct Array {Vec4* data;int size;unsigned int capacity;};
enum Status {Success=0,InvalidInput,NonfiniteResult};
inline bool Finite(float value){union B {float f;unsigned int u;};B b;b.f=value;return (b.u&0x7f800000u)!=0x7f800000u;}
inline bool Valid(const Array& a){
 if(a.size<=0||a.size>65535||!a.data||(a.capacity&0x7fffffffu)<static_cast<unsigned int>(a.size))return false;
 for(int i=0;i<a.size;++i){const Vec4& p=a.data[i];if(!Finite(p.x)||!Finite(p.y)||!Finite(p.z)||!Finite(p.w))return false;}
 return true;
}
#if defined(_MSC_VER) && defined(_M_IX86)
namespace detail {
static const unsigned int K_88e5c=0x3f800000u;
static const unsigned int K_88f74=0x3f000000u;
static const unsigned int K_89268=0x3c23d70au;
static const unsigned int K_9147c=0x34000000u;
static const unsigned int K_9f420=0x00000000u;
static const unsigned int K_9f424=0x00000000u;
static const unsigned int K_9f428=0x00000000u;
static __declspec(naked) void CapacityInvariantFailure(){__asm {int 3}
 __asm {ret}
}
static void LessCore();
static void SortCore();
static void AabbCore();
static void NormalizeCore();
static void UnnormalizeCore();
static void DeduplicateCore();
static void MatrixProductCore();
static void MatrixProductInPlaceCore();
static __declspec(naked) void LessCore(){
  __asm {
    mov         ecx,dword ptr [esp+8]
    fld         dword ptr [ecx]
    mov         edx,dword ptr [esp+0Ch]
    fcomp       dword ptr [edx]
    fnstsw      ax
    test        ah,5
    jp L_6529b
L_65293:
    mov         eax,dword ptr [esp+4]
    mov         byte ptr [eax],1
    ret
L_6529b:
    fld         dword ptr [edx]
    fld         dword ptr [ecx]
    fucompp
    fnstsw      ax
    test        ah,44h
    jp L_652b5
    fld         dword ptr [ecx+4]
    fcomp       dword ptr [edx+4]
    fnstsw      ax
    test        ah,5
    jnp L_65293
L_652b5:
    fld         dword ptr [edx]
    fld         dword ptr [ecx]
    fucompp
    fnstsw      ax
    test        ah,44h
    jp L_652de
    fld         dword ptr [edx+4]
    fld         dword ptr [ecx+4]
    fucompp
    fnstsw      ax
    test        ah,44h
    jp L_652de
    fld         dword ptr [ecx+8]
    fcomp       dword ptr [edx+8]
    fnstsw      ax
    test        ah,5
    jnp L_65293
L_652de:
    mov         eax,dword ptr [esp+4]
    mov         byte ptr [eax],0
    ret
  }
}

static __declspec(naked) void SortCore(){
  __asm {
    push        ebp
    mov         ebp,esp
    and         esp,0FFFFFFF0h
    sub         esp,34h
    push        ebx
    push        esi
    mov         esi,dword ptr [ebp+14h]
    push        edi
    nop
L_65390:
    mov         eax,dword ptr [ebp+0Ch]
    mov         edi,dword ptr [ebp+10h]
    mov         ebx,dword ptr [ebp+8]
    mov         dword ptr [esp+18h],eax
    add         eax,edi
    sar         eax,1
    shl         eax,4
    mov         ecx,dword ptr [eax+ebx]
    mov         edx,dword ptr [eax+ebx+4]
    add         eax,ebx
    mov         dword ptr [esp+20h],ecx
    mov         ecx,dword ptr [eax+8]
    mov         dword ptr [esp+24h],edx
    mov         edx,dword ptr [eax+0Ch]
    mov         dword ptr [esp+28h],ecx
    mov         dword ptr [esp+2Ch],edx
L_653c3:
    mov         eax,dword ptr [esp+18h]
    shl         eax,4
    lea         ecx,[esp+20h]
    add         eax,ebx
    push        ecx
    push        eax
    lea         edx,[esp+1Eh]
    push        edx
    mov         dword ptr [esp+28h],eax
    call        esi
    mov         cl,byte ptr [eax]
    add         esp,0Ch
    test        cl,cl
    je L_65415
    mov         ebx,dword ptr [esp+1Ch]
    lea         ebx,[ebx+00000000h]
L_653f0:
    mov         ecx,dword ptr [esp+18h]
    inc         ecx
    lea         eax,[esp+20h]
    push        eax
    mov         dword ptr [esp+1Ch],ecx
    add         ebx,10h
    lea         ecx,[esp+1Ah]
    push        ebx
    push        ecx
    call        esi
    mov         cl,byte ptr [eax]
    add         esp,0Ch
    test        cl,cl
    jne L_653f0
    mov         ebx,dword ptr [ebp+8]
L_65415:
    mov         edx,edi
    shl         edx,4
    lea         eax,[edx+ebx]
    push        eax
    mov         dword ptr [esp+20h],eax
    lea         eax,[esp+24h]
    push        eax
    lea         ecx,[esp+1Fh]
    push        ecx
    call        esi
    mov         cl,byte ptr [eax]
    add         esp,0Ch
    test        cl,cl
    je L_6545d
    mov         ebx,dword ptr [esp+1Ch]
    jmp L_65440
    lea         ecx,[ecx]
L_65440:
    sub         ebx,10h
    push        ebx
    lea         edx,[esp+24h]
    push        edx
    lea         eax,[esp+1Fh]
    push        eax
    dec         edi
    call        esi
    mov         cl,byte ptr [eax]
    add         esp,0Ch
    test        cl,cl
    jne L_65440
    mov         ebx,dword ptr [ebp+8]
L_6545d:
    mov         ecx,dword ptr [esp+18h]
    cmp         edi,ecx
    jl L_654c6
    je L_654b8
    mov         edx,edi
    shl         edx,4
    fld         dword ptr [edx+ebx]
    lea         eax,[edx+ebx]
    mov         edx,dword ptr [eax+8]
    fld         dword ptr [eax+4]
    mov         dword ptr [esp+38h],edx
    fxch        st(1)
    mov         edx,dword ptr [eax+0Ch]
    mov         dword ptr [esp+3Ch],edx
    shl         ecx,4
    mov         edx,dword ptr [ecx+ebx]
    add         ecx,ebx
    mov         dword ptr [eax],edx
    mov         edx,dword ptr [ecx+4]
    mov         dword ptr [eax+4],edx
    mov         edx,dword ptr [ecx+8]
    mov         dword ptr [eax+8],edx
    mov         edx,dword ptr [ecx+0Ch]
    mov         dword ptr [eax+0Ch],edx
    fstp        dword ptr [ecx]
    mov         eax,dword ptr [esp+38h]
    mov         edx,dword ptr [esp+3Ch]
    fstp        dword ptr [ecx+4]
    mov         dword ptr [ecx+8],eax
    mov         dword ptr [ecx+0Ch],edx
    mov         ecx,dword ptr [esp+18h]
L_654b8:
    dec         edi
    inc         ecx
    cmp         ecx,edi
    mov         dword ptr [esp+18h],ecx
    jle L_653c3
L_654c6:
    mov         eax,dword ptr [ebp+0Ch]
    cmp         eax,edi
    jge L_654dd
    push        esi
    push        edi
    push        eax
    push        ebx
    call SortCore
    mov         ecx,dword ptr [esp+28h]
    add         esp,10h
L_654dd:
    cmp         ecx,dword ptr [ebp+10h]
    jge L_654ea
    mov         dword ptr [ebp+0Ch],ecx
    jmp L_65390
L_654ea:
    pop         edi
    pop         esi
    pop         ebx
    mov         esp,ebp
    pop         ebp
    ret
  }
}

static __declspec(naked) void AabbCore(){
  __asm {
    mov         edx,dword ptr [esp+4]
    mov         eax,dword ptr [edx+4]
    test        eax,eax
    jle L_658cc
    mov         eax,dword ptr [edx]
    mov         ecx,dword ptr [esp+8]
    push        esi
    mov         esi,dword ptr [eax]
    mov         dword ptr [ecx],esi
    mov         esi,dword ptr [eax+4]
    mov         dword ptr [ecx+4],esi
    mov         esi,dword ptr [eax+8]
    mov         dword ptr [ecx+8],esi
    mov         eax,dword ptr [eax+0Ch]
    mov         dword ptr [ecx+0Ch],eax
    mov         eax,dword ptr [edx]
    mov         esi,dword ptr [eax]
    mov         dword ptr [ecx+10h],esi
    mov         esi,dword ptr [eax+4]
    mov         dword ptr [ecx+14h],esi
    mov         esi,dword ptr [eax+8]
    mov         dword ptr [ecx+18h],esi
    mov         eax,dword ptr [eax+0Ch]
    mov         dword ptr [ecx+1Ch],eax
    mov         eax,dword ptr [edx+4]
    push        edi
    xor         edi,edi
    test        eax,eax
    jle L_658ca
    xor         esi,esi
L_65805:
    mov         eax,dword ptr [edx]
    fld         dword ptr [esi+eax]
    fld         dword ptr [ecx]
    fst         dword ptr [esp+0Ch]
    fcomp       st(1)
    fnstsw      ax
    test        ah,5
    jp L_6581f
    fstp        st(0)
    fld         dword ptr [esp+0Ch]
L_6581f:
    fstp        dword ptr [ecx]
    mov         eax,dword ptr [edx]
    fld         dword ptr [esi+eax]
    fld         dword ptr [ecx+10h]
    fst         dword ptr [esp+0Ch]
    fcomp       st(1)
    fnstsw      ax
    test        ah,41h
    jne L_6583c
    fstp        st(0)
    fld         dword ptr [esp+0Ch]
L_6583c:
    fstp        dword ptr [ecx+10h]
    mov         eax,dword ptr [edx]
    fld         dword ptr [esi+eax+4]
    fld         dword ptr [ecx+4]
    fst         dword ptr [esp+0Ch]
    fcomp       st(1)
    fnstsw      ax
    test        ah,5
    jp L_6585b
    fstp        st(0)
    fld         dword ptr [esp+0Ch]
L_6585b:
    fstp        dword ptr [ecx+4]
    mov         eax,dword ptr [edx]
    fld         dword ptr [esi+eax+4]
    fld         dword ptr [ecx+14h]
    fst         dword ptr [esp+0Ch]
    fcomp       st(1)
    fnstsw      ax
    test        ah,41h
    jne L_6587a
    fstp        st(0)
    fld         dword ptr [esp+0Ch]
L_6587a:
    fstp        dword ptr [ecx+14h]
    mov         eax,dword ptr [edx]
    fld         dword ptr [esi+eax+8]
    fld         dword ptr [ecx+8]
    fst         dword ptr [esp+0Ch]
    fcomp       st(1)
    fnstsw      ax
    test        ah,5
    jp L_65899
    fstp        st(0)
    fld         dword ptr [esp+0Ch]
L_65899:
    fstp        dword ptr [ecx+8]
    mov         eax,dword ptr [edx]
    fld         dword ptr [esi+eax+8]
    fld         dword ptr [ecx+18h]
    fst         dword ptr [esp+0Ch]
    fcomp       st(1)
    fnstsw      ax
    test        ah,41h
    jne L_658b8
    fstp        st(0)
    fld         dword ptr [esp+0Ch]
L_658b8:
    fstp        dword ptr [ecx+18h]
    mov         eax,dword ptr [edx+4]
    inc         edi
    add         esi,10h
    cmp         edi,eax
    jl L_65805
L_658ca:
    pop         edi
    pop         esi
L_658cc:
    ret
  }
}

static __declspec(naked) void NormalizeCore(){
  __asm {
    push        ebp
    mov         ebp,esp
    and         esp,0FFFFFFF0h
    sub         esp,84h
    push        ebx
    push        esi
    push        edi
    mov         edi,dword ptr [ebp+8]
    lea         eax,[esp+10h]
    push        eax
    push        edi
    call AabbCore
    fld         dword ptr [esp+18h]
    fadd        dword ptr [esp+28h]
    mov         esi,dword ptr [ebp+10h]
    mov         ecx,dword ptr [ebp+0Ch]
    add         esp,8
    fstp        dword ptr [esi]
    fld         dword ptr [esp+14h]
    fadd        dword ptr [esp+24h]
    fstp        dword ptr [esi+4]
    fld         dword ptr [esp+18h]
    fadd        dword ptr [esp+28h]
    fstp        dword ptr [esi+8]
    fld         dword ptr [esp+1Ch]
    fadd        dword ptr [esp+2Ch]
    fstp        dword ptr [esi+0Ch]
    fld         dword ptr [esi]
    fmul        dword ptr [K_88f74]
    fstp        dword ptr [esi]
    fld         dword ptr [esi+4]
    fmul        dword ptr [K_88f74]
    fstp        dword ptr [esi+4]
    fld         dword ptr [esi+8]
    fmul        dword ptr [K_88f74]
    fstp        dword ptr [esi+8]
    fld         dword ptr [esi+0Ch]
    fmul        dword ptr [K_88f74]
    fstp        dword ptr [esi+0Ch]
    fld         dword ptr [esp+20h]
    fsub        dword ptr [esp+10h]
    fstp        dword ptr [ecx]
    fld         dword ptr [esp+24h]
    fsub        dword ptr [esp+14h]
    fstp        dword ptr [ecx+4]
    fld         dword ptr [esp+28h]
    fsub        dword ptr [esp+18h]
    fstp        dword ptr [ecx+8]
    fld         dword ptr [esp+2Ch]
    fsub        dword ptr [esp+1Ch]
    fstp        dword ptr [ecx+0Ch]
    fld         dword ptr [ecx+8]
    fcomp       dword ptr [K_9147c]
    fld         dword ptr [K_88e5c]
    fnstsw      ax
    test        ah,41h
    jne L_65993
    fdiv        dword ptr [ecx+8]
L_65993:
    fld         dword ptr [ecx+4]
    fcomp       dword ptr [K_9147c]
    fld         dword ptr [K_88e5c]
    fnstsw      ax
    test        ah,41h
    jne L_659ac
    fdiv        dword ptr [ecx+4]
L_659ac:
    fld         dword ptr [ecx]
    fcomp       dword ptr [K_9147c]
    fld         dword ptr [K_88e5c]
    fnstsw      ax
    test        ah,41h
    jne L_659c3
    fdiv        dword ptr [ecx]
L_659c3:
    mov         eax,dword ptr [edi+4]
    fstp        dword ptr [esp+60h]
    xor         ebx,ebx
    cmp         eax,ebx
    fstp        dword ptr [esp+74h]
    mov         dword ptr [esp+6Ch],0
    fstp        dword ptr [esp+00000088h]
    mov         dword ptr [esp+68h],0
    mov         dword ptr [esp+64h],0
    mov         dword ptr [esp+7Ch],0
    mov         dword ptr [esp+78h],0
    mov         dword ptr [esp+70h],0
    mov         dword ptr [esp+0000008Ch],0
    mov         dword ptr [esp+00000084h],0
    mov         dword ptr [esp+00000080h],0
    mov         dword ptr [esp+0Ch],ebx
    jle L_65aea
L_65a34:
    mov         eax,dword ptr [edi]
    fld         dword ptr [eax+ebx]
    mov         ecx,dword ptr [K_9f420]
    fsub        dword ptr [esi]
    add         eax,ebx
    fld         dword ptr [eax+4]
    mov         edx,ecx
    fsub        dword ptr [esi+4]
    fld         dword ptr [eax+8]
    mov         eax,dword ptr [K_9f424]
    fsub        dword ptr [esi+8]
    mov         dword ptr [esp+34h],ecx
    mov         ecx,eax
    mov         dword ptr [esp+38h],edx
    fstp        dword ptr [esp+18h]
    mov         edx,dword ptr [esp+18h]
    mov         dword ptr [esp+44h],eax
    fxch        st(1)
    mov         eax,dword ptr [K_9f428]
    fstp        dword ptr [esp+30h]
    mov         dword ptr [esp+48h],ecx
    mov         ecx,eax
    fstp        dword ptr [esp+40h]
    mov         dword ptr [esp+50h],edx
    lea         edx,[esp+60h]
    mov         dword ptr [esp+58h],ecx
    push        edx
    lea         ecx,[esp+34h]
    mov         dword ptr [esp+40h],0
    mov         dword ptr [esp+50h],0
    mov         dword ptr [esp+58h],eax
    mov         dword ptr [esp+60h],0
    call MatrixProductInPlaceCore
    fld         dword ptr [esp+50h]
    mov         eax,dword ptr [edi]
    fld         dword ptr [esp+40h]
    mov         ecx,dword ptr [esp+30h]
    fstp        dword ptr [eax+ebx+4]
    add         eax,ebx
    fstp        dword ptr [eax+8]
    mov         dword ptr [eax],ecx
    mov         dword ptr [eax+0Ch],0
    mov         eax,dword ptr [esp+0Ch]
    mov         ecx,dword ptr [edi+4]
    inc         eax
    add         ebx,10h
    cmp         eax,ecx
    mov         dword ptr [esp+0Ch],eax
    jl L_65a34
L_65aea:
    pop         edi
    pop         esi
    pop         ebx
    mov         esp,ebp
    pop         ebp
    ret
  }
}

static __declspec(naked) void UnnormalizeCore(){
  __asm {
    push        ebp
    mov         ebp,esp
    and         esp,0FFFFFFF0h
    sub         esp,64h
    mov         eax,dword ptr [ebp+0Ch]
    mov         ecx,dword ptr [eax]
    mov         edx,dword ptr [eax+4]
    mov         eax,dword ptr [eax+8]
    push        ebx
    push        esi
    push        edi
    mov         edi,dword ptr [ebp+8]
    mov         dword ptr [esp+68h],eax
    mov         eax,dword ptr [edi+4]
    xor         esi,esi
    cmp         eax,esi
    mov         dword ptr [esp+4Ch],0
    mov         dword ptr [esp+48h],0
    mov         dword ptr [esp+44h],0
    mov         dword ptr [esp+5Ch],0
    mov         dword ptr [esp+58h],0
    mov         dword ptr [esp+50h],0
    mov         dword ptr [esp+6Ch],0
    mov         dword ptr [esp+64h],0
    mov         dword ptr [esp+60h],0
    mov         dword ptr [esp+40h],ecx
    mov         dword ptr [esp+54h],edx
    mov         dword ptr [esp+0Ch],esi
    jle L_65c51
    mov         ebx,dword ptr [ebp+10h]
L_65b82:
    mov         eax,dword ptr [edi]
    mov         ecx,dword ptr [eax+esi]
    mov         edx,dword ptr [K_9f420]
    add         eax,esi
    mov         dword ptr [esp+10h],ecx
    mov         ecx,edx
    mov         dword ptr [esp+18h],ecx
    mov         ecx,dword ptr [K_9f424]
    mov         dword ptr [esp+14h],edx
    mov         dword ptr [esp+1Ch],0
    mov         edx,dword ptr [eax+4]
    mov         dword ptr [esp+20h],edx
    mov         edx,ecx
    mov         dword ptr [esp+24h],ecx
    mov         ecx,dword ptr [K_9f428]
    mov         dword ptr [esp+28h],edx
    mov         dword ptr [esp+2Ch],0
    mov         eax,dword ptr [eax+8]
    mov         dword ptr [esp+30h],eax
    mov         edx,ecx
    lea         eax,[esp+40h]
    mov         dword ptr [esp+34h],ecx
    push        eax
    lea         ecx,[esp+14h]
    mov         dword ptr [esp+3Ch],edx
    mov         dword ptr [esp+40h],0
    call MatrixProductInPlaceCore
    fld         dword ptr [esp+30h]
    mov         eax,dword ptr [edi]
    fld         dword ptr [esp+20h]
    mov         ecx,dword ptr [esp+10h]
    fstp        dword ptr [eax+esi+4]
    mov         dword ptr [eax+esi],ecx
    fstp        dword ptr [eax+esi+8]
    mov         dword ptr [eax+esi+0Ch],0
    add         eax,esi
    mov         eax,dword ptr [edi]
    fld         dword ptr [eax+esi]
    add         eax,esi
    fadd        dword ptr [ebx]
    add         esi,10h
    fstp        dword ptr [eax]
    fld         dword ptr [ebx+4]
    fadd        dword ptr [eax+4]
    fstp        dword ptr [eax+4]
    fld         dword ptr [ebx+8]
    fadd        dword ptr [eax+8]
    fstp        dword ptr [eax+8]
    fld         dword ptr [ebx+0Ch]
    fadd        dword ptr [eax+0Ch]
    fstp        dword ptr [eax+0Ch]
    mov         eax,dword ptr [esp+0Ch]
    mov         ecx,dword ptr [edi+4]
    inc         eax
    cmp         eax,ecx
    mov         dword ptr [esp+0Ch],eax
    jl L_65b82
L_65c51:
    pop         edi
    pop         esi
    pop         ebx
    mov         esp,ebp
    pop         ebp
    ret
  }
}

static __declspec(naked) void DeduplicateCore(){
  __asm {
    push        ebp
    mov         ebp,esp
    and         esp,0FFFFFFF0h
    sub         esp,14h
    push        ebx
    mov         ebx,dword ptr [ebp+0Ch]
    mov         ecx,dword ptr [ebx]
    push        esi
    mov         esi,dword ptr [ebx+4]
    dec         esi
    push        edi
    mov         edi,ecx
    mov         dword ptr [esp+18h],edi
    js L_66a02
    lea         edx,[edi-10h]
    mov         dword ptr [esp+14h],edx
    jmp L_66830
L_6682a:
    mov         edx,dword ptr [esp+14h]
    mov         edi,edi
L_66830:
    cmp         edx,dword ptr [ebx]
    jb L_66886
    fld         dword ptr [ecx]
    fsub        dword ptr [K_89268]
    fstp        dword ptr [esp+1Ch]
L_66840:
    fld         dword ptr [edx]
    fcomp       dword ptr [esp+1Ch]
    fnstsw      ax
    test        ah,5
    jnp L_66886
    fld         dword ptr [edx]
    fsub        dword ptr [ecx]
    fld         dword ptr [edx+4]
    fsub        dword ptr [ecx+4]
    fld         dword ptr [edx+8]
    fsub        dword ptr [ecx+8]
    fld         st(0)
    fmul        st,st(1)
    fld         st(2)
    fmul        st,st(3)
    faddp       st(1),st
    fld         st(3)
    fmul        st,st(4)
    faddp       st(1),st
    fcomp       dword ptr [ebp+8]
    fstp        st(0)
    fnstsw      ax
    fstp        st(0)
    test        ah,5
    fstp        st(0)
    jnp L_668b5
    mov         eax,dword ptr [ebx]
    sub         edx,10h
    cmp         edx,eax
    jae L_66840
L_66886:
    mov         edx,dword ptr [esp+14h]
    mov         eax,edi
    add         edi,10h
    add         edx,10h
    mov         dword ptr [esp+14h],edx
    mov         edx,dword ptr [ecx]
    mov         dword ptr [eax],edx
    mov         edx,dword ptr [ecx+4]
    mov         dword ptr [eax+4],edx
    mov         edx,dword ptr [ecx+8]
    mov         dword ptr [eax+8],edx
    mov         edx,dword ptr [ecx+0Ch]
    mov         dword ptr [esp+18h],edi
    mov         dword ptr [eax+0Ch],edx
    jmp L_669f8
L_668b5:
    lea         ebx,[esi-1]
    lea         eax,[ebx+1]
    cmp         eax,4
    jl L_669b0
L_668c4:
    fld         dword ptr [edx]
    fsub        dword ptr [ecx+10h]
    fld         dword ptr [edx+4]
    fsub        dword ptr [ecx+14h]
    fld         dword ptr [edx+8]
    fsub        dword ptr [ecx+18h]
    fld         st(0)
    fmul        st,st(1)
    fld         st(2)
    fmul        st,st(3)
    faddp       st(1),st
    fld         st(3)
    fmul        st,st(4)
    faddp       st(1),st
    fcomp       dword ptr [ebp+8]
    fstp        st(0)
    fnstsw      ax
    fstp        st(0)
    test        ah,5
    fstp        st(0)
    jp L_669f5
    fld         dword ptr [edx]
    add         ecx,10h
    fsub        dword ptr [ecx+10h]
    dec         esi
    fld         dword ptr [edx+4]
    fsub        dword ptr [ecx+14h]
    fld         dword ptr [edx+8]
    fsub        dword ptr [ecx+18h]
    fld         st(0)
    fmul        st,st(1)
    fld         st(2)
    fmul        st,st(3)
    faddp       st(1),st
    fld         st(3)
    fmul        st,st(4)
    faddp       st(1),st
    fcomp       dword ptr [ebp+8]
    fstp        st(0)
    fnstsw      ax
    fstp        st(0)
    test        ah,5
    fstp        st(0)
    jp L_669f5
    fld         dword ptr [edx]
    add         ecx,10h
    fsub        dword ptr [ecx+10h]
    dec         esi
    fld         dword ptr [edx+4]
    fsub        dword ptr [ecx+14h]
    fld         dword ptr [edx+8]
    fsub        dword ptr [ecx+18h]
    fld         st(0)
    fmul        st,st(1)
    fld         st(2)
    fmul        st,st(3)
    faddp       st(1),st
    fld         st(3)
    fmul        st,st(4)
    faddp       st(1),st
    fcomp       dword ptr [ebp+8]
    fstp        st(0)
    fnstsw      ax
    fstp        st(0)
    test        ah,5
    fstp        st(0)
    jp L_669f5
    fld         dword ptr [edx]
    add         ecx,10h
    fsub        dword ptr [ecx+10h]
    dec         esi
    fld         dword ptr [edx+4]
    fsub        dword ptr [ecx+14h]
    fld         dword ptr [edx+8]
    fsub        dword ptr [ecx+18h]
    fld         st(0)
    fmul        st,st(1)
    fld         st(2)
    fmul        st,st(3)
    faddp       st(1),st
    fld         st(3)
    fmul        st,st(4)
    faddp       st(1),st
    fcomp       dword ptr [ebp+8]
    fstp        st(0)
    fnstsw      ax
    fstp        st(0)
    test        ah,5
    fstp        st(0)
    jp L_669f5
    add         ecx,10h
    sub         ebx,4
    dec         esi
    cmp         ebx,3
    jge L_668c4
L_669b0:
    test        ebx,ebx
    jl L_669f5
    lea         edi,[ecx+18h]
L_669b7:
    fld         dword ptr [edx]
    fsub        dword ptr [edi-8]
    fld         dword ptr [edx+4]
    fsub        dword ptr [edi-4]
    fld         dword ptr [edx+8]
    fsub        dword ptr [edi]
    fld         st(0)
    fmul        st,st(1)
    fld         st(2)
    fmul        st,st(3)
    faddp       st(1),st
    fld         st(3)
    fmul        st,st(4)
    faddp       st(1),st
    fcomp       dword ptr [ebp+8]
    fstp        st(0)
    fnstsw      ax
    fstp        st(0)
    test        ah,5
    fstp        st(0)
    jp L_669f1
    add         ecx,10h
    add         edi,10h
    dec         esi
    dec         ebx
    jns L_669b7
L_669f1:
    mov         edi,dword ptr [esp+18h]
L_669f5:
    mov         ebx,dword ptr [ebp+0Ch]
L_669f8:
    add         ecx,10h
    dec         esi
    jns L_6682a
L_66a02:
    sub         edi,dword ptr [ebx]
    mov         eax,dword ptr [ebp+10h]
    mov         esi,edi
    sar         esi,4
    mov         dword ptr [eax],esi
    mov         eax,dword ptr [ebx+8]
    and         eax,7FFFFFFFh
    cmp         eax,esi
    jge L_66a2e
    add         eax,eax
    cmp         esi,eax
    jl L_66a22
    mov         eax,esi
L_66a22:
    push        10h
    push        eax
    push        ebx
    call CapacityInvariantFailure
    add         esp,0Ch
L_66a2e:
    pop         edi
    mov         dword ptr [ebx+4],esi
    pop         esi
    pop         ebx
    mov         esp,ebp
    pop         ebp
    ret
  }
}

static __declspec(naked) void MatrixProductCore(){
  __asm {
    mov         eax,dword ptr [esp+4]
    mov         edx,dword ptr [esp+8]
    fld         dword ptr [edx]
    fld         dword ptr [edx+4]
    fld         dword ptr [edx+8]
    fld         st(2)
    fmul        dword ptr [eax]
    fld         st(1)
    fmul        dword ptr [eax+20h]
    faddp       st(1),st
    fld         st(2)
    fmul        dword ptr [eax+10h]
    faddp       st(1),st
    fstp        dword ptr [ecx]
    fld         st(0)
    fmul        dword ptr [eax+24h]
    fld         st(2)
    fmul        dword ptr [eax+14h]
    faddp       st(1),st
    fld         st(3)
    fmul        dword ptr [eax+4]
    faddp       st(1),st
    fstp        dword ptr [ecx+4]
    fmul        dword ptr [eax+28h]
    fxch        st(1)
    fmul        dword ptr [eax+18h]
    faddp       st(1),st
    fxch        st(1)
    fmul        dword ptr [eax+8]
    faddp       st(1),st
    fstp        dword ptr [ecx+8]
    mov         dword ptr [ecx+0Ch],0
    fld         dword ptr [edx+10h]
    fld         dword ptr [edx+14h]
    fld         dword ptr [edx+18h]
    fld         st(2)
    fmul        dword ptr [eax]
    fld         st(1)
    fmul        dword ptr [eax+20h]
    faddp       st(1),st
    fld         st(2)
    fmul        dword ptr [eax+10h]
    faddp       st(1),st
    fstp        dword ptr [ecx+10h]
    fld         st(0)
    fmul        dword ptr [eax+24h]
    fld         st(2)
    fmul        dword ptr [eax+14h]
    faddp       st(1),st
    fld         st(3)
    fmul        dword ptr [eax+4]
    faddp       st(1),st
    fstp        dword ptr [ecx+14h]
    fmul        dword ptr [eax+28h]
    fxch        st(1)
    fmul        dword ptr [eax+18h]
    faddp       st(1),st
    fxch        st(1)
    fmul        dword ptr [eax+8]
    faddp       st(1),st
    fstp        dword ptr [ecx+18h]
    mov         dword ptr [ecx+1Ch],0
    fld         dword ptr [edx+20h]
    fld         dword ptr [edx+24h]
    fld         dword ptr [edx+28h]
    fld         st(2)
    fmul        dword ptr [eax]
    fld         st(1)
    fmul        dword ptr [eax+20h]
    faddp       st(1),st
    fld         st(2)
    fmul        dword ptr [eax+10h]
    faddp       st(1),st
    fstp        dword ptr [ecx+20h]
    fld         st(0)
    fmul        dword ptr [eax+24h]
    fld         st(2)
    fmul        dword ptr [eax+14h]
    faddp       st(1),st
    fld         st(3)
    fmul        dword ptr [eax+4]
    faddp       st(1),st
    fstp        dword ptr [ecx+24h]
    fmul        dword ptr [eax+28h]
    fxch        st(1)
    fmul        dword ptr [eax+18h]
    faddp       st(1),st
    fxch        st(1)
    fmul        dword ptr [eax+8]
    faddp       st(1),st
    fstp        dword ptr [ecx+28h]
    mov         dword ptr [ecx+2Ch],0
    ret         8
  }
}

static __declspec(naked) void MatrixProductInPlaceCore(){
  __asm {
    push        ebp
    mov         ebp,esp
    and         esp,0FFFFFFF0h
    sub         esp,3Ch
    mov         eax,dword ptr [ebp+8]
    push        esi
    mov         esi,ecx
    push        eax
    push        esi
    lea         ecx,[esp+18h]
    call MatrixProductCore
    mov         ecx,dword ptr [esp+10h]
    mov         edx,dword ptr [esp+14h]
    mov         eax,dword ptr [esp+18h]
    mov         dword ptr [esi],ecx
    mov         ecx,dword ptr [esp+1Ch]
    mov         dword ptr [esi+4],edx
    mov         edx,dword ptr [esp+20h]
    mov         dword ptr [esi+8],eax
    mov         eax,dword ptr [esp+24h]
    mov         dword ptr [esi+0Ch],ecx
    mov         ecx,dword ptr [esp+28h]
    mov         dword ptr [esi+10h],edx
    mov         edx,dword ptr [esp+2Ch]
    mov         dword ptr [esi+14h],eax
    mov         eax,dword ptr [esp+30h]
    mov         dword ptr [esi+18h],ecx
    mov         ecx,dword ptr [esp+34h]
    mov         dword ptr [esi+1Ch],edx
    mov         edx,dword ptr [esp+38h]
    mov         dword ptr [esi+20h],eax
    mov         eax,dword ptr [esp+3Ch]
    mov         dword ptr [esi+24h],ecx
    mov         dword ptr [esi+28h],edx
    mov         dword ptr [esi+2Ch],eax
    pop         esi
    mov         esp,ebp
    pop         ebp
    ret         4
  }
}
}
// Each operation mutates caller-owned points; no allocator or mutable globals.
inline Status Normalize(Array* a,Vec4* span,Vec4* center){
 if(!a||!span||!center||!Valid(*a))return InvalidInput;
 typedef void(__cdecl* Proc)(Array*,Vec4*,Vec4*);
 reinterpret_cast<Proc>(&detail::NormalizeCore)(a,span,center);
 return Valid(*a)&&Finite(span->x)&&Finite(span->y)&&Finite(span->z)&&Finite(span->w)&&Finite(center->x)&&Finite(center->y)&&Finite(center->z)&&Finite(center->w)?Success:NonfiniteResult;
}
inline Status Sort(Array* a){
 if(!a||!Valid(*a))return InvalidInput;
 if(a->size>1){typedef void(__cdecl* Proc)(Vec4*,int,int,void(*)());reinterpret_cast<Proc>(&detail::SortCore)(a->data,0,a->size-1,&detail::LessCore);}
 return Success;
}
inline Status Deduplicate(Array* a,float squaredDistance,int* retainedCount){
 if(!a||!retainedCount||!Valid(*a)||!Finite(squaredDistance)||squaredDistance<0)return InvalidInput;
 typedef void(__cdecl* Proc)(float,Array*,int*);
 reinterpret_cast<Proc>(&detail::DeduplicateCore)(squaredDistance,a,retainedCount);
 return Valid(*a)&&*retainedCount==a->size?Success:NonfiniteResult;
}
inline Status Unnormalize(Array* a,const Vec4& span,const Vec4& center){
 if(!a||!Valid(*a)||!Finite(span.x)||!Finite(span.y)||!Finite(span.z)||!Finite(span.w)||!Finite(center.x)||!Finite(center.y)||!Finite(center.z)||!Finite(center.w))return InvalidInput;
 typedef void(__cdecl* Proc)(Array*,const Vec4*,const Vec4*);
 reinterpret_cast<Proc>(&detail::UnnormalizeCore)(a,&span,&center);
 return Valid(*a)?Success:NonfiniteResult;
}
#else
#error SourcePhysicsHullPreprocess requires the recovered MSVC x86 arithmetic profile.
#endif
}
