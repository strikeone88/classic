/*
    NLA.H

    Novak Labs. Animation Engine Version 0.01

    This engine will load and show 5:6:5 NLA files.

    Copyright (C) 2008 Novak Laboratories
    Written by J. Palencia (ilina@bloodykisses.zzn.com)
*/

#ifndef __NLA_H
#define __NLA_H

    #ifndef __GLINK_H
    #include <glink.h>
    #endif

    /* An NLA animation. */
    typedef struct /* direct cast: linkable_t */
    {
        linkable_t  link;

        int         xres, yres;

        unsigned    fcount, pallen;
        int         bits;

        unsigned    *palette;
        char        **frames;
    }
    anim_t;

    /* Loads an NLA animation and returns its structure. */
    anim_t *nla__load (char *fname);

    /* Destroys a previously loaded NLA. */
    void nla__destroy (anim_t *x);

    /* Draws a frame and uses the coordinates as the center. */
    void nla__drawframe (int x, int y, anim_t *q, unsigned frame,
                         int x1, int y1, int x2, int y2);

    /* Clones an NLA animation and dups the palette but not the data. */
    anim_t *nla__clone (anim_t *x);

    /* Modifies the palette of the given NLA (using bitwise AND) . */
    void nla__paletteAND (anim_t *x, int aR, int aG, int aB);

#endif
