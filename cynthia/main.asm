/*
    MAIN.ASM

    Cynthia Flat-Mode Platform's Initializer Version 0.01b

    Copyright (C) 2008-2009 RedStar Technologies
    Written by J. Palencia (ilina@bloodykisses.zzn.com)
*/

Section Text Word 'Code'

/****************************************************************************/
[Text]

Start:
    mov     edx, cr0
    test    dl, 1
    jnz     Abort
    or      dl, 01h
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
    jnz     EnableA20.Done

    push    A20NotReady
    call    .Print

EnableA20.Done:
    mov     al, 0FFh
    out     021h, al
    out     0A1h, al

    mov     ebx, cs
    shl     ebx, 4

    mov     word ptr [GDTR], GDT.End - GDT.Start - 1

    lea     ecx, [ebx+GDT.Start]
    mov     [GDTR+02h], ecx

    mov     [Code32+00h], dword 0000FFFFh
    mov     [Data32+00h], dword 0000FFFFh
    mov     [Code32+04h], dword 00CF9A00h
    mov     [Data32+04h], dword 00CF9200h

    lgdt    [GDTR]

    mov     cr0, edx

    mov     cx, 10h
    mov     ds, cx
    mov     es, cx
    mov     fs, cx
    mov     gs, cx

    mov     ss, cx
    mov     esp, 0A0000h

    add     [ebx+$$+02h], ebx

    dw      0EA66h
    dd      $+6
    dw      08h

    bits    32

    lea     esi, [ebx+0200h]
    mov     ecx, 08DE00h
    mov     edi, 100000h
    rep     movsb

    lea     ebx, [ebx+GDT.Start]

    push    ebx
    jmp     100000h

    bits    16

Abort:
    push    Unable
    call    .Print

    sti
    jmp     .Exit

/****************************************************************************/

.Print:
    pop     di
    pop     si
    mov     ah, 0Eh
    sub     bx, bx

.PrintC:
    mov     al, [si]
    inc     si
    test    al, al
    jz      .PrintC_Done
    int     10h
    jmp     .PrintC

.PrintC_Done:
    jmp     di

.Exit:
    int     19h

/****************************************************************************/

Sign:
    db      "RedStar Cynthia Version 0.01b Copyright (C) 2008-09 ilina@bloodykisses.zzn.com", 0

Unable:
    db      "Error: Unable to prepare platform", 13, 10, 13, 10, 0

A20NotReady:
    db      "Warning: A20 line was not enabled!", 13, 10, 0

/****************************************************************************/

ORG Start

GDTR:
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
