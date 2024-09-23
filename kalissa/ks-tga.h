/*
    KS-TGA.H

    Kalissa TGA Writer Version 0.03 -- Interface File

    Copyright (C) 2007 RedStar Technologies
    Written by J. Palencia (zipox@ureach.com)
*/

#ifndef __KS_TGA_H
#define __KS_TGA_H

    #include <stdio.h>

    #ifndef __XMSH_H
    #include "xmsh.h"
    #endif

    /* Supported output formats. */
    #define F__TGA      0
    #define F__BMP      1

    /* Targa Surface Specification */
    typedef struct
    {
        long        height;
        long        width;

        unsigned    surface_id;
        unsigned    long size;

        int         powered_by_xms;
        unsigned    handle;

        int         format;
        int         abort;

        moveEMBst_t wr;
        moveEMBst_t rd;

        char        *output;
        FILE        *temp;
    }
    surface_t;

    /* Creates a surface for the writer. */
    const surface_t *w__CreateSurface (unsigned, unsigned, char *, long);

    /* Closes the given surface. */
    void w__CloseSurface (const surface_t *);

    /* Writes a pixel at (x, y) of the surface. */
    void w__WritePixel (const surface_t *surface, int x, int y, long c);

    /* Reads a pixel from (x, y). */
    void w__ReadPixel (const surface_t *surface, int x, int y, int *r,
                       int *g, int *b);

    /* Writes a horizontal line at (x, y). */
    void w__HorzLine (const surface_t *surface, int x, int y, int w, long c);

    /* Writes a vertical line at (x, y). */
    void w__VertLine (const surface_t *surface, int x, int y, int h, long c);

#endif
