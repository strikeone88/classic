/*
    OZ.C

    RedStar Ozyara Assembler Version 1.07

    Copyright (C) 2007-2014 RedStar Technologies
    Written by J. Palencia (ciachn@gmail.com)
*/

    #include <gstring.h>
    #include <stdarg.h>
    #include <aurora.h>
    #include <alloc.h>
    #include <gfrac.h>
    #include <time.h>
    #include <rule.h>
    #include <pcc.h>

    #include "asm$h.h"

    /* Action macro to avoid writing the same so many times. */
    #define ACTION(x) void __##x (int form, int n, char *argv [], unsigned m [])

    /* Set to x if you want to enable debugging. */
    #define debug(x)        //x

    /* Count of bytes in a fraction part. */
    #define OzyaraByteCount 16

    /* Maximum depth level for macros. */
    #define MAX_DEPTH   16

    /* Maximum count of line-information entries. */
    #define MAX_LINFO       128
    #define MAX_LINFOMASK   127

    /* Searches for a symbol given its name. */
    #define symbol__search(x) (symbol_t *)dict__srch (&symbols, x)

    /* Helper function for the symbols dictionary. */
    #define symbol__add(p) dict__add (&symbols, p)

    /* Creates a file and closes it. */
    #define NewFile(p) fclose (fopen (p, "w+b"))

    /* Writes an element to the section file uses selected_data_type. */
    #define wData(n) writeElem (selected_data_type, n, csection->file)

    /* Writes a byte, word or dword to the section file. */
    #define putByte(n) writeElem (T__BYTE, n, csection->file)
    #define putWord(n) writeElem (T__WORD, n, csection->file)
    #define putDword(n) writeElem (T__DWORD, n, csection->file)

    /* The offset type. */
    #define dword_t     unsigned long

    /* Instruction information. */
    typedef struct /* direct cast: linkable_t, strn_t */
    {
        linkable_t  link;
        char        *name;

        unsigned    char *data;
        unsigned    long offs;
    }
    ins_t;

    /* Section information structure. */
    typedef struct /* direct cast: linkable_t */
    {
        linkable_t  link;

        char        *name;
        char        *klass;

        struct      group_s *group;

        int         align;
        int         combine;

        char        *temp_filename;

        FILE        *file;

        unsigned    n_index;
        unsigned    c_index;

        unsigned    char index;
        unsigned    char mode;

        char        *export;
        char        *import;

        unsigned    long origin;

        dword_t     o__bin;
    }
    section_t;

    /* Group information structure. */
    typedef struct group_s /* direct cast: linkable_t */
    {
        linkable_t  link;

        char        *name;

        unsigned    n_index;
        unsigned    char index;
    }
    group_t;

    /* Symbol information structure. */
    typedef struct /* direct cast: tstr_t */
    {
        linkable_t  link;
        char        *name;

        section_t   *section;
        dword_t     offset;

        char        pubext;
        char        type;
    }
    symbol_t;

    /* Variable information structure. */
    typedef struct /* direct cast: tstr_t */
    {
        linkable_t  link;
        char        *name;

        gFrac       Value;
        void        *Buf;

        char        type;
    }
    variable_t;

    /* VO Symbol information structure. */
    typedef struct /* direct cast: linkable_t */
    {
        linkable_t  link;

        char        *name;
        unsigned    index;
    }
    sym_t;

    /* Opened stream information (duplicated name and stream). */
    typedef struct /* direct cast: linkable_t */
    {
        linkable_t  link;

        char        *name;
        FILE        *fp;

        char        kill_name;
    }
    xstream_t;

    /* Integer constant information. */
    typedef struct /* direct cast: linkable_t */
    {
        linkable_t  link;

        symbol_t    *Symbol;
        char        Valid;

        int         Type;
        gFrac       Value;
    }
    IntConst;

    /* External pattern node. */
    typedef struct
    {
        linkable_t  link;
        char        *value;
        section_t   *s;
    }
    pattern_t;

    /* Dictionary of symbols, list of patterns and variables. */
    static list_t symbols, publicPatterns, externalPatterns, variables;

    /* List of sections, groups and VO symbols. */
    static list_t sections, groups, syms;

    /* List of opened streams. */
    static list_t xstreams;

    /* Ozyara's Instruction Set File. */
    static FILE *iset;

    /* Instruction set dictionary. */
    static list_t *isetdict;
    static int fast;

    /* Selected data type, temporal name, and current group. */
    static int selected_data_type, selected_data_len;
    static char temp_name [32];
    static group_t *cgroup;

    /* Temporal section attributes. */
    static int attr_combine, attr_align;
    static unsigned long attr_base;
    static char attr_class [32];

    /* Count of errors. */
    static unsigned errc;

    /* Global temporal buffer. */
    static char gbuf [512];

    /* More temporal buffers (chiz!). */
    static char tbuf [512], qbuf [512];

    /* Current input filename. */
    static char *fname;

    /* Current active section (and temp section var used by matchPattern). */
    static section_t *csection, *tsection;

    /* File of local fixups (to be fixed before outputing). */
    static FILE *ffile;

    /* File being assembled. */
    static FILE *cfile;

    /* Files of publics, externals, debug-data and fixups. */
    static FILE *f__pub, *f__ext, *f__dbg, *f__fix;

    /* Count of publics, externals, debug-data and fixups. */
    static unsigned long pubCount, extCount, dbgCount, fixCount;

    /* The string to be eaten by the scanner. */
    static char *s__string;

    /* Indicates if fixing expressions or just assembling. */
    static int fixing;

    /* Here's where the result of the expression is stored when fixing. */
    static unsigned long ExprResult;
    static IntConst ExprResultEx;

    /* Indicates if VO fixups should be skipped. */
    static int skipvofixups;

    /* Address of the next locationCounter (after the current instruction). */
    static dword_t nLocationCounter;

    /* Line number that the current buffer had. */
    static int p__linenum;

    /* Current VO file. */
    static FILE *vo;

    /* Count of temporal files and highest temporal file created. */
    static unsigned temp_files, hi_temp_file;

    /* Offsets to some fields of the VO file to fix. */
    static dword_t o__entry, o__sym, o__pub, o__ext, o__dbg, o__fix;

    /* Line information. */
    static unsigned long lineoffs [MAX_LINFO];
    static unsigned plinenum [MAX_LINFO];
    static unsigned cplinenum, freeline;

    /* Program entry point. */
    static char *entry;

    /* Converts a GFrac into a 32-bit floating point. */
    char *gFracToSingle (gFrac A);

    /* Converts a GFrac into a 64-bit floating point. */
    char *gFracToDouble (gFrac A);

    /* Converts a GFrac into an 80-bit floating point. */
    char *gFracToReal80 (gFrac A);

    /* Allocates a block on the file and returns the offset. [1] */
    unsigned long d__alloc (unsigned long size)
    {
            return 0;
    }

    /* Writes N bytes from buf to the offset. [1] */
    void d__write (void *buf, unsigned n, unsigned long offset)
    {
    }

    /* Reads N bytes from the given offset into buf. [1] */
    void d__read (void *buf, unsigned n, unsigned long offset)
    {
            fseek (iset, offset, SEEK_SET);
            fread (buf, n, 1, iset);
    }

    /* Opens a file and adds it to the opened streams list. */
    static FILE *xfopen (char *filename, char *mode, char kill_name, char mk)
    {
        xstream_t *xs;
        FILE *fp;

            if (mk) NewFile (filename);

            fp = fopen (filename, mode);
            if (fp == NULL) return NULL;

            xs = new (xstream_t);

            xs->kill_name = kill_name;
            xs->name = filename;
            xs->fp = fp;

            list__add (&xstreams, xs);

            return fp;
    }

    /* Closes all the files in the opened streams list. */
    static void xfcloseall (void)
    {
        xstream_t *xs;

            while ((xs = list__pop (&xstreams)) != NULL)
            {
                fclose (xs->fp);

                if (xs->kill_name) delete (xs->name);
                delete (xs);
            }
    }

    /* Adds a symbol to the VO symbol list. */
    static unsigned vo__addsym (char *value)
    {
        sym_t *sym = new (sym_t);

            sym->index = 1 + COUNT(&syms);
            sym->name = value;

            list__add (&syms, sym);

            return sym->index;
    }

    /* Searches a symbol on the VO symbol list, returns index. */
    static unsigned vo__srchsym (char *s)
    {
        sym_t *sym = gsearch (&syms, s);

            if (sym == NULL) return 0;

            return sym->index;
    }

    /* Returns the next line in the parsing list. */
    static char *getNextLine (int act, list_t *p)
    {
        static char buf [512];
        static strn_t *node;
        char *s;

            if (act == 0) /* Set List */
            {
                node = TOP(p);
                return NULL;
            }

            if (act == 1) /* Reset */
            {
                node = NULL;
                return NULL;
            }

            if (act == 2) /* Get Node */
            {
                return (char *)node;
            }

            if (act == 3) /* Set Node */
            {
                node = (strn_t *)p;
                return NULL;
            }

            if (act == 4) /* Get Buf */
                return buf;

            if (act == 5) /* Set Buf */
            {
                strcpy (buf, (void *)p);
                return NULL;
            }

            if (!node) return NULL;

            sprintf (buf, "%s\n", node->str);
            node = NEXT(node);

            return buf;
    }

    /* Returns a character from the input stream. */
    int s__getc (FILE *fp)
    {
        char *s;
        int c;

            if (fp)
            {
                if ((c = getc (fp)) == '\n')
                {
                    plinenum [freeline] = ++cplinenum;
                    lineoffs [freeline] = ftell (fp);

                    freeline = ++freeline & MAX_LINFOMASK;
                }

                return c;
            }

            if (!*s__string)
            {
                s = getNextLine (-1, NULL);
                if (!s) return EOF;

                s__string = s;
            }

            return *s__string++;
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
                case '*':   s = " **ERROR**";
                            errc++;
                            break;

                case '$':   s = " **FATAL**";
                            errc++;
                            break;

                case '#':   s = " **WARNING**";
                            break;

                default:    s = "";
                            fmts--;
            }

            if (flg) printf ("\n"); else flg = 1;

            if (fname)
                printf ("%s %u%s: ", fname, p__linenum ? p__linenum : linenum, s);
            else
                printf ("ozyara%s: ", s);

            vprintf (fmts, p);
    }

    /* Returns the path only without the filename. */
    static char *getpath (char *s)
    {
        char *p;

            s = strcpy (gbuf, s);

            for (p = s; *s != '\0'; s++) if (*s == '\\' || *s == '/') p = s;
            if (*p == '\\' || *p == '/') p++;

            *p = '\0';

            return gbuf;
    }

    /* Returns the filename only of the given path (w/o extension). */
    static char *getfilename (char *s)
    {
        char *p;

            for (p = s; *s != '\0'; s++) if (*s == '\\' || *s == '/') p = s;
            if (*p == '\\' || *p == '/') p++;

            strcpy (gbuf, p);

            for (p = gbuf; *p != '\0' && *p != '.'; p++);

            *p = '\0';

            return dups (gbuf);
    }

    /* Adds an extension to the file is none is given. */
    static char *add_ext (char *s, char *e)
    {
        char *p, *q = s;

            strcpy (gbuf, s);
            q = s = gbuf;

            for (p = s; *s != '\0'; s++) if (*s == '\\' || *s == '/') p = s;
            if (*p == '\\' || *p == '/') p++;

            if (strchr (p, '.')) return dups (q);

            return dups (strcat (gbuf, e));
    }

    /* Reads a string (leading-length format) from fp to dest. */
    static void rstr (FILE *fp, char *dest)
    {
        int i;

            fread (dest, 1, i = (unsigned char)getc (fp), fp);
            dest [i] = '\0';
    }

    /* Writes a string in leading-length format to fp. */
    static void wstr (const char *s, FILE *fp)
    {
        int i = strlen (s);

            if (i > 255) i = 255;

            putc (i, fp);
            while (i--) putc (*s++, fp);
    }

    /* Builds a serial temporal filename. */
    static char *build_tempfilename (void)
    {
            sprintf (gbuf, "oz$%05u.tmp", temp_files++);

            if (temp_files > hi_temp_file)
                hi_temp_file = temp_files;

            return dups (gbuf);
    }

    /* Removes the temporal files. */
    static void remove_temps (void)
    {
        unsigned i;

            for (i = 0; i < hi_temp_file; i++)
            {
                sprintf (gbuf, "oz$%05u.tmp", i);
                remove (gbuf);
            }
    }

    /* Writes a word to the given file. */
    static void putword (unsigned n, FILE *fp)
    {
            putc (n, fp);
            putc (n >> 8, fp);
    }

    /* Writes a dword to the given file. */
    static void putdword (dword_t n, FILE *fp)
    {
            putc (n, fp);
            putc (n >> 8, fp);
            putc (n >> 16, fp);
            putc (n >> 24, fp);
    }

    /* Reads a word from the file. */
    static unsigned getword (FILE *fp)
    {
        unsigned n;

            n = getc (fp);
            n |= ((unsigned)((unsigned char)getc (fp))) << 8;

            return n;
    }

    /* Reads a dword from the file. */
    static dword_t getdword (FILE *fp)
    {
        dword_t n;

            n = getc (fp);
            n |= ((dword_t)((unsigned char)getc (fp))) << 8;
            n |= ((dword_t)((unsigned char)getc (fp))) << 16;
            n |= ((dword_t)((unsigned char)getc (fp))) << 24;

            return n;
    }

    /* Parses the given line. */
    static int parseLine (unsigned linenum, char *buf)
    {
        int i;

            setinputstream (NULL);
            getNextLine (1, NULL);

            p__linenum = linenum;
            s__string = buf;

            errc = 0;

            i = parseinput () && !errc;
            p__linenum = 0;

            return i;
    }

    /* Parses the given list. */
    static int parseList (unsigned linenum, list_t *list)
    {
        int i;

            setinputstream (NULL);
            getNextLine (0, list);

            p__linenum = linenum - 1;
            s__string = "\x01";

            errc = 0;

            i = parseinput () && !errc;
            p__linenum = 0;

            return i;
    }

    unsigned long readsymbol ();

    /* Returns the code of a symbol. */
    static int symbolCode (char *s, unsigned *sindex)
    {
        static char buffer [128];
        void *p = s__savectx ();
        unsigned long c;

            strcpy (s__string = buffer, s);
            setinputstream (NULL);

            c = readsymbol ();

            *sindex = (unsigned)(c >> 16);

            s__loadctx (p);

            return c & 0xFFFF;
    }

    /* Returns the length of the file and rewinds. */
    static dword_t FileLength (FILE *fp)
    {
        dword_t len;

            fseek (fp, 0, SEEK_END);
            len = ftell (fp);
            rewind (fp);

            return len;
    }

    /* Writes a VO area from a file (f2). */
    static void addArea (FILE *f1, FILE *f2, unsigned long count)
    {
        int i;

            fseek (f1, 0, SEEK_END);
            fseek (f2, 0, SEEK_END);

            putdword (ftell (f2), f1);

            if (count != 0xFFFFFFFFUL) putdword (count, f1);

            rewind (f2);

            while (1)
            {
                i = fread (gbuf, 1, sizeof (gbuf), f2);
                if (!i) break;

                fwrite (gbuf, 1, i, f1);
            }
    }

    /* Sets the dword at offs of vo to val. */
    static void fixDword (dword_t offs, dword_t val)
    {
            fseek (vo, offs, SEEK_SET);
            putdword (val, vo);
    }

    /* Shows a message of 'undefined symbol'. */
    static void undefined (char *s)
    {
            pmsg ("*undefined symbol '%s'", s);
    }

    /* Shows a message of 'section not defined'. */
    static void secnotdef (char *s)
    {
            pmsg ("*section '%s' not defined", s);
    }

    /* Tries to fix all the local fixups (everything in ffile). */
    static int fixLocalFixups (void);

    /* Writes a new public symbol to the publics file. */
    static void write_public (char *v);

    /* Resets the internal core variables. */
    static void resetCore (void);

/****************************************************************************/

    int main (int argc, char *argv [])
    {
        section_t *x; sym_t *sym; symbol_t *t; ins_t *ins;
        dword_t temp = coreleft(), temp2;
        char *p, *q, *s, *vo_name;
        int i, index = 1;
        FILE *fp;

            fast = 1;

            printf ("RedStar Ozyara Version 1.07 Copyright (C) 2007-2014 ciachn@gmail.com\n");

            if (argc < 2)
            {
                printf ("Syntax: oz [options] source-asm-files%-15s* Build Date: %s\n"

                        "\n", "", __DATE__);
                return 1;
            }

            fname = NULL;
            errc = 0;

/*violet:handle options!!*/

            if (index >= argc)
            {
                pmsg ("*no source files were specified\n\n");
                return 1;
            }

            iset = fopen ("ozyara.dat", "rb");
            if (!iset)
            {
                strcat (getpath (argv [0]), "/ozyara.dat");

                iset = fopen (gbuf, "rb");
                if (!iset)
                {
                    pmsg ("*(fatal!) unable to locate ozyara.dat\n\n");
                    return 2;
                }
            }

            if (getword (iset) != 'IS' || getword (iset) != 'ET')
            {
                pmsg ("*(fatal!) invalid format of instruction set file\n\n");
                return 2;
            }

            if (getword (iset) != 0x0001)
            {
                pmsg ("*(fatal!) unsupported version of instruction set file\n\n");
                return 2;
            }

            isetdict = new (list_t);

            fseek (iset, getdword (iset), SEEK_SET);

            while ((i = getc (iset)) != 0)
            {
                ins = new (ins_t);
                ins->name = alloc (i + 1);

                fread (ins->name, i, 1, iset);
                ins->name [i] = '\0';

                ins->offs = getdword (iset);

                dict__add (isetdict, ins);
            }

            hi_temp_file = 0;

            s__initialize (0);

            while (index < argc)
            {
                resetCore ();

                iString ();
                iRule ();

                entry = NULL;

                cplinenum = 1;
                freeline = 1;

                lineoffs [0] = 0;
                plinenum [0] = 1;

                memset (plinenum, 0, sizeof (plinenum));

                s = add_ext (argv [index++], ".asm");

                fp = xfopen (s, "rt", 1, 0);
                if (fp == NULL)
                {
                    pmsg ("*unable to open input file '%s'", s);
                    goto done;
                }

                fixing = skipvofixups = 0;

                ffile = xfopen ("oz$lfix.tmp", "r+b", 0, 1);
                if (ffile == NULL)
                {
              cant: pmsg ("*unable to create required temporal file");
                    goto done;
                }

                pubCount = extCount = dbgCount = fixCount = 0;

                f__fix = xfopen ("oz$fix.tmp", "r+b", 0, 1);
                if (f__fix == NULL) goto cant;

                f__pub = xfopen ("oz$pub.tmp", "r+b", 0, 1);
                if (f__pub == NULL) goto cant;

                f__ext = xfopen ("oz$ext.tmp", "r+b", 0, 1);
                if (f__ext == NULL) goto cant;

                temp_files = 0;

                p = getfilename (s);
                q = add_ext (p, ".vo");
                delete (p);

                vo = xfopen (q, "r+b", 0, 1);

                putc (0x80, vo);
                putc (0x00, vo);
                putc (0x01, vo);

                wstr ("RedStar Ozyara Assembler Version 0.06", vo);

                setinputstream (cfile = fp);
                fname = s;

                i = parseinput ();

                fclose (fp);

                if (!i || errc)
                {
                    fname = NULL;
                    goto done;
                }

                i = fixLocalFixups ();

                fname = NULL;
                temp2 = 0;

                if (i) { errc = 1; goto done; }

                fseek (vo, 0, SEEK_END);

                for (sym = TOP(&syms); sym; sym = NEXT(sym))
                    temp2 += 1 + strlen (sym->name);

                putdword (temp2, vo);
                putdword (COUNT(&syms), vo);

                temp2 = ftell (vo) - 8;

                while ((sym = list__extract (&syms)) != NULL)
                {
                    wstr (sym->name, vo);
                    delete (sym);
                }

                if (entry)
                {
                    t = symbol__search (entry);
                    if (t)
                    {
                        fseek (vo, o__entry, SEEK_SET);
                        putc (t->section->index, vo);
                        putdword (t->offset, vo);
                    }
                    else
                    {
                        undefined (entry);
                        delete (entry);

                        goto done;
                    }

                    delete (entry);
                }

                fixDword (o__sym, temp2);

                if (pubCount)
                {
                    fixDword (o__pub, FileLength (vo));
                    addArea (vo, f__pub, pubCount);
                }

                if (extCount)
                {
                    fixDword (o__ext, FileLength (vo));
                    addArea (vo, f__ext, extCount);
                }

                if (fixCount)
                {
                    fixDword (o__fix, FileLength (vo));
                    addArea (vo, f__fix, fixCount);
                }

/*violet:FIX THE OTHER TAGS*/

                for (x = TOP(&sections); x; x = NEXT(x))
                {
                    fixDword (x->o__bin, FileLength (vo));

                    fp = fopen (x->temp_filename, "r+b");
                    if (fp == NULL)
                    {
                        pmsg ("*unable to open temporal section bin file");
                        continue;
                    }

                    addArea (vo, fp, 0xFFFFFFFFUL);
                    fclose (fp);
                }

                temp2 = coreleft();

                fseek (vo, 0, SEEK_END);

                pmsg ("%s successfully assembled, %s is %lu bytes long", s, q, ftell (vo));
                pmsg ("%lu bytes used (%lu bytes left)", temp - temp2, temp2);

            done:;
                xfcloseall ();

                if (errc) remove (q);

                fname = NULL;

                if (index < argc) printf ("\n");

                diString ();
                diRule ();

                if (errc) pmsg ("$unable to complete operation, errors were found!!");
                errc = 0;
            }

            remove_temps ();

            remove ("oz$lfix.tmp");

            remove ("oz$fix.tmp");
            remove ("oz$pub.tmp");
            remove ("oz$ext.tmp");

            pmsg ("finished.\n\n");

            return errc ? 1 : 0;
    }

    void __catcher (unsigned r, unsigned e, unsigned st)
    {
        int i;

            for (i = 0; i < MAX_LINFO; i++)
                if (plinenum [i] == linenum) break;

            errc++;

            if (i != MAX_LINFO && 0)
            {
                fseek (cfile, lineoffs [i], SEEK_SET);
                fgets (gbuf, sizeof (gbuf), cfile);
                *strchr (gbuf, '\n') = '\0';

                pmsg ("%s", gbuf);
                pmsg ("");
                for (i = 1; i < colnum; i++) printf (" ");
                printf ("^ syntax error");
            }
            else
            {
                symbolstr [symbollen] = '\0';
                pmsg ("*syntax error at line %u column %u (next token: '%s')", linenum, colnum, symbolstr);
            }
    }

    int __number (int n)
    {
        int i, j, k;

            if (!n) return 0;

            symbolstr [symbollen] = '\0';
            if (strchr (symbolstr, '.'))
            {
                for (i = j = 0; i < n; i++)
                {
                    k = symbolstr [i];

                    if (k == '.')
                    {
                        if (j++) break;
                        continue;
                    }
            
                    if ((0x30 > k || k > 0x39) && k != '_')
                        break;
                }

                if (i != n)
                {
                    pmsg ("*incorrect formation of floating-point number");
                    return c__dec_number;
                }

                return c__flt_number;
            }

            switch (symbolstr [symbollen - 1] & 0xDF)
            {
                case 'H':
                    n = c__hex_number;
                    break;

                case 'D':
                    n = c__dec_number;
                    break;

                case 'Q':
                    n = c__oct_number;
                    break;

                case 'B':
                    n = c__bin_number;
                    break;

                default:
                    n = c__dec_number;
                    symbollen++;
            }

            symbollen--;
            return n;
    }

    int __ident (int n)
    {
        variable_t *v;

            if (symbollen > 255)
            {
                pmsg ("#symbol truncated to 255 characters");
                symbollen = 255;
            }

            if (!n) return 0;

            if (symbollen == 3)
            {
                if (symbolstr [0] == 'S' || symbolstr [0] == 's')
                {
                    if (symbolstr [1] == 'H' || symbolstr [1] == 'h')
                    {
                        if (symbolstr [2] == 'L' || symbolstr [2] == 'l')
                            return 0;

                        if (symbolstr [2] == 'R' || symbolstr [2] == 'r')
                            return 0;
                    }
                }

                if (symbolstr [0] == 'N' || symbolstr [0] == 'n')
                {
                    if (symbolstr [1] == 'O' || symbolstr [1] == 'o')
                    {
                        if (symbolstr [2] == 'T' || symbolstr [2] == 't')
                            return 0;
                    }
                }

                if (symbolstr [0] == 'X' || symbolstr [0] == 'x')
                {
                    if (symbolstr [1] == 'O' || symbolstr [1] == 'o')
                    {
                        if (symbolstr [2] == 'R' || symbolstr [2] == 'r')
                            return 0;
                    }
                }

                if (symbolstr [0] == 'A' || symbolstr [0] == 'a')
                {
                    if (symbolstr [1] == 'N' || symbolstr [1] == 'n')
                    {
                        if (symbolstr [2] == 'D' || symbolstr [2] == 'd')
                            return 0;
                    }
                }
            }

            if (symbollen == 2)
            {
                if (symbolstr [0] == 'O' || symbolstr [0] == 'o')
                {
                    if (symbolstr [1] == 'R' || symbolstr [1] == 'r')
                        return 0;
                }
            }

            symbolstr [symbollen] = '\0';

            if (dict__isrch (isetdict, symbolstr))
                return c__extern_mnemonic;

            if (symbolstr [0] == '.')
                return 0;

            if ((v = gsearch (&variables, symbolstr)) != NULL)
            {
                if (v->type)
                {
/*violet: string-literal*/
                }
                else
                {
                    gFracFtoa (symbolstr, v->Value, 16, TrimZeroes);
                    symbollen = strlen (symbolstr);

                    return c__dec_number;
                }
            }

            return 0;
    }

    int __mident (int n)
    {
        char *r, *p, *q, *s = symbolstr + 1;
        variable_t *v; unsigned i;

            if (!n) return 0;

            n = s [symbollen - 2];
            s [symbollen - 2] = '\0';

            r = gbuf;
            p = s;

            while ((q = strchr (p, '(')) != NULL)
            {
                strncpy (r, p, n = q - p);
                r += n;

                p = strchr (q++, ')');
                if (!p) break;

                *p++ = '\0';

                if ((v = gsearch (&variables, q)) != NULL)
                {
                    if (v->type)
                    {
/*violet: string-literal*/
                    }
                    else
                    {
                        gFracFtoa (r, v->Value, 16, TrimZeroes);
                        r += strlen (r);
                    }
                }
            }

            strcpy (r, p);

            strncpy (symbolstr, gbuf, (symbollen = strlen (gbuf)) + 1);
            n = symbolCode (symbolstr, &i);

            s__index = (unsigned long)i << 16;
            return n;
    }

/****************************************************************************/

    /* This is actually kinda self-explanator ;) */
    #define is16BitOperand(s)   (s) == T__WORD
    #define is32BitOperand(s)   (s) == T__DWORD

    /* Some semantic types for integers. */
    #define T__INT      0x01
    #define T__REL      0x02
    #define T__PTR      0x03
    #define T__SEC      0x04
    #define T__FREL     0x05 /* Relative symbol (forward reference) */
    #define T__GRP      0x06
    #define T__EXT      0x07
    #define T__EXT_SEC  0x08

    #define T__REL32    0x11
    #define T__FREL32   0x12
    #define T__EXT32    0x13

    #define T__REL48    0x21
    #define T__FREL48   0x22
    #define T__EXT48    0x23

    #define T__REL64    0x31
    #define T__FREL64   0x32
    #define T__EXT64    0x33

    #define T__RPTR32   0x41
    #define T__FPTR32   0x42
    #define T__EPTR32   0x43

    #define T__RPTR48   0x51
    #define T__FPTR48   0x52
    #define T__EPTR48   0x53

    #define T__RPTR64   0x61
    #define T__FPTR64   0x62
    #define T__EPTR64   0x63

    #define T__FRAC     0x71

    /* VO fixup target type. */
    #define FTT__SEC    0
    #define FTT__GRP    1
    #define FTT__EXT    2

    /* VO fixup area info. */
    #define FAI__16Offs 0x00
    #define FAI__16Base 0x01
    #define FAI__32Offs 0x02
    #define FAI__32Base 0x03
    #define FAI__32Ptr  0x04
    #define FAI__48Ptr  0x05
    #define FAI__64Ptr  0x06

    /* Operand types. */
    #define OP__REG8    0x00
    #define OP__REG16   0x01
    #define OP__REG32   0x02
    #define OP__SREG    0x03
    #define OP__CREG    0x04
    #define OP__DREG    0x05
    #define OP__TREG    0x06
    #define OP__MMXREG  0x07
    #define OP__XMMREG  0x08
    #define OP__FPREG   0x09

    #define OP__MEM     0x10
    #define OP__MEM8    0x11
    #define OP__MEM16   0x12
    #define OP__MEM32   0x13
    #define OP__MEM48   0x14
    #define OP__MEM64   0x15
    #define OP__MEM80   0x16
    #define OP__MEM128  0x17

    #define OP__IMM     0x1A

    #define OP__SOVER   0x20

    #define OP__PTR32   0x30
    #define OP__PTR48   0x31

    /* Memory operand flags. */
    #define FL__BASE    0x01
    #define FL__INDEX   0x02
    #define FL__SCALE   0x04
    #define FL__DISP    0x08
    #define FL__X32     0x10
    #define FL__X16     0x20
    #define FL__NEG     0x80

    #define FL__BIMASK  0x03    /* Mask for Base and Index. */
    #define FL__DIMASK  0x0F    /* Mask for Base, Index, Scale and Disp. */

    /* Memory operand component shift. */
    #define SH__BASE    0x00
    #define SH__INDEX   0x04
    #define SH__SCALE   0x08
    #define COMP__MASK  0x0F

    #define MAX_FORMS           255
    #define MAX_SPECIFIERS      32
    #define MAX_OPERANDS        32

    /* Data types. */
    #define T__BYTE     0x00
    #define T__WORD     0x01
    #define T__DWORD    0x02
    #define T__FWORD    0x03
    #define T__QWORD    0x04
    #define T__DQWORD   0x05
    #define T__TBYTE    0x06
    #define T__PARA     0x07
    #define T__UNSPEC   0xFF

    /* Instruction set operand type. */
    #define ABSTRACT_OPERANDM   0x01
    #define ABSTRACT_OPERAND    0x02
    #define EXPLICIT_OPERAND    0x03
    #define CONSTANT            0x04
    #define OPCODE_SPECIFIER    0x05
    #define OPCODE_LENGTH       0x06

    /* Abstract Operand */
    enum AbstractOperand
    {
        __rel8=1, __rel16, __rel32, __r8, __r16, __r32, __sreg,
        __mmxreg, __xmmreg, __imm8, __imm16, __imm32, __rm8,
        __rm16, __rm32, __rm64, __rm128, __mem, __mem8, __mem16,
        __mem32, __mem48, __mem64, __mem128, __i16moffs, __i32moffs,
        __moffs8, __moffs16, __moffs32, __cr, __tr, __dr, __r32m16,
        __r128m32, __r128m64, __far32, __far48, __far64, __ptr32, __ptr48,
        __ptr64, __eptr32, __eptr48, __fpreg, __mem80, __rm80
    };

    /* Opcode Specifier */
    enum OpcodeSpecifier
    {
        __S0, __S1, __S2, __S3, __S4, __S5, __S6, __S7, __Sr, __Sd, __ib,
        __iw, __id, __Prb, __Prw, __Prd, __db, __dw, __dd, __cb, __cw, __cd,
        __rm, __pm, __xm, __pd, __pf, __pq, __idw, __idd
    };

    /* Stack of operands and stuff. */
    static unsigned segment_override [64];
    static unsigned long opvalue [64];
    static unsigned char opextra [64];
    static unsigned optop, opcount;
    static unsigned optype [64];

    /* Some indicators. */
    static unsigned seg_override, data_len, memory_type;

    /* Memory operand information (t). */
    static unsigned memComp, memFlags;
    static unsigned long exprComp, symComp;
    static unsigned exprType;

    /* Current bit-mode (16 or 32). */
    static unsigned cbits;

    /* The last label that was defined. */
    static symbol_t *lastLabel;

    /* Macro decoding information. */
    static unsigned skipMacro, level;

    /* Destroys a symbol node. */
    static int killSym (symbol_t *p)
    {
            delete (p->name);
            delete (p);

            return 0;
    }

    /* Resets the internal core variables. */
    static void resetCore (void)
    {
        variable_t *v; section_t *s; group_t *g;
        strn_t *p; pattern_t *t;

            skipMacro = level = 0;

            while ((s = list__pop (&sections)) != NULL)
            {
                if (s->klass) delete (s->klass);

                delete (s->temp_filename);
                delete (s->name);
                delete (s);
            }

            while ((g = list__pop (&groups)) != NULL)
            {
                delete (g->name);
                delete (g);
            }

            while ((p = list__pop (&publicPatterns)) != NULL)
            {
                delete (p->str);
                delete (p);
            }

            while ((t = list__pop (&externalPatterns)) != NULL)
            {
                delete (t->value);
                delete (t);
            }

            while ((v = list__pop (&variables)) != NULL)
            {
                if (v->type) delete (v->Buf);
                gFracDelete (v->Value);
                delete (v);
            }

            dict__destroy (&symbols, killSym);

            lastLabel = NULL;
            cbits = 16;
    }

    /* Searches for a section given its name. */
    static section_t *section__search (char *name)
    {
            return (section_t *)gsearch (&sections, name);
    }

    /* Searches for a group given its name. */
    static group_t *group__search (char *name)
    {
            return (group_t *)gsearch (&groups, name);
    }

    /* Shows an error message. */
    static void noSection (void)
    {
            if (csection)
                pmsg ("*data or code within imported section has no effect");
            else
                pmsg ("*data or code has no effect outside a section");
    }

    /* Shows an error message. */
    static void intExp (void)
    {
            pmsg ("*integer number expected");
    }

    /* Writes a data element given its type. */
    static void writeElem (int type, dword_t value, FILE *fp)
    {
            switch (type)
            {
                case 0:
                    putc (value, fp);
                    break;

                case 1:
                    putword (value, fp);
                    break;

                case 2:
                    putdword (value, fp);
                    break;

                case 3:
                    putdword (value, fp);
                    putword (0, fp);
                    break;

                case 4:
                    putdword (value, fp);
                    putdword (0, fp);
                    break;
            }
    }

    /* Writes a data element given its type (extended version). */
    static void writeElemEx (int type, IntConst Q, FILE *fp)
    {
        static int TypeSize [] = { 1, 2, 4, 6, 8, 10, 16 };
        int i, j = TypeSize [type];
        gInt Temp;

            if (Q.Type != T__FRAC)
            {
                Temp = gFracInt (Q.Value, 0);

                for (i = OzyaraByteCount - 1; j--; i--)
                    putc (Temp.Value [i], fp);

                gIntDelete (Temp);
            }
            else
            {
                switch (j)
                {
                    case 4:
                        fwrite (gFracToSingle (Q.Value), 1, 4, fp);
                        break;

                    case 8:
                        fwrite (gFracToDouble (Q.Value), 1, 8, fp);
                        break;

                    case 10:
                        fwrite (gFracToReal80 (Q.Value), 1, 10, fp);
                        break;

                    default:
                        pmsg ("*incorrect data size for floating point number");
                        break;
                }
            }
    }

    /* Returns the exact count of bytes needed to store the given value. */
    static int value_len (dword_t x)
    {
            if (x < 256) return 1;
            if (x < 65536) return 2;
            if (x < 16777216) return 3;

            return 4;
    }

    /* Writes a value of N bytes in little-endian format. */
    static void write_value (int n, dword_t x, FILE *fp)
    {
            switch (n)
            {
                case 1:
                    putc (x, fp);
                    break;

                case 2:
                    putword (x, fp);
                    break;

                case 3:
                    putword (x, fp);
                    putc (x >> 16, fp);
                    break;

                case 4:
                    putdword (x, fp);
                    break;
            }
    }

    /* Writes an VO fixup to the fixups file. */
    static void write_vo_fixup (unsigned s, dword_t offs, char tt, unsigned t, char ai, char rel)
    {
        int len = value_len (offs), len2;

            len2 = tt != FTT__EXT ? 1 : value_len (t);

            putc (((len-1) << 6) | ((tt+len2-1) << 3) | ai | (rel<<5), f__fix);
            putc (s, f__fix);

            write_value (len, offs, f__fix);
            write_value (len2, t, f__fix);

            fixCount++;
    }

    /* Writes a public definition to the publics file. */
    static void write_vo_public (unsigned sec, unsigned sym, dword_t offs)
    {
        int d6t5, d7;

            d6t5 = (value_len (offs) - 1) << 5;
            d7 = (value_len (sym) - 1) << 7;

            if (sec > 31)
            {
                putc (d7 | d6t5, f__pub);
                putc (sec, f__pub);
            }
            else
                putc (d7 | d6t5 | sec, f__pub);

            write_value (d7 ? 2 : 1, sym, f__pub);
            write_value ((d6t5 >> 5) + 1, offs, f__pub);

            pubCount++;
    }

    /* Tries to fix all the local fixups (everything in ffile). */
    static int fixLocalFixups (void)
    {
        section_t *x, *px = NULL;
        dword_t offs, tmp;
        int type, res = 0;
        FILE *fp = NULL;
        unsigned perrc;
        symbol_t *s;
        long t;

            putdword (0, ffile);
            rewind (ffile);

            fixing = 1;

            while ((x = (section_t *)getdword (ffile)) != NULL)
            {
                csection = x;

                if ((dword_t)x == 0xFFFFFFFFUL)
                {
                    tmp = getword (ffile);
                    type = (unsigned char)getc (ffile);
                }
                else
                {
                    tmp = getword (ffile);
                    type = getc (ffile);
                    offs = getdword (ffile);
                }

                tbuf [0] = 1;

                switch (type)
                {
                    case 0x20:
                        skipvofixups = 1;

                    case 0:
                        strcpy (&tbuf [1], "db ");
                        break;

                    case 0x21:
                        skipvofixups = 1;

                    case 1:
                        strcpy (&tbuf [1], "dw ");
                        break;

                    case 0x22:
                        skipvofixups = 1;

                    case 2:
                        strcpy (&tbuf [1], "dd ");
                        break;

                    case 3:
                        strcpy (&tbuf [1], "df ");
                        break;

                    case 4:
                        strcpy (&tbuf [1], "dq ");
                        break;

                    case 0x80:
                        rstr (ffile, tbuf);
                        perrc = errc;
                        write_public (tbuf);
                        if (errc != perrc) res++;
                        continue;
                }

                rstr (ffile, &tbuf [4]);

                if (px != x)
                {
                    if (fp) fclose (fp);
                    px = x;

                    fp = fopen (x->temp_filename, "r+b");
                    if (fp == NULL)
                    {
                        pmsg ("*unable to open temporal section bin file");
                        continue;
                    }

                    csection->file = fp;
                }

                if (!fp) continue;

                fseek (fp, offs, SEEK_SET);
                strcat (tbuf, "\n");

                perrc = errc;
                parseLine (tmp, tbuf);

                skipvofixups = 0;

                offs += csection->origin;

                switch (type)
                {
                    case 0x20:
                        ExprResult -= offs + 1;
                        t = ExprResult;

                        if (t < -128 || t > 127)
                        {
                            linenum = tmp;
                            pmsg ("*8-bit relative value out of range");
                        }

                        break;

                    case 0x21:
                        ExprResult -= offs + 2;
                        break;

                    case 0x22:
                        ExprResult -= offs + 4;
                        break;
                }

                if (errc != perrc) res++;

                writeElem (type & 15, ExprResult, fp);
            }

            if (fp) fclose (fp);

            return res;
    }

    /* Returns the logical location counter of the current section. */
    unsigned long locationCounter (void)
    {
            return ftell (csection->file) + csection->origin;
    }

    /* Writes a fixup */
    static void writeFixup (int type, char *buf)
    {
            if (fixing) return;

            putdword ((dword_t)csection, ffile);
            putword (linenum, ffile);

            putc (type, ffile);
            putdword (ftell (csection->file), ffile);
            wstr (buf, ffile);
    }

    /* Writes a special fixup */
    static void write_sFixup (int type, char *buf)
    {
            putdword (0xFFFFFFFFUL, ffile);
            putword (linenum, ffile);

            putc (type, ffile);
            wstr (buf, ffile);
    }

    /* Shows a message of 'X already defined'. */
    static void already (char *s, int d)
    {
            pmsg ("*%s '%s' already defined", d ? "symbol" : "section", s);
    }

    /* Writes a new public symbol to the publics file. */
    static void write_public (char *v)
    {
        symbol_t *p;
        unsigned i;

            p = symbol__search (v);
            if (p == NULL)
            {
                if (!fixing)
                    write_sFixup (0x80, v);
                else
                    undefined (v);

                return;
            }

            if (p->pubext)
            {
                if (p->pubext == 2)
                    pmsg ("*symbol '%s' already defined as %s", v, "external");

                return;
            }

            i = vo__srchsym (p->name);
            if (!i) i = vo__addsym (p->name);

            write_vo_public (p->section->index, i, p->offset);

            p->pubext = 1;
    }

    static int isForward (int n)
    {
            switch (n)
            {
                case T__FREL:
                case T__FPTR32:
                case T__FPTR48:
                case T__FPTR64:
                    return 1;
            }

            return 0;
    }

    static void makeExternal (char *v, section_t *s)
    {
        symbol_t *p;
        int i;

            p = symbol__search (v);
            if (p != NULL)
            {
                if (p->pubext)
                {
                    if (p->pubext == 1)
                        pmsg ("*symbol '%s' already defined as %s", v, "public");

                    return;
                }

                already (v, 1);
                return;
            }

            p = new (symbol_t);

            p->name = dups (v);
            p->section = s;
            p->pubext = 2;

            if (!(i = vo__srchsym (p->name))) i = vo__addsym (p->name);

            symbol__add (p);

            putword (i, f__ext);

            if (s && s->import)
                putc (s->index, f__ext);
            else
                putc (0, f__ext);

            extCount++;
    }

    static void IntConstDelete (IntConst P)
    {
            gFracDelete (P.Value);
    }

    static int MapCharToType (int Char)
    {
            switch (Char)
            {
                case 'I':   return T__INT;
                case 'R':   return T__REL;
                case 'P':   return T__PTR;
                case 'S':   return T__SEC;
                case 'F':   return T__FREL;
                case 'G':   return T__GRP;
                case 'E':   return T__EXT;
                case 's':   return T__EXT_SEC;
                case 'f':   return T__FRAC;

                case '1':   return T__REL32;
                case '2':   return T__REL48;
                case '3':   return T__REL64;

                case '4':   return T__FREL32;
                case '5':   return T__FREL48;
                case '6':   return T__FREL64;

                case '7':   return T__RPTR32;
                case '8':   return T__RPTR48;
                case '9':   return T__RPTR64;

                case '!':   return T__FPTR32;
                case '@':   return T__FPTR48;
                case '#':   return T__FPTR64;

                case '$':   return T__EXT32;
                case '%':   return T__EXT48;
                case '^':   return T__EXT64;

                case '&':   return T__EPTR32;
                case '*':   return T__EPTR48;
                case '(':   return T__EPTR64;
            }

            return T__INT;
    }

    static int ResultType (int Type1, int Type2, char *Map)
    {
        int TypeA, TypeB, TypeC;

            if (Type2 == -1)
            {
                while (*Map)
                {
                    TypeA = MapCharToType (*Map++);
                    TypeC = MapCharToType (*Map++);
                    if (*Map == '-') Map++;

                    if (Type1 == TypeA) return TypeC;
                }

                return -1;
            }

            while (*Map)
            {
                TypeA = MapCharToType (*Map++);
                TypeB = MapCharToType (*Map++);
                TypeC = MapCharToType (*Map++);
                if (*Map == '-') Map++;

                if (Type1 == TypeA && Type2 == TypeB)
                    return TypeC;
            }

            return -1;
    }

    static char *TypeString (int Type)
    {
            switch (Type)
            {
                case T__INT:        return "immediate";
                case T__REL:        return "relative";
                case T__PTR:        return "pointer";
                case T__SEC:        return "section";
                case T__FREL:       return "relative";
                case T__GRP:        return "group";
                case T__EXT:        return "external";
                case T__EXT_SEC:    return "ext-base";
                case T__FRAC:       return "fraction";
            }

            return "unk";
    }

    static void IncorrectForm (int TypeA, int TypeB, char *Operator)
    {
            if (TypeB == -1)
                pmsg ("*incorrect expression form: %s %s",
                    Operator, TypeString (TypeA));
            else
                pmsg ("*incorrect expression form: %s %s %s",
                    TypeString (TypeA), Operator, TypeString (TypeB));
    }

    static void DivZero (void)
    {
            pmsg ("*division by zero");
    }

    static IntConst __EvalExpr (rule_t *Expr, int Code)
    {
        static section_t *Section;
        static symbol_t *Symbol;
        static group_t *Group;
        static char *P;
        IntConst A, B;
        int Type, i;

            switch (Code)
            {
                case 0x00: /* expr */
                    Case0: Expr = Expr->rule [0];

                case 0x01: /* logical-expression */
                    switch (Expr->form)
                    {
                        case 0x00: /* shift-expression */
                            Expr = Expr->rule [0];
                            break;

                        case 0x01: /* logical-expression AND shift-expression */
                            A = __EvalExpr (Expr->rule [0], 0x01);
                            if (!A.Valid) return A;

                            B = __EvalExpr (Expr->rule [2], 0x02);
                            if (!B.Valid)
                            {
                                retB: IntConstDelete (A);
                                return B;
                            }

                            Type = ResultType (A.Type, B.Type, "III-IFF-FIF");
                            if (Type < 0)
                            {
                                IncorrectForm (A.Type, B.Type, "AND");
                                Bad2: B.Valid = 0;
                                goto retB;
                            }

                            A.Type = Type;
                            A.Value.Numer = gIntAnd (A.Value.Numer,
                                                     B.Value.Numer,
                                                     DeleteA);

                            retA: IntConstDelete (B);
                            return A;

                        case 0x02: /* logical-expression OR shift-expression */
                            A = __EvalExpr (Expr->rule [0], 0x01);
                            if (!A.Valid) return A;

                            B = __EvalExpr (Expr->rule [2], 0x02);
                            if (!B.Valid) goto retB;

                            Type = ResultType (A.Type, B.Type, "III-IFF-FIF");
                            if (Type < 0)
                            {
                                IncorrectForm (A.Type, B.Type, "OR");
                                goto Bad2;
                            }

                            A.Type = Type;
                            A.Value.Numer = gIntOr  (A.Value.Numer,
                                                     B.Value.Numer,
                                                     DeleteA);

                            goto retA;

                        case 0x03: /* logical-expression XOR shift-expression */
                            A = __EvalExpr (Expr->rule [0], 0x01);
                            if (!A.Valid) return A;

                            B = __EvalExpr (Expr->rule [2], 0x02);
                            if (!B.Valid) goto retB;

                            Type = ResultType (A.Type, B.Type, "III-IFF-FIF");
                            if (Type < 0)
                            {
                                IncorrectForm (A.Type, B.Type, "XOR");
                                goto Bad2;
                            }

                            A.Type = Type;
                            A.Value.Numer = gIntXor (A.Value.Numer,
                                                     B.Value.Numer,
                                                     DeleteA);

                            goto retA;
                    }

                case 0x02: /* shift-expression */
                    switch (Expr->form)
                    {
                        case 0x00: /* shift-expression */
                            Expr = Expr->rule [0];
                            break;

                        case 0x01: /* shift-expression SHL additive-expression */
                            A = __EvalExpr (Expr->rule [0], 0x02);
                            if (!A.Valid) return A;

                            B = __EvalExpr (Expr->rule [2], 0x03);
                            if (!B.Valid) goto retB;

                            Type = ResultType (A.Type, B.Type, "III-IFF-FIF");
                            if (Type < 0)
                            {
                                IncorrectForm (A.Type, B.Type, "SHL");
                                goto Bad2;
                            }

                            A.Type = Type;
                            A.Value.Numer = gIntShl (A.Value.Numer,
                                                     B.Value.Numer,
                                                     DeleteA);

                            goto retA;

                        case 0x02: /* shift-expression SHR additive-expression */
                            A = __EvalExpr (Expr->rule [0], 0x02);
                            if (!A.Valid) return A;

                            B = __EvalExpr (Expr->rule [2], 0x03);
                            if (!B.Valid) goto retB;

                            Type = ResultType (A.Type, B.Type, "III-IFF-FIF");
                            if (Type < 0)
                            {
                                IncorrectForm (A.Type, B.Type, "SHR");
                                goto Bad2;
                            }

                            A.Type = Type;
                            A.Value.Numer = gIntShr (A.Value.Numer,
                                                     B.Value.Numer,
                                                     DeleteA);

                            goto retA;
                    }

                case 0x03: /* additive-expression */
                    switch (Expr->form)
                    {
                        case 0x00: /* multiplicative-expression */
                            Expr = Expr->rule [0];
                            break;

                        case 0x01: /* additive-expression + multiplicative-expression */
                            A = __EvalExpr (Expr->rule [0], 0x03);
                            if (!A.Valid) return A;

                            B = __EvalExpr (Expr->rule [2], 0x04);
                            if (!B.Valid) goto retB;

                            Type = ResultType (A.Type, B.Type,
"III-IFF-FIF-Iff-fIf-fff-IRR-IEE-EIE-RIR-I11-I22-"
"I33-I11-I22-I33-I77-I88-I99-7I7-8I8-9I9-I44-I55-"
"I66-4I4-5I5-6I6-I!!-I@@-I##-!I!-@I@-#I#-I$$-I%%-"
"I^^-I&&-I**-I((-$I$-%I%-^I^-&I&-*I*-(I("
);

                            if (Type < 0)
                            {
                                IncorrectForm (A.Type, B.Type, "+");
                                goto Bad2;
                            }

                            if ((A.Type = Type) == T__FRAC)
                            {
                                A.Value = gFracAdd (A.Value, B.Value, DeleteA);
                            }
                            else
                            {
                                A.Value.Numer = gIntAdd (A.Value.Numer,
                                                         B.Value.Numer,
                                                         DeleteA);
                            }

                            goto retA;

                        case 0x02: /* additive-expression - multiplicative-expression */
                            A = __EvalExpr (Expr->rule [0], 0x03);
                            if (!A.Valid) return A;

                            B = __EvalExpr (Expr->rule [2], 0x04);
                            if (!B.Valid) goto retB;

                            Type = ResultType (A.Type, B.Type, "III-RIR-RRI-FIF-FFF-FRF-RFF-Iff-fIf-fff");
                            if (Type < 0)
                            {
                                IncorrectForm (A.Type, B.Type, "-");
                                goto Bad2;
                            }

                            if ((A.Type = Type) == T__FRAC)
                            {
                                A.Value = gFracSub (A.Value, B.Value, DeleteA);
                            }
                            else
                            {
                                A.Value.Numer = gIntSub (A.Value.Numer,
                                                         B.Value.Numer,
                                                         DeleteA);
                            }

                            goto retA;
                    }

                case 0x04: /* multiplicative-expression */
                    switch (Expr->form)
                    {
                        case 0x00: /* unary-expression */
                            Expr = Expr->rule [0];
                            break;

                        case 0x01: /* multiplicative-expression * unary-expression */
                            A = __EvalExpr (Expr->rule [0], 0x04);
                            if (!A.Valid) return A;

                            B = __EvalExpr (Expr->rule [2], 0x05);
                            if (!B.Valid) goto retB;

                            Type = ResultType (A.Type, B.Type, "III-IFF-FIF-Iff-fIf-fff");
                            if (Type < 0)
                            {
                                IncorrectForm (A.Type, B.Type, "*");
                                goto Bad2;
                            }

                            if ((A.Type = Type) == T__FRAC)
                            {
                                A.Value = gFracMul (A.Value, B.Value, DeleteA);
                            }
                            else
                            {
                                A.Value.Numer = gIntMul (A.Value.Numer,
                                                         B.Value.Numer,
                                                         DeleteA);
                            }

                            goto retA;

                        case 0x02: /* multiplicative-expression / unary-expression */
                            A = __EvalExpr (Expr->rule [0], 0x04);
                            if (!A.Valid) return A;

                            B = __EvalExpr (Expr->rule [2], 0x05);
                            if (!B.Valid) goto retB;

                            if (gFracZero (B.Value))
                            {
                                DivZ: DivZero ();
                                B.Valid = 0;
                                goto retB;
                            }

                            Type = ResultType (A.Type, B.Type, "III-IFF-FIF-Iff-fIf-fff");
                            if (Type < 0)
                            {
                                IncorrectForm (A.Type, B.Type, "*");
                                goto Bad2;
                            }

                            if ((A.Type = Type) == T__FRAC)
                            {
                                A.Value = gFracDiv (A.Value, B.Value, DeleteA);
                            }
                            else
                            {
                                A.Value.Numer = gIntDiv (A.Value.Numer,
                                                         B.Value.Numer,
                                                         DeleteA);
                            }

                            goto retA;

                        case 0x03: /* multiplicative-expression MOD unary-expression */
                            A = __EvalExpr (Expr->rule [0], 0x04);
                            if (!A.Valid) return A;

                            B = __EvalExpr (Expr->rule [2], 0x05);
                            if (!B.Valid) goto retB;

                            if (gFracZero (B.Value)) goto DivZ;

                            Type = ResultType (A.Type, B.Type, "III-IFF-FIF-Iff-fIf-fff");
                            if (Type < 0)
                            {
                                IncorrectForm (A.Type, B.Type, "MOD");
                                goto Bad2;
                            }

                            if ((A.Type = Type) == T__FRAC)
                            {
                                A.Value = gFracMod (A.Value, B.Value, DeleteA);
                            }
                            else
                            {
                                A.Value.Numer = gIntMod (A.Value.Numer,
                                                         B.Value.Numer,
                                                         DeleteA);
                            }

                            goto retA;

                    }

                case 0x05: /* unary-expression */
                    switch (Expr->form)
                    {
                        case 0x00: /* primary-expression */
                            Expr = Expr->rule [0];
                            break;

                        case 0x01: /* NOT unary-expression */
                            A = __EvalExpr (Expr->rule [1], 0x05);
                            if (!A.Valid) return A;

                            Type = ResultType (A.Type, -1, "II");
                            if (Type < 0)
                            {
                                IncorrectForm (A.Type, -1, "NOT");
                                Bad1: A.Valid = 0;
                                return A;
                            }

                            A.Type = Type;
                            A.Value.Numer = gIntNot (A.Value.Numer, DeleteA);

                            return A;

                        case 0x02: /* + unary-expression */
                            return __EvalExpr (Expr->rule [1], 0x05);

                        case 0x03: /* - unary-expression */
                            A = __EvalExpr (Expr->rule [1], 0x05);
                            if (!A.Valid) return A;

                            Type = ResultType (A.Type, -1, "II-ff");
                            if (Type < 0)
                            {
                                IncorrectForm (A.Type, -1, "-");
                                goto Bad1;
                            }

                            if ((A.Type = Type) == T__FRAC)
                                A.Value = gFracNeg (A.Value, DeleteA);
                            else
                                A.Value.Numer = gIntNeg (A.Value.Numer, DeleteA);

                            return A;
                    }

                case 0x06: /* primary-expression */
                    switch (Expr->form)
                    {
                        case 0x00: /* ( expr ) */
                            Expr = Expr->rule [1];
                            goto Case0;

                        case 0x01: /* flt-number */
                            A.Value = gFracAtof (Expr->value [0], 1, OzyaraByteCount);
                            A.Type = T__FRAC;
                            A.Valid = 1;

                            return A;

                        case 0x02:  /* hex-number */
                        case 0x03:  /* dec-number */
                        case 0x04:  /* oct-number */
                        case 0x05:  /* bin-number */
                            A.Value = gFracAtof (Expr->value [0], Expr->form - 2, OzyaraByteCount);

                            A.Type = T__INT;
                            A.Valid = 1;
                            return A;

                        case 0x06:
                            P = 1 + Expr->value [0];
                            A.Value = gFracNew (OzyaraByteCount);
                            A.Type = T__INT;
                            A.Valid = 1;

                            Type = strlen (P) - 1;

                            for (Code = 0; Code < Type; Code++)
                                A.Value.Numer.Value [OzyaraByteCount - Code - 1] = P [Code];

                            return A;

//violet:if pmsg(*) then return invalid IntConst?
                        case 0x07:
                            A.Value = gFracNew (OzyaraByteCount);
                            A.Valid = 1;

                            C7: Symbol = symbol__search (strcpy (gbuf, Expr->value [0]));
                            if (!Symbol)
                            {
                                Section = section__search (gbuf);
                                if (!Section)
                                {
                                    Group = group__search (gbuf);
                                    if (!Group)
                                    {
                                        if (fixing)
                                        {
                                            if (matchPattern (&externalPatterns, gbuf, 1))
                                            {
                                                makeExternal (gbuf, tsection);
                                                goto C7;
                                            }

                                            undefined (gbuf);
                                            A.Type = T__INT;
                                        }
                                        else
                                        {
                                            if (matchPattern (&externalPatterns, gbuf, 1))
                                            {
                                                makeExternal (gbuf, tsection);
                                                goto C7;
                                            }

                                            A.Type = T__FREL;
                                        }
                                    }
                                    else
                                        A.Type = T__GRP;
                                }
                                else
                                    A.Type = T__SEC;
                            }
                            else
                            {
                                if (Symbol->pubext == 2)
                                {
                                    A.Type = T__EXT;
                                }
                                else
                                {
                                    gFracDelete (A.Value);
                                    A.Value = gFracConv (Symbol->offset, 1, OzyaraByteCount);
                                    A.Type = T__REL;
                                }
                            }

                            O1: A.Symbol = Symbol;
                            return A;

                        case 0x08:
                            A.Value = gFracNew (OzyaraByteCount);
                            A.Valid = 1;

                            Symbol = symbol__search (strcpy (gbuf, Expr->value [0]));
                            if (!Symbol)
                            {
                                Section = section__search (gbuf);
                                if (!Section)
                                {
                                    Group = group__search (gbuf);
                                    if (!Group)
                                    {
                                        if (fixing)
                                        {
                                            undefined (gbuf);
                                            A.Type = T__INT;
                                        }
                                        else
                                        {
                                            sprintf (qbuf, "section %s", gbuf);
                                            strcpy (gbuf, qbuf);
        
                                            A.Type = T__FREL;
                                        }

                                        goto O1;
                                    }
                                }

                                pmsg ("*incorrect use of 'section' operator");
                                A.Type = T__INT;
                            }
                            else
                            {
                                if (Symbol->pubext != 2)
                                {
                                    strcpy (gbuf, Symbol->section->name);
                                    A.Type = T__SEC;
                                }
                                else
                                    A.Type = T__EXT_SEC;
                            }

                            goto O1;

                        case 0x09:
                            A.Value = gFracNew (OzyaraByteCount);
                            A.Valid = 1;

                            ptr32: Symbol = symbol__search (strcpy (gbuf, Expr->value [0]));
                            if (!Symbol)
                            {
                                Section = section__search (gbuf);
                                if (!Section)
                                {
                                    Group = group__search (gbuf);
                                    if (!Group)
                                    {
                                        if (fixing)
                                        {
                                            if (matchPattern (&externalPatterns, gbuf, 1))
                                            {
                                                makeExternal (gbuf, tsection);
                                                goto ptr32;
                                            }

                                            undefined (gbuf);
                                            A.Type = T__INT;
                                        }
                                        else
                                        {
                                            if (matchPattern (&externalPatterns, gbuf, 1))
                                            {
                                                makeExternal (gbuf, tsection);
                                                goto ptr32;
                                            }

                                            sprintf (qbuf, "ptr32 %s", gbuf);
                                            strcpy (gbuf, qbuf);

                                            A.Type = T__FPTR32;
                                        }
        
                                        goto O1;
                                    }
                                }

                                pmsg ("*incorrect use of 'ptr32' operator");
                                A.Type = T__INT;
                            }
                            else
                            {
                                A.Type = Symbol->pubext != 2 ? T__RPTR32 : T__EPTR32;
                                gFracDelete (A.Value);
                                A.Value = gFracConv (Symbol->offset, 1, OzyaraByteCount);
                            }

                            goto O1;

                        case 0x0A:
                            A.Value = gFracNew (OzyaraByteCount);
                            A.Valid = 1;

                            ptr48: Symbol = symbol__search (strcpy (gbuf, Expr->value [0]));
                            if (!Symbol)
                            {
                                Section = section__search (gbuf);
                                if (!Section)
                                {
                                    Group = group__search (gbuf);
                                    if (!Group)
                                    {
                                        if (fixing)
                                        {
                                            if (matchPattern (&externalPatterns, gbuf, 1))
                                            {
                                                makeExternal (gbuf, tsection);
                                                goto ptr48;
                                            }

                                            undefined (gbuf);
                                            A.Type = T__INT;
                                        }
                                        else
                                        {
                                            if (matchPattern (&externalPatterns, gbuf, 1))
                                            {
                                                makeExternal (gbuf, tsection);
                                                goto ptr48;
                                            }

                                            sprintf (qbuf, "ptr48 %s", gbuf);
                                            strcpy (gbuf, qbuf);

                                            A.Type = T__FPTR48;
                                        }
        
                                        goto O1;
                                    }
                                }

                                pmsg ("*incorrect use of 'ptr48' operator");
                                A.Type = T__INT;
                            }
                            else
                            {
                                A.Type = Symbol->pubext != 2 ? T__RPTR48 : T__EPTR48;
                                gFracDelete (A.Value);
                                A.Value = gFracConv (Symbol->offset, 1, OzyaraByteCount);
                            }

                            goto O1;

                        case 0x0B:
                            A.Value = gFracNew (OzyaraByteCount);
                            A.Valid = 1;

                            ptr64: Symbol = symbol__search (strcpy (gbuf, Expr->value [0]));
                            if (!Symbol)
                            {
                                Section = section__search (gbuf);
                                if (!Section)
                                {
                                    Group = group__search (gbuf);
                                    if (!Group)
                                    {
                                        if (fixing)
                                        {
                                            if (matchPattern (&externalPatterns, gbuf, 1))
                                            {
                                                makeExternal (gbuf, tsection);
                                                goto ptr64;
                                            }

                                            undefined (gbuf);
                                            A.Type = T__INT;
                                        }
                                        else
                                        {
                                            if (matchPattern (&externalPatterns, gbuf, 1))
                                            {
                                                makeExternal (gbuf, tsection);
                                                goto ptr64;
                                            }

                                            sprintf (qbuf, "ptr64 %s", gbuf);
                                            strcpy (gbuf, qbuf);

                                            A.Type = T__FPTR64;
                                        }
        
                                        goto O1;
                                    }
                                }

                                pmsg ("*incorrect use of 'ptr64' operator");
                                A.Type = T__INT;
                            }
                            else
                            {
                                A.Type = Symbol->pubext != 2 ? T__RPTR64 : T__EPTR64;
                                gFracDelete (A.Value);
                                A.Value = gFracConv (Symbol->offset, 1, OzyaraByteCount);
                            }

                            goto O1;

                        case 0x0C:
                            A.Value = gFracNew (OzyaraByteCount);
                            A.Valid = 1;

                            if (cbits == 16) goto ptr32; goto ptr48;

                        case 0x0D:
                            A.Value = gFracConv (Expr->LocationCounter, 1, OzyaraByteCount);
                            strcpy (gbuf, "$");
                            A.Type = T__REL;
                            A.Symbol = NULL;
                            A.Valid = 1;
                            return A;

                        case 0x0E:
                            A.Value = gFracNew (OzyaraByteCount);
                            strcpy (gbuf, "$$");
                            A.Symbol = NULL;
                            A.Type = T__REL;
                            A.Valid = 1;
                            return A;

                        case 0x0F:
                            A = __EvalExpr (Expr->rule [3], 0x00);
                            if (!A.Valid) return A;

                            gFracCopyL (A.Value, *(unsigned long *)gFracToSingle (A.Value), 1);
                            A.Type = T__INT;
                            A.Valid = 1;
                            return A;

                        case 0x10:
                            A = __EvalExpr (Expr->rule [3], 0x00);
                            if (!A.Valid) return A;

                            A.Type = T__FRAC;
                            A.Valid = 1;
                            return A;

                        case 0x11:
                            A = __EvalExpr (Expr->rule [3], 0x00);
                            if (!A.Valid) return A;

                            A.Type = T__INT;
                            A.Valid = 1;
                            return A;
                    }
            }

            A.Value = gFracNew (OzyaraByteCount);
            A.Valid = 0;
            return A;
    }

    static gFrac EvalExpr (rule_t *Expr, int Flags)
    {
        gFrac Result;

            Result = __EvalExpr (Expr, 0x00).Value;
            ExprResult = gFracGetLong (Result, 0);

            if (Flags & DeleteA) dRule (Expr);

            return Result;
    }

    static IntConst EvalExprEx (rule_t *Expr)
    {
            return __EvalExpr (Expr, 0x00);
    }

    static void writeExpr (rule_t *ex, int type, char *gbuf, int Flags, int Neg)
    {
        unsigned n;

            ExprResultEx = EvalExprEx (ex);
            if (Neg) ExprResultEx.Value = gFracNeg (ExprResultEx.Value, DeleteA);
            ExprResult = gFracGetLong (ExprResultEx.Value, 0);

            switch (ExprResultEx.Type)
            {
                case T__SEC:
debug(pmsg ("SECTION: %s", gbuf));
                    if (!type)
                    {
                        pmsg ("*%s section base", "can't write fixup for 8-bit");
                        break;
                    }

                    n = 1 + (type - 1) * 2;

                    write_vo_fixup (csection->index, locationCounter (), FTT__SEC,
                        section__search (gbuf)->index, n, 0);

                    break;

                case T__REL:
                    if (skipvofixups) break;

debug(pmsg ("RELATIVE: %s", gbuf));
                    if (!type)
                    {
                        pmsg ("*%s local offset", "can't write fixup for 8-bit");
                        break;
                    }

                    n = (type - 1) * 2;

                    if (gbuf [0] == '$' && gbuf [1] == '$' && !gbuf [2])
                    {
/*violet: expr - $$ */
                        ExprResultEx.Value = gFracAdd (ExprResultEx.Value,
                                             gFracConv (nLocationCounter, 1,
                                             OzyaraByteCount), DeleteBoth);

                        goto cSec;
                    }

                    if (gbuf [0] == '$' && !gbuf [1])
                    {
                  cSec: write_vo_fixup (csection->index, locationCounter (), FTT__SEC,
                            csection->index, n, 0);
                    }
                    else
                    {
                        write_vo_fixup (csection->index, locationCounter (), FTT__SEC,
                            ((symbol_t *)symbol__search (gbuf))->section->index, n, 0);
                    }

                    break;

                case T__REL32:
debug(pmsg ("REL32: %s", gbuf));
                    write_vo_fixup (csection->index, locationCounter (), FTT__SEC,
                        ((symbol_t *)symbol__search (gbuf))->section->index, 4, 0);

                    break;

                case T__REL48:
debug(pmsg ("REL48: %s", gbuf));
                    write_vo_fixup (csection->index, locationCounter (), FTT__SEC,
                        ((symbol_t *)symbol__search (gbuf))->section->index, 5, 0);

                    break;

                case T__REL64:
debug(pmsg ("REL64: %s", gbuf));
                    write_vo_fixup (csection->index, locationCounter (), FTT__SEC,
                        ((symbol_t *)symbol__search (gbuf))->section->index, 6, 0);

                    break;

                case T__GRP:
debug(pmsg ("GROUP: %s", gbuf));
                    if (!type)
                    {
                        pmsg ("*%s group base", "can't write fixup for 8-bit");
                        break;
                    }

                    n = 1 + (type - 1) * 2;

                    write_vo_fixup (csection->index, locationCounter (), FTT__GRP,
                        group__search (gbuf)->index, n, 0);

                    break;

                case T__EXT:
debug(pmsg ("EXTERN: %s", gbuf));
                    if (!type)
                    {
                        pmsg ("*%s external offset", "can't write fixup for 8-bit");
                        break;
                    }

                    n = (type - 1) * 2;

                    write_vo_fixup (csection->index, locationCounter (), FTT__EXT,
                        vo__srchsym (gbuf), n, skipvofixups);

                    break;

                case T__EXT32:
debug(pmsg ("EXT32: %s", gbuf));
                    write_vo_fixup (csection->index, locationCounter (), FTT__EXT,
                        vo__srchsym (gbuf), 4, 0);

                    break;

                case T__EXT48:
debug(pmsg ("EXT48: %s", gbuf));
                    write_vo_fixup (csection->index, locationCounter (), FTT__EXT,
                        vo__srchsym (gbuf), 5, 0);

                    break;

                case T__EXT64:
debug(pmsg ("EXT64: %s", gbuf));
                    write_vo_fixup (csection->index, locationCounter (), FTT__EXT,
                        vo__srchsym (gbuf), 6, 0);

                    break;

                case T__EXT_SEC:
debug(pmsg ("EXTERN-SEC: %s", gbuf));
                    if (!type)
                    {
                        pmsg ("*%s external base", "can't write fixup for 8-bit");
                        break;
                    }

                    n = 1 + (type - 1) * 2;

                    write_vo_fixup (csection->index, locationCounter (), FTT__EXT,
                        vo__srchsym (gbuf), n, 0);

                    break;

                case T__EPTR32:
debug(pmsg ("EPTR32: %s", gbuf));
                    if (!type)
                    {
                        pmsg ("*%s external ptr32", "can't write fixup for 8-bit");
                        break;
                    }

                    write_vo_fixup (csection->index, locationCounter (), FTT__EXT,
                        vo__srchsym (gbuf), 4, 0);

                    break;

                case T__EPTR48:
debug(pmsg ("EPTR48: %s", gbuf));
                    if (!type)
                    {
                        pmsg ("*%s external ptr48", "can't write fixup for 8-bit");
                        break;
                    }

                    write_vo_fixup (csection->index, locationCounter (), FTT__EXT,
                        vo__srchsym (gbuf), 5, 0);

                    break;

                case T__EPTR64:
debug(pmsg ("EPTR64: %s", gbuf));
                    if (!type)
                    {
                        pmsg ("*%s external ptr64", "can't write fixup for 8-bit");
                        break;
                    }

                    write_vo_fixup (csection->index, locationCounter (), FTT__EXT,
                        vo__srchsym (gbuf), 6, 0);

                    break;

                case T__RPTR32:
debug(pmsg ("RPTR32: %s", gbuf));
                    if (!type)
                    {
                        pmsg ("*%s local ptr32", "can't write fixup for 8-bit");
                        break;
                    }

                    write_vo_fixup (csection->index, locationCounter (), FTT__SEC,
                        ((symbol_t *)symbol__search (gbuf))->section->index, 4, 0);

                    break;

                case T__RPTR48:
debug(pmsg ("RPTR48: %s", gbuf));
                    if (!type)
                    {
                        pmsg ("*%s local ptr48", "can't write fixup for 8-bit");
                        break;
                    }

                    write_vo_fixup (csection->index, locationCounter (), FTT__SEC,
                        ((symbol_t *)symbol__search (gbuf))->section->index, 5, 0);

                    break;

                case T__RPTR64:
debug(pmsg ("RPTR64: %s", gbuf));
                    if (!type)
                    {
                        pmsg ("*%s local ptr64", "can't write fixup for 8-bit");
                        break;
                    }

                    write_vo_fixup (csection->index, locationCounter (), FTT__SEC,
                        ((symbol_t *)symbol__search (gbuf))->section->index, 6, 0);

                    break;
            }

            if (ExprResultEx.Type != T__INT && isForward (ExprResultEx.Type))
            {
                RuleToString (ex, qbuf);
                writeFixup (type, qbuf);
            }

            if (Flags & DeleteA) dRule (ex);

            if (!fixing) writeElemEx (type, ExprResultEx, csection->file);

            if (Flags & DeleteB) IntConstDelete (ExprResultEx);
    }

    static void writeRel (void *ex, int type, char *gbuf, int Flags)
    {
        unsigned n;
        long t;

            ExprResultEx = EvalExprEx (ex);
            ExprResult = gFracGetLong (ExprResultEx.Value, 0);

            if (ExprResultEx.Type == T__EXT)
            {
                if (type)
                {
                    n = (type - 1) * 2;

                    write_vo_fixup (csection->index, locationCounter (), FTT__EXT,
                        vo__srchsym (gbuf), n, 1);
                }
                else
                    pmsg ("*%s external offset", "can't write fixup for 8-bit");
            }

            if (ExprResultEx.Type != T__FREL)
            {
                switch (type)
                {
                    case 0:
                        n = 1;
                        break;

                    case 1:
                        n = 2;
                        break;

                    case 2:
                        n = 4;
                        break;
                }

                ExprResult -= locationCounter () + n;
                t = ExprResult;

                if ((t < -128 || t > 127) && n == 1) pmsg ("*8-bit relative value out of range");

                if (ExprResultEx.Symbol && ExprResultEx.Symbol->section != csection && ExprResultEx.Type != T__EXT)
                {
                    write_vo_fixup (csection->index, locationCounter (), FTT__SEC,
                        ExprResultEx.Symbol->section->index, (type - 1) * 2, 1);
                }
            }
            else
            {
                RuleToString (ex, qbuf);
                writeFixup (type + 0x20, qbuf);
            }

            writeElem (type, ExprResult, csection->file);

            if (Flags & DeleteA) dRule (ex);
            if (Flags & DeleteB) IntConstDelete (ExprResultEx);
    }

    static int needsSymbol (int t)
    {
            return t >= T__REL;
    }

    static unsigned getExprType (int type)
    {
            switch (type)
            {
                case T__REL:
                case T__FREL:
                case T__EXT:
                    return T__REL;

                case T__REL32:
                case T__FREL32:
                case T__EXT32:
                    return T__REL32;

                case T__REL48:
                case T__FREL48:
                case T__EXT48:
                    return T__REL48;

                case T__REL64:
                case T__FREL64:
                case T__EXT64:
                    return T__REL64;

                case T__RPTR32:
                case T__FPTR32:
                case T__EPTR32:
                    return T__RPTR32;

                case T__RPTR48:
                case T__FPTR48:
                case T__EPTR48:
                    return T__RPTR48;

                case T__RPTR64:
                case T__FPTR64:
                case T__EPTR64:
                    return T__RPTR64;
            }

            return T__INT;
    }

    static int isExternal (int type)
    {
            switch (type)
            {
                case T__EXT:
                case T__EXT32:
                case T__EXT48:
                case T__EXT64:
                case T__EPTR32:
                case T__EPTR48:
                case T__EPTR64:
                    return 1;
            }

            return 0;
    }

    static int matchPattern (list_t *p, char *s, int m)
    {
        int i, j = strlen (s);
        pattern_t *n;

            for (n = TOP(p); n; n = NEXT(n))
            {
                i = strlen (n->value);
                if (j < i) continue;

                if (!strncmp (s, n->value, i))
                {
                    if (m) tsection = n->s;
                    return 1;
                }
            }

            return 0;
    }

    ACTION (pushback_f)
    {
            p__pushback (argv [0], form);
    }

    ACTION (mnemonic)
    {
            p__pushback (argv [0], form);

/*
violet:reset
*/
            opcount = optop = memFlags = memComp = symComp = 0;
            seg_override = memory_type = data_len = 0;
    }

    ACTION(temp_name)
    {
            strcpy (temp_name, argv [0]);

            attr_align = T__DWORD;
            attr_class [0] = '\0';
            attr_combine = 0;
            attr_base = 0;
    }

    ACTION(group_name)
    {
        unsigned i;

            strcpy (temp_name, argv [0]);

            cgroup = group__search (temp_name);
            if (cgroup) return;

            cgroup = new (group_t);

            cgroup->name = dups (temp_name);

            i = vo__srchsym (cgroup->name);
            if (!i) i = vo__addsym (cgroup->name);

            list__add (&groups, cgroup);

            cgroup->n_index = i;
            cgroup->index = COUNT(&groups);
    }

    ACTION(section_start)
    {
        void *ex;

            if (csection)
            {
                fclose (csection->file);
                csection = NULL;
            }

            csection = section__search (temp_name);
            if (csection == NULL)
            {
                secnotdef (temp_name);
                return;
            }

            csection->file = fopen (csection->temp_filename, "r+b");
            if (csection->file == NULL)
            {
           ret: pmsg ("*unable to open temporal file");
                csection = NULL;
                return;
            }

            fseek (csection->file, 0, SEEK_END);
    }

    ACTION(section_end)
    {
            if (csection)
            {
                fclose (csection->file);
                csection = NULL;
            }
    }

    static int ValidInteger (IntConst P)
    {
            return P.Valid && P.Type != T__FRAC;
    }

    ACTION(origin_statement)
    {
        rule_t *Expr = popRule ();
        unsigned long Value;
        IntConst Result;

            if (!csection || csection->import)
            {
                dRule (Expr);

                noSection ();
                return;
            }

            Result = EvalExprEx (Expr);
            if (!ValidInteger (Result))
            {
                IntConstDelete (Result);
                dRule (Expr);

                intExp ();
                return;
            }

            Value = gFracGetLong (Result.Value, 0);
            IntConstDelete (Result);
            dRule (Expr);

            if (Value < csection->origin)
                pmsg ("*offset must be above base 0x%lx", csection->origin);
            else
                fseek (csection->file, Value - csection->origin, SEEK_SET);
    }

    ACTION(align_statement)
    {
        unsigned long Value, Pad;
        IntConst Result;
        rule_t *Expr;

            if (!csection || csection->import)
            {
                dRule (popRule ());
                if (!form) dRule (popRule ());
                noSection ();
                return;
            }

            Result = EvalExprEx (Expr = popRule ());
            dRule (Expr);

            if (!ValidInteger (Result))
            {
                IntConstDelete (Result);
                if (!form) dRule (popRule ());
                intExp ();
                return;
            }

            Value = gFracGetLong (Result.Value, 0);
            IntConstDelete (Result);

            if (!form)
            {
                Result = EvalExprEx (Expr = popRule ());
                dRule (Expr);

                if (!ValidInteger (Result))
                {
                    IntConstDelete (Result);
                    intExp ();
                    return;
                }

                Pad = Value;
                Value = gFracGetLong (Result.Value, 0);
                IntConstDelete (Result);
            }
            else
                Pad = 0;

            Value -= locationCounter () % Value;
            while (Value--) putc (Pad, csection->file);
    }

    ACTION(publics_statement)
    {
        strn_t *p;

            if (gsearch (&publicPatterns, argv [1])) return;

            list__add (&publicPatterns, new_strn (dups (argv [1])));
    }

    ACTION(externs_statement)
    {
        pattern_t *p = gsearch (&externalPatterns, argv [1]);

            if (p)
            {
                if (!p->s && csection) p->s = csection;
                return;
            }

            p = new (pattern_t);
            p->value = dups (argv [1]);
            p->s = csection ? csection : NULL;

            list__add (&externalPatterns, p);
    }

    ACTION(entry_statement)
    {
            if (entry)
            {
                pmsg ("#overriding previous entry point (%s) with %s", entry, argv [1]);
                delete (entry);
            }

            entry = dups (argv [1]);
    }

    ACTION(bits_statement)
    {
        IntConst Result;
        rule_t *Expr;

            Result = EvalExprEx (Expr = popRule ());
            dRule (Expr);

            if (!ValidInteger (Result))
            {
                IntConstDelete (Result);
                intExp ();
                return;
            }

            n = gFracGetLong (Result.Value, 0);
            IntConstDelete (Result);

            if (n != 16 && n != 32)
            {
                pmsg ("*BITS constant must be either 16 or 32");
                return;
            }

            cbits = n;
    }

    ACTION(export_statement)
    {
        section_t *Section = section__search (argv [1]);
        char *s;

            if (!Section)
            {
                secnotdef (argv [1]);
                return;
            }

            if (Section->import)
            {
                pmsg ("#section prepared for %s, unable to override.", "import");
                return;
            }

            s = dups (chop (argv [3] + 1));

            if (Section->export)
            {
                pmsg ("#overriding previous export name (%s) with %s",
                    Section->export, s);

                delete (Section->export);
            }

            Section->export = s;
    }

    ACTION(import_statement)
    {
        section_t *Section = section__search (argv [1]);
        char *s;

            if (!Section)
            {
                secnotdef (argv [1]);
                return;
            }

            if (Section->export)
            {
                pmsg ("#section prepared for %s, unable to override.", "export");
                return;
            }

            s = dups (chop (argv [3] + 1));

            if (Section->import)
            {
                pmsg ("#overriding previous import name (%s) with %s",
                    Section->import, s);

                delete (Section->import);
            }

            Section->import = s;
    }

    ACTION(data_element)
    {
        unsigned char *s; long count;
        rule_t *ExprA, *ExprB;
        IntConst Result;

            if ((!csection && !fixing) || csection->import)
            {
                if (form != 0x01) dRule (popRule ());
                if (!form) dRule (popRule ());

                noSection ();
                return;
            }

            nLocationCounter = selected_data_len + locationCounter ();

            switch (form)
            {
                case 0: /* count dup (value) */
                    ExprA = popRule ();
                    ExprB = popRule ();

                    Result = EvalExprEx (ExprB);
                    dRule (ExprB);

                    if (!ValidInteger (Result))
                    {
                        IntConstDelete (Result);
                        dRule (ExprA);

                        intExp ();
                        return;
                    }

                    count = gFracGetLong (Result.Value, 0);
                    IntConstDelete (Result);

                    if (count < 1)
                    {
                        pmsg ("*incorrect count value (%ld) specified", count);
                        dRule (ExprA);

                        break;
                    }

                    while (count--)
                        writeExpr (ExprA, selected_data_type, gbuf, DeleteB, 0);

                    dRule (ExprA);
                    break;

                case 1: /* string-literal */
                    s = chop (argv [0] + 1);

                    while ((n = (unsigned char)*s++) != '\0') wData (n);
                    break;

                case 2: /* expression */
                    writeExpr (popRule (), selected_data_type, gbuf, DeleteBoth, 0);
                    break;
            }
    }

    ACTION(data_length_short_spec)
    {
            if (lastLabel) lastLabel->type = m [0] + 1;

            switch (selected_data_type = m [0])
            {
                case 0: selected_data_len = 1;
                break;

                case 1: selected_data_len = 2;
                break;

                case 2: selected_data_len = 4;
                break;

                case 3: selected_data_len = 6;
                break;

                case 4: selected_data_len = 8;
                break;

                case 5: selected_data_len = 10;
                break;

                case 6: selected_data_len = 16;
                break;
            }

            lastLabel = NULL;
    }

    ACTION(label_definition)
    {
        symbol_t *p;
        int i;

            if (!csection || csection->import) { noSection (); return; }

            p = symbol__search (argv [0]);
            if (p != NULL)
            {
                already (argv [0], 1);
                return;
            }

            p = new (symbol_t);

            p->offset = locationCounter ();
            p->name = dups (argv [0]);
            p->section = csection;

            symbol__add (p);
            lastLabel = p;

            if (matchPattern (&publicPatterns, p->name, 0))
                write_public (p->name);
    }

    ACTION(label_statement)
    {
        symbol_t *p;

            if (!csection || csection->import) { noSection (); return; }

            p = symbol__search (argv [0]);
            if (p != NULL)
            {
                already (argv [0], 1);
                return;
            }

            p = new (symbol_t);

            p->offset = locationCounter ();
            p->name = dups (argv [0]);
            p->section = csection;
            p->type = form ? 0 : m [2] + 1;

            symbol__add (p);
            lastLabel = p;
    }

    ACTION(variable_value)
    {
        variable_t *v;
        gFrac Result;

            Result = EvalExpr (popRule (), DeleteA);

            if (argv [0][0] != '.')
            {
                pmsg ("variable must begin with period to modify it");
                gFracDelete (Result);
                return;
            }

            v = gsearch (&variables, argv [0] + 1);
            if (!v)
            {
                list__add (&variables, v = new (variable_t));
                v->name = dups (argv [0] + 1);
            }

            if (v->type && v->Buf) delete (v->Buf);
//violet:insecure!!!! delete v->Value!!!
            v->Value = Result;
            v->type = 0;
    }

    ACTION(section_attr)
    {
        IntConst Result; rule_t *Expr;
        int i;

            if (3 <= form && form <= 8)
            {
                attr_align = form - 0x03;
                return;
            }

            switch (form)
            {
                case 0x00: /* Combine */
                    attr_combine = m [0];
                    break;

                case 0x01: /* Class */
                    strcpy (attr_class, chop (argv [0] + 1));
                    break;

                case 0x02: /* Base */
                    Result = EvalExprEx (Expr = popRule ());

                    if (!ValidInteger (Result))
                    {
                        IntConstDelete (Result);
                        dRule (Expr);

                        intExp ();
                        return;
                    }

                    attr_base = gFracGetLong (Result.Value, 0);

                    IntConstDelete (Result);
                    dRule (Expr);

                    break;
            }
    }

    ACTION(section_def)
    {
        section_t *x;
        unsigned i;

            if (COUNT(&sections) == 255)
            {
                pmsg ("#maximum section limit reached!");
                return;
            }

            if ((x = section__search (temp_name)) != NULL)
            {
                already (temp_name, 0);
                return;
            }

            x = new (section_t);

            if (!attr_class [0]) x->klass = 0; else x->klass = dups (attr_class);
            x->name = dups (temp_name);
            x->group = NULL;

            x->combine = attr_combine;
            x->align = attr_align;
            x->origin = attr_base;

            x->temp_filename = build_tempfilename ();

            NewFile (x->temp_filename);

            list__add (&sections, x);

            i = vo__srchsym (x->name);
            if (!i) i = vo__addsym (x->name);

            x->n_index = i;

            if (x->klass)
            {
                i = vo__srchsym (x->klass);
                if (!i) i = vo__addsym (x->klass);
            }
            else
                i = 0;

            x->index = COUNT(&sections);
            x->c_index = i;
    }

    ACTION (group_member_list)
    {
        section_t *x;
        group_t *g;
        char *s;

            x = section__search (s = form ? argv [0] : argv [2]);
            if (x == NULL)
            {
                pmsg ("#section '%s' not defined", s);
                return;
            }

            if (x->group && x->group != cgroup)
            {
                pmsg ("#section '%s' belongs to group '%s'",
                    x->name, x->group->name);

                return;
            }

            x->group = cgroup;
    }

    ACTION(stmt_start)
    {
        section_t *p;
        group_t *q;

            putc (0x82, vo);
            wstr (fname, vo);
            o__sym = ftell (vo);
            putdword (0, vo);
            putc (COUNT(&sections), vo);
            putc (COUNT(&groups), vo);

            for (p = TOP(&sections); p; p = NEXT(p))
            {
                /* SECTION-DEF */
                putc (0x83, vo);

                putc (p->n_index, vo);
                putc (p->c_index, vo);

                putdword (0, vo);

                putc ((p->align << 5) | (p->combine << 4), vo);

                p->o__bin = ftell (vo);
                putdword (0, vo);
                putdword (p->origin, vo);

                if (p->import || p->export)
                {
                    /* EXPIMP-NAME */
                    putc (0x86, vo);
                    putc (p->import ? 1 : 0, vo);

                    wstr (p->import ? p->import : p->export, vo);
                }
            }

            for (q = TOP(&groups); q; q = NEXT(q))
            {
                /* GROUP-DEF */
                putc (0x84, vo);

                for (n = 0, p = TOP(&sections); p; p = NEXT(p))
                    if (p->group == q) n++;

                putc (n, vo);
                putc (q->n_index, vo);

                for (p = TOP(&sections); p; p = NEXT(p))
                {
                    if (p->group != q) continue;

                    /* GROUP-MEMBER */
                    putc (0x85, vo);
                    putc (p->index, vo);
                }
            }

            /* E-POINT */
            putc (0x87, vo);
            o__entry = ftell (vo);
            putc (0, vo);
            putdword (0, vo);

            /* IMP-DEF */
            putc (0x88, vo);
            o__pub = ftell (vo);    putdword (0, vo);
            o__ext = ftell (vo);    putdword (0, vo);
            o__dbg = ftell (vo);    putdword (0, vo);
            o__fix = ftell (vo);    putdword (0, vo);

            /* VO-END */
            putc (0x89, vo);
    }

    ACTION (public_symbol)
    {
            write_public (argv [form ? 0 : 2]);
    }

    ACTION (extern_symbol)
    {
            makeExternal (argv [form ? 0 : 2], csection);
    }

    ACTION (reg)
    {
            optype [optop] = OP__REG8 + form;
            opvalue [optop++] = m [0];

            opcount++;
    }

    ACTION (segment_override)
    {
        static unsigned override_prefix [] =
        { 0x26, 0x2E, 0x36, 0x3E, 0x64, 0x65 };

            seg_override = override_prefix [m [0]];
    }

    ACTION (explicit_size_specification)
    {
        static unsigned data_length [] =
        {
            OP__MEM8, OP__MEM16, OP__MEM32, OP__MEM48, OP__MEM64, OP__MEM80,
            OP__MEM128
        };

            data_len = data_length [m [0]];
    }

    ACTION (explicit_mem_type_specification)
    {
        static unsigned mem_type [] = { 32, 48, 64 };

            memory_type = mem_type [m [0]];
    }

    ACTION (x32memOpA)
    {
            if (form)
            {
                if (memFlags & FL__X32)
                {
               inv: pmsg ("invalid indexing mode");
                    return;
                }

                if (form < 3) form = FL__BASE, n = SH__BASE;
                else form = FL__INDEX, n = SH__INDEX;

                if (memFlags & form) goto inv;

                memFlags |= form | FL__X16;
                memComp |= (m [0] & COMP__MASK) << n;

                return;
            }

            if (memFlags & FL__X16) goto inv;

            if (!(memFlags & FL__BASE))
            {
                memFlags |= FL__BASE | FL__X32;
                memComp |= (m [0] & COMP__MASK) << SH__BASE;
                return;
            }

            if (!(memFlags & FL__INDEX))
            {
                memFlags |= FL__INDEX | FL__X32;
                memComp |= (m [0] & COMP__MASK) << SH__INDEX;
                return;
            }

            goto inv;
    }

    ACTION (x32memOpB)
    {
        IntConst Result;
        rule_t *Expr;

            Expr = popRule ();

            if (memFlags & FL__DISP)
            {
                dRule (Expr);

           inv: pmsg ("*invalid indexing mode");
                return;
            }

            Result = EvalExprEx (Expr);
            if (!ValidInteger (Result))
            {
                IntConstDelete (Result);
                dRule (Expr);

                intExp ();
                return;
            }
            else
            {
                ExprResult = gFracGetLong (Result.Value, 0);
                exprType = Result.Type;
                IntConstDelete (Result);
            }

            memFlags |= FL__DISP;

            exprComp = (unsigned long)Expr;
            symComp = needsSymbol (exprType) ? (unsigned long)dups (gbuf) : 0;
    }

    static int isbyte (long x)
    {
            return -128 <= x && x <= 127;
    }

    static int isbyteu (long x)
    {
			x = x < 0 ? -x : x;
			return 0 <= x && x <= 255;
    }

    ACTION (x32scale)
    {
            if (memFlags & FL__X16) goto inv;

            n = atoi (argv [2]);

            if (n != 1 && n != 2 && n != 4 && n != 8)
            {
                pmsg ("*incorrect constant for SIB scale specified");
                return;
            }

            if (m [0] == 4)
            {
                if (n == 1)
                {
                    if (!(memFlags & FL__BASE))
                    {
                        memFlags |= FL__BASE | FL__X32;
                        memComp |= (m [0] & COMP__MASK) << SH__BASE;
                        return;
                    }

                    if (!(memFlags & FL__INDEX))
                    {
                        memFlags |= FL__INDEX | FL__X32;
                        memComp |= (m [0] & COMP__MASK) << SH__INDEX;
                        return;
                    }

                    goto inv;
                }

                pmsg ("*incorrect register for SIB index specified");
                return;
            }

            if (!(memFlags & FL__INDEX))
            {
                memFlags |= FL__X32 | FL__SCALE | FL__INDEX;

                switch (n)
                {
                    case 1: n = 0; break;
                    case 2: n = 1; break;
                    case 4: n = 2; break;
                    case 8: n = 3; break;
                }

                memComp |= (m [0] & COMP__MASK) << SH__INDEX;
                memComp |= (n & COMP__MASK) << SH__SCALE;
            }
            else
                inv: pmsg ("*invalid indexing mode");
    }

    ACTION (x32mem)
    {
        symbol_t *p;

            segment_override [optop] = seg_override;

            if (!(memFlags & FL__BASE) && !(memFlags & FL__INDEX))
            {
                memFlags &= ~FL__X16;
                memFlags &= ~FL__X32;

                if (ExprResult <= 0xFFFFUL)
                    memFlags |= cbits == 32 ? FL__X32 : FL__X16;
                else
                    memFlags |= FL__X32;
            }

            if (form == 1 || form == 3 || form == 7 || form == 9)
                memFlags |= FL__NEG;

            if (!data_len)
            {
                if (symComp)
                {
                    p = symbol__search (symComp);
                    if (p != NULL)
                    {
                        optype [optop] = OP__MEM + p->type;
                        goto Ok;
                    }
                }

                optype [optop] = OP__MEM;
            }
            else
                optype [optop] = data_len;

        Ok: opvalue [optop++] = memFlags;

            optype [optop] = getExprType (exprType) | (memory_type << 8);
            opvalue [optop++] = exprComp;

            optype [optop] = memComp;
            opvalue [optop++] = symComp;

            opcount++;

            seg_override = memory_type = data_len = memFlags = memComp = 0;
    }

    ACTION (immediate)
    {
        IntConst Result;
        rule_t *Expr;

            Expr = popRule ();
            Result = EvalExprEx (Expr);
            if (!Result.Valid || Result.Type == T__FRAC)
            {
                IntConstDelete (Result);
                dRule (Expr);

                intExp ();
                return;
            }
            else
                exprType = Result.Type;

            exprComp = (unsigned long)Expr;
            symComp = needsSymbol (exprType) ? (unsigned long)dups (gbuf) : 0;

            optype [optop] = OP__IMM;

            if (!Result.Symbol)
            {
                if (needsSymbol (exprType))
                {
                    if (gbuf [0] == '$' && gbuf [1] == '$' && !gbuf [2])
                        opextra [optop] = 1;
                    else
                        if (gbuf [0] == '$' && !gbuf [1])
                            opextra [optop] = 1;
                        else
                            opextra [optop] = 0;
                }
                else
                    opextra [optop] = 0;
            }
            else
                opextra [optop] = Result.Symbol->section == csection;

            opvalue [optop++] = exprComp;

            optype [optop] = getExprType (exprType) | (exprType << 8);
            opvalue [optop++] = symComp;

            optype [optop] = form == 2 ? T__UNSPEC : m [0];
            opvalue [optop++] = gFracGetLong (Result.Value, 0);
            opcount++;

            IntConstDelete (Result);

            memory_type = data_len = memFlags = memComp = 0;
    }

    ACTION (epointer)
    {
        rule_t *ExprA, *ExprB;
        IntConst Result;

            ExprA = popRule ();
            ExprB = popRule ();

            Result = EvalExprEx (ExprA);
            if (!ValidInteger (Result))
            {
                IntConstDelete (Result);

                dRule (ExprA);
                dRule (ExprB);

                intExp ();
                return;
            }
            else
            {
                exprType = Result.Type;
                IntConstDelete (Result);
            }

            exprComp = (unsigned long)ExprA;
            symComp = needsSymbol (exprType) ? (unsigned long)dups (gbuf) : 0;

            optype [optop] = cbits == 16 ? OP__PTR32 : OP__PTR48;
            opvalue [optop++] = exprComp;
            opvalue [optop++] = symComp;

            Result = EvalExprEx (ExprB);
            if (!ValidInteger (Result))
            {
                IntConstDelete (Result);

                dRule (ExprA);
                dRule (ExprB);

                optop -= 2;

                intExp ();
                return;
            }
            else
            {
                exprType = Result.Type;
                IntConstDelete (Result);
            }

            exprComp = (unsigned long)ExprB;
            symComp = needsSymbol (exprType) ? (unsigned long)dups (gbuf) : 0;

            opvalue [optop++] = exprComp;
            opvalue [optop++] = symComp;

            opcount++;

            memory_type = data_len = memFlags = memComp = 0;
    }

    static int isMEM (int n)
    {
            switch (n)
            {
                case OP__MEM:
                case OP__MEM8:
                case OP__MEM16:
                case OP__MEM32:
                case OP__MEM48:
                case OP__MEM64:
                case OP__MEM80:
                case OP__MEM128:
                    return 1;
            }

            return 0;
    }

    static char operandSize (int x)
    {
            switch (x)
            {
                case __rel8:
                case __r8:
                case __imm8:
                case __rm8:
                case __mem8:
                case __moffs8:
                    return T__BYTE;

                case __rel16:
                case __r16:
                case __sreg:
                case __imm16:
                case __rm16:
                case __mem16:
                case __moffs16:
                    return T__WORD;

                case __rel32:
                case __r32:
                case __imm32:
                case __rm32:
                case __mem32:
                case __moffs32:
                case __cr:
                case __tr:
                case __dr:
                case __far32:
                case __ptr32:
                    return T__DWORD;

                case __mmxreg:
                case __rm64:
                case __mem64:
                case __far64:
                case __ptr64:
                    return T__QWORD;

                case __xmmreg:
                case __rm128:
                case __mem128:
                    return T__DQWORD;

                case __fpreg:
                case __rm80:
                case __mem80:
                    return T__TBYTE;

                case __r32m16:
                case __r128m32:
                case __r128m64:
                case __mem:
                    return T__UNSPEC;

                case __mem48:
                case __far48:
                case __ptr48:
                    return T__FWORD;

                default:
                    return T__UNSPEC;
            }
    }

    static unsigned insLength (char *q)
    {
        unsigned rb, rw, rd, reg, regl, mem, meml, offsl, datl, imml;
        unsigned v, t, i, j, k, opc, spc, len = 0;
        long yval;

            /* Search for Segment Override. */
            for (j = k = i = 0; j < opcount && !i; j++)
            {
                switch (t = optype [k])
                {
                    case OP__MEM:
                    case OP__MEM8:
                    case OP__MEM16:
                    case OP__MEM32:
                    case OP__MEM48:
                    case OP__MEM64:
                    case OP__MEM80:
                    case OP__MEM128:
                        if (segment_override [k])
                            i = segment_override [k];

                    case OP__IMM:
                        k += 3;
                        break;

                    case OP__PTR32:
                    case OP__PTR48:
                        k += 4;
                        break;

                    default:
                        k++;
                        break;
                }
            }

            opc = *q++;
            spc = *q++;

            offsl = T__UNSPEC;
            regl = T__UNSPEC;
            meml = T__UNSPEC;
            datl = T__UNSPEC;
            imml = T__UNSPEC;

            reg = MAX_FORMS;
            mem = MAX_FORMS;

            /* Map Operands */
            for (j = k = 0; j < opc; j++)
            {
                v = k;

                switch (t = optype [k])
                {
                    case OP__MEM:
                    case OP__MEM8:
                    case OP__MEM16:
                    case OP__MEM32:
                    case OP__MEM48:
                    case OP__MEM64:
                    case OP__MEM80:
                    case OP__MEM128:
                    case OP__IMM:
                        k += 3;
                        break;

                    case OP__PTR32:
                    case OP__PTR48:
                        k += 4;
                        break;

                    default:
                        k++;
                        break;
                }

                switch (*q++)
                {
                    case ABSTRACT_OPERANDM:
                        switch (*q++)
                        {
                            case __r8:
                                meml = T__BYTE;
                                mem = v;
                                break;

                            case __r16:
                                meml = T__WORD;
                                mem = v;
                                break;

                            case __sreg:
                                meml = T__WORD;
                                mem = v;
                                break;

                            case __r32:
                                meml = T__DWORD;
                                mem = v;
                                break;

                            case __mmxreg:
                                meml = T__QWORD;
                                mem = v;
                                break;

                            case  __xmmreg:
                                meml = T__DQWORD;
                                mem = v;
                                break;

                            case  __fpreg:
                                meml = T__TBYTE;
                                mem = v;
                                break;

                            case __cr:
                            case __tr:
                            case __dr:
                                meml = T__DWORD;
                                mem = v;
                                break;
                        }

                        break;

                    case ABSTRACT_OPERAND:
                        switch (*q++)
                        {
                            case __rel8:
                            case __rel16:
                            case __rel32:
                                break;

                            case __r8:
                                regl = T__BYTE;
                                reg = rb = v;
                                break;

                            case __r16:
                                regl = T__WORD;
                                reg = rw = v;
                                break;

                            case __sreg:
                                regl = T__WORD;
                                reg = v;
                                break;

                            case __r32:
                                regl = T__DWORD;
                                reg = rd = v;
                                break;

                            case __mmxreg:
                                regl = T__QWORD;
                                reg = v;
                                break;

                            case  __xmmreg:
                                regl = T__DQWORD;
                                reg = v;
                                break;

                            case  __fpreg:
                                regl = T__TBYTE;
                                reg = v;
                                break;

                            case __cr:
                            case __tr:
                            case __dr:
                                regl = T__DWORD;
                                reg = v;
                                break;

                            case __imm8:
                                imml = T__BYTE;
                                break;

                            case __imm16:
                                imml = T__WORD;
                                break;

                            case __imm32:
                                imml = T__DWORD;
                                break;

                            case __mem8:
                            case __rm8:
                                meml = T__BYTE;
                                mem = v;
                                break;

                            case __mem16:
                            case __rm16:
                                meml = T__WORD;
                                mem = v;
                                break;

                            case __mem32:
                            case __rm32:
                                meml = T__DWORD;
                                mem = v;
                                break;

                            case __mem64:
                            case __rm64:
                                meml = T__QWORD;
                                mem = v;
                                break;

                            case __mem128:
                            case __rm128:
                                meml = T__DQWORD;
                                mem = v;
                                break;

                            case __mem80:
                            case __rm80:
                                meml = T__TBYTE;
                                mem = v;
                                break;

                            case __mem:
                                meml = cbits == 32 ? T__DWORD : T__WORD;
                                mem = v;
                                break;

                            case __mem48:
                                meml = T__FWORD;
                                mem = v;
                                break;

                            case __i16moffs:
                                offsl = T__WORD;
                                break;

                            case __i32moffs:
                                offsl = T__DWORD;
                                break;

                            case __moffs8:
                                datl = T__BYTE;
                                break;

                            case __moffs16:
                                datl = T__WORD;
                                break;

                            case __moffs32:
                                datl = T__DWORD;
                                break;

                            case __r32m16:
                                if (t == OP__REG32) meml = T__DWORD; else meml = T__WORD;
                                mem = v;
                                break;

                            case __r128m32:
                                if (t == OP__XMMREG) meml = T__DQWORD; else meml = T__DWORD;
                                mem = v;
                                break;

                            case __r128m64:
                                if (t == OP__XMMREG) meml = T__DQWORD; else meml = T__QWORD;
                                mem = v;
                                break;

                            case __far32:
                                meml = T__WORD;
                                mem = v;
                                break;

                            case __far48:
                                meml = T__DWORD;
                                mem = v;
                                break;

                            case __far64:
                                meml = T__DWORD;
                                mem = v;
                                break;

                            case __ptr32:
                            case __ptr48:
                            case __ptr64:
                                break;

                            case __eptr32:
                                break;

                            case __eptr48:
                                break;
                        }

                        break;

                    default:
                        q++;
                        break;
                }
            }

            /* Skip Opcode Length */
            q += 2;
            j = 1;

            /* Operand-Size Prefix */
            if (*q == OPCODE_SPECIFIER && (q [1] == __rm || q [1] == __pm || q [1] == __xm))
            {
                if (q [1] == __rm && cbits == 32) len++;
                if (q [1] == __pm && cbits == 16) len++;

                q += 2;
                j++;
            }
            else
            {
                /* If explicit RM/PM not specified we'll try to
                   guess the operand size. */

                if
                (
                    is32BitOperand (regl) || is32BitOperand (datl) ||
                    is32BitOperand (meml) || is32BitOperand (imml) ||
                    is32BitOperand (offsl)
                )
                {
                    if (cbits == 16) len++;
                }
                else
                {
                    if
                    (
                        is16BitOperand (regl) || is16BitOperand (datl) ||
                        is16BitOperand (meml) || is16BitOperand (imml) ||
                        is16BitOperand (offsl)
                    )
                    {
                        if (cbits == 32) len++;
                    }
                }
            }

            /* Segment Override */
            if (i) len++;

            /* Address-Size Prefix */
            if (mem != MAX_FORMS)
            {
                if (cbits == 16 && opvalue [mem] & FL__X32 ||
                    cbits == 32 && opvalue [mem] & FL__X16) len++;
            }

            /* Constants, ModR/M, SIB, Displacement and Immediates */
            while (j++ < spc)
            {
                if (*q++ == CONSTANT)
                {
                    i = *q++;

                    if (*q == OPCODE_SPECIFIER)
                    {
                        switch (q [1])
                        {
                            case __Prb:
                                i += opvalue [rb];
                                q += 2;
                                j++;
                                break;

                            case __Prw:
                                i += opvalue [rw];
                                q += 2;
                                j++;
                                break;

                            case __Prd:
                                i += opvalue [rd];
                                q += 2;
                                j++;
                                break;
                        }
                    }

                    len++;
                    continue;
                }

                switch (*q++)
                {
                    case __S0:
                    case __S1:
                    case __S2:
                    case __S3:
                    case __S4:
                    case __S5:
                    case __S6:
                    case __S7:
                        i = *(q - 1) - __S0;
                        goto encode_ModRM;

                    case __Sd:

                    case __Sr:
                        i = reg == MAX_FORMS ? 0 : opvalue [reg];

          encode_ModRM: i <<= 3;

                        if (!isMEM (optype [mem]))
                        {
                            len++;
                            break;
                        }

                        memFlags = opvalue [mem];
                        exprComp = opvalue [mem + 1];
                        memComp = optype [mem + 2];
                        symComp = opvalue [mem + 2];

                        k = 0;

                        if (memFlags & FL__X16)
                        {
                            switch (memFlags & FL__BIMASK)
                            {
                                case FL__BASE + FL__INDEX:
                                case FL__INDEX:
                                    break;

                                case FL__BASE:
                                    if (((memComp >> SH__BASE) & COMP__MASK) == 0x05) /* BP */
                                        k = 6;

                                    break;

                                default:
                                    k = 8;
                                    break;
                            }

                            if (memFlags & FL__DISP)
                            {
                                gFracDelete (EvalExpr ((rule_t *)exprComp, 0));
                                yval = ExprResult;
                                if (memFlags & FL__NEG) yval = -yval;

                                if (k == 8)
                                {
                                    len += 3;
                                    goto Done_ModRM;
                                }

                                if (optype [mem + 1] == T__INT)
                                {
                                    if (isbyte (yval))
                                        len += 2;
                                    else
                                        len += 3;
                                }
                                else
                                    len += 3;
                            }
                            else
                            {
                                if (k == 0x06)
                                    len+=2;
                                else
                                    len++;
                            }
                        }
                        else
                        {
                            if (memFlags & FL__DISP)
                            {
                                gFracDelete (EvalExpr ((rule_t *)exprComp, 0));
                                yval = ExprResult;
                                if (memFlags & FL__NEG) yval = -yval;
                            }

                            if (memFlags & FL__SCALE)
                            {
                                if (memFlags & FL__BASE)
                                {
                                    v = 0100;

                                    if (memFlags & FL__DISP)
                                    {
                                        if (optype [mem + 1] == T__INT)
                                        {
                                            if (isbyte (yval))
                                                v = 0100;
                                            else
                                                v = 0200;
                                        }
                                        else
                                            v = 0200;
                                    }

                                    len+=2;

                                    t = ((memComp >> SH__SCALE) & COMP__MASK) << 6;
                                    k = ((memComp >> SH__INDEX) & COMP__MASK) << 3;
                                    i = (memComp >> SH__BASE) & COMP__MASK;

                                    if (memFlags & FL__DISP)
                                    {
                                        if (optype [mem + 1] == T__INT)
                                        {
                                            if (v == 0200)
                                                len+=4;
                                            else
                                                len+=1;
                                        }
                                        else
                                            len+=4;
                                    }
                                    else
                                    {
                                        if (v == 0200)
                                            len+=4;
                                        else
                                            len+=1;
                                    }
                                }
                                else
                                {
                                    v = ((memComp >> SH__SCALE) & COMP__MASK) << 6;
                                    k = ((memComp >> SH__INDEX) & COMP__MASK) << 3;

                                    len+=6;
                                }

                                goto Done_ModRM;
                            }

                            if (memFlags & FL__INDEX)
                            {
                                v = 0000;

                                if (memFlags & FL__DISP)
                                {
                                    v = 0200;

                                    if (optype [mem + 1] == T__INT)
                                        if (isbyte (yval)) v = 0100;
                                }

                                t = i;

                                i = (memComp >> SH__INDEX) & COMP__MASK;
                                k = (memComp >> SH__BASE) & COMP__MASK;

                                if (i == 4)
                                {
                                    i = k;
                                    k = 4;
                                }

                                if (k == 5)
                                {
                                    k = i;
                                    i = 5;
                                }

                                if (k == 5 && !v) v = 0100;

                                i <<= 3;

                                len+=2;

                                if (v)
                                {
                                    if (memFlags & FL__DISP)
                                    {
                                        if (optype [mem + 1] == T__INT)
                                        {
                                            if (v == 0100)
                                                len+=1;
                                            else
                                                len+=4;
                                        }
                                        else
                                            len+=4;
                                    }
                                    else
                                        len+=1;
                                }
                            }
                            else
                            {
                                if (memFlags & FL__BASE)
                                {
                                    k = (memComp >> SH__BASE) & COMP__MASK;

                                    if (k == 4) /* ESP */
                                    {
                                        if (memFlags & FL__DISP)
                                        {
                                            if (optype [mem + 1] == T__INT)
                                            {
                                                if (isbyte (yval))
                                                    len+=3;
                                                else
                                                    len+=6;
                                            }
                                            else
                                                len+=6;
                                        }
                                        else
                                            len+=2;

                                        goto Done_ModRM;
                                    }

                                    if (k == 5) /* EBP */
                                    {
                                        if (memFlags & FL__DISP)
                                        {
                                            if (optype [mem + 1] == T__INT)
                                            {
                                                if (isbyte (yval))
                                                    len+=3;
                                                else
                                                    len+=6;
                                            }
                                            else
                                                len+=6;
                                        }
                                        else
                                            len+=3;

                                        goto Done_ModRM;
                                    }

                                    /* Any Register */
                                    if (memFlags & FL__DISP)
                                    {
                                        if (optype [mem + 1] == T__INT)
                                        {
                                            if (isbyte (yval))
                                                len+=2;
                                            else
                                                len+=5;
                                        }
                                        else
                                            len+=5;
                                    }
                                    else
                                        len+=1;
                                }
                                else
                                    len+=5;
                            }
                        }

            Done_ModRM: break;

                    case __cb:
                    case __db:
                    case __ib:
                        len+=1;
                        break;

                    case __cw:
                    case __dw:
                    case __iw:
                    case __idw:
                        len+=2;
                        break;

                    case __pd:
                    case __cd:
                    case __dd:
                    case __id:
                    case __idd:
                        len+=4;
                        break;

                    case __pf:
                        len+=6;
                        break;

                    case __pq:
                        len+=8;
                        break;
                }
            }

            return len;
    }

    static void assemble_instruction (char *name)
    {
        static unsigned formn [MAX_FORMS], explicit [MAX_FORMS], susp [MAX_FORMS];
        static unsigned reg, regl, mem, meml, dat, datl, offs, offsl, imml;
        static unsigned rb, rw, rd, ib, iw, id, cb, cw, cd, pd, pf, pq;
        static unsigned opc, spc, tformc, formc, xform, u67, exreg;
        static unsigned i, ex, sus, il, j, k, t, v, xt, val, cop;
        static unsigned char *p, *q, *r, *u, *forms [MAX_FORMS];
        static unsigned char operand [MAX_FORMS][MAX_OPERANDS];
        static unsigned long xval;
        static ins_t *ins;
        static long yval;

            ins = dict__isrch (isetdict, name);
            if (!ins)
            {
                pmsg ("*(fatal internal error) instruction '%s' is gone!!", name);
                return;
            }

            if (fast)
            {
                if (!ins->data)
                {
                    fseek (iset, ins->offs, SEEK_SET);

                    ins->data = alloc (i = getword (iset));
                    fread (ins->data, 1, i, iset);
                }

                q = ins->data;
            }
            else
            {
                fseek (iset, ins->offs, SEEK_SET);

                p = q = alloc (i = getword (iset));
                fread (p, 1, i, iset);
            }

            tformc = *q++;

            for (i = formc = 0; i < tformc; i++)
            {
                r = q;

                opc = *q++;
                spc = *q++;

                ex = sus = 0;

                u = q + ((opc + spc) << 1);

                if (opc != opcount) goto Failed;

                for (j = k = 0; j < opc; j++)
                {
                    val = opvalue [k];

                    switch (t = optype [k])
                    {
                        case OP__MEM:
                        case OP__MEM8:
                        case OP__MEM16:
                        case OP__MEM32:
                        case OP__MEM48:
                        case OP__MEM64:
                        case OP__MEM80:
                        case OP__MEM128:
                        case OP__IMM:
                            xval = yval = opvalue [k + 2];
                            il = optype [k + 2];
                            xt = optype [k + 1];
                            v = xt >> 8;
                            xt &= 0xFF;
                            k += 3;
                            break;

                        case OP__PTR32:
                        case OP__PTR48:
                            k += 3;

                        default:
                            xt = -1;
                            v = 0;
                            k++;
                            break;
                    }

                    switch (*q++)
                    {
                        case ABSTRACT_OPERANDM:
                        case ABSTRACT_OPERAND:

                            cop = operandSize (*q);

                            switch (*q++)
                            {
                                case __rel8:
                                    if (t == OP__IMM && il == T__BYTE) break;

                                    if (il != T__UNSPEC) goto Failed;

                                    if (t != OP__IMM || xt != T__REL)
                                        goto Failed;

                                    if (v == T__FREL || isExternal (v))
                                        sus++;

                                    if (!isbyte (xval - locationCounter () - 4))
                                        sus++;

                                    if (!opextra [k - 3]) sus++;
                                    break;

                                case __rel16:
                                    if (t != OP__IMM || xt != T__REL || cbits!=16) goto Failed;
                                    break;

                                case __rel32:
                                    if (t != OP__IMM || xt != T__REL || cbits!=32) goto Failed;
                                    break;

                                case __r8:
                                    if (t != OP__REG8) goto Failed;
                                    break;

                                case __r16:
                                    if (t != OP__REG16) goto Failed;
                                    break;

                                case __r32:
                                    if (t != OP__REG32) goto Failed;
                                    break;

                                case __sreg:
                                    if (t != OP__SREG) goto Failed;
                                    break;

                                case __mmxreg:
                                    if (t != OP__MMXREG) goto Failed;
                                    break;

                                case __xmmreg:
                                    if (t != OP__XMMREG) goto Failed;
                                    break;

                                case __fpreg:
                                    if (t != OP__FPREG) goto Failed;
                                    break;

                                case __imm8:
                                    if (t != OP__IMM || xt != T__INT || (il != T__UNSPEC && il != T__BYTE)) goto Failed;
                                    if (il == T__BYTE) break;
									if (!isbyteu (yval)) goto Failed; // Fixup: 01:29am, 19-Jul-2014
                                    if (!isbyte (yval)) sus++;
                                    break;

                                case __imm16:
                                    if (t != OP__IMM || (il != T__UNSPEC && il != T__WORD)) goto Failed;
                                    if (il == T__WORD) break;
                                    if (xt == T__INT && (-32768 > yval || yval > 65535)) goto Failed;
                                    if (cbits == 32) sus++;
                                    break;

                                case __imm32:
                                    if (t != OP__IMM || (il != T__UNSPEC && il != T__DWORD)) goto Failed;
                                    if (il == T__DWORD) break;
                                    if (cbits == 16) sus++;
                                    break;

                                case __rm8:
                                    if (t != OP__REG8 && t != OP__MEM8 && t != OP__MEM || v)
                                        goto Failed;
                                    break;

                                case __rm16:
                                    if (t != OP__REG16 && t != OP__MEM16 && t != OP__MEM || v)
                                        goto Failed;
                                    break;

                                case __rm32:
                                    if (t != OP__REG32 && t != OP__MEM32 && t != OP__MEM || v)
                                        goto Failed;
                                    break;

                                case __rm64:
                                    if (t != OP__MMXREG && t != OP__MEM64 && t != OP__MEM || v)
                                        goto Failed;
                                    break;

                                case __rm80:
                                    if (t != OP__FPREG && t != OP__MEM80 && t != OP__MEM || v)
                                        goto Failed;
                                    break;

                                case __rm128:
                                    if (t != OP__XMMREG && t != OP__MEM128 && t != OP__MEM || v)
                                        goto Failed;
                                    break;

                                case __mem:
                                    if (!isMEM (t) || v) goto Failed;
                                    break;

                                case __mem8:
                                    if (t != OP__MEM8 && t != OP__MEM || v)
                                        goto Failed;
                                    break;

                                case __mem16:
                                    if (t != OP__MEM16 && t != OP__MEM || v)
                                        goto Failed;
                                    break;

                                case __mem32:
                                    if (t != OP__MEM32 && t != OP__MEM || v)
                                        goto Failed;
                                    break;

                                case __mem48:
                                    if (t != OP__MEM48 && t != OP__MEM || v)
                                        goto Failed;
                                    break;

                                case __mem64:
                                    if (t != OP__MEM64 && t != OP__MEM || v)
                                        goto Failed;
                                    break;

                                case __mem80:
                                    if (t != OP__MEM80 && t != OP__MEM || v)
                                        goto Failed;
                                    break;

                                case __mem128:
                                    if (t != OP__MEM128 && t != OP__MEM || v)
                                        goto Failed;
                                    break;

                                case __i16moffs:
                                    if ((t != OP__MEM || v) || (val & FL__DIMASK) != FL__DISP)
                                        goto Failed;
                                    break;

                                case __i32moffs:
                                    if ((t != OP__MEM || v) || (val & FL__DIMASK) != FL__DISP)
                                        goto Failed;
                                    break;

                                case __moffs8:
                                    if ((t != OP__MEM8 && t != OP__MEM || v) || (val & FL__DIMASK) != FL__DISP)
                                        goto Failed;
                                    break;

                                case __moffs16:
                                    if ((t != OP__MEM16 && t != OP__MEM) || (val & FL__DIMASK) != FL__DISP || v)
                                        goto Failed;
                                    break;

                                case __moffs32:
                                    if ((t != OP__MEM32 && t != OP__MEM) || (val & FL__DIMASK) != FL__DISP || v)
                                        goto Failed;
                                    break;

                                case __cr:
                                    if (t != OP__CREG) goto Failed;
                                    break;

                                case __tr:
                                    if (t != OP__TREG) goto Failed;
                                    break;

                                case __dr:
                                    if (t != OP__DREG) goto Failed;
                                    break;

                                case __r32m16:
                                    if (t != OP__REG32 && t != OP__MEM16 && t != OP__MEM  || v)
                                        goto Failed;
                                    break;

                                case __r128m32:
                                    if (t != OP__XMMREG && t != OP__MEM32 && t != OP__MEM || v)
                                        goto Failed;
                                    break;

                                case __r128m64:
                                    if (t != OP__XMMREG && t != OP__MEM64 && t != OP__MEM || v)
                                        goto Failed;
                                    break;

                                case __far32:
                                    if (t != OP__MEM || v != 32) goto Failed;
                                    break;

                                case __far48:
                                    if (t != OP__MEM || v != 48) goto Failed;
                                    break;

                                case __far64:
                                    if (t != OP__MEM || v != 64) goto Failed;
                                    break;

                                case __ptr32:
                                    if (t != OP__IMM || xt != T__RPTR32) goto Failed;
                                    break;

                                case __ptr48:
                                    if (t != OP__IMM || xt != T__RPTR48) goto Failed;
                                    break;

                                case __ptr64:
                                    if (t != OP__IMM || xt != T__RPTR64) goto Failed;
                                    break;

                                case __eptr32:
                                    if (t != OP__PTR32) goto Failed;
                                    break;

                                case __eptr48:
                                    if (t != OP__PTR48) goto Failed;
                                    break;
                            }

                            break;

                        case EXPLICIT_OPERAND:
                            xt = *q & 0x0F;

                            switch (*q++ >> 4)
                            {
                                case 0x00: /* REG8 */
                                    if (t != OP__REG8 || val != xt) goto Failed;
                                    cop = T__BYTE;
                                    break;

                                case 0x01: /* REG16 */
                                    if (t != OP__REG16 || val != xt) goto Failed;
                                    cop = T__WORD;
                                    break;

                                case 0x02: /* REG32 */
                                    if (t != OP__REG32 || val != xt) goto Failed;
                                    cop = T__DWORD;
                                    break;

                                case 0x03: /* SREG */
                                    if (t != OP__SREG || val != xt) goto Failed;
                                    cop = T__WORD;
                                    break;

                                case 0x04: /* CREG */
                                    if (t != OP__CREG || val != xt) goto Failed;
                                    cop = T__DWORD;
                                    break;

                                case 0x05: /* DREG */
                                    if (t != OP__DREG || val != xt) goto Failed;
                                    cop = T__DWORD;
                                    break;

                                case 0x06: /* TREG */
                                    if (t != OP__TREG || val != xt) goto Failed;
                                    cop = T__DWORD;
                                    break;

                                case 0x07: /* FPREG */
                                    if (t != OP__FPREG || val != xt) goto Failed;
                                    cop = T__QWORD;
                                    break;
                            }

                            ex++;
                            break;

                        case CONSTANT:
                            if (t != OP__IMM || xt != T__INT || xval != *q++)
                                goto Failed;

                            cop = T__BYTE;

                            ex++;
                            break;
                    }

                    operand [formc][j] = cop;
                }

                if (j == opc)
                {
                    explicit [formc] = ex;
                    susp [formc] = sus;

                    forms [formc] = r;
                    formn [formc++] = i;
                }

        Failed: q = u;
            }

            if (!formc)
            {
                pmsg ("*incorrect combination of operands for: %s", name);
                goto Bye;
            }

            nLocationCounter = 0;
            xform = MAX_FORMS;

            for (i = 0; i < formc; i++)
            {
                q = forms [i];

                opc = *q++;
                spc = *q++;

                q += opc << 1;

                if (*q++ != OPCODE_LENGTH)
                {
                    pmsg ("*(fatal) instruction set file is corrupt!!!");
                    p__finish = 1;

                    if (!fast) delete (p);
                    return;
                }

                if (xform == MAX_FORMS || *q <= t || explicit [i] > v || susp [i] < susp [xform])
                {
                    if (xform == MAX_FORMS) goto Fast;
                    if (susp [i] > susp [xform]) continue;
                    if (*q < t || explicit [i] > v || susp [i] < susp [xform]) goto Fast;

                    for (j = 0; j < opc; j++)
                        if (operand [xform][j] != operand [i][j]) break;

                    if (j == opc) continue;

                    if (imml == T__UNSPEC) imml = insLength (forms [xform]);

                    j = insLength (forms [i]);
                    if (j < imml)
                    {
                        nLocationCounter = imml = j;
                        v = explicit [xform = i];
                        t = *q;

                        continue;
                    }

                    if (j != imml || explicit [i] < v) continue;

                    pmsg ("#type override is required (%s %u/%u)", name, formn [xform], formn [i]);
                    continue;

              Fast: v = explicit [xform = i];
                    imml = T__UNSPEC;
                    t = *q;

                    continue;
                }
            }

    Encode: /* Search for Segment Override. */
            for (j = k = i = 0; j < opcount && !i; j++)
            {
                switch (t = optype [k])
                {
                    case OP__MEM:
                    case OP__MEM8:
                    case OP__MEM16:
                    case OP__MEM32:
                    case OP__MEM48:
                    case OP__MEM64:
                    case OP__MEM80:
                    case OP__MEM128:
                        if (segment_override [k])
                            i = segment_override [k];

                    case OP__IMM:
                        k += 3;
                        break;

                    default:
                        k++;
                        break;
                }
            }

            q = forms [xform];

            if (!nLocationCounter) nLocationCounter = insLength (q);

            nLocationCounter += locationCounter ();

            opc = *q++;
            spc = *q++;

            reg = MAX_FORMS;
            mem = MAX_FORMS;
            dat = MAX_FORMS;
            imml = T__UNSPEC;
            offsl = T__UNSPEC;

            exreg = 0;

            /* Map Operands */
            for (j = k = 0; j < opc; j++)
            {
                val = opvalue [v = k];

                switch (t = optype [k])
                {
                    case OP__MEM:
                    case OP__MEM8:
                    case OP__MEM16:
                    case OP__MEM32:
                    case OP__MEM48:
                    case OP__MEM64:
                    case OP__MEM80:
                    case OP__MEM128:
                    case OP__IMM:
                        xval = opvalue [k + 2];
                        xt = optype [k + 1];
                        k += 3;
                        break;

                    default:
                        xt = -1;
                        k++;
                        break;
                }

                switch (*q++)
                {
                    case ABSTRACT_OPERANDM:
                        switch (*q++)
                        {
                            case __r8:
                                meml = T__BYTE;
                                mem = v;
                                break;

                            case __r16:
                                meml = T__WORD;
                                mem = v;
                                break;

                            case __sreg:
                                meml = T__WORD;
                                mem = v;
                                break;

                            case __r32:
                                meml = T__DWORD;
                                mem = v;
                                break;

                            case __mmxreg:
                                meml = T__QWORD;
                                mem = v;
                                break;

                            case  __xmmreg:
                                meml = T__DQWORD;
                                mem = v;
                                break;

                            case  __fpreg:
                                meml = T__TBYTE;
                                mem = v;
                                break;

                            case __cr:
                            case __tr:
                            case __dr:
                                meml = T__DWORD;
                                mem = v;
                                break;
                        }

                        break;

                    case ABSTRACT_OPERAND:
                        switch (*q++)
                        {
                            case __rel8:
                                cb = v;
                                break;

                            case __rel16:
                                cw = v;
                                break;

                            case __rel32:
                                cd = v;
                                break;

                            case __r8:
                                regl = T__BYTE;
                                reg = rb = v;
                                break;

                            case __r16:
                                regl = T__WORD;
                                reg = rw = v;
                                break;

                            case __sreg:
                                regl = T__WORD;
                                reg = v;
                                break;

                            case __r32:
                                regl = T__DWORD;
                                reg = rd = v;
                                break;

                            case __mmxreg:
                                regl = T__QWORD;
                                reg = v;
                                break;

                            case __fpreg:
                                regl = T__TBYTE;
                                reg = v;
                                break;

                            case  __xmmreg:
                                regl = T__DQWORD;
                                reg = v;
                                break;

                            case __cr:
                            case __tr:
                            case __dr:
                                regl = T__DWORD;
                                reg = v;
                                break;

                            case __imm8:
                                imml = T__BYTE;
                                ib = v;
                                break;

                            case __imm16:
                                imml = T__WORD;
                                iw = v;
                                break;

                            case __imm32:
                                imml = T__DWORD;
                                id = v;
                                break;

                            case __mem8:
                            case __rm8:
                                meml = T__BYTE;
                                mem = v;
                                break;

                            case __mem16:
                            case __rm16:
                                meml = T__WORD;
                                mem = v;
                                break;

                            case __mem32:
                            case __rm32:
                                meml = T__DWORD;
                                mem = v;
                                break;

                            case __mem64:
                            case __rm64:
                                meml = T__QWORD;
                                mem = v;
                                break;

                            case __mem80:
                            case __rm80:
                                meml = T__TBYTE;
                                mem = v;
                                break;

                            case __mem128:
                            case __rm128:
                                meml = T__DQWORD;
                                mem = v;
                                break;

                            case __mem:
                                meml = cbits == 32 ? T__DWORD : T__WORD;
                                mem = v;
                                break;

                            case __mem48:
                                meml = T__FWORD;
                                mem = v;
                                break;

                            case __i16moffs:
                                offsl = T__WORD;
                                offs = v;
                                break;

                            case __i32moffs:
                                offsl = T__DWORD;
                                offs = v;
                                break;

                            case __moffs8:
                                datl = meml = T__BYTE;
                                dat = mem = v;
                                break;

                            case __moffs16:
                                datl = meml = T__WORD;
                                dat = mem = v;
                                break;

                            case __moffs32:
                                datl = meml = T__DWORD;
                                dat = mem = v;
                                break;

                            case __r32m16:
                                if (t == OP__REG32) meml = T__DWORD; else meml = T__WORD;
                                mem = v;
                                break;

                            case __r128m32:
                                if (t == OP__XMMREG) meml = T__DQWORD; else meml = T__DWORD;
                                mem = v;
                                break;

                            case __r128m64:
                                if (t == OP__XMMREG) meml = T__DQWORD; else meml = T__QWORD;
                                mem = v;
                                break;

                            case __far32:
                                meml = T__WORD;
                                mem = v;
                                break;

                            case __far48:
                                meml = T__DWORD;
                                mem = v;
                                break;

                            case __far64:
                                meml = T__DWORD;
                                mem = v;
                                break;

                            case __ptr32:
                                pd = v;
                                break;

                            case __ptr48:
                                pf = v;
                                break;

                            case __ptr64:
                                pq = v;
                                break;

                            case __eptr32:
                            case __eptr48:
                                pq = v;
                                break;
                        }

                        break;

                    case EXPLICIT_OPERAND:
                        exreg = *q++ & 0x0F;
                        break;

                    default:
                        q++;
                        break;
                }
            }

            /* Skip Opcode Length */
            q += 2;
            j = 1;

            /* Operand-Size Prefix */
            if (*q == OPCODE_SPECIFIER && (q [1] == __rm || q [1] == __pm || q [1] == __xm))
            {
                if (q [1] == __rm && cbits == 32) putByte (0x66);
                if (q [1] == __pm && cbits == 16) putByte (0x66);

                q += 2;
                j++;
            }
            else
            {
                /* If explicit RM/PM not specified we'll try to
                   guess the operand size. */

                if
                (
                    is32BitOperand (reg != T__UNSPEC ? regl : T__UNSPEC) ||
                    is32BitOperand (dat != T__UNSPEC ? datl : T__UNSPEC) ||
                    is32BitOperand (mem != T__UNSPEC ? meml : T__UNSPEC) ||
                    is32BitOperand (imml) || is32BitOperand (offsl)
                )
                {
                    if (cbits == 16) putByte (0x66);
                }
                else
                {
                    if
                    (
                        is16BitOperand (reg != T__UNSPEC ? regl : T__UNSPEC) ||
                        is16BitOperand (dat != T__UNSPEC ? datl : T__UNSPEC) ||
                        is16BitOperand (mem != T__UNSPEC ? meml : T__UNSPEC) ||
                        is16BitOperand (imml) || is16BitOperand (offsl)
                    )
                    {
                        if (cbits == 32) putByte (0x66);
                    }
                }
            }

            /* Segment Override */
            if (i) putByte (i);

            u67 = 0;

            /* Address-Size Prefix */
            if (mem != MAX_FORMS)
            {
                if (cbits == 16 && opvalue [mem] & FL__X32 ||
                    cbits == 32 && opvalue [mem] & FL__X16)
                {
                    putByte (0x67);
                    u67 = 1;
                }
            }

            /* Constants, ModR/M, SIB, Displacement and Immediates */
            while (j++ < spc)
            {
                if (*q++ == CONSTANT)
                {
                    i = *q++;

                    if (*q == OPCODE_SPECIFIER)
                    {
                        switch (q [1])
                        {
                            case __Prb:
                                i += opvalue [rb];
                                q += 2;
                                j++;
                                break;

                            case __Prw:
                                i += opvalue [rw];
                                q += 2;
                                j++;
                                break;

                            case __Prd:
                                i += opvalue [rd];
                                q += 2;
                                j++;
                                break;
                        }
                    }

                    putByte (i);
                    continue;
                }

                switch (*q++)
                {
                    case __S0:
                    case __S1:
                    case __S2:
                    case __S3:
                    case __S4:
                    case __S5:
                    case __S6:
                    case __S7:
                        i = *(q - 1) - __S0;
                        goto encode_ModRM;

                    case __Sd:
                        i = reg == MAX_FORMS ? 0 : opvalue [reg];
                        putByte (0300 + i + (i << 3));
                        break;

                    case __Sr:
                        i = reg == MAX_FORMS ? exreg : opvalue [reg];

          encode_ModRM: i <<= 3;

                        if (!isMEM (optype [mem]))
                        {
                            putByte (0300 + i + opvalue [mem]);
                            break;
                        }

                        memFlags = opvalue [mem];
                        exprComp = opvalue [mem + 1];
                        memComp = optype [mem + 2];
                        symComp = opvalue [mem + 2];

                        if (memFlags & FL__X16)
                        {
                            switch (memFlags & FL__BIMASK)
                            {
                                case FL__BASE + FL__INDEX:
                                    if (((memComp >> SH__BASE) & COMP__MASK) == 0x03) /* BX */
                                    {
                                        if (((memComp >> SH__INDEX) & COMP__MASK) == 0x06) /* SI */
                                            k = 0;
                                        else /* DI */
                                            k = 1;
                                    }
                                    else /* BP */
                                    {
                                        if (((memComp >> SH__INDEX) & COMP__MASK) == 0x06) /* SI */
                                            k = 2;
                                        else /* DI */
                                            k = 3;
                                    }

                                    break;

                                case FL__INDEX:
                                    if (((memComp >> SH__INDEX) & COMP__MASK) == 0x06) /* SI */
                                        k = 4;
                                    else /* DI */
                                        k = 5;

                                    break;

                                case FL__BASE:
                                    if (((memComp >> SH__BASE) & COMP__MASK) == 0x05) /* BP */
                                        k = 6;
                                    else
                                        k = 7;

                                    break;

                                default:
                                    k = 8;
                                    break;
                            }

                            if (memFlags & FL__DISP)
                            {
                                gFracDelete (EvalExpr ((rule_t *)exprComp, 0));
                                yval = ExprResult;
                                if (memFlags & FL__NEG) yval = -yval;

                                if (k == 8)
                                {
                                    putByte (0000 + i + 6);

                                    if (optype [mem + 1] == T__INT)
                                    {
                                        dRule ((rule_t *)exprComp);
                                        putWord (yval);
                                    }
                                    else
                                        writeExpr ((void *)exprComp, T__WORD, (void *)symComp, DeleteBoth, memFlags & FL__NEG);

                                    goto Done_ModRM;
                                }

                                if (optype [mem + 1] == T__INT)
                                {
                                    if (isbyte (yval))
                                    {
                                        putByte (0100 + i + k);
                                        putByte (yval);
                                    }
                                    else
                                    {
                                        putByte (0200 + i + k);
                                        putWord (yval);
                                    }

                                    dRule ((rule_t *)exprComp);
                                }
                                else
                                {
                                    putByte (0200 + i + k);
                                    writeExpr ((void *)exprComp, T__WORD, (void *)symComp, DeleteBoth, memFlags & FL__NEG);
                                }
                            }
                            else
                            {
                                if (k == 0x06)
                                {
                                    putByte (0100 + i + k);
                                    putByte (0);
                                }
                                else
                                    putByte (0000 + i + k);
                            }
                        }
                        else
                        {
                            if (memFlags & FL__DISP)
                            {
                                gFracDelete (EvalExpr ((rule_t *)exprComp, 0));
                                yval = ExprResult;
                                if (memFlags & FL__NEG) yval = -yval;
                            }

                            if (memFlags & FL__SCALE)
                            {
                                if (memFlags & FL__BASE)
                                {
                                    v = 0100;

                                    if (memFlags & FL__DISP)
                                    {
                                        if (optype [mem + 1] == T__INT)
                                        {
                                            if (isbyte (yval))
                                                v = 0100;
                                            else
                                                v = 0200;
                                        }
                                        else
                                            v = 0200;
                                    }

                                    putByte (v + i + 0004);

                                    t = ((memComp >> SH__SCALE) & COMP__MASK) << 6;
                                    k = ((memComp >> SH__INDEX) & COMP__MASK) << 3;
                                    i = (memComp >> SH__BASE) & COMP__MASK;

                                    putByte (t + k + i);

                                    if (memFlags & FL__DISP)
                                    {
                                        if (optype [mem + 1] == T__INT)
                                        {
                                            if (v == 0200)
                                                putDword (yval);
                                            else
                                                putByte (yval);

                                            dRule ((rule_t *)exprComp);
                                        }
                                        else
                                            writeExpr ((void *)exprComp, T__DWORD, (void *)symComp, DeleteBoth, memFlags & FL__NEG);
                                    }
                                    else
                                    {
                                        if (v == 0200)
                                            putDword (0);
                                        else
                                            putByte (0);
                                    }
                                }
                                else
                                {
                                    v = ((memComp >> SH__SCALE) & COMP__MASK) << 6;
                                    k = ((memComp >> SH__INDEX) & COMP__MASK) << 3;

                                    putByte (004 + i);
                                    putByte (k + v + 5);

                                    if (memFlags & FL__DISP)
                                    {
                                        if (optype [mem + 1] == T__INT)
                                        {
                                            putDword (yval);
                                            dRule ((rule_t *)exprComp);
                                        }
                                        else
                                            writeExpr ((void *)exprComp, T__DWORD, (void *)symComp, DeleteBoth, memFlags & FL__NEG);
                                    }
                                    else
                                        putDword (0);
                                }

                                goto Done_ModRM;
                            }

                            if (memFlags & FL__INDEX)
                            {
                                v = 0000;

                                if (memFlags & FL__DISP)
                                {
                                    v = 0200;

                                    if (optype [mem + 1] == T__INT)
                                        if (isbyte (yval)) v = 0100;
                                }

                                t = i;

                                i = (memComp >> SH__INDEX) & COMP__MASK;
                                k = (memComp >> SH__BASE) & COMP__MASK;

                                if (i == 4)
                                {
                                    i = k;
                                    k = 4;
                                }

                                if (k == 5)
                                {
                                    k = i;
                                    i = 5;
                                }

                                if (k == 5 && !v) v = 0100;

                                i <<= 3;

                                putByte (0004 + v + t);
                                putByte (i + k);

                                if (v)
                                {
                                    if (memFlags & FL__DISP)
                                    {
                                        if (optype [mem + 1] == T__INT)
                                        {
                                            if (v == 0100)
                                                putByte (yval);
                                            else
                                                putDword (yval);

                                            dRule ((rule_t *)exprComp);
                                        }
                                        else
                                            writeExpr ((void *)exprComp, T__DWORD, (void *)symComp, DeleteBoth, memFlags & FL__NEG);
                                    }
                                    else
                                        putByte (0);
                                }
                            }
                            else
                            {
                                if (memFlags & FL__BASE)
                                {
                                    k = (memComp >> SH__BASE) & COMP__MASK;

                                    if (k == 4) /* ESP */
                                    {
                                        if (memFlags & FL__DISP)
                                        {
                                            if (optype [mem + 1] == T__INT)
                                            {
                                                if (isbyte (yval))
                                                {
                                                    putByte (0104 + i);
                                                    putByte (0044);
                                                    putByte (yval);
                                                }
                                                else
                                                {
                                                    putByte (0004 + i);
                                                    putByte (0045);
                                                    putDword (yval);
                                                }

                                                dRule ((rule_t *)exprComp);
                                            }
                                            else
                                            {
                                                putByte (0204 + i);
                                                putByte (0044);
                                                writeExpr ((void *)exprComp, T__DWORD, (void *)symComp, DeleteBoth, memFlags & FL__NEG);
                                            }
                                        }
                                        else
                                        {
                                            putByte (0004 + i);
                                            putByte (0044);
                                        }

                                        goto Done_ModRM;
                                    }

                                    if (k == 5) /* EBP */
                                    {
                                        if (memFlags & FL__DISP)
                                        {
                                            if (optype [mem + 1] == T__INT)
                                            {
                                                if (isbyte (yval))
                                                {
                                                    putByte (0104 + i);
                                                    putByte (0045);
                                                    putByte (yval);
                                                }
                                                else
                                                {
                                                    putByte (0204 + i);
                                                    putByte (0045);
                                                    putDword (yval);
                                                }

                                                dRule ((rule_t *)exprComp);
                                            }
                                            else
                                            {
                                                putByte (0204 + i);
                                                putByte (0045);
                                                writeExpr ((void *)exprComp, T__DWORD, (void *)symComp, DeleteBoth, memFlags & FL__NEG);
                                            }
                                        }
                                        else
                                        {
                                            putByte (0104 + i);
                                            putByte (0045);
                                            putByte (0);
                                        }

                                        goto Done_ModRM;
                                    }

                                    /* Other Register */
                                    if (memFlags & FL__DISP)
                                    {
                                        if (optype [mem + 1] == T__INT)
                                        {
                                            if (isbyte (yval))
                                            {
                                                putByte (0100 + i + k);
                                                putByte (yval);
                                            }
                                            else
                                            {
                                                putByte (0200 + i + k);
                                                putDword (yval);
                                            }

                                            dRule ((rule_t *)exprComp);
                                        }
                                        else
                                        {
                                            putByte (0200 + i + k);
                                            writeExpr ((void *)exprComp, T__DWORD, (void *)symComp, DeleteBoth, memFlags & FL__NEG);
                                        }
                                    }
                                    else
                                        putByte (0000 + i + k);
                                }
                                else
                                {
                                    putByte (0005 + i);
                                    writeExpr ((void *)exprComp, T__DWORD, (void *)symComp, DeleteBoth, memFlags & FL__NEG);
                                }
                            }
                        }

            Done_ModRM: if (symComp) delete ((void *)symComp);
                        break;

                    case __ib:
                        writeExpr ((void *)opvalue [ib], T__BYTE, (void *)opvalue [ib+1], DeleteBoth, 0);
                        if (opvalue [ib+1]) delete ((void *)opvalue [ib+1]);
                        break;

                    case __iw:
                        writeExpr ((void *)opvalue [iw], T__WORD, (void *)opvalue [iw+1], DeleteBoth, 0);
                        if (opvalue [iw+1]) delete ((void *)opvalue [iw+1]);
                        break;

                    case __id:
                        writeExpr ((void *)opvalue [id], T__DWORD, (void *)opvalue [id+1], DeleteBoth, 0);
                        if (opvalue [id+1]) delete ((void *)opvalue [id+1]);
                        break;

                    case __db:
                        if (cbits == 32) goto md; else goto mw;

                    case __dw:
                    mw: if ((u67 && cbits == 16) || (!u67 && cbits == 32)) goto dd;
                    dw: writeExpr ((void *)opvalue [dat+1], T__WORD, (void *)opvalue [dat+2], DeleteBoth, 0);
                        if (opvalue [dat+2]) delete ((void *)opvalue [dat+2]);
                        break;

                    case __dd:
                    md: if ((u67 && cbits == 32) || (!u67 && cbits == 16)) goto dw;
                    dd: writeExpr ((void *)opvalue [dat+1], T__DWORD, (void *)opvalue [dat+2], DeleteBoth, 0);
                        if (opvalue [dat+2]) delete ((void *)opvalue [dat+2]);
                        break;

                    case __idw:
                        writeExpr ((void *)opvalue [offs+1], T__WORD, (void *)opvalue [offs+2], DeleteBoth, 0);
                        if (opvalue [offs+2]) delete ((void *)opvalue [offs+2]);
                        break;

                    case __idd:
                        writeExpr ((void *)opvalue [offs+1], T__DWORD, (void *)opvalue [offs+2], DeleteBoth, 0);
                        if (opvalue [offs+2]) delete ((void *)opvalue [offs+2]);
                        break;

                    case __cb:
                        writeRel ((void *)opvalue [cb], T__BYTE, (void *)opvalue [cb+1], DeleteBoth);
                        if (opvalue [cb+1]) delete ((void *)opvalue [cb+1]);
                        break;

                    case __cw:
                        writeRel ((void *)opvalue [cw], T__WORD, (void *)opvalue [cw+1], DeleteBoth);
                        if (opvalue [cw+1]) delete ((void *)opvalue [cw+1]);
                        break;

                    case __cd:
                        writeRel ((void *)opvalue [cd], T__DWORD, (void *)opvalue [cd+1], DeleteBoth);
                        if (opvalue [cd+1]) delete ((void *)opvalue [cd+1]);
                        break;

                    case __pd:
                        if (optype [pd] == OP__PTR32)
                        {
                            writeExpr ((void *)opvalue [pd], T__WORD, (void *)opvalue [pd+1], DeleteBoth, 0);
                            if (opvalue [pd+1]) delete ((void *)opvalue [pd+1]);

                            writeExpr ((void *)opvalue [pd+2], T__WORD, (void *)opvalue [pd+3], DeleteBoth, 0);
                            if (opvalue [pd+3]) delete ((void *)opvalue [pd+3]);
                        }
                        else
                        {
                            writeExpr ((void *)opvalue [pd], T__DWORD, (void *)opvalue [pd+1], DeleteBoth, 0);
                            if (opvalue [pd+1]) delete ((void *)opvalue [pd+1]);
                        }

                        break;

                    case __pf:
                        if (optype [pd] == OP__PTR48)
                        {
                            writeExpr ((void *)opvalue [pd], T__DWORD, (void *)opvalue [pd+1], DeleteBoth, 0);
                            if (opvalue [pd+1]) delete ((void *)opvalue [pd+1]);

                            writeExpr ((void *)opvalue [pd+2], T__WORD, (void *)opvalue [pd+3], DeleteBoth, 0);
                            if (opvalue [pd+3]) delete ((void *)opvalue [pd+3]);
                        }
                        else
                        {
                            writeExpr ((void *)opvalue [pd], T__FWORD, (void *)opvalue [pd+1], DeleteBoth, 0);
                            if (opvalue [pd+1]) delete ((void *)opvalue [pd+1]);
                        }

                        break;

                    case __pq:
                        writeExpr ((void *)opvalue [pd], T__QWORD, (void *)opvalue [pd+1], DeleteBoth, 0);
                        if (opvalue [pd+1]) delete ((void *)opvalue [pd+1]);
                        break;
                }
            }

       Bye: if (!fast) delete (p);
    }

    ACTION (instruction)
    {
            assemble_instruction (argv [0]);
    }

    ACTION (statement)
    {
        static unsigned long reptCount [MAX_DEPTH];
        static unsigned reptLine [MAX_DEPTH];
        static list_t *reptList [MAX_DEPTH];
        void *sP, *sS, *o1, *o2, *t;
        IntConst Result;
        unsigned long Value;
        rule_t *Expr;

            if (!form)
            {
                Result = EvalExprEx (Expr = popRule ());
                if (!ValidInteger (Result))
                {
                    IntConstDelete (Result);
                    dRule (Expr);

                    intExp ();
                    return;
                }

                Value = gFracGetLong (Result.Value, 0);
                IntConstDelete (Result);
                dRule (Expr);

                if (level == MAX_DEPTH - 1)
                {
                    pmsg ("*maximum macro depth reached!");
                    skipMacro = 1;
                }
                else
                {
                    reptCount [level] = Value;
                    reptList [level] = new (list_t);
                    reptLine [level++] = linenum + 1;
                }
            }

            if (form == 2 && !skipMacro)
                list__add (reptList [level - 1], new_strn (dups (argv [0] + 1)));

            if (form != 1) return;

            if (skipMacro)
            {
                skipMacro = 0;
                return;
            }

            n = level - 1;

            if (reptCount [n] >= 1 && COUNT(reptList [n]))
            {
                sP = p__savectx ();
                sS = s__savectx ();

                t = dups (getNextLine (4, NULL));

                o1 = getNextLine (2, NULL);
                o2 = s__string;

                while (reptCount [n]--)
                    parseList (reptLine [n], reptList [n]);

                getNextLine (5, t);
                delete (t);

                getNextLine (3, (void *)o1);
                s__string = o2;

                p__loadctx (sP);
                s__loadctx (sS);
            }

            level = n;
    }
