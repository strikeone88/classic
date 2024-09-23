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

    /* Star sprite. */
    static anim_t *star;

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

    void createStar (int x, int y, int c)
    {
        static int initialized = 0;
        star_t *s;

            if (!initialized)
            {
                if (useHQ)
                {
                    star = nla__load ("data/star.nla");
                    if (!star) animDie ("star");
                }
                else
                {
                    star = nla__load ("data/starlq.nla");
                    if (!star) animDie ("starlq");
                }

                hw = star->xres >> 1;
                hh = star->yres >> 1;

                hyl = sHeight - hh;
                hxl = sWidth - hw;

                lxl = hw;
                lyl = hh;

                initialized = 1;
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
