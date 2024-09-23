/* 
    KGFX.H

    Kelly 3D Graphics Engine Version 0.12 for 640x480x16bpp

    This engine requires an 80386+ processor, VESA VBE 2.0+, at least
    1800 K-bytes of Video-RAM, at least 32 MB of RAM, and it MUST run
    in TRUE real-mode.

    NOTE: The original engine was modified to fit the particular purpose
          of this project, all the 3D-related functions are gone, also
          a pixel-blending function was added (putpixelBlend).

    Copyright (C) 2007-2008 RedStar Technologies
    Written by J. Palencia (ilina@bloodykisses.zzn.com)
*/

#ifndef __GFX_H
#define __GFX_H

    /* Packs the given 8:8:8 into a 5:6:5 integer. */
    #define packrgb(r,g,b) (((((r)&255)>>3)<<11)|((((g)&255)>>2)<<5)|(((b)&255)>>3))

    /* Unpacks the given 5:6:5 integer into an 8:8:8. */
    #define unpackrgb(pack,r,g,b) r=(pack>>11)<<3,g=((pack>>5)&0x3F)<<2,b=(pack&0x1F)<<3

    /* Screen resolution (DO NOT CHANGE!!!). */
    #define s__width    640
    #define s__height   480

    /* Error codes for the graphics engine. */
    enum KGFX_ERR
    {
        KGFX_CANT_SET   = 0x01, /* Can't set the video mode. */
        KGFX_CANT_ENTER = 0x02, /* Unable to enter unreal mode. */
        KGFX_NO_VBE     = 0x03, /* VESA VBE not found. */
        KGFX_NO_MEM     = 0x04, /* Not Enough Memory. */
    };

    /* Initializes the graphics engine, returns KGFX_ERR. */
    int startGfx (void);

    /* Basically returns to text mode. */
    void stopGfx (void);

    /* Puts a pixel on the screen (pixel is in 5:6:5 format). */
    void putpixel (int, int, int);

    /* Blends a pixel on the screen (pixel is in 5:6:5 format). */
    void putpixelBlend (int, int, int);

    /* Returns the pixel located at (x, y). */
    int getpixel (int, int);

    /* Changes the visible page. */
    void vpage (int);

    /* Changes the active page. */
    void apage (int);

    /* Clears the given page (1st parm) with the given color. */
    void clearpage (int, int);

#endif
