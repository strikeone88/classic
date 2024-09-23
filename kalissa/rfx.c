/*
    RFX.C

    RedStar Fast Fixed Font Support Version 0.2a

    NOTE: This source file was slightly modified (a couple of lines)
          to work ONLY with 16-bit modes.

    NOTE2: Modified for Kalissa, write-only mode.

    Copyright (C) 2007 RedStar Technologies
    Written by J. Palencia (zipox@ureach.com)
*/

    #include <stdarg.h>
    #include <string.h>
    #include <stdio.h>
    #include <conio.h>

    #include "rfx.h"

    /* RFX Signature. */
    #define RFX_SIGN    0x46584652UL

    /* Base pointer. */
    static unsigned char *base_p = NULL;

    /* Offsets table. */
    static unsigned int *offsets;

    /* Current foreground. */
    color_t rfx_color;

    /* Current line width (for centering purposes). */
    int rfx_width;

    /* This is the global text buffer. */
    extern char text_buf [256];

    /* External function to plot pixels. */
    void putpixel (int, int, color_t);

    /* Sets the rfx_color. */
    void rtextattr (color_t fg)
    {
            rfx_color = fg;
    }

    /* Checks the RFX header, returns pointer to offsets table. */
    static unsigned char *check_header (unsigned char *s)
    {
            if (*(*(unsigned long **)&s)++ != RFX_SIGN)
                return NULL;

            s++;

            if (*(*(unsigned int **)&s)++ != 256)
                return NULL;

            return s + *(int *)s + 2;
    }

    /* Sets the internal font to the given one. */
    int set_font (unsigned char *r)
    {
            if ((r = check_header (base_p = r)) == NULL)
            {
                base_p = NULL;
                return -1;
            }

            offsets = (unsigned int *)r;

            return 0;
    }

    /* Puts a character on the given position. */
    void rputc (int x, int y, int ch)
    {
        unsigned char q, *p;
        int c = rfx_color;

            if (ch < 0 || ch > 255 || base_p == NULL)
                return;

            if (ch == 0x20) return;

            p = offsets [ch] + base_p;

            ch = *(*(unsigned int **)&p)++;

            while (ch--)
            {
                q = *p++;

                putpixel (x + (q >> 4), y + (q & 0x0F), c);
            }
    }

    /* Prints a string on the given position. */
    void rprintf (int x, int y, char *s, ...)
    {
        va_list p; int n;

            if (s == NULL) return;

            va_start (p, s);

            vsprintf (text_buf, s, p);

            n = strlen (s = text_buf);

            if (x < 0) x = -x - 1 + (rfx_width - (n << 3)) / 2;

            while (n--)
            {
                if (*s == '\b')
                {
                    x -= 8;
                    s++;
                }
                else
                {
                    rputc (x, y, (unsigned char)*s++);
                    x += 8;
                }
            }
    }

    /* Returns the length in pixels of the string. */
    int rlength (char *s)
    {
            return strlen (s) * 8;
    }
