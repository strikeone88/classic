/*
    GX86C.C

    Giselle X86 Core Version 0.01

    Copyright (C) 2008 RedStar Technologies
    Written by J. Palencia (ilina@bloodykisses.zzn.com)
*/

    #include <giselle.h>
    #include <stdarg.h>

    /* Gets an argument from a variable called "args". */
    #define argv(t) va_arg (args, t)

    /* Context Structure. */
    typedef struct
    {
        GContext    *GCt;

        char        *OutputFile;
        FILE        *Out;
    }
    Context;

    static char *AlignStr [] =
    {
        "byte", "word", "dword", "para", "page256", "page"
    };

    static char *CombineStr [] =
    {
        "public", "private"
    };

    static void endl (Context *Ct)
    {
            fprintf (Ct->Out, "\n");
    }

    static void WriteByte (Context *Ct, unsigned value, int f)
    {
            if (!(f & 2))
                fprintf (Ct->Out, "db 0%02xh", value);
            else
                fprintf (Ct->Out, "db ");

            if (f & 1) endl (Ct);
    }

    static void WriteWord (Context *Ct, unsigned value, int f)
    {
            if (!(f & 2))
                fprintf (Ct->Out, "dw 0%04xh", value);
            else
                fprintf (Ct->Out, "dw ");

            if (f & 1) endl (Ct);
    }

    static void WriteDword (Context *Ct, unsigned long value, int f)
    {
            if (!(f & 2))
                fprintf (Ct->Out, "dd 0%08lxh", value);
            else
                fprintf (Ct->Out, "dd ");

            if (f & 1) endl (Ct);
    }

    static void WriteData (Context *Ct, unsigned Type, FILE *fp)
    {
        unsigned long Length;
        unsigned i, Target;

            if (Type == GDHollow)
            {
                Length = lgetd (fp);
                fprintf (Ct->Out, "org $+%lu\n", Length);
                return;
            }

            Target = Type != GDConstant ? lgetd (fp) : 0;
            Length = lgetd (fp);

            if (Length > 4)
            {
                switch (Length & 3)
                {
                    case 0x00:
               AsDword: Length >>= 2;

                        while (Length--)
                            WriteDword (Ct, lgetd (fp), 1);

                        break;

                    case 0x01:
                        WriteByte (Ct, getc (fp), 1);
                        goto AsDword;

                    case 0x02:
                        WriteWord (Ct, lgetw (fp), 1);
                        goto AsDword;

                    case 0x03:
                        WriteWord (Ct, lgetw (fp), 1);
                        WriteByte (Ct, getc (fp), 1);
                        goto AsDword;
                }
            }
            else
            {
                i = Type == GDSectionBase ? 0x02 : 0x00;

                switch (Length)
                {
                    case 0x01:
                        WriteByte (Ct, getc (fp), i);
                        break;

                    case 0x02:
                        WriteWord (Ct, lgetw (fp), i);
                        break;

                    case 0x04:
                        WriteDword (Ct, lgetd (fp), i);
                        break;
                }

                switch (Type)
                {
                    case GDSectionBase:
                        fprintf (Ct->Out, "%s", GSISection (Ct->GCt, Target)->name);
                        break;

                    case GDSectionLength:
                        break;

                    case GDSymbolOffset:
                        fprintf (Ct->Out, "+%s", GSISymbol (Ct->GCt, Target)->name);
                        break;
                }

                endl (Ct);
            }
    }

    static void WriteSymbol (Context *Ct, GSymbol *P)
    {
        int i = P->flags & 3;

            if (i != 0x02)
            {
                fprintf (Ct->Out, "%s label\n", P->name);
                if (i) fprintf (Ct->Out, "public %s\n", P->name);
            }
            else
                fprintf (Ct->Out, "extern %s\n", P->name);
    }

    static void Begin (Context *Ct)
    {
        unsigned long offset;
        unsigned Type;
        GSection *P;
        GSymbol *Q;
        FILE *fp;

            for (P = TOP(Ct->GCt->Sections); P; P = NEXT(P))
            {
                fprintf (Ct->Out, "section %s %s %s '%s'\n",
                    P->name, AlignStr [P->flags & 7],
                    CombineStr [P->flags & GSPrivate ? 1 : 0], P->class);
            }

            for (P = TOP(Ct->GCt->Sections); P; P = NEXT(P))
            {
                fp = fopen (P->bin, "rb");
                if (!fp) continue; //violet:error!!

                fprintf (Ct->Out, "\n[%s]\n", P->name);

                Q = TOP(P->Symbols);
                offset = 0;

                while (1)
                {
                    while (Q && offset == Q->offset)
                    {
                        WriteSymbol (Ct, Q);
                        Q = NEXT (Q);
                    }

                    Type = getc (fp);
                    if (feof (fp)) break;

                    if (Type != GDCode)
                        WriteData (Ct, Type, fp);

                    offset++;
                }

                fprintf (Ct->Out, "[ends]\n");

                fclose (fp);
            }
    }

    /* GCore Interface. */
    void *gx86c (unsigned Cmd, ...)
    {
        va_list args;
        Context *Ct;

            va_start (args, Cmd);

            switch (Cmd)
            {
                case GCIdentifier:
                    return "gx86c";

                case GCCreateContext:
                    return new (Context);
            }

            Ct = argv (Context *);
            if (!Ct) return GCError;

            switch (Cmd)
            {
                case GCDestroyContext:
                    delete (Ct);
                    break;

                case GCOutputFile:
                    Ct->OutputFile = argv (char *);
                    break;

                case GCBegin:
                    Ct->GCt = argv (GContext *);
                    if (!Ct->OutputFile) return GCError;

                    Ct->Out = fopen (Ct->OutputFile, "wt");
                    if (!Ct->Out) return GCError;

                    Begin (Ct);

                    fclose (Ct->Out);

/*violet:build VO file!!*/

                    break;
            }

            return GCOk;
    }
