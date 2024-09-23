/*
    GINT.H

    Generic Integer Manipulation Library Version 0.01

    Copyright (C) 2008 RedStar Technologies
    Written by J. Palencia (ilina@bloodykisses.zzn.com)
*/

#ifndef __GINT_H
#define __GINT_H

    #ifndef __GLINK_H
    #include <glink.h>
    #endif

    /* Definition to make things clearer. */
    #define private static

    /* Upper 8-bits of Flags (flags are 16-bits wide). */
    #define DeleteBoth      (DeleteA + DeleteB)
    #define GetHiDelta      0x800
    #define SignedComp      0x400
    #define SignedDiv       0x400
    #define SignedMul       0x400
    #define SignedConv      0x400
    #define DeleteA         0x200
    #define DeleteB         0x100

    /* Generic Integer Structure. */
    typedef struct
    {
        unsigned char ByteCount;
        unsigned char *Value;
    }
    gInt;

    /* Condition Codes (lower 8-bits of Flags). */
    enum CondCode { cL, cLE, cG, cGE, cB, cBE, cA, cAE, cE, cNE };

    /* Creates a new bCount-byte integer. */
    gInt gIntNew (int bCount);

    /* Deletes an integer. */
    void gIntDelete (gInt P);

    /* Converts a long integer into a bCount-byte integer. */
    gInt gIntConv (unsigned long Value, int bCount);

    /* Clonates the given gInt. */
    gInt gIntClone (gInt A);

    /* Copies the given S into D. */
    gInt gIntCopy (gInt D, gInt S);

    /* Copies the given long integer into D. */
    gInt gIntCopyL (gInt D, unsigned long Value);

    /* Returns the lower 32-bits of the given gInt. */
    long gIntGetLong (gInt A, int Flags);

    /* Returns the result of ~A. */
    gInt gIntNot (gInt A, int Flags);

    /* Returns the result of -A. */
    gInt gIntNeg (gInt A, int Flags);

    /* Returns TRUE if A is valid. */
    int gIntValid (gInt A);

    /* Returns TRUE if A is zero. */
    int gIntZero (gInt A);

    /* Returns TRUE if A is negative. */
    int gIntNegative (gInt A);

    /* Returns the absolute value of A. */
    gInt gIntAbs (gInt A, int Flags);

    /* Returns the result of A & B. */
    gInt gIntAnd (gInt A, gInt B, int Flags);

    /* Returns the result of A | B. */
    gInt gIntOr (gInt A, gInt B, int Flags);

    /* Returns the result of A ^ B. */
    gInt gIntXor (gInt A, gInt B, int Flags);

    /* Returns the result of A + B. */
    gInt gIntAdd (gInt A, gInt B, int Flags);

    /* Returns the result of A - B. */
    gInt gIntSub (gInt A, gInt B, int Flags);

    /* Returns the result of A << B. */
    gInt gIntShl (gInt A, gInt B, int Flags);

    /* Returns the result of A >> B. */
    gInt gIntShr (gInt A, gInt B, int Flags);

    /* Returns the result of A <<< B. */
    gInt gIntRol (gInt A, gInt B, int Flags);

    /* Returns the result of A >>> B. */
    gInt gIntRor (gInt A, gInt B, int Flags);

    /* Returns the result of A (C) B. */
    int gIntComp (gInt A, gInt B, int Flags);

    /* Returns the integer result of Log2(A). */
    int gIntLog2 (gInt A, int Flags);

    /* Returns the result of A * B. */
    gInt gIntMul (gInt A, gInt B, int Flags);

    /* Returns the result of A / B and A % B. */
    void gIntDivMod (gInt A, gInt B, gInt *Quo, gInt *Rem, int Flags);

    /* Returns the result of A / B. */
    gInt gIntDiv (gInt A, gInt B, int Flags);

    /* Returns the result of A % B. */
    gInt gIntMod (gInt A, gInt B, int Flags);

    /* Converts a string into a bCount-byte integer. */
    gInt gIntAtoi (const char *s, int r, int bCount);

    /* Converts an integer into a string. */
    char *gIntItoa (char *d, gInt A, int radix, int Flags);

    /* Radix Table. */
    extern unsigned char RadixTable [];

    /* Digits Array. */
    extern unsigned char *Digits;

#endif
