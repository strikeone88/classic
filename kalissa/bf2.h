/*
    BF2.C

    RedStar Binary Font Version 2.0 (Modified for Write-Only)

    Copyright (C) 2007 Novak Laboratories & RedStar Technologies
    Written by J. Palencia (zipox@ureach.com)
*/

#ifndef __BF2
#define __BF2

    #ifndef __DEFS_H__
    #include "defs.h"
    #endif

    /* Changes the internal character offset table for the given font. */
    int bf2__setfont (char *p, unsigned len);

    /* Puts a character on the given position. */
    int bputc (int x, int y, int ch);

    /* Prints a string on the given position. */
    void bprintf (int x, int y, char *s, ...);

    /* Returns the width of the character cell of ch. */
    int gCC_W (int ch);

    /* Returns the width of the character cell. */
    int gCC_w (void);

    /* Returns the height of the character cell. */
    int gCC_h (void);

    /* Returns the length in pixels of the string. */
    int blength (char *s);

    /* Sets the bf2_color. */
    void btextattr (color_t fg);

    /* Current line width (for centering purposes). */
    extern int bf2_width;

    /* Default space size. */
    extern int DEF_SPACE;

#endif
