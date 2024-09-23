/*
    GDEV.H

    Graphics Device Controller Interface Definitions

    Copyright (C) 2007-2008 RedStar Technologies
    Written by J. Palencia (ilina@bloodykisses.zzn.com)
*/

#ifndef __GDEV_H
#define __GDEV_H

    #ifndef __IFM_H
    #include <ifm.h>
    #endif

    /* GDev Commands. */
    enum GCommand
    {
        GStartGfx, GStopGfx, GAllocateBuffer, GReleaseBuffer, GAllocatePage,
        GReleasePage, GScreenWidth, GScreenHeight, GPackBuffer, GShowPage,
        GSyncShowPage, GClearBuffer, GClearPage
    };

#endif
