#pragma once
// Reconstructed scalar convex graph, merge, retry, validation and planes.
// Source instructions and owned constants only; no original runtime dependency.
// See CONVEX_GRAPH_MERGE_CONTRACT.md and the recovery manifest.
namespace NativeHullConvex { namespace detail {
static const unsigned int K_888a8[2]={0x00000000u,0x73757242u};
static const unsigned int K_88e5c[2]={0x3f800000u,0x566e5f6du};
static const unsigned int K_88eb8[2]={0x3a83126fu,0x61657243u};
static const unsigned int K_88f74[2]={0x3f000000u,0x43fa0000u};
static const unsigned int K_89268[2]={0x3c23d70au,0x65646973u};
static const unsigned int K_89ce8[2]={0x40000000u,0x6c726f77u};
static const unsigned int K_8a1bc[2]={0x38d1b717u,0x3c23d70bu};
static const unsigned int K_90fd8[2]={0x40800000u,0x416f4d43u};
static const unsigned int K_91444[2]={0x41000000u,0x3eaaaaabu};
static const unsigned int K_91474[2]={0x358637bdu,0x40490fdbu};
static const unsigned int K_9147c[2]={0x34000000u,0x7f7fffffu};
static const unsigned int K_91484[2]={0x33d6bf95u,0xbf7fffefu};
static const unsigned int K_91488[2]={0xbf7fffefu,0xb58637bdu};
static const unsigned int K_9148c[2]={0xb58637bdu,0x7effffffu};
static const unsigned int K_91490[2]={0x7effffffu,0x3f8ccccdu};
static const unsigned int K_91494[2]={0x3f8ccccdu,0x40490fdbu};
static const unsigned int K_9f420[2]={0x00000000u,0x00000000u};
static const unsigned int K_9f424[2]={0x00000000u,0x00000000u};
static const unsigned int K_9f428[2]={0x00000000u,0x00000000u};
static __declspec(naked) void CoreBlock(){
  __asm {
    jmp L_64f70
L_64e50:
    sub         esp,10h
    push        edi
    mov         edi,dword ptr [esp+18h]
    mov         eax,dword ptr [edi+8]
    cmp         eax,10h
    jne L_64e83
    mov         ecx,dword ptr [edi]
    test        cl,0Fh
    jne L_64e83
    mov         eax,dword ptr [esp+20h]
    mov         edx,dword ptr [esp+1Ch]
    push        eax
    mov         eax,dword ptr [edi+4]
    push        edx
    push        eax
    push        ecx
    call L_6a620
    add         esp,10h
    pop         edi
    add         esp,10h
    ret
L_64e83:
    cdq
    and         edx,3
    add         eax,edx
    mov         edx,dword ptr [edi+4]
    push        esi
    xor         esi,esi
    mov         ecx,80000000h
    sar         eax,2
    cmp         edx,esi
    mov         dword ptr [esp+0Ch],esi
    mov         dword ptr [esp+10h],esi
    mov         dword ptr [esp+14h],ecx
    mov         dword ptr [esp+1Ch],esi
    jle L_64f29
    push        ebx
    lea         edx,[eax*4+00000000h]
    push        ebp
    xor         ebx,ebx
    mov         dword ptr [esp+10h],edx
    jmp L_64ec4
L_64ebc:
    mov         ecx,dword ptr [esp+1Ch]
    mov         esi,dword ptr [esp+18h]
L_64ec4:
    lea         ebp,[esi+1]
    and         ecx,7FFFFFFFh
    cmp         ecx,ebp
    jge L_64eea
    lea         eax,[ecx+ecx]
    cmp         ebp,eax
    jl L_64eda
    mov         eax,ebp
L_64eda:
    push        10h
    push        eax
    lea         eax,[esp+1Ch]
    push        eax
    call L_73590
    add         esp,0Ch
L_64eea:
    mov         ecx,dword ptr [edi]
    mov         dword ptr [esp+18h],ebp
    mov         edx,dword ptr [ebx+ecx]
    mov         ebp,dword ptr [esp+14h]
    shl         esi,4
    mov         dword ptr [esi+ebp],edx
    mov         eax,dword ptr [edi]
    mov         ecx,dword ptr [eax+ebx+4]
    add         esi,ebp
    mov         dword ptr [esi+4],ecx
    mov         edx,dword ptr [edi]
    mov         eax,dword ptr [edx+ebx+8]
    mov         edx,dword ptr [esp+10h]
    mov         dword ptr [esi+8],eax
    mov         eax,dword ptr [esp+24h]
    mov         ecx,dword ptr [edi+4]
    inc         eax
    add         ebx,edx
    cmp         eax,ecx
    mov         dword ptr [esp+24h],eax
    jl L_64ebc
    pop         ebp
    pop         ebx
L_64f29:
    mov         ecx,dword ptr [esp+24h]
    mov         edx,dword ptr [esp+20h]
    mov         eax,dword ptr [edi+4]
    push        ecx
    mov         ecx,dword ptr [esp+10h]
    push        edx
    push        eax
    push        ecx
    call L_6a620
    mov         eax,dword ptr [esp+24h]
    add         esp,10h
    test        eax,eax
    pop         esi
    js L_64f68
    movd ecx,xmm7
    mov         edx,dword ptr [ecx]
    and         eax,7FFFFFFFh
    push        12h
    shl         eax,4
    push        eax
    mov         eax,dword ptr [esp+10h]
    push        eax
    call        dword ptr [edx+14h]
L_64f68:
    pop         edi
    add         esp,10h
    ret
L_64f70:
    sub         esp,448h
    push        esi
    lea         ecx,[esp+3Ch]
    call L_6ad00
    mov         esi,dword ptr [esp+00000454h]
    mov         ecx,dword ptr [esp+00000450h]
    push        esi
    lea         eax,[esp+40h]
    push        eax
    push        ecx
    call L_64e50
    lea         edx,[esp+48h]
    push        esi
    push        edx
    call L_68410
    mov         eax,dword ptr [esp+0000046Ch]
    push        eax
    push        esi
    lea         ecx,[esp+58h]
    push        ecx
    lea         edx,[esp+28h]
    push        edx
    lea         eax,[esp+2Bh]
    push        eax
    mov         byte ptr [esp+30h],1
    mov         dword ptr [esp+34h],37A7C5ACh
    mov         dword ptr [esp+38h],368637BDh
    mov         dword ptr [esp+3Ch],358637BDh
    mov         dword ptr [esp+40h],3727C5ACh
    mov         dword ptr [esp+44h],3D4CCCCDh
    mov         dword ptr [esp+48h],358637BDh
    mov         dword ptr [esp+4Ch],358637BDh
    mov         dword ptr [esp+50h],322BCC77h
    mov         dword ptr [esp+54h],358637BDh
    mov         dword ptr [esp+58h],38D1B717h
    mov         dword ptr [esp+5Ch],3727C5ACh
    mov         dword ptr [esp+60h],37A7C5ACh
    call L_684d0
    mov         eax,dword ptr [esp+70h]
    add         esp,28h
    test        eax,eax
    pop         esi
    js L_6504e
    movd ecx,xmm7
    mov         edx,dword ptr [ecx]
    and         eax,7FFFFFFFh
    push        12h
    shl         eax,3
    push        eax
    mov         eax,dword ptr [esp+44h]
    push        eax
    call        dword ptr [edx+14h]
L_6504e:
    add         esp,448h
    ret
L_65060:
    mov         eax,dword ptr [esp+8]
    push        ebx
    push        ebp
    mov         ebp,dword ptr [esp+18h]
    push        esi
    push        edi
    mov         edi,dword ptr [esp+1Ch]
    cmp         edi,ebp
    je L_65094
    mov         cx,word ptr [edi]
    cmp         cx,word ptr [ebp]
    jne L_65094
    movzx       edx,word ptr [edi+2]
    movzx       ecx,word ptr [ebp+2]
    mov         dx,word ptr [eax+edx*8]
    cmp         dx,word ptr [eax+ecx*8]
    mov         byte ptr [esp+1Ch],1
    jne L_65099
L_65094:
    mov         byte ptr [esp+1Ch],0
L_65099:
    mov         ebx,dword ptr [esp+24h]
    mov         ecx,dword ptr [esp+28h]
    cmp         ebx,ecx
    je L_650c4
    mov         dx,word ptr [ebx]
    cmp         dx,word ptr [ecx]
    jne L_650c4
    movzx       edx,word ptr [ebx+2]
    movzx       esi,word ptr [ecx+2]
    mov         dx,word ptr [eax+edx*8]
    cmp         dx,word ptr [eax+esi*8]
    mov         byte ptr [esp+28h],1
    jne L_650c9
L_650c4:
    mov         byte ptr [esp+28h],0
L_650c9:
    mov         dl,byte ptr [esp+1Ch]
    test        dl,dl
    je L_650dd
    mov         dl,byte ptr [esp+28h]
    test        dl,dl
    je L_650dd
    mov         dl,1
    jmp L_650df
L_650dd:
    xor         dl,dl
L_650df:
    test        dl,dl
    je L_65272
    cmp         edi,ecx
    je L_65110
    mov         dx,word ptr [edi]
    cmp         dx,word ptr [ecx]
    jne L_65110
    movzx       edx,word ptr [edi+2]
    mov         bx,word ptr [eax+edx*8]
    movzx       esi,word ptr [ecx+2]
    xor         edx,edx
    cmp         bx,word ptr [eax+esi*8]
    mov         ebx,dword ptr [esp+24h]
    setne       dl
    test        dl,dl
    je L_6513d
L_65110:
    cmp         ebx,ecx
    je L_6513d
    mov         si,word ptr [ecx]
    cmp         word ptr [ebx],si
    jne L_6513d
    movzx       edx,word ptr [ebx+2]
    movzx       ebx,word ptr [ecx+2]
    mov         bp,word ptr [eax+edx*8]
    xor         edx,edx
    cmp         bp,word ptr [eax+ebx*8]
    mov         ebp,dword ptr [esp+20h]
    mov         ebx,dword ptr [esp+24h]
    setne       dl
    test        dl,dl
    je L_65175
L_6513d:
    movzx       edx,word ptr [edi+2]
    mov         si,word ptr [ecx]
    cmp         word ptr [eax+edx*8],si
    lea         edx,[eax+edx*8]
    mov         dword ptr [esp+20h],edx
    jne L_65175
    movzx       edx,word ptr [ecx+2]
    mov         dx,word ptr [eax+edx*8]
    cmp         dx,word ptr [edi]
    jne L_65175
    cmp         dword ptr [esp+20h],ecx
    mov         dword ptr [esp+1Ch],0
    mov         edx,dword ptr [esp+1Ch]
    sete        dl
    test        dl,dl
    je L_651a4
L_65175:
    movzx       edx,word ptr [ebx+2]
    cmp         word ptr [eax+edx*8],si
    lea         edx,[eax+edx*8]
    mov         dword ptr [esp+20h],edx
    jne L_651a4
    movzx       edx,word ptr [ecx+2]
    mov         dx,word ptr [eax+edx*8]
    cmp         dx,word ptr [ebx]
    jne L_651a4
    mov         esi,dword ptr [esp+20h]
    xor         edx,edx
    cmp         esi,ecx
    sete        dl
    mov         dword ptr [esp+20h],edx
    jmp L_651ac
L_651a4:
    mov         dword ptr [esp+20h],1
L_651ac:
    cmp         edi,ebp
    je L_651d2
    mov         cx,word ptr [edi]
    cmp         cx,word ptr [ebp]
    jne L_651d2
    movzx       edx,word ptr [edi+2]
    movzx       esi,word ptr [ebp+2]
    mov         dx,word ptr [eax+edx*8]
    xor         ecx,ecx
    cmp         dx,word ptr [eax+esi*8]
    setne       cl
    test        cl,cl
    je L_651fc
L_651d2:
    cmp         ebx,ebp
    je L_651fc
    mov         dx,word ptr [ebp]
    cmp         word ptr [ebx],dx
    jne L_651fc
    movzx       ecx,word ptr [ebx+2]
    mov         bx,word ptr [eax+ecx*8]
    movzx       esi,word ptr [ebp+2]
    xor         ecx,ecx
    cmp         bx,word ptr [eax+esi*8]
    mov         ebx,dword ptr [esp+24h]
    setne       cl
    test        cl,cl
    je L_65224
L_651fc:
    movzx       edx,word ptr [edi+2]
    lea         esi,[eax+edx*8]
    mov         dx,word ptr [ebp]
    cmp         word ptr [esi],dx
    jne L_65224
    movzx       ecx,word ptr [ebp+2]
    mov         cx,word ptr [eax+ecx*8]
    cmp         cx,word ptr [edi]
    jne L_65224
    xor         ecx,ecx
    cmp         esi,ebp
    sete        cl
    test        cl,cl
    je L_65247
L_65224:
    movzx       ecx,word ptr [ebx+2]
    cmp         word ptr [eax+ecx*8],dx
    lea         ecx,[eax+ecx*8]
    jne L_65247
    movzx       edx,word ptr [ebp+2]
    mov         ax,word ptr [eax+edx*8]
    cmp         ax,word ptr [ebx]
    jne L_65247
    xor         eax,eax
    cmp         ecx,ebp
    sete        al
    jmp L_6524c
L_65247:
    mov         eax,1
L_6524c:
    mov         cl,byte ptr [esp+20h]
    test        cl,cl
    je L_65265
    test        al,al
    je L_65265
    mov         eax,dword ptr [esp+14h]
    pop         edi
    pop         esi
    mov         cl,1
    pop         ebp
    mov         byte ptr [eax],cl
    pop         ebx
    ret
L_65265:
    mov         eax,dword ptr [esp+14h]
    pop         edi
    pop         esi
    xor         cl,cl
    pop         ebp
    mov         byte ptr [eax],cl
    pop         ebx
    ret
L_65272:
    mov         eax,dword ptr [esp+14h]
    pop         edi
    pop         esi
    pop         ebp
    mov         byte ptr [eax],dl
    pop         ebx
    ret
L_65280:
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
L_652f0:
    mov         edx,dword ptr [esp+4]
    fld         dword ptr [edx]
    fsub        dword ptr [ecx]
    fstp        dword ptr [eax]
    fld         dword ptr [edx+4]
    fsub        dword ptr [ecx+4]
    fstp        dword ptr [eax+4]
    fld         dword ptr [edx+8]
    fsub        dword ptr [ecx+8]
    fstp        dword ptr [eax+8]
    fld         dword ptr [edx+0Ch]
    fsub        dword ptr [ecx+0Ch]
    fstp        dword ptr [eax+0Ch]
    ret
L_65320:
    mov         eax,dword ptr [esp+8]
    fld         dword ptr [eax+4]
    mov         ecx,dword ptr [esp+0Ch]
    fcomp       dword ptr [ecx+4]
    fnstsw      ax
    test        ah,5
    mov         eax,dword ptr [esp+4]
    jp L_6533e
    mov         cl,1
    mov         byte ptr [eax],cl
    ret
L_6533e:
    xor         cl,cl
    mov         byte ptr [eax],cl
    ret
L_65350:
    mov         eax,dword ptr [esp+8]
    fld         dword ptr [eax+10h]
    mov         ecx,dword ptr [esp+0Ch]
    fcomp       dword ptr [ecx+10h]
    fnstsw      ax
    test        ah,5
    mov         eax,dword ptr [esp+4]
    jp L_6536e
    mov         cl,1
    mov         byte ptr [eax],cl
    ret
L_6536e:
    xor         cl,cl
    mov         byte ptr [eax],cl
    ret
L_65380:
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
    call L_65380
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
L_65500:
    push        ebp
    mov         ebp,esp
    and         esp,0FFFFFFF0h
    sub         esp,54h
    push        ebx
    mov         ebx,dword ptr [ebp+14h]
    push        esi
    push        edi
    nop
L_65510:
    mov         esi,dword ptr [ebp+10h]
    mov         edi,dword ptr [ebp+0Ch]
    mov         ecx,dword ptr [ebp+8]
    lea         eax,[edi+esi]
    sar         eax,1
    lea         eax,[eax+eax*4]
    lea         edx,[ecx+eax*4]
    mov         eax,dword ptr [edx]
    mov         ecx,dword ptr [edx+4]
    mov         dword ptr [esp+40h],eax
    mov         eax,dword ptr [edx+8]
    mov         dword ptr [esp+44h],ecx
    mov         ecx,dword ptr [edx+0Ch]
    mov         edx,dword ptr [edx+10h]
    mov         dword ptr [esp+48h],eax
    mov         dword ptr [esp+4Ch],ecx
    mov         dword ptr [esp+50h],edx
L_65546:
    mov         ecx,dword ptr [ebp+8]
    lea         edx,[esp+40h]
    lea         eax,[edi+edi*4]
    lea         eax,[ecx+eax*4]
    push        edx
    push        eax
    mov         dword ptr [esp+24h],eax
    lea         eax,[esp+22h]
    push        eax
    call        ebx
    mov         cl,byte ptr [eax]
    add         esp,0Ch
    test        cl,cl
    je L_65591
    mov         eax,dword ptr [esp+1Ch]
    jmp L_65573
L_6556f:
    mov         eax,dword ptr [esp+1Ch]
L_65573:
    lea         ecx,[esp+40h]
    add         eax,14h
    push        ecx
    push        eax
    lea         edx,[esp+22h]
    push        edx
    inc         edi
    mov         dword ptr [esp+28h],eax
    call        ebx
    mov         cl,byte ptr [eax]
    add         esp,0Ch
    test        cl,cl
    jne L_6556f
L_65591:
    mov         ecx,dword ptr [ebp+8]
    lea         eax,[esi+esi*4]
    lea         eax,[ecx+eax*4]
    push        eax
    lea         edx,[esp+44h]
    mov         dword ptr [esp+20h],eax
    push        edx
    lea         eax,[esp+23h]
    push        eax
    call        ebx
    mov         cl,byte ptr [eax]
    add         esp,0Ch
    test        cl,cl
    je L_655de
    mov         eax,dword ptr [esp+1Ch]
    jmp L_655c0
L_655ba:
    mov         eax,dword ptr [esp+1Ch]
    mov         edi,edi
L_655c0:
    sub         eax,14h
    push        eax
    lea         ecx,[esp+44h]
    push        ecx
    lea         edx,[esp+23h]
    push        edx
    dec         esi
    mov         dword ptr [esp+28h],eax
    call        ebx
    mov         cl,byte ptr [eax]
    add         esp,0Ch
    test        cl,cl
    jne L_655ba
L_655de:
    cmp         esi,edi
    jl L_65668
    je L_6565e
    mov         ecx,dword ptr [ebp+8]
    lea         eax,[esi+esi*4]
    lea         eax,[ecx+eax*4]
    mov         edx,eax
    mov         ebx,dword ptr [edx]
    mov         dword ptr [esp+20h],ebx
    mov         ebx,dword ptr [edx+4]
    mov         dword ptr [esp+24h],ebx
    mov         ebx,dword ptr [edx+8]
    mov         dword ptr [esp+28h],ebx
    mov         ebx,dword ptr [edx+0Ch]
    mov         edx,dword ptr [edx+10h]
    mov         dword ptr [esp+30h],edx
    lea         edx,[edi+edi*4]
    lea         ecx,[ecx+edx*4]
    mov         dword ptr [esp+2Ch],ebx
    mov         edx,ecx
    mov         ebx,dword ptr [edx]
    mov         dword ptr [eax],ebx
    mov         ebx,dword ptr [edx+4]
    mov         dword ptr [eax+4],ebx
    mov         ebx,dword ptr [edx+8]
    mov         dword ptr [eax+8],ebx
    mov         ebx,dword ptr [edx+0Ch]
    mov         dword ptr [eax+0Ch],ebx
    mov         edx,dword ptr [edx+10h]
    mov         ebx,dword ptr [ebp+14h]
    mov         dword ptr [eax+10h],edx
    mov         eax,dword ptr [esp+20h]
    mov         edx,dword ptr [esp+24h]
    mov         dword ptr [ecx],eax
    mov         eax,dword ptr [esp+28h]
    mov         dword ptr [ecx+4],edx
    mov         edx,dword ptr [esp+2Ch]
    mov         dword ptr [ecx+8],eax
    mov         eax,dword ptr [esp+30h]
    mov         dword ptr [ecx+0Ch],edx
    mov         dword ptr [ecx+10h],eax
L_6565e:
    dec         esi
    inc         edi
    cmp         edi,esi
    jle L_65546
L_65668:
    mov         eax,dword ptr [ebp+0Ch]
    cmp         eax,esi
    jge L_6567e
    mov         ecx,dword ptr [ebp+8]
    push        ebx
    push        esi
    push        eax
    push        ecx
    call L_65500
    add         esp,10h
L_6567e:
    cmp         edi,dword ptr [ebp+10h]
    jge L_6568b
    mov         dword ptr [ebp+0Ch],edi
    jmp L_65510
L_6568b:
    pop         edi
    pop         esi
    pop         ebx
    mov         esp,ebp
    pop         ebp
    ret
L_656a0:
    push        ebp
    mov         ebp,esp
    and         esp,0FFFFFFF0h
    sub         esp,24h
    push        ebx
    push        esi
    push        edi
    mov         edi,dword ptr [ebp+8]
    nop
L_656b0:
    mov         esi,dword ptr [ebp+10h]
    mov         ebx,dword ptr [ebp+0Ch]
    lea         eax,[ebx+esi]
    sar         eax,1
    mov         ecx,dword ptr [edi+eax*8]
    mov         edx,dword ptr [edi+eax*8+4]
    mov         dword ptr [esp+20h],ecx
    mov         dword ptr [esp+24h],edx
    lea         ebx,[ebx+00000000h]
L_656d0:
    lea         ecx,[esp+20h]
    push        ecx
    lea         eax,[edi+ebx*8]
    push        eax
    lea         edx,[esp+22h]
    push        edx
    call        dword ptr [ebp+14h]
    mov         cl,byte ptr [eax]
    add         esp,0Ch
    test        cl,cl
    je L_65712
    lea         eax,[edi+ebx*8]
    jmp L_656f3
L_656ef:
    mov         eax,dword ptr [esp+1Ch]
L_656f3:
    lea         ecx,[esp+20h]
    add         eax,8
    push        ecx
    push        eax
    lea         edx,[esp+22h]
    push        edx
    inc         ebx
    mov         dword ptr [esp+28h],eax
    call        dword ptr [ebp+14h]
    mov         cl,byte ptr [eax]
    add         esp,0Ch
    test        cl,cl
    jne L_656ef
L_65712:
    lea         eax,[edi+esi*8]
    push        eax
    lea         eax,[esp+24h]
    push        eax
    lea         ecx,[esp+23h]
    push        ecx
    call        dword ptr [ebp+14h]
    mov         cl,byte ptr [eax]
    add         esp,0Ch
    test        cl,cl
    je L_65754
    lea         eax,[edi+esi*8]
    jmp L_65735
L_65731:
    mov         eax,dword ptr [esp+1Ch]
L_65735:
    sub         eax,8
    push        eax
    lea         edx,[esp+24h]
    mov         dword ptr [esp+20h],eax
    push        edx
    lea         eax,[esp+23h]
    push        eax
    dec         esi
    call        dword ptr [ebp+14h]
    mov         cl,byte ptr [eax]
    add         esp,0Ch
    test        cl,cl
    jne L_65731
L_65754:
    cmp         esi,ebx
    jl L_65780
    je L_65776
    mov         edx,dword ptr [edi+ebx*8]
    mov         eax,dword ptr [edi+esi*8]
    mov         ecx,dword ptr [edi+esi*8+4]
    mov         dword ptr [edi+esi*8],edx
    mov         edx,dword ptr [edi+ebx*8+4]
    mov         dword ptr [edi+esi*8+4],edx
    mov         dword ptr [edi+ebx*8],eax
    mov         dword ptr [edi+ebx*8+4],ecx
L_65776:
    dec         esi
    inc         ebx
    cmp         ebx,esi
    jle L_656d0
L_65780:
    mov         eax,dword ptr [ebp+0Ch]
    cmp         eax,esi
    jge L_65796
    mov         ecx,dword ptr [ebp+14h]
    push        ecx
    push        esi
    push        eax
    push        edi
    call L_656a0
    add         esp,10h
L_65796:
    cmp         ebx,dword ptr [ebp+10h]
    jge L_657a3
    mov         dword ptr [ebp+0Ch],ebx
    jmp L_656b0
L_657a3:
    pop         edi
    pop         esi
    pop         ebx
    mov         esp,ebp
    pop         ebp
    ret
L_657b0:
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
L_658d0:
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
    call L_657b0
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
    call L_71c10
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
L_65b00:
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
    call L_71c10
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
L_65c60:
    push        ebp
    mov         ebp,esp
    and         esp,0FFFFFFF0h
    sub         esp,54h
    mov         eax,dword ptr [ebp+10h]
    mov         edx,dword ptr [eax+8]
    cmp         edx,3
    mov         ecx,dword ptr [eax]
    push        ebx
    push        esi
    push        edi
    jge L_660b9
    mov         esi,dword ptr [ebp+14h]
    mov         edi,dword ptr [esi+8]
    cmp         edi,3
    jge L_660b9
    cmp         edx,2
    mov         dword ptr [esp+14h],0
    mov         dword ptr [esp+24h],3
    jne L_65cee
    mov         eax,dword ptr [eax+4]
    movzx       edx,word ptr [eax+2]
    movzx       edi,word ptr [eax]
    movzx       ebx,word ptr [eax+edx*8]
    cmp         ebx,edi
    jge L_65cb9
    mov         eax,edi
    mov         edi,ebx
    mov         ebx,eax
L_65cb9:
    mov         eax,dword ptr [esi+4]
    movzx       edx,word ptr [eax]
    cmp         dword ptr [esi+8],2
    mov         dword ptr [esp+10h],edx
    jne L_65d19
    mov         eax,dword ptr [esi+4]
    movzx       esi,word ptr [eax+2]
    movzx       eax,word ptr [eax+esi*8]
    cmp         eax,edx
    mov         dword ptr [esp+14h],eax
    jge L_65ce4
    mov         dword ptr [esp+14h],edx
    mov         dword ptr [esp+10h],eax
L_65ce4:
    mov         dword ptr [esp+24h],4
    jmp L_65d19
L_65cee:
    cmp         edi,2
    jne L_66096
    mov         edx,dword ptr [eax+4]
    mov         eax,dword ptr [esi+4]
    movzx       edi,word ptr [edx]
    movzx       edx,word ptr [eax+2]
    movzx       ebx,word ptr [eax+edx*8]
    cmp         ebx,edi
    movzx       eax,word ptr [eax]
    mov         dword ptr [esp+10h],eax
    jge L_65d19
    mov         eax,edi
    mov         edi,ebx
    mov         ebx,eax
L_65d19:
    mov         edx,edi
    shl         edx,4
    lea         esi,[edx+ecx]
    mov         eax,ebx
    shl         eax,4
    fld         dword ptr [eax+ecx]
    lea         edx,[eax+ecx]
    fsub        dword ptr [esi]
    mov         eax,dword ptr [esp+10h]
    fld         dword ptr [edx+4]
    shl         eax,4
    fsub        dword ptr [esi+4]
    add         eax,ecx
    fstp        dword ptr [esp+54h]
    fld         dword ptr [edx+8]
    fsub        dword ptr [esi+8]
    fld         st(1)
    fmul        st,st(2)
    fld         st(1)
    fmul        st,st(2)
    faddp       st(1),st
    fld         dword ptr [esp+54h]
    fmul        dword ptr [esp+54h]
    faddp       st(1),st
    fst         dword ptr [esp+20h]
    fld         dword ptr [eax]
    fsub        dword ptr [edx]
    fstp        dword ptr [esp+40h]
    fld         dword ptr [eax+4]
    fsub        dword ptr [edx+4]
    fstp        dword ptr [esp+44h]
    fld         dword ptr [eax+8]
    fsub        dword ptr [edx+8]
    fstp        dword ptr [esp+48h]
    fld         dword ptr [esp+40h]
    fmul        dword ptr [esp+40h]
    fld         dword ptr [esp+48h]
    fmul        dword ptr [esp+48h]
    faddp       st(1),st
    fld         dword ptr [esp+44h]
    fmul        dword ptr [esp+44h]
    faddp       st(1),st
    fst         dword ptr [esp+28h]
    fstp        dword ptr [esp+18h]
    fld         dword ptr [esi]
    fsub        dword ptr [eax]
    fstp        dword ptr [esp+30h]
    fld         dword ptr [esi+4]
    fsub        dword ptr [eax+4]
    fstp        dword ptr [esp+34h]
    fld         dword ptr [esi+8]
    fsub        dword ptr [eax+8]
    fstp        dword ptr [esp+38h]
    fld         dword ptr [esp+30h]
    fmul        dword ptr [esp+30h]
    fld         dword ptr [esp+38h]
    fmul        dword ptr [esp+38h]
    faddp       st(1),st
    fld         dword ptr [esp+34h]
    fmul        dword ptr [esp+34h]
    faddp       st(1),st
    fst         dword ptr [esp+2Ch]
    fstp        dword ptr [esp+1Ch]
    fld         dword ptr [K_888a8]
    fld         st(1)
    fucompp
    fnstsw      ax
    test        ah,44h
    jnp L_65dfa
    fsqrt
    fdivr       dword ptr [K_88e5c]
    jmp L_65e02
L_65dfa:
    fstp        st(0)
    fld         dword ptr [K_888a8]
L_65e02:
    fld         st(0)
    fmul        st,st(3)
    fstp        dword ptr [esp+50h]
    fld         dword ptr [esp+54h]
    fmul        st,st(1)
    fstp        dword ptr [esp+54h]
    fxch        st(1)
    fmul        st,st(1)
    fstp        dword ptr [esp+58h]
    fstp        st(0)
    fstp        st(0)
    fld         dword ptr [esp+28h]
    fld         dword ptr [K_888a8]
    fld         st(1)
    fucompp
    fnstsw      ax
    test        ah,44h
    jnp L_65e3f
    fsqrt
    fdivr       dword ptr [K_88e5c]
    jmp L_65e47
L_65e3f:
    fstp        st(0)
    fld         dword ptr [K_888a8]
L_65e47:
    fld         dword ptr [esp+40h]
    fmul        st,st(1)
    fstp        dword ptr [esp+40h]
    fld         dword ptr [esp+44h]
    fmul        st,st(1)
    fstp        dword ptr [esp+44h]
    fld         dword ptr [esp+48h]
    fmul        st,st(1)
    fstp        dword ptr [esp+48h]
    fstp        st(0)
    fld         dword ptr [esp+2Ch]
    fld         dword ptr [K_888a8]
    fld         st(1)
    fucompp
    fnstsw      ax
    test        ah,44h
    jnp L_65e86
    fsqrt
    fdivr       dword ptr [K_88e5c]
    jmp L_65e8e
L_65e86:
    fstp        st(0)
    fld         dword ptr [K_888a8]
L_65e8e:
    fld         dword ptr [esp+30h]
    fmul        st,st(1)
    fstp        dword ptr [esp+30h]
    fld         dword ptr [esp+34h]
    fmul        st,st(1)
    fstp        dword ptr [esp+34h]
    fld         dword ptr [esp+38h]
    fmul        st,st(1)
    fstp        dword ptr [esp+38h]
    fstp        st(0)
    fld         dword ptr [esp+40h]
    fsub        dword ptr [esp+50h]
    fld         dword ptr [esp+44h]
    fsub        dword ptr [esp+54h]
    fld         dword ptr [esp+48h]
    fsub        dword ptr [esp+58h]
    fld         st(2)
    fmul        st,st(3)
    fld         st(1)
    fmul        st,st(2)
    faddp       st(1),st
    fld         st(2)
    fmul        st,st(3)
    faddp       st(1),st
    fcomp       dword ptr [ebp+0Ch]
    fstp        st(0)
    fnstsw      ax
    fstp        st(0)
    test        ah,5
    fstp        st(0)
    jp L_660b9
    cmp         dword ptr [esp+24h],4
    jne L_65fee
    mov         eax,dword ptr [esp+14h]
    shl         eax,4
    fld         dword ptr [ecx+eax]
    add         ecx,eax
    fsub        dword ptr [edx]
    fstp        dword ptr [esp+50h]
    fld         dword ptr [ecx+4]
    fsub        dword ptr [edx+4]
    fld         dword ptr [ecx+8]
    fsub        dword ptr [edx+8]
    fstp        dword ptr [esp+58h]
    fld         dword ptr [esp+50h]
    fmul        dword ptr [esp+50h]
    fld         dword ptr [esp+58h]
    fmul        dword ptr [esp+58h]
    faddp       st(1),st
    fld         st(1)
    fmul        st,st(2)
    faddp       st(1),st
    fst         dword ptr [esp+2Ch]
    fld         dword ptr [K_888a8]
    fld         st(1)
    fucompp
    fnstsw      ax
    test        ah,44h
    jnp L_65f4e
    fsqrt
    fdivr       dword ptr [K_88e5c]
    jmp L_65f56
L_65f4e:
    fstp        st(0)
    fld         dword ptr [K_888a8]
L_65f56:
    fld         dword ptr [esp+50h]
    fmul        st,st(1)
    fxch        st(2)
    fmul        st,st(1)
    fstp        dword ptr [esp+54h]
    fld         dword ptr [esp+58h]
    fmul        st,st(1)
    fstp        dword ptr [esp+58h]
    fld         dword ptr [esp+40h]
    fsubrp      st(2),st
    fstp        st(0)
    fld         dword ptr [esp+44h]
    fsub        dword ptr [esp+54h]
    fld         dword ptr [esp+48h]
    fsub        dword ptr [esp+58h]
    fld         st(0)
    fmul        st,st(1)
    fld         st(2)
    fmul        st,st(3)
    faddp       st(1),st
    fld         st(3)
    fmul        st,st(4)
    faddp       st(1),st
    fcomp       dword ptr [ebp+0Ch]
    fstp        st(0)
    fnstsw      ax
    fstp        st(0)
    test        ah,5
    fstp        st(0)
    jp L_660b9
    fld         dword ptr [esp+2Ch]
    fcomp       dword ptr [esp+18h]
    fnstsw      ax
    test        ah,41h
    jne L_65fee
    mov         edx,dword ptr [esp+14h]
    push        esi
    lea         eax,[esp+34h]
    mov         dword ptr [esp+14h],edx
    call L_652f0
    fld         dword ptr [esp+34h]
    fmul        dword ptr [esp+34h]
    add         esp,4
    fld         dword ptr [esp+38h]
    fmul        dword ptr [esp+38h]
    faddp       st(1),st
    fld         dword ptr [esp+34h]
    fmul        dword ptr [esp+34h]
    faddp       st(1),st
    fstp        dword ptr [esp+1Ch]
L_65fee:
    fld         dword ptr [esp+18h]
    fcomp       dword ptr [esp+1Ch]
    fnstsw      ax
    test        ah,41h
    jne L_66003
    fld         dword ptr [esp+18h]
    jmp L_66007
L_66003:
    fld         dword ptr [esp+1Ch]
L_66007:
    fld         dword ptr [esp+20h]
    fcomp       st(1)
    fnstsw      ax
    test        ah,41h
    jne L_6601a
    fstp        st(0)
    fld         dword ptr [esp+20h]
L_6601a:
    fld         st(0)
    fld         dword ptr [esp+20h]
    fucompp
    fnstsw      ax
    test        ah,44h
    jp L_66042
    mov         ecx,dword ptr [ebp+18h]
    fstp        st(0)
    push        ebx
    push        edi
    call L_6a7d0
    mov         eax,dword ptr [ebp+8]
    mov         byte ptr [eax],1
    pop         edi
    pop         esi
    pop         ebx
    mov         esp,ebp
    pop         ebp
    ret
L_66042:
    fld         st(0)
    fld         dword ptr [esp+18h]
    fucompp
    fnstsw      ax
    test        ah,44h
    jp L_6606e
    mov         eax,dword ptr [esp+10h]
    fstp        st(0)
    mov         ecx,dword ptr [ebp+18h]
    push        eax
    push        ebx
    call L_6a7d0
    mov         eax,dword ptr [ebp+8]
    mov         byte ptr [eax],1
    pop         edi
    pop         esi
    pop         ebx
    mov         esp,ebp
    pop         ebp
    ret
L_6606e:
    fld         dword ptr [esp+1Ch]
    fucompp
    fnstsw      ax
    test        ah,44h
    jp L_660ac
    mov         ecx,dword ptr [esp+10h]
    push        edi
    push        ecx
    mov         ecx,dword ptr [ebp+18h]
    call L_6a7d0
    mov         eax,dword ptr [ebp+8]
    mov         byte ptr [eax],1
    pop         edi
    pop         esi
    pop         ebx
    mov         esp,ebp
    pop         ebp
    ret
L_66096:
    mov         edx,dword ptr [esi+4]
    movzx       ecx,word ptr [edx]
    mov         edx,dword ptr [eax+4]
    movzx       eax,word ptr [edx]
    push        ecx
    mov         ecx,dword ptr [ebp+18h]
    push        eax
    call L_6a7d0
L_660ac:
    mov         eax,dword ptr [ebp+8]
    mov         byte ptr [eax],1
    pop         edi
    pop         esi
    pop         ebx
    mov         esp,ebp
    pop         ebp
    ret
L_660b9:
    mov         eax,dword ptr [ebp+8]
    pop         edi
    pop         esi
    mov         byte ptr [eax],0
    pop         ebx
    mov         esp,ebp
    pop         ebp
    ret
L_660d0:
    push        ebp
    mov         ebp,esp
    and         esp,0FFFFFFF0h
    sub         esp,30h
    mov         eax,dword ptr [ebp+8]
    mov         ecx,dword ptr [ebp+0Ch]
    fld         dword ptr [eax]
    fsub        dword ptr [ecx]
    fst         dword ptr [esp+20h]
    fld         dword ptr [eax+4]
    fsub        dword ptr [ecx+4]
    fst         dword ptr [esp+1Ch]
    fstp        dword ptr [esp+24h]
    fld         dword ptr [eax+8]
    fsub        dword ptr [ecx+8]
    fstp        dword ptr [esp+28h]
    fabs
    fcomp       dword ptr [K_91474]
    fnstsw      ax
    test        ah,5
    jp L_661d3
    fld         dword ptr [esp+1Ch]
    fabs
    fcomp       dword ptr [K_91474]
    fnstsw      ax
    test        ah,5
    jp L_661d3
    fld         dword ptr [esp+24h]
    mov         ecx,dword ptr [ebp+10h]
    fmul        dword ptr [K_888a8]
    mov         dword ptr [ecx+0Ch],0
    fld         dword ptr [esp+28h]
    fmul        dword ptr [K_888a8]
    fsubp       st(1),st
    fld         st(0)
    fstp        dword ptr [ecx]
    fld         dword ptr [esp+20h]
    fmul        dword ptr [K_888a8]
    fld         dword ptr [esp+28h]
    fsub        st,st(1)
    fst         dword ptr [esp+10h]
    fstp        dword ptr [ecx+4]
    fsub        dword ptr [esp+24h]
    fstp        dword ptr [esp+0Ch]
    fld         dword ptr [esp+0Ch]
    mov         eax,dword ptr [esp+0Ch]
    fmul        dword ptr [esp+0Ch]
    mov         dword ptr [ecx+8],eax
    fld         dword ptr [esp+10h]
    fmul        dword ptr [esp+10h]
    faddp       st(1),st
    fld         st(1)
    fmul        st,st(2)
    faddp       st(1),st
    fld         dword ptr [K_888a8]
    fld         st(1)
    fucompp
    fnstsw      ax
    test        ah,44h
    jnp L_661a6
    fsqrt
    fdivr       dword ptr [K_88e5c]
    jmp L_661ae
L_661a6:
    fstp        st(0)
    fld         dword ptr [K_888a8]
L_661ae:
    fxch        st(1)
    fmul        st,st(1)
    fstp        dword ptr [ecx]
    fld         dword ptr [esp+10h]
    fmul        st,st(1)
    fstp        dword ptr [ecx+4]
    fld         dword ptr [esp+0Ch]
    fmul        st,st(1)
    fstp        dword ptr [ecx+8]
    fmul        dword ptr [K_888a8]
    fstp        dword ptr [ecx+0Ch]
    mov         esp,ebp
    pop         ebp
    ret
L_661d3:
    fld         dword ptr [esp+28h]
    fmul        dword ptr [esp+28h]
    fld         dword ptr [esp+24h]
    fmul        dword ptr [esp+24h]
    faddp       st(1),st
    fld         dword ptr [esp+20h]
    fmul        dword ptr [esp+20h]
    faddp       st(1),st
    fld         dword ptr [K_888a8]
    fld         st(1)
    fucompp
    fnstsw      ax
    test        ah,44h
    jnp L_6620a
    fsqrt
    fdivr       dword ptr [K_88e5c]
    jmp L_66212
L_6620a:
    fstp        st(0)
    fld         dword ptr [K_888a8]
L_66212:
    fld         dword ptr [esp+20h]
    mov         ecx,dword ptr [ebp+10h]
    fmul        st,st(1)
    xor         edx,edx
    fld         st(1)
    mov         dword ptr [ecx+0Ch],edx
    fmul        dword ptr [esp+24h]
    fstp        dword ptr [esp+24h]
    fld         dword ptr [esp+28h]
    fmul        st,st(2)
    fld         dword ptr [esp+24h]
    fmul        dword ptr [K_888a8]
    fst         dword ptr [esp+14h]
    fsub        st,st(1)
    fst         dword ptr [esp+10h]
    fstp        dword ptr [ecx]
    fmul        dword ptr [K_888a8]
    fstp        dword ptr [esp+0Ch]
    fld         dword ptr [K_888a8]
    fmul        st,st(1)
    fstp        dword ptr [esp+1Ch]
    fld         dword ptr [esp+0Ch]
    fsub        dword ptr [esp+1Ch]
    fst         dword ptr [esp+18h]
    fstp        dword ptr [ecx+4]
    fstp        st(1)
    fld         dword ptr [esp+14h]
    fsubr       st,st(1)
    fst         dword ptr [ecx+8]
    fld         st(0)
    fmul        st,st(1)
    fld         dword ptr [esp+18h]
    fmul        dword ptr [esp+18h]
    faddp       st(1),st
    fld         dword ptr [esp+10h]
    fmul        dword ptr [esp+10h]
    faddp       st(1),st
    fcomp       dword ptr [K_9147c]
    fnstsw      ax
    fstp        st(0)
    test        ah,5
    jp L_662c2
    fld         dword ptr [esp+24h]
    mov         dword ptr [ecx+0Ch],edx
    fsub        dword ptr [esp+0Ch]
    fstp        dword ptr [ecx]
    fld         dword ptr [esp+0Ch]
    fsub        st,st(1)
    fstp        dword ptr [ecx+4]
    fstp        st(0)
    fld         dword ptr [esp+1Ch]
    fsub        dword ptr [esp+14h]
    fstp        dword ptr [ecx+8]
    jmp L_662c4
L_662c2:
    fstp        st(0)
L_662c4:
    fld         dword ptr [ecx+8]
    fld         dword ptr [ecx+4]
    fld         dword ptr [ecx]
    fld         st(0)
    fmul        st,st(1)
    fld         st(2)
    fmul        st,st(3)
    faddp       st(1),st
    fld         st(3)
    fmul        st,st(4)
    faddp       st(1),st
    fstp        st(3)
    fstp        st(0)
    fstp        st(0)
    fld         dword ptr [K_888a8]
    fld         st(1)
    fucompp
    fnstsw      ax
    test        ah,44h
    jnp L_662fd
    fsqrt
    fdivr       dword ptr [K_88e5c]
    jmp L_66305
L_662fd:
    fstp        st(0)
    fld         dword ptr [K_888a8]
L_66305:
    fld         st(0)
    fmul        dword ptr [ecx]
    fstp        dword ptr [ecx]
    fld         st(0)
    fmul        dword ptr [ecx+4]
    fstp        dword ptr [ecx+4]
    fld         st(0)
    fmul        dword ptr [ecx+8]
    fstp        dword ptr [ecx+8]
    fmul        dword ptr [K_888a8]
    fstp        dword ptr [ecx+0Ch]
    mov         esp,ebp
    pop         ebp
    ret
L_66330:
    push        ebp
    mov         ebp,esp
    and         esp,0FFFFFFF0h
    sub         esp,14h
    mov         eax,dword ptr [ebp+8]
    mov         ecx,dword ptr [eax]
    mov         edx,dword ptr [eax+4]
    mov         eax,dword ptr [ebp+10h]
    mov         dword ptr [eax],edx
    mov         edx,dword ptr [ebp+0Ch]
    mov         edx,dword ptr [edx+4]
    push        ebx
    mov         dword ptr [eax+4],edx
    xor         edx,edx
    mov         dword ptr [eax+8],edx
    mov         dword ptr [eax+10h],edx
    mov         edx,dword ptr [eax]
    push        esi
    mov         esi,dword ptr [ebp+14h]
    push        edi
    mov         dword ptr [eax+0Ch],0FFFFFFFFh
    mov         eax,dword ptr [eax+4]
    movzx       edi,word ptr [eax]
    movzx       ebx,word ptr [edx]
    shl         edi,4
    add         edi,ecx
    push        esi
    shl         ebx,4
    add         ebx,ecx
    push        edi
    push        ebx
    mov         dword ptr [esp+20h],ecx
    call L_660d0
    mov         ecx,dword ptr [ebp+8]
    mov         eax,dword ptr [ecx+8]
    mov         edx,dword ptr [ebp+0Ch]
    add         eax,dword ptr [edx+8]
    shl         eax,1
    add         esp,0Ch
    test        eax,eax
    mov         dword ptr [esp+1Ch],eax
    mov         dword ptr [esp+18h],0
    jle L_664c7
    lea         ebx,[ebx+00000000h]
L_663b0:
    mov         eax,dword ptr [ebp+8]
    mov         eax,dword ptr [eax+8]
    dec         eax
    mov         byte ptr [esp+10h],0
    mov         dword ptr [esp+0Ch],eax
    js L_6642e
    jmp L_663d0
L_663c4:
    mov         eax,dword ptr [esp+0Ch]
    jmp L_663d0
    lea         ebx,[ebx+00000000h]
L_663d0:
    mov         ecx,dword ptr [ebp+8]
    mov         edx,dword ptr [ecx+4]
    movzx       ecx,word ptr [edx+eax*8]
    lea         edx,[edx+eax*8]
    mov         eax,dword ptr [esp+14h]
    shl         ecx,4
    add         ecx,eax
    fld         dword ptr [ecx]
    fsub        dword ptr [ebx]
    fld         dword ptr [ecx+4]
    fsub        dword ptr [ebx+4]
    fld         dword ptr [ecx+8]
    fsub        dword ptr [ebx+8]
    fmul        dword ptr [esi+8]
    fxch        st(2)
    fmul        dword ptr [esi]
    faddp       st(2),st
    fmul        dword ptr [esi+4]
    faddp       st(1),st
    fcomp       dword ptr [K_91484]
    fnstsw      ax
    test        ah,41h
    jne L_66428
    mov         eax,dword ptr [ebp+10h]
    push        esi
    push        edi
    push        ecx
    mov         dword ptr [eax],edx
    mov         ebx,ecx
    call L_660d0
    add         esp,0Ch
    mov         byte ptr [esp+10h],1
L_66428:
    dec         dword ptr [esp+0Ch]
    jns L_663c4
L_6642e:
    mov         ecx,dword ptr [ebp+0Ch]
    mov         eax,dword ptr [ecx+8]
    dec         eax
    mov         dword ptr [esp+0Ch],eax
    js L_6649f
    jmp L_66441
L_6643d:
    mov         eax,dword ptr [esp+0Ch]
L_66441:
    mov         edx,dword ptr [ebp+0Ch]
    mov         ecx,dword ptr [edx+4]
    lea         edx,[ecx+eax*8]
    movzx       ecx,word ptr [edx]
    mov         eax,dword ptr [esp+14h]
    shl         ecx,4
    add         ecx,eax
    fld         dword ptr [ecx]
    fsub        dword ptr [edi]
    fld         dword ptr [ecx+4]
    fsub        dword ptr [edi+4]
    fld         dword ptr [ecx+8]
    fsub        dword ptr [edi+8]
    fmul        dword ptr [esi+8]
    fxch        st(2)
    fmul        dword ptr [esi]
    faddp       st(2),st
    fmul        dword ptr [esi+4]
    faddp       st(1),st
    fcomp       dword ptr [K_91484]
    fnstsw      ax
    test        ah,41h
    jne L_66499
    mov         eax,dword ptr [ebp+10h]
    push        esi
    push        ecx
    push        ebx
    mov         dword ptr [eax+4],edx
    mov         edi,ecx
    call L_660d0
    add         esp,0Ch
    mov         byte ptr [esp+10h],1
L_66499:
    dec         dword ptr [esp+0Ch]
    jns L_6643d
L_6649f:
    mov         al,byte ptr [esp+10h]
    test        al,al
    je L_664c7
    push        esi
    push        edi
    push        ebx
    call L_660d0
    mov         eax,dword ptr [esp+24h]
    mov         ecx,dword ptr [esp+28h]
    add         esp,0Ch
    inc         eax
    cmp         eax,ecx
    mov         dword ptr [esp+18h],eax
    jl L_663b0
L_664c7:
    pop         edi
    pop         esi
    pop         ebx
    mov         esp,ebp
    pop         ebp
    ret
L_664d0:
    push        ebp
    mov         ebp,esp
    and         esp,0FFFFFFF0h
    sub         esp,3Ch
    mov         eax,dword ptr [ebp+0Ch]
    mov         ecx,dword ptr [ebp+14h]
    fld         dword ptr [eax]
    fsub        dword ptr [ecx]
    push        esi
    fld         dword ptr [eax+4]
    mov         esi,dword ptr [ebp+8]
    fsub        dword ptr [ecx+4]
    fstp        dword ptr [esp+34h]
    fld         dword ptr [eax+8]
    fsub        dword ptr [ecx+8]
    fld         st(0)
    fmul        st,st(1)
    fld         dword ptr [esp+34h]
    fmul        dword ptr [esp+34h]
    faddp       st(1),st
    fld         st(2)
    fmul        st,st(3)
    faddp       st(1),st
    fcom        dword ptr [esi+8]
    fnstsw      ax
    test        ah,5
    jp L_66526
    fstp        st(0)
    fstp        st(0)
    fstp        st(0)
    fld         dword ptr [K_888a8]
    pop         esi
    mov         esp,ebp
    pop         ebp
    ret
L_66526:
    fld         dword ptr [K_888a8]
    fld         st(1)
    fucompp
    fnstsw      ax
    test        ah,44h
    jnp L_66541
    fsqrt
    fdivr       dword ptr [K_88e5c]
    jmp L_66549
L_66541:
    fstp        st(0)
    fld         dword ptr [K_888a8]
L_66549:
    fld         st(0)
    mov         eax,dword ptr [ebp+10h]
    fmul        st,st(3)
    mov         edx,dword ptr [ebp+18h]
    fstp        dword ptr [esp+30h]
    fld         dword ptr [esp+34h]
    fmul        st,st(1)
    fstp        dword ptr [esp+34h]
    fmul        st,st(1)
    fstp        st(2)
    fstp        st(0)
    fld         dword ptr [esp+30h]
    fmul        dword ptr [eax]
    fld         dword ptr [esp+34h]
    fmul        dword ptr [eax+4]
    faddp       st(1),st
    fld         st(1)
    fmul        dword ptr [eax+8]
    faddp       st(1),st
    fchs
    fstp        dword ptr [esp+14h]
    fld         dword ptr [ecx]
    fsub        dword ptr [edx]
    fld         dword ptr [ecx+4]
    fsub        dword ptr [edx+4]
    fld         dword ptr [ecx+8]
    fsub        dword ptr [edx+8]
    fld         st(1)
    fmul        dword ptr [eax+8]
    fld         st(1)
    fmul        dword ptr [eax+4]
    fsubp       st(1),st
    fstp        dword ptr [esp+20h]
    fmul        dword ptr [eax]
    fld         st(2)
    fmul        dword ptr [eax+8]
    fsubp       st(1),st
    fstp        dword ptr [esp+24h]
    fxch        st(1)
    fmul        dword ptr [eax+4]
    fxch        st(1)
    fmul        dword ptr [eax]
    fsubp       st(1),st
    fst         dword ptr [esp+28h]
    fmul        dword ptr [esp+28h]
    fld         dword ptr [esp+24h]
    fmul        dword ptr [esp+24h]
    faddp       st(1),st
    fld         dword ptr [esp+20h]
    fmul        dword ptr [esp+20h]
    faddp       st(1),st
    fld         dword ptr [K_888a8]
    fld         st(1)
    fucompp
    fnstsw      ax
    test        ah,44h
    jnp L_665f2
    fsqrt
    fdivr       dword ptr [K_88e5c]
    jmp L_665fa
L_665f2:
    fstp        st(0)
    fld         dword ptr [K_888a8]
L_665fa:
    fld         dword ptr [esp+20h]
    fmul        st,st(1)
    fld         st(1)
    fmul        dword ptr [esp+24h]
    fxch        st(2)
    fmul        dword ptr [esp+28h]
    fmul        st,st(3)
    fxch        st(2)
    fmul        dword ptr [esp+34h]
    faddp       st(2),st
    fmul        dword ptr [esp+30h]
    faddp       st(1),st
    fstp        dword ptr [esp+10h]
    fstp        st(0)
    fld         dword ptr [esp+10h]
    fmul        dword ptr [esp+10h]
    fld         dword ptr [esp+14h]
    fmul        dword ptr [esp+14h]
    faddp       st(1),st
    fcomp       dword ptr [esi+8]
    fnstsw      ax
    test        ah,5
    jp L_66649
    fld         dword ptr [K_91490]
    pop         esi
    mov         esp,ebp
    pop         ebp
    ret
L_66649:
    fld         dword ptr [esp+14h]
    mov         eax,dword ptr [esi+30h]
    fabs
    mov         dword ptr [esp+1Ch],eax
    fst         dword ptr [esp+18h]
    fcomp       dword ptr [K_91474]
    fnstsw      ax
    test        ah,5
    jp L_6669a
    fld         dword ptr [esp+10h]
    fabs
    fcomp       dword ptr [K_91484]
    fnstsw      ax
    test        ah,5
    jp L_6669a
    fld         dword ptr [esp+10h]
    fcomp       dword ptr [K_888a8]
    fnstsw      ax
    test        ah,5
    jp L_6673f
    fld         dword ptr [K_90fd8]
    jmp L_66779
L_6669a:
    fld         dword ptr [esp+10h]
    fabs
    fcomp       dword ptr [esp+18h]
    fnstsw      ax
    test        ah,5
    jp L_666d9
    fld         dword ptr [esp+10h]
    fdiv        dword ptr [esp+14h]
    fsubr       dword ptr [K_89ce8]
    fld         dword ptr [esp+14h]
    fcomp       dword ptr [K_9148c]
    fnstsw      ax
    test        ah,5
    jp L_66771
    fld         dword ptr [K_90fd8]
    jmp L_66777
L_666d9:
    fld         dword ptr [esp+14h]
    fdiv        dword ptr [esp+10h]
    fld         dword ptr [esp+10h]
    fcomp       dword ptr [K_91488]
    fnstsw      ax
    test        ah,5
    jp L_6670d
    fld         dword ptr [esp+18h]
    fcomp       dword ptr [K_91474]
    fnstsw      ax
    test        ah,5
    jp L_6670d
    fstp        st(0)
    fld         dword ptr [K_90fd8]
    jmp L_6672e
L_6670d:
    fld         dword ptr [esp+10h]
    fcomp       dword ptr [K_9148c]
    fnstsw      ax
    test        ah,5
    jp L_66726
    fld         dword ptr [K_90fd8]
    jmp L_6672c
L_66726:
    fld         dword ptr [K_888a8]
L_6672c:
    faddp       st(1),st
L_6672e:
    fld         st(0)
    fabs
    fcomp       dword ptr [esp+1Ch]
    fnstsw      ax
    test        ah,5
    jp L_66747
    fstp        st(0)
L_6673f:
    fld         dword ptr [K_888a8]
    jmp L_66779
L_66747:
    fld         dword ptr [esp+10h]
    fcomp       dword ptr [K_91474]
    fnstsw      ax
    test        ah,41h
    jne L_66771
    fld         dword ptr [esp+18h]
    fcomp       dword ptr [K_91484]
    fnstsw      ax
    test        ah,5
    jp L_66771
    fld         dword ptr [K_91444]
    jmp L_66777
L_66771:
    fld         dword ptr [K_888a8]
L_66777:
    faddp       st(1),st
L_66779:
    fld         dword ptr [esp+1Ch]
    fchs
    fld         st(1)
    fcompp
    fnstsw      ax
    test        ah,5
    jp L_66790
    fadd        dword ptr [K_91444]
L_66790:
    fcom        dword ptr [K_91444]
    fnstsw      ax
    test        ah,41h
    jne L_667a5
    fstp        st(0)
    fld         dword ptr [K_888a8]
L_667a5:
    pop         esi
    mov         esp,ebp
    pop         ebp
    ret
L_667b0:
    mov         eax,dword ptr [ecx+10h]
    mov         edx,dword ptr [ecx+14h]
    mov         ax,word ptr [eax]
    cmp         ax,word ptr [edx]
    jbe L_667c7
    mov         eax,dword ptr [ecx+10h]
    mov         dword ptr [ecx+10h],edx
    mov         dword ptr [ecx+14h],eax
L_667c7:
    mov         edx,dword ptr [ecx+14h]
    mov         eax,dword ptr [ecx+18h]
    mov         dx,word ptr [edx]
    cmp         dx,word ptr [eax]
    jbe L_667e1
    mov         edx,dword ptr [ecx+18h]
    mov         eax,dword ptr [ecx+14h]
    mov         dword ptr [ecx+14h],edx
    mov         dword ptr [ecx+18h],eax
L_667e1:
    mov         eax,dword ptr [ecx+10h]
    mov         edx,dword ptr [ecx+14h]
    mov         ax,word ptr [eax]
    cmp         ax,word ptr [edx]
    jbe L_667f8
    mov         eax,dword ptr [ecx+10h]
    mov         dword ptr [ecx+10h],edx
    mov         dword ptr [ecx+14h],eax
L_667f8:
    ret
L_66800:
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
    call L_73590
    add         esp,0Ch
L_66a2e:
    pop         edi
    mov         dword ptr [ebx+4],esi
    pop         esi
    pop         ebx
    mov         esp,ebp
    pop         ebp
    ret
L_66a40:
    push        ebp
    mov         ebp,esp
    and         esp,0FFFFFFF0h
    sub         esp,14h
    push        ebx
    push        esi
    push        edi
    mov         edi,dword ptr [ebp+18h]
    mov         esi,dword ptr [edi+4]
    mov         eax,dword ptr [edi+8]
    lea         ebx,[esi+1]
    and         eax,7FFFFFFFh
    cmp         eax,ebx
    jge L_66a75
    add         eax,eax
    cmp         ebx,eax
    jl L_66a69
    mov         eax,ebx
L_66a69:
    push        10h
    push        eax
    push        edi
    call L_73590
    add         esp,0Ch
L_66a75:
    mov         eax,dword ptr [edi]
    mov         edx,dword ptr [ebp+8]
    mov         dword ptr [edi+4],ebx
    shl         esi,4
    add         esi,eax
    mov         eax,dword ptr [ebp+10h]
    mov         ecx,esi
    mov         esi,dword ptr [ebp+0Ch]
    fld         dword ptr [esi]
    xor         ebx,ebx
    fsub        dword ptr [eax]
    fld         dword ptr [esi+4]
    fsub        dword ptr [eax+4]
    fld         dword ptr [esi+8]
    fsub        dword ptr [eax+8]
    fld         st(2)
    fmul        dword ptr [edx+8]
    fld         st(1)
    fmul        dword ptr [edx]
    fsubp       st(1),st
    fstp        dword ptr [esp+18h]
    fld         st(1)
    fmul        dword ptr [edx]
    fxch        st(3)
    fmul        dword ptr [edx+4]
    fsubp       st(3),st
    fxch        st(2)
    fstp        dword ptr [esp+1Ch]
    fxch        st(1)
    fmul        dword ptr [edx+4]
    fxch        st(1)
    fmul        dword ptr [edx+8]
    mov         edx,dword ptr [esp+18h]
    mov         dword ptr [ecx+4],edx
    mov         edx,dword ptr [esp+1Ch]
    fsubp       st(1),st
    mov         dword ptr [ecx+8],edx
    mov         edx,dword ptr [ebp+14h]
    mov         dword ptr [ecx+0Ch],ebx
    fstp        dword ptr [ecx]
    fld         dword ptr [edx]
    fsub        dword ptr [eax]
    fld         dword ptr [edx+4]
    fsub        dword ptr [eax+4]
    fld         dword ptr [edx+8]
    fsub        dword ptr [eax+8]
    fxch        st(2)
    fmul        dword ptr [ecx]
    fxch        st(2)
    fmul        dword ptr [esp+1Ch]
    faddp       st(2),st
    fmul        dword ptr [esp+18h]
    faddp       st(1),st
    fcomp       dword ptr [K_91474]
    fnstsw      ax
    test        ah,41h
    jne L_66b28
    fld         dword ptr [ecx]
    mov         dword ptr [ecx+0Ch],ebx
    fchs
    fstp        dword ptr [ecx]
    fld         dword ptr [esp+18h]
    fchs
    fstp        dword ptr [ecx+4]
    fld         dword ptr [esp+1Ch]
    fchs
    fstp        dword ptr [ecx+8]
L_66b28:
    fld         dword ptr [ecx+8]
    fld         dword ptr [ecx+4]
    fld         dword ptr [ecx]
    fld         st(0)
    fmul        st,st(1)
    fld         st(2)
    fmul        st,st(3)
    faddp       st(1),st
    fld         st(3)
    fmul        st,st(4)
    faddp       st(1),st
    fstp        st(3)
    fstp        st(0)
    fstp        st(0)
    fcom        dword ptr [K_8a1bc]
    fnstsw      ax
    test        ah,41h
    jne L_66bb3
    fld         dword ptr [K_888a8]
    fld         st(1)
    fucompp
    fnstsw      ax
    test        ah,44h
    jnp L_66b6e
    fsqrt
    fdivr       dword ptr [K_88e5c]
    jmp L_66b76
L_66b6e:
    fstp        st(0)
    fld         dword ptr [K_888a8]
L_66b76:
    fld         st(0)
    fmul        dword ptr [ecx]
    fst         dword ptr [ecx]
    fld         st(1)
    fmul        dword ptr [ecx+4]
    fst         dword ptr [ecx+4]
    fld         st(2)
    fmul        dword ptr [ecx+8]
    fst         dword ptr [ecx+8]
    fxch        st(3)
    fmul        dword ptr [K_888a8]
    fstp        dword ptr [ecx+0Ch]
    fmul        dword ptr [esi+4]
    fxch        st(1)
    fmul        dword ptr [esi]
    faddp       st(1),st
    fxch        st(1)
    fmul        dword ptr [esi+8]
    faddp       st(1),st
    fchs
    fstp        dword ptr [ecx+0Ch]
    pop         edi
    pop         esi
    pop         ebx
    mov         esp,ebp
    pop         ebp
    ret
L_66bb3:
    mov         esi,dword ptr [edi+4]
    fstp        st(0)
    mov         eax,dword ptr [edi+8]
    dec         esi
    and         eax,7FFFFFFFh
    cmp         eax,esi
    jge L_66bd9
    add         eax,eax
    cmp         esi,eax
    jl L_66bcd
    mov         eax,esi
L_66bcd:
    push        10h
    push        eax
    push        edi
    call L_73590
    add         esp,0Ch
L_66bd9:
    mov         dword ptr [edi+4],esi
    pop         edi
    pop         esi
    pop         ebx
    mov         esp,ebp
    pop         ebp
    ret
L_66bf0:
    push        ebx
    push        esi
    push        edi
    mov         edi,dword ptr [esp+10h]
    mov         eax,dword ptr [edi+4]
    mov         ecx,dword ptr [edi]
    lea         esi,[eax-1]
    lea         eax,[esi+1]
    cmp         eax,4
    mov         edx,ecx
    jl L_66ce8
    shr         eax,2
    mov         ebx,eax
    neg         eax
    lea         esi,[esi+eax*4]
    push        ebp
    jmp L_66c20
    lea         ebx,[ebx+00000000h]
L_66c20:
    fld         dword ptr [K_888a8]
    fld         dword ptr [ecx+0Ch]
    fucompp
    fnstsw      ax
    test        ah,44h
    jp L_66c4d
    mov         ebp,dword ptr [ecx]
    mov         eax,edx
    mov         dword ptr [eax],ebp
    mov         ebp,dword ptr [ecx+4]
    mov         dword ptr [eax+4],ebp
    mov         ebp,dword ptr [ecx+8]
    mov         dword ptr [eax+8],ebp
    mov         ebp,dword ptr [ecx+0Ch]
    add         edx,10h
    mov         dword ptr [eax+0Ch],ebp
L_66c4d:
    fld         dword ptr [K_888a8]
    add         ecx,10h
    fld         dword ptr [ecx+0Ch]
    fucompp
    fnstsw      ax
    test        ah,44h
    jp L_66c7d
    mov         ebp,dword ptr [ecx]
    mov         eax,edx
    mov         dword ptr [eax],ebp
    mov         ebp,dword ptr [ecx+4]
    mov         dword ptr [eax+4],ebp
    mov         ebp,dword ptr [ecx+8]
    mov         dword ptr [eax+8],ebp
    mov         ebp,dword ptr [ecx+0Ch]
    add         edx,10h
    mov         dword ptr [eax+0Ch],ebp
L_66c7d:
    fld         dword ptr [K_888a8]
    add         ecx,10h
    fld         dword ptr [ecx+0Ch]
    fucompp
    fnstsw      ax
    test        ah,44h
    jp L_66cad
    mov         ebp,dword ptr [ecx]
    mov         eax,edx
    mov         dword ptr [eax],ebp
    mov         ebp,dword ptr [ecx+4]
    mov         dword ptr [eax+4],ebp
    mov         ebp,dword ptr [ecx+8]
    mov         dword ptr [eax+8],ebp
    mov         ebp,dword ptr [ecx+0Ch]
    add         edx,10h
    mov         dword ptr [eax+0Ch],ebp
L_66cad:
    fld         dword ptr [K_888a8]
    add         ecx,10h
    fld         dword ptr [ecx+0Ch]
    fucompp
    fnstsw      ax
    test        ah,44h
    jp L_66cdd
    mov         ebp,dword ptr [ecx]
    mov         eax,edx
    mov         dword ptr [eax],ebp
    mov         ebp,dword ptr [ecx+4]
    mov         dword ptr [eax+4],ebp
    mov         ebp,dword ptr [ecx+8]
    mov         dword ptr [eax+8],ebp
    mov         ebp,dword ptr [ecx+0Ch]
    add         edx,10h
    mov         dword ptr [eax+0Ch],ebp
L_66cdd:
    add         ecx,10h
    dec         ebx
    jne L_66c20
    pop         ebp
L_66ce8:
    test        esi,esi
    jl L_66d23
    inc         esi
    lea         ecx,[ecx]
L_66cf0:
    fld         dword ptr [K_888a8]
    fld         dword ptr [ecx+0Ch]
    fucompp
    fnstsw      ax
    test        ah,44h
    jp L_66d1d
    mov         ebx,dword ptr [ecx]
    mov         eax,edx
    mov         dword ptr [eax],ebx
    mov         ebx,dword ptr [ecx+4]
    mov         dword ptr [eax+4],ebx
    mov         ebx,dword ptr [ecx+8]
    mov         dword ptr [eax+8],ebx
    mov         ebx,dword ptr [ecx+0Ch]
    add         edx,10h
    mov         dword ptr [eax+0Ch],ebx
L_66d1d:
    add         ecx,10h
    dec         esi
    jne L_66cf0
L_66d23:
    mov         ebx,dword ptr [edi]
    mov         eax,dword ptr [edi+8]
    sub         edx,ebx
    sar         edx,4
    mov         esi,edx
    and         eax,7FFFFFFFh
    cmp         eax,esi
    jge L_66d4c
    add         eax,eax
    cmp         esi,eax
    jl L_66d40
    mov         eax,esi
L_66d40:
    push        10h
    push        eax
    push        edi
    call L_73590
    add         esp,0Ch
L_66d4c:
    mov         dword ptr [edi+4],esi
    pop         edi
    pop         esi
    pop         ebx
    ret
L_66d60:
    push        ebp
    mov         ebp,esp
    and         esp,0FFFFFFF0h
    sub         esp,44h
    mov         eax,dword ptr [ebp+0Ch]
    mov         ecx,dword ptr [eax+0Ch]
    mov         edx,dword ptr [ebp+18h]
    mov         eax,dword ptr [ebp+8]
    mov         dword ptr [esp],ecx
    movzx       ecx,word ptr [edx]
    shl         ecx,4
    add         ecx,eax
    mov         eax,dword ptr [ebp+14h]
    mov         edx,dword ptr [eax]
    mov         dword ptr [esp+4],edx
    mov         edx,dword ptr [eax+4]
    mov         dword ptr [esp+8],edx
    mov         edx,dword ptr [eax+8]
    mov         eax,dword ptr [eax+0Ch]
    mov         dword ptr [esp+0Ch],edx
    mov         edx,dword ptr [ebp+1Ch]
    mov         dword ptr [esp+10h],eax
    mov         eax,dword ptr [ebp+20h]
    fld         dword ptr [edx]
    push        ebx
    push        esi
    fsub        dword ptr [eax]
    push        edi
    fstp        dword ptr [esp+20h]
    fld         dword ptr [edx+4]
    fsub        dword ptr [eax+4]
    fstp        dword ptr [esp+24h]
    fld         dword ptr [edx+8]
    fsub        dword ptr [eax+8]
    fstp        dword ptr [esp+28h]
    fld         dword ptr [ebp+10h]
    fcomp       dword ptr [esp+0Ch]
    fnstsw      ax
    test        ah,5
    jnp L_66e78
    fld         dword ptr [K_91444]
    fsub        dword ptr [esp+0Ch]
    fcomp       dword ptr [ebp+10h]
    fnstsw      ax
    test        ah,5
    jnp L_66e78
    fld         dword ptr [ebp+10h]
    fsub        dword ptr [K_90fd8]
    fabs
    fcomp       dword ptr [esp+0Ch]
    fnstsw      ax
    test        ah,5
    jp L_66f19
    fld         dword ptr [esp+28h]
    fmul        dword ptr [esp+10h]
    fld         dword ptr [esp+18h]
    fmul        dword ptr [esp+20h]
    fsubp       st(1),st
    fld         dword ptr [esp+20h]
    fmul        dword ptr [esp+14h]
    fld         dword ptr [esp+24h]
    fmul        dword ptr [esp+10h]
    fsubp       st(1),st
    fstp        dword ptr [esp+0Ch]
    fld         dword ptr [esp+24h]
    fmul        dword ptr [esp+18h]
    fld         dword ptr [esp+28h]
    fmul        dword ptr [esp+14h]
    fsubp       st(1),st
    fstp        dword ptr [esp+10h]
    fld         st(0)
    fmul        st,st(1)
    fld         dword ptr [esp+0Ch]
    fmul        dword ptr [esp+0Ch]
    faddp       st(1),st
    fld         dword ptr [esp+10h]
    fmul        dword ptr [esp+10h]
    faddp       st(1),st
    fld         dword ptr [K_888a8]
    fld         st(1)
    fucompp
    fnstsw      ax
    test        ah,44h
    jnp L_66ee9
    fsqrt
    fdivr       dword ptr [K_88e5c]
    jmp L_66ef1
L_66e78:
    fld         dword ptr [esp+18h]
    fmul        dword ptr [esp+20h]
    fld         dword ptr [esp+28h]
    fmul        dword ptr [esp+10h]
    fsubp       st(1),st
    fld         dword ptr [esp+24h]
    fmul        dword ptr [esp+10h]
    fld         dword ptr [esp+20h]
    fmul        dword ptr [esp+14h]
    fsubp       st(1),st
    fstp        dword ptr [esp+0Ch]
    fld         dword ptr [esp+28h]
    fmul        dword ptr [esp+14h]
    fld         dword ptr [esp+24h]
    fmul        dword ptr [esp+18h]
    fsubp       st(1),st
    fstp        dword ptr [esp+10h]
    fld         dword ptr [esp+0Ch]
    fmul        dword ptr [esp+0Ch]
    fld         st(1)
    fmul        st,st(2)
    faddp       st(1),st
    fld         dword ptr [esp+10h]
    fmul        dword ptr [esp+10h]
    faddp       st(1),st
    fld         dword ptr [K_888a8]
    fld         st(1)
    fucompp
    fnstsw      ax
    test        ah,44h
    jnp L_66ee9
    fsqrt
    fdivr       dword ptr [K_88e5c]
    jmp L_66ef1
L_66ee9:
    fstp        st(0)
    fld         dword ptr [K_888a8]
L_66ef1:
    fld         dword ptr [esp+10h]
    fmul        st,st(1)
    fstp        dword ptr [esp+10h]
    fld         st(0)
    fmul        st,st(2)
    fstp        dword ptr [esp+14h]
    fld         dword ptr [esp+0Ch]
    fmul        st,st(1)
    fstp        dword ptr [esp+18h]
    fmul        dword ptr [K_888a8]
    fstp        dword ptr [esp+1Ch]
    fstp        st(0)
L_66f19:
    cmp         ecx,edx
    fld         dword ptr [esp+18h]
    fmul        dword ptr [esp+20h]
    mov         eax,dword ptr [ecx+0Ch]
    fld         dword ptr [esp+28h]
    mov         dword ptr [esp+2Ch],eax
    fmul        dword ptr [esp+10h]
    fsubp       st(1),st
    fld         dword ptr [esp+24h]
    fmul        dword ptr [esp+10h]
    fld         dword ptr [esp+20h]
    fmul        dword ptr [esp+14h]
    fsubp       st(1),st
    fld         dword ptr [esp+28h]
    fmul        dword ptr [esp+14h]
    fld         dword ptr [esp+24h]
    fmul        dword ptr [esp+18h]
    fsubp       st(1),st
    fadd        dword ptr [ecx]
    fstp        dword ptr [esp+20h]
    fxch        st(1)
    fadd        dword ptr [ecx+4]
    fstp        dword ptr [esp+24h]
    fadd        dword ptr [ecx+8]
    jne L_66f86
    mov         eax,dword ptr [esp+20h]
    fst         dword ptr [esp+48h]
    mov         dword ptr [esp+40h],eax
    mov         eax,dword ptr [esp+24h]
    mov         dword ptr [esp+44h],eax
    mov         eax,dword ptr [esp+2Ch]
    jmp L_66f9d
L_66f86:
    mov         eax,dword ptr [ecx]
    mov         dword ptr [esp+40h],eax
    mov         eax,dword ptr [ecx+4]
    mov         dword ptr [esp+44h],eax
    mov         eax,dword ptr [ecx+8]
    mov         dword ptr [esp+48h],eax
    mov         eax,dword ptr [ecx+0Ch]
L_66f9d:
    cmp         ecx,edx
    mov         dword ptr [esp+4Ch],eax
    jne L_66fc0
    mov         edx,dword ptr [ecx]
    fstp        st(0)
    mov         eax,dword ptr [ecx+4]
    mov         dword ptr [esp+30h],edx
    mov         edx,dword ptr [ecx+8]
    mov         dword ptr [esp+34h],eax
    mov         eax,dword ptr [ecx+0Ch]
    mov         dword ptr [esp+38h],edx
    jmp L_66fd8
L_66fc0:
    mov         ecx,dword ptr [esp+20h]
    fstp        dword ptr [esp+38h]
    mov         edx,dword ptr [esp+24h]
    mov         eax,dword ptr [esp+2Ch]
    mov         dword ptr [esp+30h],ecx
    mov         dword ptr [esp+34h],edx
L_66fd8:
    mov         ebx,dword ptr [ebp+24h]
    mov         dword ptr [esp+3Ch],eax
    mov         eax,dword ptr [ebx+4]
    xor         esi,esi
    test        eax,eax
    jle L_67029
    jmp L_66ff0
    lea         ebx,[ebx+00000000h]
L_66ff0:
    mov         ecx,dword ptr [ebx]
    lea         edi,[ecx+esi*8]
    lea         edx,[esp+30h]
    push        edx
    mov         edx,dword ptr [edi]
    lea         eax,[esp+44h]
    push        eax
    movzx       eax,word ptr [edx]
    mov         edx,dword ptr [ebp+8]
    lea         ecx,[esp+18h]
    push        ecx
    mov         ecx,dword ptr [ebp+0Ch]
    shl         eax,4
    add         eax,edx
    push        eax
    push        ecx
    call L_664d0
    add         esp,14h
    fstp        dword ptr [edi+4]
    mov         eax,dword ptr [ebx+4]
    inc         esi
    cmp         esi,eax
    jl L_66ff0
L_67029:
    mov         eax,dword ptr [ebx+4]
    cmp         eax,1
    jle L_67045
    mov         edx,dword ptr [ebx]
    push        offset L_65320
    dec         eax
    push        eax
    push        0
    push        edx
    call L_656a0
    add         esp,10h
L_67045:
    pop         edi
    pop         esi
    pop         ebx
    mov         esp,ebp
    pop         ebp
    ret
L_67050:
    mov         edx,dword ptr [esp+10h]
    sub         esp,1Ch
    push        ebx
    mov         ebx,dword ptr [esp+2Ch]
    mov         ecx,dword ptr [ebx+4]
    test        ecx,ecx
    push        ebp
    push        esi
    push        edi
    jne L_67071
    mov         eax,dword ptr [edx]
    mov         eax,dword ptr [eax+4]
    mov         dword ptr [esp+10h],eax
    jmp L_670aa
L_67071:
    mov         eax,dword ptr [edx+4]
    test        eax,eax
    jne L_67083
    mov         eax,dword ptr [ebx]
    mov         eax,dword ptr [eax+4]
    mov         dword ptr [esp+10h],eax
    jmp L_670aa
L_67083:
    mov         eax,dword ptr [edx]
    fld         dword ptr [eax+4]
    mov         eax,dword ptr [ebx]
    fld         dword ptr [eax+4]
    fst         dword ptr [esp+14h]
    fcomp       st(1)
    fnstsw      ax
    test        ah,5
    jp L_670a6
    mov         eax,dword ptr [esp+14h]
    fstp        st(0)
    mov         dword ptr [esp+10h],eax
    jmp L_670aa
L_670a6:
    fstp        dword ptr [esp+10h]
L_670aa:
    mov         edi,dword ptr [esp+34h]
    mov         ebp,dword ptr [edi+4]
    mov         esi,dword ptr [esp+40h]
    xor         eax,eax
    test        ecx,ecx
    mov         dword ptr [esp+34h],ebp
    mov         dword ptr [esp+14h],eax
    jle L_671a1
    jmp L_670d0
L_670c9:
    mov         edx,dword ptr [esp+3Ch]
    lea         ecx,[ecx]
L_670d0:
    mov         ecx,dword ptr [ebx]
    fld         dword ptr [ecx+eax*8+4]
    lea         ecx,[ecx+eax*8]
    fsub        dword ptr [esp+10h]
    mov         eax,dword ptr [esp+30h]
    fld         st(0)
    fcomp       dword ptr [eax]
    fnstsw      ax
    test        ah,41h
    je L_672c6
    mov         ebx,dword ptr [ecx]
    fadd        dword ptr [edi+10h]
    mov         ecx,dword ptr [edi]
    movzx       edx,word ptr [ecx]
    mov         eax,dword ptr [esi+4]
    fstp        dword ptr [esp+28h]
    xor         ecx,ecx
    test        eax,eax
    mov         dword ptr [esp+24h],edx
    jle L_67147
    mov         eax,dword ptr [esi]
    lea         ecx,[ecx]
L_67110:
    mov         ebp,dword ptr [eax]
    mov         dx,word ptr [ebx]
    cmp         word ptr [ebp],dx
    jne L_67138
    mov         edx,dword ptr [eax+4]
    mov         dx,word ptr [edx]
    mov         ebp,dword ptr [esp+34h]
    cmp         dx,word ptr [ebp]
    jne L_67138
    mov         edx,dword ptr [esp+24h]
    cmp         dword ptr [eax+0Ch],edx
    je L_67294
L_67138:
    mov         edx,dword ptr [esi+4]
    inc         ecx
    add         eax,14h
    cmp         ecx,edx
    jl L_67110
    mov         ebp,dword ptr [esp+34h]
L_67147:
    mov         eax,dword ptr [esi+8]
    mov         ecx,dword ptr [esi+4]
    and         eax,7FFFFFFFh
    cmp         ecx,eax
    jne L_67161
    push        14h
    push        esi
    call L_73600
    add         esp,8
L_67161:
    mov         eax,dword ptr [esi+4]
    mov         edx,dword ptr [esi]
    lea         ecx,[eax+eax*4]
    lea         eax,[edx+ecx*4]
    mov         ecx,dword ptr [esp+24h]
    mov         edx,dword ptr [esp+28h]
    mov         dword ptr [eax],ebx
    mov         dword ptr [eax+4],ebp
    mov         dword ptr [eax+8],edi
    mov         dword ptr [eax+0Ch],ecx
    mov         dword ptr [eax+10h],edx
    inc         dword ptr [esi+4]
L_67185:
    mov         eax,dword ptr [esp+14h]
    mov         ebx,dword ptr [esp+38h]
    mov         ecx,dword ptr [ebx+4]
    inc         eax
    cmp         eax,ecx
    mov         dword ptr [esp+14h],eax
    jl L_670c9
    mov         edx,dword ptr [esp+3Ch]
L_671a1:
    mov         eax,dword ptr [edi]
    mov         ecx,dword ptr [edx+4]
    mov         dword ptr [esp+38h],eax
    xor         eax,eax
    test        ecx,ecx
    mov         dword ptr [esp+34h],eax
    jle L_67301
    jmp L_671c0
    lea         ebx,[ebx+00000000h]
L_671c0:
    mov         ecx,dword ptr [edx]
    fld         dword ptr [ecx+eax*8+4]
    mov         edx,dword ptr [esp+30h]
    fsub        dword ptr [esp+10h]
    lea         ecx,[ecx+eax*8]
    fld         st(0)
    fcomp       dword ptr [edx]
    fnstsw      ax
    test        ah,41h
    je L_672ff
    mov         eax,dword ptr [edi+4]
    fadd        dword ptr [edi+10h]
    mov         ebx,dword ptr [ecx]
    movzx       ecx,word ptr [eax]
    mov         eax,dword ptr [esi+4]
    fstp        dword ptr [esp+28h]
    mov         dword ptr [esp+24h],ecx
    xor         ecx,ecx
    test        eax,eax
    jle L_67231
    mov         edx,dword ptr [esp+38h]
    mov         eax,dword ptr [esi]
    mov         dx,word ptr [edx]
L_67205:
    mov         ebp,dword ptr [eax]
    cmp         word ptr [ebp],dx
    jne L_67226
    mov         ebp,dword ptr [eax+4]
    mov         bp,word ptr [ebp]
    cmp         bp,word ptr [ebx]
    jne L_67226
    mov         ebp,dword ptr [esp+24h]
    cmp         dword ptr [eax+0Ch],ebp
    je L_672cd
L_67226:
    mov         ebp,dword ptr [esi+4]
    inc         ecx
    add         eax,14h
    cmp         ecx,ebp
    jl L_67205
L_67231:
    mov         ecx,dword ptr [esi+8]
    mov         eax,dword ptr [esi+4]
    and         ecx,7FFFFFFFh
    cmp         eax,ecx
    jne L_6724c
    push        14h
    push        esi
    call L_73600
    add         esp,8
L_6724c:
    mov         eax,dword ptr [esi+4]
    lea         edx,[eax+eax*4]
    mov         eax,dword ptr [esi]
    lea         ecx,[eax+edx*4]
    mov         edx,dword ptr [esp+38h]
    mov         eax,dword ptr [esp+24h]
    mov         dword ptr [ecx],edx
    mov         edx,dword ptr [esp+28h]
    mov         dword ptr [ecx+4],ebx
    mov         dword ptr [ecx+8],edi
    mov         dword ptr [ecx+0Ch],eax
    mov         dword ptr [ecx+10h],edx
    inc         dword ptr [esi+4]
L_67274:
    mov         eax,dword ptr [esp+34h]
    mov         edx,dword ptr [esp+3Ch]
    mov         ecx,dword ptr [edx+4]
    inc         eax
    cmp         eax,ecx
    mov         dword ptr [esp+34h],eax
    jl L_671c0
    pop         edi
    pop         esi
    pop         ebp
    pop         ebx
    add         esp,1Ch
    ret
L_67294:
    lea         eax,[ecx+ecx*4]
    mov         ecx,dword ptr [esi]
    fld         dword ptr [ecx+eax*4+10h]
    lea         ecx,[ecx+eax*4]
    fcomp       dword ptr [esp+28h]
    fnstsw      ax
    test        ah,5
    jp L_67185
    mov         dword ptr [ecx],ebx
    mov         dword ptr [ecx+4],ebp
    mov         dword ptr [ecx+8],edi
    mov         dword ptr [ecx+0Ch],edx
    mov         edx,dword ptr [esp+28h]
    mov         dword ptr [ecx+10h],edx
    jmp L_67185
L_672c6:
    fstp        st(0)
    jmp L_671a1
L_672cd:
    lea         eax,[ecx+ecx*4]
    mov         ecx,dword ptr [esi]
    fld         dword ptr [ecx+eax*4+10h]
    lea         ecx,[ecx+eax*4]
    fcomp       dword ptr [esp+28h]
    fnstsw      ax
    test        ah,5
    jp L_67274
    mov         edx,dword ptr [esp+38h]
    mov         eax,dword ptr [esp+28h]
    mov         dword ptr [ecx],edx
    mov         dword ptr [ecx+4],ebx
    mov         dword ptr [ecx+8],edi
    mov         dword ptr [ecx+0Ch],ebp
    mov         dword ptr [ecx+10h],eax
    jmp L_67274
L_672ff:
    fstp        st(0)
L_67301:
    pop         edi
    pop         esi
    pop         ebp
    pop         ebx
    add         esp,1Ch
    ret
L_67310:
    push        ebp
    mov         ebp,esp
    and         esp,0FFFFFFF0h
    sub         esp,74h
    mov         eax,dword ptr [ebp+18h]
    mov         ecx,dword ptr [ebp+14h]
    push        ebx
    mov         byte ptr [eax],0
    cmp         byte ptr [ecx],0
    push        esi
    push        edi
    je L_67a7c
    mov         esi,dword ptr [ebp+8]
    mov         edx,dword ptr [esi+8]
    mov         ebx,dword ptr [esi+4]
    mov         edi,dword ptr [esi]
    xor         ecx,ecx
    xor         eax,eax
    cmp         edx,ecx
    mov         dword ptr [esp+2Ch],ebx
    mov         dword ptr [esp+40h],edi
    jle L_67360
    lea         esp,[esp+00000000h]
L_67350:
    mov         edx,dword ptr [esi+4]
    mov         word ptr [edx+eax*8+6],cx
    mov         edx,dword ptr [esi+8]
    inc         eax
    cmp         eax,edx
    jl L_67350
L_67360:
    mov         edx,dword ptr [esi+8]
    xor         eax,eax
    cmp         edx,ecx
    mov         dword ptr [esp+3Ch],eax
    jle L_67689
L_67371:
    mov         ecx,dword ptr [esi+4]
    lea         eax,[ecx+eax*8]
    mov         edx,1
    cmp         word ptr [eax+6],dx
    je L_67673
    movzx       ecx,word ptr [eax+2]
    mov         word ptr [eax+6],dx
    mov         word ptr [ebx+ecx*8+6],dx
    movzx       eax,word ptr [eax]
    lea         ecx,[ebx+ecx*8]
    movzx       ecx,word ptr [ecx]
    mov         dword ptr [esp+1Ch],ecx
    mov         esi,eax
    shl         ecx,4
    add         ecx,edi
    shl         esi,4
    add         esi,edi
    fld         dword ptr [ecx]
    mov         dword ptr [esp+30h],eax
    mov         dword ptr [esp+24h],eax
    fsub        dword ptr [esi]
    mov         eax,dword ptr [esp+1Ch]
    mov         dword ptr [esp+28h],eax
    mov         eax,dword ptr [esp+30h]
    fstp        dword ptr [esp+50h]
    xor         edx,edx
    cmp         dword ptr [esp+1Ch],eax
    fld         dword ptr [ecx+4]
    fsub        dword ptr [esi+4]
    mov         dword ptr [esp+34h],358637BDh
    fstp        dword ptr [esp+54h]
    fld         dword ptr [ecx+8]
    fsub        dword ptr [esi+8]
    mov         dword ptr [esp+44h],0
    mov         dword ptr [esp+4Ch],80000000h
    mov         dword ptr [esp+48h],edx
    fstp        dword ptr [esp+58h]
    fld         dword ptr [esp+54h]
    fmul        dword ptr [esp+54h]
    fld         dword ptr [esp+58h]
    fmul        dword ptr [esp+58h]
    faddp       st(1),st
    fld         dword ptr [esp+50h]
    fmul        dword ptr [esp+50h]
    faddp       st(1),st
    fstp        dword ptr [esp+38h]
    je L_67611
    fld         dword ptr [K_888a8]
    fld         dword ptr [esi+0Ch]
    fucompp
    fnstsw      ax
    test        ah,44h
    jp L_67611
    fld         dword ptr [K_888a8]
    fld         dword ptr [ecx+0Ch]
    fucompp
    fnstsw      ax
    test        ah,44h
    jp L_67611
    mov         ecx,dword ptr [ebp+10h]
    mov         eax,dword ptr [ebp+0Ch]
    inc         ecx
    cmp         eax,ecx
    mov         dword ptr [esp+14h],eax
    jge L_67611
    shl         eax,4
    lea         ecx,[eax+edi+4]
    mov         dword ptr [esp+20h],ecx
    mov         edi,edi
L_67470:
    mov         eax,dword ptr [esp+14h]
    cmp         dword ptr [esp+30h],eax
    je L_675f1
    cmp         dword ptr [esp+1Ch],eax
    je L_675f1
    fld         dword ptr [K_888a8]
    mov         ecx,dword ptr [esp+20h]
    fld         dword ptr [ecx+8]
    fucompp
    fnstsw      ax
    test        ah,44h
    jp L_675f1
    fld         dword ptr [ecx-4]
    mov         eax,dword ptr [ebp+14h]
    fsub        dword ptr [esi]
    fld         dword ptr [ecx]
    fsub        dword ptr [esi+4]
    fld         dword ptr [ecx+4]
    fsub        dword ptr [esi+8]
    fld         dword ptr [esp+58h]
    fmul        st,st(1)
    fld         st(3)
    fmul        dword ptr [esp+50h]
    faddp       st(1),st
    fld         st(2)
    fmul        dword ptr [esp+54h]
    faddp       st(1),st
    fstp        dword ptr [esp+18h]
    fld         dword ptr [esp+54h]
    fmul        st,st(1)
    fld         st(2)
    fmul        dword ptr [esp+58h]
    fsubp       st(1),st
    fstp        dword ptr [esp+70h]
    fld         dword ptr [esp+58h]
    fmul        st,st(3)
    fxch        st(1)
    fmul        dword ptr [esp+50h]
    fsubp       st(1),st
    fstp        dword ptr [esp+74h]
    fmul        dword ptr [esp+50h]
    fld         dword ptr [esp+54h]
    fmul        st,st(2)
    fsubp       st(1),st
    fstp        st(1)
    fld         dword ptr [esp+70h]
    fmul        dword ptr [esp+70h]
    fld         st(1)
    fmul        st,st(2)
    faddp       st(1),st
    fld         dword ptr [esp+74h]
    fmul        dword ptr [esp+74h]
    faddp       st(1),st
    fcomp       dword ptr [eax+8]
    fnstsw      ax
    fstp        st(0)
    test        ah,5
    jp L_675f1
    fld         dword ptr [esp+18h]
    fcomp       dword ptr [esp+34h]
    fnstsw      ax
    test        ah,5
    jp L_67575
    mov         eax,dword ptr [esp+4Ch]
    and         eax,7FFFFFFFh
    cmp         edx,eax
    jne L_67558
    lea         ecx,[esp+44h]
    push        4
    push        ecx
    call L_73600
    mov         edx,dword ptr [esp+50h]
    add         esp,8
L_67558:
    mov         eax,dword ptr [esp+24h]
    mov         ecx,dword ptr [esp+44h]
    mov         dword ptr [ecx+edx*4],eax
    mov         eax,dword ptr [esp+14h]
    mov         ecx,dword ptr [esp+18h]
    mov         dword ptr [esp+24h],eax
    mov         dword ptr [esp+34h],ecx
    jmp L_675e8
L_67575:
    fld         dword ptr [esp+18h]
    fcomp       dword ptr [esp+38h]
    fnstsw      ax
    test        ah,41h
    mov         eax,dword ptr [esp+4Ch]
    jne L_675c1
    and         eax,7FFFFFFFh
    cmp         edx,eax
    jne L_675a4
    lea         edx,[esp+44h]
    push        4
    push        edx
    call L_73600
    mov         edx,dword ptr [esp+50h]
    add         esp,8
L_675a4:
    mov         eax,dword ptr [esp+28h]
    mov         ecx,dword ptr [esp+44h]
    mov         dword ptr [ecx+edx*4],eax
    mov         eax,dword ptr [esp+14h]
    mov         ecx,dword ptr [esp+18h]
    mov         dword ptr [esp+28h],eax
    mov         dword ptr [esp+38h],ecx
    jmp L_675e8
L_675c1:
    and         eax,7FFFFFFFh
    cmp         edx,eax
    jne L_675dd
    lea         edx,[esp+44h]
    push        4
    push        edx
    call L_73600
    mov         edx,dword ptr [esp+50h]
    add         esp,8
L_675dd:
    mov         eax,dword ptr [esp+44h]
    mov         ecx,dword ptr [esp+14h]
    mov         dword ptr [eax+edx*4],ecx
L_675e8:
    mov         edx,dword ptr [esp+48h]
    inc         edx
    mov         dword ptr [esp+48h],edx
L_675f1:
    mov         eax,dword ptr [esp+20h]
    mov         ecx,dword ptr [esp+14h]
    add         eax,10h
    mov         dword ptr [esp+20h],eax
    mov         eax,dword ptr [ebp+10h]
    inc         ecx
    inc         eax
    cmp         ecx,eax
    mov         dword ptr [esp+14h],ecx
    jl L_67470
L_67611:
    xor         ecx,ecx
    test        edx,edx
    jle L_6764d
    jmp L_67620
    lea         esp,[esp+00000000h]
L_67620:
    mov         eax,dword ptr [esp+44h]
    mov         eax,dword ptr [eax+ecx*4]
    cmp         eax,dword ptr [esp+24h]
    je L_67648
    cmp         eax,dword ptr [esp+28h]
    je L_67648
    mov         edx,dword ptr [ebp+18h]
    shl         eax,4
    mov         dword ptr [eax+edi+0Ch],3F800000h
    mov         byte ptr [edx],1
    mov         edx,dword ptr [esp+48h]
L_67648:
    inc         ecx
    cmp         ecx,edx
    jl L_67620
L_6764d:
    mov         eax,dword ptr [esp+4Ch]
    test        eax,eax
    js L_67670
    movd ecx,xmm7
    mov         edx,dword ptr [ecx]
    and         eax,7FFFFFFFh
    push        12h
    shl         eax,2
    push        eax
    mov         eax,dword ptr [esp+4Ch]
    push        eax
    call        dword ptr [edx+14h]
L_67670:
    mov         esi,dword ptr [ebp+8]
L_67673:
    mov         eax,dword ptr [esp+3Ch]
    mov         ecx,dword ptr [esi+8]
    inc         eax
    cmp         eax,ecx
    mov         dword ptr [esp+3Ch],eax
    jl L_67371
    xor         ecx,ecx
L_67689:
    mov         edx,dword ptr [esi+8]
    xor         eax,eax
    cmp         edx,ecx
    jle L_676a2
L_67692:
    mov         edx,dword ptr [esi+4]
    mov         word ptr [edx+eax*8+6],cx
    mov         edx,dword ptr [esi+8]
    inc         eax
    cmp         eax,edx
    jl L_67692
L_676a2:
    mov         ecx,dword ptr [ebp+10h]
    mov         eax,dword ptr [ebp+0Ch]
    mov         edx,ecx
    sub         edx,eax
    cmp         edx,2
    jle L_67a7c
    cmp         dword ptr [esi+8],2
    jle L_67a7c
    inc         ecx
    cmp         eax,ecx
    mov         dword ptr [esp+24h],eax
    mov         dword ptr [esp+34h],ecx
    jge L_67a7c
    shl         eax,4
    lea         eax,[eax+edi+0Ch]
    mov         dword ptr [esp+20h],eax
    jmp L_676e0
    lea         ecx,[ecx]
L_676e0:
    fld         dword ptr [K_888a8]
    fld         dword ptr [eax]
    fucompp
    fnstsw      ax
    test        ah,44h
    jp L_67a5c
    mov         ecx,dword ptr [esi+8]
    xor         eax,eax
    xor         edx,edx
    cmp         ecx,eax
    mov         dword ptr [esp+44h],eax
    mov         dword ptr [esp+48h],edx
    mov         dword ptr [esp+4Ch],80000000h
    mov         dword ptr [esp+3Ch],eax
    jle L_67a39
    jmp L_67720
    lea         ebx,[ebx+00000000h]
L_67720:
    mov         ecx,dword ptr [esi+4]
    cmp         word ptr [ecx+eax*8+6],1
    lea         ecx,[ecx+eax*8]
    je L_678e3
    movzx       ecx,word ptr [ecx]
    cmp         ecx,dword ptr [esp+24h]
    jne L_678e3
    mov         edx,dword ptr [esi+4]
    mov         word ptr [edx+eax*8+6],1
    mov         ecx,dword ptr [esi+4]
    movzx       edx,word ptr [ecx+eax*8+2]
    lea         esi,[ebx+edx*8]
    mov         dword ptr [esp+38h],esi
    jmp L_67760
    lea         ebx,[ebx+00000000h]
L_67760:
    movzx       eax,word ptr [esi+4]
    mov         word ptr [ebx+eax*8+6],1
    movzx       edx,word ptr [esi+4]
    movzx       eax,word ptr [ebx+edx*8]
    movzx       ecx,word ptr [esi]
    lea         edx,[ebx+edx*8]
    movzx       edx,word ptr [edx+4]
    movzx       edx,word ptr [ebx+edx*8]
    shl         ecx,4
    add         ecx,edi
    shl         eax,4
    add         eax,edi
    fld         dword ptr [ecx]
    shl         edx,4
    add         edx,edi
    fsub        dword ptr [eax]
    mov         edi,dword ptr [esp+48h]
    mov         ebx,edi
    inc         edi
    fstp        dword ptr [esp+70h]
    fld         dword ptr [ecx+4]
    fsub        dword ptr [eax+4]
    fstp        dword ptr [esp+74h]
    fld         dword ptr [ecx+8]
    fsub        dword ptr [eax+8]
    fstp        dword ptr [esp+78h]
    fld         dword ptr [edx]
    fsub        dword ptr [eax]
    fstp        dword ptr [esp+60h]
    fld         dword ptr [edx+4]
    fsub        dword ptr [eax+4]
    fstp        dword ptr [esp+64h]
    fld         dword ptr [edx+8]
    fsub        dword ptr [eax+8]
    mov         eax,dword ptr [esp+4Ch]
    and         eax,7FFFFFFFh
    cmp         eax,edi
    fstp        dword ptr [esp+68h]
    jge L_677f4
    add         eax,eax
    cmp         edi,eax
    jl L_677e4
    mov         eax,edi
L_677e4:
    push        10h
    push        eax
    lea         eax,[esp+4Ch]
    push        eax
    call L_73590
    add         esp,0Ch
L_677f4:
    fld         dword ptr [esp+68h]
    mov         eax,dword ptr [esp+44h]
    fmul        dword ptr [esp+74h]
    shl         ebx,4
    fld         dword ptr [esp+64h]
    add         ebx,eax
    fmul        dword ptr [esp+78h]
    mov         ecx,ebx
    mov         dword ptr [esp+48h],edi
    mov         dword ptr [ecx+0Ch],0
    fsubp       st(1),st
    fst         dword ptr [ecx]
    fld         dword ptr [esp+60h]
    fmul        dword ptr [esp+78h]
    fld         dword ptr [esp+68h]
    fmul        dword ptr [esp+70h]
    fsubp       st(1),st
    fstp        dword ptr [esp+18h]
    fld         dword ptr [esp+64h]
    mov         edx,dword ptr [esp+18h]
    fmul        dword ptr [esp+70h]
    mov         dword ptr [ecx+4],edx
    fld         dword ptr [esp+60h]
    fmul        dword ptr [esp+74h]
    fsubp       st(1),st
    fstp        dword ptr [esp+14h]
    fld         dword ptr [esp+14h]
    mov         eax,dword ptr [esp+14h]
    fmul        dword ptr [esp+14h]
    mov         dword ptr [ecx+8],eax
    fld         dword ptr [esp+18h]
    fmul        dword ptr [esp+18h]
    faddp       st(1),st
    fld         st(1)
    fmul        st,st(2)
    faddp       st(1),st
    fld         dword ptr [K_888a8]
    fld         st(1)
    fucompp
    fnstsw      ax
    test        ah,44h
    jnp L_6788b
    fsqrt
    fdivr       dword ptr [K_88e5c]
    jmp L_67893
L_6788b:
    fstp        st(0)
    fld         dword ptr [K_888a8]
L_67893:
    fld         st(0)
    mov         eax,dword ptr [esp+2Ch]
    fmul        st,st(2)
    mov         ebx,dword ptr [esp+2Ch]
    mov         edi,dword ptr [esp+40h]
    fstp        dword ptr [ecx]
    fld         dword ptr [esp+18h]
    fmul        st,st(1)
    fstp        dword ptr [ecx+4]
    fld         dword ptr [esp+14h]
    fmul        st,st(1)
    fstp        dword ptr [ecx+8]
    fmul        dword ptr [K_888a8]
    fstp        dword ptr [ecx+0Ch]
    movzx       ecx,word ptr [esi+4]
    movzx       edx,word ptr [eax+ecx*8+2]
    fstp        st(0)
    lea         esi,[eax+edx*8]
    cmp         esi,dword ptr [esp+38h]
    jne L_67760
    mov         edx,dword ptr [esp+48h]
    mov         eax,dword ptr [esp+3Ch]
    mov         esi,dword ptr [ebp+8]
L_678e3:
    mov         ecx,dword ptr [esi+8]
    inc         eax
    cmp         eax,ecx
    mov         dword ptr [esp+3Ch],eax
    jl L_67720
    test        edx,edx
    jle L_67a39
    fld         dword ptr [K_888a8]
    mov         eax,dword ptr [esp+44h]
    fld         dword ptr [K_888a8]
    add         eax,8
    fst         dword ptr [esp+54h]
    mov         ecx,edx
    fld         dword ptr [K_888a8]
    fst         dword ptr [esp+58h]
    fld         dword ptr [K_888a8]
L_67924:
    fxch        st(3)
    add         eax,10h
    dec         ecx
    fadd        dword ptr [eax-18h]
    fxch        st(3)
    fxch        st(2)
    fadd        dword ptr [eax-14h]
    fxch        st(2)
    fxch        st(1)
    fadd        dword ptr [eax-10h]
    fxch        st(1)
    fadd        dword ptr [eax-0Ch]
    jne L_67924
    fxch        st(1)
    mov         ecx,dword ptr [ebp+14h]
    fst         dword ptr [esp+58h]
    fxch        st(2)
    fst         dword ptr [esp+54h]
    fstp        st(1)
    fxch        st(1)
    fld         st(2)
    fmul        st,st(3)
    fld         st(1)
    fmul        st,st(2)
    faddp       st(1),st
    fld         st(2)
    fmul        st,st(3)
    faddp       st(1),st
    fstp        st(2)
    fstp        st(0)
    fld         st(0)
    fcomp       dword ptr [ecx+0Ch]
    fnstsw      ax
    test        ah,41h
    jne L_67a35
    fld         dword ptr [K_888a8]
    fld         st(1)
    fucompp
    fnstsw      ax
    test        ah,44h
    jnp L_67998
    fsqrt
    fdivr       dword ptr [K_88e5c]
    fstp        dword ptr [esp+1Ch]
    jmp L_679a2
L_67998:
    fstp        st(0)
    mov         dword ptr [esp+1Ch],0
L_679a2:
    test        edx,edx
    fld         dword ptr [esp+1Ch]
    fmul        st,st(1)
    mov         dword ptr [esp+28h],0
    fstp        dword ptr [esp+50h]
    fstp        st(0)
    fld         dword ptr [esp+54h]
    fmul        dword ptr [esp+1Ch]
    fstp        dword ptr [esp+54h]
    fld         dword ptr [esp+58h]
    fmul        dword ptr [esp+1Ch]
    fstp        dword ptr [esp+58h]
    jle L_67a23
    fld         dword ptr [K_88e5c]
    fsub        dword ptr [ecx+0Ch]
    mov         ecx,dword ptr [esp+44h]
    add         ecx,4
    fstp        dword ptr [esp+3Ch]
    jmp L_679f0
    lea         esp,[esp+00000000h]
    nop
L_679f0:
    fld         dword ptr [esp+58h]
    fmul        dword ptr [ecx+4]
    fld         dword ptr [esp+50h]
    fmul        dword ptr [ecx-4]
    faddp       st(1),st
    fld         dword ptr [esp+54h]
    fmul        dword ptr [ecx]
    faddp       st(1),st
    fcomp       dword ptr [esp+3Ch]
    fnstsw      ax
    test        ah,5
    jnp L_67a39
    mov         eax,dword ptr [esp+28h]
    inc         eax
    add         ecx,10h
    cmp         eax,edx
    mov         dword ptr [esp+28h],eax
    jl L_679f0
L_67a23:
    mov         eax,dword ptr [esp+20h]
    mov         ecx,dword ptr [ebp+18h]
    mov         dword ptr [eax],3F800000h
    mov         byte ptr [ecx],1
    jmp L_67a39
L_67a35:
    fstp        st(0)
    fstp        st(0)
L_67a39:
    mov         eax,dword ptr [esp+4Ch]
    test        eax,eax
    js L_67a5c
    movd ecx,xmm7
    mov         edx,dword ptr [ecx]
    and         eax,7FFFFFFFh
    push        12h
    shl         eax,4
    push        eax
    mov         eax,dword ptr [esp+4Ch]
    push        eax
    call        dword ptr [edx+14h]
L_67a5c:
    mov         ecx,dword ptr [esp+24h]
    mov         eax,dword ptr [esp+20h]
    mov         edx,dword ptr [esp+34h]
    inc         ecx
    add         eax,10h
    cmp         ecx,edx
    mov         dword ptr [esp+24h],ecx
    mov         dword ptr [esp+20h],eax
    jl L_676e0
L_67a7c:
    pop         edi
    pop         esi
    pop         ebx
    mov         esp,ebp
    pop         ebp
    ret
L_67a90:
    push        ebp
    mov         ebp,esp
    and         esp,0FFFFFFF0h
    sub         esp,0C4h
    mov         ecx,dword ptr [ebp+10h]
    mov         edx,dword ptr [ecx]
    mov         eax,dword ptr [ecx+4]
    push        ebx
    mov         dword ptr [esp+5Ch],edx
    mov         edx,dword ptr [ecx+8]
    mov         dword ptr [esp+8],eax
    xor         eax,eax
    test        edx,edx
    push        esi
    push        edi
    mov         dword ptr [esp+14h],eax
    jle L_67e42
L_67ac0:
    mov         edx,dword ptr [ecx+4]
    mov         esi,dword ptr [edx+eax*8]
    movzx       ebx,word ptr [edx+eax*8+4]
    cmp         eax,ebx
    mov         edx,dword ptr [esp+10h]
    movzx       edx,word ptr [edx+ebx*8+4]
    mov         dword ptr [esp+000000C8h],esi
    mov         dword ptr [esp+0Ch],edx
    jge L_67e32
    cmp         eax,edx
    jge L_67e32
    mov         edi,dword ptr [ebp+18h]
    mov         esi,dword ptr [edi+4]
    mov         eax,dword ptr [edi+8]
    lea         edx,[esi+1]
    and         eax,7FFFFFFFh
    cmp         eax,edx
    jge L_67b1b
    add         eax,eax
    cmp         edx,eax
    jl L_67b0c
    mov         eax,edx
L_67b0c:
    push        10h
    push        eax
    push        edi
    call L_73590
    mov         ecx,dword ptr [ebp+10h]
    add         esp,0Ch
L_67b1b:
    lea         eax,[esi+1]
    mov         dword ptr [edi+4],eax
    mov         eax,dword ptr [edi]
    mov         ecx,dword ptr [ecx]
    shl         esi,4
    add         esi,eax
    movzx       eax,word ptr [esp+000000C8h]
    shl         eax,4
    mov         edx,dword ptr [eax+ecx+4]
    add         eax,ecx
    mov         dword ptr [esp+44h],edx
    mov         edx,dword ptr [esp+10h]
    fld         dword ptr [eax]
    mov         eax,dword ptr [eax+8]
    mov         dword ptr [esp+48h],eax
    movzx       eax,word ptr [edx+ebx*8]
    shl         eax,4
    add         eax,ecx
    lea         edx,[edx+ebx*8]
    mov         ebx,dword ptr [esp+10h]
    fld         dword ptr [eax]
    mov         dword ptr [esp+000000ACh],edx
    fld         dword ptr [eax+4]
    mov         eax,dword ptr [eax+8]
    mov         dword ptr [esp+000000B8h],eax
    mov         eax,dword ptr [esp+0Ch]
    lea         eax,[ebx+eax*8]
    mov         dword ptr [esp+0000008Ch],eax
    movzx       eax,word ptr [eax]
    shl         eax,4
    add         eax,ecx
    mov         ecx,dword ptr [eax+4]
    mov         dword ptr [esp+24h],ecx
    fld         dword ptr [eax]
    mov         eax,dword ptr [eax+8]
    fld         st(3)
    mov         dword ptr [esp+28h],eax
    fsub        st,st(3)
    fld         dword ptr [esp+44h]
    fsub        st,st(3)
    fstp        dword ptr [esp+74h]
    fld         dword ptr [esp+48h]
    fsub        dword ptr [esp+000000B8h]
    fld         st(1)
    fmul        st,st(2)
    fld         st(1)
    fmul        st,st(2)
    faddp       st(1),st
    fld         dword ptr [esp+74h]
    fmul        dword ptr [esp+74h]
    faddp       st(1),st
    fstp        dword ptr [esp+0Ch]
    fld         dword ptr [K_888a8]
    fld         dword ptr [esp+0Ch]
    fucompp
    fnstsw      ax
    test        ah,44h
    jnp L_67bea
    fld         dword ptr [esp+0Ch]
    fsqrt
    fdivr       dword ptr [K_88e5c]
    jmp L_67bf0
L_67bea:
    fld         dword ptr [K_888a8]
L_67bf0:
    fld         st(0)
    fmul        st,st(3)
    fstp        dword ptr [esp+70h]
    fld         dword ptr [esp+74h]
    fmul        st,st(1)
    fstp        dword ptr [esp+74h]
    fxch        st(1)
    fmul        st,st(1)
    fstp        dword ptr [esp+78h]
    fstp        st(0)
    fstp        st(0)
    fsub        st,st(2)
    fstp        dword ptr [esp+00000090h]
    fld         dword ptr [esp+24h]
    fsub        st,st(1)
    fstp        dword ptr [esp+00000094h]
    fstp        st(0)
    fstp        st(0)
    fld         dword ptr [esp+28h]
    fsub        dword ptr [esp+000000B8h]
    fstp        dword ptr [esp+00000098h]
    fld         dword ptr [esp+00000090h]
    fmul        dword ptr [esp+00000090h]
    fld         dword ptr [esp+00000098h]
    fmul        dword ptr [esp+00000098h]
    faddp       st(1),st
    fld         dword ptr [esp+00000094h]
    fmul        dword ptr [esp+00000094h]
    faddp       st(1),st
    fld         dword ptr [K_888a8]
    fld         st(1)
    fucompp
    fnstsw      ax
    test        ah,44h
    jnp L_67c83
    fsqrt
    fdivr       dword ptr [K_88e5c]
    jmp L_67c8b
L_67c83:
    fstp        st(0)
    fld         dword ptr [K_888a8]
L_67c8b:
    fld         dword ptr [esp+00000090h]
    mov         eax,dword ptr [ebp+0Ch]
    fmul        st,st(1)
    mov         dword ptr [esi+0Ch],0
    fld         dword ptr [esp+00000094h]
    fmul        st,st(2)
    fld         dword ptr [esp+00000098h]
    fmul        st,st(3)
    fld         dword ptr [esp+74h]
    fmul        st,st(1)
    fld         st(2)
    fmul        dword ptr [esp+78h]
    fsubp       st(1),st
    fst         dword ptr [esp+18h]
    fstp        dword ptr [esi]
    fld         dword ptr [esp+78h]
    fmul        st,st(3)
    fxch        st(1)
    fmul        dword ptr [esp+70h]
    fsubp       st(1),st
    fst         dword ptr [esp+0Ch]
    fstp        dword ptr [esi+4]
    fmul        dword ptr [esp+70h]
    fld         dword ptr [esp+74h]
    fmul        st,st(2)
    fsubp       st(1),st
    fstp        dword ptr [esp+1Ch]
    mov         ecx,dword ptr [esp+1Ch]
    fstp        st(0)
    mov         dword ptr [esi+8],ecx
    fstp        st(0)
    fld         dword ptr [esp+1Ch]
    fmul        dword ptr [esp+1Ch]
    fld         dword ptr [esp+0Ch]
    fmul        dword ptr [esp+0Ch]
    faddp       st(1),st
    fld         dword ptr [esp+18h]
    fmul        dword ptr [esp+18h]
    faddp       st(1),st
    fld         st(0)
    fcomp       dword ptr [eax+8]
    fnstsw      ax
    test        ah,5
    jp L_67d4c
    mov         esi,dword ptr [edi+4]
    fstp        st(0)
    mov         eax,dword ptr [edi+8]
    fstp        st(0)
    dec         esi
    and         eax,7FFFFFFFh
    cmp         eax,esi
    jge L_67d44
    add         eax,eax
    cmp         esi,eax
    jl L_67d38
    mov         eax,esi
L_67d38:
    push        10h
    push        eax
    push        edi
    call L_73590
    add         esp,0Ch
L_67d44:
    mov         dword ptr [edi+4],esi
    jmp L_67e2b
L_67d4c:
    fld         dword ptr [K_888a8]
    fld         st(1)
    fucompp
    fnstsw      ax
    test        ah,44h
    jnp L_67d67
    fsqrt
    fdivr       dword ptr [K_88e5c]
    jmp L_67d6f
L_67d67:
    fstp        st(0)
    fld         dword ptr [K_888a8]
L_67d6f:
    fld         dword ptr [esp+18h]
    mov         ebx,dword ptr [ebp+1Ch]
    fmul        st,st(1)
    fst         dword ptr [esi]
    fld         st(1)
    fmul        dword ptr [esp+0Ch]
    fst         dword ptr [esi+4]
    fld         st(2)
    fmul        dword ptr [esp+1Ch]
    fst         dword ptr [esi+8]
    fxch        st(3)
    fmul        dword ptr [K_888a8]
    fstp        dword ptr [esi+0Ch]
    fxch        st(2)
    fmul        dword ptr [esp+48h]
    fxch        st(2)
    fmul        dword ptr [esp+44h]
    faddp       st(2),st
    fmul        st,st(2)
    faddp       st(1),st
    fchs
    fstp        dword ptr [esi+0Ch]
    mov         edi,dword ptr [ebx+4]
    mov         ecx,dword ptr [ebx+8]
    and         ecx,7FFFFFFFh
    fstp        st(0)
    lea         eax,[edi+1]
    cmp         ecx,eax
    mov         dword ptr [esp+0Ch],ecx
    jge L_67de2
    add         ecx,ecx
    cmp         eax,ecx
    jge L_67dcf
    mov         eax,ecx
L_67dcf:
    push        20h
    push        eax
    push        ebx
    call L_73590
    mov         edx,dword ptr [esp+000000B8h]
    add         esp,0Ch
L_67de2:
    lea         eax,[edi+1]
    mov         dword ptr [ebx+4],eax
    mov         eax,dword ptr [ebx]
    shl         edi,5
    add         edi,eax
    mov         eax,dword ptr [esi]
    mov         ecx,edi
    mov         dword ptr [ecx],eax
    mov         eax,dword ptr [esi+4]
    mov         dword ptr [ecx+4],eax
    mov         eax,dword ptr [esi+8]
    mov         dword ptr [ecx+8],eax
    mov         eax,dword ptr [esi+0Ch]
    mov         dword ptr [ecx+0Ch],eax
    mov         eax,dword ptr [ebp+10h]
    mov         esi,dword ptr [eax+4]
    mov         eax,dword ptr [esp+14h]
    shl         eax,3
    mov         dword ptr [ecx+14h],edx
    mov         edx,dword ptr [esp+0000008Ch]
    add         esi,eax
    mov         dword ptr [ecx+10h],esi
    mov         dword ptr [ecx+18h],edx
    call L_667b0
L_67e2b:
    mov         eax,dword ptr [esp+14h]
    mov         ecx,dword ptr [ebp+10h]
L_67e32:
    mov         edx,dword ptr [ecx+8]
    inc         eax
    cmp         eax,edx
    mov         dword ptr [esp+14h],eax
    jl L_67ac0
L_67e42:
    mov         esi,dword ptr [ebp+18h]
    mov         edx,dword ptr [esi+4]
    mov         ecx,dword ptr [ebp+20h]
    test        edx,edx
    sete        al
    mov         byte ptr [ecx],al
    mov         eax,dword ptr [ebp+1Ch]
    mov         ecx,dword ptr [eax+4]
    test        ecx,ecx
    jle L_681b2
    mov         ecx,1
    xor         ebx,ebx
    mov         dword ptr [esp+14h],ecx
    jmp L_67e70
    lea         ecx,[ecx]
L_67e70:
    cmp         ecx,dword ptr [eax+4]
    mov         dword ptr [esp+18h],ecx
    jge L_6819c
    lea         ecx,[ebx+20h]
    mov         dword ptr [esp+0Ch],ecx
L_67e84:
    mov         eax,dword ptr [eax]
    fld         dword ptr [ebx+eax]
    mov         edx,dword ptr [ebx+eax+10h]
    fld         dword ptr [ebx+eax+4]
    mov         esi,dword ptr [ebx+eax+14h]
    fld         dword ptr [ebx+eax+8]
    mov         edi,dword ptr [ebx+eax+18h]
    fstp        dword ptr [esp+48h]
    mov         dword ptr [esp+50h],edx
    fld         dword ptr [ebx+eax+0Ch]
    mov         dx,word ptr [ebx+eax+1Ch]
    fstp        dword ptr [esp+4Ch]
    mov         word ptr [esp+5Ch],dx
    fld         dword ptr [ecx+eax]
    mov         edx,dword ptr [ecx+eax+10h]
    fstp        dword ptr [esp+20h]
    mov         dword ptr [esp+30h],edx
    fld         dword ptr [ecx+eax+4]
    mov         edx,dword ptr [ecx+eax+14h]
    fstp        dword ptr [esp+24h]
    mov         dword ptr [esp+54h],esi
    fld         dword ptr [ecx+eax+8]
    mov         eax,dword ptr [ecx+eax+18h]
    fstp        dword ptr [esp+28h]
    mov         dword ptr [esp+38h],eax
    fld         dword ptr [esp+20h]
    mov         eax,dword ptr [ebp+0Ch]
    fadd        st,st(2)
    mov         dword ptr [esp+58h],edi
    mov         dword ptr [esp+34h],edx
    fstp        dword ptr [esp+000000B0h]
    fld         dword ptr [esp+24h]
    fadd        st,st(1)
    fstp        st(2)
    fstp        st(0)
    fld         dword ptr [esp+28h]
    fadd        dword ptr [esp+48h]
    fld         dword ptr [eax+18h]
    fld         st(1)
    fmul        st,st(2)
    fld         st(3)
    fmul        st,st(4)
    faddp       st(1),st
    fld         dword ptr [esp+000000B0h]
    fmul        dword ptr [esp+000000B0h]
    faddp       st(1),st
    fld         st(1)
    fmul        st,st(2)
    fcompp
    fstp        st(0)
    fnstsw      ax
    fstp        st(0)
    test        ah,41h
    fstp        st(0)
    jne L_68177
    fld         dword ptr [esp+20h]
    lea         ecx,[esp+20h]
    push        ecx
    fchs
    mov         ecx,dword ptr [esp+34h]
    fstp        dword ptr [esp+44h]
    fld         dword ptr [esp+28h]
    lea         eax,[esp+44h]
    push        eax
    fchs
    mov         eax,dword ptr [esp+18h]
    fstp        dword ptr [esp+4Ch]
    fld         dword ptr [esp+30h]
    push        edx
    mov         edx,dword ptr [esp+5Ch]
    fchs
    push        esi
    fstp        dword ptr [esp+58h]
    push        ecx
    push        edx
    push        eax
    lea         ecx,[esp+0000008Bh]
    push        ecx
    call L_65060
    mov         cl,byte ptr [eax]
    add         esp,20h
    test        cl,cl
    jne L_67fc8
    mov         ecx,dword ptr [esp+38h]
    lea         edx,[esp+20h]
    push        edx
    mov         edx,dword ptr [esp+34h]
    lea         eax,[esp+44h]
    push        eax
    mov         eax,dword ptr [esp+58h]
    push        ecx
    mov         ecx,dword ptr [esp+1Ch]
    push        esi
    push        edx
    push        eax
    push        ecx
    lea         edx,[esp+00000089h]
    push        edx
    call L_65060
    mov         cl,byte ptr [eax]
    add         esp,20h
    test        cl,cl
    je L_68002
L_67fc8:
    mov         eax,dword ptr [ebp+20h]
    mov         ecx,dword ptr [ebp+18h]
    push        ecx
    mov         byte ptr [eax],1
    movzx       edx,word ptr [edi]
    mov         eax,dword ptr [esp+68h]
    movzx       ecx,word ptr [esi]
    shl         edx,4
    shl         ecx,4
    add         edx,eax
    push        edx
    mov         edx,dword ptr [esp+58h]
    add         ecx,eax
    push        ecx
    movzx       ecx,word ptr [edx]
    shl         ecx,4
    add         ecx,eax
    push        ecx
    lea         edx,[esp+50h]
    push        edx
    call L_66a40
    add         esp,14h
L_68002:
    mov         edx,dword ptr [esp+34h]
    lea         eax,[esp+20h]
    push        eax
    mov         eax,dword ptr [esp+34h]
    lea         ecx,[esp+44h]
    push        ecx
    mov         ecx,dword ptr [esp+58h]
    push        edx
    mov         edx,dword ptr [esp+1Ch]
    push        edi
    push        eax
    push        ecx
    push        edx
    lea         eax,[esp+00000087h]
    push        eax
    call L_65060
    mov         cl,byte ptr [eax]
    add         esp,20h
    test        cl,cl
    jne L_6806c
    mov         eax,dword ptr [esp+38h]
    lea         ecx,[esp+20h]
    push        ecx
    mov         ecx,dword ptr [esp+34h]
    lea         edx,[esp+44h]
    push        edx
    mov         edx,dword ptr [esp+58h]
    push        eax
    mov         eax,dword ptr [esp+1Ch]
    push        edi
    push        ecx
    push        edx
    push        eax
    lea         ecx,[esp+00000085h]
    push        ecx
    call L_65060
    mov         cl,byte ptr [eax]
    add         esp,20h
    test        cl,cl
    je L_680a6
L_6806c:
    mov         edx,dword ptr [ebp+20h]
    mov         eax,dword ptr [ebp+18h]
    mov         byte ptr [edx],1
    movzx       ecx,word ptr [esi]
    movzx       edx,word ptr [edi]
    push        eax
    mov         eax,dword ptr [esp+68h]
    shl         ecx,4
    shl         edx,4
    add         ecx,eax
    add         edx,eax
    push        ecx
    mov         ecx,dword ptr [esp+58h]
    push        edx
    movzx       edx,word ptr [ecx]
    shl         edx,4
    add         edx,eax
    push        edx
    lea         eax,[esp+50h]
    push        eax
    call L_66a40
    add         esp,14h
L_680a6:
    mov         eax,dword ptr [esp+34h]
    lea         ecx,[esp+20h]
    push        ecx
    mov         ecx,dword ptr [esp+34h]
    lea         edx,[esp+44h]
    push        edx
    mov         edx,dword ptr [esp+18h]
    push        eax
    push        edi
    push        ecx
    push        esi
    push        edx
    lea         eax,[esp+00000086h]
    push        eax
    call L_65060
    mov         cl,byte ptr [eax]
    add         esp,20h
    test        cl,cl
    jne L_68139
    mov         eax,dword ptr [esp+38h]
    lea         ecx,[esp+20h]
    push        ecx
    mov         ecx,dword ptr [esp+34h]
    lea         edx,[esp+44h]
    push        edx
    mov         edx,dword ptr [esp+18h]
    push        eax
    push        edi
    push        ecx
    push        esi
    push        edx
    lea         eax,[esp+00000088h]
    push        eax
    call L_65060
    mov         cl,byte ptr [eax]
    add         esp,20h
    test        cl,cl
    jne L_68139
    mov         eax,dword ptr [esp+38h]
    lea         ecx,[esp+20h]
    push        ecx
    mov         ecx,dword ptr [esp+38h]
    lea         edx,[esp+44h]
    push        edx
    mov         edx,dword ptr [esp+18h]
    push        eax
    push        edi
    push        ecx
    push        esi
    push        edx
    lea         eax,[esp+0000008Ah]
    push        eax
    call L_65060
    mov         cl,byte ptr [eax]
    add         esp,20h
    test        cl,cl
    je L_68173
L_68139:
    mov         ecx,dword ptr [ebp+20h]
    mov         eax,dword ptr [esp+50h]
    mov         edx,dword ptr [ebp+18h]
    mov         byte ptr [ecx],1
    movzx       ecx,word ptr [eax]
    mov         eax,dword ptr [esp+64h]
    push        edx
    movzx       edx,word ptr [edi]
    shl         ecx,4
    add         ecx,eax
    push        ecx
    movzx       ecx,word ptr [esi]
    shl         edx,4
    add         edx,eax
    push        edx
    shl         ecx,4
    add         ecx,eax
    push        ecx
    lea         edx,[esp+50h]
    push        edx
    call L_66a40
    add         esp,14h
L_68173:
    mov         ecx,dword ptr [esp+0Ch]
L_68177:
    mov         edx,dword ptr [esp+18h]
    mov         eax,dword ptr [ebp+1Ch]
    mov         esi,dword ptr [eax+4]
    inc         edx
    add         ecx,20h
    cmp         edx,esi
    mov         dword ptr [esp+18h],edx
    mov         dword ptr [esp+0Ch],ecx
    jl L_67e84
    mov         esi,dword ptr [ebp+18h]
    mov         ecx,dword ptr [esp+14h]
L_6819c:
    mov         edi,dword ptr [eax+4]
    inc         ecx
    lea         edx,[ecx-1]
    add         ebx,20h
    cmp         edx,edi
    mov         dword ptr [esp+14h],ecx
    jl L_67e70
L_681b2:
    mov         eax,dword ptr [esi+4]
    cmp         eax,1
    jle L_681ce
    push        offset L_65280
    dec         eax
    push        eax
    mov         eax,dword ptr [esi]
    push        0
    push        eax
    call L_65380
    add         esp,10h
L_681ce:
    mov         edx,dword ptr [ebp+0Ch]
    mov         eax,dword ptr [edx+10h]
    lea         ecx,[esp+0000008Ch]
    push        ecx
    push        esi
    push        eax
    call L_66800
    mov         eax,dword ptr [ebp+8]
    add         esp,0Ch
    pop         edi
    pop         esi
    mov         byte ptr [eax],1
    pop         ebx
    mov         esp,ebp
    pop         ebp
    ret
L_68200:
    push        esi
    mov         esi,ecx
    mov         ecx,dword ptr [esi+8]
    push        edi
    mov         edi,dword ptr [esp+0Ch]
    mov         edx,dword ptr [edi+4]
    mov         eax,ecx
    and         eax,7FFFFFFFh
    cmp         eax,edx
    jge L_6824d
    test        ecx,ecx
    js L_68231
    movd ecx,xmm7
    mov         edx,dword ptr [ecx]
    push        12h
    shl         eax,3
    push        eax
    mov         eax,dword ptr [esi]
    push        eax
    call        dword ptr [edx+14h]
L_68231:
    mov         eax,dword ptr [edi+4]
    movd ecx,xmm7
    mov         edx,dword ptr [ecx]
    push        12h
    shl         eax,3
    push        eax
    call        dword ptr [edx+10h]
    mov         dword ptr [esi],eax
    mov         ecx,dword ptr [edi+4]
    mov         dword ptr [esi+8],ecx
L_6824d:
    mov         edx,dword ptr [edi+4]
    mov         eax,dword ptr [esi]
    mov         dword ptr [esi+4],edx
    mov         edx,dword ptr [edi+4]
    test        edx,edx
    mov         ecx,dword ptr [edi]
    jle L_68274
    sub         ecx,eax
    push        ebx
L_68261:
    mov         ebx,dword ptr [ecx+eax]
    mov         dword ptr [eax],ebx
    mov         ebx,dword ptr [ecx+eax+4]
    mov         dword ptr [eax+4],ebx
    add         eax,8
    dec         edx
    jne L_68261
    pop         ebx
L_68274:
    lea         eax,[esi+0Ch]
    sub         edi,esi
    mov         ecx,80h
    mov         edi,edi
L_68280:
    mov         edx,dword ptr [edi+eax]
    mov         dword ptr [eax],edx
    mov         edx,dword ptr [edi+eax+4]
    mov         dword ptr [eax+4],edx
    add         eax,8
    dec         ecx
    jne L_68280
    pop         edi
    mov         eax,esi
    pop         esi
    ret         4
L_682a0:
    push        ebx
    push        ebp
    mov         ebp,dword ptr [esp+18h]
    mov         eax,dword ptr [ebp+8]
    push        esi
    mov         esi,dword ptr [esp+10h]
    push        edi
    mov         edi,dword ptr [esi+8]
    and         eax,7FFFFFFFh
    cmp         eax,edi
    jge L_682cf
    add         eax,eax
    cmp         edi,eax
    jl L_682c3
    mov         eax,edi
L_682c3:
    push        2
    push        eax
    push        ebp
    call L_73590
    add         esp,0Ch
L_682cf:
    mov         ebx,dword ptr [esp+1Ch]
    mov         dword ptr [ebp+4],edi
    mov         eax,dword ptr [esi+8]
    mov         ecx,dword ptr [ebx+8]
    xor         edi,edi
    test        eax,eax
    mov         dword ptr [esp+1Ch],ecx
    jle L_68360
L_682e6:
    mov         eax,dword ptr [esi+4]
    lea         ecx,[eax+edi*8]
    mov         ax,word ptr [ecx+6]
    cmp         ax,1
    mov         dword ptr [esp+20h],ecx
    je L_6830b
    cmp         ax,2
    je L_6830b
    mov         ecx,dword ptr [ebp]
    mov         word ptr [ecx+edi*2],0FFFFh
    jmp L_68354
L_6830b:
    mov         edx,dword ptr [ebp]
    mov         ax,word ptr [ebx+8]
    mov         word ptr [edx+edi*2],ax
    mov         edx,dword ptr [ebx+0Ch]
    mov         eax,dword ptr [ebx+8]
    lea         esi,[ebx+4]
    and         edx,7FFFFFFFh
    cmp         eax,edx
    jne L_68338
    push        8
    push        esi
    call L_73600
    mov         ecx,dword ptr [esp+28h]
    add         esp,8
L_68338:
    mov         ecx,dword ptr [ecx]
    mov         eax,dword ptr [esi+4]
    mov         edx,dword ptr [esi]
    mov         dword ptr [edx+eax*8],ecx
    mov         ecx,dword ptr [esp+20h]
    mov         ecx,dword ptr [ecx+4]
    mov         dword ptr [edx+eax*8+4],ecx
    inc         dword ptr [esi+4]
    mov         esi,dword ptr [esp+14h]
L_68354:
    mov         eax,dword ptr [esi+8]
    inc         edi
    cmp         edi,eax
    jl L_682e6
    mov         ecx,dword ptr [esp+1Ch]
L_68360:
    cmp         ecx,dword ptr [ebx+8]
    jge L_68391
L_68365:
    mov         edx,dword ptr [ebx+4]
    mov         esi,dword ptr [ebp]
    lea         eax,[edx+ecx*8]
    movzx       edx,word ptr [eax+2]
    mov         dx,word ptr [esi+edx*2]
    mov         word ptr [eax+2],dx
    movzx       edx,word ptr [eax+4]
    mov         esi,dword ptr [ebp]
    mov         dx,word ptr [esi+edx*2]
    mov         word ptr [eax+4],dx
    mov         eax,dword ptr [ebx+8]
    inc         ecx
    cmp         ecx,eax
    jl L_68365
L_68391:
    pop         edi
    pop         esi
    pop         ebp
    pop         ebx
    ret
L_683a0:
    push        ebx
    mov         ebx,dword ptr [esp+8]
    push        esi
    mov         esi,ecx
    mov         dword ptr [esi],0
    mov         dword ptr [esi+4],0
    mov         dword ptr [esi+8],80000000h
    movd ecx,xmm7
    mov         edx,dword ptr [ecx+24h]
    lea         eax,[ebx+ebx+0Fh]
    and         eax,0FFFFFFF0h
    cmp         eax,edx
    jle L_683d8
    mov         edx,dword ptr [ecx]
    push        eax
    call        dword ptr [edx+24h]
    jmp L_683ec
L_683d8:
    push        ebp
    push        edi
    mov         edi,dword ptr [ecx+20h]
    lea         ebp,[edi+eax]
    sub         edx,eax
    mov         eax,edi
    pop         edi
    mov         dword ptr [ecx+20h],ebp
    mov         dword ptr [ecx+24h],edx
    pop         ebp
L_683ec:
    mov         ecx,ebx
    mov         dword ptr [esi],eax
    or          ecx,80000000h
    mov         dword ptr [esi+0Ch],eax
    mov         dword ptr [esi+10h],ebx
    mov         dword ptr [esi+8],ecx
    mov         eax,esi
    pop         esi
    pop         ebx
    ret         4
L_68410:
    sub         esp,10h
    push        ebp
    push        edi
    mov         edi,dword ptr [esp+1Ch]
    mov         eax,dword ptr [edi+8]
    mov         ebp,dword ptr [edi+4]
    xor         edx,edx
    test        eax,eax
    mov         dword ptr [esp+8],edx
    jle L_684c7
    push        ebx
    push        esi
    nop
L_68430:
    mov         eax,dword ptr [edi+4]
    mov         esi,dword ptr [eax+edx*8]
    movzx       eax,word ptr [eax+edx*8+4]
    cmp         edx,eax
    jge L_684b5
    movzx       ecx,word ptr [ebp+eax*8+4]
    cmp         edx,ecx
    jge L_684b5
    movzx       eax,word ptr [ebp+eax*8]
    movzx       ecx,word ptr [ebp+ecx*8]
    movzx       esi,si
    mov         dword ptr [esp+14h],esi
    mov         esi,dword ptr [esp+28h]
    mov         ebx,dword ptr [esi+10h]
    add         esi,0Ch
    mov         dword ptr [esp+1Ch],eax
    mov         eax,dword ptr [esi+8]
    lea         edi,[ebx+1]
    and         eax,7FFFFFFFh
    cmp         eax,edi
    mov         dword ptr [esp+18h],ecx
    jge L_68492
    add         eax,eax
    cmp         edi,eax
    jl L_68482
    mov         eax,edi
L_68482:
    push        0Ch
    push        eax
    push        esi
    call L_73590
    mov         edx,dword ptr [esp+1Ch]
    add         esp,0Ch
L_68492:
    mov         ecx,dword ptr [esi]
    lea         eax,[ebx+ebx*2]
    lea         eax,[ecx+eax*4]
    mov         ecx,dword ptr [esp+14h]
    mov         dword ptr [esi+4],edi
    mov         edi,dword ptr [esp+24h]
    mov         dword ptr [eax],ecx
    mov         ecx,dword ptr [esp+18h]
    mov         dword ptr [eax+4],ecx
    mov         ecx,dword ptr [esp+1Ch]
    mov         dword ptr [eax+8],ecx
L_684b5:
    mov         eax,dword ptr [edi+8]
    inc         edx
    cmp         edx,eax
    mov         dword ptr [esp+10h],edx
    jl L_68430
    pop         esi
    pop         ebx
L_684c7:
    pop         edi
    pop         ebp
    add         esp,10h
    ret
L_684d0:
    sub         esp,14h
    mov         edx,dword ptr [esp+28h]
    xor         eax,eax
    push        esi
    mov         dword ptr [esp+0Ch],eax
    mov         dword ptr [esp+10h],eax
    lea         eax,[esp+8]
    push        eax
    mov         eax,dword ptr [esp+2Ch]
    lea         ecx,[esp+10h]
    push        ecx
    mov         ecx,dword ptr [esp+2Ch]
    push        edx
    mov         edx,dword ptr [esp+2Ch]
    push        eax
    push        ecx
    push        edx
    lea         eax,[esp+1Fh]
    push        eax
    mov         dword ptr [esp+30h],80000000h
    call L_67a90
    mov         eax,dword ptr [esp+30h]
    mov         esi,dword ptr [esp+38h]
    mov         cl,byte ptr [esp+24h]
    add         esp,1Ch
    test        eax,eax
    mov         byte ptr [esi],cl
    js L_6853e
    movd ecx,xmm7
    mov         edx,dword ptr [ecx]
    and         eax,7FFFFFFFh
    push        12h
    shl         eax,5
    push        eax
    mov         eax,dword ptr [esp+14h]
    push        eax
    call        dword ptr [edx+14h]
L_6853e:
    mov         eax,esi
    pop         esi
    add         esp,14h
    ret
L_68550:
    mov         ecx,dword ptr [esp+4]
    mov         edx,dword ptr [ecx+8]
    sub         esp,18h
    push        ebx
    mov         ebx,dword ptr [esp+24h]
    push        esi
    xor         eax,eax
    test        edx,edx
    push        edi
    jle L_68583
L_68567:
    mov         edx,dword ptr [ecx+4]
    movzx       edx,word ptr [edx+eax*8]
    mov         esi,dword ptr [ebx]
    shl         edx,4
    mov         dword ptr [edx+esi+0Ch],40000000h
    mov         edx,dword ptr [ecx+8]
    inc         eax
    cmp         eax,edx
    jl L_68567
L_68583:
    mov         esi,dword ptr [ebx+4]
    movd ecx,xmm7
    mov         eax,dword ptr [ecx+24h]
    lea         edx,[esi*4+0000000Fh]
    and         edx,0FFFFFFF0h
    cmp         edx,eax
    push        ebp
    mov         dword ptr [esp+10h],edx
    jle L_685ac
    mov         eax,dword ptr [ecx]
    push        edx
    call        dword ptr [eax+24h]
    mov         ebp,eax
    jmp L_685ba
L_685ac:
    mov         ebp,dword ptr [ecx+20h]
    lea         edi,[edx+ebp]
    sub         eax,edx
    mov         dword ptr [ecx+20h],edi
    mov         dword ptr [ecx+24h],eax
L_685ba:
    mov         eax,dword ptr [ebx+4]
    or          esi,80000000h
    mov         dword ptr [esp+1Ch],esi
    xor         edi,edi
    xor         esi,esi
    test        eax,eax
    mov         dword ptr [esp+30h],ebp
    jle L_6862b
    xor         ecx,ecx
    xor         edx,edx
    jmp L_685e0
    lea         esp,[esp+00000000h]
L_685e0:
    mov         eax,dword ptr [ebx]
    cmp         dword ptr [edx+eax+0Ch],40000000h
    jne L_68618
    mov         ebp,dword ptr [edx+eax]
    mov         dword ptr [ecx+eax],ebp
    mov         ebp,dword ptr [edx+eax+4]
    mov         dword ptr [ecx+eax+4],ebp
    mov         ebp,dword ptr [edx+eax+8]
    mov         dword ptr [ecx+eax+8],ebp
    mov         ebp,dword ptr [edx+eax+0Ch]
    mov         dword ptr [ecx+eax+0Ch],ebp
    mov         ebp,dword ptr [esp+30h]
    mov         dword ptr [ebp+esi*4],edi
    inc         edi
    add         ecx,10h
    jmp L_68620
L_68618:
    mov         dword ptr [ebp+esi*4],0FFFFFFFFh
L_68620:
    mov         eax,dword ptr [ebx+4]
    inc         esi
    add         edx,10h
    cmp         esi,eax
    jl L_685e0
L_6862b:
    mov         eax,dword ptr [ebx+8]
    and         eax,7FFFFFFFh
    cmp         eax,edi
    jge L_6864b
    add         eax,eax
    cmp         edi,eax
    jl L_6863f
    mov         eax,edi
L_6863f:
    push        10h
    push        eax
    push        ebx
    call L_73590
    add         esp,0Ch
L_6864b:
    mov         esi,dword ptr [esp+2Ch]
    mov         dword ptr [ebx+4],edi
    mov         ecx,dword ptr [esi+8]
    xor         eax,eax
    test        ecx,ecx
    jle L_6867a
    jmp L_68660
    lea         ecx,[ecx]
L_68660:
    mov         ecx,dword ptr [esi+4]
    movzx       edx,word ptr [ecx+eax*8]
    mov         dx,word ptr [ebp+edx*4]
    lea         ecx,[ecx+eax*8]
    mov         word ptr [ecx],dx
    mov         ecx,dword ptr [esi+8]
    inc         eax
    cmp         eax,ecx
    jl L_68660
L_6867a:
    mov         eax,dword ptr [esp+10h]
    test        eax,eax
    movd ecx,xmm7
    je L_6869c
    mov         edx,dword ptr [ecx+2Ch]
    cmp         ebp,edx
    je L_68693
    test        edx,edx
    jne L_6869c
L_68693:
    mov         edx,dword ptr [ecx]
    push        eax
    push        ebp
    call        dword ptr [edx+28h]
    jmp L_686ac
L_6869c:
    mov         edi,dword ptr [ecx+20h]
    mov         edx,dword ptr [ecx+24h]
    sub         edi,eax
    add         edx,eax
    mov         dword ptr [ecx+20h],edi
    mov         dword ptr [ecx+24h],edx
L_686ac:
    mov         eax,dword ptr [esp+1Ch]
    test        eax,eax
    js L_686cb
    movd ecx,xmm7
    mov         edx,dword ptr [ecx]
    and         eax,7FFFFFFFh
    push        12h
    shl         eax,2
    push        eax
    push        ebp
    call        dword ptr [edx+14h]
L_686cb:
    mov         edx,dword ptr [esi+8]
    xor         ecx,ecx
    xor         eax,eax
    cmp         edx,ecx
    pop         ebp
    jle L_686ef
L_686d7:
    mov         edx,dword ptr [esi+4]
    movzx       edx,word ptr [edx+eax*8]
    mov         edi,dword ptr [ebx]
    shl         edx,4
    mov         dword ptr [edx+edi+0Ch],ecx
    mov         edx,dword ptr [esi+8]
    inc         eax
    cmp         eax,edx
    jl L_686d7
L_686ef:
    pop         edi
    pop         esi
    pop         ebx
    add         esp,18h
    ret
L_68700:
    sub         esp,48h
    push        ebx
    mov         ebx,dword ptr [esp+5Ch]
    push        ebp
    push        esi
    push        edi
    mov         edi,dword ptr [esp+5Ch]
    mov         dword ptr [ebx+8],0
    mov         eax,dword ptr [edi+8]
    push        eax
    lea         ecx,[esp+48h]
    call L_683a0
    lea         eax,[esp+44h]
    push        eax
    push        ebx
    push        ecx
    mov         eax,esp
    push        edi
    mov         byte ptr [eax],1
    call L_682a0
    mov         esi,dword ptr [esp+70h]
    mov         eax,dword ptr [esi+8]
    add         esp,10h
    push        eax
    lea         ecx,[esp+34h]
    call L_683a0
    lea         ecx,[esp+30h]
    push        ecx
    push        ebx
    push        ecx
    mov         eax,esp
    push        esi
    mov         byte ptr [eax],0
    call L_682a0
    mov         ebp,dword ptr [esp+74h]
    mov         eax,dword ptr [ebp+4]
    mov         edx,dword ptr [ebx+8]
    lea         eax,[eax+eax*2]
    add         eax,edx
    mov         edx,dword ptr [ebx+0Ch]
    lea         esi,[ebx+4]
    and         edx,7FFFFFFFh
    add         esp,10h
    cmp         edx,eax
    jge L_68789
    push        8
    push        eax
    push        esi
    call L_73590
    add         esp,0Ch
L_68789:
    mov         ecx,dword ptr [ebp+4]
    xor         eax,eax
    cmp         ecx,eax
    mov         dword ptr [esp+5Ch],eax
    jle L_6883c
    mov         dword ptr [esp+68h],eax
    mov         edi,edi
L_687a0:
    mov         eax,dword ptr [ebp]
    add         eax,dword ptr [esp+68h]
    mov         ecx,dword ptr [eax]
    mov         dx,word ptr [ecx]
    mov         word ptr [eax+8],dx
    movzx       edx,word ptr [ecx+4]
    mov         ebp,dword ptr [edi+4]
    mov         dx,word ptr [ebp+edx*8]
    mov         word ptr [eax+0Ch],dx
    mov         edx,dword ptr [eax+4]
    mov         bp,word ptr [edx]
    mov         word ptr [eax+0Eh],bp
    mov         ebp,dword ptr [edi+4]
    movzx       edx,word ptr [edx+2]
    sub         ecx,ebp
    mov         ebp,dword ptr [esp+44h]
    sar         ecx,3
    movzx       ecx,word ptr [ebp+ecx*2]
    shl         edx,3
    sar         edx,3
    cmp         ecx,0FFFFh
    jne L_687f1
    xor         ecx,ecx
    jmp L_687f7
L_687f1:
    mov         ebp,dword ptr [esi]
    lea         ecx,[ebp+ecx*8]
L_687f7:
    mov         dword ptr [eax],ecx
    mov         ecx,dword ptr [esp+30h]
    movzx       ecx,word ptr [ecx+edx*2]
    cmp         ecx,0FFFFh
    jne L_6880d
    xor         ecx,ecx
    jmp L_68812
L_6880d:
    mov         edx,dword ptr [esi]
    lea         ecx,[edx+ecx*8]
L_68812:
    mov         ebp,dword ptr [esp+68h]
    mov         dword ptr [eax+4],ecx
    mov         eax,dword ptr [esp+5Ch]
    mov         ecx,dword ptr [esp+64h]
    mov         edx,dword ptr [ecx+4]
    add         ebp,10h
    inc         eax
    cmp         eax,edx
    mov         dword ptr [esp+68h],ebp
    mov         dword ptr [esp+5Ch],eax
    mov         ebp,ecx
    jl L_687a0
    xor         eax,eax
L_6883c:
    mov         edx,dword ptr [esi]
    mov         ecx,dword ptr [ebx+8]
    mov         dword ptr [esp+60h],eax
    mov         eax,dword ptr [ebp+4]
    dec         eax
    mov         dword ptr [esp+68h],edx
    mov         edx,dword ptr [ebp]
    mov         dword ptr [esp+10h],0FFFFh
    mov         dword ptr [esp+2Ch],ecx
    js L_68a29
    mov         ecx,eax
    shl         ecx,4
    inc         eax
    mov         dword ptr [esp+1Ch],ecx
    mov         dword ptr [esp+20h],eax
    jmp L_68880
L_68872:
    mov         ecx,dword ptr [esp+1Ch]
    mov         ebp,dword ptr [esp+64h]
    lea         ebx,[ebx+00000000h]
L_68880:
    mov         eax,dword ptr [ebp]
    add         eax,ecx
    mov         ecx,dword ptr [ebx+8]
    mov         dword ptr [esp+5Ch],ecx
    mov         ecx,dword ptr [esi+4]
    lea         edi,[ecx+1]
    mov         dword ptr [esi+4],edi
    mov         edi,dword ptr [esi]
    lea         edi,[edi+ecx*8]
    mov         ecx,dword ptr [ebx+8]
    mov         dword ptr [esp+18h],ecx
    mov         ecx,dword ptr [esi+4]
    lea         ebp,[ecx+1]
    mov         dword ptr [esi+4],ebp
    mov         ebp,dword ptr [esi]
    lea         ebp,[ebp+ecx*8]
    mov         ecx,dword ptr [eax]
    cmp         ecx,dword ptr [edx]
    jne L_6895a
    mov         ecx,dword ptr [eax+4]
    cmp         word ptr [ecx+6],2
    sete        dl
    test        dl,dl
    mov         byte ptr [esp+24h],dl
    je L_688d3
    movzx       edx,word ptr [ecx+2]
    jmp L_688d6
L_688d3:
    mov         edx,dword ptr [ebx+8]
L_688d6:
    mov         dword ptr [esp+14h],edx
    mov         dl,byte ptr [esp+24h]
    test        dl,dl
    je L_688ec
    movzx       ecx,word ptr [ecx+2]
    mov         edx,dword ptr [esp+68h]
    jmp L_688f7
L_688ec:
    mov         ecx,dword ptr [esi+4]
    lea         edx,[ecx+1]
    mov         dword ptr [esi+4],edx
    mov         edx,dword ptr [esi]
L_688f7:
    lea         ecx,[edx+ecx*8]
    mov         edx,dword ptr [eax+4]
    mov         dx,word ptr [edx]
    mov         word ptr [edi],dx
    mov         dx,word ptr [esp+18h]
    mov         word ptr [edi+4],dx
    mov         dx,word ptr [esp+10h]
    mov         word ptr [edi+2],dx
    mov         edx,dword ptr [esp+60h]
    test        edx,edx
    je L_68926
    mov         di,word ptr [esp+5Ch]
    mov         word ptr [edx+2],di
L_68926:
    mov         dx,word ptr [eax+0Eh]
    mov         edi,dword ptr [esp+68h]
    mov         word ptr [ecx],dx
    mov         dx,word ptr [esp+5Ch]
    mov         word ptr [ecx+4],dx
    mov         edx,dword ptr [eax+4]
    sub         edx,edi
    sar         edx,3
    mov         word ptr [ecx+2],dx
    mov         edx,dword ptr [eax+4]
    mov         ecx,dword ptr [esp+14h]
    mov         word ptr [edx+2],cx
    mov         dx,word ptr [eax+8]
    jmp L_689f5
L_6895a:
    cmp         word ptr [ecx+6],2
    sete        dl
    test        dl,dl
    mov         byte ptr [esp+28h],dl
    je L_68970
    movzx       edx,word ptr [ecx+2]
    jmp L_68973
L_68970:
    mov         edx,dword ptr [ebx+8]
L_68973:
    mov         dword ptr [esp+14h],edx
    mov         dl,byte ptr [esp+28h]
    test        dl,dl
    je L_68989
    movzx       ecx,word ptr [ecx+2]
    mov         edx,dword ptr [esp+68h]
    jmp L_68994
L_68989:
    mov         ecx,dword ptr [esi+4]
    lea         edx,[ecx+1]
    mov         dword ptr [esi+4],edx
    mov         edx,dword ptr [esi]
L_68994:
    lea         ecx,[edx+ecx*8]
    mov         dx,word ptr [eax+0Eh]
    mov         word ptr [edi],dx
    mov         dx,word ptr [esp+14h]
    mov         word ptr [edi+4],dx
    mov         dx,word ptr [esp+10h]
    mov         word ptr [edi+2],dx
    mov         edx,dword ptr [esp+60h]
    test        edx,edx
    je L_689c1
    mov         di,word ptr [esp+5Ch]
    mov         word ptr [edx+2],di
L_689c1:
    mov         dx,word ptr [eax+0Ch]
    mov         edi,dword ptr [esp+68h]
    mov         word ptr [ecx],dx
    mov         dx,word ptr [esp+18h]
    mov         word ptr [ecx+4],dx
    mov         edx,dword ptr [eax]
    sub         edx,edi
    sar         edx,3
    mov         word ptr [ecx+2],dx
    mov         ecx,dword ptr [eax]
    mov         dx,word ptr [esp+14h]
    mov         word ptr [ecx+2],dx
    mov         ecx,dword ptr [eax]
    mov         dx,word ptr [ecx]
    mov         cx,word ptr [esp+5Ch]
L_689f5:
    mov         word ptr [ebp],dx
    mov         edx,dword ptr [esp+18h]
    mov         word ptr [ebp+4],cx
    mov         ecx,dword ptr [esp+1Ch]
    mov         dword ptr [esp+10h],edx
    mov         edx,eax
    mov         eax,dword ptr [esp+20h]
    sub         ecx,10h
    dec         eax
    mov         dword ptr [esp+60h],ebp
    mov         dword ptr [esp+1Ch],ecx
    mov         dword ptr [esp+20h],eax
    jne L_68872
    mov         ecx,dword ptr [esp+2Ch]
L_68a29:
    mov         eax,dword ptr [esp+60h]
    mov         dx,word ptr [esp+10h]
    mov         word ptr [eax+2],cx
    mov         eax,dword ptr [esp+68h]
    mov         word ptr [eax+ecx*8+2],dx
    mov         eax,dword ptr [esp+40h]
    movd ecx,xmm7
    lea         eax,[eax+eax+0Fh]
    and         eax,0FFFFFFF0h
    je L_68a6a
    mov         edx,dword ptr [ecx+2Ch]
    mov         esi,dword ptr [esp+3Ch]
    cmp         esi,edx
    je L_68a61
    test        edx,edx
    jne L_68a6a
L_68a61:
    mov         edx,dword ptr [ecx]
    push        eax
    push        esi
    call        dword ptr [edx+28h]
    jmp L_68a7a
L_68a6a:
    mov         esi,dword ptr [ecx+20h]
    mov         edx,dword ptr [ecx+24h]
    sub         esi,eax
    add         edx,eax
    mov         dword ptr [ecx+20h],esi
    mov         dword ptr [ecx+24h],edx
L_68a7a:
    mov         eax,dword ptr [esp+38h]
    test        eax,eax
    js L_68a9c
    movd ecx,xmm7
    mov         edx,dword ptr [ecx]
    and         eax,7FFFFFFFh
    push        12h
    shl         eax,1
    push        eax
    mov         eax,dword ptr [esp+38h]
    push        eax
    call        dword ptr [edx+14h]
L_68a9c:
    mov         eax,dword ptr [esp+54h]
    movd ecx,xmm7
    lea         eax,[eax+eax+0Fh]
    and         eax,0FFFFFFF0h
    je L_68ac7
    mov         edx,dword ptr [ecx+2Ch]
    mov         esi,dword ptr [esp+50h]
    cmp         esi,edx
    je L_68abe
    test        edx,edx
    jne L_68ac7
L_68abe:
    mov         edx,dword ptr [ecx]
    push        eax
    push        esi
    call        dword ptr [edx+28h]
    jmp L_68ad7
L_68ac7:
    mov         esi,dword ptr [ecx+20h]
    mov         edx,dword ptr [ecx+24h]
    sub         esi,eax
    add         edx,eax
    mov         dword ptr [ecx+20h],esi
    mov         dword ptr [ecx+24h],edx
L_68ad7:
    mov         eax,dword ptr [esp+4Ch]
    test        eax,eax
    js L_68af9
    movd ecx,xmm7
    mov         edx,dword ptr [ecx]
    and         eax,7FFFFFFFh
    push        12h
    shl         eax,1
    push        eax
    mov         eax,dword ptr [esp+4Ch]
    push        eax
    call        dword ptr [edx+14h]
L_68af9:
    pop         edi
    pop         esi
    pop         ebp
    pop         ebx
    add         esp,48h
    ret
L_68b10:
    sub         esp,214h
    mov         ecx,dword ptr [esp+00000218h]
    mov         edx,dword ptr [ecx+0Ch]
    mov         eax,dword ptr [esp+0000021Ch]
    push        ebp
    mov         ebp,dword ptr [eax]
    push        esi
    mov         esi,dword ptr [esp+00000238h]
    mov         dword ptr [esp+0Ch],edx
    mov         edx,dword ptr [esi+4]
    cmp         edx,1
    jle L_68c8e
    push        edi
    mov         edi,dword ptr [esi]
    fld         dword ptr [edi+0Ch]
    fsub        dword ptr [edi+4]
    fcomp       dword ptr [esp+10h]
    fnstsw      ax
    test        ah,5
    jp L_68c8d
    lea         eax,[esp+20h]
    mov         ecx,80000040h
    mov         dword ptr [esp+14h],eax
    mov         dword ptr [esp+18h],0
    mov         dword ptr [esp+1Ch],ecx
    mov         eax,dword ptr [edi+4]
    xor         edi,edi
    test        edx,edx
    mov         dword ptr [esp+0Ch],eax
    jle L_68bea
    push        ebx
    jmp L_68b87
L_68b83:
    mov         ecx,dword ptr [esp+20h]
L_68b87:
    mov         edx,dword ptr [esi]
    fld         dword ptr [edx+edi*8+4]
    lea         ebx,[edx+edi*8]
    fsub        dword ptr [esp+10h]
    fcomp       dword ptr [esp+14h]
    fnstsw      ax
    test        ah,41h
    je L_68be9
    mov         eax,dword ptr [esp+1Ch]
    and         ecx,7FFFFFFFh
    cmp         eax,ecx
    jne L_68bbc
    lea         eax,[esp+18h]
    push        8
    push        eax
    call L_73600
    add         esp,8
L_68bbc:
    mov         ecx,dword ptr [ebx]
    mov         eax,dword ptr [esp+1Ch]
    mov         edx,dword ptr [esp+18h]
    mov         dword ptr [edx+eax*8],ecx
    mov         edx,dword ptr [esp+1Ch]
    mov         ecx,dword ptr [ebx+4]
    mov         eax,dword ptr [esp+18h]
    mov         dword ptr [eax+edx*8+4],ecx
    mov         edx,dword ptr [esp+1Ch]
    mov         eax,dword ptr [esi+4]
    inc         edx
    inc         edi
    cmp         edi,eax
    mov         dword ptr [esp+1Ch],edx
    jl L_68b83
L_68be9:
    pop         ebx
L_68bea:
    mov         edx,dword ptr [esp+00000238h]
    mov         eax,dword ptr [esp+00000234h]
    lea         ecx,[esp+14h]
    push        ecx
    mov         ecx,dword ptr [esp+00000234h]
    push        edx
    mov         edx,dword ptr [esp+00000234h]
    push        eax
    mov         eax,dword ptr [esp+18h]
    push        ecx
    mov         ecx,dword ptr [esp+00000234h]
    push        edx
    push        eax
    push        ecx
    push        ebp
    call L_66d60
    mov         eax,dword ptr [esp+34h]
    mov         ecx,dword ptr [eax]
    mov         edx,dword ptr [esi]
    mov         dword ptr [edx],ecx
    mov         eax,dword ptr [eax+4]
    mov         dword ptr [edx+4],eax
    mov         ecx,dword ptr [esi]
    mov         edx,dword ptr [esp+2Ch]
    mov         dword ptr [ecx+4],edx
    mov         eax,dword ptr [esi+8]
    and         eax,7FFFFFFFh
    add         esp,20h
    cmp         eax,1
    jge L_68c63
    add         eax,eax
    cmp         eax,1
    jg L_68c57
    mov         eax,1
L_68c57:
    push        8
    push        eax
    push        esi
    call L_73590
    add         esp,0Ch
L_68c63:
    mov         eax,dword ptr [esp+1Ch]
    test        eax,eax
    mov         dword ptr [esi+4],1
    js L_68c8d
    movd ecx,xmm7
    mov         edx,dword ptr [ecx]
    and         eax,7FFFFFFFh
    push        12h
    shl         eax,3
    push        eax
    mov         eax,dword ptr [esp+1Ch]
    push        eax
    call        dword ptr [edx+14h]
L_68c8d:
    pop         edi
L_68c8e:
    pop         esi
    pop         ebp
    add         esp,214h
    ret
L_68ca0:
    sub         esp,1Ch
    push        ebx
    mov         ebx,dword ptr [esp+40h]
    mov         ecx,dword ptr [ebx+4]
    mov         eax,1
    cmp         ecx,eax
    jne L_68dc0
    push        ebp
    mov         ebp,dword ptr [esp+48h]
    cmp         dword ptr [ebp+4],eax
    jne L_68dbf
    mov         ecx,dword ptr [esp+38h]
    mov         eax,dword ptr [ecx+8]
    test        eax,eax
    push        esi
    push        edi
    mov         edi,dword ptr [ecx]
    je L_68ced
    mov         dx,word ptr [esp+3Ch]
    movzx       esi,dx
    cmp         esi,0FFFFFFFFh
    je L_68ced
    mov         eax,dword ptr [eax+4]
    cmp         word ptr [eax],dx
    jne L_68ced
    mov         edi,dword ptr [ecx+4]
L_68ced:
    mov         edx,dword ptr [ebx]
    mov         ecx,dword ptr [ebp]
    fld         dword ptr [edx+4]
    fsub        dword ptr [ecx+4]
    mov         esi,dword ptr [esp+30h]
    fabs
    fcomp       dword ptr [esi+0Ch]
    fnstsw      ax
    test        ah,5
    jp L_68dbd
    mov         dword ptr [esp+14h],0
    mov         dword ptr [esp+18h],80000002h
    lea         eax,[esp+1Ch]
    mov         dword ptr [esp+10h],eax
    mov         eax,dword ptr [ecx+4]
    mov         dword ptr [esp+4Ch],eax
    mov         eax,dword ptr [edx]
    mov         dword ptr [esp+1Ch],eax
    mov         edx,dword ptr [edx+4]
    mov         dword ptr [esp+20h],edx
    mov         dword ptr [esp+14h],1
    mov         eax,dword ptr [ecx]
    mov         dword ptr [esp+24h],eax
    mov         ecx,dword ptr [ecx+4]
    mov         eax,dword ptr [esp+48h]
    lea         edx,[esp+10h]
    push        edx
    mov         edx,dword ptr [esp+3Ch]
    push        eax
    mov         eax,dword ptr [esp+54h]
    mov         dword ptr [esp+30h],ecx
    mov         ecx,dword ptr [esp+4Ch]
    push        ecx
    mov         ecx,dword ptr [esp+40h]
    push        edi
    push        edx
    push        eax
    push        esi
    push        ecx
    mov         dword ptr [esp+34h],2
    call L_66d60
    mov         edx,dword ptr [esp+30h]
    mov         eax,dword ptr [ebx]
    mov         ecx,dword ptr [edx]
    mov         edx,dword ptr [eax]
    add         esp,20h
    cmp         ecx,edx
    je L_68d8e
    mov         eax,dword ptr [ebp]
L_68d8e:
    fld         dword ptr [eax+4]
    fmul        dword ptr [K_88f74]
    fstp        dword ptr [eax+4]
    mov         eax,dword ptr [esp+18h]
    test        eax,eax
    js L_68dbd
    movd ecx,xmm7
    mov         edx,dword ptr [ecx]
    and         eax,7FFFFFFFh
    push        12h
    shl         eax,3
    push        eax
    mov         eax,dword ptr [esp+18h]
    push        eax
    call        dword ptr [edx+14h]
L_68dbd:
    pop         edi
    pop         esi
L_68dbf:
    pop         ebp
L_68dc0:
    pop         ebx
    add         esp,1Ch
    ret
L_68dd0:
    sub         esp,210h
    mov         eax,dword ptr [esp+00000218h]
    mov         edx,dword ptr [esp+0000021Ch]
    push        ebx
    push        ebp
    mov         ebp,dword ptr [eax+4]
    push        esi
    lea         ecx,[esp+1Ch]
    mov         ebx,80000080h
    push        edi
    mov         dword ptr [esp+14h],ecx
    mov         dword ptr [esp+1Ch],ebx
    mov         dword ptr [esp+20h],edx
    mov         dword ptr [esp+18h],1
    jmp L_68e10
    lea         ebx,[ebx+00000000h]
L_68e10:
    mov         eax,dword ptr [esp+18h]
    mov         ecx,dword ptr [esp+14h]
    mov         esi,dword ptr [ecx+eax*4-4]
    dec         eax
    mov         dword ptr [esp+18h],eax
    mov         dword ptr [esp+10h],esi
    mov         eax,esi
    jmp L_68e30
    lea         esp,[esp+00000000h]
L_68e30:
    movzx       edx,word ptr [eax+4]
    lea         eax,[ebp+edx*8]
    movzx       edx,word ptr [eax+6]
    mov         ecx,edx
    dec         ecx
    je L_68e4b
    dec         ecx
    je L_68f07
    dec         ecx
    jne L_68e58
L_68e4b:
    cmp         edx,dword ptr [esp+00000230h]
    jne L_68f1a
L_68e58:
    mov         cx,word ptr [esp+00000230h]
    mov         word ptr [eax+6],cx
    mov         ebx,dword ptr [esp+1Ch]
L_68e68:
    cmp         eax,esi
    jne L_68e30
    lea         esp,[esp]
L_68e70:
    movzx       edx,word ptr [esi+4]
    movzx       eax,word ptr [ebp+edx*8+2]
    cmp         word ptr [ebp+eax*8+6],0
    lea         esi,[ebp+edx*8]
    lea         edi,[ebp+eax*8]
    jne L_68ebe
    mov         eax,dword ptr [esp+18h]
    and         ebx,7FFFFFFFh
    cmp         eax,ebx
    jne L_68ea6
    lea         ecx,[esp+14h]
    push        4
    push        ecx
    call L_73600
    add         esp,8
L_68ea6:
    mov         eax,dword ptr [esp+18h]
    mov         edx,dword ptr [esp+14h]
    mov         dword ptr [edx+eax*4],edi
    mov         eax,dword ptr [esp+18h]
    mov         ebx,dword ptr [esp+1Ch]
    inc         eax
    mov         dword ptr [esp+18h],eax
L_68ebe:
    cmp         esi,dword ptr [esp+10h]
    jne L_68e70
    mov         eax,dword ptr [esp+18h]
    test        eax,eax
    jne L_68e10
    mov         esi,dword ptr [esp+00000224h]
    mov         byte ptr [esi],1
L_68eda:
    test        ebx,ebx
    js L_68efa
    movd ecx,xmm7
    mov         eax,dword ptr [esp+14h]
    mov         edx,dword ptr [ecx]
    and         ebx,7FFFFFFFh
    push        12h
    shl         ebx,2
    push        ebx
    push        eax
    call        dword ptr [edx+14h]
L_68efa:
    pop         edi
    mov         eax,esi
    pop         esi
    pop         ebp
    pop         ebx
    add         esp,210h
    ret
L_68f07:
    cmp         dword ptr [esp+00000230h],3
    je L_68e68
    jmp L_68e58
L_68f1a:
    mov         esi,dword ptr [esp+00000224h]
    mov         byte ptr [esi],0
    jmp L_68eda
L_68f30:
    sub         esp,1Ch
    mov         ecx,dword ptr [esp+24h]
    mov         eax,dword ptr [ecx+4]
    mov         edx,dword ptr [ecx+8]
    push        ebx
    push        ebp
    mov         dword ptr [esp+0Ch],eax
    xor         ebp,ebp
    push        esi
    xor         eax,eax
    cmp         edx,ebp
    push        edi
    jle L_68f60
    lea         ecx,[ecx]
L_68f50:
    mov         edx,dword ptr [ecx+4]
    mov         word ptr [edx+eax*8+6],bp
    mov         edx,dword ptr [ecx+8]
    inc         eax
    cmp         eax,edx
    jl L_68f50
L_68f60:
    mov         dl,byte ptr [esp+3Ch]
    test        dl,dl
    mov         ebx,dword ptr [esp+40h]
    je L_68f72
    mov         eax,dword ptr [ebx]
    mov         edi,dword ptr [eax]
    jmp L_68f77
L_68f72:
    mov         ecx,dword ptr [ebx]
    mov         edi,dword ptr [ecx+4]
L_68f77:
    mov         esi,dword ptr [ebx+4]
    dec         esi
    js L_68fca
    mov         ecx,esi
    shl         ecx,4
L_68f82:
    test        dl,dl
    mov         eax,dword ptr [ebx]
    je L_68f8d
    mov         eax,dword ptr [ecx+eax]
    jmp L_68f91
L_68f8d:
    mov         eax,dword ptr [ecx+eax+4]
L_68f91:
    cmp         eax,edi
    je L_68fbe
    inc         ebp
    test        dl,dl
    mov         edi,eax
    je L_68fa7
    movzx       edx,word ptr [eax+2]
    mov         eax,dword ptr [esp+14h]
    lea         eax,[eax+edx*8]
L_68fa7:
    mov         edx,3
    cmp         word ptr [eax+6],dx
    je L_69058
    mov         word ptr [eax+6],dx
    mov         dl,byte ptr [esp+3Ch]
L_68fbe:
    dec         esi
    sub         ecx,10h
    test        esi,esi
    jge L_68f82
    test        ebp,ebp
    jne L_68fd9
L_68fca:
    mov         eax,dword ptr [esp+30h]
    pop         edi
    pop         esi
    pop         ebp
    mov         byte ptr [eax],1
    pop         ebx
    add         esp,1Ch
    ret
L_68fd9:
    test        dl,dl
    je L_68fe3
    mov         ecx,dword ptr [ebx]
    mov         edi,dword ptr [ecx]
    jmp L_68fe8
L_68fe3:
    mov         eax,dword ptr [ebx]
    mov         edi,dword ptr [eax+4]
L_68fe8:
    mov         esi,dword ptr [ebx+4]
    dec         esi
    mov         ebp,2
    js L_69048
    mov         edx,esi
    shl         edx,4
L_68ff8:
    mov         cl,byte ptr [esp+3Ch]
    test        cl,cl
    mov         eax,dword ptr [ebx]
    je L_69007
    mov         eax,dword ptr [edx+eax]
    jmp L_6900b
L_69007:
    mov         eax,dword ptr [edx+eax+4]
L_6900b:
    cmp         eax,edi
    je L_6903c
    test        cl,cl
    mov         edi,eax
    jne L_69020
    movzx       ecx,word ptr [eax+2]
    mov         eax,dword ptr [esp+14h]
    lea         eax,[eax+ecx*8]
L_69020:
    movzx       ecx,word ptr [eax+6]
    sub         ecx,0
    je L_69036
    dec         ecx
    je L_69058
    sub         ecx,ebp
    jne L_6903c
    mov         word ptr [eax+6],bp
    jmp L_6903c
L_69036:
    mov         word ptr [eax+6],1
L_6903c:
    dec         esi
    sub         edx,10h
    test        esi,esi
    jge L_68ff8
    mov         dl,byte ptr [esp+3Ch]
L_69048:
    test        dl,dl
    mov         eax,dword ptr [ebx]
    mov         cl,1
    mov         byte ptr [esp+18h],cl
    je L_69067
    mov         ebp,dword ptr [eax]
    jmp L_6906a
L_69058:
    mov         eax,dword ptr [esp+30h]
    pop         edi
    pop         esi
    pop         ebp
    mov         byte ptr [eax],0
    pop         ebx
    add         esp,1Ch
    ret
L_69067:
    mov         ebp,dword ptr [eax+4]
L_6906a:
    mov         eax,dword ptr [ebx+4]
    dec         eax
    js L_69132
    mov         edi,eax
    shl         edi,4
    inc         eax
    mov         dword ptr [esp+20h],eax
    mov         edi,edi
L_69080:
    test        dl,dl
    mov         eax,dword ptr [ebx]
    je L_6908b
    mov         eax,dword ptr [edi+eax]
    jmp L_6908f
L_6908b:
    mov         eax,dword ptr [edi+eax+4]
L_6908f:
    cmp         eax,ebp
    je L_6911c
    test        dl,dl
    mov         ebp,eax
    je L_690a1
    mov         esi,eax
    jmp L_690ac
L_690a1:
    movzx       eax,word ptr [eax+2]
    mov         esi,dword ptr [esp+14h]
    lea         esi,[esi+eax*8]
L_690ac:
    cmp         word ptr [esi+6],1
    jne L_690de
    test        cl,cl
    je L_690da
    mov         ecx,dword ptr [esp+34h]
    push        1
    push        esi
    push        ecx
    lea         edx,[esp+1Eh]
    push        edx
    call L_68dd0
    mov         cl,byte ptr [eax]
    mov         dl,byte ptr [esp+4Ch]
    add         esp,10h
    test        cl,cl
    je L_690da
    mov         al,1
    jmp L_690dc
L_690da:
    xor         al,al
L_690dc:
    mov         cl,al
L_690de:
    movzx       eax,word ptr [esi+2]
    mov         esi,dword ptr [esp+14h]
    cmp         word ptr [esi+eax*8+6],3
    lea         eax,[esi+eax*8]
    jne L_6911c
    test        cl,cl
    je L_69118
    mov         ecx,dword ptr [esp+34h]
    push        3
    push        eax
    push        ecx
    lea         edx,[esp+1Fh]
    push        edx
    call L_68dd0
    mov         cl,byte ptr [eax]
    mov         dl,byte ptr [esp+4Ch]
    add         esp,10h
    test        cl,cl
    je L_69118
    mov         al,1
    jmp L_6911a
L_69118:
    xor         al,al
L_6911a:
    mov         cl,al
L_6911c:
    mov         eax,dword ptr [esp+20h]
    sub         edi,10h
    dec         eax
    mov         dword ptr [esp+20h],eax
    jne L_69080
    mov         byte ptr [esp+18h],cl
L_69132:
    test        dl,dl
    mov         eax,dword ptr [ebx]
    je L_6913c
    mov         eax,dword ptr [eax]
    jmp L_6913f
L_6913c:
    mov         eax,dword ptr [eax+4]
L_6913f:
    mov         dword ptr [esp+1Ch],eax
    mov         eax,dword ptr [ebx+4]
    dec         eax
    js L_6926b
    mov         ecx,eax
    shl         ecx,4
    inc         eax
    mov         dword ptr [esp+28h],eax
    mov         dword ptr [esp+20h],ecx
    mov         eax,ecx
    jmp L_69163
L_6915f:
    mov         dl,byte ptr [esp+3Ch]
L_69163:
    test        dl,dl
    mov         ecx,dword ptr [esp+40h]
    mov         esi,dword ptr [ecx]
    je L_69172
    mov         eax,dword ptr [esi+eax]
    jmp L_69176
L_69172:
    mov         eax,dword ptr [esi+eax+4]
L_69176:
    cmp         eax,dword ptr [esp+1Ch]
    je L_6924d
    test        dl,dl
    mov         dword ptr [esp+1Ch],eax
    je L_6918c
    mov         ebp,eax
    jmp L_69197
L_6918c:
    movzx       eax,word ptr [eax+2]
    mov         edi,dword ptr [esp+14h]
    lea         ebp,[edi+eax*8]
L_69197:
    mov         ebx,2
    cmp         word ptr [ebp+6],bx
    jne L_6924d
    test        dl,dl
    mov         byte ptr [esp+24h],1
    je L_691b3
    mov         edi,dword ptr [esi]
    jmp L_691b6
L_691b3:
    mov         edi,dword ptr [esi+4]
L_691b6:
    mov         edx,dword ptr [ecx+4]
    dec         edx
    js L_691ed
    mov         ecx,edx
    shl         ecx,4
    add         ecx,esi
L_691c3:
    mov         al,byte ptr [esp+3Ch]
    test        al,al
    je L_691cf
    mov         eax,dword ptr [ecx]
    jmp L_691d2
L_691cf:
    mov         eax,dword ptr [ecx+4]
L_691d2:
    cmp         eax,edi
    je L_691de
    cmp         word ptr [eax+6],bx
    mov         edi,eax
    jne L_691e8
L_691de:
    dec         edx
    sub         ecx,10h
    test        edx,edx
    jge L_691c3
    jmp L_691ed
L_691e8:
    mov         byte ptr [esp+24h],0
L_691ed:
    mov         eax,dword ptr [esp+34h]
    mov         edx,dword ptr [eax+8]
    xor         bl,bl
    xor         ecx,ecx
    test        edx,edx
    jle L_69214
    mov         eax,dword ptr [eax+4]
    add         eax,6
L_69202:
    cmp         word ptr [eax],0
    je L_69212
    inc         ecx
    add         eax,8
    cmp         ecx,edx
    jl L_69202
    jmp L_69214
L_69212:
    mov         bl,1
L_69214:
    mov         al,byte ptr [esp+24h]
    test        al,al
    je L_6924d
    test        bl,bl
    je L_6924d
    mov         al,byte ptr [esp+18h]
    test        al,al
    je L_69247
    mov         ecx,dword ptr [esp+34h]
    push        3
    push        ebp
    push        ecx
    lea         edx,[esp+1Fh]
    push        edx
    call L_68dd0
    mov         cl,byte ptr [eax]
    add         esp,10h
    test        cl,cl
    je L_69247
    mov         al,1
    jmp L_69249
L_69247:
    xor         al,al
L_69249:
    mov         byte ptr [esp+18h],al
L_6924d:
    mov         eax,dword ptr [esp+20h]
    mov         ecx,dword ptr [esp+28h]
    sub         eax,10h
    dec         ecx
    mov         dword ptr [esp+20h],eax
    mov         dword ptr [esp+28h],ecx
    jne L_6915f
    mov         cl,byte ptr [esp+18h]
L_6926b:
    mov         esi,dword ptr [esp+34h]
    mov         edx,dword ptr [esi+8]
    xor         eax,eax
    test        edx,edx
    jle L_69292
L_69278:
    test        cl,cl
    je L_6928b
    mov         ecx,dword ptr [esi+4]
    cmp         word ptr [ecx+eax*8+6],0
    je L_6928b
    mov         cl,1
    jmp L_6928d
L_6928b:
    xor         cl,cl
L_6928d:
    inc         eax
    cmp         eax,edx
    jl L_69278
L_69292:
    mov         eax,dword ptr [esp+30h]
    pop         edi
    pop         esi
    pop         ebp
    mov         byte ptr [eax],cl
    pop         ebx
    add         esp,1Ch
    ret
L_692a0:
    mov         eax,dword ptr [esp+8]
    mov         ecx,dword ptr [eax+8]
    sub         esp,10h
    cmp         ecx,1
    je L_693b9
    mov         ecx,dword ptr [eax+4]
    mov         eax,dword ptr [eax]
    mov         edx,dword ptr [esp+24h]
    push        esi
    mov         esi,dword ptr [esp+34h]
    mov         dword ptr [esp+8],eax
    movzx       eax,word ptr [edx+2]
    push        edi
    lea         edi,[ecx+eax*8]
    push        ebx
    mov         dword ptr [esp+14h],ecx
    mov         dword ptr [esp+18h],edi
    push        ebp
    jmp L_692e0
    lea         esp,[esp+00000000h]
L_692e0:
    mov         ax,word ptr [edi]
    cmp         ax,word ptr [esp+30h]
    jne L_692f4
    mov         dword ptr [esp+10h],40800000h
    jmp L_69321
L_692f4:
    mov         ecx,dword ptr [esp+3Ch]
    mov         edx,dword ptr [esp+38h]
    mov         ebp,dword ptr [esp+14h]
    push        ecx
    mov         ecx,dword ptr [esp+30h]
    push        edx
    movzx       edx,ax
    mov         eax,dword ptr [esp+2Ch]
    shl         edx,4
    push        ecx
    add         edx,ebp
    push        edx
    push        eax
    call L_664d0
    add         esp,14h
    fstp        dword ptr [esp+10h]
L_69321:
    mov         ebp,dword ptr [esi+4]
    mov         eax,dword ptr [esi+8]
    lea         ebx,[ebp+1]
    and         eax,7FFFFFFFh
    cmp         eax,ebx
    jge L_69347
    add         eax,eax
    cmp         ebx,eax
    jl L_6933b
    mov         eax,ebx
L_6933b:
    push        8
    push        eax
    push        esi
    call L_73590
    add         esp,0Ch
L_69347:
    mov         ecx,dword ptr [esi]
    mov         edx,dword ptr [esp+10h]
    lea         eax,[ecx+ebp*8]
    mov         dword ptr [esi+4],ebx
    mov         dword ptr [eax],edi
    mov         dword ptr [eax+4],edx
    movzx       ecx,word ptr [edi+4]
    mov         eax,dword ptr [esp+18h]
    movzx       edx,word ptr [eax+ecx*8+2]
    lea         edi,[eax+edx*8]
    cmp         edi,dword ptr [esp+1Ch]
    jne L_692e0
    mov         eax,dword ptr [esi+4]
    cmp         eax,1
    pop         ebp
    pop         ebx
    jle L_69390
    push        offset L_65320
    dec         eax
    push        eax
    mov         eax,dword ptr [esi]
    push        0
    push        eax
    call L_656a0
    add         esp,10h
L_69390:
    mov         ecx,dword ptr [esp+34h]
    mov         edx,dword ptr [esp+30h]
    mov         eax,dword ptr [esp+2Ch]
    push        esi
    push        ecx
    mov         ecx,dword ptr [esp+2Ch]
    push        edx
    mov         edx,dword ptr [esp+2Ch]
    push        eax
    mov         eax,dword ptr [esp+2Ch]
    push        ecx
    push        edx
    push        eax
    call L_68b10
    add         esp,1Ch
    pop         edi
    pop         esi
L_693b9:
    add         esp,10h
    ret
L_693c0:
    sub         esp,14h
    push        ebx
    mov         ebx,dword ptr [esp+24h]
    mov         ecx,dword ptr [ebx+4]
    push        ebp
    xor         eax,eax
    cmp         ecx,eax
    push        esi
    mov         esi,dword ptr [esp+30h]
    push        edi
    mov         dword ptr [esp+14h],eax
    jle L_69514
    mov         dword ptr [esp+10h],eax
    jmp L_693f0
L_693e6:
    xor         eax,eax
    jmp L_693f0
    lea         ebx,[ebx+00000000h]
L_693f0:
    mov         ebp,dword ptr [esp+10h]
    mov         dword ptr [esi+4],eax
    mov         eax,dword ptr [esi+8]
    mov         edi,dword ptr [ebx]
    add         edi,ebp
    and         eax,7FFFFFFFh
    cmp         eax,1
    mov         dword ptr [esp+18h],edi
    jge L_69424
    add         eax,eax
    cmp         eax,1
    jg L_69418
    mov         eax,1
L_69418:
    push        10h
    push        eax
    push        esi
    call L_73590
    add         esp,0Ch
L_69424:
    mov         eax,dword ptr [esi]
    mov         dword ptr [esi+4],1
    mov         ecx,dword ptr [edi]
    mov         dword ptr [eax],ecx
    mov         edx,dword ptr [edi+4]
    mov         dword ptr [eax+4],edx
    mov         ebp,dword ptr [edi+8]
    test        ebp,ebp
    je L_694f3
    jmp L_69448
L_69444:
    mov         edi,dword ptr [esp+18h]
L_69448:
    mov         eax,dword ptr [ebp]
    cmp         eax,dword ptr [edi]
    jne L_694a9
    mov         ecx,dword ptr [ebp+4]
    cmp         ecx,dword ptr [edi+4]
    jne L_694a9
    mov         edx,dword ptr [ebp+0Ch]
    cmp         edx,dword ptr [edi+0Ch]
    jne L_694a9
    mov         edi,dword ptr [esp+2Ch]
    mov         ebx,dword ptr [esp+28h]
    push        esi
    push        ecx
    mov         eax,esp
    mov         byte ptr [eax],1
    mov         eax,dword ptr [edi+4]
    push        eax
    lea         eax,[esp+28h]
    push        ebx
    push        eax
    call L_68f30
    add         esp,14h
    push        esi
    push        ecx
    mov         eax,esp
    mov         byte ptr [eax],0
    mov         eax,dword ptr [ebx+4]
    push        eax
    lea         ecx,[esp+2Ch]
    push        edi
    push        ecx
    call L_68f30
    mov         al,byte ptr [esp+30h]
    add         esp,14h
    test        al,al
    je L_694a9
    mov         al,byte ptr [esp+20h]
    test        al,al
    jne L_69517
L_694a9:
    mov         edi,dword ptr [esi+4]
    mov         eax,dword ptr [esi+8]
    lea         ebx,[edi+1]
    and         eax,7FFFFFFFh
    cmp         eax,ebx
    jge L_694cf
    add         eax,eax
    cmp         ebx,eax
    jl L_694c3
    mov         eax,ebx
L_694c3:
    push        10h
    push        eax
    push        esi
    call L_73590
    add         esp,0Ch
L_694cf:
    mov         ecx,dword ptr [esi]
    mov         dword ptr [esi+4],ebx
    mov         edx,dword ptr [ebp]
    shl         edi,4
    add         edi,ecx
    mov         dword ptr [edi],edx
    mov         eax,dword ptr [ebp+4]
    mov         dword ptr [edi+4],eax
    mov         ebp,dword ptr [ebp+8]
    test        ebp,ebp
    jne L_69444
    mov         ebx,dword ptr [esp+30h]
L_694f3:
    mov         eax,dword ptr [esp+14h]
    mov         edx,dword ptr [esp+10h]
    mov         ecx,dword ptr [ebx+4]
    inc         eax
    add         edx,14h
    cmp         eax,ecx
    mov         dword ptr [esp+14h],eax
    mov         dword ptr [esp+10h],edx
    jl L_693e6
    xor         eax,eax
L_69514:
    mov         dword ptr [esi+4],eax
L_69517:
    pop         edi
    pop         esi
    pop         ebp
    pop         ebx
    add         esp,14h
    ret
L_69520:
    push        ebp
    mov         ebp,esp
    and         esp,0FFFFFFF0h
    sub         esp,0E24h
    mov         eax,dword ptr [ebp+14h]
    push        ebx
    mov         ebx,dword ptr [ebp+0Ch]
    mov         ecx,dword ptr [ebx]
    push        esi
    mov         esi,dword ptr [ebp+8]
    push        edi
    mov         edi,dword ptr [ebp+10h]
    push        eax
    push        edi
    mov         dword ptr [eax],ecx
    mov         edx,dword ptr [esi+8]
    push        ebx
    push        edx
    lea         eax,[esp+00000097h]
    push        eax
    call L_65c60
    mov         cl,byte ptr [eax]
    add         esp,14h
    test        cl,cl
    jne L_69f5e
    mov         ecx,dword ptr [ebx]
    lea         edx,[esp+000000DCh]
    lea         eax,[esp+000000B0h]
    push        eax
    mov         dword ptr [esp+0000008Ch],ecx
    mov         ecx,edx
    push        ecx
    push        edi
    push        ebx
    mov         dword ptr [esp+000000E0h],edx
    mov         dword ptr [esp+000000E8h],80000001h
    mov         dword ptr [esp+000000E4h],1
    call L_66330
    mov         ecx,dword ptr [esp+000000C8h]
    mov         eax,dword ptr [esp+000000C4h]
    mov         edx,dword ptr [esp+000000C0h]
    mov         edi,dword ptr [edi+8]
    mov         dword ptr [esp+000000B8h],ecx
    mov         ecx,dword ptr [ebx+8]
    mov         dword ptr [esp+000000B4h],eax
    lea         eax,[esp+0000010Ch]
    lea         edi,[ecx+edi+2]
    mov         ecx,dword ptr [esi+20h]
    mov         dword ptr [esp+000000B0h],edx
    mov         edx,dword ptr [esp+000000CCh]
    mov         dword ptr [esp+00000100h],eax
    xor         eax,eax
    lea         esi,[esp+0000063Ch]
    mov         dword ptr [esp+000000D4h],ecx
    lea         ecx,[edi+edi*2]
    mov         dword ptr [esp+000000BCh],edx
    mov         dword ptr [esp+00000104h],eax
    mov         dword ptr [esp+00000630h],esi
    mov         dword ptr [esp+00000634h],eax
    add         esp,10h
    test        ecx,ecx
    mov         edx,80000040h
    mov         eax,80000080h
    lea         esi,[esp+000000D0h]
    mov         dword ptr [esp+000000F8h],edx
    mov         dword ptr [esp+0000009Ch],edi
    mov         dword ptr [esp+000000CCh],ecx
    mov         dword ptr [esp+00000628h],eax
    mov         dword ptr [esp+00000080h],esi
    mov         dword ptr [esp+00000090h],0
    jle L_69e1f
L_69664:
    cmp         dword ptr [esp+00000090h],edi
    jle L_69696
    fld         dword ptr [esp+000000C4h]
    fmul        dword ptr [K_91494]
    fst         dword ptr [esp+000000C4h]
    fcomp       dword ptr [K_88e5c]
    fnstsw      ax
    test        ah,41h
    jne L_69696
    inc         edi
    mov         dword ptr [esp+0000009Ch],edi
L_69696:
    movd ecx,xmm7
    mov         edx,dword ptr [ecx+24h]
    cmp         edx,150h
    jge L_696b9
    mov         edx,dword ptr [ecx]
    push        150h
    call        dword ptr [edx+24h]
    mov         esi,eax
    mov         dword ptr [esp+7Ch],esi
    jmp L_696d4
L_696b9:
    mov         eax,dword ptr [ecx+20h]
    lea         esi,[eax+00000150h]
    add         edx,0FFFFFEB0h
    mov         dword ptr [ecx+20h],esi
    mov         dword ptr [ecx+24h],edx
    mov         dword ptr [esp+7Ch],eax
    mov         esi,eax
L_696d4:
    xor         ecx,ecx
    cmp         esi,ecx
    je L_696e9
    lea         eax,[esi+0Ch]
    mov         dword ptr [esi],eax
    mov         dword ptr [esi+4],ecx
    mov         dword ptr [esi+8],80000010h
L_696e9:
    mov         edx,dword ptr [esp+000000F8h]
    mov         eax,dword ptr [esp+000000F4h]
    and         edx,7FFFFFFFh
    cmp         eax,edx
    jne L_69715
    lea         eax,[esp+000000F0h]
    push        4
    push        eax
    call L_73600
    add         esp,8
    xor         ecx,ecx
L_69715:
    mov         eax,dword ptr [esp+000000F4h]
    mov         edx,dword ptr [esp+000000F0h]
    mov         dword ptr [edx+eax*4],esi
    inc         dword ptr [esp+000000F4h]
    mov         edx,dword ptr [esp+00000080h]
    cmp         dword ptr [edx+4],ecx
    mov         dword ptr [esp+00000098h],ecx
    jle L_69dc2
    mov         dword ptr [esp+00000094h],ecx
    jmp L_69750
    lea         ecx,[ecx]
L_69750:
    mov         eax,dword ptr [esp+00000080h]
    mov         esi,dword ptr [eax]
    mov         edx,dword ptr [esp+00000094h]
    mov         ebx,dword ptr [esp+00000088h]
    add         esi,edx
    lea         edx,[esp+0000041Ch]
    mov         dword ptr [esp+00000410h],edx
    mov         dword ptr [esp+00000414h],ecx
    mov         eax,80000040h
    mov         dword ptr [esp+00000418h],eax
    mov         dword ptr [esp+00000204h],ecx
    mov         dword ptr [esp+00000208h],eax
    lea         edx,[esp+0000020Ch]
    mov         dword ptr [esp+00000200h],edx
    mov         eax,dword ptr [esi]
    movzx       edi,word ptr [eax]
    mov         eax,dword ptr [esi+4]
    mov         edx,dword ptr [esp+00000088h]
    shl         edi,4
    add         edi,ebx
    movzx       ebx,word ptr [eax]
    mov         eax,dword ptr [esi+8]
    shl         ebx,4
    add         ebx,edx
    test        eax,eax
    je L_69c35
    mov         ecx,dword ptr [eax]
    movzx       ecx,word ptr [ecx]
    shl         ecx,4
    add         ecx,edx
    mov         edx,dword ptr [eax+4]
    movzx       edx,word ptr [edx]
    mov         eax,dword ptr [esp+00000088h]
    shl         edx,4
    add         edx,eax
    cmp         ecx,edi
    mov         dword ptr [esp+0000008Ch],ebx
    je L_697fb
    mov         dword ptr [esp+0000008Ch],edi
L_697fb:
    fld         dword ptr [edx]
    fsub        dword ptr [ecx]
    fld         dword ptr [edx+4]
    fsub        dword ptr [ecx+4]
    fstp        dword ptr [esp+34h]
    fld         dword ptr [edx+8]
    fsub        dword ptr [ecx+8]
    fld         st(0)
    fmul        st,st(1)
    fld         dword ptr [esp+34h]
    fmul        dword ptr [esp+34h]
    faddp       st(1),st
    fld         st(2)
    fmul        st,st(3)
    faddp       st(1),st
    fld         dword ptr [K_888a8]
    fld         st(1)
    fucompp
    fnstsw      ax
    test        ah,44h
    jnp L_6983e
    fsqrt
    fdivr       dword ptr [K_88e5c]
    jmp L_69846
L_6983e:
    fstp        st(0)
    fld         dword ptr [K_888a8]
L_69846:
    fld         st(0)
    mov         eax,dword ptr [esp+0000008Ch]
    fmul        st,st(3)
    fstp        dword ptr [esp+30h]
    fld         dword ptr [esp+34h]
    fmul        st,st(1)
    fstp        dword ptr [esp+34h]
    fxch        st(1)
    fmul        st,st(1)
    fstp        dword ptr [esp+38h]
    fstp        st(0)
    fstp        st(0)
    fld         dword ptr [eax]
    fsub        dword ptr [ecx]
    fld         dword ptr [eax+4]
    fsub        dword ptr [ecx+4]
    fstp        dword ptr [esp+24h]
    fld         dword ptr [eax+8]
    fsub        dword ptr [ecx+8]
    fld         st(0)
    fmul        st,st(1)
    fld         dword ptr [esp+24h]
    fmul        dword ptr [esp+24h]
    faddp       st(1),st
    fld         st(2)
    fmul        st,st(3)
    faddp       st(1),st
    fld         dword ptr [K_888a8]
    fld         st(1)
    fucompp
    fnstsw      ax
    test        ah,44h
    jnp L_698ae
    fsqrt
    fdivr       dword ptr [K_88e5c]
    jmp L_698b6
L_698ae:
    fstp        st(0)
    fld         dword ptr [K_888a8]
L_698b6:
    fld         st(0)
    mov         dword ptr [esp+1Ch],0
    fmul        st,st(3)
    fstp        dword ptr [esp+20h]
    fld         dword ptr [esp+24h]
    fmul        st,st(1)
    fstp        dword ptr [esp+24h]
    fxch        st(1)
    fmul        st,st(1)
    fstp        dword ptr [esp+28h]
    fstp        st(0)
    fstp        st(0)
    fld         dword ptr [esp+28h]
    fmul        dword ptr [esp+34h]
    fld         dword ptr [esp+24h]
    fmul        dword ptr [esp+38h]
    fsubp       st(1),st
    fld         dword ptr [esp+38h]
    fmul        dword ptr [esp+20h]
    fld         dword ptr [esp+28h]
    fmul        dword ptr [esp+30h]
    fsubp       st(1),st
    fstp        dword ptr [esp+14h]
    fld         dword ptr [esp+24h]
    fmul        dword ptr [esp+30h]
    fld         dword ptr [esp+34h]
    fmul        dword ptr [esp+20h]
    fsubp       st(1),st
    fstp        dword ptr [esp+18h]
    fld         dword ptr [esp+18h]
    fmul        dword ptr [esp+18h]
    fld         dword ptr [esp+14h]
    fmul        dword ptr [esp+14h]
    faddp       st(1),st
    fld         st(1)
    fmul        st,st(2)
    faddp       st(1),st
    fcomp       dword ptr [K_91474]
    fnstsw      ax
    test        ah,5
    jp L_69bc6
    mov         eax,dword ptr [esp+0000008Ch]
    fstp        st(0)
    fld         dword ptr [eax]
    fsub        dword ptr [edx]
    fstp        dword ptr [esp+20h]
    fld         dword ptr [eax+4]
    fsub        dword ptr [edx+4]
    fstp        dword ptr [esp+24h]
    fld         dword ptr [eax+8]
    fsub        dword ptr [edx+8]
    fld         st(0)
    fmul        st,st(1)
    fld         dword ptr [esp+24h]
    fmul        dword ptr [esp+24h]
    faddp       st(1),st
    fld         dword ptr [esp+20h]
    fmul        dword ptr [esp+20h]
    faddp       st(1),st
    fld         dword ptr [K_888a8]
    fld         st(1)
    fucompp
    fnstsw      ax
    test        ah,44h
    jnp L_69997
    fsqrt
    fdivr       dword ptr [K_88e5c]
    jmp L_6999f
L_69997:
    fstp        st(0)
    fld         dword ptr [K_888a8]
L_6999f:
    fld         dword ptr [esp+20h]
    mov         dword ptr [esp+1Ch],0
    fmul        st,st(1)
    fstp        dword ptr [esp+20h]
    fld         dword ptr [esp+24h]
    fmul        st,st(1)
    fstp        dword ptr [esp+24h]
    fxch        st(1)
    fmul        st,st(1)
    fstp        dword ptr [esp+28h]
    fstp        st(0)
    fld         dword ptr [esp+28h]
    fmul        dword ptr [esp+34h]
    fld         dword ptr [esp+24h]
    fmul        dword ptr [esp+38h]
    fsubp       st(1),st
    fld         dword ptr [esp+38h]
    fmul        dword ptr [esp+20h]
    fld         dword ptr [esp+28h]
    fmul        dword ptr [esp+30h]
    fsubp       st(1),st
    fstp        dword ptr [esp+14h]
    fld         dword ptr [esp+24h]
    fmul        dword ptr [esp+30h]
    fld         dword ptr [esp+34h]
    fmul        dword ptr [esp+20h]
    fsubp       st(1),st
    fstp        dword ptr [esp+18h]
    fld         dword ptr [esp+18h]
    fmul        dword ptr [esp+18h]
    fld         dword ptr [esp+14h]
    fmul        dword ptr [esp+14h]
    faddp       st(1),st
    fld         st(1)
    fmul        st,st(2)
    faddp       st(1),st
    fcomp       dword ptr [K_91474]
    fnstsw      ax
    test        ah,5
    jp L_69bc6
    fstp        st(0)
    fld         dword ptr [esp+20h]
    fadd        dword ptr [esp+30h]
    fstp        dword ptr [esp+60h]
    fld         dword ptr [esp+24h]
    fadd        dword ptr [esp+34h]
    fstp        dword ptr [esp+64h]
    fld         dword ptr [esp+28h]
    fadd        dword ptr [esp+38h]
    fstp        dword ptr [esp+68h]
    fld         dword ptr [esp+30h]
    fsub        dword ptr [esp+20h]
    fstp        dword ptr [esp+50h]
    fld         dword ptr [esp+34h]
    fsub        dword ptr [esp+24h]
    fstp        dword ptr [esp+54h]
    fld         dword ptr [esp+38h]
    fsub        dword ptr [esp+28h]
    fst         dword ptr [esp+58h]
    fmul        dword ptr [esp+58h]
    fld         dword ptr [esp+54h]
    fmul        dword ptr [esp+54h]
    faddp       st(1),st
    fld         dword ptr [esp+50h]
    fmul        dword ptr [esp+50h]
    faddp       st(1),st
    fcom        dword ptr [K_91474]
    fnstsw      ax
    test        ah,5
    jnp L_69b9c
    fld         dword ptr [esp+68h]
    fmul        dword ptr [esp+68h]
    fld         dword ptr [esp+64h]
    fmul        dword ptr [esp+64h]
    faddp       st(1),st
    fld         dword ptr [esp+60h]
    fmul        dword ptr [esp+60h]
    faddp       st(1),st
    fcom        dword ptr [K_91474]
    fnstsw      ax
    test        ah,5
    jnp L_69b9a
    fld         dword ptr [K_888a8]
    fld         st(1)
    fucompp
    fnstsw      ax
    test        ah,44h
    jnp L_69ae7
    fsqrt
    fdivr       dword ptr [K_88e5c]
    jmp L_69aef
L_69ae7:
    fstp        st(0)
    fld         dword ptr [K_888a8]
L_69aef:
    fld         dword ptr [esp+60h]
    fmul        st,st(1)
    fstp        dword ptr [esp+60h]
    fld         dword ptr [esp+64h]
    fmul        st,st(1)
    fstp        dword ptr [esp+64h]
    fld         dword ptr [esp+68h]
    fmul        st,st(1)
    fstp        dword ptr [esp+68h]
    fstp        st(0)
    fld         dword ptr [K_888a8]
    fld         st(1)
    fucompp
    fnstsw      ax
    test        ah,44h
    jnp L_69b2a
    fsqrt
    fdivr       dword ptr [K_88e5c]
    jmp L_69b32
L_69b2a:
    fstp        st(0)
    fld         dword ptr [K_888a8]
L_69b32:
    fld         dword ptr [esp+50h]
    mov         dword ptr [esp+1Ch],0
    fmul        st,st(1)
    fstp        dword ptr [esp+50h]
    fld         dword ptr [esp+54h]
    fmul        st,st(1)
    fstp        dword ptr [esp+54h]
    fld         dword ptr [esp+58h]
    fmul        st,st(1)
    fstp        dword ptr [esp+58h]
    fstp        st(0)
    fld         dword ptr [esp+54h]
    fmul        dword ptr [esp+68h]
    fld         dword ptr [esp+58h]
    fmul        dword ptr [esp+64h]
    fsubp       st(1),st
    fld         dword ptr [esp+58h]
    fmul        dword ptr [esp+60h]
    fld         dword ptr [esp+68h]
    fmul        dword ptr [esp+50h]
    fsubp       st(1),st
    fstp        dword ptr [esp+14h]
    fld         dword ptr [esp+64h]
    fmul        dword ptr [esp+50h]
    fld         dword ptr [esp+54h]
    fmul        dword ptr [esp+60h]
    fsubp       st(1),st
    fstp        dword ptr [esp+18h]
    jmp L_69bc6
L_69b9a:
    fstp        st(0)
L_69b9c:
    mov         eax,dword ptr [esp+000000A4h]
    fstp        st(0)
    mov         ecx,dword ptr [esp+000000A8h]
    fld         dword ptr [esp+000000A0h]
    mov         edx,dword ptr [esp+000000ACh]
    mov         dword ptr [esp+14h],eax
    mov         dword ptr [esp+18h],ecx
    mov         dword ptr [esp+1Ch],edx
L_69bc6:
    fld         dword ptr [esp+18h]
    fmul        dword ptr [esp+18h]
    fld         dword ptr [esp+14h]
    fmul        dword ptr [esp+14h]
    faddp       st(1),st
    fld         st(1)
    fmul        st,st(2)
    faddp       st(1),st
    fld         dword ptr [K_888a8]
    fld         st(1)
    fucompp
    fnstsw      ax
    test        ah,44h
    jnp L_69bfd
    fsqrt
    fdivr       dword ptr [K_88e5c]
    fstp        dword ptr [esp+4Ch]
    jmp L_69c07
L_69bfd:
    fstp        st(0)
    mov         dword ptr [esp+4Ch],0
L_69c07:
    fmul        dword ptr [esp+4Ch]
    fst         dword ptr [esp+10h]
    fld         dword ptr [esp+14h]
    fmul        dword ptr [esp+4Ch]
    fst         dword ptr [esp+14h]
    fld         dword ptr [esp+18h]
    fmul        dword ptr [esp+4Ch]
    fstp        dword ptr [esp+18h]
    fld         dword ptr [esp+1Ch]
    fmul        dword ptr [esp+4Ch]
    fstp        dword ptr [esp+1Ch]
    jmp L_69c61
L_69c35:
    fld         dword ptr [esp+000000B0h]
    mov         eax,dword ptr [esp+000000B8h]
    mov         ecx,dword ptr [esp+000000BCh]
    fst         dword ptr [esp+10h]
    fld         dword ptr [esp+000000B4h]
    mov         dword ptr [esp+18h],eax
    fst         dword ptr [esp+14h]
    mov         dword ptr [esp+1Ch],ecx
L_69c61:
    mov         edx,dword ptr [esp+18h]
    fxch        st(1)
    mov         eax,dword ptr [esp+1Ch]
    fstp        dword ptr [esp+000000A0h]
    mov         dword ptr [esp+000000A8h],edx
    xor         edx,edx
    fstp        dword ptr [esp+000000A4h]
    mov         dx,word ptr [esi+0Ch]
    lea         ecx,[esp+00000410h]
    push        ecx
    mov         ecx,dword ptr [ebp+0Ch]
    push        ebx
    push        edi
    mov         dword ptr [esp+000000B8h],eax
    mov         eax,dword ptr [esi]
    push        eax
    push        edx
    mov         edx,dword ptr [ebp+8]
    lea         eax,[esp+24h]
    push        eax
    push        ecx
    push        edx
    call L_692a0
    mov         ecx,dword ptr [esi+4]
    xor         edx,edx
    mov         dx,word ptr [esi+0Ch]
    lea         eax,[esp+00000220h]
    push        eax
    push        ebx
    push        edi
    push        ecx
    mov         ecx,dword ptr [ebp+10h]
    push        edx
    mov         edx,dword ptr [ebp+8]
    lea         eax,[esp+44h]
    push        eax
    push        ecx
    push        edx
    call L_692a0
    add         esp,40h
    lea         eax,[esp+00000200h]
    push        eax
    xor         edx,edx
    mov         dx,word ptr [esi+0Ch]
    lea         ecx,[esp+00000414h]
    push        ecx
    mov         ecx,dword ptr [ebp+10h]
    push        ebx
    push        edi
    push        esi
    push        edx
    mov         edx,dword ptr [ecx]
    lea         eax,[esp+28h]
    push        eax
    mov         eax,dword ptr [ebp+8]
    push        edx
    push        eax
    call L_68ca0
    mov         ecx,dword ptr [esp+000000A0h]
    push        ecx
    lea         edx,[esp+00000228h]
    push        edx
    lea         eax,[esp+0000043Ch]
    push        eax
    lea         ecx,[esp+000000F4h]
    push        esi
    push        ecx
    call L_67050
    mov         eax,dword ptr [esp+00000240h]
    add         esp,38h
    test        eax,eax
    js L_69d57
    movd ecx,xmm7
    mov         edx,dword ptr [ecx]
    and         eax,7FFFFFFFh
    push        12h
    shl         eax,3
    push        eax
    mov         eax,dword ptr [esp+00000208h]
    push        eax
    call        dword ptr [edx+14h]
L_69d57:
    mov         eax,dword ptr [esp+00000418h]
    test        eax,eax
    js L_69d80
    movd ecx,xmm7
    mov         edx,dword ptr [ecx]
    and         eax,7FFFFFFFh
    push        12h
    shl         eax,3
    push        eax
    mov         eax,dword ptr [esp+00000418h]
    push        eax
    call        dword ptr [edx+14h]
L_69d80:
    mov         eax,dword ptr [esp+00000098h]
    mov         ecx,dword ptr [esp+00000080h]
    mov         esi,dword ptr [esp+00000094h]
    mov         edx,dword ptr [ecx+4]
    inc         eax
    add         esi,14h
    xor         ecx,ecx
    cmp         eax,edx
    mov         dword ptr [esp+00000098h],eax
    mov         dword ptr [esp+00000094h],esi
    jl L_69750
    mov         esi,dword ptr [esp+7Ch]
    mov         edi,dword ptr [esp+0000009Ch]
    mov         ebx,dword ptr [ebp+0Ch]
L_69dc2:
    mov         eax,dword ptr [esi+4]
    cmp         eax,1
    mov         dword ptr [esp+00000080h],esi
    jle L_69de4
    mov         edx,dword ptr [esi]
    push        offset L_65350
    dec         eax
    push        eax
    push        ecx
    push        edx
    call L_65500
    add         esp,10h
L_69de4:
    mov         ecx,dword ptr [ebp+10h]
    lea         eax,[esp+00000620h]
    push        eax
    push        esi
    push        ecx
    push        ebx
    call L_693c0
    mov         eax,dword ptr [esp+00000634h]
    add         esp,10h
    test        eax,eax
    jne L_69e11
    inc         dword ptr [esp+00000090h]
    jmp L_69664
L_69e11:
    mov         edx,dword ptr [esp+000000F8h]
    mov         eax,dword ptr [esp+00000628h]
L_69e1f:
    mov         esi,dword ptr [esp+000000F4h]
    dec         esi
    js L_69ead
    lea         ecx,[ecx]
L_69e30:
    mov         edx,dword ptr [esp+000000F0h]
    mov         edx,dword ptr [edx+esi*4]
    mov         eax,dword ptr [edx+8]
    test        eax,eax
    js L_69e5d
    movd ecx,xmm7
    mov         edx,dword ptr [edx]
    mov         edi,dword ptr [ecx]
    and         eax,7FFFFFFFh
    lea         eax,[eax+eax*4]
    push        12h
    shl         eax,2
    push        eax
    push        edx
    call        dword ptr [edi+14h]
L_69e5d:
    mov         eax,dword ptr [esp+000000F0h]
    movd ecx,xmm7
    mov         eax,dword ptr [eax+esi*4]
    mov         edx,dword ptr [ecx+2Ch]
    cmp         eax,edx
    je L_69e78
    test        edx,edx
    jne L_69e85
L_69e78:
    mov         edx,dword ptr [ecx]
    push        150h
    push        eax
    call        dword ptr [edx+28h]
    jmp L_69e9c
L_69e85:
    mov         edx,dword ptr [ecx+20h]
    mov         eax,dword ptr [ecx+24h]
    add         edx,0FFFFFEB0h
    add         eax,150h
    mov         dword ptr [ecx+20h],edx
    mov         dword ptr [ecx+24h],eax
L_69e9c:
    dec         esi
    jns L_69e30
    mov         edx,dword ptr [esp+000000F8h]
    mov         eax,dword ptr [esp+00000628h]
L_69ead:
    mov         ecx,dword ptr [esp+00000624h]
    test        ecx,ecx
    mov         dword ptr [esp+000000F4h],0
    je L_69f67
    mov         eax,dword ptr [ebp+14h]
    mov         edx,dword ptr [ebp+10h]
    push        eax
    lea         ecx,[esp+00000624h]
    push        ecx
    push        edx
    push        ebx
    call L_68700
    mov         eax,dword ptr [esp+00000638h]
    add         esp,10h
    test        eax,eax
    js L_69f09
    movd ecx,xmm7
    mov         edx,dword ptr [ecx]
    and         eax,7FFFFFFFh
    push        12h
    shl         eax,4
    push        eax
    mov         eax,dword ptr [esp+00000628h]
    push        eax
    call        dword ptr [edx+14h]
L_69f09:
    mov         eax,dword ptr [esp+000000F8h]
    test        eax,eax
    js L_69f32
    movd ecx,xmm7
    mov         edx,dword ptr [ecx]
    and         eax,7FFFFFFFh
    push        12h
    shl         eax,2
    push        eax
    mov         eax,dword ptr [esp+000000F8h]
    push        eax
    call        dword ptr [edx+14h]
L_69f32:
    mov         eax,dword ptr [esp+000000D8h]
    test        eax,eax
    js L_69f5e
    movd ecx,xmm7
    mov         edx,dword ptr [ecx]
    and         eax,7FFFFFFFh
    lea         eax,[eax+eax*4]
    push        12h
    shl         eax,2
    push        eax
    mov         eax,dword ptr [esp+000000D8h]
    push        eax
    call        dword ptr [edx+14h]
L_69f5e:
    xor         eax,eax
    pop         edi
    pop         esi
    pop         ebx
    mov         esp,ebp
    pop         ebp
    ret
L_69f67:
    test        eax,eax
    js L_69f90
    movd ecx,xmm7
    mov         edx,dword ptr [ecx]
    and         eax,7FFFFFFFh
    push        12h
    shl         eax,4
    push        eax
    mov         eax,dword ptr [esp+00000628h]
    push        eax
    call        dword ptr [edx+14h]
    mov         edx,dword ptr [esp+000000F8h]
L_69f90:
    test        edx,edx
    js L_69fb3
    movd ecx,xmm7
    mov         eax,dword ptr [ecx]
    and         edx,7FFFFFFFh
    push        12h
    shl         edx,2
    push        edx
    mov         edx,dword ptr [esp+000000F8h]
    push        edx
    call        dword ptr [eax+14h]
L_69fb3:
    mov         eax,dword ptr [esp+000000D8h]
    test        eax,eax
    js L_69fdf
    movd ecx,xmm7
    mov         edx,dword ptr [ecx]
    and         eax,7FFFFFFFh
    lea         eax,[eax+eax*4]
    push        12h
    shl         eax,2
    push        eax
    mov         eax,dword ptr [esp+000000D8h]
    push        eax
    call        dword ptr [edx+14h]
L_69fdf:
    pop         edi
    pop         esi
    mov         eax,1
    pop         ebx
    mov         esp,ebp
    pop         ebp
    ret
L_69ff0:
    sub         esp,844h
    push        ebx
    mov         ebx,dword ptr [esp+00000858h]
    push        ebp
    push        esi
    xor         esi,esi
    inc         ebx
    push        edi
    mov         edi,dword ptr [esp+00000860h]
    mov         eax,ebx
    sub         eax,edi
    cmp         eax,4
    mov         dword ptr [esp+10h],esi
    mov         ebp,edi
    mov         dword ptr [esp+14h],ebx
    jl L_6a0bf
    mov         edx,dword ptr [esp+0000085Ch]
    mov         eax,dword ptr [edx]
    mov         ecx,edi
    shl         ecx,4
    lea         edx,[ecx+eax+1Ch]
    mov         ecx,ebx
    sub         ecx,edi
    sub         ecx,4
    shr         ecx,2
    inc         ecx
    lea         ebp,[edi+ecx*4]
L_6a042:
    fld         dword ptr [K_888a8]
    fld         dword ptr [edx-10h]
    fucompp
    fnstsw      ax
    test        ah,44h
    jp L_6a05b
    mov         eax,1
    jmp L_6a05d
L_6a05b:
    xor         eax,eax
L_6a05d:
    fld         dword ptr [K_888a8]
    add         esi,eax
    fld         dword ptr [edx]
    fucompp
    fnstsw      ax
    test        ah,44h
    jp L_6a077
    mov         eax,1
    jmp L_6a079
L_6a077:
    xor         eax,eax
L_6a079:
    fld         dword ptr [K_888a8]
    add         esi,eax
    fld         dword ptr [edx+10h]
    fucompp
    fnstsw      ax
    test        ah,44h
    jp L_6a094
    mov         eax,1
    jmp L_6a096
L_6a094:
    xor         eax,eax
L_6a096:
    fld         dword ptr [K_888a8]
    add         esi,eax
    fld         dword ptr [edx+20h]
    fucompp
    fnstsw      ax
    test        ah,44h
    jp L_6a0b1
    mov         eax,1
    jmp L_6a0b3
L_6a0b1:
    xor         eax,eax
L_6a0b3:
    add         esi,eax
    add         edx,40h
    dec         ecx
    jne L_6a042
    mov         dword ptr [esp+10h],esi
L_6a0bf:
    cmp         ebp,ebx
    jge L_6a106
    mov         edx,dword ptr [esp+0000085Ch]
    mov         eax,dword ptr [edx]
    mov         ecx,ebp
    shl         ecx,4
    lea         edx,[ecx+eax+0Ch]
    mov         ecx,ebx
    sub         ecx,ebp
    lea         esp,[esp+00000000h]
L_6a0e0:
    fld         dword ptr [K_888a8]
    fld         dword ptr [edx]
    fucompp
    fnstsw      ax
    test        ah,44h
    jp L_6a0f8
    mov         eax,1
    jmp L_6a0fa
L_6a0f8:
    xor         eax,eax
L_6a0fa:
    add         esi,eax
    add         edx,10h
    dec         ecx
    jne L_6a0e0
    mov         dword ptr [esp+10h],esi
L_6a106:
    cmp         esi,3
    jle L_6a3b4
    mov         ecx,dword ptr [esp+00000864h]
    lea         eax,[edi+ecx]
    cdq
    sub         eax,edx
    mov         ebp,eax
    lea         ecx,[esp+00000444h]
    sar         ebp,1
    call L_6ad00
    mov         esi,dword ptr [esp+0000085Ch]
    mov         edx,dword ptr [esi]
    lea         ecx,[esp+34h]
    mov         dword ptr [esp+00000444h],edx
    call L_6ad00
    mov         eax,dword ptr [esi]
    mov         esi,dword ptr [esp+00000858h]
    mov         dword ptr [esp+34h],eax
    mov         al,1
    test        al,al
    mov         byte ptr [esp+10h],al
    lea         ebx,[ebp+1]
    je L_6a1a3
    nop
L_6a160:
    mov         edx,dword ptr [esp+0000085Ch]
    lea         ecx,[esp+00000444h]
    push        ecx
    push        ebp
    push        edi
    push        edx
    push        esi
    mov         dword ptr [esp+00000460h],0
    call L_69ff0
    lea         eax,[esp+24h]
    push        eax
    push        esi
    push        ebp
    lea         ecx,[esp+00000464h]
    push        edi
    push        ecx
    call L_67310
    mov         al,byte ptr [esp+38h]
    add         esp,28h
    test        al,al
    jne L_6a160
L_6a1a3:
    mov         byte ptr [esp+10h],1
    jmp L_6a1b0
    lea         ebx,[ebx+00000000h]
L_6a1b0:
    mov         eax,dword ptr [esp+00000864h]
    mov         ecx,dword ptr [esp+0000085Ch]
    lea         edx,[esp+34h]
    push        edx
    push        eax
    push        ebx
    push        ecx
    push        esi
    mov         dword ptr [esp+50h],0
    call L_69ff0
    mov         eax,dword ptr [esp+00000878h]
    lea         edx,[esp+24h]
    push        edx
    push        esi
    push        eax
    lea         ecx,[esp+54h]
    push        ebx
    push        ecx
    call L_67310
    mov         al,byte ptr [esp+38h]
    add         esp,28h
    test        al,al
    jne L_6a1b0
    mov         eax,dword ptr [esp+00000864h]
    lea         edx,[esp+30h]
    push        edx
    push        esi
    push        eax
    lea         ecx,[esp+00000450h]
    push        edi
    push        ecx
    mov         byte ptr [esp+44h],1
    call L_67310
    mov         eax,dword ptr [esp+00000878h]
    lea         edx,[esp+28h]
    push        edx
    push        esi
    push        eax
    lea         ecx,[esp+54h]
    push        edi
    push        ecx
    mov         byte ptr [esp+3Ch],1
    call L_67310
    mov         al,byte ptr [esp+58h]
    add         esp,28h
    test        al,al
    jne L_6a24e
    mov         al,byte ptr [esp+14h]
    test        al,al
    jne L_6a24e
    xor         al,al
    jmp L_6a250
L_6a24e:
    mov         al,1
L_6a250:
    test        al,al
    mov         byte ptr [esp+10h],al
    jne L_6a160
    mov         eax,dword ptr [esp+0000044Ch]
    test        eax,eax
    mov         ebp,dword ptr [esp+00000868h]
    jne L_6a2c7
    mov         eax,dword ptr [esp+3Ch]
    xor         edi,edi
    test        eax,eax
    jle L_6a342
    lea         esi,[ebp+4]
    nop
L_6a280:
    mov         eax,dword ptr [esi+8]
    mov         ecx,dword ptr [esi+4]
    mov         edx,dword ptr [esp+38h]
    and         eax,7FFFFFFFh
    cmp         ecx,eax
    lea         ebx,[edx+edi*8]
    jne L_6a2a1
    push        8
    push        esi
    call L_73600
    add         esp,8
L_6a2a1:
    mov         eax,dword ptr [ebx]
    mov         ecx,dword ptr [esi+4]
    mov         edx,dword ptr [esi]
    mov         dword ptr [edx+ecx*8],eax
    mov         eax,dword ptr [ebx+4]
    mov         dword ptr [edx+ecx*8+4],eax
    mov         edx,dword ptr [esi+4]
    mov         eax,dword ptr [esp+3Ch]
    inc         edx
    inc         edi
    cmp         edi,eax
    mov         dword ptr [esi+4],edx
    jl L_6a280
    jmp L_6a342
L_6a2c7:
    mov         ecx,dword ptr [esp+3Ch]
    test        ecx,ecx
    jne L_6a32b
    xor         edi,edi
    test        eax,eax
    jle L_6a342
    lea         esi,[ebp+4]
    lea         esp,[esp]
L_6a2e0:
    mov         edx,dword ptr [esi+8]
    mov         eax,dword ptr [esi+4]
    mov         ecx,dword ptr [esp+00000448h]
    and         edx,7FFFFFFFh
    cmp         eax,edx
    lea         ebx,[ecx+edi*8]
    jne L_6a305
    push        8
    push        esi
    call L_73600
    add         esp,8
L_6a305:
    mov         edx,dword ptr [ebx]
    mov         eax,dword ptr [esi+4]
    mov         ecx,dword ptr [esi]
    mov         dword ptr [ecx+eax*8],edx
    mov         edx,dword ptr [ebx+4]
    mov         dword ptr [ecx+eax*8+4],edx
    mov         edx,dword ptr [esi+4]
    mov         eax,dword ptr [esp+0000044Ch]
    inc         edx
    inc         edi
    cmp         edi,eax
    mov         dword ptr [esi+4],edx
    jl L_6a2e0
    jmp L_6a342
L_6a32b:
    push        ebp
    lea         eax,[esp+38h]
    push        eax
    lea         ecx,[esp+0000044Ch]
    push        ecx
    push        esi
    call L_69520
    add         esp,10h
L_6a342:
    mov         eax,dword ptr [ebp+8]
    test        eax,eax
    jne L_6a359
    lea         edx,[esp+00000448h]
    push        edx
    lea         ecx,[ebp+4]
    call L_68200
L_6a359:
    mov         eax,dword ptr [esp+40h]
    test        eax,eax
    js L_6a37c
    movd ecx,xmm7
    mov         edx,dword ptr [ecx]
    and         eax,7FFFFFFFh
    push        12h
    shl         eax,3
    push        eax
    mov         eax,dword ptr [esp+40h]
    push        eax
    call        dword ptr [edx+14h]
L_6a37c:
    mov         eax,dword ptr [esp+00000450h]
    test        eax,eax
    js L_6a4b2
    movd ecx,xmm7
    mov         edx,dword ptr [ecx]
    and         eax,7FFFFFFFh
    push        12h
    shl         eax,3
    push        eax
    mov         eax,dword ptr [esp+00000450h]
    push        eax
    call        dword ptr [edx+14h]
    pop         edi
    pop         esi
    pop         ebp
    pop         ebx
    add         esp,844h
    ret
L_6a3b4:
    mov         ecx,dword ptr [esp+0000085Ch]
    mov         edx,dword ptr [ecx]
    mov         ebx,dword ptr [esp+00000868h]
    cmp         edi,dword ptr [esp+14h]
    mov         dword ptr [ebx],edx
    lea         esi,[esp+24h]
    mov         edx,80000003h
    mov         dword ptr [esp+18h],esi
    mov         dword ptr [esp+1Ch],0
    mov         dword ptr [esp+20h],edx
    mov         ebp,edi
    jge L_6a451
    shl         edi,4
    mov         edi,edi
L_6a3f0:
    mov         eax,dword ptr [esp+0000085Ch]
    fld         dword ptr [K_888a8]
    mov         ecx,dword ptr [eax]
    fld         dword ptr [edi+ecx+0Ch]
    fucompp
    fnstsw      ax
    test        ah,44h
    jp L_6a445
    mov         eax,dword ptr [esp+1Ch]
    and         edx,7FFFFFFFh
    cmp         eax,edx
    jne L_6a42d
    lea         edx,[esp+18h]
    push        4
    push        edx
    call L_73600
    mov         esi,dword ptr [esp+20h]
    add         esp,8
L_6a42d:
    mov         eax,dword ptr [esp+1Ch]
    mov         dword ptr [esi+eax*4],ebp
    mov         eax,dword ptr [esp+1Ch]
    mov         edx,dword ptr [esp+20h]
    mov         esi,dword ptr [esp+18h]
    inc         eax
    mov         dword ptr [esp+1Ch],eax
L_6a445:
    mov         eax,dword ptr [esp+14h]
    inc         ebp
    add         edi,10h
    cmp         ebp,eax
    jl L_6a3f0
L_6a451:
    mov         eax,dword ptr [esp+10h]
    sub         eax,0
    je L_6a496
    dec         eax
    je L_6a484
    dec         eax
    je L_6a474
    mov         ecx,dword ptr [esi+8]
    mov         edx,dword ptr [esi+4]
    mov         eax,dword ptr [esi]
    push        ecx
    push        edx
    push        eax
    mov         ecx,ebx
    call L_6a880
    jmp L_6a48e
L_6a474:
    mov         ecx,dword ptr [esi+4]
    mov         edx,dword ptr [esi]
    push        ecx
    push        edx
    mov         ecx,ebx
    call L_6a7d0
    jmp L_6a48e
L_6a484:
    mov         eax,dword ptr [esi]
    push        eax
    mov         ecx,ebx
    call L_6a770
L_6a48e:
    mov         edx,dword ptr [esp+20h]
    mov         esi,dword ptr [esp+18h]
L_6a496:
    test        edx,edx
    js L_6a4b2
    movd ecx,xmm7
    mov         eax,dword ptr [ecx]
    and         edx,7FFFFFFFh
    push        12h
    shl         edx,2
    push        edx
    push        esi
    call        dword ptr [eax+14h]
L_6a4b2:
    pop         edi
    pop         esi
    pop         ebp
    pop         ebx
    add         esp,844h
    ret
L_6a4c0:
    push        ebp
    mov         ebp,esp
    and         esp,0FFFFFFF0h
    sub         esp,34h
    push        ebx
    mov         ebx,dword ptr [ebp+10h]
    test        ebx,ebx
    push        esi
    mov         esi,dword ptr [ebp+18h]
    push        edi
    mov         dword ptr [esi+4],0
    jle L_6a529
    mov         edi,dword ptr [ebp+0Ch]
    add         edi,8
L_6a4e3:
    mov         eax,dword ptr [esi+8]
    mov         ecx,dword ptr [esi+4]
    and         eax,7FFFFFFFh
    cmp         ecx,eax
    jne L_6a4fd
    push        10h
    push        esi
    call L_73600
    add         esp,8
L_6a4fd:
    mov         ecx,dword ptr [esi+4]
    mov         edx,dword ptr [esi]
    mov         eax,ecx
    shl         eax,4
    add         eax,edx
    inc         ecx
    mov         dword ptr [esi+4],ecx
    mov         ecx,dword ptr [edi-8]
    mov         dword ptr [eax],ecx
    mov         edx,dword ptr [edi-4]
    mov         dword ptr [eax+4],edx
    mov         ecx,dword ptr [edi]
    mov         dword ptr [eax+8],ecx
    mov         edx,dword ptr [edi+4]
    add         edi,10h
    dec         ebx
    mov         dword ptr [eax+0Ch],edx
    jne L_6a4e3
L_6a529:
    lea         eax,[esp+20h]
    push        eax
    lea         ecx,[esp+34h]
    push        ecx
    push        esi
    call L_658d0
    mov         eax,dword ptr [esi+4]
    add         esp,0Ch
    xor         ebx,ebx
    cmp         eax,1
    jle L_6a559
    mov         edx,dword ptr [esi]
    push        offset L_65280
    dec         eax
    push        eax
    push        ebx
    push        edx
    call L_65380
    add         esp,10h
L_6a559:
    mov         ecx,dword ptr [ebp+8]
    mov         edx,dword ptr [ecx+4]
    lea         eax,[esp+1Ch]
    push        eax
    push        esi
    push        edx
    call L_66800
    mov         edi,dword ptr [ebp+14h]
    mov         al,1
    add         esp,0Ch
    mov         byte ptr [esp+18h],al
    xor         cl,cl
    lea         esp,[esp+00000000h]
L_6a580:
    test        cl,cl
    je L_6a588
    test        al,al
    je L_6a5fa
L_6a588:
    mov         ecx,dword ptr [esi+4]
    test        al,al
    sete        byte ptr [esp+1Ch]
    xor         eax,eax
    cmp         ecx,ebx
    jle L_6a5b1
    xor         ecx,ecx
    lea         ebx,[ebx+00000000h]
L_6a5a0:
    mov         edx,dword ptr [esi]
    mov         dword ptr [ecx+edx+0Ch],ebx
    mov         edx,dword ptr [esi+4]
    inc         eax
    add         ecx,10h
    cmp         eax,edx
    jl L_6a5a0
L_6a5b1:
    mov         ecx,dword ptr [ebp+8]
    mov         dword ptr [edi+8],ebx
    mov         eax,dword ptr [esi+4]
    push        edi
    dec         eax
    push        eax
    push        ebx
    push        esi
    push        ecx
    call L_69ff0
    push        esi
    push        edi
    call L_68550
    push        esi
    call L_66bf0
    mov         eax,dword ptr [ebp+8]
    mov         ecx,dword ptr [esi+4]
    lea         edx,[esp+38h]
    push        edx
    push        eax
    dec         ecx
    push        ecx
    push        ebx
    push        edi
    call L_67310
    push        esi
    call L_66bf0
    mov         al,byte ptr [esp+50h]
    mov         cl,byte ptr [esp+54h]
    add         esp,38h
    jmp L_6a580
L_6a5fa:
    lea         edx,[esp+20h]
    push        edx
    lea         eax,[esp+34h]
    push        eax
    push        esi
    call L_65b00
    add         esp,0Ch
    pop         edi
    pop         esi
    pop         ebx
    mov         esp,ebp
    pop         ebp
    ret
L_6a620:
    sub         esp,3Ch
    push        ebx
    mov         ebx,dword ptr [esp+48h]
    push        ebp
    mov         ebp,dword ptr [esp+48h]
    push        esi
    mov         esi,dword ptr [esp+58h]
    push        edi
    mov         edi,dword ptr [esp+58h]
    push        esi
    push        edi
    push        ebx
    lea         eax,[esp+24h]
    push        ebp
    push        eax
    mov         dword ptr [esp+30h],37A7C5ACh
    mov         dword ptr [esp+38h],358637BDh
    mov         dword ptr [esp+3Ch],3727C5ACh
    mov         dword ptr [esp+40h],3D4CCCCDh
    mov         dword ptr [esp+44h],358637BDh
    mov         dword ptr [esp+48h],358637BDh
    mov         dword ptr [esp+4Ch],322BCC77h
    mov         dword ptr [esp+50h],358637BDh
    mov         dword ptr [esp+54h],38D1B717h
    mov         dword ptr [esp+58h],3727C5ACh
    mov         dword ptr [esp+5Ch],37A7C5ACh
    mov         byte ptr [esp+2Ch],0
    mov         dword ptr [esp+34h],368637BDh
    call L_6a4c0
    push        esi
    push        edi
    push        ebx
    push        ebp
    lea         ecx,[esp+3Ch]
    push        ecx
    lea         edx,[esp+3Ch]
    push        edx
    call L_6b460
    mov         al,byte ptr [esp+40h]
    add         esp,2Ch
    test        al,al
    jne L_6a761
    push        esi
    push        edi
    push        ebx
    lea         eax,[esp+24h]
    push        ebp
    push        eax
    mov         byte ptr [esp+2Ch],1
    call L_6a4c0
    push        esi
    push        edi
    push        ebx
    push        ebp
    lea         ecx,[esp+3Ch]
    push        ecx
    lea         edx,[esp+3Bh]
    push        edx
    call L_6b460
    mov         al,byte ptr [eax]
    add         esp,2Ch
    test        al,al
    mov         byte ptr [esp+14h],al
    jne L_6a761
    push        esi
    push        edi
    push        ebx
    lea         eax,[esp+24h]
    push        ebp
    push        eax
    mov         dword ptr [esp+34h],3456BF95h
    call L_6a4c0
    push        esi
    push        edi
    push        ebx
    push        ebp
    lea         ecx,[esp+3Ch]
    push        ecx
    lea         edx,[esp+3Bh]
    push        edx
    call L_6b460
    mov         al,byte ptr [eax]
    add         esp,2Ch
    test        al,al
    mov         byte ptr [esp+14h],al
    jne L_6a761
    push        esi
    push        edi
    push        ebx
    lea         eax,[esp+24h]
    push        ebp
    push        eax
    mov         dword ptr [esp+5Ch],358637BDh
    call L_6a4c0
    push        esi
    push        edi
    push        ebx
    push        ebp
    lea         ecx,[esp+3Ch]
    push        ecx
    lea         edx,[esp+3Bh]
    push        edx
    call L_6b460
    add         esp,2Ch
L_6a761:
    pop         edi
    pop         esi
    pop         ebp
    pop         ebx
    add         esp,3Ch
    ret
L_6a770:
    sub         esp,8
    mov         dx,word ptr [esp+0Ch]
    xor         eax,eax
    push        esi
    lea         esi,[ecx+4]
    mov         dword ptr [ecx+8],eax
    mov         ecx,dword ptr [esi+4]
    mov         word ptr [esp+6],ax
    mov         word ptr [esp+8],ax
    mov         eax,dword ptr [esi+8]
    and         eax,7FFFFFFFh
    cmp         ecx,eax
    mov         word ptr [esp+4],dx
    jne L_6a7aa
    push        8
    push        esi
    call L_73600
    add         esp,8
L_6a7aa:
    mov         ecx,dword ptr [esi+4]
    mov         edx,dword ptr [esi]
    mov         eax,dword ptr [esp+4]
    mov         dword ptr [edx+ecx*8],eax
    mov         eax,dword ptr [esp+8]
    mov         dword ptr [edx+ecx*8+4],eax
    inc         dword ptr [esi+4]
    pop         esi
    add         esp,8
    ret         4
L_6a7d0:
    sub         esp,10h
    mov         dx,word ptr [esp+14h]
    xor         eax,eax
    push        esi
    lea         esi,[ecx+4]
    mov         dword ptr [ecx+8],eax
    mov         ecx,dword ptr [esi+4]
    mov         word ptr [esp+4],dx
    mov         edx,1
    mov         word ptr [esp+0Eh],ax
    mov         word ptr [esp+10h],ax
    mov         eax,dword ptr [esi+8]
    mov         word ptr [esp+6],dx
    mov         word ptr [esp+8],dx
    mov         dx,word ptr [esp+1Ch]
    and         eax,7FFFFFFFh
    cmp         ecx,eax
    mov         word ptr [esp+0Ch],dx
    jne L_6a823
    push        8
    push        esi
    call L_73600
    add         esp,8
L_6a823:
    mov         ecx,dword ptr [esi+4]
    mov         edx,dword ptr [esi]
    mov         eax,dword ptr [esp+4]
    mov         dword ptr [edx+ecx*8],eax
    mov         eax,dword ptr [esp+8]
    mov         dword ptr [edx+ecx*8+4],eax
    mov         eax,dword ptr [esi+4]
    inc         eax
    mov         dword ptr [esi+4],eax
    mov         ecx,dword ptr [esi+8]
    and         ecx,7FFFFFFFh
    cmp         eax,ecx
    jne L_6a856
    push        8
    push        esi
    call L_73600
    add         esp,8
L_6a856:
    mov         edx,dword ptr [esi+4]
    mov         eax,dword ptr [esi]
    mov         ecx,dword ptr [esp+0Ch]
    mov         dword ptr [eax+edx*8],ecx
    mov         ecx,dword ptr [esp+10h]
    mov         dword ptr [eax+edx*8+4],ecx
    inc         dword ptr [esi+4]
    pop         esi
    add         esp,10h
    ret         8
L_6a880:
    push        ebp
    mov         ebp,esp
    and         esp,0FFFFFFF0h
    sub         esp,64h
    push        ebx
    mov         dword ptr [ecx+8],0
    push        esi
    mov         esi,dword ptr [ecx]
    push        edi
    mov         edi,dword ptr [ebp+0Ch]
    mov         eax,edi
    shl         eax,4
    mov         edx,dword ptr [eax+esi+8]
    fld         dword ptr [eax+esi]
    fld         dword ptr [eax+esi+4]
    add         eax,esi
    mov         dword ptr [esp+68h],edx
    fxch        st(1)
    mov         edx,dword ptr [eax+0Ch]
    mov         dword ptr [esp+6Ch],edx
    mov         edx,dword ptr [ebp+8]
    shl         edx,4
    fsub        dword ptr [edx+esi]
    add         edx,esi
    mov         ebx,dword ptr [ebp+10h]
    shl         ebx,4
    fstp        dword ptr [esp+60h]
    add         esi,ebx
    mov         ebx,dword ptr [esi+4]
    fsub        dword ptr [edx+4]
    mov         dword ptr [esp+54h],ebx
    fld         dword ptr [esp+68h]
    mov         ebx,dword ptr [esi+8]
    fsub        dword ptr [edx+8]
    mov         dword ptr [esp+58h],ebx
    fld         dword ptr [esp+6Ch]
    mov         ebx,dword ptr [esi+0Ch]
    fsub        dword ptr [edx+0Ch]
    mov         dword ptr [esp+5Ch],ebx
    fld         st(1)
    fmul        st,st(2)
    fld         st(3)
    fmul        st,st(4)
    fld         dword ptr [esp+60h]
    fmul        dword ptr [esp+60h]
    fld         st(3)
    fmul        st,st(4)
    fadd        st,st(1)
    fadd        st,st(2)
    fadd        st,st(3)
    fstp        dword ptr [esp+10h]
    fld         dword ptr [esi]
    fsub        dword ptr [eax]
    fstp        dword ptr [esp+50h]
    fld         dword ptr [esp+54h]
    fsub        dword ptr [eax+4]
    fstp        dword ptr [esp+54h]
    fld         dword ptr [esp+58h]
    fsub        dword ptr [eax+8]
    fstp        dword ptr [esp+58h]
    fld         dword ptr [esp+5Ch]
    fsub        dword ptr [eax+0Ch]
    mov         eax,dword ptr [edx+4]
    fld         dword ptr [esp+58h]
    mov         dword ptr [esp+24h],eax
    fmul        dword ptr [esp+58h]
    mov         eax,dword ptr [edx+8]
    mov         dword ptr [esp+28h],eax
    fstp        dword ptr [esp+48h]
    fld         dword ptr [esp+54h]
    fmul        dword ptr [esp+54h]
    fstp        dword ptr [esp+40h]
    fld         dword ptr [esp+50h]
    fmul        dword ptr [esp+50h]
    fstp        dword ptr [esp+38h]
    fld         st(0)
    fmul        st,st(1)
    fadd        dword ptr [esp+38h]
    fadd        dword ptr [esp+40h]
    fadd        dword ptr [esp+48h]
    fstp        dword ptr [esp+18h]
    fstp        st(0)
    fld         dword ptr [edx]
    fsub        dword ptr [esi]
    mov         edx,dword ptr [edx+0Ch]
    mov         dword ptr [esp+2Ch],edx
    fstp        dword ptr [esp+20h]
    fld         dword ptr [esp+24h]
    fsub        dword ptr [esi+4]
    fstp        dword ptr [esp+24h]
    fld         dword ptr [esp+28h]
    fsub        dword ptr [esi+8]
    fstp        dword ptr [esp+28h]
    fld         dword ptr [esp+2Ch]
    fsub        dword ptr [esi+0Ch]
    fst         dword ptr [esp+2Ch]
    fmul        dword ptr [esp+2Ch]
    fld         dword ptr [esp+28h]
    fmul        dword ptr [esp+28h]
    faddp       st(1),st
    fld         dword ptr [esp+24h]
    fmul        dword ptr [esp+24h]
    faddp       st(1),st
    fld         dword ptr [esp+20h]
    fmul        dword ptr [esp+20h]
    faddp       st(1),st
    fstp        dword ptr [esp+20h]
    fadd        st,st(1)
    fadd        st,st(2)
    fstp        st(3)
    fstp        st(0)
    fstp        st(0)
    fld         dword ptr [K_888a8]
    fld         st(1)
    fucompp
    fnstsw      ax
    test        ah,44h
    jnp L_6a9fa
    fsqrt
    fdivr       dword ptr [K_88e5c]
    jmp L_6aa02
L_6a9fa:
    fstp        st(0)
    fld         dword ptr [K_888a8]
L_6aa02:
    fld         dword ptr [esp+60h]
    fmul        st,st(1)
    fstp        dword ptr [esp+60h]
    fld         st(0)
    fmul        st,st(3)
    fstp        dword ptr [esp+64h]
    fmul        st,st(1)
    fstp        dword ptr [esp+68h]
    fstp        st(0)
    fstp        st(0)
    fld         dword ptr [esp+38h]
    fadd        dword ptr [esp+40h]
    fadd        dword ptr [esp+48h]
    fld         dword ptr [K_888a8]
    fld         st(1)
    fucompp
    fnstsw      ax
    test        ah,44h
    jnp L_6aa45
    fsqrt
    fdivr       dword ptr [K_88e5c]
    jmp L_6aa4d
L_6aa45:
    fstp        st(0)
    fld         dword ptr [K_888a8]
L_6aa4d:
    fld         dword ptr [esp+50h]
    fmul        st,st(1)
    fld         st(1)
    fmul        dword ptr [esp+54h]
    fstp        dword ptr [esp+54h]
    fxch        st(1)
    fmul        dword ptr [esp+58h]
    fstp        dword ptr [esp+58h]
    fsub        dword ptr [esp+60h]
    fld         dword ptr [esp+54h]
    fsub        dword ptr [esp+64h]
    fld         dword ptr [esp+58h]
    fsub        dword ptr [esp+68h]
    fld         st(0)
    fmul        st,st(1)
    fld         st(2)
    fmul        st,st(3)
    faddp       st(1),st
    fld         st(3)
    fmul        st,st(4)
    faddp       st(1),st
    fcomp       dword ptr [K_91474]
    fstp        st(0)
    fnstsw      ax
    fstp        st(0)
    test        ah,5
    fstp        st(0)
    jp L_6ab3d
    fld         dword ptr [esp+18h]
    fcomp       dword ptr [esp+20h]
    fnstsw      ax
    test        ah,41h
    jne L_6aab7
    fld         dword ptr [esp+18h]
    jmp L_6aabb
L_6aab7:
    fld         dword ptr [esp+20h]
L_6aabb:
    fld         dword ptr [esp+10h]
    fcomp       st(1)
    fnstsw      ax
    test        ah,41h
    jne L_6aace
    fstp        st(0)
    fld         dword ptr [esp+10h]
L_6aace:
    fld         st(0)
    fld         dword ptr [esp+10h]
    fucompp
    fnstsw      ax
    test        ah,44h
    jp L_6aaf2
    mov         eax,dword ptr [ebp+8]
    fstp        st(0)
    push        edi
    push        eax
    call L_6a7d0
    pop         edi
    pop         esi
    pop         ebx
    mov         esp,ebp
    pop         ebp
    ret         0Ch
L_6aaf2:
    fld         st(0)
    fld         dword ptr [esp+18h]
    fucompp
    fnstsw      ax
    test        ah,44h
    jp L_6ab16
    mov         edx,dword ptr [ebp+10h]
    fstp        st(0)
    push        edx
    push        edi
    call L_6a7d0
    pop         edi
    pop         esi
    pop         ebx
    mov         esp,ebp
    pop         ebp
    ret         0Ch
L_6ab16:
    fld         dword ptr [esp+20h]
    fucompp
    fnstsw      ax
    test        ah,44h
    jp L_6aceb
    mov         eax,dword ptr [ebp+8]
    mov         edx,dword ptr [ebp+10h]
    push        eax
    push        edx
    call L_6a7d0
    pop         edi
    pop         esi
    pop         ebx
    mov         esp,ebp
    pop         ebp
    ret         0Ch
L_6ab3d:
    mov         eax,dword ptr [ebp+8]
    mov         edx,dword ptr [ebp+10h]
    xor         esi,esi
    mov         word ptr [esp+42h],si
    mov         word ptr [esp+1Ch],si
    lea         esi,[ecx+4]
    mov         ecx,dword ptr [esi+4]
    mov         word ptr [esp+48h],ax
    mov         word ptr [esp+10h],ax
    mov         eax,dword ptr [esi+8]
    mov         word ptr [esp+40h],di
    mov         word ptr [esp+38h],di
    mov         ebx,3
    mov         edi,4
    and         eax,7FFFFFFFh
    cmp         ecx,eax
    mov         word ptr [esp+4Ah],1
    mov         word ptr [esp+4Ch],2
    mov         word ptr [esp+44h],5
    mov         word ptr [esp+3Ah],bx
    mov         word ptr [esp+3Ch],di
    mov         word ptr [esp+20h],dx
    mov         word ptr [esp+22h],2
    mov         word ptr [esp+24h],1
    mov         word ptr [esp+18h],dx
    mov         word ptr [esp+1Ah],5
    mov         word ptr [esp+12h],di
    mov         word ptr [esp+14h],bx
    jne L_6abd2
    push        8
    push        esi
    call L_73600
    add         esp,8
L_6abd2:
    mov         ecx,dword ptr [esi+4]
    mov         edx,dword ptr [esi]
    mov         eax,dword ptr [esp+48h]
    mov         dword ptr [edx+ecx*8],eax
    mov         eax,dword ptr [esp+4Ch]
    mov         dword ptr [edx+ecx*8+4],eax
    mov         edi,dword ptr [esi+4]
    inc         edi
    mov         dword ptr [esi+4],edi
    mov         ecx,dword ptr [esi+8]
    and         ecx,7FFFFFFFh
    mov         eax,edi
    cmp         eax,ecx
    jne L_6ac07
    push        8
    push        esi
    call L_73600
    add         esp,8
L_6ac07:
    mov         edx,dword ptr [esi+4]
    mov         eax,dword ptr [esi]
    mov         ecx,dword ptr [esp+40h]
    mov         dword ptr [eax+edx*8],ecx
    mov         ecx,dword ptr [esp+44h]
    mov         dword ptr [eax+edx*8+4],ecx
    inc         dword ptr [esi+4]
    mov         edx,dword ptr [esi+8]
    mov         eax,dword ptr [esi+4]
    and         edx,7FFFFFFFh
    cmp         eax,edx
    jne L_6ac39
    push        8
    push        esi
    call L_73600
    add         esp,8
L_6ac39:
    mov         eax,dword ptr [esi+4]
    mov         ecx,dword ptr [esi]
    mov         edx,dword ptr [esp+38h]
    mov         dword ptr [ecx+eax*8],edx
    mov         edx,dword ptr [esp+3Ch]
    mov         dword ptr [ecx+eax*8+4],edx
    mov         edi,dword ptr [esi+4]
    inc         edi
    mov         dword ptr [esi+4],edi
    mov         eax,dword ptr [esi+8]
    and         eax,7FFFFFFFh
    mov         ecx,edi
    cmp         ecx,eax
    jne L_6ac6d
    push        8
    push        esi
    call L_73600
    add         esp,8
L_6ac6d:
    mov         ecx,dword ptr [esi+4]
    mov         edx,dword ptr [esi]
    mov         eax,dword ptr [esp+20h]
    mov         dword ptr [edx+ecx*8],eax
    mov         eax,dword ptr [esp+24h]
    mov         dword ptr [edx+ecx*8+4],eax
    mov         edi,dword ptr [esi+4]
    inc         edi
    mov         dword ptr [esi+4],edi
    mov         ecx,dword ptr [esi+8]
    and         ecx,7FFFFFFFh
    mov         eax,edi
    cmp         eax,ecx
    jne L_6aca2
    push        8
    push        esi
    call L_73600
    add         esp,8
L_6aca2:
    mov         edx,dword ptr [esi+4]
    mov         eax,dword ptr [esi]
    mov         ecx,dword ptr [esp+18h]
    mov         dword ptr [eax+edx*8],ecx
    mov         ecx,dword ptr [esp+1Ch]
    mov         dword ptr [eax+edx*8+4],ecx
    inc         dword ptr [esi+4]
    mov         edx,dword ptr [esi+8]
    mov         eax,dword ptr [esi+4]
    and         edx,7FFFFFFFh
    cmp         eax,edx
    jne L_6acd4
    push        8
    push        esi
    call L_73600
    add         esp,8
L_6acd4:
    mov         eax,dword ptr [esi+4]
    mov         ecx,dword ptr [esi]
    mov         edx,dword ptr [esp+10h]
    mov         dword ptr [ecx+eax*8],edx
    mov         edx,dword ptr [esp+14h]
    mov         dword ptr [ecx+eax*8+4],edx
    inc         dword ptr [esi+4]
L_6aceb:
    pop         edi
    pop         esi
    pop         ebx
    mov         esp,ebp
    pop         ebp
    ret         0Ch
L_6ad00:
    push        esi
    push        edi
    mov         edi,ecx
    lea         esi,[edi+4]
    mov         dword ptr [edi],0
    lea         eax,[esi+0Ch]
    mov         dword ptr [esi],eax
    mov         dword ptr [esi+4],0
    mov         dword ptr [esi+8],80000080h
    mov         eax,dword ptr [esi+8]
    and         eax,7FFFFFFFh
    jge L_6ad42
    add         eax,eax
    xor         ecx,ecx
    test        eax,eax
    setle       cl
    push        8
    dec         ecx
    and         eax,ecx
    push        eax
    push        esi
    call L_73590
    add         esp,0Ch
L_6ad42:
    mov         eax,edi
    pop         edi
    mov         dword ptr [esi+4],0
    pop         esi
    ret
L_6ad50:
    mov         eax,dword ptr [esp+8]
    mov         edx,dword ptr [eax+14h]
    mov         ecx,dword ptr [eax+10h]
    mov         eax,dword ptr [eax+18h]
    movzx       ecx,word ptr [ecx]
    push        ebx
    push        ebp
    movzx       ebp,word ptr [edx]
    push        esi
    push        edi
    movzx       edi,word ptr [eax]
    mov         eax,dword ptr [esp+1Ch]
    mov         edx,dword ptr [eax+10h]
    movzx       esi,word ptr [edx]
    cmp         ecx,esi
    mov         edx,dword ptr [eax+14h]
    mov         eax,dword ptr [eax+18h]
    movzx       ebx,word ptr [edx]
    movzx       eax,word ptr [eax]
    mov         dword ptr [esp+18h],ebp
    mov         dl,1
    jne L_6adc2
    cmp         ebp,ebx
    je L_6ad92
    cmp         ebp,eax
    jne L_6ada6
L_6ad92:
    mov         ebp,dword ptr [esp+20h]
    cmp         byte ptr [ebp],0
    je L_6ad9e
    xor         dl,dl
L_6ad9e:
    mov         byte ptr [ebp],1
    mov         ebp,dword ptr [esp+18h]
L_6ada6:
    cmp         edi,ebx
    je L_6adae
    cmp         edi,eax
    jne L_6adc2
L_6adae:
    mov         ebp,dword ptr [esp+24h]
    cmp         byte ptr [ebp],0
    je L_6adba
    xor         dl,dl
L_6adba:
    mov         byte ptr [ebp],1
    mov         ebp,dword ptr [esp+18h]
L_6adc2:
    cmp         ecx,ebx
    jne L_6adea
    cmp         ebp,eax
    jne L_6add8
    mov         ecx,dword ptr [esp+20h]
    cmp         byte ptr [ecx],0
    je L_6add5
    xor         dl,dl
L_6add5:
    mov         byte ptr [ecx],1
L_6add8:
    cmp         edi,eax
    jne L_6adea
    mov         ecx,dword ptr [esp+24h]
    cmp         byte ptr [ecx],0
    je L_6ade7
    xor         dl,dl
L_6ade7:
    mov         byte ptr [ecx],1
L_6adea:
    cmp         ebp,esi
    jne L_6adf6
    cmp         edi,ebx
    je L_6adfe
    cmp         edi,eax
    je L_6adfe
L_6adf6:
    cmp         ebp,ebx
    jne L_6ae0c
    cmp         edi,eax
    jne L_6ae0c
L_6adfe:
    mov         eax,dword ptr [esp+28h]
    cmp         byte ptr [eax],0
    je L_6ae09
    xor         dl,dl
L_6ae09:
    mov         byte ptr [eax],1
L_6ae0c:
    mov         eax,dword ptr [esp+14h]
    pop         edi
    pop         esi
    pop         ebp
    mov         byte ptr [eax],dl
    pop         ebx
    ret
L_6ae20:
    mov         eax,dword ptr [esp+10h]
    mov         eax,dword ptr [eax+4]
    sub         esp,0Ch
    test        eax,eax
    mov         cl,1
    jle L_6b0e0
    mov         edx,dword ptr [esp+14h]
    push        ebx
    push        ebp
    push        esi
    push        edi
    mov         edi,dword ptr [edx+4]
    mov         edx,dword ptr [esp+28h]
    mov         edx,dword ptr [edx+4]
    mov         dword ptr [esp+10h],edi
    mov         dword ptr [esp+14h],edx
    xor         ebp,ebp
    mov         dword ptr [esp+18h],eax
    jmp L_6ae60
L_6ae56:
    mov         edi,dword ptr [esp+10h]
    lea         ebx,[ebx+00000000h]
L_6ae60:
    xor         ebx,ebx
    cmp         edi,4
    jl L_6af42
    add         edi,0FFFFFFFCh
    shr         edi,2
    xor         esi,esi
    inc         edi
    lea         ebx,[edi*4+00000000h]
    jmp L_6ae80
    lea         ecx,[ecx]
L_6ae80:
    test        cl,cl
    je L_6af32
    mov         eax,dword ptr [esp+2Ch]
    mov         edx,dword ptr [esp+24h]
    mov         edx,dword ptr [edx]
    mov         ecx,dword ptr [eax]
    fld         dword ptr [esi+edx+4]
    fmul        dword ptr [ecx+ebp+4]
    add         ecx,ebp
    fld         dword ptr [esi+edx+8]
    fmul        dword ptr [ecx+8]
    faddp       st(1),st
    fld         dword ptr [ecx]
    fmul        dword ptr [esi+edx]
    faddp       st(1),st
    fadd        dword ptr [ecx+0Ch]
    fcomp       dword ptr [esp+30h]
    fnstsw      ax
    test        ah,5
    jp L_6af32
    fld         dword ptr [esi+edx+14h]
    fmul        dword ptr [ecx+4]
    fld         dword ptr [esi+edx+18h]
    fmul        dword ptr [ecx+8]
    faddp       st(1),st
    fld         dword ptr [esi+edx+10h]
    fmul        dword ptr [ecx]
    faddp       st(1),st
    fadd        dword ptr [ecx+0Ch]
    fcomp       dword ptr [esp+30h]
    fnstsw      ax
    test        ah,5
    jp L_6af32
    fld         dword ptr [edx+esi+20h]
    fmul        dword ptr [ecx]
    fld         dword ptr [esi+edx+24h]
    fmul        dword ptr [ecx+4]
    faddp       st(1),st
    fld         dword ptr [esi+edx+28h]
    fmul        dword ptr [ecx+8]
    faddp       st(1),st
    fadd        dword ptr [ecx+0Ch]
    fcomp       dword ptr [esp+30h]
    fnstsw      ax
    test        ah,5
    jp L_6af32
    fld         dword ptr [edx+esi+30h]
    fmul        dword ptr [ecx]
    fld         dword ptr [esi+edx+34h]
    fmul        dword ptr [ecx+4]
    faddp       st(1),st
    fld         dword ptr [esi+edx+38h]
    fmul        dword ptr [ecx+8]
    faddp       st(1),st
    fadd        dword ptr [ecx+0Ch]
    fcomp       dword ptr [esp+30h]
    fnstsw      ax
    test        ah,5
    jp L_6af32
    mov         cl,1
    jmp L_6af34
L_6af32:
    xor         cl,cl
L_6af34:
    add         esi,40h
    dec         edi
    jne L_6ae80
    mov         edi,dword ptr [esp+10h]
L_6af42:
    cmp         ebx,edi
    jge L_6af96
    mov         esi,dword ptr [esp+10h]
    mov         edx,ebx
    shl         edx,4
    sub         esi,ebx
L_6af51:
    test        cl,cl
    je L_6af8e
    mov         eax,dword ptr [esp+2Ch]
    mov         ecx,dword ptr [eax]
    fld         dword ptr [ecx+ebp+8]
    lea         eax,[ecx+ebp]
    mov         ecx,dword ptr [esp+24h]
    mov         ecx,dword ptr [ecx]
    fmul        dword ptr [ecx+edx+8]
    add         ecx,edx
    fld         dword ptr [eax+4]
    fmul        dword ptr [ecx+4]
    faddp       st(1),st
    fld         dword ptr [eax]
    fmul        dword ptr [ecx]
    faddp       st(1),st
    fadd        dword ptr [eax+0Ch]
    fcomp       dword ptr [esp+30h]
    fnstsw      ax
    test        ah,5
    jp L_6af8e
    mov         cl,1
    jmp L_6af90
L_6af8e:
    xor         cl,cl
L_6af90:
    add         edx,10h
    dec         esi
    jne L_6af51
L_6af96:
    mov         edi,dword ptr [esp+14h]
    xor         ebx,ebx
    cmp         edi,4
    jl L_6b077
    add         edi,0FFFFFFFCh
    shr         edi,2
    xor         esi,esi
    inc         edi
    lea         ebx,[edi*4+00000000h]
L_6afb5:
    test        cl,cl
    je L_6b067
    mov         eax,dword ptr [esp+2Ch]
    mov         edx,dword ptr [esp+28h]
    mov         edx,dword ptr [edx]
    mov         ecx,dword ptr [eax]
    fld         dword ptr [edx+esi+8]
    fmul        dword ptr [ecx+ebp+8]
    add         ecx,ebp
    fld         dword ptr [edx+esi+4]
    fmul        dword ptr [ecx+4]
    faddp       st(1),st
    fld         dword ptr [edx+esi]
    fmul        dword ptr [ecx]
    faddp       st(1),st
    fadd        dword ptr [ecx+0Ch]
    fcomp       dword ptr [esp+30h]
    fnstsw      ax
    test        ah,5
    jp L_6b067
    fld         dword ptr [edx+esi+18h]
    fmul        dword ptr [ecx+8]
    fld         dword ptr [edx+esi+14h]
    fmul        dword ptr [ecx+4]
    faddp       st(1),st
    fld         dword ptr [edx+esi+10h]
    fmul        dword ptr [ecx]
    faddp       st(1),st
    fadd        dword ptr [ecx+0Ch]
    fcomp       dword ptr [esp+30h]
    fnstsw      ax
    test        ah,5
    jp L_6b067
    fld         dword ptr [edx+esi+28h]
    fmul        dword ptr [ecx+8]
    fld         dword ptr [edx+esi+24h]
    fmul        dword ptr [ecx+4]
    faddp       st(1),st
    fld         dword ptr [edx+esi+20h]
    fmul        dword ptr [ecx]
    faddp       st(1),st
    fadd        dword ptr [ecx+0Ch]
    fcomp       dword ptr [esp+30h]
    fnstsw      ax
    test        ah,5
    jp L_6b067
    fld         dword ptr [edx+esi+38h]
    fmul        dword ptr [ecx+8]
    fld         dword ptr [edx+esi+34h]
    fmul        dword ptr [ecx+4]
    faddp       st(1),st
    fld         dword ptr [edx+esi+30h]
    fmul        dword ptr [ecx]
    faddp       st(1),st
    fadd        dword ptr [ecx+0Ch]
    fcomp       dword ptr [esp+30h]
    fnstsw      ax
    test        ah,5
    jp L_6b067
    mov         cl,1
    jmp L_6b069
L_6b067:
    xor         cl,cl
L_6b069:
    add         esi,40h
    dec         edi
    jne L_6afb5
    mov         edi,dword ptr [esp+14h]
L_6b077:
    cmp         ebx,edi
    jge L_6b0ca
    mov         esi,dword ptr [esp+14h]
    mov         edx,ebx
    shl         edx,4
    sub         esi,ebx
L_6b086:
    test        cl,cl
    je L_6b0c2
    mov         eax,dword ptr [esp+28h]
    mov         ecx,dword ptr [eax]
    mov         eax,dword ptr [esp+2Ch]
    fld         dword ptr [ecx+edx+8]
    mov         eax,dword ptr [eax]
    fmul        dword ptr [eax+ebp+8]
    add         ecx,edx
    fld         dword ptr [ecx+4]
    add         eax,ebp
    fmul        dword ptr [eax+4]
    faddp       st(1),st
    fld         dword ptr [ecx]
    fmul        dword ptr [eax]
    faddp       st(1),st
    fadd        dword ptr [eax+0Ch]
    fcomp       dword ptr [esp+30h]
    fnstsw      ax
    test        ah,5
    jp L_6b0c2
    mov         cl,1
    jmp L_6b0c4
L_6b0c2:
    xor         cl,cl
L_6b0c4:
    add         edx,10h
    dec         esi
    jne L_6b086
L_6b0ca:
    mov         eax,dword ptr [esp+18h]
    add         ebp,10h
    dec         eax
    mov         dword ptr [esp+18h],eax
    jne L_6ae56
    pop         edi
    pop         esi
    pop         ebp
    pop         ebx
L_6b0e0:
    mov         eax,dword ptr [esp+10h]
    mov         byte ptr [eax],cl
    add         esp,0Ch
    ret
L_6b0f0:
    sub         esp,18h
    mov         eax,dword ptr [esp+34h]
    mov         ecx,dword ptr [esp+2Ch]
    mov         edx,dword ptr [esp+28h]
    push        ebp
    push        eax
    mov         eax,dword ptr [esp+28h]
    push        ecx
    push        edx
    push        eax
    lea         ecx,[esp+17h]
    push        ecx
    call L_6ae20
    mov         cl,byte ptr [eax]
    mov         eax,dword ptr [esp+48h]
    mov         ebp,dword ptr [eax+4]
    add         esp,14h
    test        cl,cl
    setne       dl
    xor         ecx,ecx
    test        ebp,ebp
    mov         byte ptr [esp+8],dl
    mov         dword ptr [esp+18h],ecx
    jle L_6b1dd
    push        ebx
    push        esi
    push        edi
    xor         ebx,ebx
    lea         ebx,[ebx+00000000h]
L_6b140:
    xor         edi,edi
    test        ebp,ebp
    mov         byte ptr [esp+18h],0
    mov         byte ptr [esp+1Ch],0
    mov         byte ptr [esp+20h],0
    jle L_6b191
    xor         esi,esi
L_6b157:
    cmp         edi,ecx
    je L_6b189
    mov         ecx,dword ptr [esp+40h]
    mov         eax,dword ptr [ecx]
    lea         edx,[esp+20h]
    push        edx
    lea         ecx,[esp+20h]
    push        ecx
    lea         edx,[esp+20h]
    push        edx
    lea         ecx,[esi+eax]
    push        ecx
    lea         edx,[ebx+eax]
    push        edx
    lea         eax,[esp+27h]
    push        eax
    call L_6ad50
    mov         ecx,dword ptr [esp+3Ch]
    add         esp,18h
L_6b189:
    inc         edi
    add         esi,20h
    cmp         edi,ebp
    jl L_6b157
L_6b191:
    mov         al,byte ptr [esp+14h]
    test        al,al
    je L_6b1b5
    mov         al,byte ptr [esp+18h]
    test        al,al
    je L_6b1b5
    mov         al,byte ptr [esp+1Ch]
    test        al,al
    je L_6b1b5
    mov         al,byte ptr [esp+20h]
    test        al,al
    je L_6b1b5
    mov         al,1
    jmp L_6b1b7
L_6b1b5:
    xor         al,al
L_6b1b7:
    inc         ecx
    add         ebx,20h
    cmp         ecx,ebp
    mov         byte ptr [esp+14h],al
    mov         dword ptr [esp+24h],ecx
    jl L_6b140
    mov         eax,dword ptr [esp+2Ch]
    mov         cl,byte ptr [esp+14h]
    pop         edi
    pop         esi
    pop         ebx
    mov         byte ptr [eax],cl
    pop         ebp
    add         esp,18h
    ret
L_6b1dd:
    mov         eax,dword ptr [esp+20h]
    mov         byte ptr [eax],dl
    pop         ebp
    add         esp,18h
    ret
L_6b1f0:
    push        ebp
    mov         ebp,esp
    and         esp,0FFFFFFF0h
    sub         esp,34h
    mov         eax,dword ptr [ebp+20h]
    mov         ecx,dword ptr [ebp+18h]
    mov         edx,dword ptr [ebp+14h]
    push        ebx
    push        esi
    push        edi
    push        eax
    mov         eax,dword ptr [ebp+0Ch]
    push        ecx
    push        edx
    push        eax
    lea         ecx,[esp+22h]
    push        ecx
    call L_6ae20
    mov         cl,byte ptr [eax]
    mov         edx,dword ptr [ebp+1Ch]
    mov         edi,dword ptr [edx+4]
    add         esp,14h
    test        cl,cl
    setne       cl
    xor         ebx,ebx
    test        edi,edi
    mov         dword ptr [esp+28h],ebx
    mov         dword ptr [esp+14h],edi
    jle L_6b418
    xor         edx,edx
    mov         dword ptr [esp+2Ch],edx
    mov         edi,edi
L_6b240:
    test        cl,cl
    je L_6b267
    mov         eax,dword ptr [ebp+1Ch]
    mov         ecx,dword ptr [eax]
    mov         esi,dword ptr [edx+ecx+10h]
    lea         eax,[edx+ecx]
    mov         ecx,dword ptr [eax+14h]
    mov         cx,word ptr [ecx]
    cmp         word ptr [esi],cx
    je L_6b267
    mov         eax,dword ptr [eax+18h]
    cmp         cx,word ptr [eax]
    je L_6b267
    mov         al,1
    jmp L_6b269
L_6b267:
    xor         al,al
L_6b269:
    xor         esi,esi
    test        edi,edi
    mov         byte ptr [esp+1Ch],0
    mov         byte ptr [esp+20h],0
    mov         byte ptr [esp+24h],0
    jle L_6b3e2
    xor         ecx,ecx
    mov         dword ptr [esp+18h],ecx
L_6b288:
    cmp         esi,ebx
    je L_6b3ce
    test        al,al
    je L_6b3c4
    mov         eax,dword ptr [ebp+1Ch]
    mov         eax,dword ptr [eax]
    mov         ebx,dword ptr [ecx+eax+10h]
    add         ecx,eax
    add         edx,eax
    mov         eax,dword ptr [edx+10h]
    mov         ax,word ptr [eax]
    cmp         ax,word ptr [ebx]
    jne L_6b39c
    mov         eax,dword ptr [edx+14h]
    mov         ebx,dword ptr [ecx+14h]
    mov         ax,word ptr [eax]
    cmp         ax,word ptr [ebx]
    jne L_6b39c
    mov         eax,dword ptr [edx+18h]
    mov         ebx,dword ptr [ecx+18h]
    mov         ax,word ptr [eax]
    cmp         ax,word ptr [ebx]
    jne L_6b39c
    fld         dword ptr [edx]
    mov         byte ptr [esp+12h],1
    fsub        dword ptr [ecx]
    fld         dword ptr [edx+4]
    fsub        dword ptr [ecx+4]
    fld         dword ptr [edx+8]
    fsub        dword ptr [ecx+8]
    fstp        dword ptr [esp+38h]
    fld         dword ptr [edx+0Ch]
    fsub        dword ptr [ecx+0Ch]
    fstp        dword ptr [esp+3Ch]
    fxch        st(1)
    fabs
    fstp        dword ptr [esp+30h]
    fabs
    fld         dword ptr [esp+38h]
    fabs
    fstp        dword ptr [esp+38h]
    fld         dword ptr [esp+3Ch]
    fabs
    fstp        dword ptr [esp+3Ch]
    fld         dword ptr [K_88eb8]
    fcomp       dword ptr [esp+30h]
    fnstsw      ax
    test        ah,5
    jnp L_6b32f
    mov         byte ptr [esp+12h],0
L_6b32f:
    fld         dword ptr [K_88eb8]
    mov         byte ptr [esp+11h],1
    fcomp       st(1)
    fnstsw      ax
    fstp        st(0)
    test        ah,5
    jnp L_6b34a
    mov         byte ptr [esp+11h],0
L_6b34a:
    fld         dword ptr [K_88eb8]
    fcomp       dword ptr [esp+38h]
    fnstsw      ax
    test        ah,5
    jp L_6b35f
    mov         bl,1
    jmp L_6b361
L_6b35f:
    xor         bl,bl
L_6b361:
    fld         dword ptr [K_88eb8]
    fcomp       dword ptr [esp+3Ch]
    fnstsw      ax
    test        ah,5
    jp L_6b376
    mov         al,1
    jmp L_6b378
L_6b376:
    xor         al,al
L_6b378:
    movzx       edi,bl
    movzx       ebx,byte ptr [esp+11h]
    shl         edi,1
    shl         ebx,2
    or          edi,ebx
    movzx       ebx,byte ptr [esp+12h]
    shl         ebx,3
    movzx       eax,al
    or          edi,ebx
    or          edi,eax
    mov         edi,dword ptr [esp+14h]
    je L_6b3c4
L_6b39c:
    lea         eax,[esp+24h]
    push        eax
    lea         eax,[esp+24h]
    push        eax
    lea         eax,[esp+24h]
    push        eax
    push        ecx
    push        edx
    lea         ecx,[esp+27h]
    push        ecx
    call L_6ad50
    mov         cl,byte ptr [eax]
    add         esp,18h
    test        cl,cl
    je L_6b3c4
    mov         al,1
    jmp L_6b3c6
L_6b3c4:
    xor         al,al
L_6b3c6:
    mov         edx,dword ptr [esp+2Ch]
    mov         ebx,dword ptr [esp+28h]
L_6b3ce:
    mov         ecx,dword ptr [esp+18h]
    inc         esi
    add         ecx,20h
    cmp         esi,edi
    mov         dword ptr [esp+18h],ecx
    jl L_6b288
L_6b3e2:
    test        al,al
    je L_6b402
    mov         al,byte ptr [esp+1Ch]
    test        al,al
    je L_6b402
    mov         al,byte ptr [esp+20h]
    test        al,al
    je L_6b402
    mov         al,byte ptr [esp+24h]
    test        al,al
    je L_6b402
    mov         cl,1
    jmp L_6b404
L_6b402:
    xor         cl,cl
L_6b404:
    inc         ebx
    add         edx,20h
    cmp         ebx,edi
    mov         dword ptr [esp+28h],ebx
    mov         dword ptr [esp+2Ch],edx
    jl L_6b240
L_6b418:
    mov         edx,dword ptr [ebp+14h]
    mov         esi,dword ptr [edx+4]
    cmp         esi,2
    jle L_6b44c
    test        cl,cl
    je L_6b44a
    mov         eax,dword ptr [ebp+10h]
    mov         eax,dword ptr [eax+8]
    cdq
    sub         eax,edx
    sar         eax,1
    sub         edi,eax
    lea         ecx,[edi+esi-2]
    test        ecx,ecx
    jne L_6b44a
    mov         eax,dword ptr [ebp+8]
    mov         cl,1
    mov         byte ptr [eax],cl
    pop         edi
    pop         esi
    pop         ebx
    mov         esp,ebp
    pop         ebp
    ret
L_6b44a:
    xor         cl,cl
L_6b44c:
    mov         eax,dword ptr [ebp+8]
    pop         edi
    pop         esi
    mov         byte ptr [eax],cl
    pop         ebx
    mov         esp,ebp
    pop         ebp
    ret
L_6b460:
    push        ebp
    mov         ebp,esp
    and         esp,0FFFFFFF0h
    sub         esp,74h
    mov         ecx,dword ptr [ebp+0Ch]
    mov         edx,dword ptr [ecx+14h]
    mov         ecx,dword ptr [ebp+14h]
    push        ebx
    xor         ebx,ebx
    cmp         ecx,ebx
    mov         eax,80000000h
    push        esi
    push        edi
    mov         byte ptr [esp+14h],bl
    mov         dword ptr [esp+34h],ebx
    mov         dword ptr [esp+38h],ebx
    mov         dword ptr [esp+3Ch],eax
    mov         dword ptr [esp+28h],ebx
    mov         dword ptr [esp+2Ch],ebx
    mov         dword ptr [esp+30h],eax
    mov         dword ptr [esp+18h],edx
    mov         dword ptr [esp+1Ch],ebx
    mov         dword ptr [esp+20h],ebx
    mov         dword ptr [esp+24h],eax
    jle L_6b50c
    mov         esi,dword ptr [ebp+10h]
    add         esi,8
    mov         edi,ecx
    jmp L_6b4c0
L_6b4b6:
    mov         eax,dword ptr [esp+24h]
    lea         ebx,[ebx+00000000h]
L_6b4c0:
    mov         ecx,dword ptr [esp+20h]
    and         eax,7FFFFFFFh
    cmp         ecx,eax
    jne L_6b4dc
    lea         eax,[esp+1Ch]
    push        10h
    push        eax
    call L_73600
    add         esp,8
L_6b4dc:
    mov         ecx,dword ptr [esp+20h]
    mov         edx,dword ptr [esp+1Ch]
    mov         eax,ecx
    shl         eax,4
    add         eax,edx
    inc         ecx
    mov         dword ptr [esp+20h],ecx
    mov         ecx,dword ptr [esi-8]
    mov         dword ptr [eax],ecx
    mov         edx,dword ptr [esi-4]
    mov         dword ptr [eax+4],edx
    mov         ecx,dword ptr [esi]
    mov         dword ptr [eax+8],ecx
    mov         edx,dword ptr [esi+4]
    add         esi,10h
    dec         edi
    mov         dword ptr [eax+0Ch],edx
    jne L_6b4b6
L_6b50c:
    mov         esi,dword ptr [ebp+1Ch]
    lea         eax,[esp+40h]
    push        eax
    lea         ecx,[esp+54h]
    push        ecx
    push        esi
    call L_658d0
    lea         edx,[esp+6Ch]
    push        edx
    lea         eax,[esp+00000080h]
    push        eax
    lea         ecx,[esp+30h]
    push        ecx
    call L_658d0
    mov         edi,dword ptr [ebp+18h]
    lea         edx,[esp+2Ch]
    push        edx
    mov         edx,dword ptr [ebp+0Ch]
    lea         eax,[esp+50h]
    push        eax
    lea         ecx,[esp+48h]
    push        ecx
    push        esi
    push        edi
    push        edx
    lea         eax,[esp+43h]
    push        eax
    call L_67a90
    mov         al,byte ptr [esp+48h]
    add         esp,34h
    cmp         al,bl
    je L_6b585
    mov         ecx,dword ptr [esp+18h]
    push        ecx
    lea         edx,[esp+38h]
    push        edx
    lea         eax,[esp+30h]
    push        eax
    push        esi
    push        edi
    lea         ecx,[esp+30h]
    push        ecx
    lea         edx,[esp+2Bh]
    push        edx
    call L_6b0f0
    jmp L_6b5a5
L_6b585:
    mov         eax,dword ptr [esp+18h]
    push        eax
    lea         ecx,[esp+38h]
    push        ecx
    lea         edx,[esp+30h]
    push        edx
    push        esi
    push        edi
    lea         eax,[esp+30h]
    push        eax
    lea         ecx,[esp+2Bh]
    push        ecx
    call L_6b1f0
L_6b5a5:
    mov         bl,byte ptr [eax]
    add         esp,1Ch
    lea         edx,[esp+40h]
    push        edx
    lea         eax,[esp+54h]
    push        eax
    push        esi
    call L_65b00
    lea         ecx,[esp+6Ch]
    push        ecx
    lea         edx,[esp+00000080h]
    push        edx
    lea         eax,[esp+30h]
    push        eax
    call L_65b00
    mov         eax,dword ptr [esp+3Ch]
    mov         esi,dword ptr [ebp+8]
    add         esp,18h
    test        eax,eax
    mov         byte ptr [esi],bl
    js L_6b5fc
    movd ecx,xmm7
    mov         edx,dword ptr [ecx]
    and         eax,7FFFFFFFh
    push        12h
    shl         eax,4
    push        eax
    mov         eax,dword ptr [esp+24h]
    push        eax
    call        dword ptr [edx+14h]
L_6b5fc:
    mov         eax,dword ptr [esp+30h]
    test        eax,eax
    js L_6b61f
    movd ecx,xmm7
    mov         edx,dword ptr [ecx]
    and         eax,7FFFFFFFh
    push        12h
    shl         eax,4
    push        eax
    mov         eax,dword ptr [esp+30h]
    push        eax
    call        dword ptr [edx+14h]
L_6b61f:
    mov         eax,dword ptr [esp+3Ch]
    test        eax,eax
    js L_6b642
    movd ecx,xmm7
    mov         edx,dword ptr [ecx]
    and         eax,7FFFFFFFh
    push        12h
    shl         eax,5
    push        eax
    mov         eax,dword ptr [esp+3Ch]
    push        eax
    call        dword ptr [edx+14h]
L_6b642:
    pop         edi
    mov         eax,esi
    pop         esi
    pop         ebx
    mov         esp,ebp
    pop         ebp
    pushfd
    push ecx
    push edx
    movd ecx,xmm7
    movzx edx,byte ptr [eax]
    mov dword ptr [ecx+84],edx
    inc dword ptr [ecx+88]
    pop edx
    pop ecx
    popfd
    ret
L_71b10:
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
L_71c10:
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
    call L_71b10
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
L_73590:
    movd ecx,xmm7
    mov         eax,dword ptr [ecx]
    push        ebx
    mov         ebx,dword ptr [esp+10h]
    push        ebp
    mov         ebp,dword ptr [esp+10h]
    mov         edx,ebp
    imul        edx,ebx
    push        esi
    push        edi
    push        12h
    push        edx
    call        dword ptr [eax+10h]
    mov         esi,dword ptr [esp+14h]
    mov         ecx,dword ptr [esi]
    mov         edi,eax
    mov         eax,dword ptr [esi+4]
    imul        eax,ebx
    push        eax
    push        ecx
    push        edi
    call L_739c0
    mov         eax,dword ptr [esi+8]
    add         esp,0Ch
    test        eax,eax
    js L_735e8
    movd ecx,xmm7
    mov         edx,dword ptr [ecx]
    and         eax,7FFFFFFFh
    imul        eax,ebx
    push        12h
    push        eax
    mov         eax,dword ptr [esi]
    push        eax
    call        dword ptr [edx+14h]
L_735e8:
    mov         dword ptr [esi],edi
    pop         edi
    mov         dword ptr [esi+8],ebp
    pop         esi
    pop         ebp
    pop         ebx
    ret
L_73600:
    push        ebx
    push        ebp
    push        esi
    mov         esi,dword ptr [esp+10h]
    mov         eax,dword ptr [esi+4]
    test        eax,eax
    push        edi
    lea         ebp,[eax+eax]
    jne L_73617
    mov         ebp,1
L_73617:
    mov         ebx,dword ptr [esp+18h]
    movd ecx,xmm7
    mov         eax,dword ptr [ecx]
    mov         edx,ebp
    imul        edx,ebx
    push        12h
    push        edx
    call        dword ptr [eax+10h]
    mov         ecx,dword ptr [esi]
    mov         edi,eax
    mov         eax,dword ptr [esi+4]
    imul        eax,ebx
    push        eax
    push        ecx
    push        edi
    call L_739c0
    mov         eax,dword ptr [esi+8]
    add         esp,0Ch
    test        eax,eax
    js L_73663
    movd ecx,xmm7
    mov         edx,dword ptr [ecx]
    and         eax,7FFFFFFFh
    imul        eax,ebx
    push        12h
    push        eax
    mov         eax,dword ptr [esi]
    push        eax
    call        dword ptr [edx+14h]
L_73663:
    mov         dword ptr [esi],edi
    pop         edi
    mov         dword ptr [esi+8],ebp
    pop         esi
    pop         ebp
    pop         ebx
    ret
L_739c0:
    mov         ecx,dword ptr [esp+0Ch]
    push        esi
    mov         esi,dword ptr [esp+0Ch]
    mov         eax,ecx
    push        edi
    mov         edi,dword ptr [esp+0Ch]
    shr         ecx,2
    rep movs    dword ptr es:[edi],dword ptr [esi]
    mov         ecx,eax
    and         ecx,3
    rep movs    byte ptr es:[edi],byte ptr [esi]
    pop         edi
    pop         esi
    ret
  }
}
}}
