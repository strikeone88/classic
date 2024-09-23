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

        int         ok, x, y, t1, t2, f, s, c, sl;

        anim_t      *shot;

        int         remains, delay, tdelay;
    }
    shot_t;

    static anim_t *shot [4];

    unsigned char shot0d [], shot1d [];

    static int rTable [] = /* Resistance */
    {
        0, 0, 3, 16
    };

    static int vTable [] = /* Cost */
    {
        128, 256, 512, 4096
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
        int x, y, x1, y1, w, h, i, t1, t2, killed = 0;
        void *q;

            x = p->x;
            y = p->y;

            if (x < p->lxl || y < p->lyl || x > p->hxl || y > p->hyl)
            {
                if (x < p->lxl) x = p->lxl;
                if (y < p->lyl) y = p->lyl;
                if (x > p->hxl) x = p->hxl;
                if (y > p->hyl) y = p->hyl;

                createHBoom (x, y, HBoomYellow, HBoomHDelay, 255, 170, UNUSED, BoomDecF);

                removeSprite (p);
                delete (p);

                return 0;
            }

            if (p->sl == 3)
            {
                x1 = p->x - ((w = fire->xres) >> 1);
                y1 = p->y - ((h = fire->yres) >> 1);
            }
            else
            {
                x1 = p->x - p->hw;
                y1 = p->y - p->hh;
                w = p->shot->xres;
                h = p->shot->yres;
            }

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

            if (p->sl == 3)
            {
                createItem (p->x = x, p->y = y, 15, 0, RANDOM, t__item, fire, 0);
                createTrail (x, y, 2, fire, 32, 64);
            }
            else
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

    int getWeaponCost (int sl)
    {
            return vTable [sl];
    }

    void createShot (int x, int y, int t, int sl, int t2, int delay)
    {
        static int reentered = 0, initialized = 0;
        static anim_t *tt;
        shot_t *s;

            if (!initialized)
            {
                shot [0] = nla (NCreate, shot0d);
                if (!shot [0]) animDie ("shot0");

                shot [1] = nla (NCreate, shot1d);
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

                playSound (S__SHOT, 256, 256, 0);

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
            s->sl = sl;
            s->s = sl;

            s->hw = s->shot->xres >> 1;
            s->hh = s->shot->yres >> 1;

            s->hyl = sHeight - s->hh;
            s->hxl = sWidth - s->hw;

            s->tdelay = 0;

            s->lxl = s->hw;
            s->lyl = s->hh;

            spriteHandler (s, handler);
            addSprite (s, t__sshot);
    }

    unsigned char shot0d [] =
    {
      0x41, 0x35, 0x36, 0x35, 0x07, 0x00, 0x08, 0x00, 0x10, 0x00, 0x01, 0x00, 
      0x00, 0x08, 0x61, 0x28, 0x82, 0x30, 0xE2, 0x40, 0x46, 0xA2, 0x27, 0xAA, 
      0x87, 0xB2, 0xA7, 0xAA, 0xE8, 0xD4, 0xE6, 0xA3, 0xE7, 0xBC, 0x28, 0xCB, 
      0x29, 0xE6, 0xE7, 0xBD, 0x88, 0xD6, 0x00, 0x00, 0x01, 0x22, 0x21, 0x03, 
      0x45, 0x65, 0x43, 0x78, 0x9A, 0x98, 0x7B, 0xCD, 0xED, 0xCB, 0x78, 0x9A, 
      0x98, 0x73, 0x45, 0x65, 0x43, 0x01, 0x22, 0x21, 0x0F, 0xFF, 0xFF, 0xFF
    };
    
    unsigned char shot1d [] =
    {
      0x41, 0x35, 0x36, 0x35, 0x07, 0x00, 0x08, 0x00, 0x0E, 0x00, 0x01, 0x00, 
      0x20, 0x00, 0xC0, 0x00, 0x00, 0x01, 0x80, 0x01, 0xA1, 0x0B, 0xC0, 0x03, 
      0x01, 0x0C, 0xC5, 0x2D, 0x84, 0x24, 0x65, 0x2D, 0xA2, 0x14, 0xA7, 0x3E, 
      0xE7, 0x3D, 0x88, 0x46, 0x01, 0x22, 0x21, 0x03, 0x45, 0x65, 0x43, 0x67, 
      0x89, 0x87, 0x6A, 0xBC, 0xDC, 0xBA, 0x67, 0x89, 0x87, 0x63, 0x45, 0x65, 
      0x43, 0x01, 0x22, 0x21, 0x00, 0x00, 0x00, 0x00
    };
