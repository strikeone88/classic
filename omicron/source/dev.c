/*
    DEV.C

    Manejador de Dispositivos

    Este modulo controla el registro de los dispositivos, provee acceso
    a handle por nombre, es decir, dado el nombre del manejador de un(os)
    dispositivo(s) se puede obtener un handle.

    Escrito por J. Palencia (zipox@ureach.com)
*/

    #include <string.h>
    #include <dev.h>
    #include <mm.h>

    /* El primer dispositivo registrado (inicialmente ninguno). */
    static dev_s *primero = NULL;

    /* El primer descriptor de archivo. */
    static FILE *desc = NULL;

    /* Crea un nuevo descriptor de archivo y lo retorna. */
    FILE *nuevo_descriptor (void)
    {
        FILE *p;

            p = kcalloc (sizeof (FILE));
            if (p == NULL) return NULL;

            return p;
    }

    /* Clona un descriptor. */
    FILE *clonar_descriptor (FILE *fp)
    {
        FILE *n = nuevo_descriptor ();

            if (n == NULL) return NULL;

            memcpy ((char *)n, (char *)fp, sizeof (FILE));

            return n;
    }

    /* Linkea el descriptor de archivo a la lista global. */
    void link_descriptor (FILE *x)
    {
            x->siguiente = desc;
            desc = x;
    }

    /* Deslinkea el descriptor de la lista global y lo borra. */
    void unlink_descriptor (FILE *x)
    {
        FILE *p, *q;

            for (p = NULL, q = desc; q != NULL; p = q, q = q->siguiente)
                if (q == x) break;

            if (q == NULL) return;

            q = q->siguiente;

            if (p == NULL)
                desc = q;
            else
                p->siguiente = q;

            kfree (x);
    }

    /* Retorna un puntero a la estructura del dispositivo dado el nombre. */
    dev_s *get_dev (char *nombre)
    {
        dev_s *p = primero;

            if (*nombre == '\0' && *(nombre + 1) == '\a')
                if (!strcmp (nombre + 2, "DEV")) return p;

            for (; p != NULL; p = p->siguiente)
                if (!strcmp (p->nombre, nombre)) return p;

            return p;
    }

    /* Esta funcion simplemente no hace nada mas que retornar cero. */
    static int no_operation (void)
    {
            return 0;
    }

    /* Checkea si p es NULL, si lo es le asigna la direccion de no_operation
       si no, simplemente retorna. */

    static void check_and_set (void **p)
    {
            if (*p == NULL) *p = (void *)&no_operation;
    }

    /* Registra un dispositivo, para hacer eso hay que linkear el nuevo
       dispositivo a la lista. */

    int register_dev (dev_s *s)
    {
            if (get_dev (s->nombre) != NULL) return -ERROR_GENERAL;

            s->siguiente = primero;
            primero = s;

            check_and_set (&s->start);
            check_and_set (&s->stop);

            check_and_set (&s->open);
            check_and_set (&s->configure);
            check_and_set (&s->close);

            check_and_set (&s->read);
            check_and_set (&s->write);

            check_and_set (&s->seek);

            if (s->start () < 0) return -ERROR_GENERAL;

            return 0;
    }

    /* De-registra un dispositivo. Alias: Remover de la lista. */
    int unregister_dev (dev_s *s)
    {
        dev_s *q, *p;

            for (q = NULL, p = primero; p != NULL; q = p, p = p->siguiente)
                if (p == s) break;

            if (p == NULL || s->stop () < 0) return -ERROR_GENERAL;

            p = p->siguiente;

            if (q != NULL)
                q->siguiente = p;
            else
                primero = p;

            return 0;
    }

    /* Abre un pseudo-archivo que sirve para interactuar con el dispositivo
       de verdad. */

    FILE *devopen (char *nombre, flag_t f)
    {
        dev_s *p = get_dev (nombre);
        FILE *q = NULL;

            if (p == NULL || (q = nuevo_descriptor ()) == NULL) return NULL;

            q->dev_type = p->dev_type;
            q->flags = f;
            q->dev = p;

            if (p->open (q) < 0)
            {
                kfree (q);
                return NULL;
            }

            link_descriptor (q);
            return q;
    }

    /* Lee n bytes del archivo en el buffer dado. */
    size_t fread (FILE *fp, size_t n, void *buf)
    {
            if (fp == NULL || fp->dev == NULL || fp->dev->read == NULL)
                return 0;

            return fp->dev->read (fp, n, buf);
    }

    /* Escribe n bytes desde el buffer en el archivo dado. */
    size_t fwrite (FILE *fp, size_t n, void *buf)
    {
            if (fp == NULL || fp->dev == NULL || fp->dev->write == NULL)
                return 0;

            return fp->dev->write (fp, n, buf);
    }

    /* Mueve el cursor del archivo a la posicion dada basandose en el
       modo de movimiento: SEEK_SET, SEEK_CUR o SEEK_END. */

    int fseek (FILE *fp, disp_t x, int seek_mode)
    {
            if (fp == NULL || fp->dev == NULL || fp->dev->seek == NULL)
                return -ERROR_GENERAL;

            return fp->dev->seek (fp, x, seek_mode);
    }

    /* Cierra el archivo (el handler de dispositivo deberia hacer flush). */
    int fclose (FILE *fp)
    {
        int i;

            if (fp == NULL || fp->dev == NULL || fp->dev->close == NULL)
                return -ERROR_GENERAL;

            if ((i = fp->dev->close (fp)) == 0)
                unlink_descriptor (fp);

            return i;
    }

    /* Escribe el caracter "x" en el archivo. */
    int fputc (FILE *fp, int x)
    {
            if ((fp->flags & O_TEXT) && x == '\n') fputc (fp, '\r');
            return fwrite (fp, 1, &x) != 1 ? -E_ESCRITURA : 0;
    }

    /* Lee un caracter del archivo. */
    int fgetc (FILE *fp)
    {
        int x;

            x = 0;

            return fread (fp, 1, &x) != 1 ? -E_LECTURA : x;
    }

    /* Lee una word del archivo. */
    int fgetw (FILE *fp)
    {
        int x;

            return fread (fp, 2, &x) != 2 ? -E_LECTURA : x;
    }

    /* Lee una dword del archivo. */
    unsigned long fgetd (FILE *fp)
    {
        long x;

            return fread (fp, 4, &x) != 4 ? -E_LECTURA : x;
    }

    /* Escribe la cadena "s" en el archivo. */
    int fputs (FILE *fp, const char *s)
    {
        size_t l = strlen (s);

            return fwrite (fp, l, s) != l ? -E_ESCRITURA : 0;
    }
