/*
    BF2.C

    RedStar Binary Font 2.0 Engine Version 0.03

    Copyright (C) 2007-2008 Novak Laboratories
    Written by J. Palencia (ilina@bloodykisses.zzn.com)
*/

#ifndef __BF2_H
#define __BF2_H

    /* Sets the internal font variables given BF2 font data, returns
       non-zero if the font was not set. */

    int bf2__set (char *p, unsigned len);

    /* Returns the length in pixels of the string. */
    int bf2__strlen (char *s);

    /* Puts a character on the given position (returns width of c). */
    int bf2__putc (int x, int y, int color, int c);

    /* Returns the width of the character cell of n. */
    int bf2__width (int n);

    /* Returns the height of the character cell. */
    int bf2__height (void);

    /* Prints a string on the given position. */
    void bf2__printf (int x, int y, int color, char *s, ...);

    /* Length of an space in pixels. */
    extern int bf2__space;

#endif
