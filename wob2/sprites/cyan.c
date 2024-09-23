/*
    CYAN.C

    Copyright (C) 2008 Novak Laboratories
    Written by J. Palencia (ilina@bloodykisses.zzn.com)
*/

    #include <game.h>

    typedef struct /* direct cast: sprite_t */
    {
        sprite_t    sprite;

        int         x, y, f, p, r, ok;

        int         m, b, s, c, shield;
    }
    cyan_t;

    static int lxl, lyl, hxl, hyl;
    static anim_t *cyan, *scyan;
    static int hw, hh;

    static int handler (cyan_t *p, int m, ...)
    {
        int x, y, x1, y1, w, h, b, i, t;
        static unsigned long r;
        static va_list args;
        static void *q;

            va_start (args, m);

            switch (m)
            {
                case m__damageLevel:
                    return 2;

                case m__hit:
                    if (p->r--)
                    {
                        t = va_arg (args, unsigned);

                        p->x += rcost (ShotSpeed>>1, t);
                        p->y -= rsint (ShotSpeed>>1, t);

                        return 0;
                    }
                    else
                        return 1;

                case m__getX:
                    return p->x;

                case m__getY:
                    return p->y;

                case m__getW:
                    return cyan->xres;

                case m__getH:
                    return cyan->yres;

                case m__suicide:
                case m__destroy:
           Destroy: if (useHQ && useHBoom)
                        createHBoom (p->x, p->y, HBoomGreen, HBoomHDelay, UNUSED, 255, UNUSED);
                    else
                        createBoom (p->x, p->y, UNUSED, 255, UNUSED);

                    if (score && m != m__suicide)
                    {
                        ((sprite_t *)score)->handler (score, m__addKill);

                        ((sprite_t *)score)->handler (score, m__getMultiplier, &r);
                        ((sprite_t *)score)->handler (score, m__addScore, r *= CyanPrice);

                        sprintf (g__tempBuf, "%lu", r);
                        createText (p->x, p->y, g__tempBuf, 255, 255, 255, TextDecF);
                    }

                    removeSprite (p);
                    delete (p);

                    playSound (S__HIT);

                    break;

                case m__collides:
                    if (!p->ok) break;

                    x = va_arg (args, int);
                    y = va_arg (args, int);
                    x1 = va_arg (args, int);
                    y1 = va_arg (args, int);

                    return detectCollision (p->x - hw, p->y - hh,
                            cyan->xres, cyan->yres, x, y, x1, y1);

                case m__update:
                    goto Default;
            }

            return 0;

   Default: x = p->x;
            y = p->y;

            if (x < lxl) x = lxl;
            if (y < lyl) y = lyl;
            if (x > hxl) x = hxl;
            if (y > hyl) y = hyl;

            w = cyan->xres;
            h = cyan->yres;

            x1 = x - hw;
            y1 = y - hh;

            if ((q = spriteCollision (t__ship, p, x1, y1, w, h)) != NULL)
            {
                if (!p->shield) sendMessage (q, m__destroy);
                if (sendMessage (q, m__activeShield)) goto Destroy;
            }

            if (p->shield) p->shield--;

            drawFrame (p->x = x, p->y = y, p->shield ? scyan : cyan, p->f);

            p->ok = 1;

            p->f++;
            if (p->f >= cyan->fcount) p->f = 0;

            if (!player) return 0;

            x1 = sendMessage (player, m__getX);
            y1 = sendMessage (player, m__getY);

            i = abs (x1 - x);
            m = abs (y1 - y);

            if (!i && !m) return 0;

            if (!p->p--)
            {
                if ((p->c = m > i) != 0)
                {
                    p->m = m = (x1 - x) / (y1 - y);
                    p->b = b = x - m*y;

                    if ((p->s = y1 < y) != 0)
                        p->x = m * (p->y -= CyanSpeed) + b;
                    else
                        p->x = m * (p->y += CyanSpeed) + b;
                }
                else
                {
                    p->m = m = (y1 - y) / (x1 - x);
                    p->b = b = y - m*x;

                    if ((p->s = x1 < x) != 0)
                        p->y = m * (p->x -= CyanSpeed) + b;
                    else
                        p->y = m * (p->x += CyanSpeed) + b;
                }

                p->p = CyanUpdate;
            }
            else
            {
                if (p->c)
                {
                    if (p->s)
                        p->x = p->m * (p->y -= CyanSpeed) + p->b;
                    else
                        p->x = p->m * (p->y += CyanSpeed) + p->b;
                }
                else
                {
                    if (p->s)
                        p->y = p->m * (p->x -= CyanSpeed) + p->b;
                    else
                        p->y = p->m * (p->x += CyanSpeed) + p->b;
                }
            }

            return 0;
    }

    void createCyan (int x, int y)
    {
        static int initialized = 0;
        cyan_t *s;

            if (!initialized)
            {
                if (useHQ)
                {
                    cyan = nla__load ("data/cyan.nla");
                    if (!cyan) animDie ("cyan");

                    scyan = nla__load ("data/scyan.nla");
                    if (!scyan) animDie ("scyan");
                }
                else
                {
                    cyan = nla__load ("data/cyanlq.nla");
                    if (!cyan) animDie ("cyanlq");

                    scyan = nla__load ("data/scyanlq.nla");
                    if (!scyan) animDie ("scyanlq");
                }

                hw = cyan->xres >> 1;
                hh = cyan->yres >> 1;

                hyl = sHeight - hh;
                hxl = sWidth - hw;

                lxl = hw;
                lyl = hh;

                initialized = 1;
            }

            if (x == -1) return;

            s = new (cyan_t);

            s->x = x;
            s->y = y;

            s->shield = CyanShield;
            s->r = CyanR;

            spriteHandler (s, handler);
            addSprite (s, t__enemy);
    }
