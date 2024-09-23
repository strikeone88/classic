/*
    STRING.C

    Modulo para la Manipulacion de Cadenas y Bloques de Memoria

    Este archivo contiene las funciones que son necesarias para poder
    manipular cadenas (strings) o bloques de memoria.

    Escrito por J. Palencia (zipox@ureach.com)
*/

    #include <types.h>

    /* Copia los primeros "count" byte de "src" en "dest". */
    void memcpy (char *dest, const char *src, count_t count)
    {
            if (dest == NULL || src == NULL || !count) return;
            while (count--) *dest++ = *src++;
    }

    /* Le asigna a los primeros "c" bytes de "s" el valor "x". */
    void memset (char *s, char x, count_t c)
    {
            if (s == NULL) return;
            while (c--) *s++ = x;
    }

    /* Retorna el total de bytes contenidos en la cadena. */
    size_t strlen (const char *s)
    {
        size_t c = 0;

            if (s == NULL) return 0;

            while (*s++ != '\0') c++;

            return c;
    }

    /* Compara ambas cadenas, retorna < 0 si s1 < s2, > 0 si s1 > s2
       y = 0 si s1 = s2. */

    int strcmp (const char *s1, const char *s2)
    {
        int i = 0, a = 1, b = 1;

            if (s1 == NULL || s2 == NULL) return -1;

            while (!i && (a || b))
                i = (a = *s1++) - (b = *s2++);

            return i;
    }

    /* Cambia las letras de la cadena a mayusculas. */
    void strupr (char *s)
    {
        int c;

            if (s == NULL) return;

            for (; (c = *s) != '\0'; s++)
            {
                if (c >= 'a' && c <= 'z')
                    *s = c & 0xDF;
            }
    }

    /* Cambia las letras de la cadena a minusculas. */
    void strlwr (char *s)
    {
        int c;

            if (s == NULL) return;

            for (; (c = *s) != '\0'; s++)
            {
                if (c >= 'A' && c <= 'Z')
                    *s = c | 0x20;
            }
    }

    /* Retorna el puntero a la primera ocurrencia de "c" en "s". */
    char *strchr (const char *s, int c)
    {
            if (s == NULL) return NULL;

            for (; *s != '\0'; s++) if (*s == c) return s;
            return NULL;
    }
