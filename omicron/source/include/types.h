/*
    TYPES.H

    Tipos de Datos y Muchas Macros Muy Utiles

    Este archivo contiene las definiciones de los tipos de datos que
    vamos a utilizar. Basicamente son utilizados para que los
    codigos fuente se vean un poquito mejor y mas claros.

    Escrito por J. Palencia (zipox@ureach.com)
*/

#ifndef __TYPES
#define __TYPES

    #ifndef __ERRORS
    #include <errors.h>
    #endif

    /* El famoso NULL */
    #define NULL    (void *)(0)

    /* Retorna un valor mayor o igual a "a" que sea divisible por "b". */
    #define align(a,b) ((a + b - 1) & -b)

    /* */
    #define get_valist(p) (va_list)(((char *)&p)+((sizeof(p)+3)&-4))

    /* Lee un elemento de tipo "t" del puntero "x" e incrementa a este. */
    #define va_arg(x,t) (*((t *)(x))++)

    /* Lee un elemento con indice "i" de tipo "t" del puntero "x". */
    #define va_argi(x,t,i) *(((t *)(x)) + (i))

    /* Retorna el numero de elementos en un arreglo. */
    #define array_size(x)   (sizeof(x)/sizeof(x [0]))

    /* Tipos de datos con nombres descriptivos. */

    typedef unsigned char   uchar;
    typedef unsigned int    uint;
    typedef unsigned short  ushort;
    typedef unsigned long   ulong;

    typedef unsigned long   memaddr_t;
    typedef unsigned int    size_t;
    typedef unsigned int    count_t;
    typedef unsigned int    index_t;

    typedef long            disp_t;
    typedef unsigned long   offset_t;
    typedef unsigned long   flag_t;
    typedef unsigned long   time_t;

    typedef void            *va_list;

    /* La estructura de estado, sirve para guardar el estado de un
       proceso antes de saltar a otro. */

    typedef struct /* 28 bytes */
    {
        /* Estos son los registros de la arq. x86 32-bits. */

        /* Registros de Proposito General. */
        uint        EAX, ECX, EDX, EBX, EBP, ESI, EDI;

        /* Registros de Segmentos. */
        uint        DS, ES;

        /* Registros Utilizados por switch_task. */
        uint        ESP, SS, EIP, CS, EFLAGS;

        /* Todos son visibles al usuario excepto EIP y EFLAGS
           que no se pueden acceder directamente, pero aun
           asi son accesibles.
        */
    }
    state_s;

    typedef struct
    {
        unsigned    ax, bx, cx, dx;
        unsigned    bp, si, di, ds, es;
    }
    REGPACK;

    /* Cambia la direccion de la ISR de una interrupcion. */
    void setvect (int, void *, int);

    /* Lee la direccion de la ISR de una interrupcion. */
    void *getvect (int);

    /* Guarda el contexto del procesador. */
    void save_state (state_s *);

    /* Cambia del proceso SRC a DEST. */
    void switch_task (state_s *dest, state_s *src);

    /* Ejecuta una interrupcion en modo v86. */
    void v86int (int, REGPACK *);

#endif
