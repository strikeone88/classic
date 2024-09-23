
    #include <giselle.h>

    void Error (unsigned linenum)
    {
            printf ("Error at Line %u\n", linenum);
            exit (2);
    }

    void *gx86c (GCommand Cmd, ...);

    void main (void)
    {
        unsigned short Value = 0xAA55;
        unsigned long Value2 = 0xDEAD1234;
        GContext *Ct;
        GSection *S, *P, *Q;

            Ct = GCreate ();
            if (!Ct) Error (__LINE__);

            S = GASection (Ct, "_TEXT", "CODE", GSByte + GDByte + GSPublic);
            P = GASection (Ct, "_DATA", "DATA", GSDword + GDByte + GSPublic);
            Q = GASection (Ct, "_BSS", "BSS", GSDword + GDByte + GSPublic);

            if (!GASymbol (Ct, "BootSignature", S, GSYPublic))
                Error (__LINE__);

            if (GOpenOutput (Ct, S))
                Error (__LINE__);

            if (GDataItem (Ct, GDConstant, 0x0000, &Value, 2))
                Error (__LINE__);

            if (GOpenOutput (Ct, P))
                Error (__LINE__);

            if (GDataItem (Ct, GDSymbolOffset, 0x0001, &Value2, 4))
                Error (__LINE__);

            if (GOpenOutput (Ct, Q))
                Error (__LINE__);

            GHollowDataItem (Ct, 100);

            if (GSelectCore (Ct, gx86c))
                Error (__LINE__);

            GCloseOutput (Ct);

            printf ("Using \"%s\" core...", Ct->CoreId);

            GFinish (Ct);
            GDestroy (Ct);

            printf ("\nfinished.\n\n");
    }
