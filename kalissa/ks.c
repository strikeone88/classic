/*
    KS.C

    Kalissa Document Formatter Version 0.11

    Copyright (C) 2007-2009 RedStar Technologies
    Written by J. Palencia (ilina@bloodykisses.zzn.com)
*/

    #include <gstring.h>
    #include <limits.h>
    #include <stdlib.h>
    #include <stdarg.h>
    #include <pcc.h>
    #include <gutils.h>

    #include "sguifont.h"
    #include "guifont.h"

    #include "ks-tga.h"

    #include "bf2.h"
    #include "rfx.h"

    /* Global buffer. */
    static char gbuf [512];

    /* Count of errors. */
    static unsigned errc;

    /* Current input source file. */
    static char *fname;

    /* This is the global text buffer. */
    char text_buf [128];

    /* Currently selected surface, primary surface and mask. */
    static const surface_t *surface, *psurface, *surfacemask;

    /* Initial coords of the prev surface and some indicators. */
    static int psx, psy, strict, doplot;

    /* Packs an RGB color. */
    static long packpix (int r, int g, int b)
    {
            return ((long)r << 16) | ((long)g << 8) | (long)b;
    }

    /* Reads a pixel from a surface returns white if out of bounds. */
    static void sgetpixel (const surface_t *s, int x, int y, int *r, int *g, int *b)
    {
            if (x < 0 || y < 0 || x >= s->width || y >= s->height)
            {
                *r = *g = *b = 0xFF;
                return;
            }

            w__ReadPixel (s, x, y, r, g, b);
    }

    /* Reads a pixel from the surface. */
    static void getpixel (int x, int y, int *r, int *g, int *b)
    {
            if (x < 0 || y < 0 || x >= surface->width || y >= surface->height)
            {
                if (surfacemask)
                {
                    x = psx+x;
                    y = psy+y;

                    if (x >= 0 && y >= 0 && x < psurface->width && y < psurface->height)
                    {
                        w__ReadPixel (psurface, x, y, r, g, b);
                        return;
                    }
                }

                *r = *g = *b = 0;
                return;
            }

            if (surfacemask)
            {
                w__ReadPixel (surfacemask, x, y, r, g, b);

                if (*b == 0xFF)
                    w__ReadPixel (psurface, psx+x, psy+y, r, g, b);
                else
                    w__ReadPixel (surface, x, y, r, g, b);
            }
            else
                w__ReadPixel (surface, x, y, r, g, b);
    }

    /* Writes a pixel to the surface. */
    void putpixel (int x, int y, long c)
    {
            if (surfacemask)
                w__WritePixel (surfacemask, x, y, 0x00);

            w__WritePixel (surface, x, y, c);
    }

    /* Writes a pixel to the surface and the mask. */
    void mputpixel (int x, int y, int y2, long c)
    {
        static int r, g, b;

            if (surfacemask && doplot)
            {
                if (strict)
                {
                    w__ReadPixel (surfacemask, x, y2, &r, &g, &b);

                    if (b == 0xFF)
                        w__WritePixel (surfacemask, x, y2, 0x50);
                }
                else
                    w__WritePixel (surfacemask, x, y2, 0x00);
            }

            w__WritePixel (surface, x, y, c);
    }

    /* Prints a message on the screen. */
    void pmsg (const char *s, ...)
    {
        static int newline = 0;
        static va_list args;
        static char *msg;
        int cr = 0;

            if (s == NULL)
            {
                newline = cr = 0;
                return;
            }

            va_start (args, s);

            switch (*s++)
            {
                case '*':   msg = " (error)";
                            errc++;
                            break;

                case '#':   msg = " (warning)";
                            break;

                case '&':   vprintf (s, args);
                            return;

                case '@':   msg = "";
                            cr = 1;
                            break;

                default:    msg = "";
                            s--;
            }

            if (newline && !cr) printf ("\n"); else newline = 1;

            if (cr) printf ("\r");

            if (fname)
                printf ("%s %u%s: ", fname, linenum, msg);
            else
                printf ("kalissa%s: ", msg);

            vprintf (s, args);
    }

    /* Adds an extension to the file is none is given. */
    static char *add_ext (char *s, char *e)
    {
        char *p, *q = s;

            strcpy (gbuf, s);
            q = s = gbuf;

            for (p = s; *s != '\0'; s++) if (*s == '\\') p = s;
            if (*p == '\\') p++;

            if (strchr (p, '.') != NULL) return q;

            return dups (strcat (gbuf, e));
    }

    void ks__init (void);
    void ks__deinit (void);

/****************************************************************************/

    int main (int argc, char *argv [])
    {
        char *s, *p, *kst = "kalissa$.ks";
        int i, usebecky = 0, index = 1;
        FILE *fp;

            printf ("RedStar Kalissa Version 0.11 Copyright (C) 2008 J. Palencia (zipox@ureach.com)\n");

            if (set_font (guifont))
            {
                pmsg ("*unable to set default rfx font.\n");
                return 2;
            }

            if (bf2__setfont (sguifont, sizeof(sguifont)))
            {
                pmsg ("*unable to set default bf2 font.\n");
                return 2;
            }

            if (argc < 2)
            {
                printf ("Syntax: ks source-files%-28s* Build Date: %s\n\n", "", __DATE__);

                return 1;
            }

            fname = NULL;
            errc = 0;

            if (!strcmp (argv [index], "-bp"))
            {
                usebecky = 1;
                index++;
            }

            if (index > argc - 1)
            {
                pmsg ("*no source files were specified!!\n");
                return 2;
            }

            while (index < argc)
            {
                s = add_ext (argv [index++], ".ks");

                if (usebecky)
                {
                    pmsg ("preprocessing %s...\n\n", s);

                    remove (kst);

                    sprintf (text_buf, "bp %s %s", s, kst);
                    system (text_buf);

                    pmsg (NULL);

                    fp = fopen (kst, "rt");
                    if (fp == NULL)
                    {
                        pmsg ("*unable to open input file '%s'", kst);
                        continue;
                    }
                }
                else
                {
                    fp = fopen (s, "rt");
                    if (fp == NULL)
                    {
                        pmsg ("*unable to open input file '%s'", s);
                        continue;
                    }
                }

                pmsg ("processing %s", s);

                ks__init ();

                fname = s = dups (s);
                errc = 0;

                setinputstream (fp);
                __strcmp = stricmp;

                i = parseinput ();

                ks__deinit ();

                fclose (fp);

                fname = NULL;

                if (!errc && i) pmsg ("%s successfully processed.", s);

                delete (s);
            }

            if (usebecky) remove (kst);

            pmsg ("finished.\n");

            return 0;
    }

    void __catcher (unsigned r, unsigned e, unsigned st)
    {
            pmsg ("*bad syntax: r%u e%u s%u (w%u)", r, e, st, colnum);
    }

/*****************************************************************************
*****************************************************************************/

    #define KC__MASK        0xFF000000
    #define KC__DATA        0x00FFFFFF
    #define KC__PATTERN     0xFF000000
    #define KC__SOLID       0xFE000000
    #define KC__HOLLOW      0xFD000000
    #define KC__DEFAULT     0xFC000000

    #define KT__RECT        0
    #define KT__COLUMN      1
    #define KT__ROW         2
    #define KT__TEXT_BF2    3
    #define KT__TEXT_RFX    4
    #define KT__IN_BMP      5
    #define KT__ICOLUMN     6
    #define KT__IROW        7
    #define KT__FILTERL     8
    #define KT__FILTER      9
    #define KT__BF2_ALIAS   10
    #define KT__FADER       11
    #define KT__MIRROR      12
    #define KT__SPRAY       13
    #define KT__NEG         14
    #define KT__GRAY        15
    #define KT__ENHANCE     16
    #define KT__STRETCH     17
    #define KT__CLAYER      18

    #define KP__CENTER      -1
    #define KP__AUTO        -2
    #define KP__PER         -3
    #define KP__APER        -104
    #define KP__REL         -205

    #define KPT__HORZ       0
    #define KPT__VERT       1
    #define KPT__HLINE      2
    #define KPT__VLINE      3

    #define KPT__LRDIAG     4
    #define KPT__RLDIAG     5

    #define KPT__UP         4
    #define KPT__DN         5
    #define KPT__LE         6
    #define KPT__RI         7

    #define KB__DOUBLE      0
    #define KB__SINGLE      1
    #define KB__VOID        2

    static list_t *constants;
    static list_t *objstack;
    static list_t *fonts;

    typedef struct
    {
        linkable_t  link;
        long        value;
    }
    integer_t;

    typedef struct
    {
        linkable_t  link;

        int         astype;
        int         type;

        int         h_margin_l;
        int         h_margin_r;
        int         v_margin_u;
        int         v_margin_d;

        int         st_x, left_w, st_y, left_h;
        int         x, y, w, h, px, py;
        int         clip, ix, iy, iw, ih;

        long        patc1, patc2;
        long        fgc, bg, bc;

        long        stransp, transp, prim, iprim;
        int         dir;

        int         blend, subtractive, reverse;
        int         redL, redH, greenL, greenH, blueL, blueH;
        long        mA, mB;

        int         step, border, space;

        char        *rfx, *bf2, *face;
        char        *value;

        const       surface_t *surfacemask;
        const       surface_t *surface;

        int         psx, psy, align;

        int         nodraw;
        int         pre;

        char        *file1;
        char        *file2;
    }
    object_t;

    typedef struct
    {
        linkable_t  link;
        char        *name;

        void        *data;
        unsigned    len;
    }
    font_t;

    object_t *cobj;

    char *string;

    static float grid [16][16];

    static int grw, grh, grs;
    static float grr, grg, grb, grhw, grhh, grm;

    static int fill_grid (char *x)
    {
        int i, j, k;

            switch (*x)
            {
                case '*':
                    grm = 1;
                    x++;

                    grr = atof (strtok (x, ","));
                    grg = atof (strtok (NULL, ","));
                    grb = atof (strtok (NULL, ","));

                    return 1;

                case '$':
                    grm = 2;
                    x++;
                    break;

                case '!':
                    grm = 3;
                    x++;

                    grw = *x++ & 0xDF;

                    grr = atof (strtok (x, ","));
                    grg = atof (strtok (NULL, ","));
                    grb = atof (strtok (NULL, ","));

                    return 1;

                default:
                    grm = 0;
            }

            grw = atof (strtok (x, ","));
            grh = atof (strtok (NULL, ","));
            k = atoi (strtok (NULL, ","));

            if (grm == 2)
            {
                grs = grw;
                grr = grh;
                grg = k;
                grb = atof (strtok (NULL, ","));

                return 1;
            }

            grs = 0;

            if (grw < 1 || grh < 1 || grw > 15 || grh > 15 ||
                !(grw & 1) || !(grh & 1))
                return 0;

            grhw = grw >> 1;
            grhh = grh >> 1;

            memset (grid, 0, sizeof(grid));

            for (j = 0; j < grh; j++)
            {
                for (i = 0; i < grw; i++)
                {
                    x = strtok (NULL, ",");
                    if (x != NULL) grs += (grid [j][i] = atof (x));
                }
            }

            if (k) grs = k;

            if (!grs) return 0;

            return 1;
    }

    static void add_font (char *name, char *source)
    {
        font_t *font = new (font_t);
        FILE *fp; void *data;
        unsigned len;

            if (gsearch (fonts, name) != NULL)
            {
                pmsg ("#font name already used (override ignored)");
                return;
            }

pmsg ("loading font '%s' as %s", source, name);

            fp = fopen (source, "rb");
            if (fp == NULL)
            {
                pmsg ("*unable to open font file '%s'", source);
                p__finish = 1;
                return;
            }

            fseek (fp, 0, SEEK_END);
            font->len = len = ftell (fp);
            fseek (fp, 0, SEEK_SET);

            fread (data = alloc (len), len, 1, fp);

            fclose (fp);

            font->name = name;
            font->data = data;

            list__add (fonts, font);

            pmsg ("&, done");
    }

    static void ks__init (void)
    {
            constants = new (list_t);
            objstack = new (list_t);
            fonts = new (list_t);

            surfacemask = NULL;

            psurface = NULL;
            surface = NULL;

            strict = 0;

            cobj = NULL;
    }

    static void ks__deinit (void)
    {
        char *s;

            if (surface != NULL)
            {
pmsg ("closing surface");
                s = surface->output;
                ((surface_t *)surface)->abort = errc != 0;
                w__CloseSurface (surface);
                delete (s);
pmsg ("&, done");
            }

            delete (constants);
    }

    static void push_constant (long x)
    {
        integer_t *integer = new (integer_t);

            integer->value = x;

            list__add (constants, integer);
    }

    int push_expr (int form, int n, char *argv [])
    {
            push_constant (atol (argv [0]));
            return 0;
    }

    static long pop_constant (void)
    {
        integer_t *integer = list__pop (constants);
        long value;

            if (integer == NULL)
            {
                pmsg ("#internal bug extracting from empty stack");
                return 0;
            }

            value = integer->value;
            delete (integer);

            return value;
    }

    void set_string (int form, int n, char *argv [])
    {
        char *p;

            string = p = dups (chop (argv [0]) + 1);

            while ((p = strchr (p, '\n')) != NULL)
                *p++ = 0x20;
    }

    void append_string (int form, int n, char *argv [])
    {
        char *p, *s;

            if (form == 2) return;

            s = p = chop (argv [1] + 1);

            while ((p = strchr (p, '\n')) != NULL)
                *p++ = 0x20;

            n = strlen (s) + strlen (string) + 1;
            p = alloc (n);

            strcpy (p, string);
            strcat (p, s);

            delete (string);

            string = p;
    }

    static void set_default (object_t *x)
    {
        object_t *p = BOT(objstack);

            x->w = KP__AUTO;
            x->h = KP__AUTO;

            x->iw = KP__AUTO;
            x->ih = KP__AUTO;

            x->ix = KP__AUTO;
            x->iy = KP__AUTO;

            x->stransp = KC__DEFAULT;
            x->transp = KC__DEFAULT;
            x->prim = KC__DEFAULT;
            x->iprim = KC__DEFAULT;

            x->mA = 100;
            x->mB = 100;

            x->redH = 255;
            x->greenH = 255;
            x->blueH = 255;

            x->border = KB__VOID;
            x->fgc = KC__DEFAULT;
            x->space = 5;

            x->dir = KP__AUTO;
            x->astype = -1;

            if (p != NULL) /* Inheritable attributes */
            {
                x->space = p->space;
                x->face = p->face;
                x->rfx = p->rfx;
                x->bf2 = p->bf2;
                x->face = p->face;
                x->fgc = p->fgc;
                x->align = p->align;
            }
    }

    static char *mkfil (char *ext)
    {
        char *s, *p;

            s = add_ext (string, ext);
            if (s != string) delete (string);

            for (p = s; *p != '\0'; p++) if (*p == '/') *p = '\\';

            return s;
    }

    static char *xlate (char *s)
    {
        #define Mapped  7

        static char *spec [] =
        {
            "lt", "gt", "amp", "quot", "tab", "longtab", "s",

            "iexcl", "cent", "pound", "curren", "yen", "brvbar", "sect",
            "uml", "copy", "ordf", "laquo", "not", "shy", "reg", "macr",
            "deg", "plusmn", "sup2", "sup3", "acute", "micro", "para",
            "middot", "cedil", "sup1", "ordm", "raquo", "frac14", "frac12",
            "frac34", "iquest", "Agrave", "Aacute", "Acirc", "Atilde",
            "Auml", "Aring", "AElig", "Ccedil", "Egrave", "Eacute", "Ecirc",
            "Euml", "Igrave", "Iacute", "Icirc", "Iuml", "ETH", "Ntilde", 
            "Ograve", "Oacute", "Ocirc", "Otilde", "Ouml", "times", "Oslash", 
            "Ugrave", "Uacute", "Ucirc", "Uuml", "Yacute", "THORN", "szlig",
            "agrave","aacute", "acirc", "atilde", "auml", "aring", "aelig", 
            "ccedil", "egrave", "eacute", "ecirc", "euml", "igrave", "iacute", 
            "icirc", "iuml", "eth", "ntilde", "ograve", "oacute", "ocirc", 
            "otilde", "ouml", "divide","oslash","ugrave", "uacute", "ucirc", 
            "uuml", "yacute", "thorn", "yuml",

            NULL
        };

        static char map [] =
        {
            '<', '>', '&', '"', '\t', '\b', ' '
        };

        char *f, *p, *q, *o = s;
        int i;

            while ((p = strchr (s, '\\')) != NULL)
            {
                switch (p [1])
                {
                    case '\\':
                        *p = '\\';
                        break;

                    case 'n':
                        *p = '\n';
                        break;
                }

                strcpy (s = p + 1, p + 2);
            }

            s = o;

            while ((p = strchr (s, '&')) != NULL)
            {
                q = strchr (f = p++, ';');
                if (q == NULL) break;

                *q = '\0';

                for (i = 0; spec [i] != NULL; i++)
                    if (!strcmp (spec [i], p)) break;

                if (spec [i] == NULL)
                {
                    s = q + 1;
                    *q = ';';
                    continue;
                }

                if (i < Mapped)
                    *f = map [i];
                else
                    *f = 161 + i - Mapped;

                if (*f == '\t' || *f == '\b')
                {
                    i = *f == '\t' ? 4 : 8;

                    p += i - 1;

                    while (i--) *f++ = 0x20;
                }

                s = p;
                q++;

                while (*q != '\0') *p++ = *q++;
                *p = '\0';
            }

            return o;
    }

    void set_attribute (int form, int n, char *argv [], unsigned ndx [])
    {
        static char *predef [] =
        {
            "",
            "3,3,0,1,3,1,3,9,3,1,3,1", /* soften-lo */
            "3,3,0,1,1,1,1,1,1,1,1,1", /* soften-med */
            "3,3,0,-1,-3,-1,-3,41,-3,-1,-3,-1", /* sharpen-lo */
            "3,3,0,-1,-1,-1,-1,49,-1,-1,-1,-1", /* sharpen-med */
            "3,3,0,1,0,1,0,0,0,1,0,1", /* shatter */
            "3,3,1,0,0,-1,1,0,0,0,0,0", /* edge-enhance */
            "3,3,1,0,1,0,1,-4,1,0,1,0", /* edge-detect */
        };

            if (cobj == NULL) set_default (cobj = new (object_t));

            switch (form)
            {
                case 0x00: /* value */
                    cobj->value = xlate (string);
                    break;

                case 0x01: /* h-margins */
                    cobj->h_margin_r = pop_constant ();
                    cobj->h_margin_l = pop_constant ();
                    break;

                case 0x02: /* v-margins */
                    cobj->v_margin_d = pop_constant ();
                    cobj->v_margin_u = pop_constant ();
                    break;

                case 0x03: /* border */
                    cobj->border = pop_constant ();
                    cobj->bc = pop_constant ();
                    break;

                case 0x04: /* x */
                    cobj->x = pop_constant ();
                    break;

                case 0x05: /* y */
                    cobj->y = pop_constant ();
                    break;

                case 0x06: /* w */
                    cobj->w = pop_constant ();
                    break;

                case 0x07: /* h */
                    cobj->h = pop_constant ();
                    break;

                case 0x08: /* bg */
                    cobj->bg = pop_constant ();
                    cobj->step = pop_constant ();
                    cobj->patc2 = pop_constant ();
                    cobj->patc1 = pop_constant ();
                    break;

                case 0x09: /* fgc */
                    cobj->fgc = pop_constant ();
                    break;

                case 0x0A: /* rfx-face */
                    cobj->rfx = dups (argv [2]);
                    break;

                case 0x0B: /* bf2-face */
                    cobj->bf2 = dups (argv [2]);
                    break;

                case 0x0C: /* face */
                    cobj->face = dups (argv [2]);
                    break;

                case 0x0D: /* font-space */
                    cobj->space = pop_constant ();
                    break;

                case 0x0E: /* value (predef-filter-code) */
                    cobj->value = dups (predef [ndx [2]]);
                    break;

                case 0x0F: /* as: column */
                    cobj->astype = KT__COLUMN;
                    break;

                case 0x10: /* as: row */
                    cobj->astype = KT__ROW;
                    break;

                case 0x11: /* ix */
                    cobj->ix = pop_constant ();
                    break;

                case 0x12: /* iy */
                    cobj->iy = pop_constant ();
                    break;

                case 0x13: /* iw */
                    cobj->iw = pop_constant ();
                    break;

                case 0x14: /* ih */
                    cobj->ih = pop_constant ();
                    break;

                case 0x15: /* tcolor */
                    cobj->transp = pop_constant ();
                    break;

                case 0x16: /* pcolor */
                    cobj->prim = pop_constant ();
                    break;

                case 0x17: /* dir */
                    cobj->dir = pop_constant ();
                    break;

                case 0x18: /* blend */
                    cobj->blend = ndx [2];
                    break;

                case 0x19: /* blend */
                    cobj->mA = pop_constant ();
                    cobj->blend = 1;
                    break;

                case 0x1A: /* s-tcolor */
                    cobj->stransp = pop_constant ();
                    break;

                case 0x1B: /* indirect value */
                    cobj->value = xlate (string);
                    break;

                case 0x1C: /* align */
                    cobj->align = ndx [2];
                    break;

                case 0x1D: /* align */
                    cobj->align = 3;
                    break;

                case 0x1E: /* i-pcolor */
                    cobj->iprim = pop_constant ();
                    break;

                case 0x1F: /* additive */
                    cobj->subtractive = !ndx [2];
                    break;

                case 0x20: /* reverse-order */
                    cobj->reverse = ndx [2];
                    break;

                case 0x21: /* blend */
                    cobj->mB = pop_constant ();
                    cobj->mA = pop_constant ();
                    cobj->blend = 1;
                    break;

                case 0x22: /* blend */
                    cobj->mB = pop_constant ();
                    cobj->blend = 1;
                    break;

                case 0x23: /* red */
                    cobj->redH = pop_constant ();
                    cobj->redL = pop_constant ();
                    break;

                case 0x24: /* red */
                    cobj->redH = pop_constant ();
                    cobj->redL = 0;
                    break;

                case 0x25: /* green */
                    cobj->greenH = pop_constant ();
                    cobj->greenL = pop_constant ();
                    break;

                case 0x26: /* green */
                    cobj->greenH = pop_constant ();
                    cobj->greenL = 0;
                    break;

                case 0x27: /* blue */
                    cobj->blueH = pop_constant ();
                    cobj->blueL = pop_constant ();
                    break;

                case 0x28: /* blue */
                    cobj->blueH = pop_constant ();
                    cobj->blueL = 0;
                    break;

                case 0x29: /* first */
                    cobj->pre = ndx [2];
                    break;
            }
    }

    void coord_value (int form)
    {
            if (form == 2) push_constant (KP__CENTER);
            if (form == 3) push_constant (KP__AUTO);
    }

    void set_percentile (int form)
    {
            form = form ? KP__APER : KP__PER;

            push_constant (-(pop_constant () % 100) + form);
    }

    void dim_value (int form)
    {
            switch (form)
            {
                case 0x02:
                    push_constant (KP__AUTO);
                    break;

                case 0x03:
                    push_constant (-pop_constant () + KP__REL);
                    break;

                case 0x04:
                    push_constant (8192);
                    break;
            }
    }

    void color_spec (int form, int n, char *argv [], unsigned ndx [])
    {
        static long def_colors [] =
        {
            0x000000, 0x0000FF, 0x00FF00, 0xFF0000, 0xFFFFFF, 0xFFFF00
        };

        static long r;

            if (form == 1)
            {
                push_constant (def_colors [ndx [0]]);
                return;
            }

            if (!form)
            {
                r = pop_constant ();
                r |= pop_constant () << 8;
                r |= pop_constant () << 16;
            }
            else
            {
                gbuf [2] = '\0';

                gbuf [0] = argv [0][1];
                gbuf [1] = argv [0][2];
                r = atov (gbuf, R__HEX) << 16;

                gbuf [0] = argv [0][3];
                gbuf [1] = argv [0][4];
                r |= atov (gbuf, R__HEX) << 8;

                gbuf [0] = argv [0][5];
                gbuf [1] = argv [0][6];
                r |= atov (gbuf, R__HEX);
            }

            push_constant (r);
    }

    void bg_value (int form, int n, char *argv [], unsigned ndx [])
    {
        unsigned long p, q;

            switch (form)
            {
                case 0x00:  push_constant (1);
                            push_constant (KC__PATTERN | ndx [2]);
                            break;

                case 0x01:  push_constant (KC__PATTERN | ndx [2]);
                            break;

                case 0x02:  p = pop_constant ();
                            push_constant (0);
                            push_constant (p);
                            push_constant (KC__PATTERN | ndx [2]);
                            break;

                case 0x03:  p = pop_constant ();
                            push_constant (0);
                            push_constant (0);
                            push_constant (0);
                            push_constant (KC__SOLID | p);
                            break;

                case 0x04:  push_constant (0);
                            push_constant (0);
                            push_constant (0);
                            push_constant (KC__HOLLOW);
                            break;
            }
    }

    void border_value (int form)
    {
            switch (form)
            {
                case 0x00:  push_constant (KB__DOUBLE);
                            break;

                case 0x01:  push_constant (KB__SINGLE);
                            break;

                case 0x02:  push_constant (0);
                            push_constant (KB__VOID);
                            break;
            }
    }

    static int calc_per (int p, int t, int q)
    {
            return ((long)(-p + q))*t / 100;
    }

    static void drawBox (int x, int y, int w, int h, long c)
    {
            w__HorzLine (surface, x, y, w, c);
            if (surfacemask) w__HorzLine (surfacemask, x, y, w, 0);

            w__HorzLine (surface, x, y+h-1, w, c);
            if (surfacemask) w__HorzLine (surfacemask, x, y+h-1, w, 0);

            w__VertLine (surface, x, y, h, c);
            if (surfacemask) w__VertLine (surfacemask, x, y, h, 0);

            w__VertLine (surface, x+w-1, y, h, c);
            if (surfacemask) w__VertLine (surfacemask, x+w-1, y, h, 0);
    }

    static void DrawFader (object_t *obj)
    {
        int r, g, b, i, j, x, y, w, h, z1, z2, vert;
        long tcolor, pcolor, temp, lr, lg, lb;
        float k, k1, k2, ik;

            tcolor = obj->transp;
            pcolor = obj->prim;

            x = obj->x;
            y = obj->y;
            w = obj->w;
            h = obj->h;

            if (strchr (obj->value, ','))
            {
                k1 = atof (strtok (obj->value, ","));
                k2 = atof (strtok (NULL, ","));
            }
            else
                k1 = k2 = atof (obj->value);

            if (obj->dir == KP__AUTO) obj->dir = 0;

            if (obj->dir)
            {
                z1 = h;
                z2 = w;
            }
            else
            {
                z1 = w;
                z2 = h;
            }

            vert = obj->dir;

            ik = ((k2 - k1 + 1) * 128) / z1;
            k2 = (k1 * 128);

            for (j = 0; j < z2; j++)
            {
                for (i = 0, k1 = k2; i < z1; i++, k1 += ik)
                {
                    k = k1 / 128;

                    if (vert)
                        getpixel (j + x, i + y, &r, &g, &b);
                    else
                        getpixel (i + x, j + y, &r, &g, &b);

                    temp = packpix (r, g, b);
                    if (temp == tcolor) continue;

                    if (pcolor != KC__DEFAULT && pcolor != temp)
                        continue;

                    r = (k * r) / 100L;
                    g = (k * g) / 100L;
                    b = (k * b) / 100L;

                    if (r < 0) r = 0;
                    if (g < 0) g = 0;
                    if (b < 0) b = 0;

                    if (r > 255) r = 255;
                    if (b > 255) b = 255;
                    if (g > 255) g = 255;

                    if (vert)
                        putpixel (j + x, i + y, packpix(r, g, b));
                    else
                        putpixel (i + x, j + y, packpix(r, g, b));
                }

                pmsg ("@fading: %lu%% completed", j*100L/z2);
            }

            pmsg ("@fading: %lu%% completed", j*100L/z2);
    }

    static void DrawMirror (object_t *obj)
    {
        int i, j, x, y, p, q1, q2, w, h, hl, r, g, b;
        long t1, t2;

            x = obj->x;
            y = obj->y;
            w = obj->w;
            h = obj->h;

            if (obj->dir)
            {
                q2 = y + h - 1;
                q1 = y;

                hl = h >> 1;

                for (j = 0; j < hl; j++, q1++, q2--)
                {
                    for (i = 0, p = x; i < w; i++, p++)
                    {
                        getpixel (p, q1, &r, &g, &b);
                        t1 = packpix (r, g, b);

                        getpixel (p, q2, &r, &g, &b);
                        t2 = packpix (r, g, b);

                        putpixel (p, q1, t2);
                        putpixel (p, q2, t1);
                    }

                    pmsg ("@mirroring: %lu%% completed", j*100L/hl);
                }

                pmsg ("@mirroring: %lu%% completed", j*100L/hl);
            }
            else
            {
                q2 = x + w - 1;
                q1 = x;

                hl = w >> 1;

                for (i = 0; i < hl; i++, q1++, q2--)
                {
                    for (j = 0, p = y; j < h; j++, p++)
                    {
                        getpixel (q1, p, &r, &g, &b);
                        t1 = packpix (r, g, b);

                        getpixel (q2, p, &r, &g, &b);
                        t2 = packpix (r, g, b);

                        putpixel (q1, p, t2);
                        putpixel (q2, p, t1);
                    }

                    pmsg ("@mirroring: %lu%% completed", i*100L/hl);
                }

                pmsg ("@mirroring: %lu%% completed", i*100L/hl);
            }
    }

    static void DrawFrame (object_t *obj)
    {
        int i, j, k, x, y, w, h;
        long c, bg, pc1, pc2;

            x = obj->x;
            y = obj->y;
            w = obj->w;
            h = obj->h;

            bg = obj->bg;
            c = obj->bc;

            if (obj->border == KB__VOID) goto Fill;

            drawBox (x, y, w, h, c);
            x++;    y++;    w-=2;   h-=2;

            if (obj->border == KB__SINGLE) goto Fill;

            drawBox (x, y, w, h, c);
            x++;    y++;    w-=2;   h-=2;

        Fill:;
            c = bg & KC__MASK;
            if (c == KC__HOLLOW) return;

            if (c == KC__PATTERN)
            {
                switch (bg & KC__DATA)
                {
                    case KPT__HORZ:
                        j = obj->step;

                        pmsg ("");

                        for (i = 0; i < h; )
                        {
                            if (surfacemask)
                            {
                                for (k = 0; i < h && k < j; k++, i++)
                                {
                                    w__HorzLine (surface, x, y, w, obj->patc1);
                                    w__HorzLine (surfacemask, x, y++, w, 0);
                                }

                                for (k = 0; i < h && k < j; k++, i++)
                                {
                                    w__HorzLine (surface, x, y, w, obj->patc2);
                                    w__HorzLine (surfacemask, x, y++, w, 0);
                                }
                            }
                            else
                            {
                                for (k = 0; i < h && k < j; k++, i++)
                                    w__HorzLine (surface, x, y++, w, obj->patc1);

                                for (k = 0; i < h && k < j; k++, i++)
                                    w__HorzLine (surface, x, y++, w, obj->patc2);
                            }

                            pmsg ("@bg (horz): %lu%% completed", i*100L/h);
                        }

                        pmsg ("@bg (horz): %lu%% completed", i*100L/h);

                        break;

                    case KPT__VERT:
                        j = obj->step;

                        pmsg ("");

                        for (i = 0; i < w; )
                        {
                            if (surfacemask)
                            {
                                for (k = 0; i < w && k < j; k++, i++)
                                {
                                    w__VertLine (surface, x, y, h, obj->patc1);
                                    w__VertLine (surfacemask, x++, y, h, 0);
                                }

                                for (k = 0; i < w && k < j; k++, i++)
                                {
                                    w__VertLine (surface, x, y, h, obj->patc2);
                                    w__VertLine (surfacemask, x++, y, h, 0);
                                }
                            }
                            else
                            {
                                for (k = 0; i < w && k < j; k++, i++)
                                    w__VertLine (surface, x++, y, h, obj->patc1);

                                for (k = 0; i < w && k < j; k++, i++)
                                    w__VertLine (surface, x++, y, h, obj->patc2);
                            }

                            pmsg ("@bg (vert): %lu%% completed", i*100L/w);
                        }

                        pmsg ("@bg (vert): %lu%% completed", i*100L/w);

                        break;

                    case KPT__HLINE:
                        j = obj->step;

                        pmsg ("");

                        if (surfacemask)
                        {
                            for (i = 0; i <= h; i += j)
                            {
                                w__HorzLine (surface, x, k = (y += j) - j, w, obj->patc1);
                                w__HorzLine (surfacemask, x, k, w, 0);

                                pmsg ("@bg (hline): %lu%% completed", i*100L/h);
                            }

                            pmsg ("@bg (hline): %lu%% completed", i*100L/h);
                        }
                        else
                        {
                            for (i = 0; i <= h; i += j)
                            {
                                w__HorzLine (surface, x, (y += j) - j, w, obj->patc1);

                                pmsg ("@bg (hline): %lu%% completed", i*100L/h);
                            }

                            pmsg ("@bg (hline): %lu%% completed", i*100L/h);
                        }

                        break;

                    case KPT__VLINE:
                        j = obj->step;

                        pmsg ("");

                        if (surfacemask)
                        {
                            for (i = 0; i <= w; i += j)
                            {
                                w__VertLine (surface, k = (x += j) - j, y, h, obj->patc1);
                                w__VertLine (surfacemask, k, y, h, 0);

                                pmsg ("@bg (vline): %lu%% completed", i*100L/w);
                            }

                            pmsg ("@bg (vline): %lu%% completed", i*100L/w);
                        }
                        else
                        {
                            for (i = 0; i <= w; i += j)
                            {
                                w__VertLine (surface, (x += j) - j, y, h, obj->patc1);

                                pmsg ("@bg (vline): %lu%% completed", i*100L/w);
                            }

                            pmsg ("@bg (vline): %lu%% completed", i*100L/w);
                        }

                        break;

                    case KPT__LRDIAG:
                        pmsg ("");

                        for (j = 0; j < h; j++)
                        {
                            for (i = 0; i < w; )
                            {
                                for (k = 0; i < w && k < obj->step; k++, i++)
                                    putpixel (x + (i + j) % w, y + j, obj->patc1);

                                for (k = 0; i < w && k < obj->step; k++, i++)
                                    putpixel (x + (i + j) % w, y + j, obj->patc2);
                            }

                            pmsg ("@bg (lrdiag): %lu%% completed", j*100L/h);
                        }

                        pmsg ("@bg (lrdiag): %lu%% completed", j*100L/h);

                        break;

                    case KPT__RLDIAG:
                        pmsg ("");

                        for (j = 0; j < h; j++)
                        {
                            for (i = 0; i < w; )
                            {
                                for (k = 0; i < w && k < obj->step; k++, i++)
                                    putpixel (x + w - 1 - (i + j) % w, y + j, obj->patc1);

                                for (k = 0; i < w && k < obj->step; k++, i++)
                                    putpixel (x + w - 1 - (i + j) % w, y + j, obj->patc2);
                            }

                            pmsg ("@bg (rldiag): %lu%% completed", j*100L/h);
                        }

                        pmsg ("@bg (rldiag): %lu%% completed", j*100L/h);

                        break;
                }

                return;
            }

            if (c == KC__SOLID)
            {
                bg &= KC__DATA;

                if (surfacemask)
                {
                    for (i = 0; i < h; i++)
                    {
                        w__HorzLine (surface, x, y, w, bg);
                        w__HorzLine (surfacemask, x, y++, w, 0);
                    }
                }
                else
                    for (i = 0; i < h; i++) w__HorzLine (surface, x, y++, w, bg);
            }
    }

/*****************************************************************************
**************************** IMAGE READERS ***********************************
*****************************************************************************/

    #define DEF_W   2
    #define DEF_H   2

    static unsigned long getd (FILE *fp)
    {
        unsigned long A, B;

            A = (unsigned int)getw (fp);
            B = (unsigned int)getw (fp);

            return (B << 16) | A;
    }

    static unsigned long getf (FILE *fp, object_t *obj)
    {
        static unsigned char buf [3];

            fread (buf, 1, 3, fp);

            if (buf [2] < obj->redL) buf [2] = obj->redL;
            if (buf [2] > obj->redH) buf [2] = obj->redH;

            if (buf [1] < obj->greenL) buf [1] = obj->greenL;
            if (buf [1] > obj->greenH) buf [1] = obj->greenH;

            if (buf [0] < obj->blueL) buf [0] = obj->blueL;
            if (buf [0] > obj->blueH) buf [0] = obj->blueH;

            return *(unsigned long *)&buf;
    }

    static void getf_split (FILE *fp, object_t *obj, int *r, int *g, int *b)
    {
        static char buf [3];

            fread (buf, 1, 3, fp);

            *r = (unsigned char)buf [2];
            *g = (unsigned char)buf [1];
            *b = (unsigned char)buf [0];

            if (*r < obj->redL) *r = obj->redL;
            if (*r > obj->redH) *r = obj->redH;

            if (*g < obj->greenL) *g = obj->greenL;
            if (*g > obj->greenH) *g = obj->greenH;

            if (*b < obj->blueL) *b = obj->blueL;
            if (*b > obj->blueH) *b = obj->blueH;
    }

    static FILE *BMP__Check (object_t *obj)
    {
        FILE *fp;

            if (obj->value == NULL) return NULL;

            fp = fopen (obj->value, "rb");
            if (fp == NULL) return NULL;

            if (getw (fp) != 'BM')
            {
                fclose (fp);
                return NULL;
            }

            fseek (fp, 26, SEEK_CUR);

            if (getw (fp) != 24)
            {
                fclose (fp);
                return NULL;
            }

            rewind (fp);

            return fp;
    }

    static void BMP__Draw (object_t *obj)
    {
        unsigned long stcolor, tcolor, pcolor, temp, t2, base, len;
        unsigned long ipcolor, x, y, w, h, vt;
        int i, j, v, r, g, b, r2, g2, b2;
        FILE *fp;

            fp = BMP__Check (obj);
            if (fp == NULL) return;

            getw (fp);
            getd (fp);
            getd (fp);
            base = getd (fp);
            getd (fp); /* 40 */
            w = getd (fp);
            h = getd (fp);
            vt = h - 1;

            len = w * 3;
            len = (len + 3) & -4;

            h = obj->ih;
            w = obj->iw;

            if (obj->h < h) h = obj->h;
            if (obj->w < w) w = obj->w;

            stcolor = obj->stransp;
            tcolor = obj->transp;
            ipcolor = obj->iprim;
            pcolor = obj->prim;

            x = obj->x;
            y = obj->y;

            pmsg ("");

            if (pcolor != KC__DEFAULT || tcolor != KC__DEFAULT ||
                stcolor != KC__DEFAULT || ipcolor != KC__DEFAULT)
            {
                if (obj->blend)
                {
                    for (j = obj->iy, v = 0; v < h; j++, v++)
                    {
                        fseek (fp, (3L * obj->ix) + base + (vt - j) * len, SEEK_SET);

                        for (i = 0; i < w; i++)
                        {
                            getf_split (fp, obj, &r2, &g2, &b2);

                            temp = packpix (r2, g2, b2);
                            if (temp == tcolor) continue;

                            getpixel (x + i, y + v, &r, &g, &b);
                            t2 = packpix (r, g, b);

                            if (pcolor != KC__DEFAULT)
                                if (t2 != pcolor) continue;

                            if (ipcolor != KC__DEFAULT)
                                if (temp != ipcolor) continue;

                            if (stcolor == t2) continue;

                            r2 = obj->mA*r2 / 100L;
                            g2 = obj->mA*g2 / 100L;
                            b2 = obj->mA*b2 / 100L;

                            r = obj->mB*r / 100L;
                            g = obj->mB*g / 100L;
                            b = obj->mB*b / 100L;

                            if (obj->subtractive)
                            {
                                if (obj->reverse)
                                {
                                    r = r2 - r;
                                    g = g2 - g;
                                    b = b2 - b;
                                }
                                else
                                {
                                    r = r - r2;
                                    g = g - g2;
                                    b = b - b2;
                                }
                            }
                            else
                            {
                                    r = r + r2;
                                    g = g + g2;
                                    b = b + b2;
                            }

                            if (r < 0) r = 0; if (r > 255) r = 255;
                            if (g < 0) g = 0; if (g > 255) g = 255;
                            if (b < 0) b = 0; if (b > 255) b = 255;

                            putpixel (x + i, y + v, packpix (r, g, b));
                        }

                        pmsg ("@drawing blended BMP: %lu%% completed", v*100L/h);
                    }

                }
                else
                {
                    for (j = obj->iy, v = 0; v < h; j++, v++)
                    {
                        fseek (fp, (3L * obj->ix) + base + (vt - j) * len, SEEK_SET);

                        for (i = 0; i < w; i++)
                        {
                            temp = getf (fp, obj);
                            if (temp == tcolor) continue;

                            if (pcolor != KC__DEFAULT || stcolor != KC__DEFAULT)
                            {
                                getpixel (x + i, y + v, &r, &g, &b);
                                t2 = packpix (r, g, b);
                            }

                            if (pcolor != KC__DEFAULT)
                                if (t2 != pcolor) continue;

                            if (ipcolor != KC__DEFAULT)
                                if (temp != ipcolor) continue;

                            if (stcolor == t2) continue;

                            putpixel (x + i, y + v, temp);
                        }

                        pmsg ("@drawing BMP: %lu%% completed", v*100L/h);
                    }
                }
            }
            else
            {
                if (obj->blend)
                {
                    for (j = obj->iy, v = 0; v < h; j++, v++)
                    {
                        fseek (fp, (3L * obj->ix) + base + (vt - j) * len, SEEK_SET);

                        for (i = 0; i < w; i++)
                        {
                            getf_split (fp, obj, &r2, &g2, &b2);
                            getpixel (x + i, y + v, &r, &g, &b);

                            r2 = obj->mA*r2 / 100L;
                            g2 = obj->mA*g2 / 100L;
                            b2 = obj->mA*b2 / 100L;

                            r = obj->mB*r / 100L;
                            g = obj->mB*g / 100L;
                            b = obj->mB*b / 100L;

                            if (obj->subtractive)
                            {
                                if (obj->reverse)
                                {
                                    r = r2 - r;
                                    g = g2 - g;
                                    b = b2 - b;
                                }
                                else
                                {
                                    r = r - r2;
                                    g = g - g2;
                                    b = b - b2;
                                }
                            }
                            else
                            {
                                    r = r + r2;
                                    g = g + g2;
                                    b = b + b2;
                            }

                            if (r < 0) r = 0; if (r > 255) r = 255;
                            if (g < 0) g = 0; if (g > 255) g = 255;
                            if (b < 0) b = 0; if (b > 255) b = 255;

                            putpixel (x + i, y + v, packpix (r, g, b));
                        }

                        pmsg ("@drawing blended BMP: %lu%% completed", v*100L/h);
                    }
                }
                else
                {
                    for (j = obj->iy, v = 0; v < h; j++, v++)
                    {
                        fseek (fp, (3L * obj->ix) + base + (vt - j) * len, SEEK_SET);

                        for (i = 0; i < w; i++)
                            putpixel (x + i, y + v, getf (fp, obj));

                        pmsg ("@drawing BMP: %lu%% completed", v*100L/h);
                    }
                }
            }

            pmsg ("@drawing BMP: %lu%% completed", v*100L/h);

            fclose (fp);
    }

    static int BMP__GetWidth (object_t *obj)
    {
        FILE *fp;
        int i;

            if (obj->clip) return obj->iw;

            fp = BMP__Check (obj);
            if (fp == NULL) return DEF_W;

            fseek (fp, 18, SEEK_CUR);

            i = getw (fp);
            fclose (fp);

            return i;
    }

    static int BMP__GetHeight (object_t *obj)
    {
        FILE *fp;
        int i;

            if (obj->clip) return obj->ih;

            fp = BMP__Check (obj);
            if (fp == NULL) return DEF_H;

            fseek (fp, 22, SEEK_CUR);

            i = getw (fp);
            fclose (fp);

            return i;
    }

    static int GetImageWidth (object_t *obj)
    {
            switch (obj->type)
            {
                case KT__IN_BMP:    return BMP__GetWidth (obj);
            }

            return DEF_W;
    }

    static int GetImageHeight (object_t *obj)
    {
            switch (obj->type)
            {
                case KT__IN_BMP:    return BMP__GetHeight (obj);
            }

            return DEF_H;
    }

    static void PrepareClip (object_t *obj)
    {
        int w = GetImageWidth (obj), h = GetImageHeight (obj);
        int u = w, v = h;

            if (obj->iw == KP__AUTO) obj->iw = w;
            if (obj->ih == KP__AUTO) obj->ih = h;

            if (obj->iw <= KP__REL) obj->iw = w + obj->iw - KP__REL;
            if (obj->ih <= KP__REL) obj->ih = h + obj->ih - KP__REL;

            if (obj->iw <= KP__PER) obj->iw = calc_per (obj->iw, w, KP__PER);
            if (obj->ih <= KP__PER) obj->ih = calc_per (obj->ih, h, KP__PER);

            if (obj->iw > w) obj->iw = w;
            if (obj->ih > h) obj->ih = h;

            w = obj->iw;
            h = obj->ih;

            if (obj->ix > 0) obj->iw -= obj->ix;
            if (obj->iy > 0) obj->ih -= obj->iy;

            if (obj->ix == KP__CENTER)
                obj->ix = w >> 1;

            if (obj->iy == KP__CENTER)
                obj->iy = h >> 1;

            if (obj->ix <= KP__PER) obj->ix = calc_per (obj->ix, w, KP__PER);
            if (obj->iy <= KP__PER) obj->iy = calc_per (obj->iy, h, KP__PER);

            if (obj->ix == KP__AUTO)
                obj->ix = 0;

            if (obj->iy == KP__AUTO)
                obj->iy = 0;

            if (obj->ix < 0 || obj->ix >= u) obj->ix = 0;
            if (obj->iy < 0 || obj->iy >= v) obj->iy = 0;

            obj->clip = 1;

            if (obj->ix + obj->iw > u) obj->iw = u - obj->ix;
            if (obj->iy + obj->ih > v) obj->ih = v - obj->iy;
    }

    static void DrawImage (object_t *obj)
    {
            switch (obj->type)
            {
                case KT__IN_BMP:    BMP__Draw (obj);
                                    break;
            }
    }

/*****************************************************************************
******************************************************************************
*****************************************************************************/

    static int rwidth (object_t *obj)
    {
        int j, k, q, r = 0, w = obj->w;
        char *p, *s = obj->value;

            while (*s != '\0')
            {
                for (p = s, j = k = q = 0; *p != '\0' && q <= w; j++)
                {
                    if (*p == '\n') break;
                    if (*p++ == 32) k = j;
                    q += 8;
                }

                if (q <= w) k = j;

                if (!k) k = j - 1;

                if (k < 1) k = 1;

                s += k;

                if (*s == 32 || *s == '\n') s++;

                if (k > r) r = k;
            }

            return r * 8;
    }

    static int rheight (object_t *obj)
    {
        int j, k, q, h = 0, w = obj->w;
        char *p, *s = obj->value;

            while (*s != '\0')
            {
                for (p = s, j = k = q = 0; *p != '\0' && q <= w; j++)
                {
                    if (*p == '\n') break;
                    if (*p++ == 32) k = j;
                    q += 8;
                }

                if (q <= w) k = j;

                if (!k) k = j - 1;

                if (k < 1) k = 1;

                s += k;

                if (*s == 32 || *s == '\n') s++;

                h += 16;
            }

            return h;
    }

    static int bheight (object_t *obj)
    {
        int j, k, q, h = 0, w = obj->w;
        char *p, *s = obj->value;

            if (obj->border == KB__VOID) goto Ok;
            w-=2; h-=2;

            if (obj->border == KB__SINGLE) goto Ok;
            w-=2; h-=2;

        Ok: while (*s != '\0')
            {
                for (p = s, j = k = q = 0; *p != '\0' && q <= w; j++)
                {
                    if (*p == 32) k = j;
                    if (*p == '\n') break;

                    q += gCC_W ((unsigned char)*p++);
                }

                if (q <= w) k = j;

                if (!k) k = j - 1;

                if (k < 1) k = 1;

                s += k;

                if (*s == 32 || *s == '\n') s++;

                h += gCC_h ();
            }

            return h;
    }

    static int bwidth (object_t *obj)
    {
        int j, k, pq, q, r1, r = 0, w = obj->w;
        char *p, *s = obj->value;

            if (obj->border == KB__VOID) goto Ok;
            w-=2;

            if (obj->border == KB__SINGLE) goto Ok;
            w-=2;

        Ok: while (*s != '\0')
            {
                for (p = s, j = k = q = pq = 0; *p != '\0' && q <= w; j++)
                {
                    if (*p == '\n') break;
                    if (*p == 32)
                    {
                        k = j;
                        r1 = q;
                    }

                    pq = q;
                    q += gCC_W ((unsigned char)*p++);
                }

                if (q <= w)
                {
                    k = j;
                    r1 = q;
                }

                if (!k)
                {
                    k = j - 1;
                    r1 = pq;
                }

                if (k < 1)
                {
                    k = 1;
                    r1 = gCC_W ((unsigned char)*s);
                }

                s += k;

                if (*s == 32 || *s == '\n') s++;

                if (r1 > r) r = r1;
            }

            return r;
    }

    static void DrawText (object_t *obj)
    {
        int i, j, k, q, x, y, y2, w, h, align, calign;
        long pc1, pc2, tx, sp;
        char *s, *p;

            x = obj->x;
            y = obj->y;
            w = obj->w;
            h = obj->h;

            y2 = y + h - gCC_h () + 1;

            align = calign = obj->align;

            if (obj->border == KB__VOID) goto Fill;
            x++;    y++;    w-=2;   h-=2;

            if (obj->border == KB__SINGLE) goto Fill;
            x++;    y++;    w-=2;   h-=2;

        Fill:;
            if (obj->type == KT__TEXT_BF2)
            {
                btextattr (obj->fgc);
                s = obj->value;

                while (*s != '\0')
                {
                    for (p = s, j = k = q = 0; *p != '\0' && q <= w; j++)
                    {
                        if (*p == 32) k = j;
                        if (*p == '\n') break;

                        q += gCC_W ((unsigned char)*p++);
                    }

                    if (q <= w) k = j;

                    if (!k) k = j - 1;

                    if (k < 1) k = 1;

                    if (align == 2)
                    {
                        if (*(s + k) == '\0' || *(s + k) == '\n')
                            calign = 0;
                        else
                            calign = 2;
                    }

                    if (calign)
                    {
                        for (j = q = 0; j < k; j++)
                            q += gCC_W ((unsigned char)s [j]);
                    }

                    switch (calign)
                    {
                        case 0x00: /* left */
                            for (i = x; k--; )
                                i += bputc (i, y, (unsigned char)*s++);

                            break;

                        case 0x01: /* right */
                            for (i = x + (w - q); k--; )
                                i += bputc (i, y, (unsigned char)*s++);

                            break;

                        case 0x02: /* justify */
                            #define PREC    12L

                            sp = ((long)(w - q) << PREC) / k;

                            for (tx = (long)x << PREC; k--; )
                                tx += sp + ((long)bputc (tx >> PREC, y, (unsigned char)*s++) << PREC);

                            break;

                        case 0x03: /* center */
                            for (i = x + ((w - q) >> 1); k--; )
                                i += bputc (i, y, (unsigned char)*s++);

                            break;
                    }

                    if (*s == 32 || *s == '\n') s++;

                    y += gCC_h ();

                    if (y >= y2) break;
                }
            }
            else
            {
                rtextattr (obj->fgc);
                s = obj->value;

                while (*s != '\0')
                {
                    for (p = s, j = k = q = 0; *p++ != '\0' && q <= w; j++)
                    {
                        if (*(p - 1) == '\n') break;
                        if (*(p - 1) == 32) k = j;

                        q += 8;
                    }

                    if (q <= w) k = j;

                    if (!k) k = j - 1;

                    if (k < 1) k = 1;

                    for (i = x; k--; i += 8) rputc (i, y, (unsigned char)*s++);

                    if (*s == 32 || *s == '\n') s++;

                    y += 16;
                }
            }
    }

    static void readpix (object_t *q, int x, int y, int *r, int *g, int *b)
    {
            getpixel (q->x+x, q->y+y, r, g, b);
    }

    static void apply_filter (object_t *q, int x, int y, int *r,
                              int *g, int *b)
    {
        int i, j, k, u, v, ar, ag, ab, cr, cg, cb;

            ar = ag = ab = 0;

            switch (grm)
            {
                case 0x02:
                    if (strict)
                    {
                        sgetpixel (surfacemask, x, y, r, g, b);
                        doplot = !b;
                    }

                    readpix (q, x, y, &cr, &cg, &cb);

                    if (packpix (cr, cg, cb) == q->transp)
                    {
                        *r = cr;
                        *g = cg;
                        *b = cb;

                        return;
                    }

                    ar = cr + grr;
                    ag = cg + grg;
                    ab = cb + grb;

                    break;

                case 0x01:
                    doplot = 0;

                    u = x;
                    v = y;

                    if (strict && !doplot)
                    {
                        sgetpixel (surfacemask, u, v, &cr, &cg, &cb);
                        doplot = !cb;
                    }

                    readpix (q, u, v, &cr, &cg, &cb);

                    grs = 1;

                    ar = cr * grr;
                    ag = cg * grg;
                    ab = cb * grb;

                    break;

                case 0x00:
                    doplot = 0;

                    for (j = 0; j < grh; j++)
                    {
                        for (i = 0; i < grw; i++)
                        {
                            u = x+i-grhw;
                            v = y+j-grhh;

                            if (strict && !doplot)
                            {
                                sgetpixel (surfacemask, u, v, &cr, &cg, &cb);
                                doplot = !cb;
                            }

                            readpix (q, u, v, &cr, &cg, &cb);

                            k = grid [j][i];

                            ar += cr * k;
                            ag += cg * k;
                            ab += cb * k;
                        }
                    }

                    break;
            }

            *r = ar / grs;
            *g = ag / grs;
            *b = ab / grs;
    }

    static int clip_val (int x, int a, int b)
    {
            if (x < a) return a;
            if (x > b) return b;

            return x;
    }

    static void DrawFilter (object_t *obj)
    {
        int x, y, i, j, k, r, g, b;

            if (!fill_grid (obj->value)) return;

            x = obj->x;
            y = obj->y;

            if (grm != 2)
            {
                if (grm == 3)
                {
                    pmsg ("using %c filter (%d,%f,%f)", grw, grr, grg, grb);
                }
                else
                {
                    if (grm == 1)
                        pmsg ("using multiplicative filter (%f,%f,%f)", grr, grg, grb);
                    else
                        pmsg ("using %ux%u grid (normal)", grw, grh);
                }
            }
            else
            {
                pmsg ("using incremental filter (%d,%f,%f,%f)",
                    grs, grr, grg, grb);
            }

            pmsg ("");

            if (grm == 3)
            {
                doplot = 0;

                for (j = 0; j < obj->h; j++)
                {
                    for (i = 0; i < obj->w; i++)
                    {
                        readpix (obj, i, j, &r, &g, &b);

                        switch (grw)
                        {
                            case 'R':
                                if (r > g && r > b)
                                    r += grr, g += grg, b += grb;

                                break;

                            case 'G':
                                if (g > r && g > b)
                                    r += grr, g += grg, b += grb;

                                break;

                            case 'B':
                                if (b > r && b > g)
                                    r += grr, g += grg, b += grb;

                                break;
                        }

                        r = clip_val (r, 0, 255);
                        g = clip_val (g, 0, 255);
                        b = clip_val (b, 0, 255);

                        mputpixel (x+i, y+j, y+j, packpix (r, g, b));
                    }

                    pmsg ("@updating image area: %lu%% completed", j*100L/obj->h);
                }

                pmsg ("@updating image area: %lu%% completed", j*100L/obj->h);
                return;
            }

            if (grm == 2)
            {
                for (j = 0; j < obj->h; j++)
                {
                    for (i = 0; i < obj->w; i++)
                    {
                        apply_filter (obj, i, j, &r, &g, &b);

                        r = clip_val (r, 0, 255);
                        g = clip_val (g, 0, 255);
                        b = clip_val (b, 0, 255);

                        mputpixel (x+i, y+j, y+j, packpix (r, g, b));
                    }

                    pmsg ("@updating image area: %lu%% completed", j*100L/obj->h);
                }

                pmsg ("@updating image area: %lu%% completed", j*100L/obj->h);
            }
            else
            {
                k = surface->height;

                for (j = 0; j < obj->h; j++)
                {
                    for (i = 0; i < obj->w; i++)
                    {
                        apply_filter (obj, i, j, &r, &g, &b);

                        r = clip_val (r, 0, 255);
                        g = clip_val (g, 0, 255);
                        b = clip_val (b, 0, 255);

                        mputpixel (i, j+k, j, packpix (r, g, b));
                    }

                    pmsg ("@applying filter: %lu%% completed", j*100L/obj->h);
                }

                pmsg ("@applying filter: %lu%% completed", j*100L/obj->h);

                for (j = 0; j < obj->h; j++)
                {
                    for (i = 0; i < obj->w; i++)
                    {
                        w__ReadPixel (surface, i, j+k, &r, &g, &b);
                        w__WritePixel (surface, x+i, y+j, packpix (r, g, b));
                    }

                    pmsg ("@updating image area: %lu%% completed", j*100L/obj->h);
                }

                pmsg ("@updating image area: %lu%% completed", j*100L/obj->h);
            }
    }

    static void DrawSprayer (object_t *obj)
    {
        int x, y, i, j, k = surface->height;
        int r, g, b, u, v, p, m;

            m = atoi (obj->value);

            x = obj->x;
            y = obj->y;

            switch (obj->dir)
            {
                case KP__AUTO:
                    p = 0x0F;
                    break;

                case KPT__HLINE:
                case KPT__HORZ:
                    p = 0x03;
                    break;

                case KPT__VLINE:
                case KPT__VERT:
                    p = 0x0C;
                    break;

                case KPT__UP:
                    p = 0x08;
                    break;

                case KPT__DN:
                    p = 0x04;
                    break;

                case KPT__LE:
                    p = 0x02;
                    break;

                case KPT__RI:
                    p = 0x01;
                    break;
            }

            for (j = 0; j < obj->h; j++)
            {
                for (i = 0; i < obj->w; i++)
                {
                    if (p & 3)
                    {
                        do u = i + (rand () + rand () - rand ()) % m;
                        while (u < 0 || u >= obj->w || (u < i && !(p & 1)) ||
                              (u > i && !(p & 2)));
                    }
                    else
                        u = i;

                    if (p & 12)
                    {
                        do v = j + (rand () + rand () - rand ()) % m;
                        while (v < 0 || v >= obj->h || (v < j && !(p & 4)) ||
                              (v > j && !(p & 8)));
                    }
                    else
                        v = j;

                    readpix (obj, u, v, &r, &g, &b);
                    mputpixel (i, j+k, j, packpix (r, g, b));
                }

                pmsg ("@spraying: %lu%% completed", j*100L/obj->h);
            }

            for (j = 0; j < obj->h; j++)
            {
                for (i = 0; i < obj->w; i++)
                {
                    w__ReadPixel (surface, i, j+k, &r, &g, &b);
                    w__WritePixel (surface, x+i, y+j, packpix (r, g, b));
                }

                pmsg ("@updating image area: %lu%% completed", j*100L/obj->h);
            }

            pmsg ("@updating image area: %lu%% completed", j*100L/obj->h);
    }

    static void DrawNegative (object_t *obj)
    {
        unsigned long tcolor, pcolor, temp;
        int x, y, i, j, r, g, b;

            tcolor = obj->transp;
            pcolor = obj->prim;

            x = obj->x;
            y = obj->y;

            for (j = 0; j < obj->h; j++)
            {
                if (tcolor != KC__DEFAULT || pcolor != KC__DEFAULT)
                {
                    for (i = 0; i < obj->w; i++)
                    {
                        readpix (obj, i, j, &r, &g, &b);

                        temp = packpix (r, g, b);

                        if (temp == tcolor) continue;

                        if (pcolor != KC__DEFAULT && temp != pcolor)
                            continue;

                        putpixel (x+i, y+j, packpix (255-r, 255-g, 255-b));
                    }

                    pmsg ("@negativizing: %lu%% completed", j*100L/obj->h);
                }
                else
                {
                    for (i = 0; i < obj->w; i++)
                    {
                        readpix (obj, i, j, &r, &g, &b);
                        putpixel (x+i, y+j, packpix (255-r, 255-g, 255-b));
                    }

                    pmsg ("@negativizing: %lu%% completed", j*100L/obj->h);
                }
            }

            pmsg ("@negativizing: %lu%% completed", j*100L/obj->h);
    }

    static void DrawGrayScale (object_t *obj)
    {
        int x, y, i, j, k, r, g, b;
        unsigned long tcolor, pcolor, temp;

            tcolor = obj->transp;
            pcolor = obj->prim;

            x = obj->x;
            y = obj->y;

            #define RedK    0.3086
            #define GreenK  0.6094
            #define BlueK   0.0820

            for (j = 0; j < obj->h; j++)
            {
                if (tcolor != KC__DEFAULT || pcolor != KC__DEFAULT)
                {
                    for (i = 0; i < obj->w; i++)
                    {
                        readpix (obj, i, j, &r, &g, &b);

                        temp = packpix (r, g, b);

                        if (temp == tcolor) continue;

                        if (pcolor != KC__DEFAULT && temp != pcolor)
                            continue;

                        k = r*RedK + g*GreenK + b*BlueK;

                        putpixel (x+i, y+j, packpix (k, k, k));
                    }

                    pmsg ("@graying: %lu%% completed", j*100L/obj->h);
                }
                else
                {
                    for (i = 0; i < obj->w; i++)
                    {
                        readpix (obj, i, j, &r, &g, &b);
                        k = r*RedK + g*GreenK + b*BlueK;
                        putpixel (x+i, y+j, packpix (k, k, k));
                    }

                    pmsg ("@graying: %lu%% completed", j*100L/obj->h);
                }
            }

            pmsg ("@graying: %lu%% completed", j*100L/obj->h);
    }

    static void Enhance (int y1, int x1, int x2, int h, int dir)
    {
        int x, y, w, r1, g1, b1, r2, g2, b2;
        float dk, ir, ig, ib, r, g, b;

            if (dir)
            {
                w = x2 - (x = x1);
                dk = h;

                while (w--)
                {
                    h = dk, y = y1;

                    getpixel (x, y1, &r1, &g1, &b1);
                    getpixel (x, y1+h, &r2, &g2, &b2);

                    ir = (r2 - (r = r1)) / dk;
                    ig = (g2 - (g = g1)) / dk;
                    ib = (b2 - (b = b1)) / dk;

                    while (h--)
                    {
                        putpixel (x, y, packpix (r, g, b));
                        r += ir, g += ig, b += ib, y++;
                    }

                    x++;
                }
            }
            else
            {
                dk = x2 - x1;

                while (h--)
                {
                    w = dk, x = x1;

                    getpixel (x1, y1, &r1, &g1, &b1);
                    getpixel (x2, y1, &r2, &g2, &b2);

                    ir = (r2 - (r = r1)) / dk;
                    ig = (g2 - (g = g1)) / dk;
                    ib = (b2 - (b = b1)) / dk;

                    while (w--)
                    {
                        putpixel (x, y1, packpix (r, g, b));
                        r += ir, g += ig, b += ib, x++;
                    }

                    y1++;
                }
            }
    }

    static void DrawEnhance (object_t *obj)
    {
        unsigned hstep, vstep;
        int i, j, u, v;

            hstep = atoi (strtok (obj->value, ","));
            vstep = atoi (strtok (NULL, ","));

            pmsg ("@hstep: %u, vstep: %u", hstep, vstep);

            if (!hstep || !vstep) return;

            pmsg ("");

            v = 2*(obj->h/vstep);

            for (j = u = 0; j < obj->h; j += vstep, u++)
            {
                for (i = 0; i < obj->w; i += hstep)
                    Enhance (j+obj->y, i+obj->x, i+obj->x+hstep, vstep, 0);

                pmsg ("@enhancing: %lu%% completed", u*100L/v);
            }

            for (j = 0; j < obj->h; j += vstep, u++)
            {
                for (i = 0; i < obj->w; i += hstep)
                    Enhance (j+obj->y, i+obj->x, i+obj->x+hstep, vstep, 1);

                pmsg ("@enhancing: %lu%% completed", u*100L/v);
            }

            pmsg ("@enhancing: %lu%% completed", 100L);
    }

    static void DrawStretch (object_t *obj)
    {
        unsigned long i, j, w, h, vt;
        float dx, dy, ix, iy, x, y;
        unsigned long base, len;
        FILE *fp;

            fp = BMP__Check (obj);
            if (fp == NULL) return;

            getw (fp);
            getd (fp);
            getd (fp);
            base = getd (fp);
            getd (fp); /* 40 */
            w = getd (fp);
            h = getd (fp);
            vt = h - 1;

            len = w * 3;
            len = (len + 3) & -4;

            ix = (float)w / obj->w;
            iy = (float)h / obj->h;

            for (j = 0, y = 0; j < obj->h; j++, y += iy)
            {
                for (i = 0, x = 0; i < obj->w; i++, x += ix)
                {
                    fseek (fp, (3L * ((unsigned)x)) + base + (vt - ((unsigned)y))*len, SEEK_SET);
                    putpixel (obj->x+i, obj->y+j, getf (fp, obj));
                }

                pmsg ("@drawing BMP: %lu%% completed", j*100L/obj->h);
            }

            pmsg ("@drawing BMP: %lu%% completed", j*100L/obj->h);

            fclose (fp);
    }

    static void DrawCLayer (object_t *obj)
    {
        int i, j, k, r, g, b, v[3];

            for (j = 0; j < obj->h; j++)
            {
                for (i = 0; i < obj->w; i++)
                {
                    readpix (obj, i, j, &r, &g, &b);

                    for (k = 0; k < 3; k++)
                    {
                        if ((obj->value [k] & 0xDF) == 'R') v[k] = r;
                        if ((obj->value [k] & 0xDF) == 'G') v[k] = g;
                        if ((obj->value [k] & 0xDF) == 'B') v[k] = b;

                        if (obj->value [k] == '0') v[k] = 0;
                        if (obj->value [k] == '1') v[k] = 255;
                    }

                    putpixel (obj->x + i, obj->y + j, packpix (v[0], v[1], v[2]));
                }

                pmsg ("@drawing color-layer: %lu%% completed", j*100L/obj->h);
            }

            pmsg ("@drawing color-layer: %lu%% completed", j*100L/obj->h);
    }

    static int not_text (int x)
    {
            return x != KT__TEXT_BF2 && x != KT__TEXT_RFX;
    }

    static int is_img (int x)
    {
            return x == KT__IN_BMP;
    }

    void draw_post (object_t *p, int destroy)
    { /* Post */
        static const surface_t *base, *mask;
        static int lpsx, lpsy, i, j, r, g, b;
        static long u;

            i = p->astype == -1 ? p->type : p->astype;

            switch (i)
            {
                case KT__NEG:
                    if (p->nodraw) break;

                    pmsg ("making negative %u %u %u %u",
                        p->x, p->y, p->w, p->h);

                    pmsg ("");

                    DrawNegative (p);
                    pmsg ("&, done");
                    break;

                case KT__GRAY:
                    if (p->nodraw) break;

                    pmsg ("making gray scale %u %u %u %u",
                        p->x, p->y, p->w, p->h);

                    pmsg ("");

                    DrawGrayScale (p);
                    pmsg ("&, done");
                    break;

                case KT__ENHANCE:
                    if (p->nodraw) break;

                    pmsg ("making enhance-filter %u %u %u %u",
                        p->x, p->y, p->w, p->h);

                    pmsg ("");

                    DrawEnhance (p);
                    pmsg ("&, done");
                    break;

                case KT__STRETCH:
                    if (p->nodraw) break;

                    pmsg ("drawing stretched-bmp %u %u %u %u",
                        p->x, p->y, p->w, p->h);

                    pmsg ("");

                    DrawStretch (p);
                    pmsg ("&, done");
                    break;

                case KT__CLAYER:
                    if (p->nodraw) break;

                    pmsg ("drawing color-layer %u %u %u %u",
                        p->x, p->y, p->w, p->h);

                    pmsg ("");

                    DrawCLayer (p);
                    pmsg ("&, done");
                    break;

                case KT__SPRAY:
                    if (p->nodraw) break;

                    pmsg ("performing spraying %u %u %u %u",
                        p->x, p->y, p->w, p->h);

                    pmsg ("");

                    DrawSprayer (p);
                    pmsg ("&, done");
                    break;

                case KT__FADER:
                    if (p->nodraw) break;

                    pmsg ("performing fading %u %u %u %u",
                        p->x, p->y, p->w, p->h);

                    pmsg ("");

                    DrawFader (p);
                    pmsg ("&, done");
                    break;

                case KT__MIRROR:
                    if (p->nodraw) break;

                    if (p->dir == KP__AUTO) p->dir = 0;

                    pmsg ("performing %s mirroring %u %u %u %u",
                        p->dir ? "vertical" : "horizontal",
                        p->x, p->y, p->w, p->h);

                    pmsg ("");

                    DrawMirror (p);
                    pmsg ("&, done");
                    break;

                case KT__FILTER:
                    if (p->nodraw) break;

                    DrawFilter (p);
                    break;

                case KT__FILTERL:
                    if (p->nodraw) goto KTFL_D;

                    strict = 1;
                    DrawFilter (p);
                    strict = 0;

                    pmsg ("");

                    mask = surfacemask;
                    base = surface;

                    surfacemask = p->surfacemask;
                    surface = p->surface;

                    lpsx = psx;
                    lpsy = psy;

                    psx = p->psx;
                    psy = p->psy;

                    for (j = 0; j < p->h; j++)
                    {
                        for (i = 0; i < p->w; i++)
                        {
                            w__ReadPixel (mask, i, j, &r, &g, &b);
                            if (b == 0xFF) continue;

                            w__ReadPixel (base, i, j, &r, &g, &b);
                            u = packpix (r, g, b);

                            putpixel (lpsx+i, lpsy+j, u);
                        }

                        pmsg ("@updating main surface: %lu%% completed", j*100L/p->h);
                    }

                    pmsg ("@updating main surface: %lu%% completed", j*100L/p->h);

                    pmsg ("&, done");

                    w__CloseSurface (mask);
                    w__CloseSurface (base);

            KTFL_D: if (destroy)
                    {
                        delete (p->file1);
                        delete (p->file2);
                    }

                    break;
            }

            if (destroy)
            {
                if (p->value != NULL) delete (p->value);
                delete (p);
            }
    }

    void destroy_object (int form)
    {
            if (form) return;
            draw_post (list__pop (objstack), 1);
    }

    static void set_rfx_font (char *s)
    {
        font_t *font;
        void *data;

            font = gsearch (fonts, s);
            if (font == NULL)
            {
                pmsg ("*font '%s' not found, default used", s);
                data = guifont;
            }
            else
                data = font->data;

            if (set_font (data))
            {
                pmsg ("*unable to set font '%s', default used", s);
                set_font (guifont);
            }
    }

    static void set_bf2_font (char *s)
    {
        font_t *font; void *data;
        unsigned len;

            font = gsearch (fonts, s);
            if (font == NULL)
            {
                pmsg ("*font '%s' not found, default used", s);
                len = sizeof(sguifont);
                data = sguifont;
            }
            else
            {
                data = font->data;
                len = font->len;
            }

            if (bf2__setfont (data, len))
            {
                pmsg ("*unable to set font '%s', default used", s);
                bf2__setfont (sguifont, sizeof(sguifont));
            }
    }

    static void load_information (object_t *p, object_t *x)
    {
            if (x->rfx != NULL) set_rfx_font (x->rfx);
            if (x->bf2 != NULL) set_bf2_font (x->bf2);

            if (x->face != NULL && !not_text (x->type))
            {
                if (x->type == KT__TEXT_BF2)
                {
                    if (!x->bf2) set_bf2_font (x->face);
                }
                else
                {
                    if (!x->rfx) set_rfx_font (x->face);
                }
            }

            DEF_SPACE = x->space;
    }

    static int border_size (object_t *x)
    {
            switch (x->border)
            {
                case KB__SINGLE:
                    return 1;

                case KB__DOUBLE:
                    return 2;

                default:
                    return 0;
            }
    }

    static char *temp_filename (void)
    {
        static char buf [64];
        static int temp = 0;

            sprintf (buf, "sur%u.tmp", temp++);

            return dups (buf);
    }

    void make_object (int form, int n, char *argv [], unsigned ndx [])
    { /* Pre */
        int istxt, isimg, i, j, x, y, w, h, u, v;
        object_t *last = BOT(objstack);

            if (last == NULL)
            {
                list__add (objstack, last = new (object_t));
                set_default (last);

                last->w = surface->width;
                last->h = surface->height;

                last->left_w = last->w;
                last->left_h = last->h;
            }

            if (form) set_default (cobj = new (object_t));

            w = last->left_w - last->h_margin_l - last->h_margin_r;
            h = last->left_h - last->v_margin_u - last->v_margin_d;

            if (ndx [0] == KT__BF2_ALIAS) ndx [0] = KT__TEXT_BF2;

            istxt = !not_text (cobj->type = ndx [0]);
            isimg = is_img (ndx [0]);

            u = cobj->x;
            v = cobj->y;

            if (u < 0) u = 0;
            if (v < 0) v = 0;

            load_information (last, cobj);

            if (isimg) PrepareClip (cobj);

            if (cobj->w == KP__AUTO)
            {
                if (istxt)
                {
                    cobj->w = 8192;

                    if (cobj->type == KT__TEXT_BF2)
                        cobj->w = bwidth (cobj);
                    else
                        cobj->w = rwidth (cobj);
                }
                else
                {
                    if (isimg)
                        cobj->w = GetImageWidth (cobj);
                    else
                        cobj->w = w;
                }
            }

            if (istxt) cobj->w += 4*border_size (cobj);

            if (u + cobj->w > w) cobj->w = w - u;

            if (cobj->h == KP__AUTO)
            {
                if (istxt)
                {
                    if (cobj->type == KT__TEXT_BF2)
                        cobj->h = bheight (cobj);
                    else
                        cobj->h = rheight (cobj);
                }
                else
                {
                    if (isimg)
                        cobj->h = GetImageHeight (cobj);
                    else
                        cobj->h = h;
                }
            }

            if (istxt) cobj->h += 4*border_size (cobj);

            if (v + cobj->h > h) cobj->h = h - v;

            if (cobj->w <= KP__REL) cobj->w = w + cobj->w - KP__REL;
            if (cobj->h <= KP__REL) cobj->h = h + cobj->h - KP__REL;

            if (cobj->w <= KP__APER) cobj->w = calc_per (cobj->w, last->w, KP__APER);
            if (cobj->h <= KP__APER) cobj->h = calc_per (cobj->h, last->h, KP__APER);

            if (cobj->w <= KP__PER) cobj->w = calc_per (cobj->w, w, KP__PER);
            if (cobj->h <= KP__PER) cobj->h = calc_per (cobj->h, h, KP__PER);

            if (cobj->x == KP__CENTER)
                cobj->x = ((w - cobj->w) / 2);

            if (cobj->y == KP__CENTER)
                cobj->y = ((h - cobj->h) / 2);

            if (cobj->x <= KP__APER) cobj->x = calc_per (cobj->x, last->w, KP__APER);
            if (cobj->y <= KP__APER) cobj->y = calc_per (cobj->y, last->h, KP__APER);

            if (cobj->x <= KP__PER) cobj->x = calc_per (cobj->x, w, KP__PER);
            if (cobj->y <= KP__PER) cobj->y = calc_per (cobj->y, h, KP__PER);

            if (cobj->x == KP__AUTO)
                cobj->x = ((w - cobj->w) / 2);

            if (cobj->y == KP__AUTO)
                cobj->y = ((h - cobj->h) / 2);

            x = last->x + last->h_margin_l + last->st_x;
            y = last->y + last->v_margin_u + last->st_y;

            i = u + cobj->w;
            j = v + cobj->h;

            switch (last->type)
            {
                case KT__ICOLUMN:
                    y = y - last->st_y + h - 2*cobj->y - cobj->h;
                    break;

                case KT__IROW:
                    x = x - last->st_x + w - 2*cobj->x - cobj->w;
                    break;
            }

            cobj->x += x;
            cobj->y += y;

            cobj->left_w = cobj->w;
            cobj->left_h = cobj->h;

            switch (last->type)
            {
                case KT__ICOLUMN:
                case KT__COLUMN:
                    last->left_h -= j;
                    last->st_y += j;
                    break;

                case KT__IROW:
                case KT__ROW:
                    last->left_w -= i;
                    last->st_x += i;
                    break;
            }

            pmsg ("drawing %s %u %u %u %u", argv [0], cobj->x,
                cobj->y, cobj->w, cobj->h);

            DrawFrame (cobj);

            switch (istxt | (isimg << 1))
            {
                case 0x00:
                    switch (cobj->type)
                    {
                        case KT__FILTERL:
                            i = cobj->w - cobj->h_margin_l - cobj->h_margin_r;
                            j = cobj->h - cobj->v_margin_u - cobj->v_margin_d;

                            cobj->left_w = i;
                            cobj->left_h = j;

                            cobj->h_margin_l = cobj->h_margin_r = 0;
                            cobj->v_margin_u = cobj->v_margin_d = 0;

                            cobj->px = cobj->x;
                            cobj->py = cobj->y;

                            cobj->x = 0;
                            cobj->y = 0;

                            cobj->file1 = temp_filename ();
                            cobj->file2 = temp_filename ();

                            cobj->surfacemask = surfacemask;
                            cobj->surface = surface;

                            cobj->psx = psx;
                            cobj->psy = psy;

                            psx = cobj->px + cobj->h_margin_l;
                            psy = cobj->py + cobj->v_margin_u;

                            surface = w__CreateSurface (i, j, cobj->file1, 0);
                            if (surface == NULL)
                            {
                                pmsg ("*unable to create temporal surface");
                                p__finish = 1;
                            }
                            else
                                ((surface_t *)surface)->abort = 1;

                            surfacemask = w__CreateSurface (i, j, cobj->file2, 0xFF);
                            if (surfacemask == NULL)
                            {
                                pmsg ("*unable to create temporal surface");
                                p__finish = 1;
                            }
                            else
                                ((surface_t *)surfacemask)->abort = 1;

                            break;
                    }

                    break;

                case 0x01:
                    DrawText (cobj);
                    break;

                case 0x02:
                    DrawImage (cobj);
                    break;
            }

            if ((u = cobj->astype) != -1)
            {
                u = cobj->astype;
                cobj->astype = cobj->type;
                cobj->type = u;
            }

pmsg ("&, done");

            if (cobj->pre)
            {
                draw_post (cobj, 0);
                cobj->nodraw = 1;
            }

            list__add (objstack, cobj);
            cobj = NULL;
    }

    void surface_def (int form, int n, char *argv [], unsigned ndx [])
    {
        static char *formats [] = { ".tga", ".bmp" };
        long base; int xres, yres, format = 0;
        char *s; object_t *p;

            if (form) format = ndx [7];

            if (surface != NULL)
            {
pmsg ("closing surface");
                s = surface->output;
                ((surface_t *)surface)->abort = errc != 0;
                w__CloseSurface (surface);
                delete (s);
pmsg ("&, done");
            }

            base = pop_constant ();
            yres = pop_constant ();
            xres = pop_constant ();

            while ((p = list__pop (objstack)) != NULL)
                delete (p);

            s = mkfil (formats [format]);

pmsg ("creating surface %u %u %s %06lX", xres, yres, s, base);

            surface = psurface = w__CreateSurface (xres, yres, s, base);
            if (surface == NULL)
            {
                pmsg ("*unable to create surface: %u %u %s %06lX",
                    xres, yres, s, base);

                p__finish = 1;
            }

            ((surface_t *)surface)->format = format;

pmsg ("&, done");

            if (surface->powered_by_xms) pmsg (" *** powered by xms ***");
    }

    void modifier (int form, int n, char *argv [])
    {
        font_t *font;
        char *s;

            switch (form)
            {
                case 0x00: /* RFX */
                    add_font (dups (argv [2]), mkfil (".rfx"));
                    break;

                case 0x01: /* BF2 */
                    add_font (dups (argv [2]), mkfil (".bf2"));
                    break;

                case 0x02: /* ? */
                    add_font (dups (argv [2]), string);
                    break;

                case 0x03: /* shell */
                    system (string);
                    break;
            }
    }

    void set_direction (int form)
    {
            push_constant (form);
    }
