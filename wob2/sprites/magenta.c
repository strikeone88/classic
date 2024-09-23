/*
    MAGENTA.C

    Copyright (C) 2008 Novak Laboratories
    Written by J. Palencia (ilina@bloodykisses.zzn.com)
*/

    #include <game.h>

    typedef struct /* direct cast: sprite_t */
    {
        sprite_t    sprite;

        int         x, y, f, p, r, ok;

        int         m, b, s, c, shield;

        int         frameDelay;
    }
    magenta_t;

    static anim_t *magenta, *smagenta;

    static int lxl, lyl, hxl, hyl, hw, hh;

    static int handler (magenta_t *p, int m, ...)
    {
        int x, y, x1, y1, w, h, b, i, t;
        static unsigned long r;
        static va_list args;
        static void *q;

            va_start (args, m);

            switch (m)
            {
                case m__damageLevel:
                    return 3;

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

                case m__suicide:
                case m__destroy:
           Destroy: if (useHQ && useHBoom)
                        createHBoom (p->x, p->y, HBoomMagenta, HBoomHDelay, 255, UNUSED, UNUSED);
                    else
                        createBoom (p->x, p->y, 255, UNUSED, UNUSED);

                    if (score && m != m__suicide)
                    {
                        createMMagenta (x + rcost (8, _90deg), y - rsint (8, _90deg));
                        createMMagenta (x + rcost (8, _270deg), y - rsint (8, _270deg));

                        ((sprite_t *)score)->handler (score, m__addKill);

                        ((sprite_t *)score)->handler (score, m__getMultiplier, &r);
                        ((sprite_t *)score)->handler (score, m__addScore, r *= MagentaPrice);

                        sprintf (g__tempBuf, "%lu", r);
                        createText (p->x, p->y, g__tempBuf, 255, 255, 255, TextDecF);
                    }

                    removeSprite (p);
                    delete (p);

                    break;

                case m__collides:
                    if (!p->ok) break;

                    x = va_arg (args, int);
                    y = va_arg (args, int);
                    x1 = va_arg (args, int);
                    y1 = va_arg (args, int);

                    return detectCollision (p->x - hw, p->y - hh,
                            magenta->xres, magenta->yres, x, y, x1, y1);

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

            x1 = x - hw;
            y1 = y - hh;
            w = magenta->xres;
            h = magenta->yres;

            if ((q = spriteCollision (t__ship, p, x1, y1, w, h)) != NULL)
            {
                if (!p->shield) sendMessage (q, m__destroy);
                if (sendMessage (q, m__activeShield)) goto Destroy;
            }

            if (p->shield) p->shield--;

            drawFrame (p->x = x, p->y = y, p->shield ? smagenta : magenta, p->f);

            p->ok = 1;

            if (!p->frameDelay--)
            {
                p->frameDelay = MagentaFDelay;

                p->f++;
                if (p->f >= magenta->fcount) p->f = 0;
            }

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
                        p->x = m * (p->y -= MagentaSpeed) + b;
                    else
                        p->x = m * (p->y += MagentaSpeed) + b;
                }
                else
                {
                    p->m = m = (y1 - y) / (x1 - x);
                    p->b = b = y - m*x;

                    if ((p->s = x1 < x) != 0)
                        p->y = m * (p->x -= MagentaSpeed) + b;
                    else
                        p->y = m * (p->x += MagentaSpeed) + b;
                }

                p->p = MagentaUpdate;
            }
            else
            {
                if (p->c)
                {
                    if (p->s)
                        p->x = p->m * (p->y -= MagentaSpeed) + p->b;
                    else
                        p->x = p->m * (p->y += MagentaSpeed) + p->b;
                }
                else
                {
                    if (p->s)
                        p->y = p->m * (p->x -= MagentaSpeed) + p->b;
                    else
                        p->y = p->m * (p->x += MagentaSpeed) + p->b;
                }
            }

            return 0;
    }

    void createMagenta (int x, int y)
    {
        static int initialized = 0;
        magenta_t *s;

            if (!initialized)
            {
                if (useHQ)
                {
                    magenta = nla__load ("data/mag.nla");
                    if (!magenta) animDie ("mag");

                    smagenta = nla__load ("data/smag.nla");
                    if (!smagenta) animDie ("smag");
                }
                else
                {
                    magenta = nla__load ("data/maglq.nla");
                    if (!magenta) animDie ("maglq");

                    smagenta = nla__load ("data/smaglq.nla");
                    if (!smagenta) animDie ("smaglq");
                }

                hw = magenta->xres >> 1;
                hh = magenta->yres >> 1;

                hyl = sHeight - hh;
                hxl = sWidth - hw;

                lxl = hw;
                lyl = hh;

                initialized = 1;
            }

            if (x == -1) return;

            s = new (magenta_t);

            s->x = x;
            s->y = y;

            s->frameDelay = MagentaFDelay;
            s->shield = MagentaShield;

            s->r = MagentaR;

            spriteHandler (s, handler);
            addSprite (s, t__enemy);
    }
