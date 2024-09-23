/*
    NLA.C

    Novak Labs. Animation Generator Version 0.01

    This utility generates a 5:6:5 color animation using a list of
    24-bit bitmaps, the header simply contains the signature A565,
    the xres and yres (word), the count of palette entries (word),
    the count of frames, then the palette and then the frames.

    Copyright (C) 2008 Novak Laboratories
    Written by J. Palencia (ilina@bloodykisses.zzn.com)
*/

    #include <string.h>
    #include <stdio.h>

    /* Packs the given 8:8:8 into a 5:6:5 integer. */
    #define packrgb(r,g,b) (((((r)&255)>>3)<<11)|((((g)&255)>>2)<<5)|(((b)&255)>>3))

    /* Unpacks the given 5:6:5 integer into an 8:8:8. */
    #define unpackrgb(pack,r,g,b) r=(pack>>11)<<3,g=((pack>>5)&0x3F)<<2,b=(pack&0x1F)<<3

    /* Maximum count of palette entries. */
    #define MAX_PAL 4096    /* 12-bits */

    /* Temporal buffer. */
    char buf [512];

    /* Palette data. */
    unsigned palette [MAX_PAL];

    /* Count of entries used. */
    unsigned entries;

    /* Tolerance constant. */
    int K;

    unsigned findPaletteEntry (unsigned x)
    {
        int i, lr, lg, lb, hr, hg, hb, r, g, b;

            for (i = 0; i < entries; i++)
                if (palette [i] == x) return i + 1;

            if (K)
            {
                unpackrgb (x, r, g, b);

                lr = r - K;
                lg = g - K;
                lb = b - K;

                hr = r + K;
                hg = g + K;
                hb = b + K;

                for (i = 0; i < entries; i++)
                {
                    unpackrgb (palette [i], r, g, b);

                    if (lr <= r && r <= hr &&
                        lg <= g && g <= hg &&
                        lb <= b && b <= hb) return i + 1;
                }
            }

            return 0;
    }

    void addPaletteEntry (unsigned x)
    {
            if (findPaletteEntry (x)) return;
            palette [entries++] = x;
    }

    char *getline (FILE *fp)
    {
            fgets (buf, sizeof (buf), fp);
            if (feof (fp)) return NULL;

            *strchr (buf, '\n') = 0;

            return buf;
    }

    long getd (FILE *fp)
    {
        long A, B;

            A = getw (fp);
            B = getw (fp);

            return (B << 16) | A;
    }

    int checkBMP (char *f, int *xres, int *yres)
    {
        int xr, yr, i, j, r, g, b; long offs, q, vt;
        FILE *fp;

            fp = fopen (f, "rb");
            if (!fp)
            {
                printf ("Error: Can't open BMP file '%s'.\n", f);
                return 1;
            }

            if (getw (fp) != 'BM')
            {
          no24: printf ("Error: Input file '%s' is not a BMP24.\n", f);
                return 1;
            }

            getd (fp);
            getd (fp);

            offs = getd (fp);

            if (getd (fp) != 40) goto no24;

            xr = getd (fp);
            yr = getd (fp);

            if (!*xres) *xres = xr;
            if (!*yres) *yres = yr;

            if (xr != *xres || yr != *yres)
            {
                printf ("Error: File '%s' doesn't have the same resolution (must be %ux%u).\n", f, *xres, *yres);
                return 1;
            }

            if (getw (fp) != 1) goto no24;
            if (getw (fp) != 24) goto no24;

            fseek (fp, offs, SEEK_SET);

            q = 3*xr;
            q = (q + 3) & -4;

            vt = yr - 1;

            for (j = 0; j < yr; j++)
            {
                fseek (fp, (vt - j)*q + offs, SEEK_SET);

                for (i = 0; i < xr; i++)
                {
                    b = getc (fp);
                    g = getc (fp);
                    r = getc (fp);

                    addPaletteEntry (packrgb (r, g, b));
                }
            }

            fclose (fp);

            return 0;
    }

    int addBMP (char *f, FILE *out, int bits, int xr, int yr)
    {
        int i, j, c, t, r, g, b, temp, mask, count; long q, offs, vt;
        FILE *fp;

            fp = fopen (f, "rb");
            if (!fp)
            {
                printf ("Error: Can't open BMP file '%s'.\n", f);
                return 1;
            }

            mask = (1 << bits) - 1;
            count = temp = 0;

            fseek (fp, 10, SEEK_SET);
            offs = getd (fp);

            q = 3*xr;
            q = (q + 3) & -4;

            vt = yr - 1;

            for (j = 0; j < yr; j++)
            {
                fseek (fp, (vt - j)*q + offs, SEEK_SET);

                for (i = 0; i < xr; i++)
                {
                    b = getc (fp);
                    g = getc (fp);
                    r = getc (fp);

                    c = (findPaletteEntry (packrgb (r, g, b)) - 1) & mask;

                    if (count + bits >= 8)
                    {
                        t = (8 - count);
                        temp = (temp << t) | (c >> (count = bits - t));

                        putc (temp, out);

                        temp = c;

                        if (count == 8)
                        {
                            putc (temp, out);

                            temp = 0;
                            count = 0;
                        }
                    }
                    else
                    {
                        temp = (temp << bits) | c;
                        count += bits;
                    }
                }
            }

            fclose (fp);

            return 0;
    }

    int bitsRequired (unsigned x)
    {
        int bits, moves;

            bits = moves = 1;

            while (x)
            {
                if (x & 1) bits = moves;

                x >>= 1;
                moves++;
            }

            return bits;
    }

    int main (int argc, char *argv [])
    {
        int xres, yres, bits, i, frames;
        FILE *fp, *fpx;
        char *f;

            if (argc < 3)
            {
                printf ("Use: NLA BMP24-LIST OUTPUT-FILE TOLERANCE\n");
                return 1;
            }

            if (argc > 3)
                K = atoi (argv [3]);
            else
                K = 0;

            fp = fopen (argv [1], "rt");
            if (!fp)
            {
                printf ("Error: Can't open BMP24 list file %s.\n", argv [2]);
                return 2;
            }

            entries = xres = yres = frames = 0;

            printf ("Building global palette...\n");

            while ((f = getline (fp)) != NULL)
            {
                printf ("Checking %s...\r", f);

                if (checkBMP (f, &xres, &yres)) return 2;

                frames++;
            }

            if ((xres * yres) & 7)
            {
                printf ("Error: Frame area modulus eight MUST be zero (xres*yres MOD 8 = 0).\n");
                return 3;
            }

            bits = bitsRequired (entries - 1);

            printf ("\rDetected a total of %u colors (%u bits per pixel)\n", entries, bits);
            printf ("** Using %lu bytes per frame (%ux%u) **\n", (long)xres*yres*bits >> 3, xres, yres);

            rewind (fp);

            fpx = fopen (argv [2], "wb");
            if (!fpx)
            {
                printf ("!! ERROR: Can't open output file %s !!\n", argv [2]);
                return 3;
            }

            printf ("Generating output NLA file (%s)...\n", argv [2]);

            putw ('A5', fpx);
            putw ('65', fpx);

            putw (xres, fpx);
            putw (yres, fpx);

            putw (entries, fpx);
            putw (frames, fpx);

            for (i = 0; i < entries; i++) putw (palette [i], fpx);

            while ((f = getline (fp)) != NULL)
            {
                printf ("Adding %s...\r", f);

                if (addBMP (f, fpx, bits, xres, yres)) return 2;
            }

            printf ("Output file fully generated (%lu bytes).\n", ftell (fpx));

            return 0;
    }
