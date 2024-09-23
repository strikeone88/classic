;-----------------------------------------------------------------------------
;
;   KGFX.ASM
;
;   Kelly 3D Graphics Engine Version 0.12 for 640x480x16bpp
;
;   This engine requires an 80386+ processor, VESA VBE 2.0+, at least
;   1800 K-bytes of Video-RAM, at least 32 MB of RAM, and it MUST run
;   in TRUE real-mode.
;
;   NOTE: The original engine was modified to fit the particular purpose
;         of this project, all the 3D-related functions are gone, also
;         a pixel-blending function was added (putpixelBlend).
;
;   Copyright (C) 2007-2008 RedStar Technologies
;   Written by J. Palencia (ilina@bloodykisses.zzn.com)
;
;-----------------------------------------------------------------------------

_TEXT   SEGMENT BYTE PUBLIC 'CODE' USE16
        ASSUME  CS:_TEXT, DS:DGROUP
        DGROUP  GROUP _DATA, _BSS

        PUBLIC  _startGfx
        PUBLIC  _stopGfx

        PUBLIC  _putpixel
        PUBLIC  _putpixelBlend
        PUBLIC  _getpixel

        PUBLIC  _vpage
        PUBLIC  _apage
        PUBLIC  _clearpage

        .386P

;-----------------------------------------------------------------------------
    _startGfx:
        push    di

        xor     edi, edi
        mov     ax, ds
        lea     di, [@buf]
        mov     es, ax

        mov     dword ptr es:[di], 32454256h

        mov     ax, 4f00h
        int     10h

        cmp     ax, 4fh
        jne     @noVBE

        mov     ax, es:[di+18]

        mov     cx, 0111h
        mov     ax, 4f01h
        int     10h

        cmp     ax, 4fh
        jne     @cantSet

        mov     eax, dword ptr [di+28h]
        mov     [@apage], eax

        add     [@pages+8], eax
        add     [@pages+4], eax
        add     [@pages], eax

        mov     eax, cr0
        test    al, 1
        jnz     @cantEnter
        or      al, 1

        push    ds
        push    es

        pushf
        cli

        mov     ebx, 0FFFFh
        mov     ecx, ds
        mov     dword ptr [@buf+8], ebx
        shl     ecx, 4
        mov     ebx, 0CF9200h
        add     ecx, edi
        mov     dword ptr [@buf+12], ebx

        mov     dword ptr [@buf+18], ecx
        mov     word ptr [@buf+16], 15

        lgdt    fword ptr [@buf+16]
        mov     cr0, eax

        mov     ax, 8
        mov     ds, ax
        mov     es, ax
        mov     fs, ax
        mov     gs, ax

        mov     eax, cr0
        and     al, -2
        mov     cr0, eax

        popf

        pop     es
        pop     ds

        mov     ax, 4f02h
        mov     bx, 4111h
        int     10h

        cmp     ax, 4fh
        jne     @cantSet

        pop     di

        xor     ax, ax
        mov     fs, ax
        mov     gs, ax

        retf

    @noMem:
        mov     ax, 4
        retf

    @noVBE:
        pop     di
        mov     ax, 3
        retf

    @cantEnter:
        pop     di
        mov     ax, 2
        retf

    @cantSet:
        pop     di
        mov     ax, 1
        retf

;-----------------------------------------------------------------------------
    _stopGfx:
        mov     ax, 3h
        int     10h
        retf

;-----------------------------------------------------------------------------
    _putpixel:
        push    bp
        mov     bp, sp
        mov     ecx, [bp+6]
        mov     ax, [bp+10]
        shld    ebx, ecx, 18
        pop     bp
        and     bl, not 3
        shl     ecx, 16
        mov     ebx, [@Ls+bx]
        shr     ecx, 15
        add     ebx, [@apage]
        add     ebx, ecx
        mov     fs:[ebx], ax
        retf

;-----------------------------------------------------------------------------
    _putpixelBlend:
        push    bp
        mov     bp, sp
        mov     ecx, [bp+6]
        mov     ax, [bp+10]
        shld    ebx, ecx, 18
        and     bl, not 3
        shl     ecx, 16
        mov     ebx, [@Ls+bx]
        shr     ecx, 15
        add     ebx, [@apage]
        add     ebx, ecx
        mov     cx, fs:[ebx]
        pop     bp

        push    ebx

        push    ax
        push    cx
        mov     bx, ax
        and     bx, 11111b
        and     cx, 11111b
        shl     bx, 5
        add     bx, cx
        movzx   dx, BLEND32[bx]
        pop     cx
        pop     ax

        push    ax
        push    cx
        mov     bx, ax
        and     bx, 11111100000b
        and     cx, 11111100000b
        shl     bx, 1
        shr     cx, 5
        add     bx, cx
        movzx   ax, BLEND64[bx]
        shl     ax, 5
        or      dx, ax
        pop     cx
        pop     ax

        mov     bx, ax
        and     bx, 1111100000000000b
        and     cx, 1111100000000000b
        shr     bx, 6
        shr     cx, 11
        add     bx, cx
        movzx   ax, BLEND32[bx]
        shl     ax, 11
        or      dx, ax

        pop     ebx
        
        mov     fs:[ebx], dx
        retf

;-----------------------------------------------------------------------------
    _getpixel:
        push    bp
        mov     bp, sp
        mov     ecx, [bp+6]
        pop     bp
        shld    ebx, ecx, 18
        shl     ecx, 16
        and     bl, not 3
        shr     ecx, 15
        mov     ebx, [@Ls+bx]
        add     ebx, [@apage]
        add     ebx, ecx
        mov     ax, fs:[ebx]
        retf

;-----------------------------------------------------------------------------
    _vpage:
        push    bp
        mov     bp, sp
        mov     bx, [bp+6]
        cmp     bx, 3
        jae     @ret
        pop     bp
        add     bx, bx
        mov     ax, 4F07h
        mov     dx, [@lines+bx]
        mov     bx, 80h
        xor     cx, cx
        int     10h
        retf

;-----------------------------------------------------------------------------
    _apage:
        push    bp
        mov     bp, sp
        mov     bx, [bp+6]
        cmp     bx, 3
        jae     @ret
        shl     bx, 2
        mov     ebx, [@pages+bx]
        mov     [@apage], ebx
    @ret:
        pop     bp
        retf

;-----------------------------------------------------------------------------
    _clearpage:
        push    bp
        mov     bp, sp
        mov     bx, [bp+6]
        mov     ax, [bp+8]
        cmp     bx, 3
        jae     @ret
        pop     bp
        shl     bx, 2
        mov     cx, di
        xor     dx, dx
        mov     edi, [@pages+bx]
        mov     es, dx
        mov     bx, cx
        mov     dx, ax
        mov     ecx, 153600
        shl     eax, 16
        mov     ax, dx
        db      67h
        rep     stosd
        mov     di, bx
        retf

_TEXT   ENDS

;-----------------------------------------------------------------------------

_DATA   SEGMENT WORD PUBLIC 'DATA' USE16

        @apage      dd  0

        @pages      dd  000000h
                    dd  096000h
                    dd  12C000h

        @lines      dw  0000h
                    dw  01E0h
                    dw  03C0h

@Ls LABEL DWORD
dd 000000h, 000500h, 000A00h, 000F00h, 001400h, 001900h, 001E00h, 002300h
dd 002800h, 002D00h, 003200h, 003700h, 003C00h, 004100h, 004600h, 004B00h
dd 005000h, 005500h, 005A00h, 005F00h, 006400h, 006900h, 006E00h, 007300h
dd 007800h, 007D00h, 008200h, 008700h, 008C00h, 009100h, 009600h, 009B00h
dd 00A000h, 00A500h, 00AA00h, 00AF00h, 00B400h, 00B900h, 00BE00h, 00C300h
dd 00C800h, 00CD00h, 00D200h, 00D700h, 00DC00h, 00E100h, 00E600h, 00EB00h
dd 00F000h, 00F500h, 00FA00h, 00FF00h, 010400h, 010900h, 010E00h, 011300h
dd 011800h, 011D00h, 012200h, 012700h, 012C00h, 013100h, 013600h, 013B00h
dd 014000h, 014500h, 014A00h, 014F00h, 015400h, 015900h, 015E00h, 016300h
dd 016800h, 016D00h, 017200h, 017700h, 017C00h, 018100h, 018600h, 018B00h
dd 019000h, 019500h, 019A00h, 019F00h, 01A400h, 01A900h, 01AE00h, 01B300h
dd 01B800h, 01BD00h, 01C200h, 01C700h, 01CC00h, 01D100h, 01D600h, 01DB00h
dd 01E000h, 01E500h, 01EA00h, 01EF00h, 01F400h, 01F900h, 01FE00h, 020300h
dd 020800h, 020D00h, 021200h, 021700h, 021C00h, 022100h, 022600h, 022B00h
dd 023000h, 023500h, 023A00h, 023F00h, 024400h, 024900h, 024E00h, 025300h
dd 025800h, 025D00h, 026200h, 026700h, 026C00h, 027100h, 027600h, 027B00h
dd 028000h, 028500h, 028A00h, 028F00h, 029400h, 029900h, 029E00h, 02A300h
dd 02A800h, 02AD00h, 02B200h, 02B700h, 02BC00h, 02C100h, 02C600h, 02CB00h
dd 02D000h, 02D500h, 02DA00h, 02DF00h, 02E400h, 02E900h, 02EE00h, 02F300h
dd 02F800h, 02FD00h, 030200h, 030700h, 030C00h, 031100h, 031600h, 031B00h
dd 032000h, 032500h, 032A00h, 032F00h, 033400h, 033900h, 033E00h, 034300h
dd 034800h, 034D00h, 035200h, 035700h, 035C00h, 036100h, 036600h, 036B00h
dd 037000h, 037500h, 037A00h, 037F00h, 038400h, 038900h, 038E00h, 039300h
dd 039800h, 039D00h, 03A200h, 03A700h, 03AC00h, 03B100h, 03B600h, 03BB00h
dd 03C000h, 03C500h, 03CA00h, 03CF00h, 03D400h, 03D900h, 03DE00h, 03E300h
dd 03E800h, 03ED00h, 03F200h, 03F700h, 03FC00h, 040100h, 040600h, 040B00h
dd 041000h, 041500h, 041A00h, 041F00h, 042400h, 042900h, 042E00h, 043300h
dd 043800h, 043D00h, 044200h, 044700h, 044C00h, 045100h, 045600h, 045B00h
dd 046000h, 046500h, 046A00h, 046F00h, 047400h, 047900h, 047E00h, 048300h
dd 048800h, 048D00h, 049200h, 049700h, 049C00h, 04A100h, 04A600h, 04AB00h
dd 04B000h, 04B500h, 04BA00h, 04BF00h, 04C400h, 04C900h, 04CE00h, 04D300h
dd 04D800h, 04DD00h, 04E200h, 04E700h, 04EC00h, 04F100h, 04F600h, 04FB00h
dd 050000h, 050500h, 050A00h, 050F00h, 051400h, 051900h, 051E00h, 052300h
dd 052800h, 052D00h, 053200h, 053700h, 053C00h, 054100h, 054600h, 054B00h
dd 055000h, 055500h, 055A00h, 055F00h, 056400h, 056900h, 056E00h, 057300h
dd 057800h, 057D00h, 058200h, 058700h, 058C00h, 059100h, 059600h, 059B00h
dd 05A000h, 05A500h, 05AA00h, 05AF00h, 05B400h, 05B900h, 05BE00h, 05C300h
dd 05C800h, 05CD00h, 05D200h, 05D700h, 05DC00h, 05E100h, 05E600h, 05EB00h
dd 05F000h, 05F500h, 05FA00h, 05FF00h, 060400h, 060900h, 060E00h, 061300h
dd 061800h, 061D00h, 062200h, 062700h, 062C00h, 063100h, 063600h, 063B00h
dd 064000h, 064500h, 064A00h, 064F00h, 065400h, 065900h, 065E00h, 066300h
dd 066800h, 066D00h, 067200h, 067700h, 067C00h, 068100h, 068600h, 068B00h
dd 069000h, 069500h, 069A00h, 069F00h, 06A400h, 06A900h, 06AE00h, 06B300h
dd 06B800h, 06BD00h, 06C200h, 06C700h, 06CC00h, 06D100h, 06D600h, 06DB00h
dd 06E000h, 06E500h, 06EA00h, 06EF00h, 06F400h, 06F900h, 06FE00h, 070300h
dd 070800h, 070D00h, 071200h, 071700h, 071C00h, 072100h, 072600h, 072B00h
dd 073000h, 073500h, 073A00h, 073F00h, 074400h, 074900h, 074E00h, 075300h
dd 075800h, 075D00h, 076200h, 076700h, 076C00h, 077100h, 077600h, 077B00h
dd 078000h, 078500h, 078A00h, 078F00h, 079400h, 079900h, 079E00h, 07A300h
dd 07A800h, 07AD00h, 07B200h, 07B700h, 07BC00h, 07C100h, 07C600h, 07CB00h
dd 07D000h, 07D500h, 07DA00h, 07DF00h, 07E400h, 07E900h, 07EE00h, 07F300h
dd 07F800h, 07FD00h, 080200h, 080700h, 080C00h, 081100h, 081600h, 081B00h
dd 082000h, 082500h, 082A00h, 082F00h, 083400h, 083900h, 083E00h, 084300h
dd 084800h, 084D00h, 085200h, 085700h, 085C00h, 086100h, 086600h, 086B00h
dd 087000h, 087500h, 087A00h, 087F00h, 088400h, 088900h, 088E00h, 089300h
dd 089800h, 089D00h, 08A200h, 08A700h, 08AC00h, 08B100h, 08B600h, 08BB00h
dd 08C000h, 08C500h, 08CA00h, 08CF00h, 08D400h, 08D900h, 08DE00h, 08E300h
dd 08E800h, 08ED00h, 08F200h, 08F700h, 08FC00h, 090100h, 090600h, 090B00h
dd 091000h, 091500h, 091A00h, 091F00h, 092400h, 092900h, 092E00h, 093300h
dd 093800h, 093D00h, 094200h, 094700h, 094C00h, 095100h, 095600h, 095B00h

    INCLUDE BLEND/BLEND32.INC
    INCLUDE BLEND/BLEND64.INC

_DATA   ENDS

_BSS    SEGMENT WORD PUBLIC 'BSS' USE16
        @buf        db  1024 dup (?)
_BSS    ENDS

END
