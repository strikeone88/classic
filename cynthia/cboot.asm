/*
    CBOOT.ASM

    Cynthia's Boot Sector Code Version 0.01b

    Copyright (C) 2007-2009 RedStar Technologies
    Written by J. Palencia (ilina@bloodykisses.zzn.com)
*/

Section Text Byte Public 'CODE'
[Text]

Start:
        cli
        mov     bx, 70h
        xor     di, di
        mov     es, bx
        mov     ss, bx

        mov     ax, 7C0h
        xor     si, si
        mov     ds, ax

        mov     cx, 512
        rep     movsw

        mov     ds, bx
        mov     sp, di

        sti
        jmp     70h:$$

        push    dx
        mov     ax, 1110h
        mov     bx, 1000h
        mov     cx, 0002h
        mov     dx, 0001h
        lea     bp, [CharacterTable]
        int     10h
        mov     dx, 3D4h
        mov     al, 0Ah
        out     dx, al
        inc     dx
        mov     al, 0Fh
        out     dx, al
        dec     dx
        mov     al, 0Bh
        out     dx, al
        inc     dx
        sub     al, al
        out     dx, al
        pop     dx

        mov     cx, 50
        sub     bx, bx

ScrollDown:
        mov     ax, 0E0Dh
        int     10h
        mov     ax, 0E0Ah
        int     10h
        loop    ScrollDown

        lea     di, [KernelParms]
        call    LoadImage

        mov     bx, word ptr [KernelParms]
        mov     ds, bx
        mov     es, bx

        mov     ax, 3h
        int     10h

        push    bx
        push    0
        retf

;****************************************************************************
Print:
        push    ax
        push    si
        push    bx

        mov     bx, 07h
        mov     ah, 0Eh

Print.Cicle:
        mov     al, [si]
        inc     si

        test    al, al
        jnz     short Print.Put

        pop     bx
        pop     si
        pop     ax
        ret

Print.Put:
        int     10h
        jmp     short Print.Cicle

;****************************************************************************
LoadImage:
        push    dx
        push    bx
        push    di
        push    es

        mov     ah, 08h
        int     13h

        and     cl, 3Fh
        mov     [SectorsPerTrack], cl

        inc     dh
        mov     [NumberOfHeads], dh

        pop     es
        pop     di
        pop     bx
        pop     dx

        mov     cx, 4Fh
        mov     ah, 0Eh
        sub     bx, bx
        mov     al, 01h

LoadImage.DrawStatusBar:
        int     10h
        dec     cx
        jnz     LoadImage.DrawStatusBar

        mov     ax, 0E0Dh
        sub     bx, bx
        int     10h

        mov     es, [di]

        add     di, 04h
        xor     bx, bx

        mov     si, [di-02h]
        mov     cx, [di+02h]
        mov     dh, [di+04h]

        xor     bp, bp
        mov     di, [di]

LoadImage.Cicle:
        call    Read
        add     bp, di

LoadImage.Check:
        cmp     bp, 80h
        jb      short LoadImage.Skip

        sub     bp, 80h
        push    bx
        mov     ax, 0E02h
        sub     bx, bx
        int     10h
        pop     bx
        jmp     short LoadImage.Check

LoadImage.Skip:
        dec     si
        jnz     short LoadImage.Cicle
        ret

;****************************************************************************
Read:
        push    bp
        mov     bp, 40h

Read.Retry:
        mov     ax, 0201h
        int     13h
        jnc     short Read.Done
        dec     bp
        jnz     short Read.Retry

        mov     ax, 03h
        int     10h

        lea     si, [IoErrorMessage]
        call    Print

        xor     ax, ax
        int     16h

        jmp     0FFFFh:00000h

Read.Done:
        mov     al, cl
        and     al, 3Fh
        cmp     al, [SectorsPerTrack]
        jb      short Read.Upd

        and     cl, not 3Fh

        inc     dh
        cmp     dh, [NumberOfHeads]
        jb      short Read.Upd

        xor     dh, dh

        shr     cl, 6
        inc     ch
        adc     cl, 0
        shl     cl, 6

Read.Upd:
        inc     cl
        add     bx, 200h
        jnz     short Read.Ret

        mov     ax, es
        add     ax, 1000h
        mov     es, ax

Read.Ret:
        pop     bp
        ret

;****************************************************************************
IoErrorMessage:
        db  "I/O Error! Press a key to reboot...", 0

CharacterTable:
        db  00000000b, 00000000b, 00000000b, 00000000b
        db  00000000b, 00011000b, 00100100b, 01000010b
        db  00100100b, 00011000b, 00000000b, 00000000b
        db  00000000b, 00000000b, 00000000b, 00000000b
        db  00000000b, 00000000b, 00000000b, 00000000b
        db  00111100b, 01111110b, 11111111b, 11111111b
        db  11111111b, 01111110b, 00111100b, 00000000b
        db  00000000b, 00000000b, 00000000b, 00000000b

NumberOfHeads:
        db  00h

SectorsPerTrack:
        db  00h

        db  501 - ($ - Start) dup ('&')

KernelParms:
        dw  01000h, 00470h, 00009h
        db  00002h, 00000h, 00000h
        dw  0AA55h

[Ends]
