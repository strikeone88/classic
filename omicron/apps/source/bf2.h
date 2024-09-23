/*
    BF2.C

    RedStar Binary Font 2.0 Engine Version 0.03

    Copyright (C) 2007-2008 Novak Laboratories
    Written by J. Palencia (ilina@bloodykisses.zzn.com)
*/

#ifndef __BF2_H
#define __BF2_H

    #ifndef __GFX_H
    #include <gfx.h>
    #endif

    /* BF2 Commands. */
    enum BCommand
    {
        BCreateContext, BDestroyContext, BSelectGfx, BSetFont, BSetColor,
        BGetColor, BEnableViewPort, BDisableViewPort, BSetSpaceLen,
        BGetSpaceLen, BWidth, BHeight, BStringWidth, BPutC, BPrintF
    };

#endif
