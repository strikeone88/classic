/*
    GINT.H

    Generic Integer Manipulation Library Version 0.01

    Copyright (C) 2008 RedStar Technologies
    Written by J. Palencia (ilina@bloodykisses.zzn.com)
*/

    #include <stdio.h>
    #include <glink.h>
    #include <gint.h>

    /* Radix Table. */
    unsigned char RadixTable [] = { 16, 10, 8, 2 };

    /* Digits Array. */
    unsigned char *Digits = "0123456789abcdef";

    /* Creates a new bCount-byte integer. */
    gInt gIntNew (int bCount)
    {
        gInt Result;

            if (bCount)
                Result.Value = alloc (Result.ByteCount = bCount);
            else
                Result.Value = NULL;

            return Result;
    }

    /* Deletes an integer. */
    void gIntDelete (gInt P)
    {
            delete (P.Value);
    }

    /* Converts a long integer into a bCount-byte integer. */
    gInt gIntConv (unsigned long Value, int bCount)
    {
        gInt Result = gIntNew (bCount);
        unsigned char *p;
        int i;

            p = (unsigned char *)&Value;

            for (i = 0; i < 4; i++)
                Result.Value [bCount - (i + 1)] = *p++;

            return Result;
    }

    /* Clonates the given gInt. */
    gInt gIntClone (gInt A)
    {
        gInt Result = gIntNew (A.ByteCount);

            memcpy (Result.Value, A.Value, A.ByteCount);
            return Result;
    }

    /* Copies the given S into D. */
    gInt gIntCopy (gInt D, gInt S)
    {
            memcpy (D.Value, S.Value, D.ByteCount);
            return D;
    }

    /* Copies the given long integer into D. */
    gInt gIntCopyL (gInt D, unsigned long Value)
    {
        unsigned char *p;
        int i;

            memset (D.Value, 0, D.ByteCount);
            p = (unsigned char *)&Value;

            for (i = 0; i < 4; i++)
                D.Value [D.ByteCount - (i + 1)] = *p++;

            return D;
    }

    /* Returns the lower 32-bits of the given gInt. */
    long gIntGetLong (gInt A, int Flags)
    {
        long Result; int i;
        unsigned char *p;

            p = (unsigned char *)&Result;

            for (i = 0; i < 4; i++)
                *p++ = A.Value [A.ByteCount - (i + 1)];

            if (Flags & DeleteA) gIntDelete (A);

            return Result;
    }

    /* Returns the result of ~A. */
    gInt gIntNot (gInt A, int Flags)
    {
        gInt Result = gIntNew (A.ByteCount);
        int i;

            for (i = A.ByteCount - 1; i >= 0; i--)
                Result.Value [i] = ~A.Value [i];

            if (Flags & DeleteA) gIntDelete (A);

            return Result;
    }

    /* Returns the result of -A. */
    gInt gIntNeg (gInt A, int Flags)
    {
        gInt Result = gIntNew (A.ByteCount);
        int Cf = 1, i, t;

            for (i = A.ByteCount - 1; i >= 0; i--)
            {
                t = (unsigned char)~A.Value [i] + Cf;
                Cf = t > 255 ? 1 : 0;

                Result.Value [i] = t;
            }

            if (Flags & DeleteA) gIntDelete (A);

            return Result;
    }

    /* Returns TRUE if A is valid. */
    int gIntValid (gInt A)
    {
            return A.Value != NULL;
    }

    /* Returns TRUE if A is zero. */
    int gIntZero (gInt A)
    {
        int i, Result = 0;

            for (i = 0; i < A.ByteCount && !Result; i++)
                Result = A.Value [i];

            return !Result;
    }

    /* Returns TRUE if A is negative. */
    int gIntNegative (gInt A)
    {
            return A.Value [0] & 0x80 ? 1 : 0;
    }

    /* Returns the absolute value of A. */
    gInt gIntAbs (gInt A, int Flags)
    {
        gInt Result;

            if (gIntNegative (A))
                Result = gIntNeg (A, 0);
            else
                Result = gIntClone (A);

            if (Flags & DeleteA) gIntDelete (A);
            return Result;
    }

    /* Returns the result of A & B. */
    gInt gIntAnd (gInt A, gInt B, int Flags)
    {
        gInt Result = gIntNew (A.ByteCount);
        int i;

            for (i = A.ByteCount - 1; i >= 0; i--)
                Result.Value [i] = A.Value [i] & B.Value [i];

            if (Flags & DeleteA) gIntDelete (A);
            if (Flags & DeleteB) gIntDelete (B);

            return Result;
    }

    /* Returns the result of A | B. */
    gInt gIntOr (gInt A, gInt B, int Flags)
    {
        gInt Result = gIntNew (A.ByteCount);
        int i;

            for (i = A.ByteCount - 1; i >= 0; i--)
                Result.Value [i] = A.Value [i] | B.Value [i];

            if (Flags & DeleteA) gIntDelete (A);
            if (Flags & DeleteB) gIntDelete (B);

            return Result;
    }

    /* Returns the result of A ^ B. */
    gInt gIntXor (gInt A, gInt B, int Flags)
    {
        gInt Result = gIntNew (A.ByteCount);
        int i;

            for (i = A.ByteCount - 1; i >= 0; i--)
                Result.Value [i] = A.Value [i] ^ B.Value [i];

            if (Flags & DeleteA) gIntDelete (A);
            if (Flags & DeleteB) gIntDelete (B);

            return Result;
    }

    /* Returns the result of A + B. */
    gInt gIntAdd (gInt A, gInt B, int Flags)
    {
        gInt Result = gIntNew (A.ByteCount);
        int Cf = 0, i, t;

            for (i = A.ByteCount - 1; i >= 0; i--)
            {
                t = A.Value [i] + B.Value [i] + Cf;
                Cf = t > 255 ? 1 : 0;

                Result.Value [i] = t;
            }

            if (Flags & DeleteA) gIntDelete (A);
            if (Flags & DeleteB) gIntDelete (B);

            return Result;
    }

    /* Returns the result of A - B. */
    gInt gIntSub (gInt A, gInt B, int Flags)
    {
        gInt Result = gIntNew (A.ByteCount);
        int Bf = 0, i, t;

            for (i = A.ByteCount - 1; i >= 0; i--)
            {
                t = A.Value [i] - B.Value [i] - Bf;
                Bf = t < 0 ? 1 : 0;

                Result.Value [i] = t;
            }

            if (Flags & DeleteA) gIntDelete (A);
            if (Flags & DeleteB) gIntDelete (B);

            return Result;
    }

    /* Returns the result of A << B. */
    gInt gIntShl (gInt A, gInt B, int Flags)
    {
        gInt Result = gIntClone (A);
        int i, c, cl, cr;

            c = B.Value [A.ByteCount - 1] & ((A.ByteCount * 8) - 1);

            while (c)
            {
                cl = c > 7 ? 7 : c;
                cr = 8 - cl;

                for (i = 0; i < A.ByteCount; i++)
                {
                    Result.Value [i] <<= cl;

                    if (cr && i < A.ByteCount - 1)
                        Result.Value [i] += Result.Value [i + 1] >> cr;
                }

                c -= cl;
            }

            if (Flags & DeleteA) gIntDelete (A);
            if (Flags & DeleteB) gIntDelete (B);

            return Result;
    }

    /* Returns the result of A >> B. */
    gInt gIntShr (gInt A, gInt B, int Flags)
    {
        gInt Result = gIntClone (A);
        int i, c, cl, cr;

            c = B.Value [A.ByteCount - 1] & ((A.ByteCount * 8) - 1);

            while (c)
            {
                cr = c > 7 ? 7 : c;
                cl = 8 - cr;

                for (i = A.ByteCount - 1; i >= 0; i--)
                {
                    Result.Value [i] >>= cr;

                    if (cl && i)
                        Result.Value [i] += Result.Value [i - 1] << cl;
                }

                c -= cr;
            }

            if (Flags & DeleteA) gIntDelete (A);
            if (Flags & DeleteB) gIntDelete (B);

            return Result;
    }

    /* Returns the result of A <<< B. */
    gInt gIntRol (gInt A, gInt B, int Flags)
    {
        gInt Result = gIntClone (A);
        int i, c, cl, cr, aux;

            c = B.Value [A.ByteCount - 1] & ((A.ByteCount * 8) - 1);

            while (c)
            {
                cl = c > 7 ? 7 : c;
                cr = 8 - cl;

                aux = Result.Value [0] >> cr;

                for (i = 0; i < A.ByteCount; i++)
                {
                    Result.Value [i] <<= cl;

                    if (i < A.ByteCount - 1)
                        Result.Value [i] += Result.Value [i + 1] >> cr;
                    else
                        Result.Value [i] += aux;
                }

                c -= cl;
            }

            if (Flags & DeleteA) gIntDelete (A);
            if (Flags & DeleteB) gIntDelete (B);

            return Result;
    }

    /* Returns the result of A >>> B. */
    gInt gIntRor (gInt A, gInt B, int Flags)
    {
        gInt Result = gIntClone (A);
        int i, c, cl, cr, aux;

            c = B.Value [A.ByteCount - 1] & ((A.ByteCount * 8) - 1);

            while (c)
            {
                cr = c > 7 ? 7 : c;
                cl = 8 - cr;

                aux = Result.Value [A.ByteCount - 1] << cl;

                for (i = A.ByteCount - 1; i >= 0; i--)
                {
                    Result.Value [i] >>= cr;

                    if (i)
                        Result.Value [i] += Result.Value [i - 1] << cl;
                    else
                        Result.Value [i] += aux;
                }

                c -= cr;
            }

            if (Flags & DeleteA) gIntDelete (A);
            if (Flags & DeleteB) gIntDelete (B);

            return Result;
    }

    /* Returns the result of A (C) B. */
    int gIntComp (gInt A, gInt B, int Flags)
    {
        int hD = 0, Sr, i;

            Sr = 0x80 & ((A.Value [0] & 0x80) + (B.Value [0] & 0x80));

            for (i = 0; i < A.ByteCount && !hD; i++)
                hD = A.Value [i] - B.Value [i];

            if (Flags & GetHiDelta)
            {
                if (Flags & DeleteA) gIntDelete (A);
                if (Flags & DeleteB) gIntDelete (B);

                return ((Flags & SignedComp) && Sr) ? !hD : hD;
            }

            switch (Flags & 0xFF)
            {
                case cL:    hD = hD < 0;    if (Sr) hD = !hD;
                            break;

                case cLE:   hD = hD <= 0;   if (Sr) hD = !hD;
                            break;

                case cG:    hD = hD > 0;    if (Sr) hD = !hD;
                            break;

                case cGE:   hD = hD >= 0;   if (Sr) hD = !hD;
                            break;

                case cB:    hD = hD < 0;
                            break;

                case cBE:   hD = hD <= 0;
                            break;

                case cA:    hD = hD > 0;
                            break;

                case cAE:   hD = hD >= 0;
                            break;

                case cE:    hD = hD == 0;
                            break;

                case cNE:   hD = hD != 0;
                            break;
            }

            if (Flags & DeleteA) gIntDelete (A);
            if (Flags & DeleteB) gIntDelete (B);

            return hD;
    }

    /* Returns the integer result of Log2(A). */
    int gIntLog2 (gInt A, int Flags)
    {
        unsigned int i, j, r;

            for (i = 0, r = A.ByteCount * 8 - 1; i < A.ByteCount; i++)
            {
                for (j = 0x80; j > 0; j >>= 1, r--)
                    if (A.Value [i] & j) return r;
            }

            if (Flags & DeleteA) gIntDelete (A);

            return 0;
    }

    /* Returns the result of A * B. */
    gInt gIntMul (gInt A, gInt B, int Flags)
    {
        gInt P, Q, Result = gIntNew (A.ByteCount);
        int i, j, k, t, r, w, n, hA, hB, tA;

            if (Flags & SignedMul)
            {
                n = gIntNegative (A) != gIntNegative (B);
                P = gIntAbs (A, 0);
                Q = gIntAbs (B, 0);
            }
            else
            {
                P = gIntClone (A);
                Q = gIntClone (B);
            }

            if (Flags & DeleteA) gIntDelete (A);
            if (Flags & DeleteB) gIntDelete (B);

            hA = 1 + (gIntLog2 (P, 0) >> 3);
            hB = 1 + (gIntLog2 (Q, 0) >> 3);

            for (j = P.ByteCount - 1, r = 0; hB--; j--, r++)
            {
                k = Q.Value [j];
                tA = hA;

                for (i = P.ByteCount - 1; tA-- && i - r >= 0; i--)
                {
                    t = Result.Value [i - r] + P.Value [i] * k;
                    Result.Value [i - r] = t;

                    w = i - r - 1;
                    t >>= 8;

                    while (t && w >= 0)
                    {
                        t += Result.Value [w];
                        Result.Value [w--] = t;
                        t >>= 8;
                    }
                }
            }

            gIntDelete (P);
            gIntDelete (Q);

            if (Flags & SignedMul && n)
                Result = gIntNeg (Result, DeleteA);

            return Result;
    }

    /* Returns the result of A / B and A % B. */
    void gIntDivMod (gInt A, gInt B, gInt *Quo, gInt *Rem, int Flags)
    {
        int i, t, n, m, s, sr, cl, cr, hD, phD, bF, bL2;
        gInt Result, U, V, nA, nB, mB;

            if (gIntZero (B))
            {
                if (Flags & DeleteA) gIntDelete (A);
                if (Flags & DeleteB) gIntDelete (B);

                if (Quo) *Quo = gIntNew (0);
                if (Rem) *Rem = gIntNew (0);

                return;
            }

            m = (n = A.ByteCount) - 1;
            i = gIntComp (A, B, GetHiDelta);

            if (i < 0)
            {
                if (Rem) U = gIntClone (A);

                if (Flags & DeleteA) gIntDelete (A);
                if (Flags & DeleteB) gIntDelete (B);

                if (Quo) *Quo = gIntConv (0, n);
                if (Rem) *Rem = U;

                return;
            }

            if (!i)
            {
                if (Flags & DeleteA) gIntDelete (A);
                if (Flags & DeleteB) gIntDelete (B);

                if (Quo) *Quo = gIntConv (1, n);
                if (Rem) *Rem = gIntConv (0, n);

                return;
            }

            Result = gIntNew (n);
            nB = gIntNew (n);
            V = gIntNew (n);

            bL2 = gIntLog2 (B, 0);
            mB = gIntClone (B);
            nA = gIntClone (A);

            if (Flags & SignedDiv)
            {
                s = ((sr = gIntNegative (nA)) + (i = gIntNegative (mB))) & 1;

                if (sr) nA = gIntNeg (nA, DeleteA);
                if (i) mB = gIntNeg (mB, DeleteA);
            }

            while (1)
            {
                hD = phD = gIntLog2 (nA, 0) - bL2;

        FixOne: gIntCopy (nB, mB);
                gIntCopyL (V, 1);

                while (hD)
                {
                    cl = hD > 7 ? 7 : hD;
                    cr = 8 - cl;

                    for (i = 0; i < n; i++)
                    {
                        nB.Value [i] <<= cl;
                        V.Value [i] <<= cl;

                        if (cr && i < m)
                        {
                            nB.Value [i] += nB.Value [i + 1] >> cr;
                            V.Value [i] += V.Value [i + 1] >> cr;
                        }
                    }
    
                    hD -= cl;
                }

                if (gIntComp (nA, nB, GetHiDelta) < 0)
                {
                    hD = phD - 1;
                    goto FixOne;
                }

                for (i = m, bF = 0; i >= 0; i--)
                {
                    t = nA.Value [i] - nB.Value [i] - bF;
                    bF = t < 0 ? 1 : 0;

                    nA.Value [i] = t;
                }

                Result = gIntAdd (Result, V, DeleteA);

                if (gIntComp (nA, mB, GetHiDelta) < 0) break;
            }

            gIntDelete (mB);
            gIntDelete (nB);
            gIntDelete (V);

            if (Flags & DeleteA) gIntDelete (A);
            if (Flags & DeleteB) gIntDelete (B);

            if (Flags & SignedDiv)
            {
                if (s)
                    Result = gIntNeg (Result, DeleteA);

                if (sr)
                    nA = gIntNeg (nA, DeleteA);
            }

            if (!Quo)
                gIntDelete (Result);
            else
                *Quo = Result;

            if (!Rem)
                gIntDelete (nA);
            else
                *Rem = nA;
    }

    /* Returns the result of A / B. */
    gInt gIntDiv (gInt A, gInt B, int Flags)
    {
        gInt Result;

            gIntDivMod (A, B, &Result, NULL, Flags);
            return Result;
    }

    /* Returns the result of A % B. */
    gInt gIntMod (gInt A, gInt B, int Flags)
    {
        gInt Result;

            gIntDivMod (A, B, NULL, &Result, Flags);
            return Result;
    }

    /* Converts a string into a bCount-byte integer. */
    gInt gIntAtoi (const char *s, int r, int bCount)
    {
        gInt RadixValue, Result, A, B;

            Result = gIntConv (0, bCount);
            if (!s) return Result;

            RadixValue = gIntConv (RadixTable [r & 3], bCount);

            while ((r = *s++) != '\0')
            {
                if (0x30 > r || r > 0x39)
                {
                    r &= 0xDF;
                    if (r < 0x41 || r > 0x46) continue;

                    r -= 0x37;
                }
                else
                    r -= 0x30;

                A = gIntMul (RadixValue, Result, DeleteB);
                B = gIntConv (r, bCount);

                Result = gIntAdd (A, B, DeleteBoth);
            }

            gIntDelete (RadixValue);

            return Result;
    }

    /* Converts an integer into a string. */
    char *gIntItoa (char *d, gInt A, int radix, int Flags)
    {
        gInt RadixValue, Quo, Rem;
        int i, c, n = 0;
        char *r = d;

            if (!d) return NULL;

            RadixValue = gIntConv (RadixTable [radix & 3], A.ByteCount);
            Quo = gIntClone (A);

            if (radix == 1 && (Flags & SignedConv) && gIntNegative (Quo))
            {
                Quo = gIntNeg (Quo, DeleteA);
                *d++ = '-';
            }

            while (!gIntZero (Quo))
            {
                gIntDivMod (Quo, RadixValue, &Quo, &Rem, DeleteA);
                c = gIntGetLong (Rem, DeleteA);

                for (i = n++; i > 0; i--) d [i] = d [i - 1];
                d [0] = Digits [c];
            }

            if (!n) d [n++] = Digits [0];
            d [n] = '\0';

            gIntDelete (RadixValue);
            gIntDelete (Quo);

            return r;
    }
