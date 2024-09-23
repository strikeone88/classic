;*****************************************************************************
;
;   GBOOT.ASM
;
;   Generic Boot Sector (Adaptation from Cynthia's Boot)
;
;   Copyright (C) 2007-2008 RedStar Technologies
;   Written by J. Palencia (zipox@ureach.com)
;
;*****************************************************************************

_TEXT   segment byte public 'CODE'
        assume  cs:_TEXT, ds:_TEXT
        .386

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

        db      0EAh
        dw      offset @start
        dw      070h

    @start:
        push    dx

        mov     ax, 1110h
        mov     bx, 1000h
        mov     cx, 0002h
        mov     dx, 0001h
        lea     bp, [$tbl]
        int     10h

        mov     al, 0Ah
        mov     dx, 03D4h
        out     dx, al
        inc     dx
        mov     al, 10h
        out     dx, al
        dec     dx
        mov     al, 0Bh
        out     dx, al
        inc     dx
        mov     al, 10h
        out     dx, al

        pop     dx

        mov     ax, 50
    @pre:
        lea     si, [$nl]
        call    print
        dec     ax
        jnz     @pre

        lea     bx, [$kernel]
        lea     di, [$kernelParms]
        call    loadimage

        mov     bx, word ptr [$kernelParms]

        mov     ds, bx
        mov     es, bx

        mov     ax, 3h
        int     10h

        push    bx
        push    0
        retf

;****************************************************************************
    reboot:
        mov     ax, 3
        int     10h

        lea     si, [$sign]
        call    print

        lea     si, [$error]
        call    print

        lea     si, [$ioerror]
        call    print

        xor     ax, ax
        int     16h

        db      09Ah
        dw      00000h
        dw      0FFFFh

;****************************************************************************
    print:
        push    ax
        push    si
        push    bx

        mov     bx, 7
        mov     ah, 0Eh

    @print$cicle:
        mov     al, [si]
        inc     si

        test    al, al
        jnz     short @print$put

        pop     bx
        pop     si
        pop     ax

        ret

    @print$put:
        int     10h
        jmp     short @print$cicle

;****************************************************************************
    pmsg:
        push    si

        lea     si, [$sign]
        call    print

        lea     si, [$colon]
        call    print

        pop     si

        call    print

        ret

;****************************************************************************
    loadimage:
        lea     si, [$loading]
        call    pmsg

        push    dx
        push    bx
        push    di
        push    es

        mov     ah, 08h
        int     13h

        and     cl, 3Fh
        mov     [spt], cl

        inc     dh
        mov     [nh], dh

        pop     es
        pop     di
        pop     bx
        pop     dx

        mov     si, bx
        call    print

        lea     si, [$nl]
        call    print

        mov     cx, 70
        mov     ah, 0Eh
        mov     bx, 7
        mov     al, 1

    loadimage$0:
        int     10h
        dec     cx
        jnz     loadimage$0

        lea     si, [$cr]
        call    print
        
        mov     es, [di]

        add     di, 4
        xor     bx, bx

        mov     si, [di-2]
        mov     cx, [di+2]
        mov     dh, [di+4]

        xor     bp, bp
        mov     di, [di]

    @loadimage$cicle:
        call    read

        add     bp, di

    @loadimage$check:
        cmp     bp, 128
        jb      short @loadimage$skip

        sub     bp, 128

        push    bx

        mov     ax, 0E00h OR 2
        mov     bx, 14
        int     10h

        pop     bx

        jmp     short @loadimage$check

    @loadimage$skip:
        dec     si
        jnz     short @loadimage$cicle

        lea     si, [$nl]
        call    print

        ret

;****************************************************************************
    read:
        push    bp
        mov     bp, 10

    @read$retry:
        mov     ax, 0201h
        int     13h
        jnc     short @read$done

        dec     bp
        jnz     short @read$retry

        jmp     reboot

    @read$done:
        mov     al, cl
        and     al, 3Fh
        cmp     al, [spt]
        jb      short @read$upd

        and     cl, not 3Fh

        inc     dh
        cmp     dh, [nh]
        jb      short @read$upd

        xor     dh, dh

        shr     cl, 6
        inc     ch
        adc     cl, 0
        shl     cl, 6

    @read$upd:
        inc     cl
        add     bx, 512
        jnz     short @read$ret

        mov     ax, es
        add     ax, 1000h
        mov     es, ax

    @read$ret:
        pop     bp
        ret

;****************************************************************************
    $sign:
        db  13, 10, "Omicron Boot", 0

    $error:
        db  " (Error)"

    $colon:
        db  ": ", 0

    $loading:
        db  "Loading ", 0

    $nl:
        db  13, 10, 0

    $cr:
        db  13, 0

    $ioerror:
        db  "Press any key to reboot...", 0

    $kernel:
        db  "Omicron-32 Version 1.00", 0

    $tbl:
    $tbl:
        db  00000000b
        db  00000000b
        db  00000000b
        db  00000000b
        db  00000000b
        db  00011000b
        db  00100100b
        db  01000010b
        db  00100100b
        db  00011000b
        db  00000000b
        db  00000000b
        db  00000000b
        db  00000000b
        db  00000000b
        db  00000000b

        db  00000000b
        db  00000000b
        db  00000000b
        db  00000000b
        db  00111100b
        db  01111110b
        db  11111111b
        db  11111111b
        db  11111111b
        db  01111110b
        db  00111100b
        db  00000000b
        db  00000000b
        db  00000000b
        db  00000000b
        db  00000000b

        nh  db  0
        spt db  0

;****************************************************************************
        db  499 - ($ - Start) dup ('&')

        KBToLoad EQU 480

    $kernelParms:
        dw  1000h   ;10000-8FFFF
        dw  KBToLoad * 2
        dw  4480 / KBToLoad

        db  2
        db  0
        db  0

        dw  02121h
        dw  0AA55h

_TEXT   ends

end     Start
