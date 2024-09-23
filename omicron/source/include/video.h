/*
    VIDEO.H

    Escrito por J. Palencia (zipox@ureach.com)
*/

#ifndef __VIDEO
#define __VIDEO

    #ifndef __CONSOLE
    #include <console.h>
    #endif

    #ifndef __TYPES
    #include <types.h>
    #endif

    /* Direccion logica de la memoria de video en modo texto. */
    #define VIDEO_MEM   (ushort *)0xB8000

    /* Esta funcion configura el inicio y final del cursor de hardware. */
    void configurar_cursor (vconsole_s *v);

    /* Esta funcion mueve el cursor del hardware VGA a la posicion
       (pos_x, pos_y). */

    void actualizar_cursor (vconsole_s *v);

    /* Escribe el caracter dado en la posicion (pos_x, pos_y) con color
       de fondo clr_fondo y color de texto clr_texto en la pantalla. */

    void _putch (vconsole_s *v, int m, int ch);

    /* Limpia una pantalla virtual. */
    void _clrscr (vconsole_s *v, int m, int a);

    /* Mueve el cursor logico (y el de hardware si m = 1). */
    void _gotoxy (vconsole_s *v, int x, int y, int m);

    /* Actualiza la memoria de video con x. */
    void updateVideo (uint *x);

#endif
