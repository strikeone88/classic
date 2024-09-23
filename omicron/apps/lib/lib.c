/*
    LIB.C :: Library Functions
*/

    #include <stdarg.h>
    #include <stdio.h>
    #include <stdlib.h>

    /* Copia len bytes de src a dest y retorna dest. */
    void *memcpy (void *dest, const void *src, unsigned len)
    {
        void *orig = dest;

            while (len--)
                *((char *)dest)++ = *((char *)src)++;

            return orig;
    }

    /* Escribe len bytes con valor ch a dest y retorna dest. */
    void *memset (void *dest, int ch, unsigned len)
    {
        void *orig = dest;

            while (len--) *((char *)dest)++ = ch;

            return orig;
    }

    /* Retorna el total de bytes en la cadena. */
    unsigned strlen (const char *src)
    {
        unsigned len = 0;

            while (*src++ != '\0') len++;

            return len;
    }

    /* Copia el contenido de src a dest y retorna dest. */
    char *strcpy (char *dest, const char *src)
    {
            return memcpy (dest, src, strlen (src) + 1);
    }

    /* Retorna el puntero dentro de src donde esta ch. */
    char *strchr (const char *src, int ch)
    {
            while (*src != '\0' && *src != ch) src++;

            return *src == '\0' ? (void *)0 : (char *)src;
    }

    /* Compara dos cadenas y retorna 0(s1==s2), <0(s1<s2), >0(s1>s2). */
    int strcmp (const char *s1, const char *s2)
    {
        int a, b, c;

            do
            {
                a = *s1++;
                b = *s2++;

                c = a - b;
            }
            while (a && b && !c);

            return c;
    }

    /* Concatena la cadena src a dest y retorna dest. */
    char *strcat (char *dest, const char *src)
    {
            memcpy (dest + strlen (dest), src, strlen (src) + 1);
            return dest;
    }

    /* sprintf: buffer, cadena con formato. */
    void sprintf (char *buf, const char *fmt, ...)
    {
        FILE *temp = devopen ("CON", O_WRITE | O_ROUT);
        va_list ptr;

            va_start (ptr, fmt);
            *buf = '\0';

            if (temp == NULL) return;

            temp->mem_size = -1U;
            temp->mem_ptr = buf;

            vfprintf (temp, fmt, ptr);

            fclose (temp);
    }

    /* vsprintf: puntero a parameters, buffer, cadena con formato. */
    void vsprintf (char *buf, const char *fmt, void *args)
    {
        FILE *temp = devopen ("CON", O_WRITE | O_ROUT);

            *buf = '\0';

            if (temp == NULL) return;

            temp->mem_size = -1U;
            temp->mem_ptr = buf;

            vfprintf (temp, fmt, args);

            buf = temp->mem_ptr;
            *buf = '\0';

            fclose (temp);
    }

    /* fprintf: archivo, cadena con formato, argumentos variables. */
    int fprintf (FILE *fp, const char *s, ...)
    {
            return vfprintf (fp, s, get_valist (s));
    }

    /* printf: cadena con formato, argumentos variables a STDOUT. */
    int printf (const char *s, ...)
    {
            return vprintf (s, get_valist (s));
    }

    int rand (void)
    {
        static int multiplier = 0xA5791D6D;
        static int t, seed = 1;

            seed = seed * multiplier - 1;
            return (seed & 0x7FFE) + 1;
    }

    void delay (int clocks)
    {
        int ETA = clock () + clocks;

            while (clock () < ETA);
    }
