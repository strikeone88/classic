/*
    EXP.C

    Copyright (C) 2008 Novak Laboratories
    Written by J. Palencia (ilina@bloodykisses.zzn.com)
*/

    #include <game.h>

    typedef struct /* direct cast: sprite_t */
    {
        sprite_t    sprite;

        int         rad, max, inc;
        int         x, y;
    }
    explosion_t;

    static int handler (explosion_t *p, int m)
    {
        int i1, j1, i2, j2, r, z, t;
        void *q;

            if (p->rad > p->max)
            {
                removeSprite (p);
                delete (p);

                return 0;
            }

            r = p->rad;
            p->rad += p->inc;

            for (t = 0; t < 257; t+=4)
            {
                i1 = p->x + rcost (r, t);
                j1 = p->y - rsint (r, t);

                createItem (i1, j1, 15, 0, RANDOM, t__item, fire, 0);
                createTrail (i1, j1, 2, fire, 32, 64);
            }

            i1 = p->x + rcost (r, _135deg);
            j1 = p->y - rsint (r, _135deg);

            i2 = j2 = 2*r;

            while ((q = spriteCollision (t__enemy, p, i1, j1, i2, j2)) != NULL)
                sendMessage (q, m__destroy);

            return 0;
    }

    void createExplosion (int x, int y, int rad, int maxrad, int inc)
    {
        explosion_t *s;

            s = new (explosion_t);

            s->x = x;
            s->y = y;

            s->max = maxrad;
            s->rad = rad;
            s->inc = inc;

            playSound (S__EXP, 256, 256, 0);

            spriteHandler (s, handler);
            addSprite (s, t__back);
    }
