/*
    TEXT.C

    Copyright (C) 2008 Novak Laboratories
    Written by J. Palencia (ilina@bloodykisses.zzn.com)
*/

    #include <game.h>

    typedef struct /* direct cast: sprite_t */
    {
        sprite_t    sprite;

        int         x, y, r, g, b, dec, st;
        char        *s;
    }
    text_t;

    static int handler (text_t *p, int m)
    {
        int x, y, c;

            if (p->r == p->g && p->g == p->b && !p->b)
            {
                removeSprite (p);

                delete (p->s);
                delete (p);

                return 0;
            }

            if (p->st)
                bprintfs (p->x, p->y, p->s, p->r, p->g, p->b, sX1, sY1);
            else
                bprintf (p->x, p->y, p->s, p->r, p->g, p->b, sX1, sY1);

            p->r -= p->dec;
            p->g -= p->dec;
            p->b -= p->dec;

            if (p->r < 0) p->r = 0;
            if (p->g < 0) p->g = 0;
            if (p->b < 0) p->b = 0;

            return 0;
    }

    void createText (int x, int y, char *st, int r, int g, int b, int dec, int sts)
    {
        text_t *s;

            s = new (text_t);

            s->dec = dec;

            s->s = dups (st);

            if (x == -1) x = (s__width - (int)bf2 (BStringWidth, Ct, s->s)) >> 1;

            s->x = x;
            s->y = y;

            s->r = r;
            s->g = g;
            s->b = b;

            s->st = sts;

            spriteHandler (s, handler);
            addSprite (s, t__back);
    }
