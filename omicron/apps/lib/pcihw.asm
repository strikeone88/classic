Section _TEXT byte public 'CODE'
Publics _
Bits    32

[_TEXT]

_pci_pack:
    MOV     EAX, [ESP+04H]
    MOV     ECX, [ESP+08H]
    MOV     EDX, [ESP+0CH]
    SHL     EAX, 10H
    SHL     ECX, 0BH
    SHL     EDX, 08H
    ADD     EAX, ECX
    ADD     EAX, EDX
    OR      EAX, 1 SHL 31
    RET

_pci_set_reg:
    MOV     EBX, [ESP+4]
    MOV     EAX, [ESP+8]
    MOV     [EBX], AL
    RET

_pci_set_bus:
    MOV     EBX, [ESP+4]
    MOV     EAX, [ESP+8]
    MOV     [EBX+2], AL
    RET

_pci_set_dev:
    MOV     EBX, [ESP+4]
    MOV     EAX, [ESP+8]
    SHL     EAX, 3
    AND     B[EBX+1], 7H
    OR      B[EBX+1], AL
    RET

_pci_set_func:
    MOV     EBX, [ESP+4]
    MOV     EAX, [ESP+8]
    AND     B[EBX+1], 0F8H
    OR      B[EBX+1], AL
    RET

_pci_config_addr:
    MOV     EDX, 0CF8H
    MOV     EAX, [ESP+4]
    OUT     DX, EAX
    RET

_pci_getb:
    SUB     EAX, EAX
    MOV     EDX, 0CFCH
    ADD     EDX, [ESP+4]
    IN      AL, DX
    RET

_pci_getw:
    SUB     EAX, EAX
    MOV     EDX, 0CFCH
    ADD     EDX, [ESP+4]
    IN      AX, DX
    RET

_pci_getd:
    MOV     EDX, 0CFCH
    IN      EAX, DX
    RET

_pci_setw:
    MOV     EDX, 0CFCH
    ADD     EDX, [ESP+4]
    MOV     EAX, [ESP+8]
    OUT     DX, AX
    RET

_pci_setb:
    MOV     EDX, 0CFCH
    ADD     EDX, [ESP+4]
    MOV     EAX, [ESP+8]
    OUT     DX, AL
    RET

_pci_setd:
    MOV     EDX, 0CFCH
    MOV     EAX, [ESP+4]
    OUT     DX, EAX
    RET

[ENDS]
