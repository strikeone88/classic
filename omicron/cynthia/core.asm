/*
    CORE.ASM

    Cynthia Flat-Mode Platform Core Version 0.01m

    This version has been modified to fit the specific purpose
    of this project.

    Copyright (C) 2008 RedStar Technologies
    Written by J. Palencia (ilina@bloodykisses.zzn.com)
*/

Section     Text Dword 'Code'
Section     Data Dword 'Data'
Entry       Start
Bits        32

;****************************************************************************

[Text]
    KPD     dd  1024 dup (0)
    KT1     dd  1024 dup (0)
    KT2     dd  1024 dup (0)

Start:
    pop     [ExitProtected]
    pop     [GDT]

    mov     [OriginalImage], edi
    mov     [ImageLength], esi

    mov     eax, esp
    and     eax, not 3
    mov     esp, eax

    call    .InitializeIDT
    call    .InitializeTSS
    call    .InitializePIC
    call    .InitializeVars

    mov     esi, [OriginalImage]
    mov     edi, 180000h
    add     esi, [ImageLength]
    mov     ecx, 131072
    rep     movsd

    mov     eax, ServiceTable
    mov     ebx, [TotalPages]
    mov     ecx, ServiceTableF
    push    eax
    push    ecx
    shl     ebx, 12
    jmp     180000h

;============================================================================
.InitializeIDT:
    mov     eax, IFF - I00 shr 8
    lea     ebx, [I00]
    lea     esi, [IDT]
    mov     edx, 256

.InitializeIDT.Cicle:
    mov     ecx, ebx
    mov     [esi], bx
    shr     ecx, 16
    mov     [esi+6], cx
    add     esi, 8
    add     ebx, eax
    dec     edx
    jnz     .InitializeIDT.Cicle

    lidt    [IDTR]
    ret

;============================================================================
.InitializeTSS:
    mov     ebx, [GDT]

    add     ebx, 18h
    lea     ecx, [CynthiaTaskStateSegment]
    mov     [ebx+0], dword CynthiaTaskStateSegmentEnd - CynthiaTaskStateSegment - 1
    mov     [ebx+4], dword 00C08900h
    mov     [ebx+2], cx
    shr     ecx, 16
    mov     [ebx+4], cl
    mov     [ebx+7], ch

    add     ebx, 8
    lea     ecx, [V86TaskStateSegment]
    mov     [ebx+0], dword V86TaskStateSegmentEnd - V86TaskStateSegment - 1
    mov     [ebx+4], dword 00C08B00h
    mov     [ebx+2], cx
    shr     ecx, 16
    mov     [ebx+4], cl
    mov     [ebx+7], ch

    add     ebx, 8
    lea     ecx, [DispatcherTSS]
    mov     [ebx+0], dword DispatcherTSSEnd - DispatcherTSS - 1
    mov     [ebx+4], dword 00C08900h
    mov     [ebx+2], cx
    shr     ecx, 16
    mov     [ebx+4], cl
    mov     [ebx+7], ch

    ;18     CynthiaTaskStateSegment
    ;20     V86TaskStateSegment
    ;28     DispatcherTSS

    mov     eax, 18h
    ltr     ax

    ret

;============================================================================
.InitializePIC:
    mov     al, 11h
    out     020h, al
    jmp     $+2
    out     0A0h, al

    mov     al, 20h
    out     021h, al
    mov     al, 28h
    out     0A1h, al

    mov     al, 04h
    out     021h, al
    mov     al, 02h
    out     0A1h, al

    mov     al, 11h
    out     021h, al
    jmp     $+2
    out     0A1h, al

    mov     al, not (1 or 2)
    out     021h, al

    sti
    ret

;============================================================================
.InitializeVars:
    mov     [9C000h], 030CD00CDh

    mov     ebx, 100000h
    mov     ecx, 0A5791D6Dh
    mov     esi, 256

.GetMaxMemory:
    mov     eax, [ebx]
    mov     [ebx], ecx
    mov     edx, [ebx]
    cmp     edx, ecx
    mov     [ebx], eax
    je      .GetMaxMemory.Continue
    mov     [TotalPages], esi
    ret

.GetMaxMemory.Continue:
    add     ebx, 4096
    inc     esi
    jmp     .GetMaxMemory

;============================================================================
.InitializePaging:
    mov     edi, KPD
    mov     ecx, 0C00h
    sub     eax, eax
    rep     stosd

    mov     ebx, KPD
    mov     dword ptr [ebx+000h], KT1 + 07h
    mov     dword ptr [ebx+004h], KT2 + 07h

    mov     esi, KT1
    mov     ebx, 07h
    mov     ecx, 512

.InitKT1:
    mov     [esi], ebx
    add     esi, 4
    add     ebx, 4096
    dec     ecx
    jnz     .InitKT1

    mov     eax, KPD
    mov     cr3, eax

    mov     [DISTSS.CR3], eax
    mov     [DEFTSS.CR3], eax
    mov     [V86TSS.CR3], eax

    mov     eax, cr0
    or      eax, 1 SHL 31
    mov     cr0, eax

    ret

;============================================================================
Dispatcher:
    cmp     eax, 'IMMR'
    je      Dispatcher.ImmediateReturn

    cmp     eax, 'ONEC'
    je      Dispatcher.OneCall

    iretd

Dispatcher.Resume:
    mov     [DISTSS.BackLink], ebx
    iretd
    jmp     Dispatcher

Dispatcher.OneCall:
    mov     eax, 'IMMR'
    mov     ebx, ecx
    jmp     Dispatcher.Resume

Dispatcher.ImmediateReturn:
    sub     eax, eax
    mov     ebx, edx
    jmp     Dispatcher.Resume

;============================================================================
InterruptHandler:
    test    dword ptr [esp+12], 1 shl 17
    jnz     InterruptHandler.EmulateV86Interrupt

    mov     [tEBX], ebx
    mov     ebx, [esp]

    mov     dword ptr [.FromV86], 0

    mov     [iNUM], ebx

InterruptHandler.NormalInterrupt:
    push    ecx
    mov     ecx, [ebx+ServiceTable]
    test    ecx, ecx
    jz      .CatchIRQ
    mov     ebx, [ebx+ServiceTableF]
    test    ebx, 1 shl 31
    jz      .SaveRegs

    mov     ebx, [tEBX]
    mov     [tEBX], ecx
    pop     ecx
    add     esp, 4

    call    [tEBX]
    iretd

.SaveRegs:
    mov     ebx, [tEBX]
    mov     [tEBX], ecx
    pop     ecx
    add     esp, 4

    pushad
    call    [tEBX]
    popad
    iretd

.CatchIRQ:
    pop     ecx
.CatchIRQf:
    mov     ebx, [iNUM]

    cmp     ebx, 20h*4
    jb      .Default

    cmp     ebx, 2Fh*4
    ja      .Default

    push    eax
    mov     al, 20h
    out     020h, al
    out     0EBh, al
    out     0A0h, al
    pop     eax

    add     esp, 4
    mov     ebx, [tEBX]
    iretd

.Default:
    mov     ebx, es
    push    ebx
    mov     ebx, ds
    push    ebx
    push    edi
    push    esi
    push    ebp
    push    dword ptr [tEBX]
    push    edx
    push    ecx
    push    eax

    push    10h
    pop     es

    mov     ebx, 0B8000h + 160 + 64
    mov     ah, 19h
    mov     edx, cr0
    call    Print32

    mov     ecx, 9
    mov     ebx, 0B8000h
    mov     ah, 2Eh
C:
    pop     edx
    call    Print32
    add     ebx, 160
    dec     ecx
    jnz     C

    mov     ecx, 16
    mov     ebx, 0B8000h + 24
    mov     ah, 4Eh
    mov     esi, esp
C1:
    mov     edx, [esi]
    add     esi, 4
    call    Print32
    add     ebx, 160
    dec     ecx
    jnz     C1

    mov     ebx, 0B8000h + 160 - 16
    mov     ah, 4Eh

    mov     ebp, esp

    mov     edx, [ebp]
    shr     edx, 2
    call    Print32
    add     ebx, 160
    add     ebp, 4

    mov     edx, [ebp-4]
    cmp     edx, 8
    je      .WithErrorCode
    cmp     edx, 10
    jb      .NoErrorCode
    cmp     edx, 14
    ja      .NoErrorCode

.WithErrorCode:
    mov     ah, 2Ah
    mov     edx, [ebp]
    call    Print32
    add     ebx, 160
    add     ebp, 4

.NoErrorCode:
    mov     ah, 2Eh

    mov     edx, [ebp]
    mov     [tPTR], edx
    call    Print32
    add     ebx, 160
    add     ebp, 4

    test    byte ptr [.FromV86], 1
    jz      .Skip
    mov     edx, [ebp]
    shl     edx, 4
    add     edx, [ebp-4]
    mov     [tPTR], edx
.Skip:
    mov     edx, [ebp]
    call    Print32
    add     ebx, 160
    add     ebp, 4

    mov     ah, 2Fh

    mov     ebp, [tPTR]
    add     ebx, 160 + 16
    sub     ebx, 8*6
    mov     ecx, 8

.Cicle:
    mov     dl, [ebp]
    call    PrintByte
    inc     ebp
    add     ebx, 6
    dec     ecx
    jnz     .Cicle

    jmp     [ExitProtected]

;============================================================================
InterruptHandler.EmulateV86Interrupt:
    push    10h
    pop     ds

    mov     [tEBX], ebx
    mov     ebx, [esp]

    mov     dword ptr [.FromV86], 1

    mov     [iNUM], ebx

    mov     ebx, [esp+8]
    shl     ebx, 4
    add     ebx, [esp+4]
    cmp     byte ptr [ebx-2], 0CDh
    mov     ebx, [esp]
    jne     .CatchIRQf

    sub     dword ptr [esp+10h], 6
    mov     ebx, [esp+14h]
    shl     ebx, 4
    add     ebx, [esp+10h]
    push    ecx
    mov     ecx, [esp+10h]
    mov     [ebx+04h], cx
    mov     ecx, [esp+08h]
    mov     [ebx], cx
    mov     ecx, [esp+0Ch]
    mov     [ebx+02h], cx
    pop     ecx

    pop     ebx
    mov     ebx, [ebx]
    push    ebx
    and     ebx, 0FFFFh
    mov     [esp+4], ebx
    pop     ebx
    shr     ebx, 16
    mov     [esp+4], ebx

    mov     ebx, [tEBX]
    iretd

;============================================================================
Server:
    cmp     eax, 'V86I'
    je      Server.V86_Interrupt

    mov     eax, 'NIMP'
    ret

;============================================================================
Server.V86_Interrupt:
    mov     [V86TSS.ECX], ecx
    mov     [V86TSS.EBX], edx
    mov     [V86TSS.ESI], ebx
    mov     [V86TSS.DS], ebp
    mov     [V86TSS.ES], esi

    shr     esi, 16
    shr     edx, 16
    and     esi, 255
    shr     ebx, 16
    shr     ecx, 16
    shr     ebp, 16

    mov     [V86TSS.EAX], ecx
    mov     [V86TSS.EDX], edx
    mov     [V86TSS.EBP], ebx
    mov     [V86TSS.EDI], ebp

    mov     ebx, esi

    mov     eax, 09000h
    mov     ecx, 0C000h
    mov     [V86TSS.CS], eax
    mov     [V86TSS.EIP], ecx
    mov     [V86TSS.SS], eax
    mov     ecx, 0A000h
    mov     [V86TSS.ESP], ecx
    mov     [9C001h], bl

    str     ax

    mov     [DISTSS.EAX], 'ONEC'
    mov     [DISTSS.ECX], 20h
    mov     [DISTSS.EDX], eax

    int     30h

    mov     ecx, [V86TSS.EAX]
    mov     edx, [V86TSS.EDX]
    mov     ebx, [V86TSS.EBP]
    mov     ebp, [V86TSS.EDI]

    shl     edx, 16
    shl     ebx, 16
    shl     ecx, 16
    shl     ebp, 16

    and     dword ptr [V86TSS.ECX], 0FFFFh
    and     dword ptr [V86TSS.EBX], 0FFFFh
    and     dword ptr [V86TSS.ESI], 0FFFFh
    and     dword ptr [V86TSS.ECX], 0FFFFh
    and     dword ptr [V86TSS.DS], 0FFFFh
    and     dword ptr [V86TSS.ES], 0FFFFh

    or      ecx, [V86TSS.ECX]
    or      edx, [V86TSS.EBX]
    or      ebx, [V86TSS.ESI]
    or      ebp, [V86TSS.DS]
    or      esi, [V86TSS.ES]

    ret

;============================================================================
    .I = 0

    I00:    repeat 256
            \push   dword 4*I
            \db     0E9h
            \dd     InterruptHandler - $ - 4
            \.I     = I + 1
            endm
    IFF:

;============================================================================
;++
Print32:
    pushad
    mov     edi, ebx
    lea     ebx, [Digits]
    push    edx
    shr     edx, 16
    call    Print16
    pop     edx
    call    Print16
    popad
    ret

PrintByte:
    pushad
    mov     edi, ebx
    mov     al, 20h
    stosw
    lea     ebx, [Digits]
    call    Print8
    popad
    ret

Print16:
    push    dx
    mov     dl, dh
    call    Print8
    pop     dx
    call    Print8
    ret

Print8:
    push    dx
    shr     dl, 4
    call    Print4
    pop     dx
    call    Print4
    ret

Print4:
    mov     al, dl
    and     al, 15
    xlat
    stosw
    ret

Digits  db  "0123456789ABCDEF"

;--

;****************************************************************************

[Data]

    align 4
    V86TaskStateSegment:
        V86TSS.BackLink dd  0
        V86TSS.ESP0     dd  TempStackBottom
        V86TSS.SS0      dd  10h
        V86TSS.ESP1     dd  0
        V86TSS.SS1      dd  10h
        V86TSS.ESP2     dd  0
        V86TSS.SS3      dd  10h
        V86TSS.CR3      dd  0
        V86TSS.EIP      dd  0
        V86TSS.EFLAGS   dd  23202h
        V86TSS.EAX      dd  0
        V86TSS.ECX      dd  0
        V86TSS.EDX      dd  0
        V86TSS.EBX      dd  0
        V86TSS.ESP      dd  0
        V86TSS.EBP      dd  0
        V86TSS.ESI      dd  0
        V86TSS.EDI      dd  0
        V86TSS.ES       dd  0
        V86TSS.CS       dd  0
        V86TSS.SS       dd  0
        V86TSS.DS       dd  0
        V86TSS.FS       dd  0
        V86TSS.GS       dd  0
        V86TSS.LDT      dd  0
        V86TSS.Trap     dw  0
        V86TSS.MapBase  dw  $ - V86TaskStateSegment + 2
        V86TSS.IOMap    db  8192 dup (0)
    V86TaskStateSegmentEnd:

    align 4
    CynthiaTaskStateSegment:
        DEFTSS.BackLink dd  0
        DEFTSS.ESP0     dd  0
        DEFTSS.SS0      dd  0
        DEFTSS.ESP1     dd  0
        DEFTSS.SS1      dd  0
        DEFTSS.ESP2     dd  0
        DEFTSS.SS3      dd  0
        DEFTSS.CR3      dd  0
        DEFTSS.EIP      dd  0
        DEFTSS.EFLAGS   dd  0
        DEFTSS.EAX      dd  0
        DEFTSS.ECX      dd  0
        DEFTSS.EDX      dd  0
        DEFTSS.EBX      dd  0
        DEFTSS.ESP      dd  0
        DEFTSS.EBP      dd  0
        DEFTSS.ESI      dd  0
        DEFTSS.EDI      dd  0
        DEFTSS.ES       dd  0
        DEFTSS.CS       dd  0
        DEFTSS.SS       dd  0
        DEFTSS.DS       dd  0
        DEFTSS.FS       dd  0
        DEFTSS.GS       dd  0
        DEFTSS.LDT      dd  0
        DEFTSS.Trap     dw  0
        DEFTSS.MapBase  dw  $ - CynthiaTaskStateSegment + 2
        DEFTSS.IOMap    db  8192 dup (0)
    CynthiaTaskStateSegmentEnd:

    align 4
    DispatcherTSS:
        DISTSS.BackLink dd  0
        DISTSS.ESP0     dd  0
        DISTSS.SS0      dd  0
        DISTSS.ESP1     dd  0
        DISTSS.SS1      dd  0
        DISTSS.ESP2     dd  0
        DISTSS.SS3      dd  0
        DISTSS.CR3      dd  0
        DISTSS.EIP      dd  Dispatcher
        DISTSS.EFLAGS   dd  2002h
        DISTSS.EAX      dd  0   ;Execution Code
        DISTSS.ECX      dd  0   ;Next Task
        DISTSS.EDX      dd  0   ;Returning Task
        DISTSS.EBX      dd  0   ;Current Task
        DISTSS.ESP      dd  DispatcherStackBottom
        DISTSS.EBP      dd  0
        DISTSS.ESI      dd  0
        DISTSS.EDI      dd  0
        DISTSS.ES       dd  10h
        DISTSS.CS       dd  08h
        DISTSS.SS       dd  10h
        DISTSS.DS       dd  10h
        DISTSS.FS       dd  10h
        DISTSS.GS       dd  10h
        DISTSS.LDT      dd  0
        DISTSS.Trap     dw  0
        DISTSS.MapBase  dw  $ - DispatcherTSS + 2
    DispatcherTSSEnd:

;============================================================================

    align 4
    IDT:    repeat 32+16
            \dw     0
            \dw     08h
            \db     0
            \db     8Eh OR 60h
            \dw     0
            endm

            repeat 1
            \dw     0
            \dw     28H
            \db     0
            \db     85h OR 60h
            \dw     0
            endm

            repeat 256-32-16-1
            \dw     0
            \dw     08h
            \db     0
            \db     8Eh OR 60h
            \dw     0
            endm

    align 4
    IDTR    dw      7FFh
            dd      IDT

;============================================================================

    align 4

    ExitProtected   dd  0
    TotalPages      dd  0
    GDT             dd  0

    OriginalImage   dd  0
    ImageLength     dd  0

    tEBX            dd  0
    tPTR            dd  0
    iNUM            dd  0

;============================================================================

    ServiceTable    dd  32 dup (0)              ;00-1F
                    dd  17 dup (0)              ;20-30
                    dd  Server                  ;31
                    dd  256-32-16-1-1 dup (0)   ;32-FF

    ServiceTableF   dd  32 dup (0)              ;00-1F
                    dd  17 dup (0)              ;20-30
                    dd  80000000h               ;31
                    dd  256-32-16-1-1 dup (0)   ;32-FF

;============================================================================

    .FromV86        dd  0
    VideoMemory     dd  0

;============================================================================

    align 4
    org $+128
    DispatcherStackBottom:

    align 4
    org $+128
    TempStackBottom:

[ends]
