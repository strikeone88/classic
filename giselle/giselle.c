/*
    GISELLE.C

    Giselle Code Generation Engine Version 0.01

    Copyright (C) 2008 RedStar Technologies
    Written by J. Palencia (ilina@bloodykisses.zzn.com)
*/

    #include <giselle.h>

    /* Local temporal buffer. */
    static unsigned char tbuf [128];

    /* Creates a context structure. */
    GContext *GCreate (void)
    {
        GContext *Ct = new (GContext);

            Ct->Sections = new (list_t);
            return Ct;
    }

    /* Destroys the context structure. */
    void GDestroy (GContext *Ct)
    {
        void *P, *Q;

            if (Ct->Selected) fclose (Ct->Output);

            while ((P = list__extract (Ct->Sections)) != NULL)
            {
                while ((Q = list__extract (((GSection *)P)->Symbols)) != NULL)
                    delete (Q);

                delete (((GSection *)P)->Symbols);

                remove (((GSection *)P)->bin);
                delete (((GSection *)P)->bin);

                delete (P);
            }

            delete (Ct->Sections);

            if (Ct->Core) Ct->Core (GCDestroyContext, Ct->CoreCt);

            delete (Ct);
    }

    /* Adds a section node to the list. */
    GSection *GASection (GContext *Ct, char *name, char *class, GSFlags flags)
    {
        GSection *P;

            if (!Ct || !name) return NULL;

            P = new (GSection);

            P->index = 1 + COUNT (Ct->Sections);

            sprintf (tbuf, "g$s@%04u.dat", P->index);
            createfile (P->bin = dups (tbuf));

            P->Symbols = new (list_t);
            P->class = class;
            P->name = name;

            P->flags = flags;
            P->offset = 0;

            list__add (Ct->Sections, P);

            return P;
    }

    /* Exports the section using the given global name. */
    GSection *GExpSection (GSection *S, char *gname)
    {
            if (!S) return NULL;

            S->flags &= ~GSExport & ~GSImport;

            if (!gname) return S;

            S->flags |= GSExport;
            S->gname = gname;

            return S;
    }

    /* Imports the section using the given global name. */
    GSection *GImpSection (GSection *S, char *gname)
    {
            if (!S) return NULL;

            S->flags &= ~GSExport & ~GSImport;

            if (!gname) return S;

            S->flags |= GSImport;
            S->gname = gname;

            return S;
    }

    /* Removes a section node from the list. */
    void GRSection (GContext *Ct, GSection *P)
    {
            if (!Ct || !P) return;

            list__rem (Ct->Sections, P);

            remove (P->bin);
            delete (P->bin);

            delete (P);
    }

    /* Searches for a section and returns its node. */
    GSection *GSSection (GContext *Ct, char *name)
    {
        GSection *P;

            if (!Ct || !name) return NULL;

            for (P = TOP(Ct->Sections); P; P = NEXT(P))
                if (!strcmp (P->name, name)) break;

            return P;
    }

    /* Searches for a section given its index. */
    GSection *GSISection (GContext *Ct, unsigned index)
    {
        GSection *P;

            if (!Ct || !index) return NULL;

            for (P = TOP(Ct->Sections); P; P = NEXT(P))
                if (P->index == index) break;

            return P;
    }

    /* Adds a symbol node to the list. */
    GSymbol *GASymbol (GContext *Ct, char *name, GSection *Sp, GSYFlags flags)
    {
        GSymbol *P;

            if (!Ct || !name || !Sp) return NULL;

            P = new (GSymbol);

            P->index = 1 + Ct->SymbolC++;
            P->name = name;

            P->flags = flags;

            P->offset = Sp->offset;
            P->Section = Sp;

            list__add (Sp->Symbols, P);

            return P;
    }

    /* Searches for a Symbol and returns its node. */
    GSymbol *GSSymbol (GContext *Ct, char *name)
    {
        GSection *P; GSymbol *Q;

            if (!Ct || !name) return NULL;

            for (P = TOP(Ct->Sections); P; P = NEXT(P))
            {
                for (Q = TOP(P->Symbols); Q; Q = NEXT(Q))
                    if (!strcmp (Q->name, name)) return Q;
            }

            return NULL;
    }

    /* Searches for a Symbol given its index. */
    GSymbol *GSISymbol (GContext *Ct, unsigned long index)
    {
        GSection *P; GSymbol *Q;

            if (!Ct || !index) return NULL;

            for (P = TOP(Ct->Sections); P; P = NEXT(P))
            {
                for (Q = TOP(P->Symbols); Q; Q = NEXT(Q))
                    if (Q->index == index) return Q;
            }

            return NULL;
    }

    /* Updates a symbol offset given its name. */
    GSymbol *GUSymbol (GContext *Ct, char *name)
    {
        GSection *P; GSymbol *Q;

            if (!Ct || !name) return NULL;

            for (P = TOP(Ct->Sections); P; P = NEXT(P))
            {
                for (Q = TOP(P->Symbols); Q; Q = NEXT(Q))
                    if (!strcmp (Q->name, name))
                    {
                        Q->offset = P->offset;

                        list__rem (P->Symbols, Q);
                        list__add (P->Symbols, Q);

                        return Q;
                    }
            }

            return NULL;
    }

    /* Updates a symbol offset given its name. */
    GSymbol *GUISymbol (GContext *Ct, unsigned long index)
    {
        GSection *P; GSymbol *Q;

            if (!Ct || !index) return NULL;

            for (P = TOP(Ct->Sections); P; P = NEXT(P))
            {
                for (Q = TOP(P->Symbols); Q; Q = NEXT(Q))
                    if (Q->index == index)
                    {
                        Q->offset = P->offset;

                        list__rem (P->Symbols, Q);
                        list__add (P->Symbols, Q);

                        return Q;
                    }
            }

            return NULL;
    }

    /* Opens and selects the section for output. */
    int GOpenOutput (GContext *Ct, GSection *Sp)
    {
            if (!Ct || !Sp) return 1;

            if (Ct->Selected)
            {
                fclose (Ct->Output);
                Ct->Selected = NULL;
            }

            Ct->Output = fopen (Sp->bin, "r+b");
            if (!Ct->Output) return 2;

            fseek (Ct->Output, 0, SEEK_END);

            Ct->Selected = Sp;

            return 0;
    }

    /* Closes the currently selected section. */
    void GCloseOutput (GContext *Ct)
    {
            if (!Ct || !Ct->Selected) return;

            fclose (Ct->Output);
            Ct->Selected = NULL;
    }

    /* Writes a data item to the selected section. */
    int GDataItem (GContext *Ct, GDType type, unsigned long target,
                   void *buf, unsigned length)
    {
            if (!Ct || !Ct->Selected || !length || type == GDHollow)
                return 1;

            putc (type, Ct->Output);

            if (type != GDConstant)
                lputd (target, Ct->Output);

            lputd (length, Ct->Output);

            if (buf)
            {
                if (fwrite (buf, 1, length, Ct->Output) != length)
                    return 2;
            }
            else
                while (length--) putc (0, Ct->Output);

            Ct->Selected->offset++;

            return 0;
    }

    /* Writes a hollow data item to the selected section. */
    int GHollowDataItem (GContext *Ct, unsigned long length)
    {
            if (!Ct || !Ct->Selected || !length) return 1;

            putc (GDHollow, Ct->Output);
            lputd (length, Ct->Output);

            Ct->Selected->offset++;

            return 0;
    }

    /* Selects the target machine code generation core. */
    int GSelectCore (GContext *Ct, GCore *Cp)
    {
            if (!Ct || !Cp) return 1;

            Ct->CoreId = Cp (GCIdentifier);
            Ct->Core = Cp;

            return 0;
    }

    /* Enables a capability of the Core. */
    int GEnableCap (GContext *Ct, unsigned cap)
    {
            if (!Ct || !Ct->Core) return 1;
            return (int)Ct->Core (GCEnableCap, cap);
    }

    /* Disables a capability of the Core. */
    int GDisableCap (GContext *Ct, unsigned cap)
    {
            if (!Ct || !Ct->Core) return 1;
            return (int)Ct->Core (GCDisableCap, cap);
    }

    /* Indicates that the generation must be finished. */
    int GFinish (GContext *GCt)
    {
        GCore *Core;
        void *Ct;

            if (!GCt || !GCt->Core) return 1;

            GCt->CoreCt = Ct = (Core = GCt->Core) (GCCreateContext);
//violet:ask for SOURCE, OUTPUT and TRANSLATOR
            Core (GCOutputFile, Ct, "Z_TEST.ASM");

            Core (GCBegin, Ct, GCt);

            Core (GCDestroyContext, Ct);
            GCt->CoreCt = NULL;

            return 0;
    }
