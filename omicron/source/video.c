/*
    VIDEO.C

    Este archivo contiene las funciones que son necesarias para poder
    escribir en la pantalla, en modo texto 80x25.

    Escrito por J. Palencia (zipox@ureach.com)
*/

    #include <process.h>
    #include <string.h>
    #include <portio.h>
    #include <video.h>

    /* La dimension de la pantalla en caracteres (no en pixeles). */
    static const int ancho = 80, alto = 25;

    /* El numero de caracteres en una tabulacion. */
    static int tab_size = 4;

    /* Esta funcion configura el inicio y final del cursor de hardware. */
    void configurar_cursor (vconsole_s *v)
    {
            outportb (0x3D4, 0x0A);
            outportb (0x3D5, v->c_start);

            outportb (0x3D4, 0x0B);
            outportb (0x3D5, v->c_end);
    }

    /* Esta funcion mueve el cursor del hardware VGA a la posicion
       (pos_x, pos_y). */

    void actualizar_cursor (vconsole_s *v)
    {
        uint k = v->pos_y * ancho + v->pos_x;

            outportb (0x3D4, 0x0F);
            outportb (0x3D5, k & 0xFF);

            outportb (0x3D4, 0x0E);
            outportb (0x3D5, k >> 8);
    }

    /* Esta funcion chequea la posicion (x, y) para que no se salga del
       cuadro de pantalla (ancho x alto). */

    static void check_pos (vconsole_s *v, ushort *x)
    {
        uint k; int t;

            /* Si nos pasamos del ancho, entonces regresamos al
               inicio de la linea e incrementamos la posicion y. */

            if (v->pos_x >= ancho)
            {
                v->pos_x = 0;
                v->pos_y++;
            }

            /* Si nos pasamos del alto entonces hay que subir
               la pantalla para arriba por una linea (scrollup)
               y regresar la posicion y a el alto menos uno. */

            if (v->pos_y >= alto)
            {
                v->pos_y = alto - 1;

                if (!(v->c_start == v->c_end && v->c_start == 0))
                {
                    k = (ancho * (alto - 1)) << 1;
    
                    memcpy ((char *)x, (char *)(x + ancho), k);
    
                    x += k;
    
                    k = (v->clr_fondo << 12) |
                        (v->clr_texto << 8);
    
                    for (t = 0; t < ancho; t++) *x++ = k;
                }
            }

            if (x == VIDEO_MEM) actualizar_cursor (v);
    }

    /* Escribe el caracter dado en la posicion (pos_x, pos_y) con color
       de fondo clr_fondo y color de texto clr_texto en la pantalla. */

    void _putch (vconsole_s *v, int m, int ch)
    {
        ushort k, *video;

            /* Entrar a area critica. */
            enter_critical ();

            video = m ? VIDEO_MEM : &v->video;

            k = (v->clr_fondo << 12) | (v->clr_texto << 8) | (ch & 0xFF);

            /* Hay que chequear el valor dado para ver si es un caracter
               de control, si es asi, hay que tomar las acciones que
               sean necesarias, sino, pues simplemente se escribira el
               caracter dado. No todos los caracteres de control se
               trataran como tales, algunos simplemente seran escritos
               en la pantalla.
            */

            switch (ch)
            {
                case '\b':  /* Backspace */

                    /* Accion: Decrementar la posicion x, si el resultado
                               es negativo, resetear x a cero. */

                    if (--v->pos_x < 0) v->pos_x = 0;
                    break;

                case '\t': /* Tabulacion Horizontal */

                    /* Accion: Incrementar la posicion x por un numero q
                               dicho numero es el residuo de la division
                               de la posicion x actual entre el numero de
                               caracteres de una tabulacion (tab_size).
                    */

                    v->pos_x += v->pos_x % tab_size;
                    break;

                case '\n': /* Line feed, ir a la siguiente linea. */

                    /* Accion: Incrementar la posicion y. */
                    v->pos_x = 0;
                    v->pos_y++;
                    break;

                case '\r': /* Carriage return, regresar al inicio de la
                              linea. */

                    /* Accion: Resetear el valor de la posicion x. */
                    v->pos_x = 0;
                    break;

                default: /* Solo hay que escribir el caracter e
                            incrementar la posicion x. */

                    check_pos (v, video);

                    *(video + v->pos_y * ancho + v->pos_x++) = k;

                    break;
            }

            check_pos (v, video);

            /* Salir del area critica. */
            leave_critical ();
    }

    /* Mueve el cursor logico y el de hardware si m = 1. */
    void _gotoxy (vconsole_s *v, int x, int y, int m)
    {
            if (x > ancho - 1 || y > alto - 1) return;

            v->pos_x = x;
            v->pos_y = y;

            if (m) actualizar_cursor (v);
    }

    /* Limpia una pantalla virtual. */
    void _clrscr (vconsole_s *v, int m, int a)
    {
        ushort *vid = m ? VIDEO_MEM : &v->video;
        uint k, t = ancho * alto;

            k = (v->clr_fondo << 12) |
                (v->clr_texto << 8) | a;

            while (t--) *vid++ = k;

            _gotoxy (v, 0, 0, m);
    }
