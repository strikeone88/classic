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

        int         x, y, r, g, b, c, w, t, delay, bdelay;

        int         dT [aLen], dA [aLen], dC [aLen];
    }
    hboom_t;

    /* Sprite limits and half-resolutions. */
    static int hw, hh, hyl, hxl, lxl, lyl, xr, yr;

    static anim_t *base;
    static anim_t *lanim [8];

    static int lX, lY, cX, cY;

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

            cX = (x1 + x2) >> 1;
            cY = (y1 + y2) >> 1;

            if (useHQ)
                lineBlend (x1, y1, x2, y2, c1, c2, sX1, sY1);
            else
                line (x1, y1, x2, y2, c1, sX1, sY1);

            return 1;
    }

    static int handler (hboom_t *p, int m)
    {
        static int i, d, c, c2, r, g, b;

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
                #define K 31

                r = p->r + (rand () & K);
                g = p->g + (rand () & K);
                b = p->b + (rand () & K);

                if (r > 255) r = 255;
                if (g > 255) g = 255;
                if (b > 255) b = 255;

                c = packrgb (r, g, b);

                d = p->dA [i] + rsint (BoomDist, p->t);
                if (drawLine (p->dT [i], c, c2, d) && p->dC [i] < HBoomFadeC)
                    drawFrame (cX, cY, lanim [p->c], ((p->dT [i] >> HBoomStepL2) << HBoomFadeCL2) + p->dC [i]);

                d = p->dA [i] + rsint (BoomDist >> 1, p->t);
                if (drawLine (p->dT [i], c, c2, d) && p->dC [i] < HBoomFadeC)
                    drawFrame (cX, cY, lanim [p->c], ((p->dT [i] >> HBoomStepL2) << HBoomFadeCL2) + p->dC [i]);

                d = p->dA [i] + rsint (BoomDist >> 2, p->t);
                if (drawLine (p->dT [i], c, c2, d) && p->dC [i] < HBoomFadeC)
                    drawFrame (cX, cY, lanim [p->c], ((p->dT [i] >> HBoomStepL2) << HBoomFadeCL2) + p->dC [i]);

                d = p->dA [i] + rsint (BoomDist >> 3, p->t);
                if (drawLine (p->dT [i], c, c2, d) && p->dC [i] < HBoomFadeC)
                    drawFrame (cX, cY, lanim [p->c], ((p->dT [i] >> HBoomStepL2) << HBoomFadeCL2) + p->dC [i]);
            }

            p->t += BoomSpeed;
            if (p->t > _90deg) p->t = _90deg;

            if (!p->delay--)
            {
                p->delay = p->bdelay;

                for (i = 0; i < aLen; i++)
                    p->dC [i]++;
            }

            p->r -= BoomDec; if (p->r < 0) p->r = 0;
            p->g -= BoomDec; if (p->g < 0) p->g = 0;
            p->b -= BoomDec; if (p->b < 0) p->b = 0;

            p->w -= BoomDec; if (p->w < 0) p->w = 0;

            return 0;
    }

    void createHBoom (int x, int y, int c, int delay, int r, int g, int b)
    {
        static int initialized = 0;
        hboom_t *s;

            if (!useHBoom) return;

            if (!initialized)
            {
                base = nla__load ("data/line.nla");
                if (!base) animDie ("line");

                hw = (xr = base->xres) >> 1;
                hh = (yr = base->yres) >> 1;

                hyl = sHeight - hh;
                hxl = sWidth - hw;

                lxl = hw;
                lyl = hh;

                lanim [HBoomMagenta] = nla__clone (base);
                lanim [HBoomYellow] = nla__clone (base);
                lanim [HBoomOrange] = nla__clone (base);
                lanim [HBoomGreen] = nla__clone (base);
                lanim [HBoomCyan] = nla__clone (base);
                lanim [HBoomBlue] = nla__clone (base);
                lanim [HBoomWhite] = base;

                nla__paletteAND (lanim [HBoomMagenta], 255, UNUSED, 255);
                nla__paletteAND (lanim [HBoomYellow], 255, 255, UNUSED);
                nla__paletteAND (lanim [HBoomOrange], 255, 127, UNUSED);
                nla__paletteAND (lanim [HBoomGreen], UNUSED, 255, UNUSED);
                nla__paletteAND (lanim [HBoomCyan], UNUSED, 255, 255);
                nla__paletteAND (lanim [HBoomBlue], UNUSED, UNUSED, 255);

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

            s->w = 255;

            for (x = 0; x < aLen; x++)
            {
                s->dA [x] = (rand () & BoomRad) - (rand () & BoomRad);
                s->dT [x] = rand () & 255;
                s->dC [x] = 0;
            }

            spriteHandler (s, handler);
            addSprite (s, t__back);
    }
