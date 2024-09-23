;*****************************************************************************
;
;   START.ASM
;
;   Modulo de Inicio
;
;   Este modulo iniciara algunas cosas de bajo nivel que son requeridas
;   para que el kernel funcione. Luego de eso se llamara a la funcion
;   de C llamada "main".
;
;   Escrito por J. Palencia (zipox@ureach.com)
;
;*****************************************************************************

        .386p

_TEXT   segment byte public use32 'CODE'
_TEXT   ends

_DATA   segment para public use32 'DATA'
_DATA   ends

_BSS    segment para public use32 'BSS'
_BSS    ends

_CONST  segment para public use32 'CONST'
_CONST  ends

_STRING segment dword public use32 'STRING'
_STRING ends

_ENDSEG segment dword public use32 'ENDSEG'
        KernelEnd LABEL BYTE
_ENDSEG ends

        DGROUP GROUP _DATA, _BSS, _CONST, _STRING, _TEXT, _ENDSEG

_TEXT   segment byte public use32 'CODE'
        assume  cs:DGROUP, ds:DGROUP

KernelStart:
        pop     dword ptr [ServiceTableF]
        pop     dword ptr [ServiceTable]

        mov     ecx, offset KernelEnd
        mov     edx, offset KernelStart
        sub     ecx, edx
        push    ecx
        push    edx
        push    ebx
        call    _main

        test    eax, eax
        jz      reboot

        cli                             ; Hang
        jmp     $

reboot:
        mov     esp, 1                  ; Reiniciar la maquina.
        push    eax

;-----------------------------------------------------------------------------

;*******************************
        _outportb:
            push    ebp
            mov     ebp, esp
            mov     edx, [ebp+8]
            mov     al, [ebp+12]
            pop     ebp
            out     dx, al
            ret

;*******************************
        _outportw:
            push    ebp
            mov     ebp, esp
            mov     edx, [ebp+8]
            mov     eax, [ebp+12]
            pop     ebp
            out     dx,ax
            ret

;*******************************
        _outportd:
            push    ebp
            mov     ebp, esp
            mov     edx, [ebp+8]
            mov     eax, [ebp+12]
            pop     ebp
            out     dx,eax
            ret

;*******************************
        _inportb:
            push    ebp
            xor     eax, eax
            mov     ebp, esp
            mov     edx, [ebp+8]
            pop     ebp
            in      al, dx
            ret

;*******************************
        _inportw:
            push    ebp
            xor     eax, eax
            mov     ebp, esp
            mov     edx, [ebp+8]
            pop     ebp
            in      ax, dx
            ret

;*******************************
        _inportd:
            push    ebp
            mov     ebp, esp
            mov     edx, [ebp+8]
            pop     ebp
            in      eax, dx
            ret

;*******************************
        _updateVideo:
            push    ebp
            mov     ebp, esp
            push    edi
            push    esi

            pushfd
            cli

            mov     esi, [ebp+8]
            mov     edi, 0B8000h
            mov     ecx, 4000/4
            rep     movsd

            popfd

            pop     esi
            pop     edi
            pop     ebp
            ret

;*******************************
        _enable:
            sti
            ret

;*******************************
        _disable:
            cli
            ret

;*******************************
        _save_state:
            push    ebp
            mov     ebp, esp

            push    edi

            mov     edi, [ebp+8]

            stosd               ; eax
            mov     eax, ecx    ; ecx
            stosd
            mov     eax, edx    ; edx
            stosd
            mov     eax, ebx    ; ebx
            stosd
            mov     eax, [ebp]  ; ebp
            stosd
            mov     eax, esi    ; esi
            stosd
            mov     eax, [ebp-4]; edi
            stosd
            mov     eax, ds     ; ds
            stosd
            mov     eax, es     ; es
            stosd

            pop     edi
            pop     ebp
            ret

;*******************************
        _switch_task:
            pushfd
            cli

            push    ebp
            mov     ebp, esp

            mov     ebx, [ebp+16]

            mov     [ebx+24h], esp
            mov     ecx, offset ret_addr
            mov     [ebx+28h], ss
            mov     eax, [ebp+4]
            mov     [ebx+2Ch], ecx
            or      eax, 200h
            mov     dword ptr [ebx+30h], 8
            mov     [ebx+34h], eax

            mov     ebx, [ebp+12]

            mov     eax, [ebx+00h]
            mov     ecx, [ebx+04h]
            mov     edx, [ebx+08h]
            mov     ebp, [ebx+10h]
            mov     esi, [ebx+14h]
            mov     edi, [ebx+18h]
            mov     esp, [ebx+24h]

            push    dword ptr [ebx+34h]
            push    dword ptr [ebx+30h]
            push    dword ptr [ebx+2Ch]

            push    dword ptr [ebx+0Ch]
            pop     ebx

            iretd

        ret_addr:
            pop     ebp
            popfd
            ret

;*******************************
        _setvect:
            push    ebp
            mov     ebp, esp
            pushfd
            cli
            mov     eax, [ebp+8]
            mov     ecx, [ebp+12]
            shl     eax, 2
            mov     ebx, [ServiceTable]
            mov     [ebx+eax], ecx
            mov     ebx, [ServiceTableF]
            mov     edx, [ebp+16]
            mov     [ebx+eax], edx
            popfd
            pop     ebp
            ret

;*******************************
        _getvect:
            push    ebp
            mov     ebp, esp
            mov     eax, [ebp+8]
            shl     eax, 2
            add     eax, dword ptr [ServiceTable]
            mov     eax, [eax]
            pop     ebp
            ret

;*******************************
        _v86int:
            push    ebp
            mov     ebp, esp

            push    esi
            push    edi
            push    ebp

            mov     edi, [ebp+0Ch]
            mov     eax, 'I68V'
            mov     ecx, [edi+00h]
            mov     edx, [edi+0Ch]
            mov     ebx, [edi+10h]
            mov     esi, [ebp+08h]
            mov     ebp, [edi+18h]

            and     dword ptr [edi+08h], 0FFFFH
            and     dword ptr [edi+04h], 0FFFFH
            and     dword ptr [edi+1Ch], 0FFFFH
            and     dword ptr [edi+1Ch], 0FFFFH
            and     dword ptr [edi+20h], 0FFFFH

            shl     ecx, 16
            shl     edx, 16
            shl     ebx, 16
            shl     ebp, 16
            shl     esi, 16

            or      ecx, [edi+08h]
            or      edx, [edi+04h]
            or      ebx, [edi+14h]
            or      ebp, [edi+1Ch]
            or      esi, [edi+20h]

            int     31h

            pop     edi

            mov     edi, [edi+0Ch]

            push    ecx
            push    edx
            push    ebx
            push    esi
            push    ebp

            shr     ecx, 16
            shr     edx, 16
            shr     ebx, 16
            shr     esi, 16
            shr     ebp, 16

            mov     [edi+00h], ecx
            mov     [edi+0Ch], edx
            mov     [edi+10h], ebx
            mov     [ebp+08h], esi
            mov     [edi+18h], ebp

            pop     ebp
            pop     esi
            pop     ebx
            pop     edx
            pop     ecx

            and     ecx, 0FFFFh
            and     edx, 0FFFFh
            and     ebx, 0FFFFh
            and     esi, 0FFFFh
            and     ebp, 0FFFFh

            mov     [edi+08h], ecx
            mov     [edi+04h], edx
            mov     [edi+14h], ebx
            mov     [edi+1Ch], ebp
            mov     [edi+20h], esi

            pop     edi
            pop     esi
            pop     ebp
            ret

;*******************************
        _gCR0:
            mov     eax, cr0
            ret

;*******************************
        _sCR0:
            mov     eax, [esp+4]
            mov     cr0, eax
            ret

        ServiceTable LABEL DWORD
            dd      0

        ServiceTableF LABEL DWORD
            dd      0

_TEXT   ends

        PUBLIC  _outportb,  _outportw,  _inportb,   _inportw
        PUBLIC  _outportd,  _inportd

        PUBLIC  _setvect,   _getvect,   _enable,    _disable

        PUBLIC  _switch_task
        PUBLIC  _updateVideo
        PUBLIC  _save_state
        PUBLIC  _v86int

        PUBLIC  _gCR0
        PUBLIC  _sCR0

        EXTRN   _main:near

end
