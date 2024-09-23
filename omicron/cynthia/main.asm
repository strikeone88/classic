/*
    MAIN.ASM

    Cynthia Flat-Mode Platform Initializer Version 0.01

    Copyright (C) 2008 RedStar Technologies
    Written by J. Palencia (ilina@bloodykisses.zzn.com)
*/

Section Text Word 'Code'

/****************************************************************************/
[Text]

Start:
    push    Intro
    call    .Print

    call    EnableA20

    mov     edx, cr0
    test    dl, 1
    jnz     Abort

    inc     dl

    in      al, 021h
    mov     [PicMaskA], al
    in      al, 0A1h
    mov     [PicMaskB], al

    mov     al, 0FFh
    out     021h, al
    out     0A1h, al

    mov     ebx, cs
    shl     ebx, 4

    mov     word ptr [GDTR], GDT.End - GDT.Start - 1

    lea     ecx, [ebx+GDT.Start]
    mov     [GDTR+2], ecx

    mov     [Code32+0], dword 0000FFFFh
    mov     [Data32+0], dword 0000FFFFh
    mov     [Code32+4], dword 00CF9A00h
    mov     [Data32+4], dword 00CF9200h

    lgdt    [GDTR]

    mov     cr0, edx

    mov     cx, 10h
    mov     ds, cx
    mov     es, cx
    mov     fs, cx
    mov     gs, cx

    mov     ss, cx
    mov     esp, 9C000h

    add     [ebx+$$+2], ebx

    dw      0EA66h
    dd      $+6
    dw      08h

    bits    32

    lea     esi, [ebx+Bottom]
    cmp     [esi], dword 'CFX!'
    jne     ExitProtected

    push    esi
    mov     eax, [esi+8]
    mov     ecx, [esi+4]
    mov     edi, [esi+18h]
    lea     esi, [esi+edi]
    mov     edi, 100000h
    mov     edx, esi
    rep     movsb
    pop     esi

    push    edx

    mov     ecx, eax
    lea     edi, [esi + 20h]
    mov     edx, 100000h
    jecxz   Done

Fix:
    mov     eax, [edi]
    add     edi, 4
    add     [eax+edx], edx
    loop    Fix

Done:
    pop     edi
    mov     eax, [esi+14h]
    mov     esi, [esi+4]
    lea     ecx, [ebx+GDT.Start]
    add     eax, edx
    push    ecx
    call    eax

ExitProtected:
    bits    16
    cli
    jmp     $

Abort:
    push    Unable
    call    .Print

    sti
    jmp     .Exit

    PicMaskA    db  0
    PicMaskB    db  0

EnableA20:
    cli

w1: in      al, 64h
    test    al, 2
    jnz     w1

    mov     al, 0D0h
    out     64h, al

w2: in      al, 64h
    test    al, 1
    jz      w2

    in      al, 60h
    or      al, 2
    mov     ah, al

w3: in      al, 64h
    test    al, 2
    jnz     w3

    mov     al, 0D1h
    out     64h, al

w4: in      al, 64h
    test    al, 2
    jnz     w4

    mov     al, ah
    out     60h, al

w5: in      al, 64h
    test    al, 2
    jnz     w5

    mov     al, 0D0h
    out     64h, al

w6: in      al, 64h
    test    al, 1
    jz      w6

    in      al, 60h
    test    al, 2
    jnz     EnableA20.Ret

    push    A20NotReady
    call    .Print

EnableA20.Ret:
    ret

/****************************************************************************/

.Print:
    pop     si
    pop     dx
    mov     ah, 09h
    int     21h
    jmp     si

.Exit:
    mov     ax, 4C00h
    int     21h

/****************************************************************************/

Intro:
    db      "RedStar Cynthia Version 0.01 Copyright (C) 2008 ilina@bloodykisses.zzn.com", 13, 10, 36

Unable:
    db      "Error: Unable to prepare platform", 13, 10, 13, 10, 36

A20NotReady:
    db      "Warning: A20 line was not enabled!", 13, 10, 36

Bottom:

/****************************************************************************/

ORG Intro
GDTR:

ORG $+6

ORG $+6
GDT.Start:

ORG $+8
Code32: ;08

ORG $+8
Data32: ;10

ORG $+8 ;18

ORG $ + 8*(16 - 2)
GDT.End:

[Ends]
