/*
    SSHOT.C

    Copyright (C) 2008 Novak Laboratories
    Written by J. Palencia (ilina@bloodykisses.zzn.com)
*/

    #include <game.h>

    typedef struct /* direct cast: sprite_t */
    {
        sprite_t    sprite;

        int         lxl, lyl, hxl, hyl;
        int         hw, hh;

        int         ok, x, y, t1, t2, f, s, delay, c;

        anim_t      *shot;

        int         remains, tdelay;
    }
    shot_t;

    static anim_t *shot [4];

    static int rTable [] = /* Resistance */
    {
        0, 5, 5, 3
    };

    static int dTable [] = /* Delay */
    {
        3, 3, 2, 2
    };

    static int cTable [] = /* Count */
    {
        2, 3, 3, 5
    };

    static int sTable [] = /* Sprite */
    {
        0, 0, 1, 1
    };

    static int handler (shot_t *p, int m)
    {
        int x, y, x1, y1, w, h, i, killed = 0;
        void *q;

            x = p->x;
            y = p->y;

            if (x < p->lxl || y < p->lyl || x > p->hxl || y > p->hyl)
            {
                if (x < p->lxl) x = p->lxl;
                if (y < p->lyl) y = p->lyl;
                if (x > p->hxl) x = p->hxl;
                if (y > p->hyl) y = p->hyl;

                if (useHQ && useHBoom)
                    createHBoom (x, y, HBoomYellow, HBoomHDelay, 255, 170, UNUSED);
                else
                    createBoom (x, y, 255, 170, UNUSED);

                removeSprite (p);
                delete (p);

                return 0;
            }

            x1 = p->x - p->hw;
            y1 = p->y - p->hh;
            w = p->shot->xres;
            h = p->shot->yres;

            while ((q = spriteCollision (t__enemy, p, x1, y1, w, h)) != NULL)
            {
                if (sendMessage1 (q, m__hit, p->t2))
                {
                    sendMessage (q, m__destroy);
                    killed = 1;
                }

                if (!p->remains--)
                {
                    removeSprite (p);
                    delete (p);

                    return 0;
                }
            }

            if (killed) return r__restart;

            drawFrame (p->x = x, p->y = y, p->shot, p->f);

            if (!p->tdelay--)
            {
                p->c++;

                if (p->c == 2 && p->delay)
                {
                    p->delay -= 8;
                    p->c = 0;

                    if (p->delay <= 0) p->delay = 0;
                }

                p->tdelay = p->delay;

                p->x += rcost (ShotSpeed, p->t2);
                p->y -= rsint (ShotSpeed, p->t2);
            }
            else
            {
                p->x += rcost (ShotSpeed, p->t1);
                p->y -= rsint (ShotSpeed, p->t1);
            }

            p->f++;
            if (p->f >= p->shot->fcount) p->f = 0;

            return 0;
    }

    int getWeaponDelay (int sl)
    {
            return dTable [sl];
    }

    void createShot (int x, int y, int t, int sl, int t2, int delay)
    {
        static int reentered = 0, initialized = 0;
        static anim_t *tt;
        shot_t *s;

            if (!initialized)
            {
                shot [0] = nla__load ("data/shot0.nla");
                if (!shot [0]) animDie ("shot0");

                shot [1] = nla__load ("data/shot1.nla");
                if (!shot [1]) animDie ("shot1");

                initialized = 1;
            }

            if (x == -1) return;

            if (!reentered)
            {
                reentered = 1;

                x += rcost (ShotDist, t &= 255);
                y -= rsint (ShotDist, t);

                tt = shot [sTable [sl]];

                switch (cTable [sl])
                {
                    case 0x02: //2
                        createShot (x, y, t, sl, t + ShotTDlt1, ShotTDelay);
                        createShot (x, y, t, sl, t - ShotTDlt1, ShotTDelay);
                        break;

                    case 0x03: //3
                        createShot (x, y, t, sl, t + ShotTDlt1, ShotTDelay);
                        createShot (x, y, t, sl, t - ShotTDlt1, ShotTDelay);
                        createShot (x, y, t, sl, t, ShotTDelay);
                        break;

                    case 0x05: //5
                        createShot (x, y, t, sl, t + ShotTDlt1, ShotTDelay);
                        createShot (x, y, t, sl, t - ShotTDlt1, ShotTDelay);
                        createShot (x, y, t, sl, t + ShotTDlt2, ShotTDelay);
                        createShot (x, y, t, sl, t - ShotTDlt2, ShotTDelay);
                        createShot (x, y, t, sl, t, ShotTDelay);
                        break;
                }

                playSound (S__SHOT);

                reentered = 0;
                return;
            }

            s = new (shot_t);

            s->t1 = t & 255;
            s->t2 = t2 & 255;

            s->x = x;
            s->y = y;

            s->remains = rTable [sl];
            s->shot = tt;
            s->s = sl;

            s->hw = s->shot->xres >> 1;
            s->hh = s->shot->yres >> 1;

            s->hyl = sHeight - s->hh;
            s->hxl = sWidth - s->hw;

            s->delay = delay;
            s->tdelay = 0;

            s->lxl = s->hw;
            s->lyl = s->hh;

            spriteHandler (s, handler);
            addSprite (s, t__sshot);
    }
