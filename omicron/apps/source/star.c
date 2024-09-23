/*
    STAR.C

    Copyright (C) 2008 Novak Laboratories
    Written By J. Palencia (ilina@bloodykisses.zzn.com)
*/

    #include <game.h>

    typedef struct /* direct cast: sprite_t */
    {
        sprite_t    sprite;

        int         x, y, n;
    }
    star_t;

    /* Star sprite and data. */
    static anim_t *star;
    static unsigned char starD [];

    /* Sprite limits and half-resolutions. */
    static int hw, hh, hyl, hxl, lxl, lyl;

    static int handler (star_t *p)
    {
        int x, y, lsX1, lsY1;

            switch (p->n)
            {
                case 0:
                    lsX1 = 0;
                    lsY1 = 0;
                    break;

                case 1:
                    lsX1 = sX1 >> 3;
                    lsY1 = sY1 >> 3;
                    break;

                case 2:
                    lsX1 = sX1 >> 2;
                    lsY1 = sY1 >> 2;
                    break;
            }

            drawFrameWsx (p->x, p->y, star, p->n, lsX1, lsY1);

            return 0;
    }

    void initMisc (void);

    void createStar (int x, int y, int c)
    {
        static int initialized = 0;
        star_t *s;

            if (!initialized)
            {
                star = nla (NCreate, starD);
                if (!star) animDie ("star");

                hw = star->xres >> 1;
                hh = star->yres >> 1;

                hyl = sHeight - hh;
                hxl = sWidth - hw;

                lxl = hw;
                lyl = hh;

                initialized = 1;

                initMisc ();
            }

            if (x == -1) return;

            if (x < lxl || y < lyl || x > hxl || y > hyl)
                return;

            s = new (star_t);

            s->n = c % 3;

            s->x = x;
            s->y = y;

            spriteHandler (s, handler);
            addSprite (s, t__back);
    }

    unsigned char starD [] =
    {
      0x41, 0x35, 0x36, 0x35, 0x08, 0x00, 0x06, 0x00, 0x10, 0x00, 0x03, 0x00, 
      0x00, 0x00, 0x20, 0x00, 0x61, 0x08, 0xA2, 0x10, 0xC3, 0x18, 0x45, 0x29, 
      0xE7, 0x39, 0xA6, 0x31, 0x8A, 0x52, 0xEF, 0x7B, 0xE3, 0x18, 0xC7, 0x39, 
      0xAA, 0x52, 0x8E, 0x73, 0x55, 0xAD, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 
      0x00, 0x12, 0x32, 0x10, 0x00, 0x24, 0x54, 0x20, 0x00, 0x35, 0x65, 0x30, 
      0x00, 0x24, 0x54, 0x20, 0x00, 0x12, 0x32, 0x10, 0x00, 0x00, 0x00, 0x00, 
      0x00, 0x24, 0x54, 0x20, 0x00, 0x47, 0x87, 0x40, 0x00, 0x58, 0x98, 0x50, 
      0x00, 0x47, 0x87, 0x40, 0x00, 0x24, 0x54, 0x20, 0x00, 0x00, 0x00, 0x00, 
      0x00, 0xAB, 0xCB, 0xA0, 0x00, 0xBD, 0xED, 0xB0, 0x00, 0xCE, 0xFE, 0xC0, 
      0x00, 0xBD, 0xED, 0xB0, 0x00, 0xAB, 0xCB, 0xA0
    };

/*MISC*/

    #include "dollar.h"
    #include "econt.h"
    #include "bomb.h"
    #include "fire.h"
    #include "gen.h"

    anim_t *dollar, *bomb, *econt, *gen, *fire;

    void initMisc (void)
    {
            dollar = nla (NCreate, dollarD);
            if (!dollar) animDie ("dollar");

            bomb = nla (NCreate, bombD);
            if (!bomb) animDie ("bomb");

            econt = nla (NCreate, econtD);
            if (!econt) animDie ("econt");

            gen = nla (NCreate, genD);
            if (!gen) animDie ("gen");

            fire = nla (NCreate, fireD);
            if (!fire) animDie ("fire");
    }
