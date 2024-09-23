/*
    CONIO.H
*/

#ifndef __CONIO_H
#define __CONIO_H

    #ifndef __BASICS_H
    #include <basics.h>
    #endif

    /* Limpia la consola pantalla de la consola virtual. */
    #define clrscr() (void)KernelInterface (F__CLRSCR)

    /* Igualito a clrscr pero puede darle un caracter como parametro. */
    #define c_clrscr(x) (void)KernelInterface (F__C_CLRSCR, (int)(x))

    /* Cambia el color de texto actual. */
    #define setcolor(x) (void)KernelInterface (F__SETCOLOR, (int)(x))

    /* Cambia el color de fondo actual. */
    #define setbgcolor(x) (void)KernelInterface (F__SETBGCOLOR, (int)(x))

    /* Cambia el formato del cursor. */
    #define setcursor(x,y) (void)KernelInterface (F__SETCURSOR, (int)(x), (int)(y))

    /* Mueve el cursor dentro de la consola virtual actual. */
    #define gotoxy(x,y) (void)KernelInterface (F__GOTOXY, (int)(x), (int)(y))

    /* Retorna la posicion x actual. */
    #define wherex() (int)KernelInterface (F__WHEREX)

    /* Retorna la posicion y actual. */
    #define wherey() (int)KernelInterface (F__WHEREY)

    /* Retorna el numero de teclas en el buffer del teclado. */
    #define kbhit() (int)KernelInterface (F__KBHIT)

    /* Retorna un par SCANCODE:ASCII del buffer del teclado, si no hay
       nada, espera a que haya algo. */

    #define getch() (int)KernelInterface (F__GETCH)

    /* Class Code 0000 */
    enum CC_0000
    {
        F__CLRSCR = 0x00000000,     /* (void) : 0 */
        F__C_CLRSCR,                /* (int color) : 0 */
        F__SETCOLOR,                /* (int color) : 0 */
        F__SETBGCOLOR,              /* (int color) : 0 */
        F__SETCURSOR,               /* (int start, int end) : 0 */
        F__GOTOXY,                  /* (int x, int y) : 0 */
        F__WHEREX,                  /* (void) : int */
        F__WHEREY,                  /* (void) : int */
        F__KBHIT,                   /* (void) : int */
        F__GETCH,                   /* (void) : int */
    };

#endif
