/*
    CONSOLE.H

    Escrito por J. Palencia (zipox@ureach.com)
*/

#ifndef __CONSOLE
#define __CONSOLE

    #ifndef __TYPES
    #include <types.h>
    #endif

    /* El formato de una consola virtual. */
    typedef struct vconsole_s /* 4010 bytes */
    {
        int     pos_x, pos_y;

        int     clr_fondo, clr_texto;
        char    c_start, c_end;

        ushort  video [2000];
    }
    vconsole_s;

    /* Instala el dispositivo CON. */
    int instalar_CON (void);

    /* Inicializa la consola virtual a valores predeterminados. */
    void init_vconsole (vconsole_s *v);

    /* Limpia la consola pantalla de la consola virtual. */
    void clrscr (void);

    /* Igualito a clrscr pero puede darle un caracter como parametro. */
    void c_clrscr (int c);

    /* Cambia el color de texto actual. */
    void setcolor (int x);

    /* Cambia el color de fondo actual. */
    void setbgcolor (int x);

    /* Cambia el formato del cursor. */
    void setcursor (int s, int e);

    /* Mueve el cursor dentro de la consola virtual actual. */
    void gotoxy (int x, int y);

    /* Retorna la posicion x actual. */
    int wherex (void);

    /* Retorna la posicion y actual. */
    int wherey (void);

    /* Activa la consola virtual dada como la primaria. */
    void set_vconsole (vconsole_s *v);

#endif
