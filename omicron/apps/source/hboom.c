/*
    HBOOM.C

    Copyright (C) 2008 Novak Laboratories
    Written by J. Palencia (ilina@bloodykisses.zzn.com)
*/

    #include <game.h>

    #define aLen    (256 / HBoomStep)

    typedef struct /* direct cast: sprite_t */
    {
        sprite_t    sprite;

        int         x, y, r, g, b, c, t, delay, bdelay, Dec;

        int         dT [aLen], dA [aLen], dC [aLen];
    }
    hboom_t;

    /* Sprite limits and half-resolutions. */
    static int hw, hh, hyl, hxl, lxl, lyl, xr, yr;

    static anim_t *lanim [8];

    static int lX, lY, cX, cY;

    unsigned char lineD [];

    static int drawLine (int t, int r1, int g1, int b1, int r2, int g2, int b2,
                         int d)
    {
        static int x1, y1, x2, y2;

            x2 = lX + rcost (d + BoomLen, t);
            y2 = lY - rsint (d + BoomLen, t);

            x1 = lX + rcost (d, t);
            y1 = lY - rsint (d, t);

            if (x1 < 0 || y1 < 0 || x2 < 0 || y2 < 0 ||
                x1 >= sWidth || y1 >= sHeight || x2 >= sWidth ||
                y2 >= sHeight) return 0;

            cX = (x1 + x2) >> 1;
            cY = (y1 + y2) >> 1;

            lineBlend (x1, y1, x2, y2, r1, g1, b1, r2, g2, b2, sX1, sY1);

            return 1;
    }

    static int handler (hboom_t *p, int m)
    {
        static int q, i, d, r, g, b;

            lX = p->x;
            lY = p->y;

            if (p->r == p->g && p->g == p->b && !p->b)
            {
                removeSprite (p);
                delete (p);

                return 0;
            }

            for (i = 0; i < aLen; i++)
            {
                #define K 31

                r = p->r + (rand () & K);
                g = p->g + (rand () & K);
                b = p->b + (rand () & K);

                r = min (r, 255);
                g = min (g, 255);
                b = min (b, 255);

                #define Magic   rsint
                #define Max     _90deg

                for (q = 1; q < 4; q++)
                {
                    d = p->dA [i] + Magic (BoomDist / q, p->t);
                    if (drawLine (p->dT [i], r, g, b, p->r, p->g, p->b, d) && p->dC [i] < HBoomFadeC)
                        drawFrame (cX, cY, lanim [p->c], ((p->dT [i] >> HBoomStepL2) << HBoomFadeCL2) + p->dC [i]);
                }
            }

            p->t += BoomSpeed;
            if (p->t > Max) p->t = Max;

            if (!p->delay--)
            {
                p->delay = p->bdelay;

                for (i = 0; i < aLen; i++)
                    p->dC [i]++;
            }

            p->r -= p->Dec; if (p->r < 0) p->r = 0;
            p->g -= p->Dec; if (p->g < 0) p->g = 0;
            p->b -= p->Dec; if (p->b < 0) p->b = 0;

            return 0;
    }

    void createHBoom (int x, int y, int c, int delay, int r, int g, int b, int dec)
    {
        static int initialized = 0;
        hboom_t *s;
        anim_t *t;

            if (!initialized)
            {
                lanim [HBoomMagenta] = nla (NCreateEx, lineD, 255, UNUSED, 255);
                lanim [HBoomYellow] = nla (NCreateEx, lineD, 255, 255, UNUSED);
                lanim [HBoomOrange] = nla (NCreateEx, lineD, 255, 127, UNUSED);
                lanim [HBoomGreen] = nla (NCreateEx, lineD, UNUSED, 255, UNUSED);
                lanim [HBoomCyan] = nla (NCreateEx, lineD, UNUSED, 255, 255);
                lanim [HBoomBlue] = nla (NCreateEx, lineD, UNUSED, UNUSED, 255);
                lanim [HBoomWhite] = t = nla (NCreate, lineD);

                hw = (xr = t->xres) >> 1;
                hh = (yr = t->yres) >> 1;

                hyl = sHeight - hh;
                hxl = sWidth - hw;

                lxl = hw;
                lyl = hh;

                initialized = 1;
            }

            if (x == -1) return;

            s = new (hboom_t);

            s->delay = s->bdelay = delay;

            s->x = x;
            s->y = y;
            s->c = c;

            s->r = r;
            s->g = g;
            s->b = b;

            s->Dec = dec;

            for (x = 0; x < aLen; x++)
            {
                s->dA [x] = (rand () & BoomRad) - (rand () & BoomRad);
                s->dT [x] = rand () & 255;
                s->dC [x] = 0;
            }

            spriteHandler (s, handler);
            addSprite (s, t__back);
    }

    #include "line.h"
