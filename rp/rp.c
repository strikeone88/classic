/*
    RP4.C

    RedStar Package Utility Version 4.00

    This program is capable of packing, listing, unpacking and encrypting
    RedStar Packages Version 0.2, also can list and unpack older packages
    but using a compatibility mode that ignores some flags that were used
    on the previous version. Version 3.00 provides a new mechanism to use
    eve hashes instead of a keyword.

    Version 4 now compiled for Windows using Borland C++, and several data
    type size issues modified to maintain consistency across platforms.

    Copyright (C) 2007-2012 RedStar Technologies
    Written by J. Palencia (ilina@bloodykisses.zzn.com)
*/

    #include <stdlib.h>
    #include <string.h>
    #include <stdarg.h>
    #include <stdio.h>
    #include <utime.h>
    #include <time.h>
    #include <dos.h>
    #include <dir.h>
    #include <io.h>

    #include <angela.h>
    #include <eve.h>

    /* Signatures. */
    #define RP_SIGN     0x7072
    #define ORP_SIGN    0x5052
    #define E_SIGN1     0x6572
    #define E_SIGN2     0x7265

    /* Size of the entry (without filename). */
    #define E_SIZE      20

    /* Program Actions. */
    #define A_PACK      0
    #define A_UNPACK    1
    #define A_LIST      2

    /* Easier way of using the attributes. */
    #define FA_ALLNORMAL    (FA_NORMAL | FA_RDONLY | FA_DIREC | FA_ARCH)
    #define FA_PLUSHIDDEN   (FA_ALLNORMAL | FA_HIDDEN)
    #define FA_PLUSSYSTEM   (FA_ALLNORMAL | FA_SYSTEM)
    #define FA_ENTIRE       (FA_ALLNORMAL | FA_HIDDEN | FA_SYSTEM)

    /* Package entry. */
	#pragma option push -a1
    typedef struct
    {
        unsigned short  e_size;
        unsigned short  e_sign1;

        unsigned char   flags;

        unsigned char   hour;
        unsigned char   minutes;
        unsigned short  year;

        unsigned char   month;
        unsigned char   day;

        unsigned long   flen;
        unsigned short  nlen;

        unsigned char   pad;

        unsigned short  e_sign2;
        unsigned short  res;

        char            fname [1024];
    }
    re_t;

    /* Old Package entry (prior to RP-0.2). */
    typedef struct
    {
        unsigned char   flags;
        unsigned char   hour;
        unsigned char   minutes;
        unsigned short  year;
        unsigned char   month;
        unsigned char   day;
        unsigned long   flen;
        unsigned char   nlen;
    }
    ore_t;
	#pragma option pop

    /* These are the RP options. */
    static char *options [] =
    {
        "unpack", "u", "list", "l", "recurse", "r", "lwrcase", "w",
        "key", "k", "ench", "eh", NULL
    };

    /* Count of files and directories. */
    static long fcount = 0, dcount = 0, total = 0;

    /* Current input/output file. */
    static FILE *xfile;

    /* Filename. */
    static char *sxfile;

    /* Global buffers. */
    static char tbuf [1024], buf [512], ebuf [1024], kbuf [128];

    /* Indicates to force lower-case. */
    static int lowercase;

    /* Some flags that will be put in the header-flags field. */
    static int encryptData, encryptHeader, eveHash;

    /* This is the length of the keyword (and the keyword). */
    static char keyword [128], pKeyword [128];
    static unsigned klen, pKlen;

    /* Prints a message. */
    static void pmsg (char *s, ...)
    {
        static int firsttime = 1;
        va_list args;

            if (!firsttime) printf ("\n");

            va_start (args, s);
            printf ("rp%s: ", *s == '*' ? (s++, " (error)") : "");

            vprintf (s, args);

            firsttime = 0;
    }

    /* Converts all the backslashes to slashes. */
    static char *toSlash (char *s)
    {
        char *p = s;

            while ((s = strchr (s, '\\')) != NULL)
                *s++ = '/';

            return p;
    }

    /* Returns true if the filename has an extension. */
    static int hasextension (char *s)
    {
        char *p = s;

            while (*s != '\0') if (*s++ == '/') p = s;

            return strchr (p, '.') != NULL;
    }

    /* Duplicates a string and adds n bytes after it. */
    static char *duplicate (char *s, int n)
    {
        char *p = malloc (strlen (s) + n + 1);

            if (p != NULL)
                return strcpy (p, s);
            else
                return NULL;
    }

    /* Returns only the path of the given full-path. */
    static char *getpath (char *s)
    {
        char *p = s, *q = s;
        char ch;

            while (*s != '\0') if (*s++ == '/') p = s;

            ch = *p;
            *p = '\0';

            s = duplicate (q, 0);

            *p = ch;

            return s;
    }

    /* Returns only the filename of the given full-path. */
    static char *getfilename (char *s)
    {
        char *p = s;

            while (*s != '\0') if (*s++ == '/') p = s;

            return duplicate (p, 0);
    }

    /* Returns 0x80 if f is a directory. */
    static int isdir (struct ffblk *f)
    {
            return f->ff_attrib & FA_DIREC ? 0x80 : 0x00;
    }

    /* Writes a 16-bit value (little endian) to a file stream. */
    static void xputw (unsigned int x, FILE *fp)
    {
            putc ((unsigned char)(x&0xFF), fp);
            putc ((unsigned char)((x >> 8)&0xFF), fp);
    }

    /* Writes a double word to the given stream. */
    static void xputd (unsigned long x, FILE *fp)
    {
            xputw ((unsigned int)(x), fp);
            xputw ((unsigned int)(x >> 16), fp);
    }

    /* Reads a 16-bit value (little endian) from a file stream. */
    static unsigned xgetw (FILE *fp)
    {
        unsigned a = (unsigned char)getc (fp);
        unsigned b = (unsigned char)getc (fp);

        return a | (b << 8);
    }

    /* Reads a double word from the given stream. */
    static unsigned long xgetd (FILE *fp)
    {
        unsigned long p = xgetw (fp);

        return p | ((unsigned long)((unsigned int)xgetw (fp)) << 16);
    }

    /* Walks the directory and calls the given function for each
       entry that is found. */

    static void walk_directory (char *path, char *mask, unsigned attr,
                int recurse, int (*pF) (char *, struct ffblk *))
    {
        char *fullpath = strcat (duplicate (path, strlen (mask)), mask);
        struct ffblk _f, *f = &_f; char *s;
        unsigned i;

            for (i = findfirst (fullpath, f, attr); !i; i = findnext (f))
            {
                if (f->ff_name [0] == '.') continue;
                if (pF (path, f)) break;
            }

            free (fullpath);

            if (recurse)
            {
                fullpath = strcat (duplicate (path, 3), "*.*");

                for (i = findfirst (fullpath, f, attr | FA_DIREC); !i;
                     i = findnext (f))
                {
                    if (!isdir (f) || f->ff_name [0] == '.')
                        continue;

                    strcat (strcat (s = duplicate (path, strlen (f->ff_name)
                            + 1), f->ff_name), "/");

                    walk_directory (s, mask, attr, 1, pF);

                    free (s);
                }
            }
    }

    /* Walks the package and calls the given function for each entry. */
    static int walk_package (int (*pF) (re_t *, char *), char *path)
    {
        re_t x;

            while (1)
            {
				x.e_size = xgetw(xfile);
				if(feof(xfile)) break;
				x.e_sign1 = xgetw(xfile);
				x.flags = getc(xfile);
				x.hour = getc(xfile);
				x.minutes = getc(xfile);
				x.year = xgetw (xfile);
				x.month = getc (xfile);
				x.day = getc (xfile);
				x.flen = xgetd (xfile);
				x.nlen = xgetw (xfile);
				x.pad = getc (xfile);
				x.e_sign2 = xgetw (xfile);
				x.res = xgetw(xfile);
				fread ((void *)&x.fname, 1, x.e_size-20, xfile);

                if (encryptHeader)
                {
                    memcpy (kbuf, keyword, klen);
                    ax__decrypt (((char *)&x) + 2, x.e_size, ebuf, kbuf, klen);

                    memcpy (((char *)&x) + 2, ebuf, x.e_size);
                }

                if (x.e_sign1 != E_SIGN1 || x.e_sign2 != E_SIGN2)
                {
                    pmsg ("*invalid package entry (bad keyword?).\n");
                    return 1;
                }

                x.fname [x.nlen] = '\0';
                x.pad &= 3;

                if (pF (&x, path)) break;
            }

            return 0;
    }

    /* Same as above but only for older versions of RP. */
    static int owalk_package (int (*pF) (re_t *, char *), char *path)
    {
        ore_t x;
        re_t p;

            while (1)
            {
                fread ((void *)&x, 1, sizeof(ore_t), xfile);
                if (feof (xfile)) break;

                p.flags = x.flags & 0x80;

                p.minutes = x.minutes;
                p.month = x.month;
                p.hour = x.hour;
                p.year = x.year;
                p.day = x.day;

                p.flen = x.flen;
                p.nlen = x.nlen;

                p.pad = 4;

                fread ((char *)&p.fname, 1, p.nlen, xfile);
                p.fname [p.nlen] = '\0';

                if (pF (&p, path)) break;
            }

            return 0;
    }

    /* Extracts the hour from the time. */
    static int xhour (struct ffblk *f)
    {
            return f->ff_ftime >> 11;
    }

    /* Extracts the minutes from the time. */
    static int xminutes (struct ffblk *f)
    {
            return (f->ff_ftime >> 5) & 0x3F;
    }

    /* Packs an hour/minutes pair. */
    static unsigned int packtime (int hour, int minutes)
    {
            return (hour << 11) | (minutes << 5);
    }

    /* Extracts the year from the date. */
    static int xyear (struct ffblk *f)
    {
            return (f->ff_fdate >> 9) + 1980;
    }

    /* Extracts the month from the date. */
    static int xmonth (struct ffblk *f)
    {
            return (f->ff_fdate >> 5) & 0x0F;
    }

    /* Extracts the day from the date. */
    static int xday (struct ffblk *f)
    {
            return f->ff_fdate & 0x1F;
    }

    /* Writes a new entry. */
    static void wEntry (char *fn, int f, int h, int m, int y, int d,
                        int mm, unsigned long fl, int lwr)
    {
        int j, i = strlen (fn);
        static re_t re;
        char *s;

            if (lwr) strlwr (fn);

            s = fn;

            while ((s = strchr (s, '/')) != NULL)
                *s++ = '/';

            j = (E_SIZE + i + 3) & -4;

            re.e_sign1 = E_SIGN1;
            re.e_sign2 = E_SIGN2;
            re.e_size = j;

            re.flags = f;
            re.hour = h;
            re.minutes = m;
            re.year = y;
            re.month = mm;
            re.day = d;
            re.flen = fl;
            re.nlen = i;
            re.pad = fl ? 3 & (4 - (fl & 3)) : 0;

            memcpy (re.fname, fn, i);

            if (encryptHeader)
            {
                memcpy (kbuf, keyword, klen);
                ax__encrypt (((char *)&re) + 2, j, ebuf, kbuf, klen);
                memcpy (((char *)&re) + 2, ebuf, j);
            }

            xputw (re.e_size, xfile);
            xputw (re.e_sign1, xfile);
            putc (re.flags, xfile);
            putc (re.hour, xfile);
            putc (re.minutes, xfile);
            xputw (re.year, xfile);
            putc (re.month, xfile);
            putc (re.day, xfile);
            xputd (re.flen, xfile);
            xputw (re.nlen, xfile);
            putc (re.pad, xfile);
            xputw (re.e_sign2, xfile);
            xputw (re.res, xfile);
			fwrite ((void *)&re.fname, 1, j-20, xfile);
    }

    /* Packs up a directory entry. */
    static void pack_dir (char *s)
    {
            wEntry (s, 0x80, 0, 0, -1, 0, 0, 0, lowercase);
            dcount++;
    }

    /* Packs up the given entry */
    static int pack (char *path, struct ffblk *f)
    {
        unsigned long left;
        int i, j, k;
        FILE *fp;

            if (!strcmp (f->ff_name, sxfile))
                return 0;

            strcat (strcpy (tbuf, path), f->ff_name);

            if (lowercase) strlwr (tbuf);

            printf ("Adding %s...", tbuf);

            if (isdir (f))
            {
                wEntry (tbuf, 0x80, xhour (f), xminutes (f), xyear (f),
                        xday (f), xmonth (f), 0, 0);

                dcount++;
            }
            else
            {
                fp = fopen (tbuf, "rb");
                if (fp == NULL)
                {
                    printf ("failed\n");
                    return 0;
                }

                wEntry (tbuf, 0x00, xhour (f), xminutes (f), xyear (f),
                        xday (f), xmonth (f), f->ff_fsize, 0);

                if ((left = f->ff_fsize) != 0)
                {
                    left = (left + 3) & -4;

                    memcpy (kbuf, keyword, klen);

                    while (left)
                    {
                        if (left > sizeof(buf))
                            k = sizeof(buf);
                        else
                            k = left;

                        fread (buf, 1, k, fp);

                        if (encryptData)
                        {
                            ax__encrypt (buf, k, ebuf, kbuf, klen);
                            fwrite (ebuf, 1, k, xfile);
                        }
                        else
                            fwrite (buf, 1, k, xfile);

                        left -= k;
                    }
                }

                fclose (fp);

                fcount++;
            }

            printf ("done\n");

            return 0;
    }

    /* Lists the given package entry. */
    int list (re_t *x, char *p)
    {
            if (x->year == 0xFFFF)
            {
                printf ("%-38s --/--/---- --:-- %s%s ",
                    x->fname, x->flags & 0x40 ? "r" : " ", x->flags & 0x20 ? "h" : " ");
            }
            else
            {
                printf ("%-38s %02i/%02i/%04i %02i:%02i %s%s ",
                    x->fname, x->month, x->day, x->year, x->hour, x->minutes,
                    x->flags & 0x40 ? "r" : " ", x->flags & 0x20 ? "h" : " ");
            }

            if (x->flags & 0x80)
                printf ("** DIRECTORY **");
            else
                printf ("%lu bytes", x->flen);

            total += x->flen;

            printf ("\n");

            if (x->pad != 4) /* Not in Compatibility-Mode */
                fseek (xfile, (x->flen + 3) & -4, SEEK_CUR);
            else
                fseek (xfile, x->flen, SEEK_CUR);

            return 0;
    }

    /* Unpacks the given entry. */
    int unpack (re_t *x, char *path)
    {
        unsigned long left, k;
        struct utimbuf ut;
        struct tm tm;
        FILE *fp;

            if (path != NULL)
            {
                strcpy (tbuf, path);
                strcat (tbuf, x->fname);
            }
            else
                strcpy (tbuf, x->fname);

            printf ("Extracting %s...", tbuf);

            if (!(x->flags & 0x80))
            {
                if (access (tbuf, 0x00) != -1)
                    remove (tbuf);

                if (x->pad == 4)
                {
                    left = x->flen;
                    x->pad = 0;
                }
                else
                    left = (x->flen + 3) & -4;

                fp = fopen (tbuf, "wb");
                if (fp == NULL)
                {
                    printf ("failed\n");

                    fseek (xfile, left, SEEK_CUR);
                    return 0;
                }

                memcpy (kbuf, keyword, klen);

                while (left)
                {
                    if (left > sizeof(buf))
                        k = sizeof(buf);
                    else
                        k = left;

                    fread (buf, 1, k, xfile);

                    if (encryptData)
                    {
                        ax__decrypt (buf, k, ebuf, kbuf, klen);
                        fwrite (ebuf, 1, k < sizeof(buf) ? k - x->pad : k, fp);
                    }
                    else
                        fwrite (buf, 1, k < sizeof(buf) ? k - x->pad : k, fp);

                    left -= k;
                }

                fclose (fp);
            }
            else
                mkdir (tbuf);

            printf ("done\n");

            if (x->year == 0xFFFF) return 0;

            tm.tm_year = x->year - 1900;
            tm.tm_mon = x->month - 1;
            tm.tm_min = x->minutes;
            tm.tm_hour = x->hour;
            tm.tm_mday = x->day;
            tm.tm_isdst = 0;
            tm.tm_sec = 0;

            ut.modtime = mktime (&tm);
            utime (tbuf, &ut);

            return 0;
    }

    /* This function is necessary (zurry ANSI). */
    int getch (void);

    /* Reads a keyword from stdin. */
    int read_keyword (char *dest, int max)
    {
        int ch, len = 0;

            pmsg ("keyword: ");

            while (1)
            {
                ch = getch ();
                if (!ch) continue;

                if (ch == 13) break;

                if (ch == 8)
                {
                    if (!len) continue;

                    putchar (8);
                    putchar (32);
                    putchar (8);

                    len--;

                    continue;
                }

                if (ch < 32 || len >= max) continue;

                dest [len++] = ch;

                putchar ('*');
            }

            ch = len;

            while (len < max) dest [len++] = '\0';

            return ch;
    }

    /* Prepares the keyword and detects if it is an Eve hash. */
    static int prepareKeyword (char *s, int len, char *d)
    {
		if (!strnicmp (s, "eve(", 4) && len > 4)
		{
			len -= 5;
			s += 4;

			s [len] = '\0';

			eveHash = 1;
		}

		if (s != pKeyword) memcpy (pKeyword, s, pKlen = len);

		return ax__encrypt (s, len, d, NULL, 0);
    }

    /* This is the main file. */
    int main (int argc, char *argv [])
    {
        int recurse = 0, action = A_PACK, attr = FA_ENTIRE;
        char *path, *mask, *filename, *s, *key;
        int i, j, sign;
        eve_ctx_t P;

            printf ("RedStar Package (RP) Utility v4.00 Copyright (C) 2007-2012 RedStar Technologies\n");

            path = filename = NULL;
            lowercase = 0;

            if (argc < 2)
            {
                printf ("Syntax: rp4 [options] filename[.rp] source-path     * Build date: %s\n\n"
                        "Options:\n"
                        "  -unpack   Unpacks the given file         -list    Lists the package contents\n"
                        "  -recurse  Recurse subdirectories         -lwrcase Converts to lowercase\n"
                        "  -key xxx  Set package keyword            -ench    Encrypt entry headers\n"
                        "\n", __DATE__);

				return 1;
            }

            encryptData = encryptHeader = eveHash = 0;

            for (i = 1; i < argc; i++)
            {
                if (*argv [i] == '-')
                {
                    argv [i]++;

                    for (j = 0; options [j] != NULL && strcmp (argv [i],
                         options [j]); j++);

                    if (options [j] == NULL)
                    {
                        pmsg ("*incorrect commant line option '%s'\n", argv [i]);
                        return 2;
                    }

                    switch (j)
                    {
                        case 0x00: /* -unpack */
                        case 0x01:
                            action = A_UNPACK;
                            break;

                        case 0x02: /* -list */
                        case 0x03:
                            action = A_LIST;
                            break;

                        case 0x04: /* -recurse */
                        case 0x05:
                            recurse = 1;
                            break;

                        case 0x06: /* -lwrcase */
                        case 0x07:
                            lowercase = 1;
                            break;

                        case 0x08: /* -key */
                        case 0x09:
                            key = argv [++i];
                            encryptData = 1;
                            break;

                        case 0x0A: /* -ench */
                        case 0x0B:
                            encryptHeader = 1;
                            break;
                    }

                    continue;
                }

                if (filename == NULL)
                {
                    if (!hasextension (toSlash (filename = argv [i])))
                        strcat (filename = duplicate (filename, 3), ".rp");
                    else
                        filename = duplicate (filename, 0);

                    strlwr (filename);

                    continue;
                }

                if (path == NULL)
                {
                    if (*(mask = getfilename (toSlash (argv [i]))) == '\0')
                        mask = "*.*";

                    path = getpath (argv [i]);

                    continue;
                }

                pmsg ("*why is there a third (%s) non-option argument?\n", argv [i]);
                return 2;
            }

            if (encryptData)
            {
                memset (tbuf, 0, 128);
                strcpy (tbuf, key);

                klen = prepareKeyword (tbuf, strlen (tbuf), keyword);
            }

            if (!encryptData && encryptHeader && action == A_PACK)
            {
                klen = prepareKeyword (tbuf, read_keyword (tbuf, 128), keyword);
                encryptData = 1;
            }

            encryptHeader = encryptHeader ? (1 << 14) : 0;
            encryptData = encryptData ? (1 << 15) : 0;
            eveHash = eveHash ? (1 << 13) : 0;

            if (path == NULL && action == A_PACK)
            {
                mask = "*.*";
                path = "";
            }

            if (action == A_PACK)
            {
                if (access (filename, 0x00) != -1)
                    remove (filename);

                xfile = fopen (filename, "wb");
            }
            else
                xfile = fopen (filename, "rb");

            if (xfile == NULL)
            { bad:
                if (action == A_PACK)
                    pmsg ("*unable to create or open output file %s\n", filename);
                else
                    pmsg ("*unable to open input file %s\n", filename);

                return 2;
            }

            if (action != A_PACK)
            {
                sign = xgetw (xfile);

                if (sign != RP_SIGN && sign != ORP_SIGN)
                {
                    pmsg ("*input file (%s) is not a package.\n", filename);

                retx:
                    fclose (xfile);
                    return 2;
                }

                fcount = xgetd (xfile);
                dcount = xgetd (xfile);

                i = xgetw (xfile);

                fseek (xfile, 4, SEEK_CUR);

                if (sign == ORP_SIGN) goto skip;

                if ((i & 0x8000) && !(i & 0x4000))
                    fread (tbuf, 1, 16, xfile);

                if (action == A_UNPACK)
                {
                    if ((i & 0x8000) && !encryptData)
                        klen = prepareKeyword (buf, read_keyword (buf, 128), keyword);
                }
                else
                {
                    if ((i & 0x6000) && !encryptData)
                        klen = prepareKeyword (buf, read_keyword (buf, 128), keyword);
                }

                encryptHeader = i & 0x4000;
                encryptData = i & 0x8000;
                eveHash = i & 0x2000;

                if (eveHash)
                {
                    eve__init (&P);
                    eve__update (&P, pKeyword, pKlen);
                    eve__final (&P);

                    klen = prepareKeyword (eve__text (&P), 32, keyword);
                }

                if (encryptData && !encryptHeader && action == A_UNPACK)
                {
                    memcpy (kbuf, keyword, klen);
                    ax__decrypt (tbuf, 16, ebuf, kbuf, klen);

                    if (memcmp (ebuf, "Angela Algorithm", 16))
                    {
                        pmsg ("*incorrect keyword provided.\n");
                        return 1;
                    }
                }
            }

      skip: printf ("\n");

            sxfile = filename;

            switch (action)
            {
                case A_PACK:
                    pmsg (" *** Packing file %s ***\n\n", filename);

                    xputw (RP_SIGN, xfile); /* Signature */
                    xputd (0, xfile); /* fcount */
                    xputd (0, xfile); /* dcount */
                    xputw (encryptData | encryptHeader | eveHash, xfile); /* flags */
                    xputd (0, xfile); /* reserved */

                    if (!encryptHeader && encryptData)
                    {
                        memcpy (kbuf, keyword, klen);
                        ax__encrypt ("Angela Algorithm", 16, ebuf, kbuf, klen);

                        fwrite (ebuf, 1, 16, xfile);
                    }

                    for (s = path; *s != '\0'; s++)
                    {
                        if (*s != '/') continue;

                        *s = '\0';

                        pack_dir (path);

                        *s = '/';
                    }

                    walk_directory (path, mask, attr, recurse, pack);

                    fseek (xfile, 2, SEEK_SET);

                    xputd (fcount, xfile);
                    xputd (dcount, xfile);

                    i = 0;

                    break;

                case A_UNPACK:
                    pmsg (" *** Unpacking file %s ***\n\n", filename);

                    if (path != NULL)
                    {
                        if (mask [i = strlen (mask)] == '/')
                            mask [i] = '\0';

                        if ((j = path [i = strlen (path)] == '/') != 0)
                        {
                            path [i] = '\0';

                            if (access (path, 0x00) == -1)
                            { uhuh:
                                pmsg ("*unable to access %s\n", path);
                                goto retx;
                            }

                            path [i] = '/';
                        }

                        path = strcat (duplicate (s = path, strlen (mask) + 1), mask);

                        if (access (path, 0x00) == -1)
                            mkdir (path);

                        if (access (path, 0x00) == -1)
                            goto uhuh;

                        strcat (path, "/");

                        free (mask);
                        free (s);
                    }

                    if (sign == ORP_SIGN)
                        i = owalk_package (unpack, path);
                    else
                        i = walk_package (unpack, path);

                    break;

                case A_LIST:
                    pmsg (" *** Listing Contents of %s ***\n\n", filename);

                    total = 0;

                    if (sign == ORP_SIGN)
                        i = owalk_package (list, path);
                    else
                        i = walk_package (list, path);

                    if (!i)
                    {
                        printf ("\n %lu files               %lu bytes in total"
                                "\n %lu directories\n", fcount, total, dcount);
                    }

                    break;
            }

            pmsg (" *** done\n");

            fclose (xfile);

            return i;
    }
