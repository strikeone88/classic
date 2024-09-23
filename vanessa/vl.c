/*
    VL.C

    RedStar Vanessa Linker Version 0.03

    Copyright (C) 2008-2014 RedStar Technologies
    Written by J. Palencia (ciachn@gmail.com)
*/

    #include <gstring.h>
    #include <gutils.h>

    #include <stdarg.h>
    #include <aurora.h>
    #include <alloc.h>

    /* Minimum and maximum valid record code. */
    #define VO_MinCode      0x80
    #define VO_MaxCode      0x8A

    /* End of Vanessa object file record code. */
    #define VO_EOF          (0x89 - VO_MinCode)

    /* Highest version of supported VOs. */
    #define VO_MAJOR        0
    #define VO_MINOR        1

    /* Section flags. */
    #define F__MASTER       1
    #define F__CODE         2
    #define F__STACK        4

    /* Allowed executable formats. */
    #define F__SBX          0
    #define F__BIN          1
    #define F__VX           2
    #define F__EXE          3
    #define F__CFX          4

    /* Name of the temporal image and symbols file. */
    #define IMAGE_FILE      "vl$image.tmp"
    #define SYM_FILE        "vl$sym.tmp"

    /* Group information node. */
    typedef struct /* direct cast: linkable_t */
    {
        linkable_t  link;
        unsigned    long name;
        struct      section_s *base;
    }
    group_t;

    /* Section-Sequence node. */
    typedef struct /* direct cast: linkable_t */
    {
        linkable_t  link;
        struct      section_s *s;
    }
    seqnode_t;

    /* Section information structure. */
    typedef struct section_s /* direct cast: linkable_t */
    {
        linkable_t  link;

        struct      vo_s *vo;
        struct      section_s *s;

        unsigned    long name;
        unsigned    long class;
        unsigned    long group;

        unsigned    attributes;

        unsigned    long length;
        unsigned    long offset;
        unsigned    long origin;

        unsigned    long physical;

        unsigned    char flags;

        char        *import;
        char        *export;

        unsigned    long publicC;
        unsigned    long externC;
    }
    section_t;

    /* 32-bit integer node. */
    typedef struct /* direct cast: linkable_t */
    {
        linkable_t  link;
        unsigned    long value;
    }
    int_t;

    /* VO information node. */
    typedef struct vo_s /* direct cast: strn_t, linkable_t */
    {
        linkable_t  link;
        char        *path;

        char        *source, *translator;

        unsigned    long publics;
        unsigned    long fixups;
        unsigned    long debug;

        section_t   **sectionL;
        unsigned    long *groupL;

        section_t   *section;
        unsigned    long offset;

        dict_t      *indexMap;

        unsigned    long base;

        unsigned    long rfixupC; /* Runtime Section Fixup */
        unsigned    long sfixupC; /* Runtime Symbol Fixup */
        unsigned    long fixupC; /* Count of Fixups */

        dict_t      *dfixups; /* Dict of Delayed Fixups */
    }
    vo_t;

    /* LIB information node. */
    typedef struct /* direct cast: strn_t, linkable_t */
    {
        linkable_t  link;
        char        *path;
        dict_t      *dict;
    }
    lib_t;

    /* Temporal global buffers. */
    static char gbuf [1024], tbuf [1024], qbuf [256];

    /* Count of errors. */
    static unsigned errc;

    /* VO node containing the entry point. */
    static vo_t *entry;

    /* List of VO nodes, sections and groups. */
    static list_t *volist, *sections, *groups, *libs;

    /* String of options and argument counts. */
    static char *optStr = "fAmPpToOGLsBFS";
    static char *argCnt = "11100111010101";

    /* Forced executable base. */
    static unsigned long base;

    /* Indicates if Vanessa should probe local imports resolution. */
    static unsigned probeFirst;

    /* Name of valid executable formats. */
    static char *formats [] =
    {
        "sbx", "bin", "vx", "exe", "cfx"
    };

    /* Indicates the next valid record code for a given record code. */
    static unsigned nextRecordCode [][VO_MaxCode - VO_MinCode + 1] =
    {
    /*  80  81  82  83  84  85  86  87  88  89  8A  */
        0,  1,  1,  0,  0,  0,  0,  0,  0,  0,  0,  /* 80 */
        0,  1,  0,  0,  0,  0,  0,  0,  0,  1,  1,  /* 81 */
        0,  0,  0,  1,  0,  0,  0,  0,  0,  0,  0,  /* 82 */
        0,  0,  0,  1,  1,  0,  1,  1,  0,  0,  0,  /* 83 */
        0,  0,  0,  0,  0,  1,  0,  0,  0,  0,  0,  /* 84 */
        0,  0,  0,  0,  1,  1,  0,  1,  0,  0,  0,  /* 85 */
        0,  0,  0,  1,  1,  0,  0,  1,  0,  0,  0,  /* 86 */
        0,  0,  0,  0,  0,  0,  0,  0,  1,  0,  0,  /* 87 */
        0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  /* 88 */
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  /* 89 */
    };

    /* Global dictionary file. */
    static FILE *dictfile;

    /* Bottom offset of the dictionary file. */
    static unsigned long bottom;

    /* Dictionary of symbols, publics and externals. */
    static dict_t *symbols, *publics, *externals, *libdict;

    /* Output image file and map file. */
    static FILE *image, *mfile;

    /* Some indicators. */
    static int onmem, mklib;

    /* Current output executable file. */
    static FILE *output;

    /* Allocates a block on the file and returns the offset. [1] */
    unsigned long d__alloc (unsigned long size)
    {
            if (onmem)
                return (unsigned long)alloc (size);
            else
                return (bottom += size) - size;
    }

    /* Writes N bytes from buf to the offset. [1] */
    void d__write (void *buf, unsigned n, unsigned long offset)
    {
            if (!onmem)
            {
                fseek (dictfile, offset, SEEK_SET);
                fwrite (buf, 1, n, dictfile);
            }
            else
                memcpy ((void *)offset, buf, n);
    }

    /* Reads N bytes from the given offset into buf. [1] */
    void d__read (void *buf, unsigned n, unsigned long offset)
    {
            if (!onmem)
            {
                fseek (dictfile, offset, SEEK_SET);
                fread (buf, 1, n, dictfile);
            }
            else
                memcpy (buf, (void *)offset, n);
    }

    /* Similar to putsb but to the dictfile. */
    static void d__putsb (const char *s, unsigned long offset)
    {
        unsigned i = strlen (s);

            if (i > 255) i = 255;

            if (onmem)
            {
                *((char *)offset) = i;
                memcpy ((void *)(offset + 1), s, i);

                return;
            }

            fseek (dictfile, offset, SEEK_SET);

            putc (i, dictfile);

            while (i--) putc (*s++, dictfile);
    }

    /* Similar to getsb but from the dictfile. */
    static char *d__getsb (char *s, unsigned long offset)
    {
        char *p = s;
        unsigned i;

            if (!offset) return strcpy (s, "(none)");

            if (onmem)
            {
                i = *((char *)offset);
                memcpy (s, (void *)(offset + 1), i);

                s [i] = '\0';
            }
            else
            {
                fseek (dictfile, offset, SEEK_SET);
                i = getc (dictfile);

                while (i--) *s++ = getc (dictfile);
                *s = '\0';
            }

            return p;
    }

    /* Similar to lputd but to the dictfile. */
    static void d__lputd (unsigned long val, unsigned long offset)
    {
            if (!onmem)
            {
                fseek (dictfile, offset, SEEK_SET);
                lputd (val, dictfile);
            }
            else
                *((unsigned long *)offset) = val;
    }

    /* Similar to lgetd but from the dictfile. */
    static unsigned long d__lgetd (unsigned long offset)
    {
            if (!onmem)
            {
                fseek (dictfile, offset, SEEK_SET);
                return lgetd (dictfile);
            }
            else
                return *((unsigned long *)offset);
    }

    /* Similar to lputw but to the dictfile. */
    static void d__lputw (unsigned long val, unsigned long offset)
    {
            if (!onmem)
            {
                fseek (dictfile, offset, SEEK_SET);
                lputw (val, dictfile);
            }
            else
                *((unsigned short *)offset) = val;
    }

    /* Similar to lgetw but from the dictfile. */
    static unsigned long d__lgetw (unsigned long offset)
    {
            if (!onmem)
            {
                fseek (dictfile, offset, SEEK_SET);
                return lgetw (dictfile);
            }
            else
                return *((unsigned short *)offset);
    }

    /* Similar to putc but to the dictfile. */
    static void d__putc (unsigned val, unsigned long offset)
    {
            if (!onmem)
            {
                fseek (dictfile, offset, SEEK_SET);
                putc (val, dictfile);
            }
            else
                *((unsigned char *)offset) = val;
    }

    /* Similar to getc but from the dictfile. */
    static unsigned d__getc (unsigned long offset)
    {
            if (!onmem)
            {
                fseek (dictfile, offset, SEEK_SET);
                return getc (dictfile);
            }
            else
                return *((unsigned char *)offset);
    }

    /* Prints a "formal" message on the screen. */
    void pmsg (char *fmts, ...)
    {
        static int flg = 0;
        va_list p;
        char *s;

            va_start (p, fmts);

            switch (*fmts++)
            {
                case '$':   s = " **FATAL**";
                            errc++;
                            break;

                case '*':   s = " **ERROR**";
                            errc++;
                            break;

                case '#':   s = " **WARNING**";
                            break;

                default:    s = "";
                            fmts--;
            }

            if (flg) printf ("\n"); else flg = 1;

            printf ("vanessa%s: ", s);

            vprintf (fmts, p);
    }

    /* Finds a group named x. */
    static group_t *findGroup (unsigned long x)
    {
        group_t *t;

            for (t = TOP(groups); t; t = NEXT(t))
                if (t->name == x) return t;

            return NULL;
    }

    /* Finds a public symbol named x and returns info about it. */
    static int findPublic (unsigned long x, section_t **base, unsigned long *offs)
    {
            x = dict__dsrch (publics, &x, 4);
            if (!x) return 0;

            *base = (void *)d__lgetd (x);
            *offs = d__lgetd (x + 4);

            return 1;
    }

    /* Loads a VO file. */
    static void loadvo (FILE *fp, unsigned long base, char *s)
    {
        unsigned i, j, k, c, ok = 0, dc = 0, symC, groupC = 0, sectionC = 0;
        unsigned long v, p, q, r, tt, t, x, lastGroup, *sym = NULL;
        vo_t *vo; section_t *section, *ts; group_t *group;
        char *u, *m = NULL, **ssym = NULL;
        lib_t *lib;

            if (!fp)
            {
                fp = fopen (s, "rb");
                if (!fp)
                {
                    pmsg ("$unable to open input file: %s", s);
                    return;
                }

                if (!stricmp (getextension (s), "lib"))
                {
                    if (getc (fp) != 0x90)
                    {
                        pmsg ("*input file '%s' is not a valid vanessa library file", s);
                        goto ret;
                    }

                    getsb (gbuf, sizeof (gbuf), fp);

                    lib = new (lib_t);

                    lib->dict = new (dict_t);
                    lib->path = dups (s);

                    lib->dict->maxHeight = 128;
                    lib->dict->root = lgetd (fp);

                    list__add (libs, lib);

                    fclose (fp);

                    return;
                }
            }
            else
            {
                fseek (fp, base, SEEK_SET);
                dc = 1;
            }

            if (getc (fp) != 0x80)
            {
                pmsg ("*input file '%s' is not a valid vanessa object file", s);

           ret: if (!dc) fclose (fp);

                if (sym) delete (sym);
                if (m) delete (m);

                if (ssym)
                {
                    for (i = 0; i < symC; i++) delete (ssym [i]);
                    delete (ssym);
                }

                return;
            }

            i = getc (fp);
            j = getc (fp);

            if (i > VO_MAJOR || j > VO_MINOR)
            {
                pmsg ("*version %u.%02x (%s) not supported", i, j, s);
                goto ret;
            }

            if (!mklib)
            {
                vo = new (vo_t);

                if (!dc) vo->path = dups (s); else vo->path = s;

                getsb (gbuf, sizeof (gbuf), fp);
                vo->translator = dups (gbuf);

                vo->indexMap = new (dict_t);
                vo->indexMap->maxHeight = 2;

                vo->dfixups = new (dict_t);
                vo->dfixups->maxHeight = 4;

                vo->base = base;
            }
            else
                getsb (gbuf, sizeof (gbuf), fp);

            c = 0;

            while (c != VO_EOF)
            {
                i = getc (fp);

                if (VO_MinCode > i || i > VO_MaxCode)
                {
                    pmsg ("*(%s) unknown record code %02x", s, i);
                    goto ret;
                }

                i -= VO_MinCode;

                if (!nextRecordCode [c][i])
                {
                    pmsg ("*(%s, %02x) unexpected record %02x", s, c + VO_MinCode, i + VO_MinCode);
                    goto ret;
                }

                switch (c = i)
                {
                    case 0x01:
/*violet:capabilities*/
                        p = lgetd (fp);
                        q = lgetd (fp);
                        break;

                    case 0x02:
                        getsb (gbuf, sizeof (gbuf), fp);
                        p = lgetd (fp);
                        i = getc (fp);
                        j = getc (fp);

                        if (!mklib)
                        {
                            vo->sectionL = alloc (4L * i);
                            vo->groupL = alloc (4L * j);

                            vo->source = dups (gbuf);
                        }
                        else
                            m = dups (gbuf);

                        q = ftell (fp);

                        fseek (fp, base + p + 4, SEEK_SET);
                        p = symC = lgetd (fp);

                        if (mklib)
                            ssym = alloc (p * 4L);
                        else
                            sym = alloc (p * 4L);

                        j = 0;

                        while (p--)
                        {
                            getsb (gbuf, sizeof (gbuf), fp);

                            if (mklib)
                            {
                                ssym [j++] = dups (gbuf);
                                continue;
                            }

                            t = dict__dsrch (symbols, gbuf, i = strlen (gbuf));

                            if (!symbols->lastSrch)
                            {
                                r = d__alloc (i + 1);
                                d__putsb (gbuf, r);

                                dict__dadd (symbols, gbuf, i, r);

                                sym [j] = r;
                            }
                            else
                                sym [j] = t;

                            dict__dadd (vo->indexMap, &j, 2, sym [j]);

                            j++;
                        }

                        fseek (fp, q, SEEK_SET);
                        break;

                    case 0x03:
                        if (mklib)
                        {
                            fseek (fp, 15, SEEK_CUR);
                            break;
                        }

                        section = new (section_t);
                        section->vo = vo;

                        vo->sectionL [sectionC++] = section;
                        list__add (sections, section);

                        section->name = sym [getc (fp) - 1];

                        if ((i = getc (fp)) != 0)
                        {
                            section->class = sym [i - 1];

                            d__getsb (gbuf, section->class);

                            if (!stricmp (gbuf, "CODE"))
                                section->flags |= F__CODE;

                            if (!stricmp (gbuf, "STACK"))
                                section->flags |= F__STACK;
                        }

                        section->length = lgetd (fp);
                        section->attributes = getc (fp);
                        section->offset = lgetd (fp);
                        section->origin = lgetd (fp);

                        if (!section->length && section->offset)
                        {
                            p = ftell (fp);
                            fseek (fp, base + section->offset, SEEK_SET);
                            section->length = lgetd (fp);
                            fseek (fp, p, SEEK_SET);
                        }

                        if (section->offset) section->offset += 4;

                        break;

                    case 0x04:
                        if (mklib)
                        {
                            (void)getc (fp);
                            (void)getc (fp);
                            break;
                        }

                        (void)getc (fp);
                        lastGroup = sym [getc (fp) - 1];

                        vo->groupL [groupC++] = lastGroup;

                        if (findGroup (lastGroup)) break;

                        group = new (group_t);

                        group->name = lastGroup;
                        group->base = NULL;

                        list__add (groups, group);
                        break;

                    case 0x05:
                        if (mklib)
                        {
                            (void)getc (fp);
                            break;
                        }

                        vo->sectionL [getc (fp) - 1]->group = lastGroup;
                        break;

                    case 0x06:
                        i = getc (fp);
                        getsb (gbuf, sizeof (gbuf), fp);

                        if (i & 1)
                            section->import = dups (gbuf);
                        else
                            section->export = dups (gbuf);

                        break;

                    case 0x07:
                        i = getc (fp);

                        if (!i)
                        {
                            lgetd (fp);
                            break;
                        }

                        if (mklib) break;

                        vo->section = vo->sectionL [i - 1];
                        vo->offset = lgetd (fp);

                        if (!entry) entry = vo;
                        break;

                    case 0x08:
                        p = ftell (fp) + 4;
                        q = lgetd (fp);

                        if (!mklib) vo->publics = q;

                        if (!q) goto getExternals;

                        fseek (fp, base + q + 4, SEEK_SET);

                        q = lgetd (fp);

                        if (mklib)
                        {
                            p = ftell (fp);

                            rewind (fp);

                            x = d__alloc (v = fileLength (fp));

                            fseek (dictfile, x, SEEK_SET);

                            tt = v;

                            while (v)
                            {
                                if (v > sizeof (gbuf))
                                    i = sizeof (gbuf);
                                else
                                    i = v;

                                fread (gbuf, 1, i, fp);
                                fwrite (gbuf, 1, i, dictfile);

                                v -= i;
                            }

                            fseek (fp, p, SEEK_SET);

                            ok = 1;
                        }

                        while (q--)
                        {
                            i = getc (fp);

                            if ((j = i & 0x1F) == 0) j = getc (fp);

                            if (i & 0x80)
                                k = lgetw (fp);
                            else
                                k = getc (fp);

                            switch ((i >> 5) & 3)
                            {
                                case 0: r = getc (fp);
                                        break;

                                case 1: r = lgetw (fp);
                                        break;

                                case 2: r = lgetw (fp);
                                        t = getc (fp);

                                        r |= t << 16;
                                        break;

                                case 3: r = lgetd (fp);
                                        break;
                            }

                            if (mklib)
                            {
                                u = ssym [k - 1];

                                dict__dsrch (libdict, u, i = strlen (u));

                                if (!libdict->lastSrch)
                                    dict__dadd (libdict, u, i, x);
                                else
                                    pmsg ("#%s: '%s' already defined", m, u);

                                continue;
                            }

                            t = sym [k - 1];

                            v = dict__dsrch (publics, &t, 4);

                            if (!publics->lastSrch)
                            {
                                x = d__alloc (12);

                                d__lputd ((unsigned long)vo->sectionL [--j], x);
                                d__lputd (r, x + 4);
                                d__lputd (t, x + 8);

                                dict__dadd (publics, &t, 4, x);

                                vo->sectionL [j]->publicC++;
                            }
                            else
                            {
                                section = (void *)d__lgetd (v);

                                pmsg ("#%s: '%s' already defined in %s",
                                    vo->source, d__getsb (tbuf, t), section->vo->source);
                            }
                        }

                        fseek (fp, p, SEEK_SET);

          getExternals: if (mklib)
                        {
                            c = VO_EOF;
                            break;
                        }

                        q = lgetd (fp);
                        p += 4;
                        if (!q) goto getDebug;

                        fseek (fp, base + q + 4, SEEK_SET);

                        q = lgetd (fp);

                        while (q--)
                        {
                            t = sym [lgetw (fp) - 1];
                            j = getc (fp);

                            v = dict__dsrch (externals, &t, 4);
                            if (externals->lastSrch)
                            {
                                ts = (section_t *)d__lgetd (v + 4);

                                if (!ts && j)
                                {
                                    d__lputd ((unsigned long)vo->sectionL [--j], v + 4);
                                    vo->sectionL [j]->externC++;

                                    continue;
                                }

                                if (ts && j)
                                {
                                    if (strcmp (vo->sectionL [j - 1]->import, ts->import))
                                    {
                                        pmsg ("*%s: symbol %s (%s) imported differently",
                                            ts->vo->source, d__getsb (gbuf, t), vo->source);
                                    }
                                }

                                continue;
                            }

                            x = d__alloc (10);

                            d__lputd (t, x);

                            if (j)
                            {
                                d__lputd ((unsigned long)vo->sectionL [--j], x + 4);
                                vo->sectionL [j]->externC++;
                            }
                            else
                                d__lputd (0, x + 4);

                            dict__dadd (externals, &t, 4, x);
                        }

                        fseek (fp, p, SEEK_SET);

              getDebug: vo->debug = lgetd (fp);
                        vo->fixups = lgetd (fp);
                        break;

                    case 0x09:
                        break;

                    case 0x0A:
                        pmsg ("*vo file is invalid or too new, record %02x is still reserved", c + VO_MinCode);
                        goto ret;
                }
            }

            if (!mklib) list__add (volist, vo);

            if (!dc) fclose (fp);

            if (sym) delete (sym);
            if (m) delete (m);

            if (ssym)
            {
                for (i = 0; i < symC; i++) delete (ssym [i]);
                delete (ssym);
            }

            if (!ok && mklib)
                pmsg ("#module '%s' doesn't have any publics!!", s);

            if (ok) pmsg ("module '%s' added to library (%lu bytes)", s, tt);
    }

    /* Tries to resolve an external symbol (imports VOs if necessary). */
    static int resolveExternal (unsigned long offs)
    {
        unsigned long sym, section;
        FILE *fp, *df = dictfile;
        int i, xonmem = onmem;
        lib_t *lib;

            sym = d__lgetd (offs);
            section = d__lgetd (offs + 4);

            if (section) return 0;

            if (dict__dsrch (publics, &sym, 4))
                return 0;

            d__getsb (gbuf, sym);
            i = strlen (gbuf);

            onmem = 0;

            for (lib = TOP(libs); lib; lib = NEXT(lib))
            {
                fp = fopen (lib->path, "rb");
                if (!fp) continue;

                dictfile = fp;

                sym = dict__dsrch (lib->dict, gbuf, i);
                if (sym)
                {
                    dictfile = df;
                    onmem = xonmem;

                    loadvo (fp, sym, lib->path);

                    fclose (fp);

                    return 1;
                }

                fclose (fp);
            }

            dictfile = df;
            onmem = xonmem;

            return 0;
    }

    /* Resolves the externals the imports VOs from libs if necessary. */
    static void resolveExternals (void)
    {
            while (dict__dwalk (externals, resolveExternal));
    }

    /* Re-groups the sectios. */
    static void regroupSections (void)
    {
        section_t *p, *q;

            /* If a section doesn't have a group but it has the same
               name and class as one section of the group, then it
               will be added to that group. */

            for (p = TOP(sections); p; p = NEXT(p))
            {
                if (!p->group) continue;

                for (q = TOP(sections); q; q = NEXT(q))
                {
                    if (q->group) continue;

                    if (q->class == p->class && q->name == p->name)
                        q->group = p->group;
                }
            }
    }

    /* Builds the section sequence and returns a list of seqnode_t nodes. */
    static list_t *buildSectionSequence (void)
    {
        seqnode_t *p, *q, *r; section_t *s;
        unsigned long x, y, z;
        list_t *l0, *l1;

            l0 = new (list_t);
            l1 = new (list_t);

            /* Build Section Sequence */
            for (s = TOP(sections); s; s = NEXT(s))
            {
                q = new (seqnode_t);
                q->s = s;

                list__add (l0, q);
            }

            /* Group by Group-Name. */
            while ((p = list__extract (l0)) != NULL)
            {
                x = p->s->group;
                list__add (l1, p);

                for (p = TOP(l0); p; p = q)
                {
                    q = NEXT(p);
                    if (p->s->group != x) continue;

                    list__rem (l0, p);
                    list__add (l1, p);
                }
            }

            /* Group by Section-Name. */
            while ((p = list__extract (l1)) != NULL)
            {
                x = p->s->group;
                y = p->s->name;
                q = NEXT(p);

                list__add (l0, p);

                for (p = TOP(l1); p && p->s->group == x; p = q)
                {
                    q = NEXT(p);
                    if (p->s->name != y) continue;

                    list__rem (l1, p);
                    list__add (l0, p);
                }
            }

            /* Group by Class-Name. */
            while ((p = list__extract (l0)) != NULL)
            {
                x = p->s->group;
                y = p->s->name;
                z = p->s->class;

                p->s->flags |= F__MASTER;

                q = NEXT(p);

                list__add (l1, p);

                for (p = TOP(l0); p && p->s->group == x && p->s->name == y; p = q)
                {
                    q = NEXT(p);
                    if (p->s->class != z) continue;

                    list__rem (l0, p);
                    list__add (l1, p);
                }
            }

            /* Adjust F__MASTER bit of the sections. */
            for (p = TOP(l1), q = NULL; p; q = p, p = NEXT(p))
            {
                if (!p->s->group && p->s->attributes & 0x10)
                    p->s->flags |= F__MASTER;

                if (!p->s->group)
                {
                    if (p->s->flags & F__MASTER)
                        r = p;
                    else
                        p->s->s = r->s;

                    continue;
                }

                if (!q || (q->s->group != p->s->group && p->s->group))
                {
                    r = p;
                    continue;
                }

                p->s->flags &= ~F__MASTER;
                p->s->s = r->s;
            }

            delete (l0);

            return l1;
    }

    /* Returns the length of a section-group. */
    static unsigned long sectionGroupLen (section_t *s, list_t *l0)
    {
        seqnode_t *q, *r; section_t *p;

            if (!s->s) p = s; else p = s->s;

            for (q = TOP(l0); q && q->s != s; q = NEXT(q));

            while (q && (q->s == s || q->s->s == p))
                r = q, q = NEXT(q);

            if (!q)
                return r->s->physical + r->s->length - s->physical;
            else
                return q->s->physical - s->physical;
    }

    /* Writes the section image to the image file. */
    static void writeSectionImage (unsigned long align, section_t *s, list_t *l0)
    {
        unsigned long p = ftell (image), q = s->origin;
        FILE *fp;

            p = ((p + align - 1) & -align) - p;
            while (p--) putc (0, image);

            s->physical = ftell (image);

            if (!(p = s->length)) return;

            if (!s->offset)
            {
                p += q;

                while (p--) putc (0, image);
                return;
            }

            fp = fopen (s->vo->path, "rb");
            if (!fp)
            {
                pmsg ("$unable to open input file: %s", s->vo->path);
                return;
            }

            fseek (fp, s->vo->base + s->offset, SEEK_SET);

            while (q--) putc (0, image);

            s->physical = ftell (image);

            while (p--) putc (getc (fp), image);

            fclose (fp);
    }

    /* Writes the public definitions to the map file. */
    static void mapPublics (vo_t *vo)
    {
        unsigned long p, q, r, *sym;
        FILE *fp; section_t *t;
        int i, j, k;

            if (!vo->publics) return;

            fp = fopen (vo->path, "rb");
            if (!fp)
            {
                pmsg ("$unable to open input file: %s", vo->path);
                return;
            }

            fseek (fp, vo->base + vo->publics + 4, SEEK_SET);

            q = lgetd (fp);

            fprintf (mfile, "\nSource \"%s\" Translated by %s\n", vo->source, vo->translator);

            while (q--)
            {
                i = getc (fp);

                if ((j = i & 0x1F) == 0) j = getc (fp);

                if (i & 0x80)
                    k = lgetw (fp) - 1;
                else
                    k = getc (fp) - 1;

                switch ((i >> 5) & 3)
                {
                    case 0: r = getc (fp);
                            break;

                    case 1: r = lgetw (fp);
                            break;

                    case 2: r = lgetw (fp);
                            p = getc (fp);

                            r |= p << 16;
                            break;

                    case 3: r = lgetd (fp);
                            break;
                }

                p = dict__dsrch (vo->indexMap, &k, 2);
                t = vo->sectionL [j - 1];

                if (t->s)
                    r += t->physical - t->s->physical + t->s->origin;
                else
                    r += t->physical;

                fprintf (mfile, "  * %08lx %s %s\n",
                    r, d__getsb (gbuf, t->name), d__getsb (tbuf, p));
            }

            fclose (fp);
    }

    /* Converts to physical offset. */
    static unsigned long getPhys (unsigned long offset, section_t *p)
    {
            return offset - p->origin + p->physical;
    }

    /* Fixes the fixups of the VO file. */
    static void fixFixups (vo_t *vo, int format, unsigned long sbx_base)
    {
        unsigned long lu, lv, lp, boffs, borig, offset, fixoffs, count;
        section_t *s, *p, *q; group_t *group;
        FILE *fp; char *err = NULL;
        unsigned u, v, relative;
        list_t *temp; int_t *t;
        int i;

            if (!vo->fixups) return;

            fp = fopen (vo->path, "rb");
            if (!fp)
            {
                pmsg ("$unable to open input file: %s", vo->path);
                return;
            }

            fseek (fp, vo->base + vo->fixups + 4, SEEK_SET);
            count = lgetd (fp);

            temp = new (list_t);

            while (count--)
            {
                lp = ftell (fp);

                i = getc (fp);
                s = vo->sectionL [getc (fp) - 1];

                switch ((i >> 6) & 3)
                {
                    case 0:
                        fixoffs = getc (fp);
                        break;

                    case 1:
                        fixoffs = lgetw (fp);
                        break;

                    case 2:
                        fixoffs = lgetw (fp);
                        fixoffs |= ((unsigned long)getc (fp)) << 16;
                        break;

                    case 3:
                        fixoffs = lgetd (fp);
                        break;
                }

                offset = 0;
                p = NULL;

                switch ((i >> 3) & 3)
                {
                    case 0: /* Section (Byte) */
                        p = vo->sectionL [getc (fp) - 1];
                        break;

                    case 1: /* Group (Byte) */
                        group = findGroup (vo->groupL [getc (fp) - 1]);
                        if (group) p = group->base;
                        break;

                    case 2: /* External Symbol (Byte) */
                        u = getc (fp) - 1;
                        goto go;

                    case 3: /* External Symbol (Word) */
                        u = lgetw (fp) - 1;

                    go: offset = dict__dsrch (vo->indexMap, &u, 2);
                        if (offset)
                        {
                            lu = dict__dsrch (externals, &offset, 4);
                            if (!externals->lastSrch) break;

                            lu = d__lgetd (lu + 4);
                            if (!lu)
                            {
                                findPublic (offset, &p, &offset);
                            }
                            else
                            {
                                if (probeFirst) findPublic (offset, &p, &offset);

                                if (!p)
                                {
                                    vo->sfixupC++;
                                    goto addFixup;
                                }
                            }
                        }

                        break;
                }

                if (!p)
                {
                    for (t = TOP(temp); t; t = NEXT(t))
                        if (t->value == offset) break;

                    if (t) continue;

                    t = new (int_t);
                    t->value = offset;

                    list__addt (temp, t);

                    d__getsb (gbuf, offset);
                    pmsg ("#undefined symbol %s in module %s", gbuf, vo->source);

                    continue;

       addSecFixup: vo->rfixupC++;

          addFixup: dict__dadd (vo->dfixups, &vo->fixupC, 4, lp);
                    vo->fixupC++;
                    continue;
                }

                fixoffs = fixoffs - s->origin + s->physical;

                q = p->s ? p->s : p;

                boffs = q->physical;
                borig = q->origin;

                offset += -p->origin + p->physical;

                if (i & 0x20)
                    offset += -s->physical + s->origin, relative = 1;
                else
                    offset += -boffs + borig + base, relative = 0;

                if (format == F__VX && !relative)
                    offset += boffs - borig;

                switch (i & 7)
                {
                    case 0: /* 16-bit Offset */
                        if ((format == F__CFX || format == F__VX) && !relative)
                        {
                            err = "format doesn't allow 16-bit absolute offset fixups";
                            break;
                        }

                        fseek (image, fixoffs, SEEK_SET);
                        u = lgetw (image) + offset;

                        fseek (image, fixoffs, SEEK_SET);
                        lputw (u, image);

                        break;

                    case 1: /* 16-bit Base */
                        if (format == F__VX)
                        {
                            err = "vx format doesn't allow 16-bit section fixups";
                            break;
                        }

                        if (format == F__CFX)
                        {
                            fseek (image, fixoffs, SEEK_SET);
                            lputw (q->flags & F__CODE ? 0x08 : 0x10, image);
                            break;
                        }

                        if (format == F__SBX)
                        {
                            fseek (image, fixoffs, SEEK_SET);
                            u = lgetw (image) + sbx_base + (boffs >> 4);

                            fseek (image, fixoffs, SEEK_SET);
                            lputw (u, image);
                        }
                        else
                            goto addSecFixup;

                        break;

                    case 2: /* 32-bit Offset */
                        fseek (image, fixoffs, SEEK_SET);
                        lu = lgetd (image) + offset;

                        fseek (image, fixoffs, SEEK_SET);
                        lputd (lu, image);

                        if (format == F__CFX && !relative) goto addFixup;
                        break;

                    case 3: /* 32-bit Base */
                        if (format == F__CFX)
                        {
                            fseek (image, fixoffs, SEEK_SET);
                            lputd (q->flags & F__CODE ? 0x08 : 0x10, image);
                            break;
                        }

                        if (format == F__VX)
                        {
                            fseek (image, fixoffs, SEEK_SET);
                            lputd (q->physical + q->origin + base, image);
                            break;
                        }

                        if (format == F__SBX)
                        {
                            fseek (image, fixoffs, SEEK_SET);
                            lu = lgetd (image) + sbx_base + (boffs >> 4);

                            fseek (image, fixoffs, SEEK_SET);
                            lputd (lu, image);
                        }
                        else
                            goto addSecFixup;

                        break;

                    case 4: /* 32-bit Pointer */
                        if (format == F__VX)
                        {
                      nptr: err = "vx format doesn't allow pointer fixups";
                            break;
                        }

                        if (format == F__CFX)
                        {
                            fseek (image, fixoffs, SEEK_SET);
                            v = lgetw (image) + offset;

                            fseek (image, fixoffs, SEEK_SET);
                            lputw (v, image);
                            lputw (q->flags & F__CODE ? 0x08 : 0x10, image);

                            goto addSecFixup;
                        }

                        if (format == F__SBX)
                        {
                            fseek (image, fixoffs, SEEK_SET);
                            v = lgetw (image) + offset;
                            u = lgetw (image) + sbx_base + (boffs >> 4);

                            fseek (image, fixoffs, SEEK_SET);
                            lputw (v, image);
                            lputw (u, image);
                        }
                        else
                            goto addSecFixup;

                        break;

                    case 5: /* 48-bit Pointer */
                        if (format == F__VX) goto nptr;

                        if (format == F__CFX)
                        {
                            fseek (image, fixoffs, SEEK_SET);
                            v = lgetd (image) + offset;

                            fseek (image, fixoffs, SEEK_SET);
                            lputd (v, image);
                            lputw (q->flags & F__CODE ? 0x08 : 0x10, image);

                            goto addSecFixup;
                        }

                        if (format == F__SBX)
                        {
                            fseek (image, fixoffs, SEEK_SET);
                            lu = lgetd (image) + offset;
                            u = lgetw (image) + sbx_base + (boffs >> 4);

                            fseek (image, fixoffs, SEEK_SET);
                            lputd (lu, image);
                            lputw (u, image);
                        }
                        else
                            goto addSecFixup;

                        break;

                    case 6: /* 64-bit Pointer */
                        if (format == F__VX) goto nptr;

                        if (format == F__CFX)
                        {
                            fseek (image, fixoffs, SEEK_SET);
                            v = lgetd (image) + offset;

                            fseek (image, fixoffs, SEEK_SET);
                            lputd (v, image);
                            lputd (q->flags & F__CODE ? 0x08 : 0x10, image);

                            goto addSecFixup;
                        }

                        if (format == F__SBX)
                        {
                            fseek (image, fixoffs, SEEK_SET);
                            lu = lgetd (image) + offset;
                            lv = lgetd (image) + sbx_base + (boffs >> 4);

                            fseek (image, fixoffs, SEEK_SET);
                            lputd (lu, image);
                            lputd (lv, image);
                        }
                        else
                            goto addSecFixup;

                        break;
                }
            }

            if (err) pmsg ("*%s", err);

            fclose (fp);

            while ((t = list__extract (temp)) != NULL) delete (t);
            delete (temp);
    }

    /* Converts a generic fixup to an specific eXec fixup. */
    static int gfixToXFix (unsigned long d, FILE *__fp, vo_t *__vo, void *c)
    {
        static void (*conv) (unsigned, int, unsigned long, unsigned long, unsigned long, vo_t *);
        static FILE *fp;
        static vo_t *vo;

        unsigned long lu, lv, lp, boffs, borig, offset, fixoffs, count;
        section_t *s, *p; group_t *group;
        unsigned u, v;
        int i;

            if (!d)
            {
                conv = (void *)c;

                fp = __fp;
                vo = __vo;

                return 0;
            }

            fseek (fp, d, SEEK_SET);

            i = getc (fp);
            s = vo->sectionL [getc (fp) - 1];

            switch ((i >> 6) & 3)
            {
                case 0:
                    fixoffs = getc (fp);
                    break;

                case 1:
                    fixoffs = lgetw (fp);
                    break;

                case 2:
                    fixoffs = lgetw (fp);
                    fixoffs |= ((unsigned long)getc (fp)) << 16;
                    break;

                case 3:
                    fixoffs = lgetd (fp);
                    break;
            }

            offset = 0;
            p = NULL;

            switch ((i >> 3) & 3)
            {
                case 0: /* Section (Byte) */
                    p = vo->sectionL [getc (fp) - 1];
                    break;

                case 1: /* Group (Byte) */
                    group = findGroup (vo->groupL [getc (fp) - 1]);
                    if (group) p = group->base;
                    break;

                case 2: /* External Symbol (Byte) */
                    u = getc (fp) - 1;
                    break;

                case 3: /* External Symbol (Word) */
                    u = lgetw (fp) - 1;
                    break;
            }

            fixoffs = fixoffs - s->origin + s->physical;

            if (p)
            {
                boffs = p->s ? p->s->physical : p->physical;
                borig = p->s ? p->s->origin : p->origin;
            }

            if (p)
            {
                offset += -p->origin + p->physical;

                if (i & 0x20)
                    offset += -s->physical + s->origin;
                else
                    offset += -boffs + borig + base;

                u = 0;
            }
            else
                u++;

            conv (u, i, fixoffs, offset, boffs, vo);

            return 0;
    }

    /* Converts generic dfixups to specific runtime fixups. */
    static void makeRuntimeFixups (void *conv)
    {
        FILE *fp; vo_t *vo;

            for (vo = TOP(volist); vo; vo = NEXT(vo))
            {
                if (!vo->fixupC) continue;

                fp = fopen (vo->path, "rb");
                if (!fp)
                {
                    pmsg ("$unable to open input file: %s", vo->path);
                    continue;
                }

                gfixToXFix (0, fp, vo, conv);

                dict__dwalk (vo->dfixups, gfixToXFix);

                fclose (fp);
            }
    }

    /* Maker of EXE fixups. */
    static void exeM (unsigned sym, int i, unsigned long fixoffs,
                      unsigned long offset, unsigned long base)
    {
        unsigned long v;

            switch (i & 7)
            {
                case 1: /* 16-bit Base */
                    fseek (image, fixoffs, SEEK_SET);
                    lputw (base >> 4, image);
                    break;

                case 3: /* 32-bit Base */
                    fseek (image, fixoffs, SEEK_SET);
                    lputd (base >> 4, image);
                    break;

                case 4: /* 32-bit Pointer */
                    fseek (image, fixoffs, SEEK_SET);
                    v = lgetw (image) + offset;

                    fseek (image, fixoffs, SEEK_SET);
                    lputw (v, image);
                    lputw (base >> 4, image);

                    fixoffs += 2;
                    break;

                case 5: /* 48-bit Pointer */
                    fseek (image, fixoffs, SEEK_SET);
                    v = lgetd (image) + offset;

                    fseek (image, fixoffs, SEEK_SET);
                    lputd (v, image);
                    lputw (base >> 4, image);

                    fixoffs += 4;
                    break;

                case 6: /* 64-bit Pointer */
                    fseek (image, fixoffs, SEEK_SET);
                    v = lgetd (image) + offset;

                    fseek (image, fixoffs, SEEK_SET);
                    lputd (v, image);
                    lputd (base >> 4, image);

                    fixoffs += 4;
                    break;
            }

            lputd (fixoffs, output);
    }

    /* Maker of CFX fixups. */
    static void cfxM (unsigned sym, int i, unsigned long fixoffs)
    {
            lputd (fixoffs, output);
    }

    /* Returns count of bytes needed to store x, minus one. */
    static int value_len (unsigned long x)
    {
            if (x < 256) return 0;
            if (x < 65536) return 1;
            if (x < 16777216) return 2;

            return 3;
    }

    /* Maker of VX fixups. */
    static void vxM (unsigned sym, int i, unsigned long fixoffs,
                     unsigned long offset, unsigned long base,
                     vo_t *vo)
    {
            sym--;
            offset = dict__dsrch (vo->indexMap, &sym, 2);
            offset = dict__dsrch (externals, &offset, 4);
            offset = 1 + d__lgetw (offset + 8);

            i = (value_len (fixoffs) << 6) | (offset < 256 ? 0 : 0x20);
            if (offset < 32) i |= offset;

            putc (i, output);

            switch ((i >> 6) & 3)
            {
                case 0x00:
                    putc (fixoffs, output);
                    break;

                case 0x01:
                    lputw (fixoffs, output);
                    break;

                case 0x03:
                    lputw (fixoffs, output);
                    putc (fixoffs >> 16, output);
                    break;

                case 0x04:
                    lputd (fixoffs, output);
                    break;
            }

            if (!(i & 15))
            {
                if (i & 0x20)
                    lputw (offset, output);
                else
                    putc (offset, output);
            }
    }

    /* Walker to write exported symbols to the VX file. */
    static int writeExport (unsigned long offs, section_t *sbase)
    {
        static section_t *base;
        section_t *s;

            if (!offs)
            {
                base = sbase;
                return 0;
            }

            s = (section_t *)d__lgetd (offs);
            if (s == base || s->s == base)
            {
                if (!s->export) return 0;

                d__getsb (gbuf, d__lgetd (offs + 8));

                offs = d__lgetd (offs + 4);
                offs = offs - s->origin + s->physical - base->physical;

                lputd (offs, output);
                putsb (gbuf, output);
            }

            return 0;
    }

    /* Walker to write imported symbols to the VX file. */
    static int writeImport (unsigned long offs, section_t *sbase)
    {
        static unsigned globalIndex = 0;
        static section_t *base;
        section_t *s;

            if (!offs)
            {
                base = sbase;
                return 0;
            }

            s = (section_t *)d__lgetd (offs + 4);
            if (s == base || s->s == base)
            {
                if (!s->import) return 0;

                d__getsb (gbuf, d__lgetd (offs));
                putsb (gbuf, output);

                d__lputw (globalIndex++, offs + 8);
            }

            return 0;
    }

    /* Builds the executable file. */
    static void buildExecutable (int format, char *out, unsigned long origin,
        unsigned long sbx_base, unsigned long sfixupC, unsigned long rfixupC,
        unsigned long fixupC, section_t *stack, list_t *l0, long e)
    {
        unsigned long i, j, t1, t2, t3, t4, len, cur, entryoffs;
        char *expname, *impname;
        seqnode_t *n, *m;
        FILE *fp;

            len = fileLength (image);

            output = fp = fopen (out, "wb");
            if (!fp)
            {
                pmsg ("$unable to open output file: %s", out);
                return;
            }

            if (format == F__BIN || format == F__SBX) goto G__BIN;
            if (format == F__EXE) goto G__EXE;
            if (format == F__CFX) goto G__CFX;

G__VX:
            if (entry)
                entryoffs = getPhys (entry->offset, entry->section);
            else
                entryoffs = 0;

            lputd (0x78767372, fp);
            lputd (0, fp);
            lputd (len, fp);
            lputd (0, fp);
            lputd (fixupC, fp);
            lputd (entryoffs, fp);
            lputd (0, fp);

            for (n = TOP(l0), i = 0; n; n = NEXT(n))
                if (!n->s->s) i++;

            putc (i, fp);

            t3 = t4 = 0;

            for (n = TOP(l0); n; n = m)
            {
                m = NEXT(n);
                if (n->s->s) continue;

                expname = impname = NULL;
                t1 = t2 = i = 0;

                if (n->s->import) i |= 1, impname = n->s->import;
                if (n->s->export) i |= 2, expname = n->s->export;

                t1 += n->s->publicC;
                t2 += n->s->externC;

                while (m && m->s->s)
                {
                    if (m->s->import) i |= 1;
                    if (m->s->export) i |= 2;

                    if (!m->s->import && !m->s->export)
                    {
                        m = NEXT(m);
                        continue;
                    }

                    t1 += m->s->publicC;
                    t2 += m->s->externC;

                    if (i == 3)
                    {
                        pmsg ("*%s: section %s of %s produced exp/imp conflict",
                            n->s->vo->source, d__getsb (gbuf, n->s->name),
                            m->s->vo->source);

                        break;
                    }

                    if (i == 1)
                    {
                        if (impname)
                        {
                            if (strcmp (impname, m->s->import))
                            {
                                pmsg ("*%s: section %s (%s) imports from different source",
                                    n->s->vo->source, d__getsb (gbuf, n->s->name),
                                    m->s->vo->source);

                                i = 3;
                                break;
                            }
                        }
                        else
                            impname = m->s->import;
                    }

                    if (i == 2)
                    {
                        if (expname)
                        {
                            if (strcmp (expname, m->s->export))
                            {
                                pmsg ("*%s: section %s (%s) exports under different name",
                                    n->s->vo->source, d__getsb (gbuf, n->s->name),
                                    m->s->vo->source);

                                i = 3;
                                break;
                            }
                        }
                        else
                            expname = m->s->export;
                    }

                    m = NEXT(m);
                }

                if (i == 3)
                {
               Bad: fclose (fp);
                    remove (out);
                    return;
                }

                if (i == 0x02) i = 0x80;
                if (i == 0x01) i = 0xC0;

                lputd (sectionGroupLen (n->s, l0), fp);
                putc (i, fp);

                d__getsb (gbuf, n->s->name);
                putsb (gbuf, fp);

                t3 += t1;

                if (t3 > 65535)
                {
                    pmsg ("$%s: too many symbol %sports defined globally", n->s->vo->source, "ex");
                    goto Bad;
                }

                t4 += t2;

                if (t4 > 65535)
                {
                    pmsg ("$%s: too many symbol %sports defined globally", n->s->vo->source, "im");
                    goto Bad;
                }

                if (i == 0xC0)
                {
                    if (t2 > 65535)
                    {
                        pmsg ("*%s: too many symbol %sports in section %s",
                            n->s->vo->source, "im", d__getsb (gbuf, n->s->name));

                        goto Bad;
                    }

                    lputw (t2, fp);
                    putsb (impname, fp);

                    writeImport (0, n->s);

                    dict__dwalk (externals, writeImport);
                    continue;
                }

                if (i == 0x80)
                {
                    if (t1 > 65535)
                    {
                        pmsg ("*%s: too many symbol %sports in section %s",
                            n->s->vo->source, "ex", d__getsb (gbuf, n->s->name));

                        goto Bad;
                    }

                    lputw (t1, fp);
                    putsb (expname, fp);

                    writeExport (0, n->s);

                    dict__dwalk (publics, writeExport);
                    continue;
                }
            }

            t1 = ftell (fp);

            fseek (fp, 0x0C, SEEK_SET);
            lputd (t1, fp);

            fseek (fp, 0x18, SEEK_SET);
            lputw (t3, fp);
            lputw (t4, fp);

            fseek (fp, t1, SEEK_SET);

            makeRuntimeFixups ((void *)&vxM);

            t1 = ftell (fp);
            fseek (fp, 0x04, SEEK_SET);
            lputd (t1, fp);
            fseek (fp, t1, SEEK_SET);

            goto Attach;

G__CFX:
            lputw ('CF', fp);
            lputw ('X!', fp);

            lputd (len, fp);
            lputd (fixupC, fp);

            if (stack)
            {
                lputd (stack->physical, fp);
                lputd (sectionGroupLen (stack, l0), fp);
            }
            else
            {
                pmsg ("#executable has no stack");

                lputd (0, fp);
                lputd (0, fp);
            }

            if (entry)
                entryoffs = getPhys (entry->offset, entry->section);
            else
                entryoffs = 0;

            i = ((j = 32 + 4*fixupC) + 4095) & -4096L;

            lputd (entryoffs, fp);
            lputd (i, fp);
            lputd (0, fp);

            makeRuntimeFixups ((void *)&cfxM);

            i -= j;
            while (i--) putc (0x90, fp);

            goto Attach;

G__EXE:
            lputw ('MZ', fp);

            i = (len + 511) & -512;
            lputw ((i - len) & 511, fp);
            lputw (i >> 9, fp);

            lputw (rfixupC, fp);
            lputw (((28 + 4*rfixupC + 15) & -16) >> 4, fp);

            lputw (i >> 4, fp);
            lputw (0xFFFF, fp);

            if (stack)
            {
                lputw (stack->physical >> 4, fp);
                lputw (sectionGroupLen (stack, l0) - 1, fp);
            }
            else
            {
                pmsg ("#executable has no stack");

                lputw (0, fp);
                lputw (0, fp);
            }

            lputw (0, fp);

            if (entry)
                entryoffs = getPhys (entry->offset, entry->section);
            else
                entryoffs = 0;

            lputw (entryoffs & 0xF, fp);
            lputw (entryoffs >> 4, fp);

            lputw (0x1C, fp);
            lputw (0, fp);

            makeRuntimeFixups ((void *)&exeM);

            i = ftell (fp);
            j = ((i + 15) & -16) - i;

            while (j--) putc (0, fp);

Attach:
            e = e - (len + ftell (fp));
            if (e < 0) e = 0;

            rewind (image);

            while (len)
            {
                if (len > sizeof (gbuf))
                    fread (gbuf, 1, cur = sizeof (gbuf), image);
                else
                    fread (gbuf, 1, cur = len, image);

                fwrite (gbuf, 1, cur, fp);

                len -= cur;
            }

            while (e--) putc (0x90, fp);

            fclose (fp);

            pmsg ("program entry point is at offset %08lx", entryoffs);
            return;

G__BIN:
            e = e - (len + ftell (fp));
            if (e < 0) e = 0;

            rewind (image);

            while (len)
            {
                if (len > sizeof (gbuf))
                    fread (gbuf, 1, cur = sizeof (gbuf), image);
                else
                    fread (gbuf, 1, cur = len, image);

                fwrite (gbuf, 1, cur, fp);

                len -= cur;
            }

            while (e--) putc (0x90, fp);

            fclose (fp);
    }

    /* Entry point. */
    int main (int argc, char *argv [])
    {
        unsigned long t, rfC, sfC, fC, sbx_base, origin, mem = coreleft ();
        int map_publics, format, sec_align, mkgrp, i, j, k, index = 1;
        char *firstvo, *exefilename, *mapfilename, *libname;
        char *s, *p, *arg [9], show = 0; long e = 0;
        list_t *l0; vo_t *vo; group_t *g; FILE *fp;
        seqnode_t *n, *m; section_t *stack = NULL;

        static char *sign1 =
            "RedStar Vanessa Version 0.03 Copyright (C) 2008-2014 "
            "ciachn@gmail.com\n";

        static char *sign2 =
            "RedStar Vanessa Version 0.03";

        static unsigned alignValue [] = { 1, 2, 4, 16, 256, 4096 };
        static char sup [] = { 1, 0, 3, 1, 1 };

            printf (sign1);

            if (argc < 2)
            {
                printf ("Syntax: vl [options] vo-files%-24s* Build Date: %s\n"
 "\nOptions:\n"
//---*---------*---------------------------*---------*
 "   -f xxx    Select executable format    -A dval   Force section alignment\n"
 "   -m xxx    Set output map filename     -T hval   Set SBX target segment\n"
 "   -p        Include publics in map      -O hval   Executable origin offset\n"
 "   -o xxx    Set output filename         -L xxx    Create or update library\n"
 "   -G        Group all sections          -B hval   Set fixups base address\n"
 "   -s        Force to show progress      -P        Probe imports resolution\n"
 "   -F        Enable On-memory buffers    -S dval   Set min executable size\n" 

 "\nSupported executable formats:\n"
//---*---------*---------------------------*---------*
 "   sbx       Static Binary Executable    bin       Generic binary executable\n"
 "   vx        Vanessa eXecutable          exe       DOS executable\n"
 "   cfx       Cynthia Flat Executable\n"

                        "\n", "", __DATE__);

                return 1;
            }

            dictfile = fopen (s = (char *)createfile (SYM_FILE), "r+b");
            if (!dictfile)
            {
                pmsg ("$unable to open temporal file: %s\n\n", s);
                return 2;
            }

            entry = NULL;
            bottom = 4;

            sections = new (list_t);
            volist = new (list_t);
            groups = new (list_t);
            libs = new (list_t);

            symbols = new (dict_t);
            symbols->maxHeight = 128;

            publics = new (dict_t);
            publics->maxHeight = 4;

            externals = new (dict_t);
            externals->maxHeight = 4;

            exefilename = mapfilename = firstvo = NULL;

            map_publics = origin = base = onmem = 0;
            sbx_base = probeFirst = 0;
            mkgrp = mklib = 0;
            format = F__BIN;
            sec_align = 1;

            image = mfile = NULL;

            /* Interpret command line. */
            while (index < argc)
            {
                s = argv [index++];
                if (*s != '-')
                {
                    if (*s == '@')
                    {
                        fp = fopen (s + 1, "rt");
                        if (!fp)
                        {
                            pmsg ("#unable to open input file: %s", s + 1);
                            continue;
                        }

                        while ((s = getline (gbuf, sizeof (gbuf), fp)) != NULL)
                        {
                            s = rtrim (ltrim (s));

                            if (*s == '\0' || *s == '#' || (*s == '/' && s [1] == '/'))
                                continue;

                            if (!firstvo) firstvo = dups (s);

                            addextension (s, tbuf, "vo");

                            if (show) pmsg ("loading %s...", tbuf);
                            loadvo (NULL, 0, tbuf);
                        }

                        fclose (fp);

                        continue;
                    }

                    if (!firstvo) firstvo = s;

                    addextension (s, tbuf, "vo");

                    if (show) pmsg ("loading %s...", tbuf);
                    loadvo (NULL, 0, tbuf);

                    continue;
                }

                if (!(p = strchr (optStr, s [1])))
                {
                    pmsg ("*incorrect option: -%c", s [1]);
                    continue;
                }

                i = (unsigned)p - (unsigned)optStr;
                j = argCnt [i] - '0';
                k = 0;

                if (j && s [2] != '\0') arg [k++] = s + 2;

                while (k < j && index < argc)
                    arg [k++] = argv [index++];

                if (k != j)
                    pmsg ("*option -%c needs %u argument%s!!", s [1], j, j > 1 ? "s": "");

                switch (i)
                {
                    case 0x00: /* -f xxx */
                        i = findstr (arg [0], formats, arrayLen (formats));

                        if (i == -1)
                            pmsg ("*incorrect executable format specified (%s)", arg [0]);
                        else
                            format = i;

                        break;

                    case 0x01: /* -A dval */
                        i = j = atov (arg [0], R__DEC);

                        if (!i)
                        {
                            pmsg ("*what the hell are you thinking!?");
                            break;
                        }

                        if (i < 1)
                        {
                            pmsg ("*how do you align to negative boundaries?");
                            break;
                        }

                        if (i & 1 && i != 1)
                        {
                            pmsg ("*cannot align sections to odd-boundaries!!");
                            break;
                        }

                        while (j)
                        {
                            k = j >> 1;
                            if (j & 1) break;

                            j = k;
                        }

                        if (k)
                            pmsg ("*log2 of value must be an integer (AKA be in base two)");
                        else
                            sec_align = i;

                        break;

                    case 0x02: /* -m xxx */
                        mapfilename = arg [0];
                        break;

                    case 0x03: /* -P */
                        probeFirst = 1;
                        break;

                    case 0x04: /* -p */
                        map_publics = 1;
                        break;

                    case 0x05: /* -T hval */
                        sbx_base = atov (arg [0], R__HEX);
                        break;

                    case 0x06: /* -o xxx */
                        exefilename = arg [0];
                        break;

                    case 0x07: /* -O xxx */
                        origin = atov (arg [0], R__HEX);
                        break;

                    case 0x08: /* -G */
                        mkgrp = 1;
                        break;

                    case 0x09: /* -L xxx */
                        if (mklib) break;

                        mklib = 1;
                        libname = arg [0];

                        fclose (dictfile);

                        if (libname)
                            libname = dups (addextension (libname, tbuf, "lib"));

                        dictfile = fopen (libname, "r+b");
                        if (!dictfile)
                        {
                            dictfile = fopen (libname, "w+b");
                            if (!dictfile)
                            {
                                pmsg ("$unable to open output file: %s", libname);
                                goto ret;
                            }

                            putc (0x90, dictfile);
                            putsb (sign2, dictfile);

                            t = ftell (dictfile);
                            lputd (0, dictfile);

                            libdict = new (dict_t);
                            libdict->maxHeight = 128;
                        }
                        else
                        {
                            if (getc (dictfile) != 0x90)
                            {
                                pmsg ("*file '%s' is not a valid vanessa library file", libname);
                                goto ret;
                            }

                            getsb (gbuf, sizeof (gbuf), dictfile);

                            t = ftell (dictfile);

                            libdict = new (dict_t);
                            libdict->maxHeight = 128;
                            libdict->root = lgetd (dictfile);
                        }

                        bottom = fileLength (dictfile);
                        break;

                    case 0x0A: /* -s */
                        show = 1;
                        break;

                    case 0x0B: /* -B hval */
                        base = atov (arg [0], R__HEX);
                        break;

                    case 0x0C: /* -F */
                        onmem = 1;
                        break;

                    case 0x0D: /* -S dval */
                        e = atov (arg [0], R__DEC);

                        if (e < 1)
                        {
                            pmsg ("*executable size must be a positive integer!");
                            e = 0;
                        }

                        break;
                }
            }

            if (mklib)
            {
                if (!errc)
                {
                    fseek (dictfile, t, SEEK_SET);
                    lputd (libdict->root, dictfile);
                }

                pmsg ("%s successfully created (%lu bytes)",
                    libname, fileLength (dictfile));

                goto ret;
            }

            if (!COUNT(volist) && !errc) pmsg ("*no object files were specified!");
            if (errc) goto done;

            if (format == F__CFX) mkgrp = 1;

            if (format == F__VX)
            {
                if (base)
                    pmsg ("#vx format overrided the given executable base");

                base = 0x80000000UL;
            }

            /* Build filenames. */
            if (map_publics && !mapfilename)
                mapfilename = dups (setextension (firstvo, tbuf, "map"));
            else
            if (mapfilename)
                mapfilename = dups (addextension (mapfilename, tbuf, "map"));

            if (exefilename)
                exefilename = dups (addextension (exefilename, tbuf, formats [format]));
            else
                exefilename = dups (setextension (firstvo, tbuf, formats [format]));

            if ((format == F__SBX || format == F__EXE) && sec_align < 16)
                sec_align = 16;

            if (show) pmsg ("resolving externals...");

            /* Resolve all externals and import VOs if necessary. */
            resolveExternals ();
            if (errc) goto done;

            /* Re-group sections. */
            regroupSections ();

            /* Build Section Sequence */
            l0 = buildSectionSequence ();

            /* Group all sections if required (CFX format or option -G). */
            if (mkgrp)
            {
                m = TOP(l0);

                m->s->flags |= F__MASTER;
                m->s->s = NULL;

                for (n = NEXT(m); n; n = NEXT(n))
                {
                    n->s->flags &= ~F__MASTER;
                    n->s->s = m->s;
                }
            }

            /* Open temporal image file. */
            image = fopen (s = (char *)createfile (IMAGE_FILE), "r+b");
            if (!image)
            {
                pmsg ("$unable to open temporal file: %s", s);
                goto done;
            }

            /* Prepare map file. */
            if (mapfilename)
            {
                mfile = fopen (mapfilename, "wt");
                if (mfile)
                {
                    fprintf (mfile, sign1);

                    fprintf (mfile, "\n----- Sections -----\n");
                }
                else
                    pmsg ("#unable to open output map file: %s", mapfilename);
            }

            if (show) pmsg ("building binary image...");

            /* Build binary image file. */
            n = TOP(l0);

            if (origin)
            {
                if (n->s->origin < origin)
                {
                    pmsg ("*data or code below origin is not allowed");
                    goto done;
                }

                i = alignValue [(n->s->attributes >> 5) & 7];

                n->s->origin -= origin;
                writeSectionImage (max (sec_align, i), n->s, l0);
                n->s->origin += origin;

                n = NEXT(n);
            }

            for (; n; n = NEXT(n))
            {
                if (!stack && n->s->flags & F__STACK)
                    stack = n->s;

                i = alignValue [(n->s->attributes >> 5) & 7];

                writeSectionImage (max (sec_align, i), n->s, l0);
            }

            if (errc) goto done;

            /* Write the sections to the map file. */
            if (mfile)
            {
                for (n = TOP(l0); n; n = NEXT(n))
                {
                    if (!n->s->s)
                    {
                        t = sectionGroupLen (n->s, l0);

                        fprintf (mfile, "\nSection %s : Class '%s', Base %08lx, Length %lu\n",
                                d__getsb (tbuf, n->s->name),
                                d__getsb (gbuf, n->s->class),
                                n->s->physical, t);
                    }

                    fprintf (mfile, "  * %-16s %08lx %08lx %s '%s'",
                                    n->s->vo->source, n->s->physical, n->s->length,
                                    d__getsb (gbuf, n->s->name), d__getsb (tbuf, n->s->class));

                    if (n->s->import)
                        fprintf (mfile, " (Imp \"%s\")", n->s->import);

                    if (n->s->export)
                        fprintf (mfile, " (Exp \"%s\")", n->s->export);

                    fprintf (mfile, "\n");
                }
            }

            /* Set base section of each group. */
            if (TOP(groups) && mfile)
                fprintf (mfile, "\n----- Groups -----\n");

            for (g = TOP(groups); g; g = NEXT(g))
            {
                for (n = TOP(l0); n; n = NEXT(n))
                    if (n->s->group == g->name) break;

                g->base = n->s;

                if (!mfile) continue;

                fprintf (mfile, "\nGroup %s Starts at %08lx\n",
                    d__getsb (tbuf, g->name), g->base->physical);

                do
                {
                    if (n->s->group != g->name) continue;

                    fprintf (mfile, "  * %-16s %08lx %08lx %s '%s'\n",
                                    n->s->vo->source, n->s->physical,
                                    n->s->length, d__getsb (gbuf, n->s->name),
                                    d__getsb (tbuf, n->s->class));
                }
                while ((n = NEXT(n)) != NULL);
            }

            /* Fix-up!! */
            for (rfC = sfC = fC = 0, vo = TOP(volist); vo; vo = NEXT(vo))
            {
                fixFixups (vo, format, sbx_base);

                rfC += vo->rfixupC;
                sfC += vo->sfixupC;
                fC += vo->fixupC;
            }

            if (rfC && !(sup [format] & 1))
                pmsg ("*%lu section fixup%s found (needs non bin format)", rfC, rfC > 1 ? "s" : "");

            if (sfC && !(sup [format] & 2))
                pmsg ("*%lu symbol import%s found (requires vx format)", sfC, sfC > 1 ? "s" : "");

            if (errc) goto done;

            if (show) pmsg ("(last step) building executable...");

            /* Build executable file. */
            buildExecutable (format, exefilename, origin, sbx_base, sfC, rfC, fC, stack, l0, e);
            if (errc) goto done;

            /* Write publics to the mapfile. */
            if (map_publics && mfile)
            {
                fprintf (mfile, "\n----- Publics -----\n");

                for (vo = TOP(volist); vo; vo = NEXT(vo))
                    mapPublics (vo);
            }

            mem = mem - coreleft () + fileLength (dictfile);

            pmsg ("%lu bytes used in total (%lu bytes of memory left)",
                mem, coreleft ());

            goto ret;

      done: pmsg ("$unable to complete linking, %u error%s %s found",
                errc, errc > 1 ? "s" : "", errc > 1 ? "were" : "was");

       ret: pmsg ("finished.\n\n");

            if (image) fclose (image);

            if (mfile)
            {
                fclose (mfile);
                if (errc) remove (mapfilename);
            }

            if (dictfile) fclose (dictfile);

            remove (IMAGE_FILE);
            remove (SYM_FILE);

            return 0;
    }
