/*
    PORTIO.H

    Port Input/Output (Entrada/Salida a Traves de Puertos)

    Las funciones utilizadas para accesar los puertos del x86
    estan escritas en assembler, localizadas en start.asm,
    pero se pueden utilizar desde C utilizando este archivo.

    Escrito por J. Palencia (zipox@ureach.com)
*/

#ifndef __PORTIO
#define __PORTIO

    #ifndef __TYPES
    #include <types.h>
    #endif

    void outportb (uint, uint);
    void outportw (uint, uint);
    void outportd (uint, uint);

    uint inportb (uint);
    uint inportw (uint);
    uint inportd (uint);

#endif
