/*
    STDARG.H
*/

#ifndef __STDARG_H
#define __STDARG_H

    #ifndef __BASICS_H
    #include <basics.h>
    #endif

    /* */
    #define va_start(x,p) ((x)=(va_list)(((char *)&p)+((sizeof(p)+3)&-4)))

    /* */
    #define get_valist(p) (va_list)(((char *)&p)+((sizeof(p)+3)&-4))

    /* Lee un elemento de tipo "t" del puntero "x" e incrementa a este. */
    #define va_arg(x,t) (*((t *)(x))++)

#endif
