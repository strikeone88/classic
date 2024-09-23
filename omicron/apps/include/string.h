/*
    STRING.H
*/

#ifndef __STRING_H
#define __STRING_H

    /* Copia len bytes de src a dest y retorna dest. */
    void *memcpy (void *dest, const void *src, unsigned len);

    /* Escribe len bytes con valor ch a dest y retorna dest. */
    void *memset (void *dest, int ch, unsigned len);

    /* Copia len bytes (align 4) de src a dest y retorna dest. */
    void *dmemcpy (void *dest, const void *src, unsigned len);

    /* Escribe len bytes (align 4) con valor ch a dest y retorna dest. */
    void *dmemset (void *dest, int ch, unsigned len);

    /* Retorna el total de bytes en la cadena. */
    unsigned strlen (const char *src);

    /* Copia el contenido de src a dest y retorna dest. */
    char *strcpy (char *dest, const char *src);

    /* Retorna el puntero dentro de src donde esta ch. */
    char *strchr (const char *src, int ch);

    /* Compara dos cadenas y retorna 0(s1==s2), <0(s1<s2), >0(s1>s2). */
    int strcmp (const char *s1, const char *s2);

    /* Concatena la cadena src a dest y retorna dest. */
    char *strcat (char *dest, const char *src);

#endif
