/*
    STDLIB.H
*/

#ifndef __STDLIB_H
#define __STDLIB_H

    #ifndef __BASICS_H
    #include <basics.h>
    #endif

    #ifndef __PROCESS_H
    #include <process.h>
    #endif

    #ifndef __STDIO_H
    #include <stdio.h>
    #endif

    #ifndef __STRING_H
    #include <string.h>
    #endif

    /**/
    #define min(a,b) (((a) < (b)) ? (a) : (b))
    #define max(a,b) (((a) > (b)) ? (a) : (b))
    #define abs(a) (((a) < 0) ? -(a) : (a))

    int rand (void);

    /* Localiza un bloque de memoria. */
    #define malloc(x) (void *)KernelInterface (F__MALLOC, (unsigned)(x))

    /* Igualito a calloc pero nulifica el bloque. */
    #define calloc(x) (void *)KernelInterface (F__CALLOC, (unsigned)(x))

    /* Libera un bloque. */
    #define free(x) (void)KernelInterface (F__FREE, (void *)(x))

    /* Retorna el total de memoria libre en el sistema. */
    #define coreleft() (int)KernelInterface (F__CORELEFT)

    /* Cambia el due\xA4o de un bloque de memoria. */
    #define chblkown(x,y) (void)KernelInterface (F__CHBLKOWN, (void *)(x), (process_s *)(y))

    /* Retorna el total de memoria utilizada por un proceso. */
    #define psmem(x) (int)KernelInterface (F__PSMEM, (process_s *)(x))

    /* Libera todos los bloques de memoria de un proceso. */
    #define freepsmem(x) (void)KernelInterface (F__FREEPSMEM, (process_s *)(x))

    /* Dumpea los nodos de control de memoria. */
    #define dumpMemNodes(x,y) (void)KernelInterface (F__DUMPMEMNODES, (FILE *)(x), (int)(y))

    /* Class Code 0002 */
    enum CC_0002
    {
        F__MALLOC = 0x00020000,     /* (unsigned) : void * */
        F__CALLOC,                  /* (unsigned) : void * */
        F__FREE,                    /* (void *) : void */
        F__CORELEFT,                /* (void) : int */
        F__CHBLKOWN,                /* (void *, process_s *) : void */
        F__PSMEM,                   /* (process_s *) : int */
        F__FREEPSMEM,               /* (process_s *) : void */
        F__DUMPMEMNODES,            /* (FILE *, int) : void */
    };

#endif
