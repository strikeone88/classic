/*
    GFRAC.C

    Generic Fraction Manipulation Library Version 0.01

    Copyright (C) 2008 RedStar Technologies
    Written by J. Palencia (ilina@bloodykisses.zzn.com)
*/

    #include <string.h>
    #include <gfrac.h>

    /* Creates a new bCount-byte fraction. */
    gFrac gFracNew (int bCount)
    {
        gFrac Result;

            Result.Denom = gIntConv (1, bCount);
            Result.Numer = gIntNew (bCount);

            return Result;
    }

    /* Deletes a fraction. */
    void gFracDelete (gFrac P)
    {
            gIntDelete (P.Numer);
            gIntDelete (P.Denom);
    }

    /* Converts a two long integers into a bCount-byte fraction. */
    gFrac gFracConv (unsigned long Numer, unsigned long Denom, int bCount)
    {
        gFrac Result = gFracNew (bCount);

            gIntCopyL (Result.Numer, Numer);
            gIntCopyL (Result.Denom, Denom);

            return Result;
    }

    /* Clonates the given gFrac. */
    gFrac gFracClone (gFrac A)
    {
        gFrac Result;

            Result.Numer = gIntClone (A.Numer);
            Result.Denom = gIntClone (A.Denom);

            return Result;
    }

    /* Copies the given S into D. */
    gFrac gFracCopy (gFrac D, gFrac S)
    {
            gIntCopy (D.Numer, S.Numer);
            gIntCopy (D.Denom, S.Denom);

            return D;
    }

    /* Copies the given long integers into D. */
    gFrac gFracCopyL (gFrac D, unsigned long Numer, unsigned long Denom)
    {
            gIntCopyL (D.Numer, Numer);
            gIntCopyL (D.Denom, Denom);

            return D;
    }

    /* Returns the lower 32-bits of the given gFrac. */
    long gFracGetLong (gFrac A, int Flags)
    {
        long Result;

            Result = gIntGetLong (gIntDiv (A.Numer, A.Denom, SignedDiv),
                                  DeleteA);

            if (Flags & DeleteA) gFracDelete (A);

            return Result;
    }

    /* Returns the result of -A. */
    gFrac gFracNeg (gFrac A, int Flags)
    {
        gFrac Result = gFracNew (A.Numer.ByteCount);

            Result.Numer = gIntNeg (A.Numer, 0);
            Result.Denom = gIntClone (A.Denom);

            if (Flags & DeleteA) gFracDelete (A);
            return Result;
    }

    /* Returns TRUE if A is valid. */
    int gFracValid (gFrac A)
    {
            if (gIntZero (A.Denom))
                return 0;

            return gIntValid (A.Numer) && gIntValid (A.Denom);
    }

    /* Returns TRUE if A is zero. */
    int gFracZero (gFrac A)
    {
            return gIntZero (A.Numer);
    }

    /* Returns TRUE if A is negative. */
    int gFracNegative (gFrac A)
    {
            return gIntNegative (A.Numer) != gIntNegative (A.Denom);
    }

    /* Returns the absolute value of A. */
    gFrac gFracAbs (gFrac A, int Flags)
    {
        gFrac Result;

            if (gFracNegative (A))
                Result = gFracNeg (A, 0);
            else
                Result = gFracClone (A);

            if (Flags & DeleteA) gFracDelete (A);
            return Result;
    }

    /* Returns the result of A + B. */
    gFrac gFracAdd (gFrac A, gFrac B, int Flags)
    {
        gFrac Result;

            Result.Numer = gIntAdd (gIntMul (A.Numer, B.Denom, SignedMul),
                                    gIntMul (B.Numer, A.Denom, SignedMul),
                                    DeleteBoth);

            Result.Denom = gIntMul (A.Denom, B.Denom, SignedMul);

            if (Flags & DeleteA) gFracDelete (A);
            if (Flags & DeleteB) gFracDelete (B);

            return Result;
    }

    /* Returns the result of A - B. */
    gFrac gFracSub (gFrac A, gFrac B, int Flags)
    {
        gFrac Result;

            Result.Numer = gIntSub (gIntMul (A.Numer, B.Denom, SignedMul),
                                    gIntMul (B.Numer, A.Denom, SignedMul),
                                    DeleteBoth);

            Result.Denom = gIntMul (A.Denom, B.Denom, SignedMul);

            if (Flags & DeleteA) gFracDelete (A);
            if (Flags & DeleteB) gFracDelete (B);

            return Result;
    }

    /* Returns the result of A (C) B. */
    int gFracComp (gFrac A, gFrac B, int Flags)
    {
        gInt TempA, TempB;
        int Result;

            TempA = gIntMul (A.Numer, B.Denom, SignedMul);
            TempB = gIntMul (A.Denom, B.Numer, SignedMul);

            Result = gIntComp (TempA, TempB, Flags | DeleteBoth);

            if (Flags & DeleteA) gFracDelete (A);
            if (Flags & DeleteB) gFracDelete (B);

            return Result;
    }

    /* Returns the result of A * B. */
    gFrac gFracMul (gFrac A, gFrac B, int Flags)
    {
        gFrac Result;

            Result.Numer = gIntMul (A.Numer, B.Numer, SignedMul);
            Result.Denom = gIntMul (A.Denom, B.Denom, SignedMul);

            if (Flags & DeleteA) gFracDelete (A);
            if (Flags & DeleteB) gFracDelete (B);

            return Result;
    }

    /* Returns the result of A / B. */
    gFrac gFracDiv (gFrac A, gFrac B, int Flags)
    {
        gFrac Result;

            Result.Numer = gIntMul (A.Numer, B.Denom, SignedMul);
            Result.Denom = gIntMul (A.Denom, B.Numer, SignedMul);

            if (Flags & DeleteA) gFracDelete (A);
            if (Flags & DeleteB) gFracDelete (B);

            return Result;
    }

    /* Returns the result of A % B. */
    gFrac gFracMod (gFrac A, gFrac B, int Flags)
    {
        gInt TempA, TempB;
        gFrac Result;

            TempA = gIntDiv (A.Numer, A.Denom, SignedDiv);
            TempB = gIntDiv (B.Numer, B.Denom, SignedDiv);

            Result.Numer = gIntMod (TempA, TempB, SignedDiv + DeleteBoth);
            Result.Denom = gIntConv (1, Result.Numer.ByteCount);

            if (Flags & DeleteA) gFracDelete (A);
            if (Flags & DeleteB) gFracDelete (B);

            return Result;
    }

    /* Converts the fraction to an integer. */
    gInt gFracInt (gFrac A, int Flags)
    {
        gInt Result;

            Result = gIntDiv (A.Numer, A.Denom, 0);

            if (Flags & DeleteA) gFracDelete (A);

            return Result;
    }

    /* Converts a string into a bCount-byte fraction. */
    gFrac gFracAtof (const char *s, int r, int bCount)
    {
        gFrac Result; gInt RadixValue;
        char *p; int i;

            if (!s) return gFracNew (bCount);

            if ((p = strchr (s, '.')) != NULL) *p = '_';

            Result.Numer = gIntAtoi (s, r, bCount);
            Result.Denom = gIntConv (1, bCount);

            if (!p) return Result;

            RadixValue = gIntConv (RadixTable [r & 3], bCount);

            *p++ = '.';

            while ((i = *p++) != '\0')
            {
                if ((0x30 > i || i > 0x39) && (0x41 > i || i > 0x46) &&
                    (0x61 > i || i > 0x66)) continue;

                Result.Denom = gIntMul (Result.Denom, RadixValue, DeleteA);
            }

            gIntDelete (RadixValue);
            return Result;
    }

    /* Converts a fraction into a string. */
    char *gFracFtoa (char *d, gFrac A, int Prec, int Flags)
    {
        gInt Quo, Rem, Ten; gFrac Q;
        char *s, *p = d;
        int i, n;

            if (!d) return NULL;

            Q = gFracClone (A);

            if (gIntNegative (Q.Numer) != gIntNegative (Q.Denom))
                *p++ = '-';

            Q.Numer = gIntAbs (Q.Numer, DeleteA);
            Q.Denom = gIntAbs (Q.Denom, DeleteA);

            if (Flags & AsFraction)
            {
                gIntItoa (p, Q.Numer, 1, SignedConv);
                *(p += strlen (p)) = '/';
                gIntItoa (++p, Q.Denom, 1, SignedConv);

                gFracDelete (Q);
                return d;
            }

            gIntDivMod (Q.Numer, Q.Denom, &Quo, &Rem, SignedDiv);

            gIntItoa (p, Quo, 1, SignedConv);
            *(p += strlen (p)) = '.';
            gIntDelete (Quo);

            if (!Prec) Prec = 8;

            Ten = gIntConv (10, Q.Numer.ByteCount);
            s = ++p;

            while (Prec--)
            {
                Rem = gIntMul (Rem, Ten, DeleteA);

                gIntDivMod (Rem, Q.Denom, &Quo, &Rem, SignedDiv + DeleteA);

                *p++ = gIntGetLong (Quo, DeleteA) + 0x30;
            }

            *p = '\0';

            if (Flags & TrimZeroes)
            {
                s += (i = strlen (s));
                while (i--) if (*--s != 0x30) { s++; break; }
                if (*s == 0x30) *s-- = '\0';
                if (*s == '.') *s = '\0';
            }

            gIntDelete (Rem);
            gIntDelete (Ten);

            gFracDelete (Q);
            return d;
    }
