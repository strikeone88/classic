/*
    TEXT.C

    Copyright (C) 2008 Novak Laboratories
    Written by J. Palencia (ilina@bloodykisses.zzn.com)
*/

    #include <game.h>

    typedef struct /* direct cast: sprite_t */
    {
        sprite_t    sprite;

        int         x, y, r, g, b, dec;
        char        *s;
    }
    text_t;

    static int handler (text_t *p, int m)
    {
        int x, y, c;

            c = packrgb (p->r, p->g, p->b);
            if (!c)
            {
                removeSprite (p);

                delete (p->s);
                delete (p);

                return 0;
            }

            bprintf (p->x, p->y, p->s, packrgb (p->r, p->g, p->b), sX1, sY1);

            p->r -= p->dec;
            p->g -= p->dec;
            p->b -= p->dec;

            if (p->r < 0) p->r = 0;
            if (p->g < 0) p->g = 0;
            if (p->b < 0) p->b = 0;

            return 0;
    }

    void createText (int x, int y, char *st, int r, int g, int b, int dec)
    {
        text_t *s;

            s = new (text_t);

            s->dec = dec;

            s->x = x;
            s->y = y;

            s->r = r;
            s->g = g;
            s->b = b;

            s->s = dups (st);

            spriteHandler (s, handler);
            addSprite (s, t__back);
    }
