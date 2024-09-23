/*
    CONSOLE.C

    Dispositivo Consola

    Este archivo contiene los metodos que pueden ser accesados mediante
    la interfaz de dev.c para utilizar la consola del sistema. El nombre
    del dispositivo es "CON".

    Escrito por J. Palencia (zipox@ureach.com)
*/

    #include <process.h>
    #include <string.h>
    #include <video.h>
    #include <keyb.h>
    #include <dev.h>

    /* La estructura dev_s del dispositivo "CON". */
    static dev_s console;

    /* El puntero a la consola virtual visible. */
    static vconsole_s *vcon = NULL;

    /* Inicializa la consola virtual a valores predeterminados. */
    void init_vconsole (vconsole_s *v)
    {
            v->clr_fondo = 0x00;
            v->clr_texto = 0x07;

            v->c_start = 0x0F;
            v->c_end = 0x0F;

            _clrscr (v, 0, 0x20);
    }

    /* Activa la consola virtual dada como la primaria. */
    void set_vconsole (vconsole_s *v)
    {
            if (v == vcon) return;

            if (vcon != NULL)
                memcpy (&vcon->video, (char *)VIDEO_MEM, 4000);

            if (v != NULL)
            {
                updateVideo (&v->video);

                configurar_cursor (v);
                actualizar_cursor (v);
            }

            vcon = v;
    }

    /* Limpia la consola pantalla de la consola virtual. */
    void clrscr (void)
    {
        vconsole_s *xcon = process->vconsole;

            if (xcon != NULL) _clrscr (xcon, xcon == vcon, 0x20);
    }

    /* Igualito a clrscr pero puede darle un caracter como parametro. */
    void c_clrscr (int c)
    {
        vconsole_s *xcon = process->vconsole;

            if (xcon != NULL) _clrscr (xcon, xcon == vcon, c);
    }

    /* Cambia el color de texto actual. */
    void setcolor (int x)
    {
        vconsole_s *xcon;

            if ((xcon = process->vconsole) != NULL)
                xcon->clr_texto = x;
    }

    /* Cambia el color de fondo actual. */
    void setbgcolor (int x)
    {
        vconsole_s *xcon;

            if ((xcon = process->vconsole) != NULL)
                xcon->clr_fondo = x;
    }

    /* Cambia el formato del cursor. */
    void setcursor (int s, int e)
    {
        vconsole_s *xcon = process->vconsole;

            if (xcon == NULL) return;

            xcon->c_start = s;
            xcon->c_end = e;

            if (xcon == vcon) configurar_cursor (xcon);
    }

    /* Mueve el cursor dentro de la consola virtual actual. */
    void gotoxy (int x, int y)
    {
        vconsole_s *xcon = process->vconsole;

            if (xcon != NULL) _gotoxy (xcon, x, y, xcon == vcon);
    }

    /* Retorna la posicion x actual. */
    int wherex (void)
    {
            if (process->vconsole == NULL) return 0;
            return process->vconsole->pos_x;
    }

    /* Retorna la posicion y actual. */
    int wherey (void)
    {
            if (process->vconsole == NULL) return 0;
            return process->vconsole->pos_y;
    }

    /* Lee "c" bytes desde el teclado en buf. */
    static size_t console_read (FILE *fp, size_t c, char *buf)
    {
        size_t t = 0;
        int ch, k;

            if (!(fp->flags & O_READ) || !c) return 0;

            if (fp->flags & O_RIN)
            {
                while (c-- && fp->pos < fp->mem_size)
                {
                    *buf++ = ((uchar *)fp->mem_ptr) [fp->pos];
                    t++;
                    fp->pos++;
                }
            }
            else
            {
                while (1)
                {
                    while (!kbhit ());

                    k = process->vconsole == vcon;
    
                    if ((ch = getch () & 0xFF) == 0) continue;

                    if (ch == '\r') break;

                    if (ch == '\b')
                    {
                        if (t < 1) continue;

                        _putch (process->vconsole, k, '\b');
                        _putch (process->vconsole, k, '\x20');
                        _putch (process->vconsole, k, '\b');

                        buf--;
                        t--;

                        continue;
                    }

                    if (t > c - 1) continue;

                    _putch (process->vconsole, k, *buf++ = ch);
                    t++;
                }
            }

            return t;
    }

    /* Escribe "c" bytes de buf en la consola virtual actual. */
    static size_t console_write (FILE *fp, size_t c, char *buf)
    {
        vconsole_s *xcon = process->vconsole;
        size_t t = 0;
        int k;

            if (!(fp->flags & O_WRITE) || !c) return 0;

            if (fp->available != 0) xcon = (vconsole_s *)fp->available;

            if (fp->flags & O_ROUT)
            {
                while (c-- && fp->mem_size-- > 0)
                {
                    *((uchar *)fp->mem_ptr)++ = *buf++;
                    t++;
                }
            }
            else
            {
                k = xcon == vcon;

                while (c--)
                {
                    _putch (xcon, k, *buf++);
                    t++;
                }
            }

            return t;
    }

    /* Mueve el cursor del archivo. */
    static int console_seek (FILE *fp, disp_t d, int mode)
    {
            switch (mode)
            {
                case SEEK_SET:  fp->pos = d;
                                break;

                case SEEK_CUR:  fp->pos += d;
                                break;
            }

            return 0;
    }

    /* Instala el dispositivo CON. */
    int instalar_CON (void)
    {
            /* Resetear la estructura a puros ceros (nulificar). */
            memset (&console, 0, sizeof(console));

            /* Asignar solo los metodos que vamos a implementar. */
            console.read = &console_read;
            console.write = &console_write;
            console.seek = &console_seek;

            console.nombre = "CON";
            console.dev_type = CHRDEV;

            return register_dev (&console);
    }
