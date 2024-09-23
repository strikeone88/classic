/*
    NLA.C

    Novak Labs. Animation Engine Version 0.01

    This engine will load and show 5:6:5 NLA files.

    Copyright (C) 2008 Novak Laboratories
    Written by J. Palencia (ilina@bloodykisses.zzn.com)
*/

    #include <stdio.h>
    #include <nla.h>
    #include <gfx.h>

    /* Returns the count of bits required to represent the value of n. */
    static int bitsRequired (unsigned n)
    {
        int bits, moves;

            for (bits = moves = 1; n; n >>= 1, moves++)
                if (n & 1) bits = moves;

            return bits;
    }

    /* Loads an NLA animation and returns its structure. */
    anim_t *nla__load (char *fname)
    {
        FILE *fp = fopen (fname, "rb");
        anim_t *x;
        int i, colors, frameLen;

            if (!fp) return NULL;

            if (getw (fp) != 'A5')
            {
           ret: fclose (fp);
                return NULL;
            }

            if (getw (fp) != '65') goto ret;

            x = new (anim_t);

            x->xres = getw (fp);
            x->yres = getw (fp);

            x->bits = bitsRequired ((colors = getw (fp)) - 1);
            x->fcount = getw (fp);

            frameLen = ((long)x->xres * (long)x->yres * x->bits) >> 3;

            x->frames = alloc (x->fcount << 2);
            x->palette = alloc (x->pallen = colors << 1);

            fread (x->palette, colors, 2, fp);

            for (i = 0; i < x->fcount; i++)
            {
                x->frames [i] = alloc (frameLen);
                fread (x->frames [i], frameLen, 1, fp);
            }

            fclose (fp);

            return x;
    }

    /* Destroys a previously loaded NLA. */
    void nla__destroy (anim_t *x)
    {
        int i;

            if (!x) return;

            for (i = 0; i < x->fcount; i++) delete (x->frames [i]);

            delete (x->palette);
            delete (x->frames);

            delete (x);
    }

    /* Draws a frame and uses the coordinates as the center. */
    void nla__drawframe (int x, int y, anim_t *q, unsigned frame,
                         int x1, int y1, int x2, int y2)
    {
        static int i, j, k, w, uw, h, uh, u, v, cbits, bits, mask;
        static unsigned char *buf;
        static unsigned long temp;
        static unsigned *pal;

            if (!x || 0 > frame || frame > q->fcount)
                return;

            uw = w = q->xres >> 1;
            uh = h = q->yres >> 1;

            if (2*w != q->xres) uw++;
            if (2*h != q->yres) uh++;

            u = x + uw - 1;
            v = y + uh - 1;

            i = x - w;
            j = y - h;

            if ((x1 > i || i > x2) && (y1 > j || j > y2) &&
                (x1 > u || u > x2) && (y1 > v || v > y2)) return;

            mask = (1 << (bits = q->bits)) - 1;
            buf = q->frames [frame];
            pal = q->palette;
            cbits = temp = 0;

            for (j = -h, v = y - h; j < uh; j++, v++)
            {
                for (i = -w, u = x - w; i < uw; i++, u++)
                {
                    while (cbits < bits)
                    {
                        temp = (temp << 8) | *buf++;
                        cbits += 8;
                    }

                    k = temp >> (cbits -= bits);

                    if (x1 <= u && u <= x2 &&
                        y1 <= v && v <= y2)
                    {
                        putpixelBlend (u, v, pal [k & mask]);
                    }
                }
            }
    }

    /* Clones an NLA animation and dups the palette but not the data. */
    anim_t *nla__clone (anim_t *x)
    {
        anim_t *p = new (anim_t);

            memcpy (p, x, sizeof (anim_t));

            p->palette = alloc (x->pallen);

            memcpy (p->palette, x->palette, x->pallen);

            return p;
    }

    /* Modifies the palette of the given NLA (using bitwise AND) . */
    void nla__paletteAND (anim_t *x, int aR, int aG, int aB)
    {
        int i, n, r, g, b;

            for (i = 0, n = x->pallen >> 1; n; n--, i++)
            {
                unpackrgb (x->palette [i], r, g, b);
                x->palette [i] = packrgb (r & aR, g & aG, b & aB);
            }
    }
