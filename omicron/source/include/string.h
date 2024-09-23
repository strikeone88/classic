/*
    STRING.H

    Manipulacion de Cadenas y Bloques de Memoria

    Este archivo contiene los prototipos de las funciones que son
    utilizadas para manipular cadenas y bloques de memoria.

    Escrito por J. Palencia (zipox@ureach.com)
*/

#ifndef __STRING
#define __STRING

    #ifndef __TYPES
    #include <types.h>
    #endif

    /* Copia n bytes de s en d. */
    void memcpy (char *, const char *, count_t);

    /* Le asigna a los primeros "c" bytes de "s" el valor "x". */
    void memset (char *s, char, count_t);

    /* Retorna el tama\xA4o de una cadena. */
    size_t strlen (const char *);

    /* Compara dos cadenas y retorna < 0 si A < B, > 0 si A > B o 0 si
       A es igual a B. */

    int strcmp (const char *, const char *);

    /* Convierte los caracteres de s a mayuscula. */
    void strupr (char *);

    /* Convierte los caracteres de s a minuscula. */
    void strlwr (char *);

    /* Retorna un puntero a la primera ocurrencia de c en s. */
    char *strchr (const char *, int);

#endif
