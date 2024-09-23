/*
    VFPRINTF.C

    Escritura de cadenas con formato y argumentos variables
    a archivos, STDOUT o STDERR.

    Escrito por J. Palencia (zipox@ureach.com)
*/

    #include <process.h>
    #include <string.h>
    #include <file.h>

    /* Estos son los indicadores internos de vfprintf. */
    enum flags_de_vfprintf
    {
        PUNTERO_FAR = 0x01,        SPEC_LONG   = 0x02,
        L_ALIGN     = 0x04,        VALOR_HEX   = 0x08,
        VALOR_OCT   = 0x10,        VALOR_DEC   = 0x20,
        VALOR_POS   = 0x40
    };

    /* Buffer temporal para el numero en forma ASCII. */
    static char nbuf [32];

    /* Retorna el digito ASCII dado un numero. */
    static int digit (int n)
    {
            if (0 <= n && n <= 9)
                return n + '0';

            return n - 0x0A + 'a';
    }

    /* Convierte un numero entero a base n en ASCII. */
    void itoa (ulong x, char *buf, int n)
    {
        int i = 0;

            do
            {
                nbuf [i++] = x % n;
                x /= n;
            }
            while (x);

            while (i--) *buf++ = digit ((unsigned char)nbuf [i]);
            *buf = '\0';
    }

    /* vfprintf: puntero a argumentos, archivo, cadena con formato. */
    int vfprintf (FILE *fp, const char *s, va_list p)
    {
        int c, filler, tmp, pos_prefix, xo_prefix, chars, total;
        char tbuf [32]; char *tmp2; long tmp3;
        flag_t flags;

            if (fp == NULL || s == NULL) return -PUNTERO_NULO;

            while ((c = *s++) != '\0')
            {
                if (c != '%')
                {
                    if (fputc (fp, c)) return -1;
                    continue;
                }

                /* Hay que recordar que el formato va precedido SIEMPRE por
                   el signo de porcentaje. */

                total = xo_prefix = pos_prefix = 0;
                flags = PUNTERO_FAR;
                filler = 0x20;

            siguiente_modificador:;
                if ((c = *s++) == '\0') break;

                switch (c)
                {
                    case 'N': /* Puntero NEAR */
                        flags &= ~PUNTERO_FAR;
                        break;

                    case 'F': /* Puntero FAR */
                        flags |= PUNTERO_FAR;
                        break;

                    case 'h': /* Especifica short */
                        flags &= ~SPEC_LONG;
                        break;

                    case 'l': /* Especifica long */
                        flags |= SPEC_LONG;
                        break;

                    case '#': /* Prefijo de Hex: 0x, Oct: 0 */
                        xo_prefix = 0x30;
                        break;

                    case 'r': /* Cambiar caracter filler. */
                        filler = va_arg(p, uint);
                        break;

                    case ' ': /* Prefijo de Numeros Positivos: Espacio */
                        pos_prefix = 0x20;
                        break;

                    case '+': /* Prefijo de Numeros Positivos: Signo Mas */
                        pos_prefix = 0x2B;
                        break;

                    case '*': /* Total de chars en cadena de resultado */
                        total = va_arg(p, uint);
                        break;

                    case '-': /* Alinear a la izquierda. */
                        flags |= L_ALIGN;
                        break;

                    case '0': /* Cambiar filler a digito cero si !total. */
                        if (!total)
                        {
                            filler = 0x30;
                            break;
                        }

                    case '1': /* Digitos */
                    case '2': /* Estos sirven para cambiar el total de */
                    case '3': /* caracteres en la cadena resultante, */
                    case '4': /* en otras palabras, es igual a utilizar */
                    case '5': /* el asterisco (*) pero usar los digitos */
                    case '6': /* es mas inmediato. */
                    case '7':
                    case '8': /* Se pueden escribir multiples digitos */
                    case '9': /* para formar un numero, como 12. */
                        total = (total * 10) + (c - 0x30);
                        break;

                    case 'c': /* Escribir Caracter */
                        tbuf [0] = va_arg(p, uint);

                    un_char:;
                        tmp2 = &tbuf;
                        chars = 1;
                        goto alinear;

                    case 's': /* Cadena */
                        tmp2 = va_arg(p, char *);

                        chars = strlen (tmp2);
                        goto alinear;

                    case 'x': /* Numero en formato hexadecimal */
                        itoa (flags & SPEC_LONG ? va_arg(p, ulong) :
                              va_arg(p, uint), &tbuf, 16);

                    hex_val:;
                        flags |= VALOR_HEX;
                        chars = strlen (tmp2 = &tbuf);
                        if (!chars)
                        { fix_one:;
                            chars = 1;
                            *tmp2 = 0x30;
                        }
                        goto alinear;

                    case 'X': /* Numero hexadecimal en mayuscula */
                        itoa (flags & SPEC_LONG ? va_arg(p, ulong) :
                              va_arg(p, uint), &tbuf, 16);

                        strupr (&tbuf);
                        goto hex_val;

                    case 'o': /* Numero en formato octal */
                        itoa (flags & SPEC_LONG ? va_arg(p, ulong) :
                              va_arg(p, uint), &tbuf, 8);

                        flags |= VALOR_OCT;
                        chars = strlen (tmp2 = &tbuf);
                        if (!chars) goto fix_one;
                        goto alinear;

                    case 'i':
                    case 'd': /* Numero en formato decimal */
                        tmp3 = flags & SPEC_LONG ? va_arg(p, long) :
                                va_arg(p, int);

                        itoa (tmp3 < 0 ? -tmp3 : tmp3, &tbuf, 10);

                        if (tmp3 >= 0) flags |= VALOR_POS;

                    valor_dec:;
                        flags |= VALOR_DEC;

                        chars = strlen (tmp2 = &tbuf);
                        if (!chars) goto fix_one;
                        goto alinear;

                    case 'u': /* Numero en formato decimal (unsigned) */
                        tmp3 = flags & SPEC_LONG ? va_arg(p, ulong) :
                                va_arg(p, uint);

                        itoa (tmp3, &tbuf, 10);

                        flags |= VALOR_POS;
                        goto valor_dec;

                    default: /* Caracter de formato invalido */
                        total = 0;
                        tbuf [0] = c;
                        goto un_char;
                }

                goto siguiente_modificador;

            alinear:;
                if (xo_prefix)
                {
                    if (flags & VALOR_HEX)
                    {
                        if (filler == 0x30) fputs (fp, "0x");
                        total -= 2;
                    }
                    else
                    if (flags & VALOR_OCT)
                    {
                        if (filler == 0x30) fputc (fp, '0');
                        total--;
                    }
                }

                if (flags & VALOR_DEC)
                {
                    if (filler == 0x30)
                    {
                        if (!(flags & VALOR_POS))
                        {
                            fputc (fp, '-');
                            total--;
                        }
                        else
                        if (pos_prefix)
                        {
                            fputc (fp, pos_prefix);
                            total--;
                        }
                    }
                }

                c = total - chars;

                /* Si se esta alineando a la derecha, entonces hay que
                   escribir el caracter filler, c veces ANTES de la
                   cadena (es decir ahora mismo). */

                if (!(flags & L_ALIGN))
                {
                    for (tmp = c; tmp > 0; tmp--)
                        if (fputc (fp, filler)) return -1;
                }

                if (filler != 0x30)
                {
                    if (xo_prefix)
                    {
                        if (flags & VALOR_HEX)  fputs (fp, "0x");
                        else
                        if (flags & VALOR_OCT)  fputc (fp, '0');
                    }

                    if (flags & VALOR_DEC)
                    {
                        if (!(flags & VALOR_POS))
                            fputc (fp, '-');
                        else
                            if (pos_prefix) fputc (fp, pos_prefix);
                    }
                }

                /* Ahora hay que escribir la cadena. */
                while (chars--)
                    if (fputc (fp, *tmp2++)) return -1;

                /* Ahora, si estamos alineando a la izquerda entonces se
                   escribe en filler, c veces despues de la cadena, es
                   decir, ahora. */

                if (flags & L_ALIGN)
                {
                    for (tmp = c; tmp > 0; tmp--)
                        if (fputc (fp, filler)) return -1;
                }
            }

            return 0;
    }

    /* fprintf: archivo, cadena con formato, argumentos variables. */
    int fprintf (FILE *fp, const char *s, ...)
    {
            return vfprintf (fp, s, get_valist (s));
    }

    /* vprintf: puntero a argumentos, cadena con formato a STDOUT. */
    int vprintf (const char *s, va_list p)
    {
            return vfprintf (process->stdout, s, p);
    }

    /* printf: cadena con formato, argumentos variables a STDOUT. */
    int printf (const char *s, ...)
    {
            return vfprintf (process->stdout, s, get_valist (s));
    }

    /* eprintf: cadena con formato, argumentos variables a STDERR. */
    int eprintf (const char *s, ...)
    {
            return vfprintf (process->stderr, s, get_valist (s));
    }
