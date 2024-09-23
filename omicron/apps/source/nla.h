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

    #ifndef __GFX_H
    #include <gfx.h>
    #endif

    /* An NLA animation. */
    typedef struct /* direct cast: linkable_t */
    {
        linkable_t  link;

        int         xres, yres;

        unsigned    fcount, colors;
        int         bits;

        unsigned    char *palette;
        unsigned    char **frames;

        int         uw, w, uh, h;
    }
    anim_t;

    /* NLA Commands. */
    enum NCommand
    {
        NCreate, NCreateEx, NDestroy, NSetViewPort, NSetGfx, NScreenView,
        NDrawFrame, NDrawBuf, NPaletteAND, NDrawFrameP
    };

#endif
