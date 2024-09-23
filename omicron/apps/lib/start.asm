;*****************************************************************************
;
;   START.ASM
;
;   Modulo de Inicio
;
;   Escrito por J. Palencia (zipox@ureach.com)
;
;*****************************************************************************

        .486p

_TEXT   segment byte public use32 'CODE'
_TEXT   ends

_DATA   segment para public use32 'DATA'

        _KernelInterface    dd  0
        _selfProcess        dd  0

_DATA   ends

_BSS    segment para public use32 'BSS'
_BSS    ends

_CONST  segment para public use32 'CONST'
_CONST  ends

_STRING segment dword public use32 'STRING'
_STRING ends

        DGROUP GROUP _DATA, _BSS, _CONST, _STRING, _TEXT

_TEXT   segment byte public use32 'CODE'
        assume  cs:DGROUP, ds:DGROUP

start:
        mov     [_KernelInterface], ebx
        mov     [_selfProcess], eax

        call    _main

        push    eax
        push    40009h
        call    [_KernelInterface]

_dmemcpy:
        push    ebp
        mov     ebp, esp
        push    esi
        push    edi
        mov     edi, [ebp+8]
        mov     ecx, [ebp+16]
        mov     esi, [ebp+12]
        shr     ecx, 2
        pushfd
        cli
        rep     movsd
        popfd
        pop     edi
        pop     esi
        mov     eax, [ebp+8]
        pop     ebp
        ret

_dmemset:
        push    ebp
        mov     ebp, esp
        push    edi
        mov     edi, [ebp+8]
        mov     ecx, [ebp+16]
        mov     eax, [ebp+12]
        shr     ecx, 2
        pushfd
        cli
        rep     stosd
        popfd
        pop     edi
        mov     eax, [ebp+8]
        pop     ebp
        ret

_packto565:
        push    esi
        push    edi
        mov     ecx, [esp+20]
        mov     ebx, [esp+12]
        mov     esi, [esp+16]
;        pushfd
;        cli

packto565_c:
        mov     edx, [esi]          ;u  1
        add     esi, 4              ;v
        bswap   edx
        shr     edx, 8

        mov     eax, edx            ;u  1
        and     edx, 11111000b      ;v

        mov     edi, eax            ;u  1
        and     eax, 111110000000000000000000b ;v

        shr     edx, 3              ;u 1
        and     edi, 1111110000000000b ;v

        shr     eax, 8              ;u 1
        shr     edi, 5              ;u 1

        or      eax, edx
        or      eax, edi

        mov     [ebx], ax
        add     ebx, 2

        dec     ecx
        jnz     packto565_c

;        popfd
        pop     edi
        pop     esi
        mov     eax, [esp+4]
        ret

_atan:
        fild    dword ptr [esp+12]
        fild    dword ptr [esp+8]
        fild    dword ptr [esp+4]
        fpatan
        fmulp   st(1),st
        fldpi
        fdivp   st(1),st
        fistp   dword ptr [esp+4]
        mov     eax, [esp+4]
        ret

_addBufferF:
        push    ebp
        mov     ebp, esp

        push    esi
        push    edi

        pushfd
        cli

        mov     ecx, [ebp+28]
        mov     edi, [ebp+8]
        mov     esi, [ebp+12]
        mov     edx, [ebp+20]
        mov     ebx, [ebp+16]
        mov     ebp, [ebp+24]

YCycle2:
        push    ebx
        push    ebx
        push    ebx

XCycle2:
        sub     eax, eax
        mov     ebx, 255
        mov     al, [esi]
        inc     esi
        mov     [esp], eax
        mov     al, [edi]
        inc     edi
        add     eax, [esp]
        cmp     eax, ebx
        db      00Fh, 04Dh, 0C3h    ;cmovg eax, ebx
        sub     ebx, ebx
        sub     eax, ecx
        cmp     eax, ebx
        db      00Fh, 04Ch, 0C3h    ;cmovl eax, ebx
        mov     [edi-1], al

        sub     eax, eax
        mov     ebx, 255
        mov     al, [esi]
        inc     esi
        mov     [esp], eax
        mov     al, [edi]
        inc     edi
        add     eax, [esp]
        cmp     eax, ebx
        db      00Fh, 04Dh, 0C3h    ;cmovg eax, ebx
        sub     ebx, ebx
        sub     eax, ecx
        cmp     eax, ebx
        db      00Fh, 04Ch, 0C3h    ;cmovl eax, ebx
        mov     [edi-1], al

        sub     eax, eax
        mov     ebx, 255
        mov     al, [esi]
        inc     esi
        mov     [esp], eax
        mov     al, [edi]
        inc     edi
        add     eax, [esp]
        cmp     eax, ebx
        db      00Fh, 04Dh, 0C3h    ;cmovg eax, ebx
        sub     ebx, ebx
        sub     eax, ecx
        cmp     eax, ebx
        db      00Fh, 04Ch, 0C3h    ;cmovl eax, ebx
        mov     [edi-1], al

        inc     esi
        inc     edi

        dec     dword ptr [esp+4]
        jnz     XCycle2

        pop     eax
        pop     ebx
        pop     ebx

        add     edi, ebp

        dec     edx
        jnz     YCycle2

        popfd
        pop     edi
        pop     esi
        pop     ebp
        ret

_TEXT   ends

        public  _KernelInterface
        public  _selfProcess

        public  _dmemcpy
        public  _dmemset

        public  _packto565

        public  _atan

        public  _addBufferF

        extrn   _main:near

end     start
