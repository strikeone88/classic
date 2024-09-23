/*
    BOOM.C

    Copyright (C) 2008 Novak Laboratories
    Written By J. Palencia (ilina@bloodykisses.zzn.com)
*/

    #include <game.h>

    #define aLen    (256 / BoomStep)

    typedef struct /* direct cast: sprite_t */
    {
        sprite_t    sprite;

        int         x, y, r, g, b, t, w;

        int         dA [aLen],
                    dT [aLen];
    }
    boom_t;

    static int lX, lY;

    static int drawLine (int t, int c1, int c2, int d)
    {
        static int x1, y1, x2, y2;

            x2 = lX + rcost (d + BoomLen, t);
            y2 = lY - rsint (d + BoomLen, t);

            x1 = lX + rcost (d, t);
            y1 = lY - rsint (d, t);

            if (x1 < 0 || y1 < 0 || x2 < 0 || y2 < 0 ||
                x1 >= sWidth || y1 >= sHeight || x2 >= sWidth ||
                y2 >= sHeight) return 0;

            if (useHQ)
                lineBlend (x1, y1, x2, y2, c1, c2, sX1, sY1);
            else
                line (x1, y1, x2, y2, c1, sX1, sY1);

            return 1;
    }

    static int handler (boom_t *p)
    {
        int i, c, c2, d, r, g, b;

            lX = p->x;
            lY = p->y;

            if (!packrgb (p->r, p->g, p->b))
            {
                removeSprite (p);
                delete (p);

                return 0;
            }

            c2 = packrgb (p->w, p->w, p->w);

            for (i = 0; i < aLen; i++)
            {
                #define K   31
                
                r = p->r + (rand () & K);
                g = p->g + (rand () & K);
                b = p->b + (rand () & K);

                if (r > 255) r = 255;
                if (g > 255) g = 255;
                if (b > 255) b = 255;

                c = packrgb (r, g, b);

                d = p->dA [i] + rsint (BoomDist, p->t);;
                drawLine (p->dT [i], c, c2, d);

                d = p->dA [i] + rsint (BoomDist >> 1, p->t);
                drawLine (p->dT [i], c, c2, d);

                d = p->dA [i] + rsint (BoomDist >> 2, p->t);
                drawLine (p->dT [i], c, c2, d);

                d = p->dA [i] + rsint (BoomDist >> 3, p->t);
                drawLine (p->dT [i], c, c2, d);
            }

            p->t += BoomSpeed;
            if (p->t > _90deg) p->t = _90deg;

            p->r -= BoomDec; if (p->r < 0) p->r = 0;
            p->g -= BoomDec; if (p->g < 0) p->g = 0;
            p->b -= BoomDec; if (p->b < 0) p->b = 0;

            p->w -= BoomDec; if (p->w < 0) p->w = 0;

            return 0;
    }

    void createBoom (int x, int y, int r, int g, int b)
    {
        boom_t *s;

            s = new (boom_t);

            s->x = x;
            s->y = y;

            s->r = r;
            s->g = g;
            s->b = b;

            s->w = 255;

            for (x = 0; x < aLen; x++)
                s->dA [x] = (rand () & BoomRad) - (rand () & BoomRad),
                s->dT [x] = rand () & 255;

            spriteHandler (s, handler);
            addSprite (s, t__back);
    }
