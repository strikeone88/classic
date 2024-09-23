/*
    NLA.C

    Novak Labs. Animation Engine Version 0.01

    This engine will load and show 5:6:5 NLA files.

    Copyright (C) 2008 Novak Laboratories
    Written by J. Palencia (ilina@bloodykisses.zzn.com)
*/

    #include <stdlib.h>
    #include <nla.h>

    /* View port. */
    int x1, y1, x2, y2;

    /* Graphics. */
    ServiceInterface gdev, gfx;

    /* Returns the count of bits required to represent the value of n. */
    static int bitsRequired (unsigned n)
    {
        int bits, moves;

            for (bits = moves = 1; n; n >>= 1, moves++)
                if (n & 1) bits = moves;

            return bits;
    }

    void *expandFrame (anim_t *q, unsigned char *buf, int r, int g, int b)
    {
        unsigned char *dest, *xdest, *t;
        int i, j, k, cbits, bits, mask;
        unsigned long temp, *pal;

            mask = (1 << (bits = q->bits)) - 1;
            pal = (void *)q->palette;
            cbits = temp = 0;

            j = q->xres * q->yres;
            xdest = dest = malloc (j * 4);

            for (i = 0; i < j; i++)
            {
                while (cbits < bits)
                {
                    temp = (temp << 8) | *buf++;
                    cbits += 8;
                }

                k = temp >> (cbits -= bits);
                t = (unsigned char *)&pal [k & mask];

                *dest++ = *t++ & r;
                *dest++ = *t++ & g;
                *dest++ = *t++ & b;
                dest++;
            }

            return xdest;
    }

    #define inside(i,j) (x1 <= (i) && (i) <= x2 && y1 <= (j) && (j) <= y2)

    /* Draws a frame and uses the coordinates as the center. */
    int nla__drawframe (int x, int y, anim_t *q, unsigned frame, int nadd)
    {
        int i, j, k, u, v, uw = q->uw, uh = q->uh;
        unsigned char *buf, *t;
        unsigned long temp;

            u = x + uw - 1;
            v = y + uh - 1;

            i = x - q->w;
            j = y - q->h;

            buf = q->frames [frame];

            if (inside (i, j))
            {
                if (inside (u, v))
                {
                    if (nadd)
                        gfx (GPutBuffer, x - q->w, y - q->h, q->xres, q->yres, buf);
                    else
                        gfx (GAddBuffer, x - q->w, y - q->h, q->xres, q->yres, buf);

                    return 0;
                }
            }
            else
                if (!inside (u, v)) return 1;

            if (nadd)
            {
                for (j = -q->h, v = y - q->h; j < uh; j++, v++)
                {
                    for (i = -q->w, u = x - q->w; i < uw; i++, u++)
                    {
                        if (x1 <= u && u <= x2 && y1 <= v && v <= y2)
                            gfx (GPutPixelP, u, v, buf);

                        buf += 4;
                    }
                }
            }
            else
            {
                for (j = -q->h, v = y - q->h; j < uh; j++, v++)
                {
                    for (i = -q->w, u = x - q->w; i < uw; i++, u++)
                    {
                        if (x1 <= u && u <= x2 && y1 <= v && v <= y2)
                            gfx (GAddPixelP, u, v, buf);

                        buf += 4;
                    }
                }
            }

            return 0;
    }

    int nla__drawbuf (int x, int y, anim_t *q, unsigned char *buf)
    {
        int i, j, k, u, v, uw = q->uw, uh = q->uh;
        unsigned long temp;
        unsigned char *t;

            u = x + uw - 1;
            v = y + uh - 1;

            i = x - q->w;
            j = y - q->h;

            if (inside (i, j))
            {
                if (inside (u, v))
                {
                    gfx (GAddBuffer, x - q->w, y - q->h, q->xres, q->yres, buf);
                    return 0;
                }
            }
            else
                if (!inside (u, v)) return 1;

            for (j = -q->h, v = y - q->h; j < uh; j++, v++)
            {
                for (i = -q->w, u = x - q->w; i < uw; i++, u++)
                {
                    if (x1 <= u && u <= x2 && y1 <= v && v <= y2)
                        gfx (GAddPixelP, u, v, buf);

                    buf += 4;
                }
            }

            return 0;
    }

    void *nla (unsigned Command, ...)
    {
        int i, j, k, w, r, g, b;
        unsigned char *p, *q;
        anim_t *x, *t;
        va_list args;

            va_start (args, Command);

            switch (Command)
            {
                case NCreateEx:
                    p = va_arg (args, unsigned char *);
                    if (!p) break;

                    r = va_arg (args, int);
                    g = va_arg (args, int);
                    b = va_arg (args, int);

                    goto _NCreate;

                case NCreate:
                    r = g = b = 255;

                    p = va_arg (args, unsigned char *);
                    if (!p) break;

          _NCreate: if (*((short *)p)++ != 'A5') break;
                    if (*((short *)p)++ != '65') break;

                    x = new (anim_t);

                    x->xres = *((short *)p)++;
                    x->yres = *((short *)p)++;

                    x->bits = bitsRequired ((k = *((short *)p)++) - 1);
                    x->fcount = *((short *)p)++;

                    j = (x->xres * x->yres * x->bits) >> 3;
                    x->colors = k;

                    x->frames = malloc (x->fcount << 2);
                    x->palette = q = malloc (k * 4);

                    for (i = 0; i < k; i++)
                    {
                        w = *((unsigned short *)p)++;

                        *q++ = (w >> 11) << 3;
                        *q++ = ((w >> 5) & 0x3F) << 2;
                        *q++ = (w & 0x1F) << 3;
                        q++;
                    }

                    for (i = 0; i < x->fcount; i++)
                    {
                        x->frames [i] = expandFrame (x, p, r, g, b);
                        p += j;
                    }

                    x->uw = x->w = x->xres >> 1;
                    x->uh = x->h = x->yres >> 1;

                    if (2*x->w != x->xres) x->uw++;
                    if (2*x->h != x->yres) x->uh++;

                    return x;

                case NDestroy:
                    x = va_arg (args, anim_t *);
                    if (!x) returnv (1);

                    for (i = 0; i < x->fcount; i++)
                        delete (x->frames [i]);

                    delete (x->palette);
                    delete (x->frames);
                    delete (x);

                    break;

                case NSetViewPort:
                    x1 = va_arg (args, int);
                    y1 = va_arg (args, int);
                    x2 = va_arg (args, int);
                    y2 = va_arg (args, int);
                    break;

                case NSetGfx:
                    gfx = va_arg (args, ServiceInterface);
                    if (gfx) gdev = gfx (GGetDevice); else gdev = NULL;
                    break;

                case NScreenView:
                    x1 = y1 = 0;

                    y2 = (int)gdev (GScreenHeight) - 1;
                    x2 = (int)gdev (GScreenWidth) - 1;
                    break;

                case NDrawFrame: //anim, frame, x, y
                    x = va_arg (args, anim_t *);
                    if (!x) returnv (1);

                    k = va_arg (args, int);
                    if (0 > k || k > x->fcount) returnv (1);

                    i = va_arg (args, int);
                    j = va_arg (args, int);

                    returnv (nla__drawframe (i, j, x, k, 0));

                case NDrawFrameP: //anim, frame, x, y
                    x = va_arg (args, anim_t *);
                    if (!x) returnv (1);

                    k = va_arg (args, int);
                    if (0 > k || k > x->fcount) returnv (1);

                    i = va_arg (args, int);
                    j = va_arg (args, int);

                    returnv (nla__drawframe (i, j, x, k, 1));

                case NDrawBuf: //anim, buf, x, y
                    x = va_arg (args, anim_t *);
                    if (!x) returnv (1);

                    p = va_arg (args, unsigned char *);
                    if (!p) returnv (1);

                    i = va_arg (args, int);
                    j = va_arg (args, int);

                    returnv (nla__drawbuf (i, j, x, p));

                case NPaletteAND:
                    x = va_arg (args, anim_t *);
                    if (!x) returnv (1);

                    r = va_arg (args, int) & 0xFF;
                    g = va_arg (args, int) & 0xFF;
                    b = va_arg (args, int) & 0xFF;

                    p = x->palette;

                    for (i = 0, j = x->colors; j; j--, i++, p++)
                    {
                        *p++ &= r;
                        *p++ &= g;
                        *p++ &= b;
                    }

                    break;
            }

            return NULL;
    }

    int main (void)
    {
            registerInterface ("nla-0.01", &nla);
            setResident ();

            printf ("nla-0.01\n");
            return 0;
    }
