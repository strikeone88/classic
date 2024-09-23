/*
    GFRAC.H

    Generic Fraction Manipulation Library Version 0.01

    Copyright (C) 2008 RedStar Technologies
    Written by J. Palencia (ilina@bloodykisses.zzn.com)
*/

#ifndef __GFRAC_H
#define __GFRAC_H

    #ifndef __GINT_H
    #include <gint.h>
    #endif

    /* Upper 8-bits of Flags (flags are 16-bits wide). */
    #define AsFraction  0x8000
    #define TrimZeroes  0x4000

    /* Generic Fraction Structure. */
    typedef struct { gInt Numer, Denom; } gFrac;

    /* Creates a new bCount-byte fraction. */
    gFrac gFracNew (int bCount);

    /* Deletes a fraction. */
    void gFracDelete (gFrac P);

    /* Converts a two long integers into a bCount-byte fraction. */
    gFrac gFracConv (unsigned long Numer, unsigned long Denom, int bCount);

    /* Clonates the given gFrac. */
    gFrac gFracClone (gFrac A);

    /* Copies the given S into D. */
    gFrac gFracCopy (gFrac D, gFrac S);

    /* Copies the given long integers into D. */
    gFrac gFracCopyL (gFrac D, unsigned long Numer, unsigned long Denom);

    /* Returns the lower 32-bits of the given gFrac. */
    long gFracGetLong (gFrac A, int Flags);

    /* Returns the result of -A. */
    gFrac gFracNeg (gFrac A, int Flags);

    /* Returns TRUE if A is valid. */
    int gFracValid (gFrac A);

    /* Returns TRUE if A is zero. */
    int gFracZero (gFrac A);

    /* Returns TRUE if A is negative. */
    int gFracNegative (gFrac A);

    /* Returns the absolute value of A. */
    gFrac gFracAbs (gFrac A, int Flags);

    /* Returns the result of A + B. */
    gFrac gFracAdd (gFrac A, gFrac B, int Flags);

    /* Returns the result of A - B. */
    gFrac gFracSub (gFrac A, gFrac B, int Flags);

    /* Returns the result of A (C) B. */
    int gFracComp (gFrac A, gFrac B, int Flags);

    /* Returns the result of A * B. */
    gFrac gFracMul (gFrac A, gFrac B, int Flags);

    /* Returns the result of A / B. */
    gFrac gFracDiv (gFrac A, gFrac B, int Flags);

    /* Returns the result of A % B. */
    gFrac gFracMod (gFrac A, gFrac B, int Flags);

    /* Converts the fraction to an integer. */
    gInt gFracInt (gFrac A, int Flags);

    /* Converts a string into a bCount-byte fraction. */
    gFrac gFracAtof (const char *s, int r, int bCount);

    /* Converts a fraction into a string. */
    char *gFracFtoa (char *d, gFrac A, int Prec, int Flags);

#endif
