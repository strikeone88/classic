/*
    VFPRINTF.H

    Escrito por J. Palencia (zipox@ureach.com)
*/

#ifndef __VFPRINTF
#define __VFPRINTF

    #ifndef __FILE
    #include <file.h>
    #endif

    #ifndef __TYPES
    #include <types.h>
    #endif

    /* vfprintf: puntero a argumentos, archivo, cadena con formato. */
    int vfprintf (FILE *, const char *, va_list);

    /* fprintf: archivo, cadena con formato, argumentos variables. */
    int fprintf (FILE *, const char *, ...);

    /* vprintf: puntero a argumentos, cadena con formato a STDOUT. */
    int vprintf (const char *, va_list);

    /* printf: cadena con formato, argumentos variables a STDOUT. */
    int printf (const char *, ...);

    /* eprintf: cadena con formato, argumentos variables a STDERR. */
    int eprintf (const char *, ...);

#endif
