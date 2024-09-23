/*
    BF2.C

    RedStar Binary Font 2.0 Engine Version 0.03

    Copyright (C) 2007-2008 Novak Laboratories
    Written by J. Palencia (ilina@bloodykisses.zzn.com)
*/

    #include <string.h>
    #include <stdarg.h>
    #include <stdio.h>

    /* Definition of a byte (unsigned 8-bit integer). */
    #define byte_t      unsigned char

    /* Length of an space in pixels. */
    int bf2__space;

    /* Offsets table. */
    static byte_t *OffsTable [256];

    /* BF2 header information. */
    static int startCh, endCh, charHeight;

    /* This is a global general purpose temporal buffer. */
    extern char g__tempBuf [1024];

    /* Sets the internal font variables given BF2 font data, returns
       non-zero if the font was not set. */

    int bf2__set (char *p, unsigned len)
    {
        char *q = p;
        int i, j;

            if (*((unsigned long *)p) != 0x21324642UL)
                return 1;

            charHeight = *((byte_t *)(p + 0x06));
            startCh = *((byte_t *)(p + 0x04));
            endCh = *((byte_t *)(p + 0x05));

            p += len - ((j = endCh - startCh + 1) << 1);

            for (i = 0; i < j; i++)
                OffsTable [i] = q + *(*(unsigned **)&p)++;

            bf2__space = 5;

            return 0;
    }

    /* Returns the length in pixels of the string. */
    int bf2__strlen (char *s)
    {
        int ch, n = 0;

            while ((ch = (unsigned char)*s++) != '\0')
            {
                if (ch < startCh || ch > endCh)
                    n += bf2__space;
                else
                    n += *OffsTable [ch - startCh];
            }

            return n;
    }

    /* Puts a character on the given position (returns width of c). */
    int bf2__putc (int x, int y, int color, int c)
    {
        byte_t *data, width;
        int i, j;

            c = (c + 256) & 255;

            if (c < startCh || c > endCh)
                return bf2__space;

            data = OffsTable [c - startCh];
            width = *data++;

            c = *(*(unsigned **)&data)++;

            while (c--)
            {
                i = *data++;
                j = *data++;

                putpixel (x + i, y + j, color);
            }

            return width;
    }

    /* Puts a character within the box. */
    static int bf2__putcb (int x, int y, int color, int c, int x1, int y1, int x2, int y2)
    {
        byte_t *data, width;
        int i, j;

            c = (c + 256) & 255;

            if (c < startCh || c > endCh)
                return bf2__space;

            data = OffsTable [c - startCh];
            width = *data++;

            c = *(*(unsigned **)&data)++;

            while (c--)
            {
                i = x + *data++;
                j = y + *data++;

                if (x1 <= i && i <= x2 && y1 <= j && j <= y2)
                    putpixel (i, j, color);
            }

            return width;
    }

    /* Returns the width of the character cell of n. */
    int bf2__width (int n)
    {
            n = (n + 256) & 255;

            if (n < startCh || n > endCh)
                return bf2__space;

            return *OffsTable [n - startCh];
    }

    /* Returns the height of the character cell. */
    int bf2__height (void)
    {
            return charHeight;
    }

    /* Prints a string on the given position. */
    void bf2__printf (int x, int y, int color, char *s, ...)
    {
        va_list p;
        int n;

            if (s == NULL) return;

            va_start (p, s);

            vsprintf (g__tempBuf, s, p);

            n = strlen (s = g__tempBuf);

            while (n--) x += bf2__putc (x, y, color, *s++);
    }

    /* Prints a string within the given box. */
    void bf2__print (int x, int y, int color, char *s, int x1, int y1, int x2, int y2)
    {
        va_list p;
        int n;

            if (s == NULL) return;

            va_start (p, s);

            vsprintf (g__tempBuf, s, p);

            n = strlen (s = g__tempBuf);

            while (n-- && x < x2)
                x += bf2__putcb (x, y, color, *s++, x1, y1, x2, y2);
    }
