/*
    BASE.H

    Base Functions and Definitions

    Copyright (C) 2007-2008 RedStar Technologies
    Written by J. Palencia (ilina@bloodykisses.zzn.com)
*/

#ifndef __BASE_H
#define __BASE_H

    #ifndef __GLINK_H
    #include <glink.h>
    #endif

    /* Macro to make action declaration easier. */
    #define rACTION(x) void x (int form, unsigned n, char *argv [], unsigned ndx [])

    /* Macros that define unsigned types. */
    #define uint_t      unsigned short
    #define ulong_t     unsigned long
    #define uchar_t     unsigned char

#endif
