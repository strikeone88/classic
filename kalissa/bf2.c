/*
    BF2.C

    RedStar Binary Font Version 2.2 (Modified for Write-Only)

    2.1: DEF_SPACE now is a public variable.
    2.2: BF3 Now supported.

    Copyright (C) 2007-2009 Novak Laboratories & RedStar Technologies
    Written by J. Palencia (ilina@bloodykisses.zzn.com)
*/

    #include <string.h>
    #include <stdarg.h>
    #include <stdio.h>

    #include "bf2.h"

    /* Definition of a byte (unsigned 8-bit integer). */
    #define byte_t      unsigned char

    /* This is the BF2 signature. */
    #define SIGNATURE   0x21324642L

    /* This is the BF3 signature. */
    #define SIGNATURE2  0x33666272L

    /* Default space size. */
    int DEF_SPACE = 5;

    /* This is the character offsets table. */
    static byte_t *TABLE [256];

    /* These will store some values read from the BF2. */
    static int ST_CHAR, EN_CHAR, CC_H;

    /* Current foreground. */
    long bf2_color;

    /* Current line width (for centering purposes). */
    int bf2_width;

    /* This is the global text buffer. */
    extern char text_buf [256];

    int CurIsBf3;

    /* Sets the bf2_color. */
    void btextattr (color_t fg)
    {
            bf2_color = fg;
    }

    /* Changes the internal character offset table for the given font. */
    int bf2__setfont (char *p, unsigned len)
    {
        int i = 0, c;
        char *q = p;

            if (*((unsigned long *)p) != SIGNATURE)
            {
                if (*((unsigned long *)p) != SIGNATURE2)
                    return 1;

                ST_CHAR = *((byte_t *)(p + 0x04));
                EN_CHAR = *((byte_t *)(p + 0x05));
                CC_H = *((byte_t *)(p + 0x06));

                c = EN_CHAR - ST_CHAR + 1;
                p += 7;

                while (c--) TABLE [i++] = q + *(*(unsigned long **)&p)++;

                CurIsBf3 = 1;
            }
            else
            {
                ST_CHAR = *((byte_t *)(p + 0x04));
                EN_CHAR = *((byte_t *)(p + 0x05));
                CC_H = *((byte_t *)(p + 0x06));

                p = p + len - ((c = (EN_CHAR - ST_CHAR + 1)) << 1);

                while (c--) TABLE [i++] = q + *(*(unsigned **)&p)++;

                CurIsBf3 = 0;
            }

            return 0;
    }

    /* Returns the length in pixels of a string. */
    static int strsize (char *s)
    {
        int ch, size = 0;

            while ((ch = *s++) != '\0')
            {
                if (ch < ST_CHAR || ch > EN_CHAR)
                    size += DEF_SPACE;
                else
                    size += *TABLE [ch - ST_CHAR];
            }

            return size;
    }


    /* Puts a character on the given position. */
    int bputc (int x, int y, int ch)
    {
        byte_t *d, tx, ty, CC_W;
        long clr = bf2_color;

            if (ch < ST_CHAR || ch > EN_CHAR)
                return DEF_SPACE;

            d = TABLE [ch - ST_CHAR];
            CC_W = *d++;

            if (CurIsBf3) d++;

            ch = *(*(unsigned **)&d)++;

            while (ch--)
            {
                tx = *d++;
                ty = *d++;

                putpixel (x + tx, y + ty, clr);
            }

            return CC_W;
    }

    /* Returns the width of the character cell of ch. */
    int gCC_W (int ch)
    {
            if (ch < ST_CHAR || ch > EN_CHAR)
                return DEF_SPACE;

            return *TABLE [ch - ST_CHAR];
    }

    /* Returns the width of the character cell. */
    int gCC_w (void)
    {
        int ch, t = 0;

            for (ch = ST_CHAR; ch <= EN_CHAR; ch++)
                t += *TABLE [ch - ST_CHAR];

            return t / (EN_CHAR - ST_CHAR + 1);
    }

    /* Returns the height of the character cell. */
    int gCC_h (void)
    {
            return CC_H;
    }

    /* Prints a string on the given position. */
    void bprintf (int x, int y, char *s, ...)
    {
        va_list p; int n;

            if (s == NULL) return;

            va_start (p, s);

            vsprintf (text_buf, s, p);

            n = strlen (s = text_buf);

            if (x < 0) x = -x - 1 + (bf2_width - strsize (s)) / 2;

            while (n--) x += bputc (x, y, (unsigned char)*s++);
    }

    /* Returns the length in pixels of the string. */
    int blength (char *s)
    {
            return strsize (s);
    }
