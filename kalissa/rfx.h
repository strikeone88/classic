/*
    RFX.C

    RedStar Fast Fixed Font Support Version 0.2a

    NOTE: This source file was slightly modified (a couple of lines)
          to work ONLY with 16-bit modes.

    NOTE2: Modified for Kalissa, write-only mode.

    Copyright (C) 2007 RedStar Technologies
    Written by J. Palencia (zipox@ureach.com)
*/

#ifndef __RFX
#define __RFX

    #ifndef __DEFS_H__
    #include "defs.h"
    #endif

    /* Sets the internal font to the given one. */
    int set_font (unsigned char *r);

    /* Puts a character on the wanted position. */
    void rputc (int x, int y, int bc);

    /* Prints a string on the given position. */
    void rprintf (int x, int y, char *s, ...);

    /* Sets the rfx_color. */
    void rtextattr (color_t fg);

    /* Returns the length in pixels of the string. */
    int rlength (char *s);

    /* Current line width (for centering purposes). */
    extern int rfx_width;

#endif
