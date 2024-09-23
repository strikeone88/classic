;
;   XMSH.ASM
;
;   XMS Helper Version 0.1
;
;   Copyright (C) 2007 RedStar Technologies
;   Written by J. Palencia (zipox@ureach.com)
;

_TEXT   SEGMENT BYTE PUBLIC 'CODE' USE16
        ASSUME  CS:_TEXT, DS:_DATA
        .386

        PUBLIC  _checkXMS
        PUBLIC  _allocEMB
        PUBLIC  _freeEMB
        PUBLIC  _moveEMB
        PUBLIC  _moveEMB_st

;****************************************************************************
        _checkXMS:
            mov     ax, 4300h
            int     2Fh

            cmp     al, 80h
            jne     @noXMS

            mov     ax, 4310h
            int     2Fh

            mov     word ptr [@xms+2], es
            mov     word ptr [@xms], bx

            xor     ax, ax
            call    [@xms]

            cmp     ax, 0200h
            jb      @noVers

            xor     ax, ax
            retf

        @noVers:
            mov     ax, 2
            retf

        @noXMS:
            mov     ax, 1
            retf

;****************************************************************************
    _allocEMB:
            push    bp
            mov     bp, sp

            mov     ax, [bp+6]
            mov     dx, [bp+8]

            shr     ax, 10
            shl     dx, 6

            add     dx, ax
            inc     dx

            mov     ah, 9
            call    [@xms]

            pop     bp

            xor     ax, 1
            jnz     @retNull

            mov     ax, dx
            retf

        @retNull:
            xor     ax, ax
            retf

;****************************************************************************
    _freeEMB:
            push    bp
            mov     bp, sp

            mov     ah, 0Ah
            mov     dx, [bp+6]
            call    [@xms]

            pop     bp

            retf

;****************************************************************************
    _moveEMB:
            push    bp
            mov     bp, sp

            push    si

            mov     ax, [bp+6]
            mov     bx, [bp+8]
            mov     cx, [bp+10]
            mov     [@src_h], ax
            mov     word ptr [@src_o+2], cx
            mov     ax, [bp+12]
            mov     word ptr [@src_o], bx
            mov     cx, [bp+14]
            mov     word ptr [@dst_h], ax
            mov     bx, [bp+16]
            mov     word ptr [@dst_o], cx
            mov     ax, [bp+18]
            mov     word ptr [@dst_o+2], bx
            mov     cx, [bp+20]
            mov     word ptr [@length], ax
            mov     word ptr [@length+2], cx

            mov     ah, 0Bh
            lea     si, [EMBMove]
            call    [@xms]

            dec     ax

            pop     si
            pop     bp

            retf

;****************************************************************************
    _moveEMB_st:
            push    bp
            mov     bp, sp

            mov     ax, ds
            push    si
            mov     es, ax
            push    ds

            lds     si, [bp+6]

            mov     ah, 0Bh
            call    es:[@xms]

            dec     ax

            pop     ds
            pop     si
            pop     bp

            retf

_TEXT   ENDS

_DATA   SEGMENT WORD PUBLIC 'DATA' USE16

        @xms        dd      0

        EMBMove     LABEL   BYTE
        @length     dd      0
        @src_h      dw      0
        @src_o      dd      0
        @dst_h      dw      0
        @dst_o      dd      0

_DATA   ENDS

END
