/*
    TRIG.H

    Precalculated and optimized cosine and sine tables.

    Copyright (C) 2008 RedStar Technologies
    Written by J. Palencia (ilina@bloodykisses.zzn.com)
*/

#ifndef __TRIG_H
#define __TRIG_H

    /* Converts from degrees to base 256 */
    #define convdeg(t) (int)((((long)(t) << 8) / 360) & 255)

    /* Adjust the given base-256 angle to fix in 8-bits. */
    #define adjval(p) ((p) & 255)

    /* Returns cosine of t multiplied by r. */
    #define rcost(r,t) (int)((((long)(r)) * rcos [(t) & 255]) >> 14)

    /* Returns sine of t multiplied by r. */
    #define rsint(r,t) (int)((((long)(r)) * rsin [(t) & 255]) >> 14)

    /* Returns atn of t multiplied by r. */
    #define ratnt(r,t) (int)((((long)(r)) * ratn [(t) & 255]) >> 12)

    /* Returns cosine of t multiplied by r (but scaled). */
    #define rcostS(r,t) (int)(((long)(r)) * rcos [(t) & 255])

    /* Returns sine of t multiplied by r (but scaled). */
    #define rsintS(r,t) (int)(((long)(r)) * rsin [(t) & 255])

    /* Some often-used angles. */
    #define _0deg    0
    #define _30deg   convdeg (30)
    #define _45deg   convdeg (45)
    #define _60deg   convdeg (60)
    #define _90deg   convdeg (90)
    #define _135deg  convdeg (135)
    #define _180deg  convdeg (180)
    #define _225deg  convdeg (225)
    #define _270deg  convdeg (270)
    #define _315deg  convdeg (315)
    #define _360deg  convdeg (360)

    /* Cosine table scaled up 14 bits. */
    extern short int rcos [256];

    /* Sine table scaled up 14 bits. */
    extern short int rsin [256];

    /* Partial ATN table scaled up 12 bits. */
    extern unsigned ratn [256];

#endif
