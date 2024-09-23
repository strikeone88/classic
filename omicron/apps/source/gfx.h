/*
    GFX.H

    Generic Graphics Engine Version 0.01

    Copyright (C) 2007-2008 RedStar Technologies
    Written by J. Palencia (ilina@bloodykisses.zzn.com)
*/

#ifndef __GFX_H
#define __GFX_H

    #ifndef __IFM_H
    #include <ifm.h>
    #endif

    #ifndef __GDEV_H
    #include <gdev.h>
    #endif

    /* Gfx Commands. */
    enum GfxCommand
    {
        GSetDevice, GGetDevice, GPutPixel, GPutPixelP, GGetPixel, GGetPixelP,
        GAddPixel, GAddPixelP, GSubPixel, GSubPixelP, GGetBuffer, GGetPage,
        GShow, GSyncShow, GClearScreen, GClear, GAddBuffer, GAddBufferF,
        GPutBuffer
    };

#endif
