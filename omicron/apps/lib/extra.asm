Section _TEXT byte public 'CODE'
Publics _
Bits    32

[_TEXT]

_outportb:
    mov     edx, [esp+4]
    mov     eax, [esp+8]
    out     dx, al
    ret

_outportw:
    mov     edx, [esp+4]
    mov     eax, [esp+8]
    out     dx, ax
    ret

_outportd:
    mov     edx, [esp+4]
    mov     eax, [esp+8]
    out     dx, eax
    ret

_inportb:
    sub     eax, eax
    mov     edx, [esp+4]
    in      al, dx
    ret

_inportw:
    sub     eax, eax
    mov     edx, [esp+4]
    in      ax, dx
    ret

_inportd:
    mov     edx, [esp+4]
    in      eax, dx
    ret

_disable:
    cli
    ret

_enable:
    sti
    ret

_distance:
    fild    d[esp+4]
    fld     st0
    fmulp   st1,st0
    fild    d[esp+8]
    fld     st0
    fmulp   st1,st0
    faddp   st1,st0
    fsqrt
    fistp   d[esp+4]
    mov     eax, [esp+4]
    ret

_addBuffer:
    push    ebp
    mov     ebp, esp

    push    esi
    push    edi

    mov     edi, [ebp+8]
    mov     esi, [ebp+12]
    mov     ebx, [ebp+16]
    mov     edx, [ebp+20]
    shr     ebx, 1
    mov     ebp, [ebp+24]

YCycle:
    mov     ecx, ebx

XCycle:
    movq    mm0, q[esi]
    add     esi, 8
    movq    mm1, q[edi]
    add     edi, 8
    paddusb mm0, mm1
    movq    q[edi-8], mm0

    dec     ecx
    jnz     XCycle

    add     edi, ebp

    dec     edx
    jnz     YCycle

    pop     edi
    pop     esi
    pop     ebp
    ret

_clearBuf:
    mov     ecx, [esp+8]
    mov     edi, [esp+4]

    sub     eax, eax
    push    eax
    push    eax

    movq    mm0, [esp]

    add     esp, 8

    pushfd
    cli

Cycle1:
    movq    [edi], mm0
    add     edi, 8
    dec     ecx
    jnz     Cycle1

    popfd
    ret

_FlipFrameBuffer:
    Push    Esi
    Push    Edi
    Push    Ebx

    Mov     Eax, 11h

    Mov     Esi, [Esp+10h]
    Mov     Edx, 3CEh
    Mov     Ebx, 009h

Flip.Cicle:
    Out     Dx, Ax
    Mov     Edi, 0A0000h
    Mov     Ecx, 2000h
    Add     Ax, 100h

Flip.InnerLoopA:
    MovQ    MM0, [Esi]
    Add     Esi, 08h
    MovQ    [Edi], MM0
    Add     Edi, 08h
    Dec     Ecx
    Jnz     Flip.InnerLoopA

    Dec     Ebx
    Jnz     Flip.Cicle

    Out     Dx, Ax
    Mov     Edi, 0A0000h
    Mov     Ecx, 0C00h
    Add     Ax, 100h

Flip.InnerLoopB:
    MovQ    MM0, [Esi]
    Add     Esi, 08h
    MovQ    [Edi], MM0
    Add     Edi, 08h
    Dec     Ecx
    Jnz     Flip.InnerLoopB

    Emms

    Pop     Ebx
    Pop     Edi
    Pop     Esi

    Ret

[Ends]
